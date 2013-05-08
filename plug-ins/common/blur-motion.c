/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Motion Blur plug-in for PICMAN 0.99
 * Copyright (C) 1997 Daniel Skarda (0rfelyus@atrey.karlin.mff.cuni.cz)
 *
 * This plug-in is port of Motion Blur plug-in for PICMAN 0.54 by
 * Thorsten Martinsen
 *      Copyright (C) 1996 Torsten Martinsen <torsten@danbbs.dk>
 *      Bresenham algorithm stuff hacked from HP2xx written by
 *      Heinz W. Werntges
 *      Changes for version 1.11/1.12 Copyright (C) 1996 Federico Mena Quintero
 *      quartic@polloux.fciencias.unam.mx
 *
 * I also used some code from Whirl and Pinch plug-in by Federico Mena Quintero
 *      (federico@nuclecu.unam.mx)
 *
 * Copyright (C) 2007 Joerg Gittinger (sw@gittingerbox.de)
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


#define PLUG_IN_PROC         "plug-in-mblur"
#define PLUG_IN_PROC_INWARD  "plug-in-mblur-inward"
#define PLUG_IN_BINARY       "blur-motion"
#define PLUG_IN_ROLE         "picman-blur-motion"
#define PLUG_IN_VERSION      "May 2007, 1.3"


#define MBLUR_LENGTH_MAX     256.0


typedef enum
{
  MBLUR_LINEAR,
  MBLUR_RADIAL,
  MBLUR_ZOOM,
  MBLUR_MAX = MBLUR_ZOOM
} MBlurType;


typedef struct
{
  gint32    mblur_type;
  gint32    length;
  gint32    angle;
  gdouble   center_x;
  gdouble   center_y;
  gboolean  blur_outward;
} mblur_vals_t;


/***** Prototypes *****/

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const PicmanParam  *param,
                   gint             *nreturn_vals,
                   PicmanParam       **return_vals);

static void      mblur        (PicmanDrawable *drawable,
                               PicmanPreview  *preview);
static void      mblur_linear (PicmanDrawable *drawable,
                               PicmanPreview  *preview,
                               gint          x1,
                               gint          y1,
                               gint          width,
                               gint          height);
static void      mblur_radial (PicmanDrawable *drawable,
                               PicmanPreview  *preview,
                               gint          x1,
                               gint          y1,
                               gint          width,
                               gint          height);
static void      mblur_zoom   (PicmanDrawable *drawable,
                               PicmanPreview  *preview,
                               gint          x1,
                               gint          y1,
                               gint          width,
                               gint          height);

static gboolean  mblur_dialog (gint32        image_ID,
                               PicmanDrawable *drawable);

/***** Variables *****/

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run    /* run_proc   */
};

static mblur_vals_t mbvals =
{
  MBLUR_LINEAR, /* mblur_type   */
  5,            /* length       */
  10,           /* radius       */
  100000.0,     /* center_x     */
  100000.0,     /* center_y     */
  TRUE          /* blur_outward */
};


static GtkObject *length     = NULL;
static GtkObject *angle      = NULL;
static GtkWidget *center     = NULL;
static GtkWidget *dir_button = NULL;
static GtkWidget *preview    = NULL;

static gint       img_width, img_height, img_bpp;
static gboolean   has_alpha;

/***** Functions *****/

MAIN()

