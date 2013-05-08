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

#include "picmancontext.h"
#include "picmanimage.h"
#include "picmanimage-item-list.h"
#include "picmanitem.h"
#include "picmanitem-linked.h"
#include "picmanlist.h"
#include "picmanprogress.h"


/*  public functions  */

gboolean
picman_item_linked_is_locked (const PicmanItem *item)
{
  GList    *list;
  GList    *l;
  gboolean  locked = FALSE;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), FALSE);
  g_return_val_if_fail (picman_item_get_linked (item) == TRUE, FALSE);
  g_return_val_if_fail (picman_item_is_attached (item), FALSE);

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_ALL,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  for (l = list; l && ! locked; l = g_list_next (l))
    {
      PicmanItem *item = l->data;

      /*  temporarily set the item to not being linked, or we will
       *  run into a recursion because picman_item_is_position_locked()
       *  call this function if the item is linked
       */
      picman_item_set_linked (item, FALSE, FALSE);

      if (picman_item_is_position_locked (item))
        locked = TRUE;

      picman_item_set_linked (item, TRUE, FALSE);
    }

  g_list_free (list);

  return locked;
}

void
picman_item_linked_translate (PicmanItem *item,
                            gint      offset_x,
                            gint      offset_y,
                            gboolean  push_undo)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (picman_item_get_linked (item) == TRUE);
  g_return_if_fail (picman_item_is_attached (item));

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_ALL,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  picman_image_item_list_translate (picman_item_get_image (item), list,
                                  offset_x, offset_y, push_undo);

  g_list_free (list);
}

void
picman_item_linked_flip (PicmanItem            *item,
                       PicmanContext         *context,
                       PicmanOrientationType  flip_type,
                       gdouble              axis,
                       gboolean             clip_result)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (picman_item_get_linked (item) == TRUE);
  g_return_if_fail (picman_item_is_attached (item));

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_ALL,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  picman_image_item_list_flip (picman_item_get_image (item), list, context,
                             flip_type, axis, clip_result);

  g_list_free (list);
}

void
picman_item_linked_rotate (PicmanItem         *item,
                         PicmanContext      *context,
                         PicmanRotationType  rotate_type,
                         gdouble           center_x,
                         gdouble           center_y,
                         gboolean          clip_result)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (picman_item_get_linked (item) == TRUE);
  g_return_if_fail (picman_item_is_attached (item));

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_LAYERS |
                                        PICMAN_ITEM_TYPE_VECTORS,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  picman_image_item_list_rotate (picman_item_get_image (item), list, context,
                               rotate_type, center_x, center_y, clip_result);

  g_list_free (list);

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_CHANNELS,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  picman_image_item_list_rotate (picman_item_get_image (item), list, context,
                               rotate_type, center_x, center_y, TRUE);

  g_list_free (list);
}

void
picman_item_linked_transform (PicmanItem               *item,
                            PicmanContext            *context,
                            const PicmanMatrix3      *matrix,
                            PicmanTransformDirection  direction,
                            PicmanInterpolationType   interpolation_type,
                            gint                    recursion_level,
                            PicmanTransformResize     clip_result,
                            PicmanProgress           *progress)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_ITEM (item));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (picman_item_get_linked (item) == TRUE);
  g_return_if_fail (picman_item_is_attached (item));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  list = picman_image_item_list_get_list (picman_item_get_image (item), item,
                                        PICMAN_ITEM_TYPE_ALL,
                                        PICMAN_ITEM_SET_LINKED);

  list = picman_image_item_list_filter (item, list, TRUE, FALSE);

  picman_image_item_list_transform (picman_item_get_image (item), list, context,
                                  matrix, direction,
                                  interpolation_type, recursion_level,
                                  clip_result, progress);

  g_list_free (list);
}
