/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancoords-interpolate.c
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

#include <glib-object.h>

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmancoords.h"
#include "picmancoords-interpolate.h"


/* Local helper functions declarations*/
static void     picman_coords_interpolate_bezier_internal (const PicmanCoords  bezier_pt1,
                                                         const PicmanCoords  bezier_pt2,
                                                         const PicmanCoords  bezier_pt3,
                                                         const PicmanCoords  bezier_pt4,
                                                         const gdouble     start_t,
                                                         const gdouble     end_t,
                                                         const gdouble     precision,
                                                         GArray          **ret_coords,
                                                         GArray          **ret_params,
                                                         gint              depth);
static gdouble  picman_coords_get_catmull_spline_point    (const gdouble     t,
                                                         const gdouble     p0,
                                                         const gdouble     p1,
                                                         const gdouble     p2,
                                                         const gdouble     p3);

/* Functions for bezier subdivision */

void
picman_coords_interpolate_bezier (const PicmanCoords   bezier_pt1,
                                const PicmanCoords   bezier_pt2,
                                const PicmanCoords   bezier_pt3,
                                const PicmanCoords   bezier_pt4,
                                const gdouble      precision,
                                GArray           **ret_coords,
                                GArray           **ret_params)
{
  picman_coords_interpolate_bezier_internal (bezier_pt1,
                                           bezier_pt2,
                                           bezier_pt3,
                                           bezier_pt4,
                                           0.0, 1.0,
                                           precision,
                                           ret_coords, ret_params, 10);
}

/* Recursive subdivision helper function */
static void
picman_coords_interpolate_bezier_internal (const PicmanCoords  bezier_pt1,
                                         const PicmanCoords  bezier_pt2,
                                         const PicmanCoords  bezier_pt3,
                                         const PicmanCoords  bezier_pt4,
                                         const gdouble     start_t,
                                         const gdouble     end_t,
                                         const gdouble     precision,
                                         GArray          **ret_coords,
                                         GArray          **ret_params,
                                         gint              depth)
{
  /*
   * beziercoords has to contain four PicmanCoords with the four control points
   * of the bezier segment. We subdivide it at the parameter 0.5.
   */

  PicmanCoords subdivided[8];
  gdouble    middle_t = (start_t + end_t) / 2;

  subdivided[0] = bezier_pt1;
  subdivided[6] = bezier_pt4;

  /* if (!depth) g_printerr ("Hit recursion depth limit!\n"); */

  picman_coords_average (&bezier_pt1, &bezier_pt2, &(subdivided[1]));

  picman_coords_average (&bezier_pt2, &bezier_pt3, &(subdivided[7]));

  picman_coords_average (&bezier_pt3, &bezier_pt4, &(subdivided[5]));

  picman_coords_average (&(subdivided[1]), &(subdivided[7]),
                       &(subdivided[2]));

  picman_coords_average (&(subdivided[7]), &(subdivided[5]),
                       &(subdivided[4]));

  picman_coords_average (&(subdivided[2]), &(subdivided[4]),
                       &(subdivided[3]));

  /*
   * We now have the coordinates of the two bezier segments in
   * subdivided [0-3] and subdivided [3-6]
   */

  /*
   * Here we need to check, if we have sufficiently subdivided, i.e.
   * if the stroke is sufficiently close to a straight line.
   */

  if (!depth || picman_coords_bezier_is_straight (subdivided[0],
                                                subdivided[1],
                                                subdivided[2],
                                                subdivided[3],
                                                precision)) /* 1st half */
    {
      *ret_coords = g_array_append_vals (*ret_coords, &(subdivided[0]), 3);

      if (ret_params)
        {
          gdouble params[3];

          params[0] = start_t;
          params[1] = (2 * start_t + middle_t) / 3;
          params[2] = (start_t + 2 * middle_t) / 3;

          *ret_params = g_array_append_vals (*ret_params, &(params[0]), 3);
        }
    }
  else
    {
      picman_coords_interpolate_bezier_internal (subdivided[0],
                                               subdivided[1],
                                               subdivided[2],
                                               subdivided[3],
                                               start_t, (start_t + end_t) / 2,
                                               precision,
                                               ret_coords, ret_params, depth-1);
    }

  if (!depth || picman_coords_bezier_is_straight (subdivided[3],
                                                subdivided[4],
                                                subdivided[5],
                                                subdivided[6],
                                                precision)) /* 2nd half */
    {
      *ret_coords = g_array_append_vals (*ret_coords, &(subdivided[3]), 3);

      if (ret_params)
        {
          gdouble params[3];

          params[0] = middle_t;
          params[1] = (2 * middle_t + end_t) / 3;
          params[2] = (middle_t + 2 * end_t) / 3;

          *ret_params = g_array_append_vals (*ret_params, &(params[0]), 3);
        }
    }
  else
    {
      picman_coords_interpolate_bezier_internal (subdivided[3],
                                               subdivided[4],
                                               subdivided[5],
                                               subdivided[6],
                                               (start_t + end_t) / 2, end_t,
                                               precision,
                                               ret_coords, ret_params, depth-1);
    }
}


