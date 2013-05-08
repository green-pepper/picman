/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

/* This plugin by thorsten@arch.usyd.edu.au           */
/* Based on S&P's Gauss and Blur filters              */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC   "plug-in-sobel"
#define PLUG_IN_BINARY "edge-sobel"
#define PLUG_IN_ROLE   "picman-edge-sobel"


typedef struct
{
  gboolean horizontal;
  gboolean vertical;
  gboolean keep_sign;
} SobelValues;


/* Declare local functions.
 */
static void   query  (void);
static void   run    (const gchar      *name,
                      gint              nparams,
                      const PicmanParam  *param,
                      gint             *nreturn_vals,
                      PicmanParam       **return_vals);

static void   sobel  (PicmanDrawable     *drawable,
                      gboolean          horizontal,
                      gboolean          vertical,
                      gboolean          keep_sign,
                      PicmanPreview      *preview);

/*
 * Sobel interface
 */
static gboolean  sobel_dialog         (PicmanDrawable *drawable);
static void      sobel_preview_update (PicmanPreview  *preview);

/*
 * Sobel helper functions
 */
static void      sobel_prepare_row (PicmanPixelRgn *pixel_rgn,
                                    guchar       *data,
                                    gint          x,
                                    gint          y,
                                    gint          w);


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

static SobelValues bvals =
{
  TRUE,  /*  horizontal sobel  */
  TRUE,  /*  vertical sobel    */
  TRUE   /*  keep sign         */
};


MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",   "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"  },
    { PICMAN_PDB_IMAGE,    "image",      "Input image (unused)"          },
    { PICMAN_PDB_DRAWABLE, "drawable",   "Input drawable"                },
    { PICMAN_PDB_INT32,    "horizontal", "Sobel in horizontal direction" },
    { PICMAN_PDB_INT32,    "vertical",   "Sobel in vertical direction"   },
    { PICMAN_PDB_INT32,    "keep-sign",  "Keep sign of result (one direction only)" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Specialized direction-dependent edge detection"),
                          "This plugin calculates the gradient with a sobel "
                          "operator. The user can specify which direction to "
                          "use. When both directions are used, the result is "
                          "the RMS of the two gradients; if only one direction "
                          "is used, the result either the absolute value of the "
                          "gradient, or 127 + gradient (if the 'keep sign' "
                          "switch is on). This way, information about the "
                          "direction of the gradient is preserved. Resulting "
                          "images are not autoscaled.",
                          "Thorsten Schnier",
                          "Thorsten Schnier",
                          "1997",
                          N_("_Sobel..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[2];
  PicmanDrawable      *drawable;
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  picman_tile_cache_ntiles (2 * drawable->ntile_cols);

  switch (run_mode)
   {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &bvals);

      /*  First acquire information with a dialog  */
      if (! sobel_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          bvals.horizontal = (param[4].data.d_int32) ? TRUE : FALSE;
          bvals.vertical   = (param[5].data.d_int32) ? TRUE : FALSE;
          bvals.keep_sign  = (param[6].data.d_int32) ? TRUE : FALSE;
        }
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &bvals);
      break;

    default:
      break;
    }

  /*  Make sure that the drawable is gray or RGB color  */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      sobel (drawable,
             bvals.horizontal, bvals.vertical, bvals.keep_sign,
             NULL);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();


      /*  Store data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &bvals, sizeof (bvals));
    }
  else
    {
      status        = PICMAN_PDB_EXECUTION_ERROR;
      *nreturn_vals = 2;
      values[1].type          = PICMAN_PDB_STRING;
      values[1].data.d_string = _("Cannot operate on indexed color images.");
    }

  picman_drawable_detach (drawable);

  values[0].data.d_status = status;
}

static gboolean
sobel_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *toggle;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Sobel Edge Detection"), PLUG_IN_ROLE,
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

  preview = picman_drawable_preview_new (drawable, NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect (preview, "invalidated",
                    G_CALLBACK (sobel_preview_update),
                    NULL);

  toggle = gtk_check_button_new_with_mnemonic (_("Sobel _horizontally"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), bvals.horizontal);
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &bvals.horizontal);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_check_button_new_with_mnemonic (_("Sobel _vertically"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), bvals.vertical);
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &bvals.vertical);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_check_button_new_with_mnemonic (_("_Keep sign of result "
                                                 "(one direction only)"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), bvals.keep_sign);
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &bvals.keep_sign);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
sobel_preview_update (PicmanPreview *preview)
{
  sobel (picman_drawable_preview_get_drawable (PICMAN_DRAWABLE_PREVIEW (preview)),
         bvals.horizontal,
         bvals.vertical,
         bvals.keep_sign,
         preview);
}

static void
sobel_prepare_row (PicmanPixelRgn *pixel_rgn,
                   guchar       *data,
                   gint          x,
                   gint          y,
                   gint          w)
{
  gint b;

  y = CLAMP (y, 0, pixel_rgn->h - 1);
  picman_pixel_rgn_get_row (pixel_rgn, data, x, y, w);

  /*  Fill in edge pixels  */
  for (b = 0; b < pixel_rgn->bpp; b++)
    {
      data[-(int)pixel_rgn->bpp + b] = data[b];
      data[w * pixel_rgn->bpp + b] = data[(w - 1) * pixel_rgn->bpp + b];
    }
}

