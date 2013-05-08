/*
 * illusion.c  -- This is a plug-in for PICMAN 1.0
 *
 * Copyright (C) 1997  Hirotsuna Mizuno
 *                     s1041150@u-aizu.ac.jp
 *
 * Preview and new mode added May 2000 by tim copperfield
 *                     timecop@japan.co.jp
 *                     http://www.ne.jp/asahi/linux/timecop
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
 *
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC    "plug-in-illusion"
#define PLUG_IN_BINARY  "illusion"
#define PLUG_IN_ROLE    "picman-illusion"
#define PLUG_IN_VERSION "v0.8 (May 14 2000)"


static void      query  (void);
static void      run    (const gchar      *name,
                         gint              nparam,
                         const PicmanParam  *param,
                         gint             *nreturn_vals,
                         PicmanParam       **return_vals);

static void      illusion         (PicmanDrawable *drawable);
static void      illusion_preview (PicmanPreview  *preview,
                                   PicmanDrawable *drawable);
static gboolean  illusion_dialog  (PicmanDrawable *drawable);

typedef struct
{
  gint32   division;
  gboolean type1;
  gboolean type2;
} IllValues;

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run    /* run_proc   */
};

static IllValues parameters =
{
  8,     /* division */
  TRUE,  /* type 1 */
  FALSE  /* type 2 */
};

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image"                            },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable"                         },
    { PICMAN_PDB_INT32,    "division",  "The number of divisions"                },
    { PICMAN_PDB_INT32,    "type",      "Illusion type { TYPE1 (0), TYPE2 (1) }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Superimpose many altered copies of the image"),
                          "produce illusion",
                          "Hirotsuna Mizuno <s1041150@u-aizu.ac.jp>",
                          "Hirotsuna Mizuno",
                          PLUG_IN_VERSION,
                          N_("_Illusion..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Map");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *params,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   returnv[1];
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;
  PicmanRunMode        run_mode;
  PicmanDrawable      *drawable;

  INIT_I18N ();

  /* get the drawable info */
  run_mode = params[0].data.d_int32;
  drawable = picman_drawable_get (params[2].data.d_drawable);

  *nreturn_vals = 1;
  *return_vals  = returnv;

  /* switch the run mode */
  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      picman_get_data (PLUG_IN_PROC, &parameters);
      if (! illusion_dialog (drawable))
        return;
      picman_set_data (PLUG_IN_PROC, &parameters, sizeof (IllValues));
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (nparams != 5)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          parameters.division = params[3].data.d_int32;
          if (params[4].data.d_int32 == 0)
            {
              parameters.type1 = TRUE;
              parameters.type2 = FALSE;
            }
          else
            {
              parameters.type1 = FALSE;
              parameters.type2 = TRUE;
            }
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      picman_get_data (PLUG_IN_PROC, &parameters);
      break;
    }

  if (status == PICMAN_PDB_SUCCESS)
    {
      if (picman_drawable_is_rgb (drawable->drawable_id) ||
          picman_drawable_is_gray (drawable->drawable_id))
        {
          picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width() + 1));
          picman_progress_init (_("Illusion"));
          illusion (drawable);
          if (run_mode != PICMAN_RUN_NONINTERACTIVE)
            picman_displays_flush ();
        }
      else
        {
          status = PICMAN_PDB_EXECUTION_ERROR;
        }
    }

  returnv[0].type          = PICMAN_PDB_STATUS;
  returnv[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

typedef struct {
  PicmanPixelFetcher *pft;
  gdouble           center_x;
  gdouble           center_y;
  gdouble           scale;
  gdouble           offset;
  gboolean          has_alpha;
} IllusionParam_t;

static void
illusion_func (gint          x,
               gint          y,
               const guchar *src,
               guchar       *dest,
               gint          bpp,
               gpointer      data)
{
  IllusionParam_t *param = (IllusionParam_t*) data;
  gint             xx, yy, b;
  gdouble          radius, cx, cy, angle;
  guchar           pixel[4];

  cy = ((gdouble) y - param->center_y) / param->scale;
  cx = ((gdouble) x - param->center_x) / param->scale;

  angle = floor (atan2 (cy, cx) * parameters.division / G_PI_2)
    * G_PI_2 / parameters.division + (G_PI / parameters.division);
  radius = sqrt ((gdouble) (cx * cx + cy * cy));

  if (parameters.type1)
    {
      xx = x - param->offset * cos (angle);
      yy = y - param->offset * sin (angle);
    }
  else                          /* Type 2 */
    {
      xx = x - param->offset * sin (angle);
      yy = y - param->offset * cos (angle);
    }

  picman_pixel_fetcher_get_pixel (param->pft, xx, yy, pixel);

  if (param->has_alpha)
    {
      guint alpha1 = src[bpp - 1];
      guint alpha2 = pixel[bpp - 1];
      guint alpha  = (1 - radius) * alpha1 + radius * alpha2;

      if ((dest[bpp - 1] = (alpha >> 1)))
        {
          for (b = 0; b < bpp - 1; b++)
            dest[b] = ((1 - radius) * src[b] * alpha1 +
                       radius * pixel[b] * alpha2) / alpha;
        }
    }
  else
    {
      for (b = 0; b < bpp; b++)
        dest[b] = (1 - radius) * src[b] + radius * pixel[b];
    }
}

static void
illusion (PicmanDrawable *drawable)
{
  IllusionParam_t  param;
  PicmanRgnIterator *iter;
  gint             width, height;
  gint             x1, y1, x2, y2;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  width  = x2 - x1;
  height = y2 - y1;

  param.pft = picman_pixel_fetcher_new (drawable, FALSE);
  picman_pixel_fetcher_set_edge_mode (param.pft, PICMAN_PIXEL_FETCHER_EDGE_SMEAR);

  param.has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  param.center_x = (x1 + x2) / 2.0;
  param.center_y = (y1 + y2) / 2.0;
  param.scale = sqrt (width * width + height * height) / 2;
  param.offset = (gint) (param.scale / 2);

  iter = picman_rgn_iterator_new (drawable, 0);
  picman_rgn_iterator_src_dest (iter, illusion_func, &param);
  picman_rgn_iterator_free (iter);

  picman_pixel_fetcher_destroy (param.pft);
}

static void
illusion_preview (PicmanPreview  *preview,
                  PicmanDrawable *drawable)

{
  gint                  x, y;
  gint                  sx, sy;
  gint                  preview_width, preview_height;
  guchar               *src;
  guchar               *dest;
  guchar               *src_pixel;
  guchar               *dest_pixel;
  gint                  bpp;
  IllusionParam_t       param;
  gint                  width, height;
  gint                  x1, y1, x2, y2;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  width  = x2 - x1;
  height = y2 - y1;

  param.pft = picman_pixel_fetcher_new (drawable, FALSE);
  picman_pixel_fetcher_set_edge_mode (param.pft, PICMAN_PIXEL_FETCHER_EDGE_SMEAR);

  param.has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  param.center_x = (x1 + x2) / 2.0;
  param.center_y = (y1 + y2) / 2.0;
  param.scale = sqrt (width * width + height * height) / 2;
  param.offset = (gint) (param.scale / 2);

  src = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                      &preview_width, &preview_height, &bpp);
  dest = g_malloc (preview_width * preview_height * bpp);

  src_pixel = src;
  dest_pixel = dest;

  for (y = 0; y < preview_height; y++)
    {
      for (x = 0; x < preview_width; x++)
        {
          picman_preview_untransform (preview, x, y, &sx, &sy);

          illusion_func (sx, sy,
                         src_pixel, dest_pixel,
                         bpp,
                         (gpointer) &param);

          src_pixel += bpp;
          dest_pixel += bpp;
        }
    }

  picman_pixel_fetcher_destroy (param.pft);

  picman_preview_draw_buffer (preview, dest, preview_width * bpp);
  g_free (dest);
  g_free (src);
}

