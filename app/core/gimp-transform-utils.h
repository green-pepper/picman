/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#ifndef __PICMAN_TRANSFORM_UTILS_H__
#define __PICMAN_TRANSFORM_UTILS_H__


void       picman_transform_get_rotate_center    (gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                gboolean             auto_center,
                                                gdouble             *center_x,
                                                gdouble             *center_y);
void       picman_transform_get_flip_axis        (gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                PicmanOrientationType  flip_type,
                                                gboolean             auto_center,
                                                gdouble             *axis);

void       picman_transform_matrix_flip          (PicmanMatrix3         *matrix,
                                                PicmanOrientationType  flip_type,
                                                gdouble              axis);
void       picman_transform_matrix_flip_free     (PicmanMatrix3         *matrix,
                                                gdouble              x1,
                                                gdouble              y1,
                                                gdouble              x2,
                                                gdouble              y2);
void       picman_transform_matrix_rotate        (PicmanMatrix3         *matrix,
                                                PicmanRotationType     rotate_type,
                                                gdouble              center_x,
                                                gdouble              center_y);
void       picman_transform_matrix_rotate_rect   (PicmanMatrix3         *matrix,
                                                gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                gdouble              angle);
void       picman_transform_matrix_rotate_center (PicmanMatrix3         *matrix,
                                                gdouble              center_x,
                                                gdouble              center_y,
                                                gdouble              angle);
void       picman_transform_matrix_scale         (PicmanMatrix3         *matrix,
                                                gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                gdouble              t_x,
                                                gdouble              t_y,
                                                gdouble              t_width,
                                                gdouble              t_height);
void       picman_transform_matrix_shear         (PicmanMatrix3         *matrix,
                                                gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                PicmanOrientationType  orientation,
                                                gdouble              amount);
void       picman_transform_matrix_perspective   (PicmanMatrix3         *matrix,
                                                gint                 x,
                                                gint                 y,
                                                gint                 width,
                                                gint                 height,
                                                gdouble              t_x1,
                                                gdouble              t_y1,
                                                gdouble              t_x2,
                                                gdouble              t_y2,
                                                gdouble              t_x3,
                                                gdouble              t_y3,
                                                gdouble              t_x4,
                                                gdouble              t_y4);

gboolean   picman_transform_polygon_is_convex    (gdouble              x1,
                                                gdouble              y1,
                                                gdouble              x2,
                                                gdouble              y2,
                                                gdouble              x3,
                                                gdouble              y3,
                                                gdouble              x4,
                                                gdouble              y4);


#endif  /*  __PICMAN_TRANSFORM_UTILS_H__  */
