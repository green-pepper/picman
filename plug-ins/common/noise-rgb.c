/*
 * This is a plugin for PICMAN.
 *
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 * Copyright (C) 1996 Torsten Martinsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This filter adds random noise to an image.
 * The amount of noise can be set individually for each RGB channel.
 * This filter does not operate on indexed images.
 *
 * May 2000 tim copperfield [timecop@japan.co.jp]
 * Added dynamic preview.
 *
 * alt@picman.org. Fixed previews so they handle alpha channels correctly.
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define RGB_NOISE_PROC   "plug-in-rgb-noise"
#define NOISIFY_PROC     "plug-in-noisify"
#define PLUG_IN_BINARY   "noise-rgb"
#define PLUG_IN_ROLE     "picman-noise-rgb"
#define SCALE_WIDTH      125
#define TILE_CACHE_SIZE  16


typedef struct
{
  gboolean   independent;
  gboolean   correlated ;
  gdouble    noise[4];     /*  per channel  */
} NoisifyVals;

typedef struct
{
  gint       channels;
  GtkObject *channel_adj[4];
} NoisifyInterface;


/* Declare local functions.
 */
static void     query        (void);
static void     run          (const gchar      *name,
                              gint              nparams,
                              const PicmanParam  *param,
                              gint             *nreturn_vals,
                              PicmanParam       **return_vals);


static void     noisify_func (const guchar     *src,
                              guchar           *dest,
                              gint              bpp,
                              gpointer          data);

static void     noisify      (PicmanPreview      *preview);


static gdouble  gauss                            (GRand         *gr);

static gboolean noisify_dialog                   (PicmanDrawable  *drawable,
                                                  gint           channels);
static void     noisify_double_adjustment_update (GtkAdjustment *adjustment,
                                                  gpointer       data);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc */
  NULL,  /* quit_proc */
  query, /* query_proc */
  run,   /* run_proc */
};

static NoisifyVals nvals =
{
  TRUE,
  FALSE,
  { 0.20, 0.20, 0.20, 0.0 }
};

static NoisifyInterface noise_int =
{
  0,
  { NULL, NULL, NULL, NULL }
};

static GRand *noise_gr ; /* Global random number object */

MAIN ()

