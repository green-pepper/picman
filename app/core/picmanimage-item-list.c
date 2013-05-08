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
#include "picmanimage-undo.h"
#include "picmanitem.h"
#include "picmanprogress.h"

#include "picman-intl.h"


/*  public functions  */

void
picman_image_item_list_translate (PicmanImage *image,
                                GList     *list,
                                gint       offset_x,
                                gint       offset_y,
                                gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (list)
    {
      GList *l;

      if (push_undo)
        picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_DISPLACE,
                                     C_("undo-type", "Translate Items"));

      for (l = list; l; l = g_list_next (l))
        picman_item_translate (PICMAN_ITEM (l->data),
                             offset_x, offset_y, push_undo);

      if (push_undo)
        picman_image_undo_group_end (image);
    }
}

void
picman_image_item_list_flip (PicmanImage           *image,
                           GList               *list,
                           PicmanContext         *context,
                           PicmanOrientationType  flip_type,
                           gdouble              axis,
                           gboolean             clip_result)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  if (list)
    {
      GList *l;

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                                   C_("undo-type", "Flip Items"));

      for (l = list; l; l = g_list_next (l))
        picman_item_flip (PICMAN_ITEM (l->data), context,
                        flip_type, axis, clip_result);

      picman_image_undo_group_end (image);
    }
}

void
picman_image_item_list_rotate (PicmanImage        *image,
                             GList            *list,
                             PicmanContext      *context,
                             PicmanRotationType  rotate_type,
                             gdouble           center_x,
                             gdouble           center_y,
                             gboolean          clip_result)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  if (list)
    {
      GList *l;

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                                   C_("undo-type", "Rotate Items"));

      for (l = list; l; l = g_list_next (l))
        picman_item_rotate (PICMAN_ITEM (l->data), context,
                          rotate_type, center_x, center_y, clip_result);

      picman_image_undo_group_end (image);
    }
}

void
picman_image_item_list_transform (PicmanImage              *image,
                                GList                  *list,
                                PicmanContext            *context,
                                const PicmanMatrix3      *matrix,
                                PicmanTransformDirection  direction,
                                PicmanInterpolationType   interpolation_type,
                                gint                    recursion_level,
                                PicmanTransformResize     clip_result,
                                PicmanProgress           *progress)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (list)
    {
      GList *l;

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                                   C_("undo-type", "Transform Items"));

      for (l = list; l; l = g_list_next (l))
        picman_item_transform (PICMAN_ITEM (l->data), context,
                             matrix, direction,
                             interpolation_type, recursion_level,
                             clip_result, progress);

      picman_image_undo_group_end (image);
    }
}

/**
 * picman_image_item_list_get_list:
 * @image:   An @image.
 * @exclude: An item to exclude.
 * @type:    Which type of items to return.
 * @set:     Set the returned items are part of.
 *
 * This function returns a #GList of #PicmanItem<!-- -->s for which the
 * @type and @set criterions match.
 *
 * Return value: The list of items, excluding @exclude.
 **/
GList *
picman_image_item_list_get_list (const PicmanImage  *image,
                               const PicmanItem   *exclude,
                               PicmanItemTypeMask  type,
                               PicmanItemSet       set)
{
  GList *all_items;
  GList *list;
  GList *return_list = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (exclude == NULL || PICMAN_IS_ITEM (exclude), NULL);

  if (type & PICMAN_ITEM_TYPE_LAYERS)
    {
      all_items = picman_image_get_layer_list (image);

      for (list = all_items; list; list = g_list_next (list))
        {
          PicmanItem *item = list->data;

          if (item != exclude && picman_item_is_in_set (item, set))
            return_list = g_list_prepend (return_list, item);
        }

      g_list_free (all_items);
    }

  if (type & PICMAN_ITEM_TYPE_CHANNELS)
    {
      all_items = picman_image_get_channel_list (image);

      for (list = all_items; list; list = g_list_next (list))
        {
          PicmanItem *item = list->data;

          if (item != exclude && picman_item_is_in_set (item, set))
            return_list = g_list_prepend (return_list, item);
        }

      g_list_free (all_items);
    }

  if (type & PICMAN_ITEM_TYPE_VECTORS)
    {
      all_items = picman_image_get_vectors_list (image);

      for (list = all_items; list; list = g_list_next (list))
        {
          PicmanItem *item = list->data;

          if (item != exclude && picman_item_is_in_set (item, set))
            return_list = g_list_prepend (return_list, item);
        }

      g_list_free (all_items);
    }

  return g_list_reverse (return_list);
}

static GList *
picman_image_item_list_remove_children (GList          *list,
                                      const PicmanItem *parent)
{
  GList *l = list;

  while (l)
    {
      PicmanItem *item = l->data;

      l = g_list_next (l);

      if (picman_viewable_is_ancestor (PICMAN_VIEWABLE (parent),
                                     PICMAN_VIEWABLE (item)))
        {
          list = g_list_remove (list, item);
        }
    }

  return list;
}

GList *
picman_image_item_list_filter (const PicmanItem *exclude,
                             GList          *list,
                             gboolean        remove_children,
                             gboolean        remove_locked)
{
  GList *l;

  g_return_val_if_fail (exclude == NULL || PICMAN_IS_ITEM (exclude), NULL);

  if (! list)
    return NULL;

  if (remove_children)
    {
      if (exclude)
        list = picman_image_item_list_remove_children (list, exclude);

      for (l = list; l; l = g_list_next (l))
        {
          PicmanItem *item = l->data;
          GList    *next;

          next = picman_image_item_list_remove_children (g_list_next (l), item);

          l->next = next;
          if (next)
            next->prev = l;
        }
    }

  if (remove_locked)
    {
      l = list;

      while (l)
        {
          PicmanItem *item = l->data;

          l = g_list_next (l);

          if (picman_item_is_content_locked (item))
            list = g_list_remove (list, item);
        }
    }

  return list;
}
