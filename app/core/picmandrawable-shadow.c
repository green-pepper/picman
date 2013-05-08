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

#include "gegl/picman-gegl-utils.h"

#include "picmandrawable.h"
#include "picmandrawable-private.h"
#include "picmandrawable-shadow.h"


GeglBuffer *
picman_drawable_get_shadow_buffer (PicmanDrawable *drawable)
{
  PicmanItem   *item;
  gint        width;
  gint        height;
  const Babl *format;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  item = PICMAN_ITEM (drawable);

  width  = picman_item_get_width  (item);
  height = picman_item_get_height (item);
  format = picman_drawable_get_format (drawable);

  if (drawable->private->shadow)
    {
      if ((width  != gegl_buffer_get_width  (drawable->private->shadow)) ||
          (height != gegl_buffer_get_height (drawable->private->shadow)) ||
          (format != gegl_buffer_get_format (drawable->private->shadow)))
        {
          picman_drawable_free_shadow_buffer (drawable);
        }
      else
        {
          return drawable->private->shadow;
        }
    }

  drawable->private->shadow = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                               width, height),
                                               format);

  return drawable->private->shadow;
}

void
picman_drawable_free_shadow_buffer (PicmanDrawable *drawable)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  if (drawable->private->shadow)
    {
      g_object_unref (drawable->private->shadow);
      drawable->private->shadow = NULL;
    }
}

void
picman_drawable_merge_shadow_buffer (PicmanDrawable *drawable,
                                   gboolean      push_undo,
                                   const gchar  *undo_desc)
{
  gint x, y;
  gint width, height;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (GEGL_IS_BUFFER (drawable->private->shadow));

  /*  A useful optimization here is to limit the update to the
   *  extents of the selection mask, as it cannot extend beyond
   *  them.
   */
  if (picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &width, &height))
    {
      GeglBuffer *buffer = g_object_ref (drawable->private->shadow);

      picman_drawable_apply_buffer (drawable, buffer,
                                  GEGL_RECTANGLE (x, y, width, height),
                                  push_undo, undo_desc,
                                  PICMAN_OPACITY_OPAQUE, PICMAN_REPLACE_MODE,
                                  NULL, x, y);

      g_object_unref (buffer);
    }
}
