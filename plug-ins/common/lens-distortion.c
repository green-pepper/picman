/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Lens plug-in - adjust for lens distortion
 * Copyright (C) 2001-2005 David Hodson hodsond@acm.org
 * Many thanks for Lars Clausen for the original inspiration,
 *   useful discussion, optimisation and improvements.
 * Framework borrowed from many similar plugins...
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

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC     "plug-in-lens-distortion"
#define PLUG_IN_BINARY   "lens-distortion"
#define PLUG_IN_ROLE     "picman-lens-distortion"

#define RESPONSE_RESET   1

#define LENS_MAX_PIXEL_DEPTH        4


typedef struct
{
  gdouble  centre_x;
  gdouble  centre_y;
  gdouble  square_a;
  gdouble  quad_a;
  gdouble  scale_a;
  gdouble  brighten;
} LensValues;

typedef struct
{
  gdouble  normallise_radius_sq;
  gdouble  centre_x;
  gdouble  centre_y;
  gdouble  mult_sq;
  gdouble  mult_qd;
  gdouble  rescale;
  gdouble  brighten;
} LensCalcValues;


/* Declare local functions. */

static void     query (void);
static void     run   (const gchar      *name,
                       gint              nparams,
                       const PicmanParam  *param,
                       gint             *nreturn_vals,
                       PicmanParam       **return_vals);

static void     lens_distort    (PicmanDrawable *drawable);
static gboolean lens_dialog     (PicmanDrawable *drawable);


static LensValues         vals = { 0.0, 0.0, 0.0, 0.0 };
static LensCalcValues     calc_vals;

static gint               drawable_width, drawable_height;
static guchar             background_color[4];


const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


MAIN ()

