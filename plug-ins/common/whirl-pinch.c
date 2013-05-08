/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Whirl and Pinch plug-in --- two common distortions in one place
 * Copyright (C) 1997 Federico Mena Quintero
 * federico@nuclecu.unam.mx
 * Copyright (C) 1997 Scott Goehring
 * scott@poverty.bloomington.in.us
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


/* Version 2.09:
 *
 * - Another cool patch from Scott.  The radius is now in [0.0, 2.0],
 * with 1.0 being the default as usual.  In addition to a minimal
 * speed-up (one multiplication is eliminated in the calculation
 * code), it is easier to think of nice round values instead of
 * sqrt(2) :-)
 *
 * - Modified the way out-of-range pixels are handled.  This time the
 * plug-in handles `outside' pixels better; it paints them with the
 * current background color (for images without transparency) or with
 * a completely transparent background color (for images with
 * transparency).
 */


/* Version 2.08:
 *
 * This is the first version of this plug-in.  It is called 2.08
 * because it came out of merging the old Whirl 2.08 and Pinch 2.08
 * plug-ins.  */

#include "config.h"

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC    "plug-in-whirl-pinch"
#define PLUG_IN_BINARY  "whirl-pinch"
#define PLUG_IN_ROLE    "picman-whirl-pinch"
#define PLUG_IN_VERSION "May 1997, 2.09"

/***** Magic numbers *****/

#define SCALE_WIDTH  200

/***** Types *****/

typedef struct
{
  gdouble  whirl;
  gdouble  pinch;
  gdouble  radius;
} whirl_pinch_vals_t;

/***** Prototypes *****/

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const PicmanParam  *param,
                   gint             *nreturn_vals,
                   PicmanParam       **return_vals);

static void      whirl_pinch                (PicmanDrawable *drawable);
static int       calc_undistorted_coords    (double        wx,
                                             double        wy,
                                             double        whirl,
                                             double        pinch,
                                             double       *x,
                                             double       *y);

static gboolean  whirl_pinch_dialog         (PicmanDrawable *drawable);
static void      dialog_update_preview      (PicmanDrawable *drawable,
                                             PicmanPreview  *preview);


/***** Variables *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,   /* init_proc  */
  NULL,   /* quit_proc  */
  query,  /* query_proc */
  run     /* run_proc   */
};

static whirl_pinch_vals_t wpvals =
{
  90.0, /* whirl   */
  0.0,  /* pinch   */
  1.0   /* radius  */
};

static gint img_bpp, img_has_alpha;
static gint sel_x1, sel_y1, sel_x2, sel_y2;
static gint sel_width, sel_height;

static double cen_x, cen_y;
static double scale_x, scale_y;
static double radius, radius2;

/***** Functions *****/

