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
#include "picmancontainer.h"
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-guides.h"
#include "picmanimage-item-list.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-scale.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanprogress.h"
#include "picmanprojection.h"
#include "picmansamplepoint.h"
#include "picmansubprogress.h"

#include "picman-log.h"
#include "picman-intl.h"


void
picman_image_scale (PicmanImage             *image,
                  gint                   new_width,
                  gint                   new_height,
                  PicmanInterpolationType  interpolation_type,
                  PicmanProgress          *progress)
{
  PicmanProgress *sub_progress;
  GList        *all_layers;
  GList        *all_channels;
  GList        *all_vectors;
  GList        *list;
  gint          old_width;
  gint          old_height;
  gint          offset_x;
  gint          offset_y;
  gdouble       img_scale_w      = 1.0;
  gdouble       img_scale_h      = 1.0;
  gint          progress_steps;
  gint          progress_current = 0;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (new_width > 0 && new_height > 0);
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  picman_set_busy (image->picman);

  sub_progress = picman_sub_progress_new (progress);

  all_layers   = picman_image_get_layer_list (image);
  all_channels = picman_image_get_channel_list (image);
  all_vectors  = picman_image_get_vectors_list (image);

  progress_steps = (g_list_length (all_layers)   +
                    g_list_length (all_channels) +
                    g_list_length (all_vectors)  +
                    1 /* selection */);

  g_object_freeze_notify (G_OBJECT (image));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_SCALE,
                               C_("undo-type", "Scale Image"));

  old_width   = picman_image_get_width  (image);
  old_height  = picman_image_get_height (image);
  img_scale_w = (gdouble) new_width  / (gdouble) old_width;
  img_scale_h = (gdouble) new_height / (gdouble) old_height;

  offset_x = (old_width  - new_width)  / 2;
  offset_y = (old_height - new_height) / 2;

  /*  Push the image size to the stack  */
  picman_image_undo_push_image_size (image,
                                   NULL,
                                   offset_x,
                                   offset_y,
                                   new_width,
                                   new_height);

  /*  Set the new width and height early, so below image item setters
   *  (esp. guides and sample points) don't choke about moving stuff
   *  out of the image
   */
  g_object_set (image,
                "width",  new_width,
                "height", new_height,
                NULL);

  /*  Scale all channels  */
  for (list = all_channels; list; list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (sub_progress),
                                  progress_current++, progress_steps);

      picman_item_scale (item,
                       new_width, new_height, 0, 0,
                       interpolation_type, sub_progress);
    }

  /*  Scale all vectors  */
  for (list = all_vectors; list; list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (sub_progress),
                                  progress_current++, progress_steps);

      picman_item_scale (item,
                       new_width, new_height, 0, 0,
                       interpolation_type, sub_progress);
    }

  /*  Don't forget the selection mask!  */
  picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (sub_progress),
                              progress_current++, progress_steps);

  picman_item_scale (PICMAN_ITEM (picman_image_get_mask (image)),
                   new_width, new_height, 0, 0,
                   interpolation_type, sub_progress);

  /*  Scale all layers  */
  for (list = all_layers; list; list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_sub_progress_set_step (PICMAN_SUB_PROGRESS (sub_progress),
                                  progress_current++, progress_steps);

      /*  group layers are updated automatically  */
      if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
        continue;

      if (! picman_item_scale_by_factors (item,
                                        img_scale_w, img_scale_h,
                                        interpolation_type, sub_progress))
        {
          /* Since 0 < img_scale_w, img_scale_h, failure due to one or more
           * vanishing scaled layer dimensions. Implicit delete implemented
           * here. Upstream warning implemented in resize_check_layer_scaling(),
           * which offers the user the chance to bail out.
           */
          picman_image_remove_layer (image, PICMAN_LAYER (item), TRUE, NULL);
        }
    }

  /*  Scale all Guides  */
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      PicmanGuide *guide    = list->data;
      gint       position = picman_guide_get_position (guide);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          picman_image_move_guide (image, guide,
                                 (position * new_height) / old_height,
                                 TRUE);
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          picman_image_move_guide (image, guide,
                                 (position * new_width) / old_width,
                                 TRUE);
          break;

        default:
          break;
        }
    }

  /*  Scale all sample points  */
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;

      picman_image_move_sample_point (image, sample_point,
                                    sample_point->x * new_width  / old_width,
                                    sample_point->y * new_height / old_height,
                                    TRUE);
    }

  picman_image_undo_group_end (image);

  g_list_free (all_layers);
  g_list_free (all_channels);
  g_list_free (all_vectors);

  g_object_unref (sub_progress);

  picman_image_size_changed_detailed (image,
                                    -offset_x,
                                    -offset_y,
                                    old_width,
                                    old_height);

  g_object_thaw_notify (G_OBJECT (image));

  picman_unset_busy (image->picman);
}