static void
query (void)
{
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode",  "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",     "Input image" },
    { PICMAN_PDB_DRAWABLE, "drawable",  "Input drawable" },
    { PICMAN_PDB_INT32,    "type",      "Type of motion blur { LINEAR (0), RADIAL (1), ZOOM (2) }" },
    { PICMAN_PDB_INT32,    "length",    "Length" },
    { PICMAN_PDB_INT32,    "angle",     "Angle" },
    { PICMAN_PDB_FLOAT,    "center-x",  "Center X (optional)" },
    { PICMAN_PDB_FLOAT,    "center-y",  "Center Y (optional)" },
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Simulate movement using directional blur"),
                          "This plug-in simulates the effect seen when "
                          "photographing a moving object at a slow shutter "
                          "speed. Done by adding multiple displaced copies.",
                          "Torsten Martinsen, Federico Mena Quintero, Daniel Skarda, Joerg Gittinger",
                          "Torsten Martinsen, Federico Mena Quintero, Daniel Skarda, Joerg Gittinger",
                          PLUG_IN_VERSION,
                          N_("_Motion Blur..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_install_procedure (PLUG_IN_PROC_INWARD,
                          N_("Simulate movement using directional blur"),
                          "This procedure is equivalent to plug-in-mblur but "
                          "performs the zoom blur inward instead of outward.",
                          "Torsten Martinsen, Federico Mena Quintero, Daniel Skarda, Joerg Gittinger",
                          "Torsten Martinsen, Federico Mena Quintero, Daniel Skarda, Joerg Gittinger",
                          PLUG_IN_VERSION,
                          N_("_Motion Blur..."),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Blur");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam   values[1];
  PicmanRunMode        run_mode;
  PicmanPDBStatusType  status;
  PicmanDrawable      *drawable;
  gint               x1, y1, x2, y2;

  INIT_I18N ();

  status   = PICMAN_PDB_SUCCESS;
  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  /* Get the active drawable info */

  drawable = picman_drawable_get (param[2].data.d_drawable);

  img_width  = picman_drawable_width (drawable->drawable_id);
  img_height = picman_drawable_height (drawable->drawable_id);
  img_bpp    = picman_drawable_bpp (drawable->drawable_id);

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);

  mbvals.center_x = (gdouble) (x1 + x2 - 1) / 2.0;
  mbvals.center_y = (gdouble) (y1 + y2 - 1) / 2.0;

  /* Set the tile cache size */
  picman_tile_cache_ntiles (2 * drawable->ntile_cols);

  switch (run_mode)
    {
    case PICMAN_RUN_INTERACTIVE:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &mbvals);

      /* Get information from the dialog */
      if (! mblur_dialog (param[1].data.d_image, drawable))
        return;
      break;

    case PICMAN_RUN_NONINTERACTIVE:
      if (strcmp (name, PLUG_IN_PROC_INWARD) == 0)
        mbvals.blur_outward = FALSE;

      if (nparams == 8)
        {
          mbvals.center_x = param[6].data.d_float;
          mbvals.center_y = param[7].data.d_float;
        }
      else if (nparams != 6)
        {
          status = PICMAN_PDB_CALLING_ERROR;
        }

      if (status == PICMAN_PDB_SUCCESS)
        {
          mbvals.mblur_type = param[3].data.d_int32;
          mbvals.length     = param[4].data.d_int32;
          mbvals.angle      = param[5].data.d_int32;
        }

    if ((mbvals.mblur_type < 0) || (mbvals.mblur_type > MBLUR_MAX))
      status= PICMAN_PDB_CALLING_ERROR;
    break;

    case PICMAN_RUN_WITH_LAST_VALS:
      /* Possibly retrieve data */
      picman_get_data (PLUG_IN_PROC, &mbvals);
      break;

    default:
      break;
    }

  /* Blur the image */

  if ((status == PICMAN_PDB_SUCCESS) &&
      (picman_drawable_is_rgb(drawable->drawable_id) ||
       picman_drawable_is_gray(drawable->drawable_id)))
    {

      /* Run! */
      has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
      mblur (drawable, NULL);

      /* If run mode is interactive, flush displays */
      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();

      /* Store data */
      if (run_mode == PICMAN_RUN_INTERACTIVE)
        picman_set_data (PLUG_IN_PROC, &mbvals, sizeof (mblur_vals_t));
    }
  else if (status == PICMAN_PDB_SUCCESS)
    status = PICMAN_PDB_EXECUTION_ERROR;

  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static void
