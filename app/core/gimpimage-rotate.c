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
#include "picmanimage-rotate.h"
#include "picmanimage-guides.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanitem.h"
#include "picmanprogress.h"
#include "picmansamplepoint.h"


static void  picman_image_rotate_item_offset   (PicmanImage        *image,
                                              PicmanRotationType  rotate_type,
                                              PicmanItem         *item,
                                              gint              off_x,
                                              gint              off_y);
static void  picman_image_rotate_guides        (PicmanImage        *image,
                                              PicmanRotationType  rotate_type);
static void  picman_image_rotate_sample_points (PicmanImage        *image,
                                              PicmanRotationType  rotate_type);


void
picman_image_rotate (PicmanImage        *image,
                   PicmanContext      *context,
                   PicmanRotationType  rotate_type,
                   PicmanProgress     *progress)
{
  GList    *list;
  gdouble   center_x;
  gdouble   center_y;
  gdouble   progress_max;
  gdouble   progress_current = 1.0;
  gint      new_image_width;
  gint      new_image_height;
  gint      previous_image_width;
  gint      previous_image_height;
  gint      offset_x;
  gint      offset_y;
  gboolean  size_changed;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  picman_set_busy (image->picman);

  previous_image_width  = picman_image_get_width  (image);
  previous_image_height = picman_image_get_height (image);

  center_x              = previous_image_width  / 2.0;
  center_y              = previous_image_height / 2.0;

  progress_max = (picman_container_get_n_children (picman_image_get_channels (image)) +
                  picman_container_get_n_children (picman_image_get_layers (image))   +
                  picman_container_get_n_children (picman_image_get_vectors (image))  +
                  1 /* selection */);

  g_object_freeze_notify (G_OBJECT (image));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_ROTATE, NULL);

  /*  Resize the image (if needed)  */
  switch (rotate_type)
    {
    case PICMAN_ROTATE_90:
    case PICMAN_ROTATE_270:
      new_image_width  = picman_image_get_height (image);
      new_image_height = picman_image_get_width  (image);
      size_changed     = TRUE;
      offset_x         = (picman_image_get_width  (image) - new_image_width)  / 2;
      offset_y         = (picman_image_get_height (image) - new_image_height) / 2;
      break;

    case PICMAN_ROTATE_180:
      new_image_width  = picman_image_get_width  (image);
      new_image_height = picman_image_get_height (image);
      size_changed     = FALSE;
      offset_x         = 0;
      offset_y         = 0;
      break;

    default:
      g_assert_not_reached ();
      return;
    }

  /*  Rotate all channels  */
  for (list = picman_image_get_channel_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_rotate (item, context, rotate_type, center_x, center_y, FALSE);

      picman_item_set_offset (item, 0, 0);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Rotate all vectors  */
  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;

      picman_item_rotate (item, context, rotate_type, center_x, center_y, FALSE);

      picman_item_set_offset (item, 0, 0);
      picman_item_set_size (item, new_image_width, new_image_height);

      picman_item_translate (item,
                           (new_image_width  - picman_image_get_width  (image)) / 2,
                           (new_image_height - picman_image_get_height (image)) / 2,
                           FALSE);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Don't forget the selection mask!  */
  {
    PicmanChannel *mask = picman_image_get_mask (image);

    picman_item_rotate (PICMAN_ITEM (mask), context,
                      rotate_type, center_x, center_y, FALSE);

    picman_item_set_offset (PICMAN_ITEM (mask), 0, 0);

    if (progress)
      picman_progress_set_value (progress, progress_current++ / progress_max);
  }

  /*  Rotate all layers  */
  for (list = picman_image_get_layer_iter (image);
       list;
       list = g_list_next (list))
    {
      PicmanItem *item = list->data;
      gint      off_x;
      gint      off_y;

      picman_item_get_offset (item, &off_x, &off_y);

      picman_item_rotate (item, context, rotate_type, center_x, center_y, FALSE);

      picman_image_rotate_item_offset (image, rotate_type, item, off_x, off_y);

      if (progress)
        picman_progress_set_value (progress, progress_current++ / progress_max);
    }

  /*  Rotate all Guides  */
  picman_image_rotate_guides (image, rotate_type);

  /*  Rotate all sample points  */
  picman_image_rotate_sample_points (image, rotate_type);

  /*  Resize the image (if needed)  */
  if (size_changed)
    {
      gdouble xres;
      gdouble yres;

      picman_image_undo_push_image_size (image,
                                       NULL,
                                       offset_x,
                                       offset_y,
                                       new_image_width,
                                       new_image_height);

      g_object_set (image,
                    "width",  new_image_width,
                    "height", new_image_height,
                    NULL);

      picman_image_get_resolution (image, &xres, &yres);

      if (xres != yres)
        picman_image_set_resolution (image, yres, xres);
    }

  picman_image_undo_group_end (image);

  if (size_changed)
    picman_image_size_changed_detailed (image,
                                      -offset_x,
                                      -offset_y,
                                      previous_image_width,
                                      previous_image_height);

  g_object_thaw_notify (G_OBJECT (image));

  picman_unset_busy (image->picman);
}


