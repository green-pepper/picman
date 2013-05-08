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
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-grid.h"
#include "picmanimage-guides.h"
#include "picmanimage-snap.h"

#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "picman-intl.h"


static gboolean  picman_image_snap_distance (const gdouble  unsnapped,
                                           const gdouble  nearest,
                                           const gdouble  epsilon,
                                           gdouble       *mindist,
                                           gdouble       *target);



/*  public functions  */

gboolean
picman_image_snap_x (PicmanImage *image,
                   gdouble    x,
                   gdouble   *tx,
                   gdouble    epsilon_x,
                   gboolean   snap_to_guides,
                   gboolean   snap_to_grid,
                   gboolean   snap_to_canvas)
{
  gdouble   mindist = G_MAXDOUBLE;
  gboolean  snapped = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (tx != NULL, FALSE);

  *tx = x;

  if (! picman_image_get_guides (image)) snap_to_guides = FALSE;
  if (! picman_image_get_grid (image))   snap_to_grid   = FALSE;

  if (! (snap_to_guides || snap_to_grid || snap_to_canvas))
    return FALSE;

  if (x < -epsilon_x || x >= (picman_image_get_width (image) + epsilon_x))
    return FALSE;

  if (snap_to_guides)
    {
      GList *list;

      for (list = picman_image_get_guides (image); list; list = g_list_next (list))
        {
          PicmanGuide *guide    = list->data;
          gint       position = picman_guide_get_position (guide);

          if (position < 0)
            continue;

          if (picman_guide_get_orientation (guide) == PICMAN_ORIENTATION_VERTICAL)
            {
              snapped |= picman_image_snap_distance (x, position,
                                                   epsilon_x,
                                                   &mindist, tx);
            }
        }
    }

  if (snap_to_grid)
    {
      PicmanGrid *grid = picman_image_get_grid (image);
      gdouble   xspacing;
      gdouble   xoffset;
      gdouble   i;

      g_object_get (grid,
                    "xspacing", &xspacing,
                    "xoffset",  &xoffset,
                    NULL);

      /* the snap-to-grid part could probably be rewritten */
      while (xoffset > xspacing)
        xoffset -= xspacing;

      for (i = xoffset; i <= picman_image_get_width (image); i += xspacing)
        {
          if (i < 0)
            continue;

          snapped |= picman_image_snap_distance (x, i,
                                               epsilon_x,
                                               &mindist, tx);
        }
    }

  if (snap_to_canvas)
    {
      snapped |= picman_image_snap_distance (x, 0,
                                           epsilon_x,
                                           &mindist, tx);
      snapped |= picman_image_snap_distance (x, picman_image_get_width (image),
                                           epsilon_x,
                                           &mindist, tx);
    }

  return snapped;
}

gboolean
picman_image_snap_y (PicmanImage *image,
                   gdouble    y,
                   gdouble   *ty,
                   gdouble    epsilon_y,
                   gboolean   snap_to_guides,
                   gboolean   snap_to_grid,
                   gboolean   snap_to_canvas)
{
  gdouble    mindist = G_MAXDOUBLE;
  gboolean   snapped = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (ty != NULL, FALSE);

  *ty = y;

  if (! picman_image_get_guides (image)) snap_to_guides = FALSE;
  if (! picman_image_get_grid (image))   snap_to_grid   = FALSE;

  if (! (snap_to_guides || snap_to_grid || snap_to_canvas))
    return FALSE;

  if (y < -epsilon_y || y >= (picman_image_get_height (image) + epsilon_y))
    return FALSE;

  if (snap_to_guides)
    {
      GList *list;

      for (list = picman_image_get_guides (image); list; list = g_list_next (list))
        {
          PicmanGuide *guide    = list->data;
          gint       position = picman_guide_get_position (guide);

          if (position < 0)
            continue;

          if (picman_guide_get_orientation (guide) == PICMAN_ORIENTATION_HORIZONTAL)
            {
              snapped |= picman_image_snap_distance (y, position,
                                                   epsilon_y,
                                                   &mindist, ty);
            }
        }
    }

  if (snap_to_grid)
    {
      PicmanGrid *grid = picman_image_get_grid (image);
      gdouble    yspacing;
      gdouble    yoffset;
      gdouble    i;

      g_object_get (grid,
                    "yspacing", &yspacing,
                    "yoffset",  &yoffset,
                    NULL);

      while (yoffset > yspacing)
        yoffset -= yspacing;

      for (i = yoffset; i <= picman_image_get_height (image); i += yspacing)
        {
          if (i < 0)
            continue;

          snapped |= picman_image_snap_distance (y, i,
                                               epsilon_y,
                                               &mindist, ty);
        }
    }

  if (snap_to_canvas)
    {
      snapped |= picman_image_snap_distance (y, 0,
                                           epsilon_y,
                                           &mindist, ty);
      snapped |= picman_image_snap_distance (y, picman_image_get_height (image),
                                           epsilon_y,
                                           &mindist, ty);
    }

  return snapped;
}

