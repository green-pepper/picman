/* Cubism --- image filter plug-in for PICMAN
 * Copyright (C) 1996 Spencer Kimball, Tracy Scott
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
 * You can contact me at quartic@polloux.fciencias.unam.mx
 * You can contact the original PICMAN authors at picman@xcf.berkeley.edu
 * Speedups by Elliot Lee
 */

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>
#include <libpicman/picmanui.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC    "plug-in-cubism"
#define PLUG_IN_BINARY  "cubism"
#define PLUG_IN_ROLE    "picman-cubism"

#define SCALE_WIDTH     125
#define BLACK             0
#define BG                1
#define SUPERSAMPLE       4
#define MAX_POINTS        4
#define RANDOMNESS        5


typedef struct
{
  gint        npts;
  PicmanVector2 pts[MAX_POINTS];
} Polygon;

typedef struct
{
  gdouble  tile_size;
  gdouble  tile_saturation;
  gint     bg_color;
  gboolean preview;
} CubismVals;

/* Declare local functions.
 */
static void      query                (void);
static void      run                  (const gchar      *name,
                                       gint              nparams,
                                       const PicmanParam  *param,
                                       gint             *nreturn_vals,
                                       PicmanParam       **return_vals);

static void      cubism               (PicmanDrawable     *drawable,
                                       PicmanPreview      *preview);
static gboolean  cubism_dialog        (PicmanDrawable     *drawable);

static void      fill_poly_color      (Polygon          *poly,
                                       PicmanDrawable     *drawable,
                                       PicmanPreview      *preview,
                                       guchar           *col,
                                       guchar           *dest);

static void      convert_segment      (gint              x1,
                                       gint              y1,
                                       gint              x2,
                                       gint              y2,
                                       gint              offset,
                                       gint             *min,
                                       gint             *max);

static void      randomize_indices    (gint              count,
                                       gint             *indices);

static gdouble   calc_alpha_blend     (gdouble          *vec,
                                       gdouble           one_over_dist,
                                       gdouble           x,
                                       gdouble           y);

static void      polygon_add_point    (Polygon          *poly,
                                       gdouble           x,
                                       gdouble           y);
static void      polygon_translate    (Polygon          *poly,
                                       gdouble           tx,
                                       gdouble           ty);
static void      polygon_rotate       (Polygon          *poly,
                                       gdouble           theta);
static gint      polygon_extents      (Polygon          *poly,
                                       gdouble          *min_x,
                                       gdouble          *min_y,
                                       gdouble          *max_x,
                                       gdouble          *max_y);
static void      polygon_reset        (Polygon          *poly);


/*
 *  Local variables
 */

static CubismVals cvals =
{
  10.0,        /* tile_size       */
  2.5,         /* tile_saturation */
  BLACK,       /* bg_color        */
  TRUE         /* preview         */
};

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};


/*
 *  Functions
 */