MAIN()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image"                  },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable"               },
    { PICMAN_PDB_FLOAT,    "whirl",     "Whirl angle (degrees)"        },
    { PICMAN_PDB_FLOAT,    "pinch",     "Pinch amount"                 },
    { PICMAN_PDB_FLOAT,    "radius",    "Radius (1.0 is the largest circle that fits in the image, and 2.0 goes all the way to the corners)" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Distort an image by whirling and pinching"),
                          "Distorts the image by whirling and pinching, which "
                          "are two common center-based, circular distortions.  "
                          "Whirling is like projecting the image onto the "
                          "surface of water in a toilet and flushing.  "
                          "Pinching is similar to projecting the image onto "
                          "an elastic surface and pressing or pulling on the "
                          "center of the surface.",
                          "Federico Mena Quintero and Scott Goehring",
                          "Federico Mena Quintero and Scott Goehring",
                          PLUG_IN_VERSION,
                          N_("W_hirl and Pinch..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Distorts");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam values[1];

  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status;
  double             xhsiz, yhsiz;
  PicmanDrawable      *drawable;

  status   = PICMAN_PDB_SUCCESS;
  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /* Get the active drawable info */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  img_bpp       = picman_drawable_bpp (drawable->drawable_id);
  img_has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &sel_x1, &sel_y1,
                                      &sel_width, &sel_height))
    {
      g_message (_("Region affected by plug-in is empty"));
      return;
    }

      /* Set the tile cache size */
  picman_tile_cache_ntiles (2 * drawable->ntile_cols);

  /* Calculate scaling parameters */

  sel_x2 = sel_x1 + sel_width;
  sel_y2 = sel_y1 + sel_height;

  cen_x = (double) (sel_x1 + sel_x2 - 1) / 2.0;
  cen_y = (double) (sel_y1 + sel_y2 - 1) / 2.0;

  xhsiz = (double) (sel_width - 1) / 2.0;
  yhsiz = (double) (sel_height - 1) / 2.0;

  if (xhsiz < yhsiz)
    {
      scale_x = yhsiz / xhsiz;
      scale_y = 1.0;
    }
  else if (xhsiz > yhsiz)
    {
      scale_x = 1.0;
      scale_y = xhsiz / yhsiz;
    }
  else
    {
      scale_x = 1.0;
      scale_y = 1.0;
    }

  radius = MAX(xhsiz, yhsiz);

  /* See how we will run */

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &wpvals);

      /* Get information from the dialog */
      if (!whirl_pinch_dialog (drawable))
        return;

      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /* Make sure all the arguments are present */
      if (nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }
      else
        {
          wpvals.whirl  = param[3].data.d_float;
          wpvals.pinch  = param[4].data.d_float;
          wpvals.radius = param[5].data.d_float;
        }

      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &wpvals);
      break;

    default:
      break;
    }

  /* Distort the image */
  if ((status == PICMAN_PDB_SUCCESS) &&
      (picman_drawable_is_rgb (drawable->drawable_id) ||
       picman_drawable_is_gray (drawable->drawable_id)))
    {

      /* Run! */
      whirl_pinch (drawable);

      /* If run mode is interactive, flush displays */
      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /* Store data */

      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &wpvals, sizeof (whirl_pinch_vals_t));
    }
  else if (status == PICMAN_PDB_SUCCESS)
    status = PICMAN_PDB_EXECUTION_ERROR;

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static void
whirl_pinch (PicmanDrawable *drawable)
{
  PicmanPixelRgn      dest_rgn;
  gint              progress, max_progress;
  guchar           *top_row, *bot_row;
  guchar           *top_p, *bot_p;
  gint              row, col;
  guchar          **pixel;
  gdouble           whirl;
  gdouble           cx, cy;
  gint              ix, iy;
  gint              i;
  PicmanPixelFetcher *pft, *pfb;
  PicmanRGB           background;

  /* Initialize rows */
  top_row = g_new (guchar, img_bpp * sel_width);
  bot_row = g_new (guchar, img_bpp * sel_width);
  pixel = g_new (guchar *, 4);
  for (i = 0; i < 4; i++)
    pixel[i] = g_new (guchar, 4);

  /* Initialize pixel region */
  picman_pixel_rgn_init (&dest_rgn, drawable,
                       sel_x1, sel_y1, sel_width, sel_height, TRUE, TRUE);

  pft = picman_pixel_fetcher_new (drawable, FALSE);
  pfb = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_pixel_fetcher_set_bg_color (pft, &background);
  picman_pixel_fetcher_set_bg_color (pfb, &background);

  if (picman_drawable_has_alpha (drawable->drawable_id))
    {
      picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_BLACK);
      picman_pixel_fetcher_set_edge_mode (pfb, PICMAN_PIXEL_FETCHER_EDGE_BLACK);
    }
  else
    {
      picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND);
      picman_pixel_fetcher_set_edge_mode (pfb, PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND);
    }

  progress     = 0;
  max_progress = sel_width * sel_height;

  picman_progress_init (_("Whirling and pinching"));

  whirl   = wpvals.whirl * G_PI / 180;
  radius2 = radius * radius * wpvals.radius;

  for (row = sel_y1; row <= ((sel_y1 + sel_y2) / 2); row++)
    {
      top_p = top_row;
      bot_p = bot_row + img_bpp * (sel_width - 1);

      for (col = sel_x1; col < sel_x2; col++)
        {
          if (calc_undistorted_coords (col, row, whirl, wpvals.pinch, &cx, &cy))
            {
              /* We are inside the distortion area */

              /* Top */

              if (cx >= 0.0)
                ix = (int) cx;
              else
                ix = -((int) -cx + 1);

              if (cy >= 0.0)
                iy = (int) cy;
              else
                iy = -((int) -cy + 1);

              picman_pixel_fetcher_get_pixel (pft, ix,     iy,     pixel[0]);
              picman_pixel_fetcher_get_pixel (pft, ix + 1, iy,     pixel[1]);
              picman_pixel_fetcher_get_pixel (pft, ix,     iy + 1, pixel[2]);
              picman_pixel_fetcher_get_pixel (pft, ix + 1, iy + 1, pixel[3]);

              picman_bilinear_pixels_8 (top_p, cx, cy, img_bpp, img_has_alpha,
                                      pixel);
              top_p += img_bpp;
              /* Bottom */

              cx = cen_x + (cen_x - cx);
              cy = cen_y + (cen_y - cy);

              if (cx >= 0.0)
                ix = (int) cx;
              else
                ix = -((int) -cx + 1);

              if (cy >= 0.0)
                iy = (int) cy;
              else
                iy = -((int) -cy + 1);

              picman_pixel_fetcher_get_pixel (pfb, ix,     iy,     pixel[0]);
              picman_pixel_fetcher_get_pixel (pfb, ix + 1, iy,     pixel[1]);
              picman_pixel_fetcher_get_pixel (pfb, ix,     iy + 1, pixel[2]);
              picman_pixel_fetcher_get_pixel (pfb, ix + 1, iy + 1, pixel[3]);

              picman_bilinear_pixels_8 (bot_p, cx, cy, img_bpp, img_has_alpha,
                                      pixel);
              bot_p -= img_bpp;
            }
          else
            {
              /*  We are outside the distortion area;
               *  just copy the source pixels
               */

              /* Top */

              picman_pixel_fetcher_get_pixel (pft, col, row, pixel[0]);

              for (i = 0; i < img_bpp; i++)
                *top_p++ = pixel[0][i];

              /* Bottom */

              picman_pixel_fetcher_get_pixel (pfb,
                                       (sel_x2 - 1) - (col - sel_x1),
                                       (sel_y2 - 1) - (row - sel_y1),
                                       pixel[0]);

              for (i = 0; i < img_bpp; i++)
                *bot_p++ = pixel[0][i];

              bot_p -= 2 * img_bpp; /* We move backwards! */
            }
        }

      /* Paint rows to image */

      picman_pixel_rgn_set_row (&dest_rgn, top_row, sel_x1, row, sel_width);
      picman_pixel_rgn_set_row (&dest_rgn, bot_row,
                              sel_x1, (sel_y2 - 1) - (row - sel_y1), sel_width);

      /* Update progress */

      progress += sel_width * 2;
      picman_progress_update ((double) progress / max_progress);
    }

  picman_progress_update (1.0);
  picman_pixel_fetcher_destroy (pft);
  picman_pixel_fetcher_destroy (pfb);

  for (i = 0; i < 4; i++)
    g_free (pixel[i]);
  g_free (pixel);
  g_free (top_row);
  g_free (bot_row);

  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id,
                        sel_x1, sel_y1, sel_width, sel_height);
}

