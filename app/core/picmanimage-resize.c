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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picman.h"
#include "picmancontainer.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-guides.h"
#include "picmanimage-item-list.h"
#include "picmanimage-resize.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanprogress.h"
#include "picmansamplepoint.h"

#include "text/picmantextlayer.h"

#include "picman-intl.h"


void
picman_image_resize (PicmanImage    *image,
                   PicmanContext  *context,
                   gint          new_width,
                   gint          new_height,
                   gint          offset_x,
                   gint          offset_y,
                   PicmanProgress *progress)
{
  picman_image_resize_with_layers (image, context,
                                 new_width, new_height, offset_x, offset_y,
                                 PICMAN_ITEM_SET_NONE, TRUE,
                                 progress);
}

void
picman_image_resize_with_layers (PicmanImage    *image,
                               PicmanContext  *context,
                               gint          new_width,
                               gint          new_height,
                               gint          offset_x,
                               gint          offset_y,
                               PicmanItemSet   layer_set,
                               gboolean      resize_text_layers,
                               PicmanProgress *progress)
{
  GList   *list;
  GList   *resize_layers;
  gdouble  progress_max;
  gdouble  progress_current = 1.0;
  gint     old_width, old_height;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (new_width > 0 && new_height > 0);
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  picman_set_busy (image->picman);

  g_object_freeze_notify (G_OBJECT (image));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_RESIZE,
                               C_("undo-type", "Resize Image"));

  resize_layers = picman_image_item_list_get_list (image, NULL,
                                                 PICMAN_ITEM_TYPE_LAYERS,
                                                 layer_set);

  progress_max = (picman_container_get_n_children (picman_image_get_layers (image))   +
                  picman_container_get_n_children (picman_image_get_channels (image)) +
                  picman_container_get_n_children (picman_image_get_vectors (image))  +
                  g_list_length (resize_layers)                                   +
                  1 /* selection */);

  old_width  = picman_image_get_width  (image);
  old_height = picman_image_get_height (image);

  /*  Push the image size to the stack  */
  picman_image_undo_push_image_size (image, NULL,
                                   -offset_x, -offset_y,
                                   new_width, new_height);

  /*  Set the new width and height  */
  g_object_set (image,
                "width",  new_width,
                "height", new_height,
                NULL);

  /*  Resize all channels  */
  for (list = picman_image_get_channel_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_resize (item, context,
                        new_width, new_height, offset_x, offset_y);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Resize all vectors  */
  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_resize (item, context,
                        new_width, new_height, offset_x, offset_y);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Don't forget the selection mask!  */
  picman_item_resize (PICMAN_ITEM (picman_image_get_mask (image)), context,
                    new_width, new_height, offset_x, offset_y);

  if (progress)
    picman_progress_set_value (progress, progress_current++ / progress_max);

  /*  Reposition all layers  */
  for (list = picman_image_get_layer_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_translate (item, offset_x, offset_y, TRUE);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Resize all resize_layers to image size  */
  for (list = resize_layers; list; list = g_list_next (list))
    {
      PicmanItem *item = list->data;
      gint      old_offset_x;
      gint      old_offset_y;

      /*  group layers can't be resized here  */
      if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
        continue;

      if (! resize_text_layers && picman_item_is_text_layer (item))
        continue;

      picman_item_get_offset (item, &old_offset_x, &old_offset_y);

      picman_item_resize (item, context,
                        new_width, new_height,
                        old_offset_x, old_offset_y);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  g_list_free (resize_layers);

  /*  Reposition or remove all guides  */
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      PicmanGuide *guide        = list->data;
      gboolean   remove_guide = FALSE;
      gint       new_position = picman_guide_get_position (guide);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          new_position += offset_y;
          if (new_position < 0 || new_position > new_height)
            remove_guide = TRUE;
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          new_position += offset_x;
          if (new_position < 0 || new_position > new_width)
            remove_guide = TRUE;
          break;

        default:
          break;
        }

      if (remove_guide)
        picman_image_remove_guide (image, guide, TRUE);
      else if (new_position != picman_guide_get_position (guide))
        picman_image_move_guide (image, guide, new_position, TRUE);
    }

  /*  Reposition or remove sample points  */
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point        = list->data;
      gboolean         remove_sample_point = FALSE;
      gint             new_x               = sample_point->x;
      gint             new_y               = sample_point->y;

      new_y += offset_y;
      if ((sample_point->y < 0) || (sample_point->y > new_height))
        remove_sample_point = TRUE;

      new_x += offset_x;
      if ((sample_point->x < 0) || (sample_point->x > new_width))
        remove_sample_point = TRUE;

      if (remove_sample_point)
        picman_image_remove_sample_point (image, sample_point, TRUE);
      else if (new_x != sample_point->x || new_y != sample_point->y)
        picman_image_move_sample_point (image, sample_point,
                                      new_x, new_y, TRUE);
    }

  picman_image_undo_group_end (image);

  picman_image_size_changed_detailed (image,
                                    offset_x, offset_y,
                                    old_width, old_height);

  g_object_thaw_notify (G_OBJECT (image));

  picman_unset_busy (image->picman);
}

void
picman_image_resize_to_layers (PicmanImage    *image,
                             PicmanContext  *context,
                             PicmanProgress *progress)
{
  GList    *list;
  PicmanItem *item;
  gint      x, y;
  gint      width, height;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  list = picman_image_get_layer_iter (image);

  if (! list)
    return;

  /* figure out starting values */
  item = list->data;

  x      = picman_item_get_offset_x (item);
  y      = picman_item_get_offset_y (item);
  width  = picman_item_get_width  (item);
  height = picman_item_get_height (item);

  /*  Respect all layers  */
  for (list = g_list_next (list); list; list = g_list_next (list))
    {
      item = list->data;

      picman_rectangle_union (x, y,
                            width, height,
                            picman_item_get_offset_x (item),
                            picman_item_get_offset_y (item),
                            picman_item_get_width  (item),
                            picman_item_get_height (item),
                            &x, &y,
                            &width, &height);
    }

  picman_image_resize (image, context,
                     width, height, -x, -y,
                     progress);
}

void
picman_image_resize_to_selection (PicmanImage    *image,
                                PicmanContext  *context,
                                PicmanProgress *progress)
{
  PicmanChannel *selection = picman_image_get_mask (image);
  gint         x1, y1;
  gint         x2, y2;

  if (picman_channel_is_empty (selection))
    return;

  picman_channel_bounds (selection, &x1, &y1, &x2, &y2);

  picman_image_resize (image, context,
                     x2 - x1, y2 - y1,
                     - x1, - y1,
                     progress);
}
