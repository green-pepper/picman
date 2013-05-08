/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectors-warp.c
 * Copyright (C) 2005 Bill Skaggs  <weskaggs@primate.ucdavis.edu>
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

#include "vectors-types.h"

#include "libpicmanmath/picmanmath.h"

#include "core/picman-utils.h"
#include "core/picmancoords.h"

#include "picmananchor.h"
#include "picmanstroke.h"
#include "picmanvectors.h"
#include "picmanvectors-warp.h"


#define EPSILON 0.2
#define DX      2.0


static void picman_stroke_warp_point   (const PicmanStroke  *stroke,
                                      gdouble            x,
                                      gdouble            y,
                                      PicmanCoords        *point_warped,
                                      gdouble            y_offset);

static void picman_vectors_warp_stroke (const PicmanVectors *vectors,
                                      PicmanStroke        *stroke,
                                      gdouble            y_offset);


void
picman_vectors_warp_point (const PicmanVectors *vectors,
                         PicmanCoords        *point,
                         PicmanCoords        *point_warped,
                         gdouble            y_offset)
{
  gdouble     x      = point->x;
  gdouble     y      = point->y;
  gdouble     len;
  GList      *list;
  PicmanStroke *stroke;

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      stroke = list->data;

      len = picman_vectors_stroke_get_length (vectors, stroke);

      if (x < len)
        break;

      x -= len;
    }

  if (! list)
    {
      point_warped->x = 0;
      point_warped->y = 0;
      return;
    }

  picman_stroke_warp_point (stroke, x, y, point_warped, y_offset);
}

static void
picman_stroke_warp_point (const PicmanStroke *stroke,
                        gdouble           x,
                        gdouble           y,
                        PicmanCoords       *point_warped,
                        gdouble           y_offset)
{
  PicmanCoords point_zero  = { 0, };
  PicmanCoords point_minus = { 0, };
  PicmanCoords point_plus  = { 0, };
  gdouble    slope;
  gdouble    dx, dy, nx, ny, len;

  if (! picman_stroke_get_point_at_dist (stroke, x, EPSILON,
                                       &point_zero, &slope))
    {
      point_warped->x = 0;
      point_warped->y = 0;
      return;
    }

  point_warped->x = point_zero.x;
  point_warped->y = point_zero.y;

  if (! picman_stroke_get_point_at_dist (stroke, x - DX, EPSILON,
                                       &point_minus, &slope))
    return;

  if (! picman_stroke_get_point_at_dist (stroke, x + DX, EPSILON,
                                       &point_plus, &slope))
    return;

  dx = point_plus.x - point_minus.x;
  dy = point_plus.y - point_minus.y;

  len = hypot (dx, dy);

  if (len < 0.01)
    return;

  nx = - dy / len;
  ny =   dx / len;

  point_warped->x = point_zero.x + nx * (y - y_offset);
  point_warped->y = point_zero.y + ny * (y - y_offset);
}

static void
picman_vectors_warp_stroke (const PicmanVectors *vectors,
                          PicmanStroke        *stroke,
                          gdouble            y_offset)
{
  GList *list;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      picman_vectors_warp_point (vectors,
                               &anchor->position, &anchor->position,
                               y_offset);
    }
}

void
picman_vectors_warp_vectors (const PicmanVectors *vectors,
                           PicmanVectors       *vectors_in,
                           gdouble            y_offset)
{
  GList *list;

  for (list = vectors_in->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_vectors_warp_stroke (vectors, stroke, y_offset);
    }
}