mblur_linear (PicmanDrawable *drawable,
              PicmanPreview  *preview,
              gint          x1,
              gint          y1,
              gint          width,
              gint          height)
{
  PicmanPixelRgn      dest_rgn;
  PicmanPixelFetcher *pft;
  gpointer          pr;
  PicmanRGB           background;

  guchar *dest;
  guchar *d;
  guchar  pixel[4];
  gint32  sum[4];
  gint    progress, max_progress;
  gint    c, p;
  gint    x, y, i, n;
  gint    dx, dy, px, py, swapdir, err, e, s1, s2;

  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_pixel_fetcher_set_bg_color (pft, &background);

  progress     = 0;
  max_progress = width * height;

  n = mbvals.length;
  px = (gdouble) n * cos (mbvals.angle / 180.0 * G_PI);
  py = (gdouble) n * sin (mbvals.angle / 180.0 * G_PI);

  /*
   * Initialization for Bresenham algorithm:
   * dx = abs(x2-x1), s1 = sign(x2-x1)
   * dy = abs(y2-y1), s2 = sign(y2-y1)
   */
  if ((dx = px) != 0)
    {
      if (dx < 0)
        {
          dx = -dx;
          s1 = -1;
        }
      else
        s1 = 1;
    }
  else
    s1 = 0;

  if ((dy = py) != 0)
    {
      if (dy < 0)
        {
          dy = -dy;
          s2 = -1;
        }
      else
        s2 = 1;
    }
  else
    s2 = 0;

  if (dy > dx)
    {
      swapdir = dx;
      dx = dy;
      dy = swapdir;
      swapdir = 1;
    }
  else
    swapdir = 0;

  dy *= 2;
  err = dy - dx;        /* Initial error term   */
  dx *= 2;

  for (pr = picman_pixel_rgns_register (1, &dest_rgn), p = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), p++)
    {
      dest = dest_rgn.data;

      for (y = dest_rgn.y; y < dest_rgn.y + dest_rgn.h; y++)
        {
          d = dest;

          for (x = dest_rgn.x; x < dest_rgn.x + dest_rgn.w; x++)
            {
              gint xx, yy;

              xx = x;
              yy = y;
              e = err;

              for (c = 0; c < img_bpp; c++)
                sum[c]= 0;

              for (i = 0; i < n; )
                {
                  picman_pixel_fetcher_get_pixel (pft, xx, yy, pixel);

                  if (has_alpha)
                    {
                      gint32 alpha = pixel[img_bpp-1];

                      sum[img_bpp-1] += alpha;
                      for (c = 0; c < img_bpp-1; c++)
                        sum[c] += pixel[c] * alpha;
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        sum[c] += pixel[c];
                    }
                  i++;

                  while (e >= 0 && dx)
                    {
                      if (swapdir)
                        xx += s1;
                      else
                        yy += s2;
                      e -= dx;
                    }

                  if (swapdir)
                    yy += s2;
                  else
                    xx += s1;

                  e += dy;

                  if ((xx < x1) || (xx >= x1 + width) ||
                      (yy < y1) || (yy >= y1 + height))
                    break;
                }

              if (i == 0)
                {
                  picman_pixel_fetcher_get_pixel (pft, xx, yy, d);
                }
              else
                {
                  if (has_alpha)
                    {
                      gint32 alpha = sum[img_bpp-1];

                      if ((d[img_bpp-1] = alpha/i) != 0)
                        {
                          for (c = 0; c < img_bpp-1; c++)
                            d[c] = sum[c] / alpha;
                        }
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        d[c] = sum[c] / i;
                    }
                }

              d += dest_rgn.bpp;
            }

          dest += dest_rgn.rowstride;
        }

      if (preview)
        {
          picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                             &dest_rgn);
        }
      else
        {
          progress += dest_rgn.w * dest_rgn.h;

          if ((p % 8) == 0)
            picman_progress_update ((gdouble) progress / max_progress);
        }
    }

  picman_pixel_fetcher_destroy (pft);
}


