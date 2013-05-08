/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandrawable-operation.c
 * Copyright (C) 2007 Øyvind Kolås <pippin@picman.org>
 *                    Sven Neumann <sven@picman.org>
 *                    Michael Natterer <mitch@picman.org>
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

#include "gegl/picman-gegl-apply-operation.h"

#include "picmandrawable.h"
#include "picmandrawable-operation.h"
#include "picmandrawable-shadow.h"
#include "picmanimagemapconfig.h"
#include "picmanprogress.h"


/*  public functions  */

void
picman_drawable_apply_operation (PicmanDrawable *drawable,
                               PicmanProgress *progress,
                               const gchar  *undo_desc,
                               GeglNode     *operation)
{
  GeglBuffer    *dest_buffer;
  GeglRectangle  rect;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (GEGL_IS_NODE (operation));

  if (! picman_item_mask_intersect (PICMAN_ITEM (drawable),
                                  &rect.x,     &rect.y,
                                  &rect.width, &rect.height))
    return;

  dest_buffer = picman_drawable_get_shadow_buffer (drawable);

  picman_gegl_apply_operation (picman_drawable_get_buffer (drawable),
                             progress, undo_desc,
                             operation,
                             dest_buffer, &rect);

  picman_drawable_merge_shadow_buffer (drawable, TRUE, undo_desc);
  picman_drawable_free_shadow_buffer (drawable);

  picman_drawable_update (drawable, rect.x, rect.y, rect.width, rect.height);

  if (progress)
    picman_progress_end (progress);
}

void
picman_drawable_apply_operation_by_name (PicmanDrawable *drawable,
                                       PicmanProgress *progress,
                                       const gchar  *undo_desc,
                                       const gchar  *operation_type,
                                       GObject      *config)
{
  GeglNode *node;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (operation_type != NULL);
  g_return_if_fail (config == NULL || PICMAN_IS_IMAGE_MAP_CONFIG (config));

  node = g_object_new (GEGL_TYPE_NODE,
                       "operation", operation_type,
                       NULL);

  if (config)
    gegl_node_set (node,
                   "config", config,
                   NULL);

  picman_drawable_apply_operation (drawable, progress, undo_desc, node);

  g_object_unref (node);
}
