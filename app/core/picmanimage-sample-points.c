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

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picman.h"
#include "picmanimage.h"
#include "picmanimage-private.h"
#include "picmanimage-sample-points.h"
#include "picmanimage-undo-push.h"
#include "picmansamplepoint.h"

#include "picman-intl.h"


/*  public functions  */

PicmanSamplePoint *
picman_image_add_sample_point_at_pos (PicmanImage *image,
                                    gint       x,
                                    gint       y,
                                    gboolean   push_undo)
{
  PicmanSamplePoint *sample_point;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (x >= 0 && x < picman_image_get_width  (image), NULL);
  g_return_val_if_fail (y >= 0 && y < picman_image_get_height (image), NULL);

  sample_point = picman_sample_point_new (image->picman->next_sample_point_ID++);

  if (push_undo)
    picman_image_undo_push_sample_point (image, C_("undo-type", "Add Sample Point"),
                                       sample_point);

  picman_image_add_sample_point (image, sample_point, x, y);
  picman_sample_point_unref (sample_point);

  return sample_point;
}

void
picman_image_add_sample_point (PicmanImage       *image,
                             PicmanSamplePoint *sample_point,
                             gint             x,
                             gint             y)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->sample_points = g_list_append (private->sample_points, sample_point);

  sample_point->x = x;
  sample_point->y = y;
  picman_sample_point_ref (sample_point);

  picman_image_sample_point_added (image, sample_point);
}

void
picman_image_remove_sample_point (PicmanImage       *image,
                                PicmanSamplePoint *sample_point,
                                gboolean         push_undo)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (push_undo)
    picman_image_undo_push_sample_point (image,
                                       C_("undo-type", "Remove Sample Point"),
                                       sample_point);

  private->sample_points = g_list_remove (private->sample_points, sample_point);

  picman_image_sample_point_removed (image, sample_point);

  sample_point->x = -1;
  sample_point->y = -1;
  picman_sample_point_unref (sample_point);
}

void
picman_image_move_sample_point (PicmanImage       *image,
                              PicmanSamplePoint *sample_point,
                              gint             x,
                              gint             y,
                              gboolean         push_undo)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (sample_point != NULL);
  g_return_if_fail (x >= 0);
  g_return_if_fail (y >= 0);
  g_return_if_fail (x < picman_image_get_width  (image));
  g_return_if_fail (y < picman_image_get_height (image));

  if (push_undo)
    picman_image_undo_push_sample_point (image,
                                       C_("undo-type", "Move Sample Point"),
                                       sample_point);

  sample_point->x = x;
  sample_point->y = y;

  picman_image_sample_point_moved (image, sample_point);
}

GList *
picman_image_get_sample_points (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->sample_points;
}

PicmanSamplePoint *
picman_image_find_sample_point (PicmanImage *image,
                              gdouble    x,
                              gdouble    y,
                              gdouble    epsilon_x,
                              gdouble    epsilon_y)
{
  GList           *list;
  PicmanSamplePoint *ret     = NULL;
  gdouble          mindist = G_MAXDOUBLE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (epsilon_x > 0 && epsilon_y > 0, NULL);

  if (x < 0 || x >= picman_image_get_width  (image) ||
      y < 0 || y >= picman_image_get_height (image))
    {
      return NULL;
    }

  for (list = PICMAN_IMAGE_GET_PRIVATE (image)->sample_points;
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;
      gdouble          dist;

      if (sample_point->x < 0 || sample_point->y < 0)
        continue;

      dist = hypot ((sample_point->x + 0.5) - x,
                    (sample_point->y + 0.5) - y);
      if (dist < MIN (epsilon_y, mindist))
        {
          mindist = dist;
          ret = sample_point;
        }
    }

  return ret;
}
