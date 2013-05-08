/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#include "picmandrawable.h"
#include "picmanimage.h"
#include "picmanimage-pick-color.h"
#include "picmanpickable.h"


gboolean
picman_image_pick_color (PicmanImage     *image,
                       PicmanDrawable  *drawable,
                       gint           x,
                       gint           y,
                       gboolean       sample_merged,
                       gboolean       sample_average,
                       gdouble        average_radius,
                       const Babl   **sample_format,
                       PicmanRGB       *color,
                       gint          *color_index)
{
  PicmanPickable *pickable;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (drawable == NULL || PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (drawable == NULL ||
                        picman_item_get_image (PICMAN_ITEM (drawable)) == image,
                        FALSE);

  if (! sample_merged)
    {
      if (! drawable)
        drawable = picman_image_get_active_drawable (image);

      if (! drawable)
        return FALSE;
    }

  if (sample_merged)
    {
      pickable = PICMAN_PICKABLE (picman_image_get_projection (image));
    }
  else
    {
      gint off_x, off_y;

      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);
      x -= off_x;
      y -= off_y;

      pickable = PICMAN_PICKABLE (drawable);
    }

  /* Do *not* call picman_pickable_flush() here because it's too expensive
   * to call it unconditionally each time e.g. the cursor view is updated.
   * Instead, call picman_pickable_flush() in the callers if needed.
   */

  if (sample_format)
    *sample_format = picman_pickable_get_format (pickable);

  return picman_pickable_pick_color (pickable, x, y,
                                   sample_average, average_radius,
                                   color, color_index);
}
