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

#include "picman.h"
#include "picmancontext.h"
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-crop.h"
#include "picmanimage-guides.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmansamplepoint.h"

#include "picman-intl.h"


/*  public functions  */

void
picman_image_crop (PicmanImage   *image,
                 PicmanContext *context,
                 gint         x1,
                 gint         y1,
                 gint         x2,
                 gint         y2,
                 gboolean     crop_layers)
{
  GList *list;
  gint   width, height;
  gint   previous_width, previous_height;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  previous_width  = picman_image_get_width (image);
  previous_height = picman_image_get_height (image);

  width  = x2 - x1;
  height = y2 - y1;

  /*  Make sure new width and height are non-zero  */
  if (width < 1 || height < 1)
    return;

  picman_set_busy (image->picman);

  g_object_freeze_notify (G_OBJECT (image));

  if (crop_layers)
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_CROP,
                                 C_("undo-type", "Crop Image"));
  else
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_RESIZE,
                                 C_("undo-type", "Resize Image"));

  /*  Push the image size to the stack  */
  picman_image_undo_push_image_size (image, NULL,
                                   x1, y1, width, height);

  /*  Set the new width and height  */
  g_object_set (image,
                "width",  width,
                "height", height,
                NULL);

  /*  Resize all channels  */
  for (list = picman_image_get_channel_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_resize (item, context, width, height, -x1, -y1);
    }

  /*  Resize all vectors  */
  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_resize (item, context, width, height, -x1, -y1);
    }

  /*  Don't forget the selection mask!  */
  picman_item_resize (PICMAN_ITEM (picman_image_get_mask (image)), context,
                    width, height, -x1, -y1);

  /*  crop all layers  */
  list = picman_image_get_layer_iter (image);

  while (list)
    {
      PicmanItem *item = list->data;

      list = g_list_next (list);

      picman_item_translate (item, -x1, -y1, TRUE);

      if (crop_layers)
        {
          gint off_x, off_y;
          gint lx1, ly1, lx2, ly2;

          picman_item_get_offset (item, &off_x, &off_y);

          lx1 = CLAMP (off_x, 0, picman_image_get_width  (image));
          ly1 = CLAMP (off_y, 0, picman_image_get_height (image));
          lx2 = CLAMP (picman_item_get_width  (item) + off_x,
                       0, picman_image_get_width (image));
          ly2 = CLAMP (picman_item_get_height (item) + off_y,
                       0, picman_image_get_height (image));

          width  = lx2 - lx1;
          height = ly2 - ly1;

          if (width > 0 && height > 0)
            {
              picman_item_resize (item, context, width, height,
                                -(lx1 - off_x),
                                -(ly1 - off_y));
            }
          else
            {
              picman_image_remove_layer (image, PICMAN_LAYER (item),
                                       TRUE, NULL);
            }
        }
    }

  /*  Reposition or remove all guides  */
  list = picman_image_get_guides (image);

  while (list)
    {
      PicmanGuide *guide        = list->data;
      gboolean   remove_guide = FALSE;
      gint       position     = picman_guide_get_position (guide);

      list = g_list_next (list);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          if ((position < y1) || (position > y2))
            remove_guide = TRUE;
          else
            position -= y1;
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          if ((position < x1) || (position > x2))
            remove_guide = TRUE;
          else
            position -= x1;
          break;

        default:
          break;
        }

      if (remove_guide)
        picman_image_remove_guide (image, guide, TRUE);
      else if (position != picman_guide_get_position (guide))
        picman_image_move_guide (image, guide, position, TRUE);
    }

  /*  Reposition or remove sample points  */
  list = picman_image_get_sample_points (image);

  while (list)
    {
      PicmanSamplePoint *sample_point        = list->data;
      gboolean         remove_sample_point = FALSE;
      gint             new_x               = sample_point->x;
      gint             new_y               = sample_point->y;

      list = g_list_next (list);

      new_y -= y1;
      if ((sample_point->y < y1) || (sample_point->y > y2))
        remove_sample_point = TRUE;

      new_x -= x1;
      if ((sample_point->x < x1) || (sample_point->x > x2))
        remove_sample_point = TRUE;

      if (remove_sample_point)
        picman_image_remove_sample_point (image, sample_point, TRUE);
      else if (new_x != sample_point->x || new_y != sample_point->y)
        picman_image_move_sample_point (image, sample_point,
                                      new_x, new_y, TRUE);
    }

  picman_image_undo_group_end (image);

  picman_image_size_changed_detailed (image,
                                    -x1, -y1,
                                    previous_width, previous_height);

  g_object_thaw_notify (G_OBJECT (image));

  picman_unset_busy (image->picman);
}
