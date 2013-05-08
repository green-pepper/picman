/* Color Enhance 0.10 --- image filter plug-in for PICMAN
 *
 * Copyright (C) 1999 Martin Weber
 * Copyright (C) 1996 Federico Mena Quintero
 *
 * You can contact me at martweb@gmx.net
 * You can contact the original PICMAN authors at picman@xcf.berkeley.edu
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

#include <libpicman/picman.h>

#include "libpicman/stdplugins-intl.h"


#define PLUG_IN_PROC "plug-in-color-enhance"


/* Declare local functions.
 */
static void   query                 (void);
static void   run                   (const gchar      *name,
                                     gint              nparams,
                                     const PicmanParam  *param,
                                     gint             *nreturn_vals,
                                     PicmanParam       **return_vals);

static void   Color_Enhance         (PicmanDrawable     *drawable);
static void   indexed_Color_Enhance (gint32            image_ID);


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
    { PICMAN_PDB_IMAGE,    "image",    "Input image"    },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable" }
  };

  picman_install_procedure (PLUG_IN_PROC,
                          N_("Stretch color saturation to cover maximum possible range"),
                          "This simple plug-in does an automatic saturation "
                          "stretch.  For each channel in the image, it finds "
                          "the minimum and maximum values... it uses those "
                          "values to stretch the individual histograms to the "
                          "full range.  For some images it may do just what "
                          "you want; for others it may not work that well.  "
                          "This version differs from Contrast Autostretch in "
                          "that it works in HSV space, and preserves hue.",
                          "Martin Weber",
                          "Martin Weber",
                          "1997",
                          N_("_Color Enhance"),
                          "RGB*, INDEXED*",
                          PICMAN_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Colors/Auto");
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
  gint32             image_ID;

  INIT_I18N();

  run_mode = param[0].data.d_int32;

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);
  image_ID = param[1].data.d_image;

  /*  Make sure that the drawable is gray or RGB color  */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_progress_init (_("Color Enhance"));
      picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));
      Color_Enhance (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else if (picman_drawable_is_indexed (drawable->drawable_id))
    {
      indexed_Color_Enhance (image_ID);

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

static gdouble
get_v (const guchar *src)
{
  gdouble h, z, v;
  gint    c, m, y;
  gint    k;
  guchar  map[3];

  c = 255 - src[0];
  m = 255 - src[1];
  y = 255 - src[2];

  k = c;
  if (m < k) k = m;
  if (y < k) k = y;

  map[0] = c - k;
  map[1] = m - k;
  map[2] = y - k;

  picman_rgb_to_hsv4(map, &h, &z, &v);

  return v;
}

static void
enhance_it (const guchar *src, guchar *dest, gdouble vlo, gdouble vhi)
{
  gdouble h, z, v;
  gint    c, m, y;
  gint    k;
  guchar  map[3];

  c = 255 - src[0];
  m = 255 - src[1];
  y = 255 - src[2];

  k = c;
  if (m < k) k = m;
  if (y < k) k = y;

  map[0] = c - k;
  map[1] = m - k;
  map[2] = y - k;

  picman_rgb_to_hsv4 (map, &h, &z, &v);

  if (vhi != vlo)
    v = (v - vlo) / (vhi - vlo);

  picman_hsv_to_rgb4 (map, h, z, v);

  c = map[0];
  m = map[1];
  y = map[2];

  c += k;
  if (c > 255) c = 255;
  m += k;
  if (m > 255) m = 255;
  y += k;
  if (y > 255) y = 255;

  dest[0] = 255 - c;
  dest[1] = 255 - m;
  dest[2] = 255 - y;
}

static void
indexed_Color_Enhance (gint32 image_ID)
{
  guchar *cmap;
  gint    ncols,i;
  gdouble vhi = 0.0, vlo = 1.0;

  cmap = picman_image_get_colormap (image_ID, &ncols);

  if (!cmap)
    {
      g_message ("colormap was NULL!  Quitting.");
      picman_quit();
    }

  for (i = 0; i < ncols; i++)
    {
      gdouble v = get_v (&cmap[3 * i]);

      if (v > vhi) vhi = v;
      if (v < vlo) vlo = v;
    }

  for (i = 0; i < ncols; i++)
    {
      enhance_it (&cmap[3 * i], &cmap[3 * i], vlo, vhi);
    }

  picman_image_set_colormap (image_ID, cmap, ncols);
}

typedef struct
{
  gdouble  vhi;
  gdouble  vlo;
  gboolean has_alpha;
} ColorEnhanceParam_t;

static void
find_vhi_vlo (const guchar *src,
              gint          bpp,
              gpointer      data)
{
  ColorEnhanceParam_t *param = (ColorEnhanceParam_t*) data;

  if (!param->has_alpha || src[3])
    {
      gdouble v = get_v (src);

      if (v > param->vhi) param->vhi = v;
      if (v < param->vlo) param->vlo = v;
    }
}

static void
color_enhance_func (const guchar *src,
                    guchar       *dest,
                    gint          bpp,
                    gpointer      data)
{
  ColorEnhanceParam_t *param = (ColorEnhanceParam_t*) data;

  enhance_it (src, dest, param->vlo, param->vhi);

  if (param->has_alpha)
    dest[3] = src[3];
}

static void
Color_Enhance (PicmanDrawable *drawable)
{
  ColorEnhanceParam_t param;

  param.has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  param.vhi = 0.0;
  param.vlo = 1.0;

  picman_rgn_iterate1 (drawable, 0 /* unused */, find_vhi_vlo, &param);
  picman_rgn_iterate2 (drawable, 0 /* unused */, color_enhance_func, &param);
}