static void
mblur_radial (PicmanDrawable *drawable,
              PicmanPreview  *preview,
              gint          x1,
              gint          y1,
              gint          width,
              gint          height)
{
  PicmanPixelRgn      dest_rgn;
  PicmanPixelFetcher *pft;
  gpointer          pr;
  PicmanRGB           background;

  gdouble   center_x;
  gdouble   center_y;
  guchar   *dest;
  guchar   *d;
  guchar    pixel[4];
  guchar    p1[4], p2[4], p3[4], p4[4];
  gint32    sum[4];

  gint      progress, max_progress, c;

  gint      x, y, i, p, n, count;
  gdouble   angle, theta, r, xr, yr;
  gdouble   phi, phi_start, s_val, c_val;
  gdouble   dx, dy;

  /* initialize */

  center_x = mbvals.center_x;
  center_y = mbvals.center_y;

  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_pixel_fetcher_set_bg_color (pft, &background);

  progress     = 0;
  max_progress = width * height;

  angle = picman_deg_to_rad (mbvals.angle);

  for (pr = picman_pixel_rgns_register (1, &dest_rgn), p = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), p++)
    {
      dest = dest_rgn.data;

      for (y = dest_rgn.y; y < dest_rgn.y + dest_rgn.h; y++)
        {
          d = dest;

          for (x = dest_rgn.x; x < dest_rgn.x + dest_rgn.w; x++)
            {
              gdouble xx = 0.0;
              gdouble yy = 0.0;

              xr = (gdouble) x - center_x;
              yr = (gdouble) y - center_y;

              r = sqrt (SQR (xr) + SQR (yr));
              n = r * angle;

              if (angle == 0.0)
                {
                  picman_pixel_fetcher_get_pixel (pft, x, y, d);
                  d += dest_rgn.bpp;
                  continue;
                }

              /* ensure quality with small angles */
              if (n < 3)
                n = 3;  /* always use at least 3 (interpolation) steps */

              /* limit loop count due to performanc reasons */
              if (n > 100)
                n = 100 + sqrt (n-100);

              if (xr != 0.0)
                {
                  phi = atan(yr/xr);
                  if (xr < 0.0)
                    phi = G_PI + phi;

                }
              else
                {
                  if (yr >= 0.0)
                    phi = G_PI_2;
                  else
                    phi = -G_PI_2;
                }

              for (c = 0; c < img_bpp; c++)
                sum[c] = 0;

              if (n == 1)
                phi_start = phi;
              else
                phi_start = phi + angle/2.0;

              theta = angle / (gdouble)n;
              count = 0;

              for (i = 0; i < n; i++)
                {
                  s_val = sin (phi_start - (gdouble) i * theta);
                  c_val = cos (phi_start - (gdouble) i * theta);

                  xx = center_x + r * c_val;
                  yy = center_y + r * s_val;

                  if ((yy < y1) || (yy >= y1 + height) ||
                      (xx < x1) || (xx >= x1 + width))
                    continue;

                  ++count;
                  if ((xx + 1 < x1 + width) && (yy + 1 < y1 + height))
                    {
                      dx = xx - floor (xx);
                      dy = yy - floor (yy);

                      picman_pixel_fetcher_get_pixel (pft, xx,   yy,   p1);
                      picman_pixel_fetcher_get_pixel (pft, xx+1, yy,   p2);
                      picman_pixel_fetcher_get_pixel (pft, xx,   yy+1, p3);
                      picman_pixel_fetcher_get_pixel (pft, xx+1, yy+1, p4);

                      for (c = 0; c < img_bpp; c++)
                        {
                          pixel[c] = (((gdouble) p1[c] * (1.0-dx) +
                                       (gdouble) p2[c] * dx) * (1.0-dy) +
                                      ((gdouble) p3[c] * (1.0-dx) +
                                       (gdouble) p4[c] * dx) * dy);
                        }
                    }
                  else
                    {
                      picman_pixel_fetcher_get_pixel (pft, xx+.5, yy+.5, pixel);
                    }

                  if (has_alpha)
                    {
                      gint32 alpha = pixel[img_bpp-1];

                      sum[img_bpp-1] += alpha;

                      for (c = 0; c < img_bpp-1; c++)
                        sum[c] += pixel[c] * alpha;
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        sum[c] += pixel[c];
                    }
                }

              if (count == 0)
                {
                  picman_pixel_fetcher_get_pixel (pft, xx, yy, d);
                }
              else
                {
                  if (has_alpha)
                    {
                      gint32 alpha = sum[img_bpp-1];

                      if ((d[img_bpp-1] = alpha/count) != 0)
                        {
                          for (c = 0; c < img_bpp-1; c++)
                            d[c] = sum[c] / alpha;
                        }
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        d[c] = sum[c] / count;
                    }
                }

              d += dest_rgn.bpp;
            }

          dest += dest_rgn.rowstride;
        }

      if (preview)
        {
          picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                             &dest_rgn);
        }
      else
        {
          progress += dest_rgn.w * dest_rgn.h;

          if ((p % 8) == 0)
            picman_progress_update ((gdouble) progress / max_progress);
        }
    }

  picman_pixel_fetcher_destroy (pft);

}


