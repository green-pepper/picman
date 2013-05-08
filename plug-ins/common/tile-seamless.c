/* Tiler v0.31
 * 22 May 1997
 * Tim Rowley <tor@cs.brown.edu>
 *
 * PICMAN - The GNU Image Manipulation Program
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

#include "config.h"

#include <string.h>

#include <libpicman/picman.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC "plug-in-make-seamless"


/* Declare local functions.
 */
static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const PicmanParam  *param,
                   gint             *nreturn_vals,
                   PicmanParam       **return_vals);

static void tile  (PicmanDrawable     *drawable);

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
  static const PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (unused)" },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Alters edges to make the image seamlessly tileable"),
                          "This plugin creates a seamless tileable from "
                          "the input drawable",
                          "Tim Rowley",
                          "Tim Rowley",
                          "1997",
                          N_("_Make Seamless"),
                          "RGB*, GRAY*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Filters/Map");
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

  run_mode = param[0].data.d_int32;

  INIT_I18N ();

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);

  /*  Make sure that the drawable is gray or RGB color  */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));
      tile(drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

static void
weld_pixels (guchar *dest1,
             guchar *dest2,
             gint    width,
             gint    height,
             gint    x,
             gint    y,
             guint   bpp,
             guchar *src1,
             guchar *src2)
{
  gdouble a = (ABS(x - width) - 1)/ (gdouble) (width - 1);
  gdouble b = (ABS(y - height) - 1) / (gdouble) (height - 1);
  gdouble w;
  guint   i;

  /* mimic ambiguous point handling in original algorithm */
  if (a < 1e-8 && b > 0.99999999)
    w = 1.0;
  else if (a > 0.99999999 && b < 1e-8)
    w = 0.0;
  else
    w = 1.0 - a*b/(a*b + (1.0 - a)*(1.0 - b));

  for (i = 0; i < bpp; i++)
    dest1[i] = dest2[i] = (guchar) (w * src1[i] + (1.0 - w) * src2[i]);
}

static void
weld_pixels_alpha (guchar *dest1,
                   guchar *dest2,
                   gint    width,
                   gint    height,
                   gint    x,
                   gint    y,
                   guint   bpp,
                   guchar *src1,
                   guchar *src2)
{
  gdouble a = (ABS(x - width) - 1)/ (gdouble) (width - 1);
  gdouble b = (ABS(y - height) - 1) / (gdouble) (height - 1);
  gdouble w;
  gdouble alpha;
  guint   ai = bpp-1;
  guint   i;

  /* mimic ambiguous point handling in original algorithm */
  if (a < 1e-8 && b > 0.99999999)
    w = 1.0;
  else if (a > 0.99999999 && b < 1e-8)
    w = 0.0;
  else
    w = 1.0 - a*b/(a*b + (1.0 - a)*(1.0 - b));

  alpha = w * src1[ai] + (1.0 - w) * src2[ai];

  dest1[ai] = dest2[ai] = (guchar) alpha;
  if (dest1[ai])
    {
      for (i = 0; i < ai; i++)
        dest1[i] = dest2[i] = (guchar) ((w * src1[i] * src1[ai]
                                        + (1.0 - w) * src2[i] * src2[ai])
                                        / alpha);
    }
}

static void
tile_region (PicmanDrawable *drawable,
             gboolean      left,
             gint          x1,
             gint          y1,
             gint          x2,
             gint          y2)
{
  glong         width, height;
  gint          bpp;
  gint          wodd, hodd;
  gint          w, h, x, y;
  gint          rgn1_x, rgn2_x;
  static gint   progress = 0;
  gint          max_progress;
  PicmanPixelRgn  src1_rgn, src2_rgn, dest1_rgn, dest2_rgn;
  gpointer      pr;
  gboolean      has_alpha;
  guint         asymmetry_correction;

  bpp = picman_drawable_bpp (drawable->drawable_id);
  has_alpha = picman_drawable_has_alpha (drawable->drawable_id);

  height = y2 - y1;
  width = x2 - x1;

  wodd = width & 1;
  hodd = height & 1;

  w = width / 2;
  h = height / 2;

  if (left)
    {
      rgn1_x = x1;
      rgn2_x = x1 + w + wodd;
    }
  else
    {
      rgn1_x = x1 + w + wodd;
      rgn2_x = x1;
    }

  asymmetry_correction = !wodd && !left;

  picman_pixel_rgn_init (&src1_rgn, drawable, rgn1_x, y1, w, h, FALSE, FALSE);
  picman_pixel_rgn_init (&dest1_rgn, drawable, rgn1_x, y1, w, h, TRUE, TRUE);
  picman_pixel_rgn_init (&src2_rgn, drawable, rgn2_x, y1 + h + hodd,
                       w, h, FALSE, FALSE);
  picman_pixel_rgn_init (&dest2_rgn, drawable, rgn2_x, y1 + h + hodd,
                       w, h, TRUE, TRUE);

  max_progress = width * height / 2;

  for (pr = picman_pixel_rgns_register (4, &src1_rgn, &dest1_rgn, &src2_rgn,
                                      &dest2_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      guchar *src1  = src1_rgn.data;
      guchar *dest1 = dest1_rgn.data;
      guchar *src2  = src2_rgn.data;
      guchar *dest2 = dest2_rgn.data;
      gint   row    = src1_rgn.y - y1;

      for (y = 0; y < src1_rgn.h; y++, row++)
        {
          guchar *s1 = src1;
          guchar *s2 = src2;
          guchar *d1 = dest1;
          guchar *d2 = dest2;
          gint col = src1_rgn.x - x1;

          if (has_alpha)
            {
              for (x = 0; x < src1_rgn.w; x++, col++)
                {
                  weld_pixels_alpha (d1, d2, w, h, col + asymmetry_correction,
                                     row, bpp, s1, s2);
                  s1 += bpp;
                  s2 += bpp;
                  d1 += bpp;
                  d2 += bpp;
                }
            }
          else
            {
              for (x = 0; x < src1_rgn.w; x++, col++)
                {
                  weld_pixels (d1, d2, w, h, col + asymmetry_correction,
                              row, bpp, s1, s2);
                  s1 += bpp;
                  s2 += bpp;
                  d1 += bpp;
                  d2 += bpp;
                }
            }

          src1 += src1_rgn.rowstride;
          src2 += src2_rgn.rowstride;
          dest1 += dest1_rgn.rowstride;
          dest2 += dest2_rgn.rowstride;
        }

      progress += src1_rgn.w * src1_rgn.h;
      picman_progress_update ((gdouble) progress / (gdouble) max_progress);
    }
}

static void
copy_region (PicmanDrawable *drawable,
             gint          x,
             gint          y,
             gint          w,
             gint          h)
{
  PicmanPixelRgn src_rgn, dest_rgn;
  gpointer     pr;

  picman_pixel_rgn_init (&src_rgn, drawable, x, y, w, h, FALSE, FALSE);
  picman_pixel_rgn_init (&dest_rgn, drawable, x, y, w, h, TRUE, TRUE);

  for (pr = picman_pixel_rgns_register (2, &src_rgn, &dest_rgn);
       pr != NULL;
       pr = picman_pixel_rgns_process (pr))
    {
      gint k;

      for (k = 0; k < src_rgn.h; k++)
        {
          memcpy (dest_rgn.data + k * dest_rgn.rowstride,
                  src_rgn.data + k * src_rgn.rowstride,
                  src_rgn.w * src_rgn.bpp);
        }
    }
}

static void
tile (PicmanDrawable *drawable)
{
  glong width, height;
  gint  x1, y1, x2, y2;

  picman_drawable_mask_bounds (drawable->drawable_id, &x1, &y1, &x2, &y2);
  picman_progress_init (_("Tiler"));

  height = y2 - y1;
  width = x2 - x1;

  if (width & 1) /* Copy middle column */
    {
       copy_region (drawable, x1 + width / 2, y1, 1, height);
    }

  if (height & 1) /* Copy middle row */
    {
      copy_region (drawable, x1, y1 + height / 2, width, 1);
    }

  tile_region (drawable, TRUE, x1, y1, x2, y2);
  tile_region (drawable, FALSE, x1, y1, x2, y2);
  picman_progress_update (1.0);

  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x1, y1, width, height);
}