static void
picman_image_rotate_item_offset (PicmanImage        *image,
                               PicmanRotationType  rotate_type,
                               PicmanItem         *item,
                               gint              off_x,
                               gint              off_y)
{
  gint x = 0;
  gint y = 0;

  switch (rotate_type)
    {
    case PICMAN_ROTATE_90:
      x = picman_image_get_height (image) - off_y - picman_item_get_width (item);
      y = off_x;
      break;

    case PICMAN_ROTATE_270:
      x = off_y;
      y = picman_image_get_width (image) - off_x - picman_item_get_height (item);
      break;

    case PICMAN_ROTATE_180:
      return;
    }

  picman_item_get_offset (item, &off_x, &off_y);

  x -= off_x;
  y -= off_y;

  if (x || y)
    picman_item_translate (item, x, y, FALSE);
}

static void
picman_image_rotate_guides (PicmanImage        *image,
                          PicmanRotationType  rotate_type)
{
  GList *list;

  /*  Rotate all Guides  */
  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      PicmanGuide           *guide       = list->data;
      PicmanOrientationType  orientation = picman_guide_get_orientation (guide);
      gint                 position    = picman_guide_get_position (guide);

      switch (rotate_type)
        {
        case PICMAN_ROTATE_90:
          switch (orientation)
            {
            case PICMAN_ORIENTATION_HORIZONTAL:
              picman_image_undo_push_guide (image, NULL, guide);
              picman_guide_set_orientation (guide, PICMAN_ORIENTATION_VERTICAL);
              picman_guide_set_position (guide,
                                       picman_image_get_height (image) - position);
              break;

            case PICMAN_ORIENTATION_VERTICAL:
              picman_image_undo_push_guide (image, NULL, guide);
              picman_guide_set_orientation (guide, PICMAN_ORIENTATION_HORIZONTAL);
              break;

            default:
              break;
            }
          break;

        case PICMAN_ROTATE_180:
          switch (orientation)
            {
            case PICMAN_ORIENTATION_HORIZONTAL:
              picman_image_move_guide (image, guide,
                                     picman_image_get_height (image) - position,
                                     TRUE);
              break;

            case PICMAN_ORIENTATION_VERTICAL:
              picman_image_move_guide (image, guide,
                                     picman_image_get_width (image) - position,
                                     TRUE);
              break;

            default:
              break;
            }
          break;

        case PICMAN_ROTATE_270:
          switch (orientation)
            {
            case PICMAN_ORIENTATION_HORIZONTAL:
              picman_image_undo_push_guide (image, NULL, guide);
              picman_guide_set_orientation (guide, PICMAN_ORIENTATION_VERTICAL);
              break;

            case PICMAN_ORIENTATION_VERTICAL:
              picman_image_undo_push_guide (image, NULL, guide);
              picman_guide_set_orientation (guide, PICMAN_ORIENTATION_HORIZONTAL);
              picman_guide_set_position (guide,
                                       picman_image_get_width (image) - position);
              break;

            default:
              break;
            }
          break;
        }
    }
}


static void
picman_image_rotate_sample_points (PicmanImage        *image,
                                 PicmanRotationType  rotate_type)
{
  GList *list;

  /*  Rotate all sample points  */
  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;
      gint             old_x;
      gint             old_y;

      picman_image_undo_push_sample_point (image, NULL, sample_point);

      old_x = sample_point->x;
      old_y = sample_point->y;

      switch (rotate_type)
        {
        case PICMAN_ROTATE_90:
          sample_point->x = old_y;
          sample_point->y = picman_image_get_height (image) - old_x;
          break;

        case PICMAN_ROTATE_180:
          sample_point->x = picman_image_get_height (image) - old_x;
          sample_point->y = picman_image_get_width  (image) - old_y;
          break;

        case PICMAN_ROTATE_270:
          sample_point->x = picman_image_get_width (image) - old_y;
          sample_point->y = old_x;
          break;
        }
    }
}