static gint
calc_undistorted_coords (gdouble  wx,
                         gdouble  wy,
                         gdouble  whirl,
                         gdouble  pinch,
                         gdouble *x,
                         gdouble *y)
{
  gdouble dx, dy;
  gdouble d, factor;
  gdouble dist;
  gdouble ang, sina, cosa;
  gint    inside;

  /* Distances to center, scaled */

  dx = (wx - cen_x) * scale_x;
  dy = (wy - cen_y) * scale_y;

  /* Distance^2 to center of *circle* (scaled ellipse) */

  d = dx * dx + dy * dy;

  /*  If we are inside circle, then distort.
   *  Else, just return the same position
   */

  inside = (d < radius2);

  if (inside)
    {
      dist = sqrt(d / wpvals.radius) / radius;

      /* Pinch */

      factor = pow (sin (G_PI_2 * dist), -pinch);

      dx *= factor;
      dy *= factor;

      /* Whirl */

      factor = 1.0 - dist;

      ang = whirl * factor * factor;

      sina = sin (ang);
      cosa = cos (ang);

      *x = (cosa * dx - sina * dy) / scale_x + cen_x;
      *y = (sina * dx + cosa * dy) / scale_y + cen_y;
    }
  else
    {
      *x = wx;
      *y = wy;
    }

  return inside;
}