static void
mblur_zoom (PicmanDrawable *drawable,
            PicmanPreview  *preview,
            gint          x1,
            gint          y1,
            gint          width,
            gint          height)
{
  PicmanPixelRgn      dest_rgn;
  PicmanPixelFetcher *pft;
  gpointer          pr;
  PicmanRGB           background;

  gdouble   center_x;
  gdouble   center_y;
  guchar   *dest, *d;
  guchar    pixel[4];
  guchar    p1[4], p2[4], p3[4], p4[4];
  gint32    sum[4];

  gint      progress, max_progress;
  gint      x, y, i, n, p, c;
  gdouble   xx_start, xx_end, yy_start, yy_end;
  gdouble   dxx, dyy;
  gdouble   dx, dy;
  gint      xy_len;
  gdouble   f, r;
  gint      drawable_x1, drawable_y1;
  gint      drawable_x2, drawable_y2;

  /* initialize */

  center_x = mbvals.center_x;
  center_y = mbvals.center_y;

  picman_drawable_mask_bounds (drawable->drawable_id,
                             &drawable_x1, &drawable_y1,
                             &drawable_x2, &drawable_y2);
  picman_pixel_rgn_init (&dest_rgn, drawable,
                       x1, y1, width, height, (preview == NULL), TRUE);

  pft = picman_pixel_fetcher_new (drawable, FALSE);

  picman_context_get_background (&background);
  picman_pixel_fetcher_set_bg_color (pft, &background);

  progress     = 0;
  max_progress = width * height;

  n = mbvals.length;

  if (n == 0)
    n = 1;

  r = sqrt (SQR (drawable->width / 2) + SQR (drawable->height / 2));
  n = ((gdouble) n * r / MBLUR_LENGTH_MAX);
  f = (r-n)/r;

  for (pr = picman_pixel_rgns_register (1, &dest_rgn), p = 0;
       pr != NULL;
       pr = picman_pixel_rgns_process (pr), p++)
    {
      dest = dest_rgn.data;

      for (y = dest_rgn.y; y < dest_rgn.y + dest_rgn.h; y++)
        {
          d = dest;

          for (x = dest_rgn.x; x < dest_rgn.x + dest_rgn.w; x++)
            {
              gdouble xx, yy;

              for (c = 0; c < img_bpp; c++)
                sum[c] = 0;

              xx_start = x;
              yy_start = y;

              if (mbvals.blur_outward)
                {
                  xx_end = center_x + ((gdouble) x - center_x) * f;
                  yy_end = center_y + ((gdouble) y - center_y) * f;
                }
              else
                {
                  xx_end = center_x + ((gdouble) x - center_x) * (1.0/f);
                  yy_end = center_y + ((gdouble) y - center_y) * (1.0/f);
                }

              xy_len = sqrt (SQR (xx_end-xx_start) + SQR (yy_end-yy_start)) + 1;

              if (xy_len < 3)
                xy_len = 3;

              dxx = (xx_end - xx_start) / (gdouble) xy_len;
              dyy = (yy_end - yy_start) / (gdouble) xy_len;

              xx = xx_start;
              yy = yy_start;

              for (i = 0; i < xy_len; i++)
                {
                  if ((yy < drawable_y1) || (yy >= drawable_y2) ||
                      (xx < drawable_x1) || (xx >= drawable_x2))
                    break;

                  if ((xx+1 < drawable_x2) && (yy+1 < drawable_y2))
                    {
                      dx = xx - floor (xx);
                      dy = yy - floor (yy);

                      picman_pixel_fetcher_get_pixel (pft, xx,   yy,   p1);
                      picman_pixel_fetcher_get_pixel (pft, xx+1, yy,   p2);
                      picman_pixel_fetcher_get_pixel (pft, xx,   yy+1, p3);
                      picman_pixel_fetcher_get_pixel (pft, xx+1, yy+1, p4);

                      for (c = 0; c < img_bpp; c++)
                        {
                          pixel[c] = (((gdouble)p1[c] * (1.0-dx) +
                                       (gdouble)p2[c] * dx) * (1.0-dy) +
                                      ((gdouble)p3[c] * (1.0-dx) +
                                       (gdouble)p4[c] * dx) * dy);
                        }
                    }
                  else
                    {
                      picman_pixel_fetcher_get_pixel (pft, xx+.5, yy+.5, pixel);
                    }

                  if (has_alpha)
                    {
                      gint32 alpha = pixel[img_bpp-1];

                      sum[img_bpp-1] += alpha;

                      for (c = 0; c < img_bpp-1; c++)
                        sum[c] += pixel[c] * alpha;
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        sum[c] += pixel[c];
                    }

                  xx += dxx;
                  yy += dyy;
                }

              if (i == 0)
                {
                  picman_pixel_fetcher_get_pixel (pft, xx, yy, d);
                }
              else
                {
                  if (has_alpha)
                    {
                      gint32 alpha = sum[img_bpp-1];

                      if ((d[img_bpp-1] = alpha/i) != 0)
                        {
                          for (c = 0; c < img_bpp-1; c++)
                            d[c] = sum[c] / alpha;
                        }
                    }
                  else
                    {
                      for (c = 0; c < img_bpp; c++)
                        d[c] = sum[c] / i;
                    }
                }

              d += dest_rgn.bpp;
            }

          dest += dest_rgn.rowstride;
        }

      if (preview)
        {
          picman_drawable_preview_draw_region (PICMAN_DRAWABLE_PREVIEW (preview),
                                             &dest_rgn);
        }
      else
        {
          progress += dest_rgn.w * dest_rgn.h;

          if ((p % 8) == 0)
            picman_progress_update ((gdouble) progress / max_progress);
        }
    }

  picman_pixel_fetcher_destroy (pft);
}