/*
 * a helper function that determines if a bezier segment is "straight
 * enough" to be approximated by a line.
 *
 * To be more exact, it also checks for the control points to be distributed
 * evenly along the line. This makes it easier to reconstruct parameters for
 * a given point along the segment.
 *
 * Needs four PicmanCoords in an array.
 */

gboolean
picman_coords_bezier_is_straight (const PicmanCoords bezier_pt1,
                                const PicmanCoords bezier_pt2,
                                const PicmanCoords bezier_pt3,
                                const PicmanCoords bezier_pt4,
                                gdouble          precision)
{
  PicmanCoords pt1, pt2;

  /* calculate the "ideal" positions for the control points */

  picman_coords_mix (2.0 / 3.0, &bezier_pt1,
                   1.0 / 3.0, &bezier_pt4,
                   &pt1);
  picman_coords_mix (1.0 / 3.0, &bezier_pt1,
                   2.0 / 3.0, &bezier_pt4,
                   &pt2);

  /* calculate the deviation of the actual control points */

  return (picman_coords_manhattan_dist (&bezier_pt2, &pt1) < precision &&
          picman_coords_manhattan_dist (&bezier_pt3, &pt2) < precision);
}


/* Functions for camull-rom interpolation */

void
picman_coords_interpolate_catmull (const PicmanCoords   catmul_pt1,
                                 const PicmanCoords   catmul_pt2,
                                 const PicmanCoords   catmul_pt3,
                                 const PicmanCoords   catmul_pt4,
                                 gdouble            precision,
                                 GArray           **ret_coords,
                                 GArray           **ret_params)
{
  gdouble     delta_x, delta_y;
  gdouble     distance;
  gdouble     dir_step;
  gdouble     delta_dir;
  gint        num_points;
  gint        n;

  PicmanCoords  past_coords;
  PicmanCoords  start_coords;
  PicmanCoords  end_coords;
  PicmanCoords  future_coords;

  delta_x = catmul_pt3.x - catmul_pt2.x;
  delta_y = catmul_pt3.y - catmul_pt2.y;

  /* Catmull-Rom interpolation requires 4 points.
   * Two endpoints plus one more at each end.
   */

  past_coords   = catmul_pt1;
  start_coords  = catmul_pt2;
  end_coords    = catmul_pt3;
  future_coords = catmul_pt4;

  distance  = sqrt (SQR (delta_x) + SQR (delta_y));

  num_points = distance / precision;

  delta_dir = end_coords.direction - start_coords.direction;

  if (delta_dir <= -0.5)
    delta_dir += 1.0;
  else if (delta_dir >= 0.5)
    delta_dir -= 1.0;

  dir_step =  delta_dir / num_points;

  for (n = 1; n <= num_points; n++)
    {
      PicmanCoords coords;
      gdouble    velocity;
      gdouble    pressure;
      gdouble    p = (gdouble) n / num_points;

      coords.x =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.x,
                                              start_coords.x,
                                              end_coords.x,
                                              future_coords.x);

      coords.y =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.y,
                                              start_coords.y,
                                              end_coords.y,
                                              future_coords.y);

      pressure =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.pressure,
                                              start_coords.pressure,
                                              end_coords.pressure,
                                              future_coords.pressure);
      coords.pressure = CLAMP (pressure, 0.0, 1.0);

      coords.xtilt =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.xtilt,
                                              start_coords.xtilt,
                                              end_coords.xtilt,
                                              future_coords.xtilt);
      coords.ytilt =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.ytilt,
                                              start_coords.ytilt,
                                              end_coords.ytilt,
                                              future_coords.ytilt);

      coords.wheel =
        picman_coords_get_catmull_spline_point (p,
                                              past_coords.wheel,
                                              start_coords.wheel,
                                              end_coords.wheel,
                                              future_coords.wheel);

      velocity = picman_coords_get_catmull_spline_point (p,
                                                       past_coords.velocity,
                                                       start_coords.velocity,
                                                       end_coords.velocity,
                                                       future_coords.velocity);
      coords.velocity = CLAMP (velocity, 0.0, 1.0);

      coords.direction = start_coords.direction + dir_step * n;

      coords.direction = coords.direction - floor (coords.direction);

      g_array_append_val (*ret_coords, coords);

      if (ret_params)
        g_array_append_val (*ret_params, p);
    }
}

static gdouble
picman_coords_get_catmull_spline_point (const gdouble  t,
                                      const gdouble  p0,
                                      const gdouble  p1,
                                      const gdouble  p2,
                                      const gdouble  p3)
{

  return ((((-t + 2.0) * t - 1.0) * t / 2.0)        * p0 +
          ((((3.0 * t - 5.0) * t) * t + 2.0) / 2.0) * p1 +
          (((-3.0 * t + 4.0) * t + 1.0) * t / 2.0)  * p2 +
          (((t - 1) * t * t) / 2.0)                 * p3);
}