static void
query (void)
{
  static const PicmanParamDef scatter_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",       "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",    "Input drawable" },
    { PICMAN_PDB_INT32,    "independent", "Noise in channels independent" },
    { PICMAN_PDB_INT32,    "correlated",  "Noise correlated (i.e. multiplicative not additive)" },
    { PICMAN_PDB_FLOAT,    "noise-1",     "Noise in the first channel (red, gray)" },
    { PICMAN_PDB_FLOAT,    "noise-2",     "Noise in the second channel (green, gray_alpha)" },
    { PICMAN_PDB_FLOAT,    "noise-3",     "Noise in the third channel (blue)" },
    { PICMAN_PDB_FLOAT,    "noise-4",     "Noise in the fourth channel (alpha)" }
  };

  static const PicmanParamDef noisify_args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",       "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable",    "Input drawable" },
    { PICMAN_PDB_INT32,    "independent", "Noise in channels independent" },
    { PICMAN_PDB_FLOAT,    "noise-1",     "Noise in the first channel (red, gray)" },
    { PICMAN_PDB_FLOAT,    "noise-2",     "Noise in the second channel (green, gray_alpha)" },
    { PICMAN_PDB_FLOAT,    "noise-3",     "Noise in the third channel (blue)" },
    { PICMAN_PDB_FLOAT,    "noise-4",     "Noise in the fourth channel (alpha)" }
  };


  picman_install_procedure (RGB_NOISE_PROC,
                          N_("Distort colors by random amounts"),
                          "Add normally distributed (zero mean) random values "
                          "to image channels.  Noise may be additive "
                          "(uncorrelated) or multiplicative (correlated - "
                          "also known as speckle noise). "
                          "For colour images colour channels may be treated "
                          "together or independently.",
                          "Torsten Martinsen",
                          "Torsten Martinsen",
                          "May 2000",
                          N_("_RGB Noise..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (scatter_args), 0,
                          scatter_args, NULL);

  picman_plugin_menu_register (RGB_NOISE_PROC, "<Image>/Filters/Noise");

  picman_install_procedure (NOISIFY_PROC,
                          "Adds random noise to image channels ",
                          "Add normally distributed random values to "
                          "image channels. For colour images each "
                          "colour channel may be treated together or "
                          "independently.",
                          "Torsten Martinsen",
                          "Torsten Martinsen",
                          "May 2000",
                          NULL,
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (noisify_args), 0,
                          noisify_args, NULL);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanRunMode        run_mode;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  noise_gr = g_rand_new ();

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);
  picman_tile_cache_ntiles (TILE_CACHE_SIZE);

  if (picman_drawable_is_gray (drawable->drawable_id))
    nvals.noise[1] = 0.0;

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (RGB_NOISE_PROC, &nvals);

      /*  First acquire information with a dialog  */
      if (! noisify_dialog (drawable, drawable->bpp))
        {
          picman_drawable_detach (drawable);
          g_rand_free (noise_gr);
          return;
        }
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (strcmp (name, NOISIFY_PROC) == 0)
        {
          /*  Make sure all the arguments are there!  */
          if (nparams != 8)
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          else
            {
              nvals.independent =
                param[3].data.d_int32 ? TRUE : FALSE;
              nvals.correlated  = FALSE ;
              nvals.noise[0]    = param[4].data.d_float;
              nvals.noise[1]    = param[5].data.d_float;
              nvals.noise[2]    = param[6].data.d_float;
              nvals.noise[3]    = param[7].data.d_float;
            }
        }
      else if (strcmp (name, RGB_NOISE_PROC) == 0)
        {
          if (nparams != 9)
            {
              status = PICMAN_PDB_CALLING_ERROR;
            }
          else
            {
              nvals.independent =
                param[3].data.d_int32 ? TRUE : FALSE;
              nvals.correlated  =
                param[4].data.d_int32 ? TRUE : FALSE;
              nvals.noise[0]    = param[5].data.d_float;
              nvals.noise[1]    = param[6].data.d_float;
              nvals.noise[2]    = param[7].data.d_float;
              nvals.noise[3]    = param[8].data.d_float;
            }
        }
      else
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (RGB_NOISE_PROC, &nvals);
      break;

    default:
      break;
    }

  /*  Make sure that the drawable is gray or RGB color  */
  if (picman_drawable_is_indexed (drawable->drawable_id))
    status = PICMAN_PDB_EXECUTION_ERROR;

  if (status == PICMAN_PDB_SUCCESS)
    {
      picman_progress_init (_("Adding noise"));

      /*  compute the luminosity which exceeds the luminosity threshold  */
      picman_rgn_iterate2 (drawable, 0 /* unused */, noisify_func, noise_gr);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /*  Store data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (RGB_NOISE_PROC, &nvals, sizeof (NoisifyVals));
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
  g_rand_free (noise_gr);
}

static void
noisify_func (const guchar *src,
              guchar       *dest,
              gint          bpp,
              gpointer      data)
{
  GRand   *gr    = data;
  gdouble  noise = 0;
  gint     b;

  for (b = 0; b < bpp; b++)
    {
      if (b == 0 || nvals.independent ||
          (b == 1 && bpp == 2) || (b == 3 && bpp == 4))
        noise = nvals.noise[b] * gauss (gr) * 127;

      if (nvals.noise[b] > 0.0)
        {
          gint p;

          if (nvals.correlated)
            {
              p = (gint) (src[b] + (src[b] * (noise / 127.0)) + 0.5);
            }
          else
            {
              p = (gint) (src[b] + noise + 0.5);
            }

          dest[b] = CLAMP0255 (p);
        }
      else
        {
          dest[b] = src[b];
        }
    }
}

static void
noisify (PicmanPreview *preview)
{
  PicmanDrawable *drawable;
  PicmanPixelRgn  src_rgn;
  guchar       *src, *dst;
  gint          i;
  gint          x1, y1;
  gint          width, height;
  gint          bpp;
  GRand        *gr = g_rand_copy (noise_gr);

  drawable =
    picman_drawable_preview_get_drawable (PICMAN_DRAWABLE_PREVIEW (preview));

  picman_preview_get_position (preview, &x1, &y1);
  picman_preview_get_size (preview, &width, &height);

  bpp = drawable->bpp;

  src = g_new (guchar, width * height * bpp);
  dst = g_new (guchar, width * height * bpp);

  picman_pixel_rgn_init (&src_rgn, drawable,
                       x1, y1, width, height,
                       FALSE, FALSE);
  picman_pixel_rgn_get_rect (&src_rgn, src, x1, y1, width, height);

  for (i = 0; i < width * height; i++)
    noisify_func (src + i * bpp, dst + i * bpp, bpp, gr);

  picman_preview_draw_buffer (preview, dst, width * bpp);

  g_free (src);
  g_free (dst);
  g_rand_free (gr);
}

static void
noisify_add_channel (GtkWidget    *table,
                     gint          channel,
                     const gchar  *name,
                     PicmanDrawable *drawable,
                     GtkWidget    *preview)
{
  GtkObject *adj;

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, channel + 1,
                              name, SCALE_WIDTH, 0,
                              nvals.noise[channel], 0.0, 1.0, 0.01, 0.1, 2,
                              TRUE, 0, 0,
                              NULL, NULL);

  g_object_set_data (G_OBJECT (adj), "drawable", drawable);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (noisify_double_adjustment_update),
                    &nvals.noise[channel]);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  noise_int.channel_adj[channel] = adj;
}

static void
noisify_add_alpha_channel (GtkWidget    *table,
                           gint          channel,
                           const gchar  *name,
                           PicmanDrawable *drawable,
                           GtkWidget    *preview)
{
  GtkObject *adj;

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, channel + 1,
                              name, SCALE_WIDTH, 0,
                              nvals.noise[channel], 0.0, 1.0, 0.01, 0.1, 2,
                              TRUE, 0, 0,
                              NULL, NULL);

  g_object_set_data (G_OBJECT (adj), "drawable", drawable);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &nvals.noise[channel]);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  noise_int.channel_adj[channel] = adj;
}

static gboolean
noisify_dialog (PicmanDrawable *drawable,
                gint          channels)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *vbox;
  GtkWidget *preview;
  GtkWidget *toggle;
  GtkWidget *table;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("RGB Noise"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, RGB_NOISE_PROC,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_window_set_transient (GTK_WINDOW (dialog));

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  preview = picman_drawable_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, FALSE, FALSE, 0);
  gtk_widget_show (preview);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (noisify),
                    NULL);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  toggle = gtk_check_button_new_with_mnemonic (_("Co_rrelated noise"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                nvals.correlated);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &nvals.correlated);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  if (picman_drawable_is_rgb (drawable->drawable_id))
    {
      toggle = gtk_check_button_new_with_mnemonic (_("_Independent RGB"));
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                    nvals.independent);
      gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
      gtk_widget_show (toggle);

      g_signal_connect (toggle, "toggled",
                        G_CALLBACK (picman_toggle_button_update),
                        &nvals.independent);
      g_signal_connect_swapped (toggle, "toggled",
                                G_CALLBACK (picman_preview_invalidate),
                                preview);
    }

  table = gtk_table_new (channels, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  noise_int.channels = channels;

  if (channels == 1)
    {
      noisify_add_channel (table, 0, _("_Gray:"), drawable, preview);
    }
  else if (channels == 2)
    {
      noisify_add_channel (table, 0, _("_Gray:"), drawable, preview);
      noisify_add_alpha_channel (table, 1, _("_Alpha:"), drawable, preview);
      gtk_table_set_row_spacing (GTK_TABLE (table), 1, 15);
    }
  else if (channels == 3)
    {
      noisify_add_channel (table, 0, _("_Red:"), drawable, preview);
      noisify_add_channel (table, 1, _("_Green:"), drawable, preview);
      noisify_add_channel (table, 2, _("_Blue:"), drawable, preview);
    }

  else if (channels == 4)
    {
      noisify_add_channel (table, 0, _("_Red:"), drawable, preview);
      noisify_add_channel (table, 1, _("_Green:"), drawable, preview);
      noisify_add_channel (table, 2, _("_Blue:"), drawable, preview);
      noisify_add_alpha_channel (table, 3, _("_Alpha:"), drawable, preview);
      gtk_table_set_row_spacing (GTK_TABLE (table), 3, 15);
    }
  else
    {
      gchar *buffer;
      gint   i;

      for (i = 0; i < channels; i++)
        {
          buffer = g_strdup_printf (_("Channel #%d:"), i);

          noisify_add_channel (table, i, buffer, drawable, preview);

          g_free (buffer);
        }
    }

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

/*
 * Return a Gaussian (aka normal) distributed random variable.
 *
 * Adapted from gauss.c included in GNU scientific library.
 *
 * Ratio method (Kinderman-Monahan); see Knuth v2, 3rd ed, p130
 * K+M, ACM Trans Math Software 3 (1977) 257-260.
*/
static gdouble
gauss (GRand *gr)
{
  gdouble u, v, x;

  do
    {
      v = g_rand_double (gr);

      do
        u = g_rand_double (gr);
      while (u == 0);

      /* Const 1.715... = sqrt(8/e) */
      x = 1.71552776992141359295 * (v - 0.5) / u;
    }
  while (x * x > -4.0 * log (u));

  return x;
}

static void
noisify_double_adjustment_update (GtkAdjustment *adjustment,
                                  gpointer       data)
{
  gint n;

  picman_double_adjustment_update (adjustment, data);

  if (! nvals.independent)
    {
      gint i;

      switch (noise_int.channels)
        {
        case 1:  n = 1;  break;
        case 2:  n = 1;  break;
        case 3:  n = 3;  break;
        default: n = 3;  break;
        }

      for (i = 0; i < n; i++)
        if (adjustment != GTK_ADJUSTMENT (noise_int.channel_adj[i]))
          gtk_adjustment_set_value (GTK_ADJUSTMENT (noise_int.channel_adj[i]),
                                    gtk_adjustment_get_value (adjustment));
    }
}