MAIN ()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",        "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",           "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",        "Input drawable" },
    { PICMAN_PDB_FLOAT,    "tile-size",       "Average diameter of each tile (in pixels)" },
    { PICMAN_PDB_FLOAT,    "tile-saturation", "Expand tiles by this amount" },
    { PICMAN_PDB_INT32,    "bg-color",        "Background color { BLACK (0), BG (1) }" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Convert the image into randomly rotated square blobs"),
                          "Help not yet written for this plug-in",
                          "Spencer Kimball & Tracy Scott",
                          "Spencer Kimball & Tracy Scott",
                          "1996",
                          N_("_Cubism..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Artistic");
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
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status = PICMAN_PDB_SUCCESS;

  INIT_I18N ();

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /*  get the active drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  /*  set cache size  */
  /* asking for a lot here but seems to give a speedup -- WES 12-23-04 */
  picman_tile_cache_ntiles (2 * drawable->ntile_rows * drawable->ntile_cols);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &cvals);

      /*  First acquire information with a dialog  */
      if (! cubism_dialog (drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      /*  Make sure all the arguments are there!  */
      if (nparams != 6)
        status = PICMAN_PDB_CALLING_ERROR;
      if (status == PICMAN_PDB_SUCCESS)
        {
          cvals.tile_size       = param[3].data.d_float;
          cvals.tile_saturation = param[4].data.d_float;
          cvals.bg_color        = param[5].data.d_int32;
        }
      if (status == PICMAN_PDB_SUCCESS &&
          (cvals.bg_color < BLACK || cvals.bg_color > BG))
        status = PICMAN_PDB_CALLING_ERROR;
      break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /*  Possibly retrieve data  */
      picman_get_data (PLUG_IN_PROC, &cvals);
      break;

    default:
      break;
    }

  /*  Render the cubism effect  */
  if ((status == PICMAN_PDB_SUCCESS) &&
      (picman_drawable_is_rgb (drawable->drawable_id) ||
       picman_drawable_is_gray (drawable->drawable_id)))
    {

      cubism (drawable, NULL);

      /*  If the run mode is interactive, flush the displays  */
      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /*  Store mvals data  */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &cvals, sizeof (CubismVals));
    }
  else if (status == PICMAN_PDB_SUCCESS)
    {
      /* picman_message ("cubism: cannot operate on indexed color images"); */
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static gboolean
cubism_dialog (PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *preview;
  GtkWidget *toggle;
  GtkWidget *table;
  GtkObject *scale_data;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Cubism"), PLUG_IN_ROLE,
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

  preview = picman_drawable_preview_new (drawable, &cvals.preview);
  gtk_box_pack_start (GTK_BOX (main_vbox), preview, TRUE, TRUE, 0);
  gtk_widget_show (preview);

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (cubism),
                            drawable);

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacing (GTK_TABLE (table), 0, 4);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  scale_data = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                                     _("_Tile size:"), SCALE_WIDTH, 5,
                                     cvals.tile_size, 0.0, 100.0, 1.0, 10.0, 1,
                                     TRUE, 0, 0,
                                     NULL, NULL);
  g_signal_connect (scale_data, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &cvals.tile_size);
  g_signal_connect_swapped (scale_data, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  scale_data =
    picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                          _("T_ile saturation:"), SCALE_WIDTH, 5,
                          cvals.tile_saturation, 0.0, 10.0, 0.1, 1, 1,
                          TRUE, 0, 0,
                          NULL, NULL);
  g_signal_connect (scale_data, "value-changed",
                    G_CALLBACK (picman_double_adjustment_update),
                    &cvals.tile_saturation);
  g_signal_connect_swapped (scale_data, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  toggle = gtk_check_button_new_with_mnemonic (_("_Use background color"));
  gtk_box_pack_start (GTK_BOX (main_vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &cvals.bg_color);
  g_signal_connect_swapped (toggle, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                (cvals.bg_color == BG));

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

static void
cubism (PicmanDrawable *drawable,
        PicmanPreview  *preview)
{
  PicmanPixelRgn src_rgn;
  guchar       bg_col[4];
  gdouble      x, y;
  gdouble      width, height;
  gdouble      theta;
  gint         ix, iy;
  gint         rows, cols;
  gint         i, j, count;
  gint         num_tiles;
  gint         x1, y1, x2, y2;
  gint         sel_width, sel_height;
  Polygon      poly;
  guchar       col[4];
  guchar      *dest = NULL;
  gint         bytes;
  gboolean     has_alpha;
  gint        *random_indices;
  gpointer     pr;
  GRand       *gr;

  gr = g_rand_new ();
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  bytes = drawable->bpp;

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &sel_width, &sel_height);
      dest = g_new (guchar, sel_height * sel_width * bytes);
    }
  else if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                           &x1, &y1, &sel_width, &sel_height))
    {
      return;
    }

  x2 = x1 + sel_width;
  y2 = y1 + sel_height;

  /*  determine the background color  */
  if (cvals.bg_color == BLACK)
    {
      bg_col[0] = bg_col[1] = bg_col[2] = bg_col[3] = 0;
    }
  else
    {
      PicmanRGB color;

      picman_context_get_background (&color);
      picman_rgb_set_alpha (&color, 0.0);
      picman_drawable_get_color_uchar (drawable->drawable_id, &color, bg_col);
    }

  cols = ((x2 - x1) + cvals.tile_size - 1) / cvals.tile_size;
  rows = ((y2 - y1) + cvals.tile_size - 1) / cvals.tile_size;

  /*  Fill the image with the background color  */
  if (preview)
    {
      for (i = 0; i < sel_width * sel_height ; i++)
        for (count = 0; count < bytes; count++)
          dest[i * bytes + count] = bg_col[count];
    }
  else
    {
      picman_progress_init (_("Cubistic transformation"));
      picman_pixel_rgn_init (&src_rgn, drawable,
                           x1, y1, (x2 - x1), (y2 - y1), TRUE, TRUE);

      for (pr = picman_pixel_rgns_register (1, &src_rgn);
           pr != NULL;
           pr = picman_pixel_rgns_process (pr))
        {
          count = src_rgn.w * src_rgn.h;
          dest  = src_rgn.data;

          while (count--)
            for (i = 0; i < bytes; i++)
              *dest++ = bg_col[i];
        }

      dest = NULL;
    }

  num_tiles = (rows + 1) * (cols + 1);
  random_indices = g_new (gint, num_tiles);
  for (i = 0; i < num_tiles; i++)
    random_indices[i] = i;

  randomize_indices (num_tiles, random_indices);

  picman_pixel_rgn_init (&src_rgn, drawable,
                       x1, y1, x2 - x1, y2 - y1, FALSE, FALSE);

  for (count = 0; count < num_tiles; count++)
    {
      i = random_indices[count] / (cols + 1);
      j = random_indices[count] % (cols + 1);
      x = j * cvals.tile_size + (cvals.tile_size / 4.0)
        - g_rand_double_range (gr, 0, cvals.tile_size/2.0) + x1;
      y = i * cvals.tile_size + (cvals.tile_size / 4.0)
        - g_rand_double_range (gr, 0, cvals.tile_size/2.0) + y1;
      width = (cvals.tile_size +
               g_rand_double_range (gr, 0, cvals.tile_size / 4.0) -
               cvals.tile_size / 8.0) * cvals.tile_saturation;
      height = (cvals.tile_size +
                g_rand_double_range (gr, 0, cvals.tile_size / 4.0) -
                cvals.tile_size / 8.0) * cvals.tile_saturation;
      theta = g_rand_double_range (gr, 0, 2 * G_PI);
      polygon_reset (&poly);
      polygon_add_point (&poly, -width / 2.0, -height / 2.0);
      polygon_add_point (&poly, width / 2.0, -height / 2.0);
      polygon_add_point (&poly, width / 2.0, height / 2.0);
      polygon_add_point (&poly, -width / 2.0, height / 2.0);
      polygon_rotate (&poly, theta);
      polygon_translate (&poly, x, y);

      /*  bounds check on x, y  */
      ix = CLAMP (x, x1, x2 - 1);
      iy = CLAMP (y, y1, y2 - 1);

      picman_pixel_rgn_get_pixel (&src_rgn, col, ix, iy);

      if (! has_alpha || col[bytes - 1])
        fill_poly_color (&poly, drawable, preview, col, dest);

      if (! preview)
        {
          if (count % 8 == 0)
            picman_progress_update ((gdouble) count / (gdouble) num_tiles);
        }
    }

  g_free (random_indices);
  g_rand_free (gr);

  if (preview)
    {
      picman_preview_draw_buffer (preview, dest, sel_width * bytes);
      g_free (dest);
    }
  else
    {
      picman_progress_update (1.0);
      /*  merge the shadow, update the drawable  */
      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
    }
}