static void
mblur (PicmanDrawable *drawable,
       PicmanPreview  *preview)
{
  gint x, y;
  gint width, height;

  if (preview)
    {
      picman_preview_get_position (preview, &x, &y);
      picman_preview_get_size (preview, &width, &height);
    }
  else if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                           &x, &y, &width, &height))
    {
      return;
    }

  if (! preview)
    picman_progress_init (_("Motion blurring"));

  switch (mbvals.mblur_type)
    {
    case MBLUR_LINEAR:
      mblur_linear (drawable, preview, x, y, width, height);
      break;

    case MBLUR_RADIAL:
      mblur_radial (drawable, preview, x, y, width, height);
      break;

    case MBLUR_ZOOM:
      mblur_zoom (drawable, preview, x, y, width, height);
      break;

    default:
      break;
    }

  if (! preview)
    {
      picman_progress_update (1.0);

      picman_drawable_flush (drawable);
      picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
      picman_drawable_update (drawable->drawable_id, x, y, width, height);
    }
}

/****************************************
 *                 UI
 ****************************************/

static void
mblur_set_sensitivity (void)
{
  if (!length || !angle || !center || !dir_button)
    return;                     /* Not initialized yet */

  switch (mbvals.mblur_type)
    {
    case MBLUR_LINEAR:
      picman_scale_entry_set_sensitive (length, TRUE);
      picman_scale_entry_set_sensitive (angle, TRUE);
      gtk_widget_set_sensitive (center, FALSE);
      gtk_widget_set_sensitive (dir_button, FALSE);
      break;

    case MBLUR_RADIAL:
      picman_scale_entry_set_sensitive (length, FALSE);
      picman_scale_entry_set_sensitive (angle, TRUE);
      gtk_widget_set_sensitive (center, TRUE);
      gtk_widget_set_sensitive (dir_button, FALSE);
      break;

    case MBLUR_ZOOM:
      picman_scale_entry_set_sensitive (length, TRUE);
      picman_scale_entry_set_sensitive (angle, FALSE);
      gtk_widget_set_sensitive (center, TRUE);
      gtk_widget_set_sensitive (dir_button, TRUE);
      break;

    default:
      break;
    }
}

static void
mblur_radio_button_update (GtkWidget *widget,
                           gpointer   data)
{
  picman_radio_button_update (widget, data);
  mblur_set_sensitivity ();
  picman_preview_invalidate (PICMAN_PREVIEW (preview));
}