gboolean
picman_image_snap_point (PicmanImage *image,
                       gdouble    x,
                       gdouble    y,
                       gdouble   *tx,
                       gdouble   *ty,
                       gdouble    epsilon_x,
                       gdouble    epsilon_y,
                       gboolean   snap_to_guides,
                       gboolean   snap_to_grid,
                       gboolean   snap_to_canvas,
                       gboolean   snap_to_vectors)
{
  gdouble  mindist_x = G_MAXDOUBLE;
  gdouble  mindist_y = G_MAXDOUBLE;
  gboolean snapped   = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (tx != NULL, FALSE);
  g_return_val_if_fail (ty != NULL, FALSE);

  *tx = x;
  *ty = y;

  if (! picman_image_get_guides (image))         snap_to_guides  = FALSE;
  if (! picman_image_get_grid (image))           snap_to_grid    = FALSE;
  if (! picman_image_get_active_vectors (image)) snap_to_vectors = FALSE;

  if (! (snap_to_guides || snap_to_grid || snap_to_canvas || snap_to_vectors))
    return FALSE;

  if (x < -epsilon_x || x >= (picman_image_get_width  (image) + epsilon_x) ||
      y < -epsilon_y || y >= (picman_image_get_height (image) + epsilon_y))
    {
      return FALSE;
    }

  if (snap_to_guides)
    {
      GList *list;

      for (list = picman_image_get_guides (image); list; list = g_list_next (list))
        {
          PicmanGuide *guide    = list->data;
          gint       position = picman_guide_get_position (guide);

          if (position < 0)
            continue;

          switch (picman_guide_get_orientation (guide))
            {
            case PICMAN_ORIENTATION_HORIZONTAL:
              snapped |= picman_image_snap_distance (y, position,
                                                   epsilon_y,
                                                   &mindist_y, ty);
              break;

            case PICMAN_ORIENTATION_VERTICAL:
              snapped |= picman_image_snap_distance (x, position,
                                                   epsilon_x,
                                                   &mindist_x, tx);
              break;

            default:
              break;
            }
        }
    }

  if (snap_to_grid)
    {
      PicmanGrid *grid = picman_image_get_grid (image);
      gdouble   xspacing, yspacing;
      gdouble   xoffset, yoffset;
      gdouble   i;

      g_object_get (grid,
                    "xspacing", &xspacing,
                    "yspacing", &yspacing,
                    "xoffset",  &xoffset,
                    "yoffset",  &yoffset,
                    NULL);

      while (xoffset > xspacing)
        xoffset -= xspacing;

      while (yoffset > yspacing)
        yoffset -= yspacing;

      for (i = xoffset; i <= picman_image_get_width (image); i += xspacing)
        {
          if (i < 0)
            continue;

          snapped |= picman_image_snap_distance (x, i,
                                               epsilon_x,
                                               &mindist_x, tx);
        }

      for (i = yoffset; i <= picman_image_get_height (image); i += yspacing)
        {
          if (i < 0)
            continue;

          snapped |= picman_image_snap_distance (y, i,
                                               epsilon_y,
                                               &mindist_y, ty);
        }
    }

  if (snap_to_canvas)
    {
      snapped |= picman_image_snap_distance (x, 0,
                                           epsilon_x,
                                           &mindist_x, tx);
      snapped |= picman_image_snap_distance (x, picman_image_get_width (image),
                                           epsilon_x,
                                           &mindist_x, tx);

      snapped |= picman_image_snap_distance (y, 0,
                                           epsilon_y,
                                           &mindist_y, ty);
      snapped |= picman_image_snap_distance (y, picman_image_get_height (image),
                                           epsilon_y,
                                           &mindist_y, ty);
    }

  if (snap_to_vectors)
    {
      PicmanVectors *vectors = picman_image_get_active_vectors (image);
      PicmanStroke  *stroke  = NULL;
      PicmanCoords   coords  = { 0, 0, 0, 0, 0 };

      coords.x = x;
      coords.y = y;

      while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
        {
          PicmanCoords nearest;

          if (picman_stroke_nearest_point_get (stroke, &coords, 1.0,
                                             &nearest,
                                             NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (x, nearest.x,
                                                   epsilon_x,
                                                   &mindist_x, tx);
              snapped |= picman_image_snap_distance (y, nearest.y,
                                                   epsilon_y,
                                                   &mindist_y, ty);
            }
        }
    }

  return snapped;
}

gboolean
picman_image_snap_rectangle (PicmanImage *image,
                           gdouble    x1,
                           gdouble    y1,
                           gdouble    x2,
                           gdouble    y2,
                           gdouble   *tx1,
                           gdouble   *ty1,
                           gdouble    epsilon_x,
                           gdouble    epsilon_y,
                           gboolean   snap_to_guides,
                           gboolean   snap_to_grid,
                           gboolean   snap_to_canvas,
                           gboolean   snap_to_vectors)
{
  gdouble  nx, ny;
  gdouble  mindist_x = G_MAXDOUBLE;
  gdouble  mindist_y = G_MAXDOUBLE;
  gdouble  x_center  = (x1 + x2) / 2.0;
  gdouble  y_center  = (y1 + y2) / 2.0;
  gboolean snapped   = FALSE;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (tx1 != NULL, FALSE);
  g_return_val_if_fail (ty1 != NULL, FALSE);

  *tx1 = x1;
  *ty1 = y1;

  if (! picman_image_get_guides (image))         snap_to_guides  = FALSE;
  if (! picman_image_get_grid (image))           snap_to_grid    = FALSE;
  if (! picman_image_get_active_vectors (image)) snap_to_vectors = FALSE;

  if (! (snap_to_guides || snap_to_grid || snap_to_canvas || snap_to_vectors))
    return FALSE;

  /*  left edge  */
  if (picman_image_snap_x (image, x1, &nx,
                         MIN (epsilon_x, mindist_x),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_x = ABS (nx - x1);
      *tx1 = nx;
      snapped = TRUE;
    }

  /*  right edge  */
  if (picman_image_snap_x (image, x2, &nx,
                         MIN (epsilon_x, mindist_x),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_x = ABS (nx - x2);
      *tx1 = RINT (x1 + (nx - x2));
      snapped = TRUE;
    }

  /*  center, vertical  */
  if (picman_image_snap_x (image, x_center, &nx,
                         MIN (epsilon_x, mindist_x),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_x = ABS (nx - x_center);
      *tx1 = RINT (x1 + (nx - x_center));
      snapped = TRUE;
    }

  /*  top edge  */
  if (picman_image_snap_y (image, y1, &ny,
                         MIN (epsilon_y, mindist_y),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_y = ABS (ny - y1);
      *ty1 = ny;
      snapped = TRUE;
    }

  /*  bottom edge  */
  if (picman_image_snap_y (image, y2, &ny,
                         MIN (epsilon_y, mindist_y),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_y = ABS (ny - y2);
      *ty1 = RINT (y1 + (ny - y2));
      snapped = TRUE;
    }

  /*  center, horizontal  */
  if (picman_image_snap_y (image, y_center, &ny,
                         MIN (epsilon_y, mindist_y),
                         snap_to_guides,
                         snap_to_grid,
                         snap_to_canvas))
    {
      mindist_y = ABS (ny - y_center);
      *ty1 = RINT (y1 + (ny - y_center));
      snapped = TRUE;
    }

  if (snap_to_vectors)
    {
      PicmanVectors *vectors = picman_image_get_active_vectors (image);
      PicmanStroke  *stroke  = NULL;
      PicmanCoords   coords1 = PICMAN_COORDS_DEFAULT_VALUES;
      PicmanCoords   coords2 = PICMAN_COORDS_DEFAULT_VALUES;

      while ((stroke = picman_vectors_stroke_get_next (vectors, stroke)))
        {
          PicmanCoords nearest;
          gdouble    dist;

          /*  top edge  */

          coords1.x = x1;
          coords1.y = y1;
          coords2.x = x2;
          coords2.y = y1;

          if (picman_stroke_nearest_tangent_get (stroke, &coords1, &coords2,
                                               1.0, &nearest,
                                               NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (y1, nearest.y,
                                                   epsilon_y,
                                                   &mindist_y, ty1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords1, &coords2,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (x1, nearest.x,
                                                   epsilon_x,
                                                   &mindist_x, tx1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords2, &coords1,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.x - x2);

              if (dist < MIN (epsilon_x, mindist_x))
                {
                  mindist_x = dist;
                  *tx1 = RINT (x1 + (nearest.x - x2));
                  snapped = TRUE;
                }
            }

          /*  bottom edge  */

          coords1.x = x1;
          coords1.y = y2;
          coords2.x = x2;
          coords2.y = y2;

          if (picman_stroke_nearest_tangent_get (stroke, &coords1, &coords2,
                                               1.0, &nearest,
                                               NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.y - y2);

              if (dist < MIN (epsilon_y, mindist_y))
                {
                  mindist_y = dist;
                  *ty1 = RINT (y1 + (nearest.y - y2));
                  snapped = TRUE;
                }
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords1, &coords2,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (x1, nearest.x,
                                                   epsilon_x,
                                                   &mindist_x, tx1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords2, &coords1,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.x - x2);

              if (dist < MIN (epsilon_x, mindist_x))
                {
                  mindist_x = dist;
                  *tx1 = RINT (x1 + (nearest.x - x2));
                  snapped = TRUE;
                }
            }

          /*  left edge  */

          coords1.x = x1;
          coords1.y = y1;
          coords2.x = x1;
          coords2.y = y2;

          if (picman_stroke_nearest_tangent_get (stroke, &coords1, &coords2,
                                               1.0, &nearest,
                                               NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (x1, nearest.x,
                                                   epsilon_x,
                                                   &mindist_x, tx1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords1, &coords2,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (y1, nearest.y,
                                                   epsilon_y,
                                                   &mindist_y, ty1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords2, &coords1,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.y - y2);

              if (dist < MIN (epsilon_y, mindist_y))
                {
                  mindist_y = dist;
                  *ty1 = RINT (y1 + (nearest.y - y2));
                  snapped = TRUE;
                }
            }

          /*  right edge  */

          coords1.x = x2;
          coords1.y = y1;
          coords2.x = x2;
          coords2.y = y2;

          if (picman_stroke_nearest_tangent_get (stroke, &coords1, &coords2,
                                               1.0, &nearest,
                                               NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.x - x2);

              if (dist < MIN (epsilon_x, mindist_x))
                {
                  mindist_x = dist;
                  *tx1 = RINT (x1 + (nearest.x - x2));
                  snapped = TRUE;
                }
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords1, &coords2,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              snapped |= picman_image_snap_distance (y1, nearest.y,
                                                   epsilon_y,
                                                   &mindist_y, ty1);
            }

          if (picman_stroke_nearest_intersection_get (stroke, &coords2, &coords1,
                                                    1.0, &nearest,
                                                    NULL, NULL, NULL) >= 0)
            {
              dist = ABS (nearest.y - y2);

              if (dist < MIN (epsilon_y, mindist_y))
                {
                  mindist_y = dist;
                  *ty1 = RINT (y1 + (nearest.y - y2));
                  snapped = TRUE;
                }
            }
        }
    }

  return snapped;
}

/* private functions */

/**
 * picman_image_snap_distance:
 * @unsnapped: One coordinate of the unsnapped position
 * @nearest:  One coordinate of a snapping position candidate
 * @epsilon:  The snapping threshold
 * @mindist:  The distance to the currently closest snapping target
 * @target:   The currently closest snapping target
 *
 * Finds out if snapping occurs from position to a snapping candidate
 * and sets the target accordingly.
 *
 * Return value: %TRUE if snapping occurred, %FALSE otherwise
 */
static gboolean
picman_image_snap_distance (const gdouble  unsnapped,
                          const gdouble  nearest,
                          const gdouble  epsilon,
                          gdouble       *mindist,
                          gdouble       *target)
{
  const gdouble dist = ABS (nearest - unsnapped);

  if (dist < MIN (epsilon, *mindist))
    {
      *mindist = dist;
      *target = nearest;

      return TRUE;
    }

  return FALSE;
}