static void
query (void)
{
  static PicmanParamDef args[] =
    {
      { PICMAN_PDB_INT32,    "run-mode",    "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
      { PICMAN_PDB_IMAGE,    "image",       "Input image (unused)" },
      { PICMAN_PDB_DRAWABLE, "drawable",    "Input drawable" },
      { PICMAN_PDB_FLOAT,    "offset-x",    "Effect centre offset in X" },
      { PICMAN_PDB_FLOAT,    "offset-y",    "Effect centre offset in Y" },
      { PICMAN_PDB_FLOAT,    "main-adjust", "Amount of second-order distortion" },
      { PICMAN_PDB_FLOAT,    "edge-adjust", "Amount of fourth-order distortion" },
      { PICMAN_PDB_FLOAT,    "rescale",     "Rescale overall image size" },
      { PICMAN_PDB_FLOAT,    "brighten",    "Adjust brightness in corners" }
    };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Corrects lens distortion"),
                          "Corrects barrel or pincushion lens distortion.",
                          "David Hodson, Aurimas Juška",
                          "David Hodson",
                          "Version 1.0.10",
                          N_("Lens Distortion..."),
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
  static PicmanParam   values[1];
  PicmanDrawable      *drawable;
  PicmanRGB            background;
  PicmanPDBStatusType  status   = PICMAN_PDB_SUCCESS;
  PicmanRunMode        run_mode;

  run_mode = param[0].data.d_int32;

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  INIT_I18N ();

  drawable = picman_drawable_get (param[2].data.d_drawable);

  drawable_width = drawable->width;
  drawable_height = drawable->height;

  /* Get background color */
  picman_context_get_background (&background);
  picman_rgb_set_alpha (&background, 0.0);
  picman_drawable_get_color_uchar (drawable->drawable_id, &background,
                                 background_color);

  /* Set the tile cache size */
  picman_tile_cache_ntiles (2 * MAX (drawable->ntile_rows, drawable->ntile_cols));

  *nreturn_vals = 1;
  *return_vals = values;

  switch (run_mode) {
  case PICMAN_RUN_INTERACTIVE:
    picman_get_data (PLUG_IN_PROC, &vals);
    if (! lens_dialog (drawable))
      return;
    break;

  case PICMAN_RUN_NONINTERACTIVE:
    if (nparams != 9)
      status = PICMAN_PDB_CALLING_ERROR;

    if (status == PICMAN_PDB_SUCCESS)
      {
        vals.centre_x = param[3].data.d_float;
        vals.centre_y = param[4].data.d_float;
        vals.square_a = param[5].data.d_float;
        vals.quad_a = param[6].data.d_float;
        vals.scale_a = param[7].data.d_float;
        vals.brighten = param[8].data.d_float;
      }

    break;

  case PICMAN_RUN_WITH_LAST_VALS:
    picman_get_data (PLUG_IN_PROC, &vals);
    break;

  default:
    break;
  }

  if ( status == PICMAN_PDB_SUCCESS )
    {
      lens_distort (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &vals, sizeof (LensValues));

      picman_drawable_detach (drawable);
    }

  values[0].data.d_status = status;
}

static void
lens_get_source_coords (gdouble  i,
                        gdouble  j,
                        gdouble *x,
                        gdouble *y,
                        gdouble *mag)
{
  gdouble radius_sq;

  gdouble off_x;
  gdouble off_y;

  gdouble radius_mult;

  off_x = i - calc_vals.centre_x;
  off_y = j - calc_vals.centre_y;
  radius_sq = (off_x * off_x) + (off_y * off_y);

  radius_sq *= calc_vals.normallise_radius_sq;

  radius_mult = radius_sq * calc_vals.mult_sq + radius_sq * radius_sq *
    calc_vals.mult_qd;
  *mag = radius_mult;
  radius_mult = calc_vals.rescale * (1.0 + radius_mult);

  *x = calc_vals.centre_x + radius_mult * off_x;
  *y = calc_vals.centre_y + radius_mult * off_y;
}

static void
lens_setup_calc (gint width, gint height)
{
  calc_vals.normallise_radius_sq =
    4.0 / (width * width + height * height);

  calc_vals.centre_x = width * (100.0 + vals.centre_x) / 200.0;
  calc_vals.centre_y = height * (100.0 + vals.centre_y) / 200.0;
  calc_vals.mult_sq = vals.square_a / 200.0;
  calc_vals.mult_qd = vals.quad_a / 200.0;
  calc_vals.rescale = pow(2.0, - vals.scale_a / 100.0);
  calc_vals.brighten = - vals.brighten / 10.0;
}

/*
 * Catmull-Rom cubic interpolation
 *
 * equally spaced points p0, p1, p2, p3
 * interpolate 0 <= u < 1 between p1 and p2
 *
 * (1 u u^2 u^3) (  0.0  1.0  0.0  0.0 ) (p0)
 *               ( -0.5  0.0  0.5  0.0 ) (p1)
 *               (  1.0 -2.5  2.0 -0.5 ) (p2)
 *               ( -0.5  1.5 -1.5  0.5 ) (p3)
 *
 */

static void
lens_cubic_interpolate (const guchar *src,
                        gint          row_stride,
                        gint          src_depth,
                        guchar       *dst,
                        gint          dst_depth,
                        gdouble       dx,
                        gdouble       dy,
                        gdouble       brighten)
{
  gfloat um1, u, up1, up2;
  gfloat vm1, v, vp1, vp2;
  gint   c;
  gfloat verts[4 * LENS_MAX_PIXEL_DEPTH];

  um1 = ((-0.5 * dx + 1.0) * dx - 0.5) * dx;
  u = (1.5 * dx - 2.5) * dx * dx + 1.0;
  up1 = ((-1.5 * dx + 2.0) * dx + 0.5) * dx;
  up2 = (0.5 * dx - 0.5) * dx * dx;

  vm1 = ((-0.5 * dy + 1.0) * dy - 0.5) * dy;
  v = (1.5 * dy - 2.5) * dy * dy + 1.0;
  vp1 = ((-1.5 * dy + 2.0) * dy + 0.5) * dy;
  vp2 = (0.5 * dy - 0.5) * dy * dy;

  /* Note: if dst_depth < src_depth, we calculate unneeded pixels here */
  /* later - select or create index array */
  for (c = 0; c < 4 * src_depth; ++c)
    {
      verts[c] = vm1 * src[c] + v * src[c+row_stride] +
        vp1 * src[c+row_stride*2] + vp2 * src[c+row_stride*3];
    }

  for (c = 0; c < dst_depth; ++c)
    {
      gfloat result;

      result = um1 * verts[c] + u * verts[c+src_depth] +
        up1 * verts[c+src_depth*2] + up2 * verts[c+src_depth*3];

      result *= brighten;

      dst[c] = CLAMP (result, 0, 255);
    }
}

static void
lens_distort_func (gint              ix,
                   gint              iy,
                   guchar           *dest,
                   gint              bpp,
                   PicmanPixelFetcher *pft)
{
  gdouble  src_x, src_y, mag;
  gdouble  brighten;
  guchar   pixel_buffer[16 * LENS_MAX_PIXEL_DEPTH];
  guchar  *pixel;
  gdouble  dx, dy;
  gint     x_int, y_int;
  gint     x, y;

  lens_get_source_coords (ix, iy, &src_x, &src_y, &mag);

  brighten = 1.0 + mag * calc_vals.brighten;
  x_int = floor (src_x);
  dx = src_x - x_int;

  y_int = floor (src_y);
  dy = src_y - y_int;

  pixel = pixel_buffer;
  for (y = y_int - 1; y <= y_int + 2; y++)
    {
      for (x = x_int -1; x <= x_int + 2; x++)
        {
          if (x >= 0  && y >= 0 &&
              x < drawable_width &&  y < drawable_height)
            {
              picman_pixel_fetcher_get_pixel (pft, x, y, pixel);
            }
          else
            {
              gint i;

              for (i = 0; i < bpp; i++)
                pixel[i] = background_color[i];
            }

          pixel += bpp;
        }
    }

  lens_cubic_interpolate (pixel_buffer, bpp * 4, bpp,
                          dest, bpp, dx, dy, brighten);
}

static void
lens_distort (PicmanDrawable *drawable)
{
  PicmanRgnIterator  *iter;
  PicmanPixelFetcher *pft;
  PicmanRGB           background;

  lens_setup_calc (drawable->width, drawable->height);

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_rgb_set_alpha (&background, 0.0);
  picman_pixel_fetcher_set_bg_color (pft, &background);
  picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND);

  picman_progress_init (_("Lens distortion"));

  iter = picman_rgn_iterator_new (drawable, 0);
  picman_rgn_iterator_dest (iter, (PicmanRgnFuncDest) lens_distort_func, pft);
  picman_rgn_iterator_free (iter);

  picman_pixel_fetcher_destroy (pft);
}