#define RMS(a, b) (sqrt ((a) * (a) + (b) * (b)))

static void
sobel (PicmanDrawable *drawable,
       gboolean      do_horizontal,
       gboolean      do_vertical,
       gboolean      keep_sign,
       PicmanPreview  *preview)
{
  PicmanPixelRgn  srcPR, destPR;
  gint          width, height;
  gint          bytes;
  gint          gradient, hor_gradient, ver_gradient;
  guchar       *dest, *d;
  guchar       *prev_row, *pr;
  guchar       *cur_row, *cr;
  guchar       *next_row, *nr;
  guchar       *tmp;
  gint          row, col;
  gint          x, y;
  gboolean      alpha;
  gint          counter;
  guchar       *preview_buffer = NULL;

  if (preview)
    {
      picman_preview_get_position (preview, &x, &y);
      picman_preview_get_size (preview, &width, &height);
    }
  else
    {
      if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                          &x, &y, &width, &height))
        return;

      picman_progress_init (_("Sobel edge detecting"));
    }

  /* Get the size of the input image. (This will/must be the same
   *  as the size of the output image.
   */
  bytes  = drawable->bpp;
  alpha  = picman_drawable_has_alpha (drawable->drawable_id);

  /*  allocate row buffers  */
  prev_row = g_new (guchar, (width + 2) * bytes);
  cur_row  = g_new (guchar, (width + 2) * bytes);
  next_row = g_new (guchar, (width + 2) * bytes);
  dest     = g_new (guchar, width * bytes);

  /*  initialize the pixel regions  */
  picman_pixel_rgn_init (&srcPR, drawable, 0, 0,
                       drawable->width, drawable->height,
                       FALSE, FALSE);

  if (preview)
    {
      preview_buffer = g_new (guchar, width * height * bytes);
    }
  else
    {
      picman_pixel_rgn_init (&destPR, drawable, 0, 0,
                           drawable->width, drawable->height,
                           TRUE, TRUE);
    }

  pr = prev_row + bytes;
  cr = cur_row  + bytes;
  nr = next_row + bytes;

  sobel_prepare_row (&srcPR, pr, x, y - 1, width);
  sobel_prepare_row (&srcPR, cr, x, y, width);
  counter =0;
  /*  loop through the rows, applying the sobel convolution  */
  for (row = y; row < y + height; row++)
    {
      /*  prepare the next row  */
      sobel_prepare_row (&srcPR, nr, x, row + 1, width);

      d = dest;
      for (col = 0; col < width * bytes; col++)
        {
          hor_gradient = (do_horizontal ?
                          ((pr[col - bytes] +  2 * pr[col] + pr[col + bytes]) -
                           (nr[col - bytes] + 2 * nr[col] + nr[col + bytes]))
                          : 0);
          ver_gradient = (do_vertical ?
                          ((pr[col - bytes] + 2 * cr[col - bytes] + nr[col - bytes]) -
                           (pr[col + bytes] + 2 * cr[col + bytes] + nr[col + bytes]))
                          : 0);
          gradient = (do_vertical && do_horizontal) ?
            (ROUND (RMS (hor_gradient, ver_gradient)) / 5.66) /* always >0 */
            : (keep_sign ? (127 + (ROUND ((hor_gradient + ver_gradient) / 8.0)))
               : (ROUND (abs (hor_gradient + ver_gradient) / 4.0)));

          if (alpha && (((col + 1) % bytes) == 0))
            { /* the alpha channel */
              *d++ = (counter == 0) ? 0 : 255;
              counter = 0;
            }
          else
            {
              *d++ = gradient;
              if (gradient > 10) counter ++;
            }
        }
      /*  shuffle the row pointers  */
      tmp = pr;
      pr = cr;
      cr = nr;
      nr = tmp;

      /*  store the dest  */
      if (preview)
        {
          memcpy (preview_buffer + width * (row - y) * bytes,
                  dest,
                  width * bytes);
        }
      else
        {
          picman_pixel_rgn_set_row (&destPR, dest, x, row, width);

          if ((row % 20) == 0)
            picman_progress_update ((double) row / (double) height);
        }
    }

  if (preview)
    {
      picman_preview_draw_buffer (preview, preview_buffer, width * bytes);
      g_free (preview_buffer);
    }
  else
    {
      picman_progress_update (1.0);
      /*  update the sobeled region  */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x, y, width, height);
    }

  g_free (prev_row);
  g_free (cur_row);
  g_free (next_row);
  g_free (dest);
}
