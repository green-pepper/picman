/* Contrast Autostretch 1.06 --- image filter plug-in for PICMAN
 *
 * Copyright (C) 1996 Federico Mena Quintero
 *
 * You can contact me at quartic@polloux.fciencias.unam.mx
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


#define PLUG_IN_PROC "plug-in-c-astretch"


/* Declare local functions.
 */
static void   query              (void);
static void   run                (const gchar      *name,
                                  gint              nparams,
                                  const PicmanParam  *param,
                                  gint             *nreturn_vals,
                                  PicmanParam       **return_vals);

static void   c_astretch         (PicmanDrawable     *drawable);
static void   indexed_c_astretch (gint32            image_ID);


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
                          N_("Stretch contrast to cover the maximum possible range"),
                          "This simple plug-in does an automatic contrast "
                          "stretch.  For each channel in the image, it finds "
                          "the minimum and maximum values... it uses those "
                          "values to stretch the individual histograms to the "
                          "full contrast range.  For some images it may do "
                          "just what you want; for others it may not work "
                          "that well.",
                          "Federico Mena Quintero",
                          "Federico Mena Quintero",
                          "1996",
                          N_("_Stretch Contrast"),
                          "RGB*, GRAY*, INDEXED*",
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
      picman_progress_init (_("Auto-stretching contrast"));
      picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));
      c_astretch (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else if (picman_drawable_is_indexed (drawable->drawable_id))
    {
      indexed_c_astretch (image_ID);

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
indexed_c_astretch (gint32 image_ID)
{
  guchar *cmap;
  gint    ncols, i;
  gint    rhi = 0, ghi = 0, bhi = 0, rlo = 255, glo = 255, blo = 255;

  cmap = picman_image_get_colormap (image_ID, &ncols);

  if (!cmap)
    {
      g_message (_("c_astretch: cmap was NULL!  Quitting...\n"));
      picman_quit();
    }

  for (i = 0; i < ncols; i++)
    {
      if (cmap[i * 3 + 0] > rhi) rhi = cmap[i * 3 + 0];
      if (cmap[i * 3 + 1] > ghi) ghi = cmap[i * 3 + 1];
      if (cmap[i * 3 + 2] > bhi) bhi = cmap[i * 3 + 2];
      if (cmap[i * 3 + 0] < rlo) rlo = cmap[i * 3 + 0];
      if (cmap[i * 3 + 1] < glo) glo = cmap[i * 3 + 1];
      if (cmap[i * 3 + 2] < blo) blo = cmap[i * 3 + 2];
    }

  for (i = 0; i < ncols; i++)
    {
      if (rhi != rlo)
        cmap[i * 3 + 0] = (255 * (cmap[i * 3 + 0] - rlo)) / (rhi - rlo);
      if (ghi != glo)
        cmap[i * 3 + 1] = (255 * (cmap[i * 3 + 1] - glo)) / (ghi - glo);
      if (rhi != rlo)
        cmap[i * 3 + 2] = (255 * (cmap[i * 3 + 2] - blo)) / (bhi - blo);
    }

  picman_image_set_colormap (image_ID, cmap, ncols);
}

typedef struct {
  gint          alpha;
  guchar        lut[256][3];
  guchar        min[3];
  guchar        max[3];
  gboolean      has_alpha;
} AutoStretchParam_t;

static void
find_min_max (const guchar *src,
              gint         bpp,
              gpointer     data)
{
  AutoStretchParam_t *param = data;
  gint                b;

  for (b = 0; b < param->alpha; b++)
    {
      if (!param->has_alpha || src[param->alpha])
        {
          if (src[b] < param->min[b])
            param->min[b] = src[b];
          if (src[b] > param->max[b])
            param->max[b] = src[b];
        }
    }
}

static void
c_astretch_func (const guchar *src,
                 guchar       *dest,
                 gint          bpp,
                 gpointer      data)
{
  AutoStretchParam_t *param = data;
  gint                b;

  for (b = 0; b < param->alpha; b++)
    dest[b] = param->lut[src[b]][b];

  if (param->has_alpha)
    dest[param->alpha] = src[param->alpha];
}

static void
c_astretch (PicmanDrawable *drawable)
{
  AutoStretchParam_t param;
  gint               b;

  param.has_alpha = picman_drawable_has_alpha (drawable->drawable_id);
  param.alpha = (param.has_alpha) ? drawable->bpp - 1 : drawable->bpp;

  /* Get minimum and maximum values for each channel */
  param.min[0] = param.min[1] = param.min[2] = 255;
  param.max[0] = param.max[1] = param.max[2] = 0;

  picman_rgn_iterate1 (drawable, 0 /* unused */, find_min_max, &param);

  /* Calculate LUTs with stretched contrast */
  for (b = 0; b < param.alpha; b++)
    {
      gint range = param.max[b] - param.min[b];
      gint x;

      if (range != 0)
        for (x = param.min[b]; x <= param.max[b]; x++)
          param.lut[x][b] = 255 * (x - param.min[b]) / range;
      else
        param.lut[param.min[b]][b] = param.min[b];
    }

  picman_rgn_iterate2 (drawable, 0 /* unused */, c_astretch_func, &param);
}
