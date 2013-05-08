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

#include <string.h>

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picmanchannel.h"
#include "picmanimage.h"
#include "picmandrawable-preview.h"
#include "picmandrawable-private.h"
#include "picmanlayer.h"
#include "picmantempbuf.h"


/*  public functions  */

PicmanTempBuf *
picman_drawable_get_new_preview (PicmanViewable *viewable,
                               PicmanContext  *context,
                               gint          width,
                               gint          height)
{
  PicmanItem  *item  = PICMAN_ITEM (viewable);
  PicmanImage *image = picman_item_get_image (item);

  if (! image->picman->config->layer_previews)
    return NULL;

  return picman_drawable_get_sub_preview (PICMAN_DRAWABLE (viewable),
                                        0, 0,
                                        picman_item_get_width  (item),
                                        picman_item_get_height (item),
                                        width,
                                        height);
}

const Babl *
picman_drawable_get_preview_format (PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  switch (picman_drawable_get_base_type (drawable))
    {
    case PICMAN_GRAY:
      if (picman_drawable_has_alpha (drawable))
        return babl_format ("Y'A u8");
      else
        return babl_format ("Y' u8");

    case PICMAN_RGB:
    case PICMAN_INDEXED:
      if (picman_drawable_has_alpha (drawable))
        return babl_format ("R'G'B'A u8");
      else
        return babl_format ("R'G'B' u8");
    }

  g_return_val_if_reached (NULL);
}

PicmanTempBuf *
picman_drawable_get_sub_preview (PicmanDrawable *drawable,
                               gint          src_x,
                               gint          src_y,
                               gint          src_width,
                               gint          src_height,
                               gint          dest_width,
                               gint          dest_height)
{
  PicmanItem    *item;
  PicmanImage   *image;
  GeglBuffer  *buffer;
  PicmanTempBuf *preview;
  gdouble      scale;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (src_x >= 0, NULL);
  g_return_val_if_fail (src_y >= 0, NULL);
  g_return_val_if_fail (src_width  > 0, NULL);
  g_return_val_if_fail (src_height > 0, NULL);
  g_return_val_if_fail (dest_width  > 0, NULL);
  g_return_val_if_fail (dest_height > 0, NULL);

  item = PICMAN_ITEM (drawable);

  g_return_val_if_fail ((src_x + src_width)  <= picman_item_get_width  (item), NULL);
  g_return_val_if_fail ((src_y + src_height) <= picman_item_get_height (item), NULL);

  image = picman_item_get_image (item);

  if (! image->picman->config->layer_previews)
    return NULL;

  buffer = picman_drawable_get_buffer (drawable);

  preview = picman_temp_buf_new (dest_width, dest_height,
                               picman_drawable_get_preview_format (drawable));

  scale = MIN ((gdouble) dest_width  / (gdouble) gegl_buffer_get_width  (buffer),
               (gdouble) dest_height / (gdouble) gegl_buffer_get_height (buffer));

  gegl_buffer_get (buffer,
                   GEGL_RECTANGLE (src_x, src_y, dest_width, dest_height),
                   scale,
                   picman_temp_buf_get_format (preview),
                   picman_temp_buf_get_data (preview),
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  return preview;
}
