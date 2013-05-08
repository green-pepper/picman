/* Autostretch HSV 0.10 --- image filter plug-in for PICMAN
 *
 * Copyright (C) 1997 Scott Goehring
 * Copyright (C) 1996 Federico Mena Quintero
 *
 * You can contact me at scott@poverty.bloomington.in.us
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


#define PLUG_IN_PROC "plug-in-autostretch-hsv"


/* Declare local functions.
 */
static void   query                   (void);
static void   run                     (const gchar      *name,
                                       gint              nparams,
                                       const PicmanParam  *param,
                                       gint             *nreturn_vals,
                                       PicmanParam       **return_vals);

static void   autostretch_hsv         (PicmanDrawable  *drawable);
static void   indexed_autostretch_hsv (gint32         image_ID);


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
                          N_("Stretch image contrast to cover the maximum possible range"),
                          "This simple plug-in does an automatic contrast "
                          "stretch.  For each channel in the image, it finds "
                          "the minimum and maximum values... it uses those "
                          "values to stretch the individual histograms to the "
                          "full contrast range.  For some images it may do "
                          "just what you want; for others it may be total "
                          "crap :).  This version differs from Contrast "
                          "Autostretch in that it works in HSV space, and "
                          "preserves hue.",
                          "Scott Goehring and Federico Mena Quintero",
                          "Scott Goehring and Federico Mena Quintero",
                          "1997",
                          N_("Stretch _HSV"),
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

  gint32 image_ID;

  INIT_I18N();

  run_mode = param[0].data.d_int32;

  /*  Get the specified drawable  */
  drawable = picman_drawable_get (param[2].data.d_drawable);
  image_ID = param[1].data.d_image;

  /*  Make sure that the drawable is gray or RGB color  */
  if (picman_drawable_is_rgb (drawable->drawable_id) ||
      picman_drawable_is_gray (drawable->drawable_id))
    {
      picman_progress_init (_("Auto-Stretching HSV"));
      picman_tile_cache_ntiles (2 * (drawable->width / picman_tile_width () + 1));
      autostretch_hsv (drawable);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else if (picman_drawable_is_indexed (drawable->drawable_id))
    {
      indexed_autostretch_hsv (image_ID);

      if (run_mode != PICMAN_RUN_NONINTERACTIVE)
        picman_displays_flush ();
    }
  else
    {
      /* picman_message ("autostretch_hsv: cannot operate on indexed color images"); */
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  *nreturn_vals = 1;
  *return_vals = values;

  values[0].type = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  picman_drawable_detach (drawable);
}

typedef struct {
  double shi;
  double slo;
  double vhi;
  double vlo;
} AutostretchData;

static void
find_max (guchar *src, gint bpp, AutostretchData *data)
{
  double h, s, v;

  picman_rgb_to_hsv4(src, &h, &s, &v);
  if (s > data->shi) data->shi = s;
  if (s < data->slo) data->slo = s;
  if (v > data->vhi) data->vhi = v;
  if (v < data->vlo) data->vlo = v;
}

static void
autostretch_hsv_func (guchar *src, guchar *dest, gint bpp,
                      AutostretchData *data)
{
  double h, s, v;

  picman_rgb_to_hsv4(src, &h, &s, &v);
  if (data->shi != data->slo)
    s = (s - data->slo) / (data->shi - data->slo);
  if (data->vhi != data->vlo)
    v = (v - data->vlo) / (data->vhi - data->vlo);
  picman_hsv_to_rgb4(dest, h, s, v);

  if (bpp == 4)
    dest[3] = src[3];
}

static void
indexed_autostretch_hsv (gint32 image_ID)
{
  guchar *cmap;
  AutostretchData data = {0.0, 1.0, 0.0, 1.0};
  gint ncols, i;

  cmap = picman_image_get_colormap (image_ID, &ncols);

  if (!cmap)
    {
      g_message (_("autostretch_hsv: cmap was NULL!  Quitting...\n"));
      picman_quit ();
    }

  for (i = 0; i < ncols; i++)
    {
      find_max (&cmap[i * 3], 3, &data);
    }

  for (i = 0; i < ncols; i++)
    {
      autostretch_hsv_func (&cmap[i * 3], &cmap[i * 3], 3, &data);
    }

  picman_image_set_colormap (image_ID, cmap, ncols);
}

static void
autostretch_hsv (PicmanDrawable *drawable)
{
  AutostretchData data = {0.0, 1.0, 0.0, 1.0};

  picman_rgn_iterate1 (drawable, 0 /* unused */, (PicmanRgnFunc1) find_max, &data);
  picman_rgn_iterate2 (drawable, 0 /* unused */, (PicmanRgnFunc2) autostretch_hsv_func,
                     &data);
}