static inline gdouble
calc_alpha_blend (gdouble *vec,
                  gdouble  one_over_dist,
                  gdouble  x,
                  gdouble  y)
{
  gdouble r;

  if (! one_over_dist)
    return 1.0;

  r = (vec[0] * x + vec[1] * y) * one_over_dist;

  return CLAMP (r, 0.2, 1.0);
}

static void
fill_poly_color (Polygon      *poly,
                 PicmanDrawable *drawable,
                 PicmanPreview  *preview,
                 guchar       *col,
                 guchar       *dest)
{
  PicmanPixelRgn  src_rgn;
  gdouble       dmin_x = 0.0;
  gdouble       dmin_y = 0.0;
  gdouble       dmax_x = 0.0;
  gdouble       dmax_y = 0.0;
  gint          xs, ys;
  gint          xe, ye;
  gint          min_x, min_y;
  gint          max_x, max_y;
  gint          size_x, size_y;
  gint         *max_scanlines, *max_scanlines_iter;
  gint         *min_scanlines, *min_scanlines_iter;
  gint          val;
  gint          alpha;
  gint          bytes;
  guchar        buf[4];
  gint          i, j, x, y;
  gdouble       sx, sy;
  gdouble       ex, ey;
  gdouble       xx, yy;
  gdouble       vec[2];
  gdouble       dist, one_over_dist;
  gint          x1, y1, x2, y2;
  gint          sel_width, sel_height;
  gint         *vals, *vals_iter, *vals_end;
  gint          b;

  sx = poly->pts[0].x;
  sy = poly->pts[0].y;
  ex = poly->pts[1].x;
  ey = poly->pts[1].y;

  dist = sqrt (SQR (ex - sx) + SQR (ey - sy));
  if (dist > 0.0)
    {
      one_over_dist = 1.0 / dist;
      vec[0] = (ex - sx) * one_over_dist;
      vec[1] = (ey - sy) * one_over_dist;
    }
  else
    {
      one_over_dist = 0.0;
      vec[0] = 0.0;
      vec[1] = 0.0;
    }

  if (preview)
    {
      picman_preview_get_position (preview, &x1, &y1);
      picman_preview_get_size (preview, &sel_width, &sel_height);
      x2 = x1 + sel_width;
      y2 = y1 + sel_height;
    }
  else
    {
      picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
      picman_pixel_rgn_init (&src_rgn, drawable,
                           x1, y1, x2 - x1, y2 - y1,
                          TRUE, TRUE);
    }
  bytes = drawable->bpp;

  polygon_extents (poly, &dmin_x, &dmin_y, &dmax_x, &dmax_y);
  min_x = (gint) dmin_x;
  min_y = (gint) dmin_y;
  max_x = (gint) dmax_x;
  max_y = (gint) dmax_y;

  size_y = (max_y - min_y) * SUPERSAMPLE;
  size_x = (max_x - min_x) * SUPERSAMPLE;

  min_scanlines = min_scanlines_iter = g_new (gint, size_y);
  max_scanlines = max_scanlines_iter = g_new (gint, size_y);
  for (i = 0; i < size_y; i++)
    {
      min_scanlines[i] = max_x * SUPERSAMPLE;
      max_scanlines[i] = min_x * SUPERSAMPLE;
    }

  if (poly->npts)
    {
      PicmanVector2 *curptr;
      gint         poly_npts = poly->npts;

      xs = (gint) (poly->pts[poly_npts-1].x);
      ys = (gint) (poly->pts[poly_npts-1].y);
      xe = (gint) poly->pts[0].x;
      ye = (gint) poly->pts[0].y;

      xs *= SUPERSAMPLE;
      ys *= SUPERSAMPLE;
      xe *= SUPERSAMPLE;
      ye *= SUPERSAMPLE;

      convert_segment (xs, ys, xe, ye, min_y * SUPERSAMPLE,
                       min_scanlines, max_scanlines);

      for (i = 1, curptr = &poly->pts[0]; i < poly_npts; i++)
        {
          xs = (gint) curptr->x;
          ys = (gint) curptr->y;
          curptr++;
          xe = (gint) curptr->x;
          ye = (gint) curptr->y;

          xs *= SUPERSAMPLE;
          ys *= SUPERSAMPLE;
          xe *= SUPERSAMPLE;
          ye *= SUPERSAMPLE;

          convert_segment (xs, ys, xe, ye, min_y * SUPERSAMPLE,
                           min_scanlines, max_scanlines);
        }
  }

  vals = g_new (gint, size_x);

  for (i = 0; i < size_y; i++, min_scanlines_iter++, max_scanlines_iter++)
    {
      if (! (i % SUPERSAMPLE))
        {
          memset (vals, 0, sizeof (gint) * size_x);
        }

      yy = (gdouble)i / (gdouble)SUPERSAMPLE + min_y;

      for (j = *min_scanlines_iter; j < *max_scanlines_iter; j++)
        {
          x = j - min_x * SUPERSAMPLE;
          vals[x] += 255;
        }

      if (! ((i + 1) % SUPERSAMPLE))
        {
          y = (i / SUPERSAMPLE) + min_y;

          if (y >= y1 && y < y2)
            {
              for (j = 0; j < size_x; j += SUPERSAMPLE)
                {
                  x = (j / SUPERSAMPLE) + min_x;

                  if (x >= x1 && x < x2)
                    {
                      for (val = 0, vals_iter = &vals[j],
                             vals_end = &vals_iter[SUPERSAMPLE];
                           vals_iter < vals_end;
                           vals_iter++)
                        val += *vals_iter;

                      val /= SQR(SUPERSAMPLE);

                      if (val > 0)
                        {
                          xx = (gdouble) j / (gdouble) SUPERSAMPLE + min_x;
                          alpha = (gint) (val * calc_alpha_blend (vec,
                                                                  one_over_dist,
                                                                  xx - sx,
                                                                  yy - sy));
                          if (preview)
                            {
                              for (b = 0; b < bytes; b++)
                                buf[b] = dest[ ( (y-y1) * sel_width + (x-x1) ) * bytes + b];
                            }
                          else
                            {
                              picman_pixel_rgn_get_pixel (&src_rgn, buf, x, y);
                            }

#ifndef USE_READABLE_BUT_SLOW_CODE
                          {
                            guchar *buf_iter = buf;
                            guchar *col_iter = col;
                            guchar *buf_end  = buf + bytes;

                            for(; buf_iter < buf_end; buf_iter++, col_iter++)
                              *buf_iter = ((guint) (*col_iter * alpha)
                                           + (((guint) *buf_iter)
                                              * (256 - alpha))) >> 8;
                          }
#else /* original, pre-ECL code */
                          for (b = 0; b < bytes; b++)
                            buf[b] = ((col[b] * alpha) + (buf[b] * (255 - alpha))) / 255;

#endif
                          if (preview)
                            {
                              for (b = 0; b < bytes; b++)
                                dest[ ( (y-y1) * sel_width + (x-x1) ) * bytes + b] = buf[b];
                            }
                          else
                            {
                              picman_pixel_rgn_set_pixel (&src_rgn, buf, x, y);
                            }
                        }
                    }
                }
            }
        }
    }

  g_free (vals);
  g_free (min_scanlines);
  g_free (max_scanlines);
}

