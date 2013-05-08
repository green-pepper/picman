/*
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This is a plug-in for PICMAN.
 *
 * Colormap-Rotation plug-in. Exchanges two color ranges.
 *
 * Copyright (C) 1999 Sven Anders (anderss@fmi.uni-passau.de)
 *                    Based on code from Pavel Grinfeld (pavel@ml.com)
 *
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

/*----------------------------------------------------------------------------
 * Change log:
 *
 * Version 2.0, 04 April 1999.
 *  Nearly complete rewrite, made plug-in stable.
 *  (Works with PICMAN 1.1 and GTK+ 1.2)
 *
 * Version 1.0, 27 March 1997.
 *  Initial (unstable) release by Pavel Grinfeld
 *
 *----------------------------------------------------------------------------*/

#include "config.h"

#include "libpicman/picman.h"
#include "libpicman/picmanui.h"

#include "color-rotate.h"
#include "color-rotate-utils.h"
#include "color-rotate-dialog.h"
#include "color-rotate-callbacks.h"

#include "libpicman/stdplugins-intl.h"


/* Forward declarations */

static void  query        (void);
static void  run          (const gchar      *name,
                           gint              nparams,
                           const PicmanParam  *param,
                           gint             *nreturn_vals,
                           PicmanParam       **return_vals);

static void  color_rotate (PicmanDrawable     *drawable);


/* Global variables */

RcmParams Current =
{
  SELECTION,           /* SELECTION ONLY */
  TRUE,                /* REAL TIME */
  DEGREES,             /* START IN DEGREES */
  GRAY_TO
};

const PicmanPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};


MAIN()


/* Query plug-in */

static void
query (void)
{
  PicmanParamDef args[] =
  {
    { PICMAN_PDB_INT32,    "run-mode", "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }"          },
    { PICMAN_PDB_IMAGE,    "image",    "Input image (used for indexed images)" },
    { PICMAN_PDB_DRAWABLE, "drawable", "Input drawable"                        },
  };

  picman_install_procedure (PLUG_IN_PROC,
			  N_("Replace a range of colors with another"),
			  "Exchanges two color ranges. "
                          "Based on code from Pavel Grinfeld (pavel@ml.com). "
                          "This version written by Sven Anders (anderss@fmi.uni-passau.de).",
			  "Sven Anders (anderss@fmi.uni-passau.de) and Pavel Grinfeld (pavel@ml.com)",
			  "Sven Anders (anderss@fmi.uni-passau.de)",
			  "04th April 1999",
			  N_("_Rotate Colors..."),
			  "RGB*",
			  PICMAN_PLUGIN,
			  G_N_ELEMENTS (args), 0,
			  args, NULL);

  picman_plugin_menu_register (PLUG_IN_PROC, "<Image>/Colors/Map");
}

static void
run (const gchar      *name,
     gint              nparams,
     const PicmanParam  *param,
     gint             *nreturn_vals,
     PicmanParam       **return_vals)
{
  static PicmanParam  values[1];
  PicmanPDBStatusType status = PICMAN_PDB_SUCCESS;

  *nreturn_vals = 1;
  *return_vals  = values;

  INIT_I18N ();

  values[0].type          = PICMAN_PDB_STATUS;
  values[0].data.d_status = status;

  Current.drawable = picman_drawable_get (param[2].data.d_drawable);
  Current.mask = picman_drawable_get (picman_image_get_selection (param[1].data.d_image));

  if (picman_drawable_is_rgb (Current.drawable->drawable_id))
    {
      if (color_rotate_dialog ())
        {
          picman_progress_init (_("Rotating the colors"));

          picman_tile_cache_ntiles (2 * (Current.drawable->width /
                                       picman_tile_width () + 1));
          color_rotate (Current.drawable);
          picman_displays_flush ();
        }
      else
        {
          status = PICMAN_PDB_CANCEL;
        }
    }
  else
    {
      status = PICMAN_PDB_EXECUTION_ERROR;
    }

  values[0].data.d_status = status;

  if (status == PICMAN_PDB_SUCCESS)
    picman_drawable_detach (Current.drawable);
}


/* Rotate colors of a single row */

static void
color_rotate_row (const guchar *src_row,
                  guchar       *dest_row,
                  gint          row,
                  gint          row_width,
                  gint          bytes)
{
  gint     col, bytenum;
  gdouble  H, S, V;
  guchar   rgb[3];

  for (col = 0; col < row_width; col++)
    {
      gboolean skip = FALSE;

      rgb[0] = src_row[col * bytes + 0];
      rgb[1] = src_row[col * bytes + 1];
      rgb[2] = src_row[col * bytes + 2];

      picman_rgb_to_hsv4 (rgb, &H, &S, &V);

      if (rcm_is_gray (S))
        {
          if (Current.Gray_to_from == GRAY_FROM)
            {
              if (rcm_angle_inside_slice (Current.Gray->hue,
                                          Current.From->angle) <= 1)
                {
                  H = Current.Gray->hue / TP;
                  S = Current.Gray->satur;
                }
              else
                {
                  skip = TRUE;
                }
            }
          else
            {
              skip = TRUE;
              picman_hsv_to_rgb4 (rgb, Current.Gray->hue / TP,
                                Current.Gray->satur, V);
            }
        }

      if (! skip)
        {
          H = rcm_linear (rcm_left_end (Current.From->angle),
                          rcm_right_end (Current.From->angle),
                          rcm_left_end (Current.To->angle),
                          rcm_right_end (Current.To->angle),
                          H * TP);

          H = angle_mod_2PI (H) / TP;
          picman_hsv_to_rgb4 (rgb, H, S, V);
        }

      dest_row[col * bytes + 0] = rgb[0];
      dest_row[col * bytes + 1] = rgb[1];
      dest_row[col * bytes + 2] = rgb[2];

      if (bytes > 3)
        {
          for (bytenum = 3; bytenum < bytes; bytenum++)
            dest_row[col * bytes + bytenum] = src_row[col * bytes + bytenum];
        }
    }
}


/* Rotate colors row by row ... */

static void
color_rotate (PicmanDrawable *drawable)
{
  PicmanPixelRgn srcPR, destPR;
  gint         width, height;
  gint         bytes;
  guchar      *src_row, *dest_row;
  gint         row;
  gint         x, y;

  if (! picman_drawable_mask_intersect (drawable->drawable_id,
                                      &x, &y, &width, &height))
    {
      return;
    }

  bytes = drawable->bpp;

  src_row  = g_new (guchar, width * bytes);
  dest_row = g_new (guchar, width * bytes);

  picman_pixel_rgn_init (&srcPR, drawable, 0, 0,
                       drawable->width,
                       drawable->height, FALSE, FALSE);
  picman_pixel_rgn_init (&destPR, drawable, 0, 0,
                       drawable->width,
                       drawable->height, TRUE, TRUE);

  for (row = y; row < (y + height); row++)
    {
      picman_pixel_rgn_get_row (&srcPR, src_row, x, row, width);

      color_rotate_row (src_row, dest_row, row, width, bytes);

      picman_pixel_rgn_set_row (&destPR, dest_row, x, row, width);

      if ((row % 10) == 0)
        picman_progress_update ((double) row / (double) height);
    }

  /*  update the processed region  */

  picman_progress_update (1.0);
  picman_drawable_flush (drawable);
  picman_drawable_merge_shadow (drawable->drawable_id, TRUE);
  picman_drawable_update (drawable->drawable_id, x, y, width, height);

  g_free (src_row);
  g_free (dest_row);
}