static gboolean
illusion_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkWidget *spinbutton;
  GtkObject *adj;
  GtkWidget *radio;
  GSList    *group = NULL;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Illusion"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

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

  preview = picman_zoom_preview_new (drawable);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (illusion_preview),
                    drawable);

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  spinbutton = picman_spin_button_new (&adj, parameters.division,
                                     -32, 64, 1, 10, 0, 1, 0);
  picman_table_attach_aligned (GTK_TABLE (table), 0, 0,
                             _("_Divisions:"), 0.0, 0.5,
                             spinbutton, 1, TRUE);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &parameters.division);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  radio = gtk_radio_button_new_with_mnemonic (group, _("Mode _1"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
  gtk_table_attach (GTK_TABLE (table), radio, 0, 2, 1, 2,
                    GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (radio);

  g_signal_connect (radio, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &parameters.type1);
  g_signal_connect_swapped (radio, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), parameters.type1);

  radio = gtk_radio_button_new_with_mnemonic (group, _("Mode _2"));
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
  gtk_table_attach (GTK_TABLE (table), radio, 0, 2, 2, 3,
                    GTK_FILL, GTK_FILL, 0, 0);
  gtk_widget_show (radio);

  g_signal_connect (radio, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &parameters.type2);
  g_signal_connect_swapped (radio, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), parameters.type2);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