static void
lens_distort_preview (PicmanDrawable *drawable,
                      PicmanPreview  *preview)
{
  guchar               *dest;
  guchar               *pixel;
  gint                  width, height, bpp;
  gint                  x, y;
  PicmanPixelFetcher     *pft;
  PicmanRGB               background;

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_rgb_set_alpha (&background, 0.0);
  picman_pixel_fetcher_set_bg_color (pft, &background);
  picman_pixel_fetcher_set_edge_mode (pft, PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND);

  lens_setup_calc (drawable->width, drawable->height);

  dest = picman_zoom_preview_get_source (PICMAN_ZOOM_PREVIEW (preview),
                                       &width, &height, &bpp);
  pixel = dest;

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          gint sx, sy;

          picman_preview_untransform (preview, x, y, &sx, &sy);

          lens_distort_func (sx, sy, pixel, bpp, pft);

          pixel += bpp;
        }
    }

  picman_pixel_fetcher_destroy (pft);

  picman_preview_draw_buffer (preview, dest, width * bpp);
  g_free (dest);
}

/* UI callback functions */

static GSList *adjustments = NULL;

static void
lens_dialog_reset (void)
{
  GSList *list;

  for (list = adjustments; list; list = list->next)
    gtk_adjustment_set_value (GTK_ADJUSTMENT (list->data), 0.0);
}

static void
lens_response (GtkWidget *widget,
               gint       response_id,
               gboolean  *run)
{
  switch (response_id)
    {
    case RESPONSE_RESET:
      lens_dialog_reset ();
      break;

    case GTK_RESPONSE_OK:
      *run = TRUE;
      /* fallthrough */

    default:
      gtk_widget_destroy (GTK_WIDGET (widget));
      break;
    }
}

static gboolean
lens_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *table;
  GtkWidget *preview;
  GtkObject *adj;
  gint       row = 0;
  gboolean   run = FALSE;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Lens Distortion"), PLUG_IN_ROLE,
                            NULL, 0,
                            picman_standard_help_func, PLUG_IN_PROC,

                            PICMAN_STOCK_RESET, RESPONSE_RESET,
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,

                            NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
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
                            G_CALLBACK (lens_distort_preview),
                            drawable);

  table = gtk_table_new (6, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_Main:"), 120, 6,
                              vals.square_a, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.square_a);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_Edge:"), 120, 6,
                              vals.quad_a, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.quad_a);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_Zoom:"), 120, 6,
                              vals.scale_a, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.scale_a);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_Brighten:"), 120, 6,
                              vals.brighten, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.brighten);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_X shift:"), 120, 6,
                              vals.centre_x, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.centre_x);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  adj = picman_scale_entry_new (GTK_TABLE (table), 0, row++,
                              _("_Y shift:"), 120, 6,
                              vals.centre_y, -100.0, 100.0, 0.1, 1.0, 3,
                              TRUE, 0, 0,
                              NULL, NULL);
  adjustments = g_slist_append (adjustments, adj);

  g_signal_connect (adj, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &vals.centre_y);
  g_signal_connect_swapped (adj, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (lens_response),
                    &run);
  g_signal_connect (dialog, "destroy",
                    G_CALLBACK (gtk_main_quit),
                    NULL);

  gtk_widget_show (dialog);

  gtk_main ();

  g_slist_free (adjustments);
  adjustments = NULL;

  return run;
}
