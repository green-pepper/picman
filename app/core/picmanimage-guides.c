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
#include "picmanimage.h"
#include "picmanguide.h"
#include "picmanimage-guides.h"
#include "picmanimage-private.h"
#include "picmanimage-undo-push.h"

#include "picman-intl.h"


/*  public functions  */

PicmanGuide *
picman_image_add_hguide (PicmanImage *image,
                       gint       position,
                       gboolean   push_undo)
{
  PicmanGuide *guide;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (position >= 0 &&
                        position <= picman_image_get_height (image), NULL);

  guide = picman_guide_new (PICMAN_ORIENTATION_HORIZONTAL,
                          image->picman->next_guide_ID++);

  if (push_undo)
    picman_image_undo_push_guide (image,
                                C_("undo-type", "Add Horizontal Guide"), guide);

  picman_image_add_guide (image, guide, position);
  g_object_unref (G_OBJECT (guide));

  return guide;
}

PicmanGuide *
picman_image_add_vguide (PicmanImage *image,
                       gint       position,
                       gboolean   push_undo)
{
  PicmanGuide *guide;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (position >= 0 &&
                        position <= picman_image_get_width (image), NULL);

  guide = picman_guide_new (PICMAN_ORIENTATION_VERTICAL,
                          image->picman->next_guide_ID++);

  if (push_undo)
    picman_image_undo_push_guide (image,
                                C_("undo-type", "Add Vertical Guide"), guide);

  picman_image_add_guide (image, guide, position);
  g_object_unref (G_OBJECT (guide));

  return guide;
}

void
picman_image_add_guide (PicmanImage *image,
                      PicmanGuide *guide,
                      gint       position)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->guides = g_list_prepend (private->guides, guide);

  picman_guide_set_position (guide, position);
  g_object_ref (G_OBJECT (guide));

  picman_image_guide_added (image, guide);
}

void
picman_image_remove_guide (PicmanImage *image,
                         PicmanGuide *guide,
                         gboolean   push_undo)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (push_undo)
    picman_image_undo_push_guide (image, C_("undo-type", "Remove Guide"), guide);

  private->guides = g_list_remove (private->guides, guide);
  picman_guide_removed (guide);

  picman_image_guide_removed (image, guide);

  picman_guide_set_position (guide, -1);
  g_object_unref (G_OBJECT (guide));
}

void
picman_image_move_guide (PicmanImage *image,
                       PicmanGuide *guide,
                       gint       position,
                       gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GUIDE (guide));
  g_return_if_fail (position >= 0);

  if (picman_guide_get_orientation (guide) == PICMAN_ORIENTATION_HORIZONTAL)
    g_return_if_fail (position <= picman_image_get_height (image));
  else
    g_return_if_fail (position <= picman_image_get_width (image));

  if (push_undo)
    picman_image_undo_push_guide (image, C_("undo-type", "Move Guide"), guide);

  picman_guide_set_position (guide, position);

  picman_image_guide_moved (image, guide);
}

GList *
picman_image_get_guides (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->guides;
}

PicmanGuide *
picman_image_get_guide (PicmanImage *image,
                      guint32    id)
{
  GList *guides;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  for (guides = PICMAN_IMAGE_GET_PRIVATE (image)->guides;
       guides;
       guides = g_list_next (guides))
    {
      PicmanGuide *guide = guides->data;

      if (picman_guide_get_ID (guide) == id &&
          picman_guide_get_position (guide) >= 0)
        return guide;
    }

  return NULL;
}

PicmanGuide *
picman_image_get_next_guide (PicmanImage *image,
                           guint32    id,
                           gboolean  *guide_found)
{
  GList *guides;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (guide_found != NULL, NULL);

  if (id == 0)
    *guide_found = TRUE;
  else
    *guide_found = FALSE;

  for (guides = PICMAN_IMAGE_GET_PRIVATE (image)->guides;
       guides;
       guides = g_list_next (guides))
    {
      PicmanGuide *guide = guides->data;

      if (picman_guide_get_position (guide) < 0)
        continue;

      if (*guide_found) /* this is the first guide after the found one */
        return guide;

      if (picman_guide_get_ID (guide) == id) /* found it, next one will be returned */
        *guide_found = TRUE;
    }

  return NULL;
}

PicmanGuide *
picman_image_find_guide (PicmanImage *image,
                       gdouble    x,
                       gdouble    y,
                       gdouble    epsilon_x,
                       gdouble    epsilon_y)
{
  GList     *list;
  PicmanGuide *ret     = NULL;
  gdouble    mindist = G_MAXDOUBLE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (epsilon_x > 0 && epsilon_y > 0, NULL);

  for (list = PICMAN_IMAGE_GET_PRIVATE (image)->guides;
       list;
       list = g_list_next (list))
    {
      PicmanGuide *guide    = list->data;
      gint       position = picman_guide_get_position (guide);
      gdouble    dist;

      if (position < 0)
        continue;

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          dist = ABS (position - y);
          if (dist < MIN (epsilon_y, mindist))
            {
              mindist = dist;
              ret = guide;
            }
          break;

        /* mindist always is in vertical resolution to make it comparable */
        case PICMAN_ORIENTATION_VERTICAL:
          dist = ABS (position - x);
          if (dist < MIN (epsilon_x, mindist / epsilon_y * epsilon_x))
            {
              mindist = dist * epsilon_y / epsilon_x;
              ret = guide;
            }
          break;

        default:
          continue;
        }

    }

  return ret;
}
