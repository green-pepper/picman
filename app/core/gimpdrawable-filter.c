/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandrawable-filter.c
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

#include "gegl/picmanapplicator.h"
#include "gegl/picman-gegl-apply-operation.h"

#include "picmandrawable.h"
#include "picmandrawable-filter.h"
#include "picmandrawable-private.h"
#include "picmandrawableundo.h"
#include "picmanfilter.h"
#include "picmanfilterstack.h"
#include "picmanimage-undo.h"
#include "picmanprogress.h"


PicmanContainer *
picman_drawable_get_filters (PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return drawable->private->filter_stack;
}

void
picman_drawable_add_filter (PicmanDrawable *drawable,
                          PicmanFilter   *filter)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (PICMAN_IS_FILTER (filter));
  g_return_if_fail (picman_drawable_has_filter (drawable, filter) == FALSE);

  picman_container_add (drawable->private->filter_stack,
                      PICMAN_OBJECT (filter));
}

void
picman_drawable_remove_filter (PicmanDrawable *drawable,
                             PicmanFilter   *filter)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (PICMAN_IS_FILTER (filter));
  g_return_if_fail (picman_drawable_has_filter (drawable, filter) == TRUE);

  picman_container_remove (drawable->private->filter_stack,
                         PICMAN_OBJECT (filter));
}

gboolean
picman_drawable_has_filter (PicmanDrawable *drawable,
                          PicmanFilter   *filter)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (PICMAN_IS_FILTER (filter), FALSE);

  return picman_container_have (drawable->private->filter_stack,
                              PICMAN_OBJECT (filter));
}

void
picman_drawable_merge_filter (PicmanDrawable *drawable,
                            PicmanFilter   *filter,
                            PicmanProgress *progress,
                            const gchar  *undo_desc)
{
  GeglRectangle rect;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (PICMAN_IS_FILTER (filter));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (picman_item_mask_intersect (PICMAN_ITEM (drawable),
                                &rect.x, &rect.y,
                                &rect.width, &rect.height))
    {
      PicmanApplicator *applicator;
      GeglBuffer     *buffer;
      GeglNode       *node;
      GeglNode       *src_node;

      picman_drawable_push_undo (drawable, undo_desc, NULL,
                               rect.x, rect.y,
                               rect.width, rect.height);

      node   = picman_filter_get_node (filter);
      buffer = picman_drawable_get_buffer (drawable);

      src_node = gegl_node_new_child (NULL,
                                      "operation", "gegl:buffer-source",
                                      "buffer",    buffer,
                                      NULL);

      gegl_node_connect_to (src_node, "output",
                            node,     "input");

      applicator = picman_filter_get_applicator (filter);

      if (applicator)
        {
          PicmanImage        *image = picman_item_get_image (PICMAN_ITEM (drawable));
          PicmanDrawableUndo *undo;

          undo = PICMAN_DRAWABLE_UNDO (picman_image_undo_get_fadeable (image));

          if (undo)
            {
              undo->paint_mode = applicator->paint_mode;
              undo->opacity    = applicator->opacity;

              undo->applied_buffer =
                picman_applicator_dup_apply_buffer (applicator, &rect);
            }
        }

      picman_gegl_apply_operation (NULL,
                                 progress, undo_desc,
                                 node,
                                 buffer,
                                 &rect);

      g_object_unref (src_node);

      picman_drawable_update (drawable,
                            rect.x, rect.y,
                            rect.width, rect.height);
    }
}