static void
convert_segment (gint  x1,
                 gint  y1,
                 gint  x2,
                 gint  y2,
                 gint  offset,
                 gint *min,
                 gint *max)
{
  gint ydiff, y, tmp;
  gdouble xinc, xstart;

  if (y1 > y2)
    {
      tmp = y2; y2 = y1; y1 = tmp;
      tmp = x2; x2 = x1; x1 = tmp;
    }
  ydiff = (y2 - y1);

  if (ydiff)
    {
      xinc = (gdouble) (x2 - x1) / (gdouble) ydiff;
      xstart = x1 + 0.5 * xinc;
      for (y = y1 ; y < y2; y++)
        {
          if (xstart < min[y - offset])
            min[y-offset] = xstart;
          if (xstart > max[y - offset])
            max[y-offset] = xstart;

          xstart += xinc;
        }
    }
}

static void
randomize_indices (gint  count,
                   gint *indices)
{
  gint i;
  gint index1, index2;
  gint tmp;
  GRand *gr;

  gr = g_rand_new();

  for (i = 0; i < count * RANDOMNESS; i++)
    {
      index1 = g_rand_int_range (gr, 0, count);
      index2 = g_rand_int_range (gr, 0, count);
      tmp = indices[index1];
      indices[index1] = indices[index2];
      indices[index2] = tmp;
    }

  g_rand_free (gr);
}

