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
#include "picmancontext.h"
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-flip.h"
#include "picmanimage-guides.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanitem.h"
#include "picmanprogress.h"
#include "picmansamplepoint.h"


void
picman_image_flip (PicmanImage           *image,
                 PicmanContext         *context,
                 PicmanOrientationType  flip_type,
                 PicmanProgress        *progress)
{
  GList   *list;
  gdouble  axis;
  gdouble  progress_max;
  gdouble  progress_current = 1.0;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  picman_set_busy (image->picman);

  switch (flip_type)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      axis = (gdouble) picman_image_get_width (image) / 2.0;
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      axis = (gdouble) picman_image_get_height (image) / 2.0;
      break;

    default:
      g_warning ("%s: unknown flip_type", G_STRFUNC);
      return;
    }

  progress_max = (picman_container_get_n_children (picman_image_get_channels (image)) +
                  picman_container_get_n_children (picman_image_get_layers (image))   +
                  picman_container_get_n_children (picman_image_get_vectors (image))  +
                  1 /* selection */);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_FLIP, NULL);

  /*  Flip all channels  */
  for (list = picman_image_get_channel_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_flip (item, context, flip_type, axis, TRUE);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Flip all vectors  */
  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_flip (item, context, flip_type, axis, FALSE);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Don't forget the selection mask!  */
  picman_item_flip (PICMAN_ITEM (picman_image_get_mask (image)), context,
                  flip_type, axis, TRUE);

  if (progress)
    picman_progress_set_value (progress, progress_current++ / progress_max);

  /*  Flip all layers  */
  for (list = picman_image_get_layer_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_flip (item, context, flip_type, axis, FALSE);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Flip all Guides  */
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      PicmanGuide *guide    = list->data;
      gint       position = picman_guide_get_position (guide);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          if (flip_type == PICMAN_ORIENTATION_VERTICAL)
            picman_image_move_guide (image, guide,
                                   picman_image_get_height (image) - position,
                                   TRUE);
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          if (flip_type == PICMAN_ORIENTATION_HORIZONTAL)
            picman_image_move_guide (image, guide,
                                   picman_image_get_width (image) - position,
                                   TRUE);
          break;

        default:
          break;
        }
    }

  /*  Flip all sample points  */
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;

      if (flip_type == PICMAN_ORIENTATION_VERTICAL)
        picman_image_move_sample_point (image, sample_point,
                                      sample_point->x,
                                      picman_image_get_height (image) -
                                      sample_point->y,
                                      TRUE);

      if (flip_type == PICMAN_ORIENTATION_HORIZONTAL)
        picman_image_move_sample_point (image, sample_point,
                                      picman_image_get_width (image) -
                                      sample_point->x,
                                      sample_point->y,
                                      TRUE);
    }

  picman_image_undo_group_end (image);

  picman_unset_busy (image->picman);
}