/**
 * picman_image_scale_check:
 * @image:      A #PicmanImage.
 * @new_width:   The new width.
 * @new_height:  The new height.
 * @max_memsize: The maximum new memory size.
 * @new_memsize: The new memory size.
 *
 * Inventory the layer list in image and check that it may be
 * scaled to @new_height and @new_width without problems.
 *
 * Return value: #PICMAN_IMAGE_SCALE_OK if scaling the image will shrink none
 *               of its layers completely away, and the new image size
 *               is smaller than @max_memsize.
 *               #PICMAN_IMAGE_SCALE_TOO_SMALL if scaling would remove some
 *               existing layers.
 *               #PICMAN_IMAGE_SCALE_TOO_BIG if the new image size would
 *               exceed the maximum specified in the preferences.
 **/
PicmanImageScaleCheckType
picman_image_scale_check (const PicmanImage *image,
                        gint             new_width,
                        gint             new_height,
                        gint64           max_memsize,
                        gint64          *new_memsize)
{
  GList  *drawables;
  GList  *all_layers;
  GList  *list;
  gint64  current_size;
  gint64  scalable_size;
  gint64  scaled_size;
  gint64  undo_size;
  gint64  redo_size;
  gint64  fixed_size;
  gint64  new_size;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), PICMAN_IMAGE_SCALE_TOO_SMALL);
  g_return_val_if_fail (new_memsize != NULL, PICMAN_IMAGE_SCALE_TOO_SMALL);

  current_size = picman_object_get_memsize (PICMAN_OBJECT (image), NULL);

  /*  the part of the image's memsize that scales linearly with the image  */
  drawables = picman_image_item_list_get_list (image, NULL,
                                             PICMAN_ITEM_TYPE_LAYERS |
                                             PICMAN_ITEM_TYPE_CHANNELS,
                                             PICMAN_ITEM_SET_ALL);

  picman_image_item_list_filter (NULL, drawables, TRUE, FALSE);

  drawables = g_list_prepend (drawables, picman_image_get_mask (image));

  scalable_size = 0;
  scaled_size   = 0;

  for (list = drawables; list; list = g_list_next (list))
    {
      PicmanDrawable *drawable = list->data;
      gdouble       width    = picman_item_get_width  (PICMAN_ITEM (drawable));
      gdouble       height   = picman_item_get_height (PICMAN_ITEM (drawable));

      scalable_size +=
        picman_drawable_estimate_memsize (drawable,
                                        width, height);

      scaled_size +=
        picman_drawable_estimate_memsize (drawable,
                                        width * new_width /
                                        picman_image_get_width (image),
                                        height * new_height /
                                        picman_image_get_height (image));
    }

  g_list_free (drawables);

  scalable_size +=
    picman_projection_estimate_memsize (picman_image_get_base_type (image),
                                      picman_image_get_precision (image),
                                      picman_image_get_width (image),
                                      picman_image_get_height (image));

  scaled_size +=
    picman_projection_estimate_memsize (picman_image_get_base_type (image),
                                      picman_image_get_precision (image),
                                      new_width, new_height);

  PICMAN_LOG (IMAGE_SCALE,
            "scalable_size = %"G_GINT64_FORMAT"  scaled_size = %"G_GINT64_FORMAT,
            scalable_size, scaled_size);

  undo_size = picman_object_get_memsize (PICMAN_OBJECT (picman_image_get_undo_stack (image)), NULL);
  redo_size = picman_object_get_memsize (PICMAN_OBJECT (picman_image_get_redo_stack (image)), NULL);

  /*  the fixed part of the image's memsize w/o any undo information  */
  fixed_size = current_size - undo_size - redo_size - scalable_size;

  /*  calculate the new size, which is:  */
  new_size = (fixed_size +   /*  the fixed part                */
              scaled_size);  /*  plus the part that scales...  */

  PICMAN_LOG (IMAGE_SCALE,
            "old_size = %"G_GINT64_FORMAT"  new_size = %"G_GINT64_FORMAT,
            current_size - undo_size - redo_size, new_size);

  *new_memsize = new_size;

  if (new_size > current_size && new_size > max_memsize)
    return PICMAN_IMAGE_SCALE_TOO_BIG;

  all_layers = picman_image_get_layer_list (image);

  for (list = all_layers; list; list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      /*  group layers are updated automatically  */
      if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
        continue;

      if (! picman_item_check_scaling (item, new_width, new_height))
        {
          g_list_free (all_layers);

          return PICMAN_IMAGE_SCALE_TOO_SMALL;
        }
    }

  g_list_free (all_layers);

  return PICMAN_IMAGE_SCALE_OK;
}