static void
mblur_center_update (PicmanSizeEntry *entry)
{
  mbvals.center_x = picman_size_entry_get_refval (entry, 0);
  mbvals.center_y = picman_size_entry_get_refval (entry, 1);
}

static gboolean
mblur_dialog (gint32        image_ID,
              PicmanDrawable *drawable)
{
  GtkWidget *dialog;
  GtkWidget *main_vbox;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *entry;
  GtkWidget *spinbutton;
  GtkWidget *button;
  GtkObject *adj;
  gdouble    xres, yres;
  gboolean   run;

  picman_ui_init (PLUG_IN_BINARY, FALSE);

  dialog = picman_dialog_new (_("Motion Blur"), PLUG_IN_ROLE,
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

  g_signal_connect_swapped (preview, "invalidated",
                            G_CALLBACK (mblur),
                            drawable);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  frame = picman_int_radio_group_new (TRUE, _("Blur Type"),
                                    G_CALLBACK (mblur_radio_button_update),
                                    &mbvals.mblur_type, mbvals.mblur_type,

                                    C_("blur-type", "_Linear"),  MBLUR_LINEAR, NULL,
                                    C_("blur-type", "_Radial"),  MBLUR_RADIAL, NULL,
                                    C_("blur-type", "_Zoom"),    MBLUR_ZOOM,   NULL,

                                    NULL);

  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  center = picman_frame_new (_("Blur Center"));
  gtk_box_pack_start (GTK_BOX (hbox), center, FALSE, FALSE, 0);
  gtk_widget_show (center);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_add (GTK_CONTAINER (center), vbox);
  gtk_widget_show (vbox);

  picman_image_get_resolution (image_ID, &xres, &yres);

  entry = picman_size_entry_new (1,
                               PICMAN_UNIT_PIXEL, "%a",
                               TRUE, FALSE, FALSE, 5,
                               PICMAN_SIZE_ENTRY_UPDATE_SIZE);
  gtk_table_set_row_spacings (GTK_TABLE (entry), 2);
  gtk_table_set_col_spacing (GTK_TABLE (entry), 0, 6);
  gtk_table_set_col_spacing (GTK_TABLE (entry), 2, 6);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (entry);

  g_signal_connect (entry, "value-changed",
                    G_CALLBACK (mblur_center_update),
                    NULL);
  g_signal_connect_swapped (entry, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  spinbutton = picman_spin_button_new (&adj, 1, 0, 1, 1, 10, 0, 1, 2);
  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (entry),
                             GTK_SPIN_BUTTON (spinbutton), NULL);
  gtk_table_attach_defaults (GTK_TABLE (entry), spinbutton, 1, 2, 0, 1);
  gtk_widget_show (spinbutton);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 0, xres, TRUE);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (entry), 0, mbvals.center_x);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (entry),
                                _("_X:"), 0, 0, 0.0);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (entry), 1, yres, TRUE);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (entry), 1, mbvals.center_y);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (entry),
                                _("_Y:"), 1, 0, 0.0);

  button = gtk_check_button_new_with_mnemonic (_("Blur _outward"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                mbvals.blur_outward);
  gtk_box_pack_start (GTK_BOX (main_vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);
  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &mbvals.blur_outward);
  g_signal_connect_swapped (button, "toggled",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);
  dir_button = button;

  frame = picman_frame_new (_("Blur Parameters"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  length = picman_scale_entry_new (GTK_TABLE (table), 0, 0,
                                 _("L_ength:"), 150, 3,
                                 mbvals.length, 1.0, MBLUR_LENGTH_MAX, 1.0, 8.0, 0,
                                 TRUE, 0, 0,
                                 NULL, NULL);

  g_signal_connect (length, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &mbvals.length);
  g_signal_connect_swapped (length, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  angle = picman_scale_entry_new (GTK_TABLE (table), 0, 1,
                                _("_Angle:"), 150, 3,
                                mbvals.angle, 0.0, 360.0, 1.0, 15.0, 0,
                                TRUE, 0, 0,
                                NULL, NULL);

  g_signal_connect (angle, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &mbvals.angle);
  g_signal_connect_swapped (angle, "value-changed",
                            G_CALLBACK (picman_preview_invalidate),
                            preview);

  mblur_set_sensitivity ();

  gtk_widget_show (dialog);

  run = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}