static void
polygon_add_point (Polygon *poly,
                   gdouble  x,
                   gdouble  y)
{
  if (poly->npts < MAX_POINTS)
    {
      poly->pts[poly->npts].x = x;
      poly->pts[poly->npts].y = y;
      poly->npts++;
    }
  else
    g_print ("Unable to add additional point.\n");
}

static void
polygon_rotate (Polygon *poly,
                gdouble  theta)
{
  gint i;
  gdouble ct, st;
  gdouble ox, oy;

  ct = cos (theta);
  st = sin (theta);

  for (i = 0; i < poly->npts; i++)
    {
      ox = poly->pts[i].x;
      oy = poly->pts[i].y;
      poly->pts[i].x = ct * ox - st * oy;
      poly->pts[i].y = st * ox + ct * oy;
    }
}

static void
polygon_translate (Polygon *poly,
                   gdouble  tx,
                   gdouble  ty)
{
  gint i;

  for (i = 0; i < poly->npts; i++)
    {
      poly->pts[i].x += tx;
      poly->pts[i].y += ty;
    }
}

static gint
polygon_extents (Polygon *poly,
                 gdouble *x1,
                 gdouble *y1,
                 gdouble *x2,
                 gdouble *y2)
{
  gint i;

  if (!poly->npts)
    return 0;

  *x1 = *x2 = poly->pts[0].x;
  *y1 = *y2 = poly->pts[0].y;

  for (i = 1; i < poly->npts; i++)
    {
      if (poly->pts[i].x < *x1)
        *x1 = poly->pts[i].x;
      if (poly->pts[i].x > *x2)
        *x2 = poly->pts[i].x;
      if (poly->pts[i].y < *y1)
        *y1 = poly->pts[i].y;
      if (poly->pts[i].y > *y2)
        *y2 = poly->pts[i].y;
    }

  return 1;
}

static void
polygon_reset (Polygon *poly)
{
  poly->npts = 0;
}
