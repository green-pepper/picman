/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancoords-interpolate.h
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

#ifndef __PICMAN_COORDS_INTERPOLATE_H__
#define __PICMAN_COORDS_INTERPOLATE_H__

void      picman_coords_interpolate_bezier  (const PicmanCoords bezier_pt1,
                                           const PicmanCoords bezier_pt2,
                                           const PicmanCoords bezier_pt3,
                                           const PicmanCoords bezier_pt4,
                                           gdouble          precision,
                                           GArray           **ret_coords,
                                           GArray           **ret_params);

gboolean  picman_coords_bezier_is_straight  (const PicmanCoords bezier_pt1,
                                           const PicmanCoords bezier_pt2,
                                           const PicmanCoords bezier_pt3,
                                           const PicmanCoords bezier_pt4,
                                           gdouble          precision);

void      picman_coords_interpolate_catmull (const PicmanCoords catmul_pt1,
                                           const PicmanCoords catmul_pt2,
                                           const PicmanCoords catmul_pt3,
                                           const PicmanCoords catmul_pt4,
                                           gdouble          precision,
                                           GArray           **ret_coords,
                                           GArray           **ret_params);

#endif /* __PICMAN_COORDS_INTERPOLATE_H__ */