static gboolean
whirl_pinch_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *table;
  GtkObject *adj;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, TRUE);

  dialog = picman_dialog_new (_("Whirl and Pinch"), PLUG_IN_ROLE,
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

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (dialog_update_preview),
                            drawable);

  /* Controls */
  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                              _("_Whirl angle:"), SCALE_WIDTH, 7,
                              wpvals.whirl, -720.0, 720.0, 1.0, 15.0, 2,
                              FALSE, -3600.0, 3600.0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &wpvals.whirl);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                              _("_Pinch amount:"), SCALE_WIDTH, 7,
                              wpvals.pinch, -1.0, 1.0, 0.01, 0.1, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &wpvals.pinch);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, 2,
                              _("_Radius:"), SCALE_WIDTH, 7,
                              wpvals.radius, 0.0, 2.0, 0.01, 0.1, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &wpvals.radius);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  /* Done */

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
dialog_update_preview (PicmanDrawable *drawable,
                       PicmanPreview  *preview)
{
  gdouble               cx, cy;
  gint                  x, y;
  gint                  sx, sy;
  gint                  width, height;
  guchar               *pixel;
  PicmanRGB               background;
  guchar               *dest;
  gint                  j;
  gint                  bpp;
  PicmanPixelFetcher     *pft;
  guchar                in_pixels[4][4];
  guchar               *in_values[4];
  gdouble               whirl;

  whirl   = wpvals.whirl * G_PI / 180.0;
  radius2 = radius * radius * wpvals.radius;

  for (j = 0; j < 4; j++)
    in_values[j] = in_pixels[j];

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_rgb_set_alpha (&background, 0.0);
  picman_pixel_fetcher_set_bg_color (pft, &background);
  picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_SMEAR);

  dest = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                       &width, &height, &bpp);

  pixel = dest;

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          picman_preview_untransform (preview, x, y, &sx, &sy);
          calc_undistorted_coords ((gdouble)sx, (gdouble)sy,
                                   whirl, wpvals.pinch,
                                   &cx, &cy);

          picman_pixel_fetcher_get_pixel (pft, cx, cy, in_pixels[0]);
          picman_pixel_fetcher_get_pixel (pft, cx + 1, cy, in_pixels[1]);
          picman_pixel_fetcher_get_pixel (pft, cx, cy + 1, in_pixels[2]);
          picman_pixel_fetcher_get_pixel (pft, cx + 1, cy + 1, in_pixels[3]);

          picman_bilinear_pixels_8 (pixel, cx, cy, bpp,
                                  img_has_alpha, in_values);

          pixel += bpp;
        }
    }

  picman_pixel_fetcher_destroy (pft);

  picman_preview_draw_buffer (preview, dest, width * bpp);
  g_free (dest);
}

