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

#include "config.h"

#include <gegl.h>

#include "core-types.h"

#include "gegl/picman-babl.h"

#include "picmanimage.h"
#include "picmanimage-preview.h"
#include "picmanimage-private.h"
#include "picmanpickable.h"
#include "picmanprojectable.h"
#include "picmanprojection.h"
#include "picmantempbuf.h"


void
picman_image_get_preview_size (PicmanViewable *viewable,
                             gint          size,
                             gboolean      is_popup,
                             gboolean      dot_for_dot,
                             gint         *width,
                             gint         *height)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);
  gdouble    xres;
  gdouble    yres;

  picman_image_get_resolution (image, &xres, &yres);

  picman_viewable_calc_preview_size (picman_image_get_width  (image),
                                   picman_image_get_height (image),
                                   size,
                                   size,
                                   dot_for_dot,
                                   xres,
                                   yres,
                                   width,
                                   height,
                                   NULL);
}

gboolean
picman_image_get_popup_size (PicmanViewable *viewable,
                           gint          width,
                           gint          height,
                           gboolean      dot_for_dot,
                           gint         *popup_width,
                           gint         *popup_height)
{
  PicmanImage *image = PICMAN_IMAGE (viewable);

  if (picman_image_get_width  (image) > width ||
      picman_image_get_height (image) > height)
    {
      gboolean scaling_up;

      picman_viewable_calc_preview_size (picman_image_get_width  (image),
                                       picman_image_get_height (image),
                                       width  * 2,
                                       height * 2,
                                       dot_for_dot, 1.0, 1.0,
                                       popup_width,
                                       popup_height,
                                       &scaling_up);

      if (scaling_up)
        {
          *popup_width  = picman_image_get_width  (image);
          *popup_height = picman_image_get_height (image);
        }

      return TRUE;
    }

  return FALSE;
}

PicmanTempBuf *
picman_image_get_new_preview (PicmanViewable *viewable,
                            PicmanContext  *context,
                            gint          width,
                            gint          height)
{
  PicmanImage      *image      = PICMAN_IMAGE (viewable);
  PicmanProjection *projection = picman_image_get_projection (image);
  const Babl     *format;
  PicmanTempBuf    *buf;
  gdouble         scale_x;
  gdouble         scale_y;

  scale_x = (gdouble) width  / (gdouble) picman_image_get_width  (image);
  scale_y = (gdouble) height / (gdouble) picman_image_get_height (image);

  format = picman_projectable_get_format (PICMAN_PROJECTABLE (image));
  format = picman_babl_format (picman_babl_format_get_base_type (format),
                             PICMAN_PRECISION_U8,
                             babl_format_has_alpha (format));

  buf = picman_temp_buf_new (width, height, format);

  gegl_buffer_get (picman_pickable_get_buffer (PICMAN_PICKABLE (projection)),
                   GEGL_RECTANGLE (0, 0, width, height),
                   MIN (scale_x, scale_y),
                   picman_temp_buf_get_format (buf),
                   picman_temp_buf_get_data (buf),
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  return buf;
}
