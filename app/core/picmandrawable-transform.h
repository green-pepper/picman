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

#ifndef __PICMAN_DRAWABLE_TRANSFORM_H__
#define __PICMAN_DRAWABLE_TRANSFORM_H__


GeglBuffer  * picman_drawable_transform_buffer_affine (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     GeglBuffer             *orig_buffer,
                                                     gint                    orig_offset_x,
                                                     gint                    orig_offset_y,
                                                     const PicmanMatrix3      *matrix,
                                                     PicmanTransformDirection  direction,
                                                     PicmanInterpolationType   interpolation_type,
                                                     gint                    recursion_level,
                                                     PicmanTransformResize     clip_result,
                                                     gint                   *new_offset_x,
                                                     gint                   *new_offset_y,
                                                     PicmanProgress           *progress);
GeglBuffer  * picman_drawable_transform_buffer_flip   (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     GeglBuffer             *orig_buffer,
                                                     gint                    orig_offset_x,
                                                     gint                    orig_offset_y,
                                                     PicmanOrientationType     flip_type,
                                                     gdouble                 axis,
                                                     gboolean                clip_result,
                                                     gint                   *new_offset_x,
                                                     gint                   *new_offset_y);

GeglBuffer  * picman_drawable_transform_buffer_rotate (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     GeglBuffer             *buffer,
                                                     gint                    orig_offset_x,
                                                     gint                    orig_offset_y,
                                                     PicmanRotationType        rotate_type,
                                                     gdouble                 center_x,
                                                     gdouble                 center_y,
                                                     gboolean                clip_result,
                                                     gint                   *new_offset_x,
                                                     gint                   *new_offset_y);

PicmanDrawable * picman_drawable_transform_affine       (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     const PicmanMatrix3      *matrix,
                                                     PicmanTransformDirection  direction,
                                                     PicmanInterpolationType   interpolation_type,
                                                     gint                    recursion_level,
                                                     PicmanTransformResize     clip_result,
                                                     PicmanProgress           *progress);

PicmanDrawable * picman_drawable_transform_flip         (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     PicmanOrientationType     flip_type,
                                                     gdouble                 axis,
                                                     gboolean                clip_result);

PicmanDrawable * picman_drawable_transform_rotate       (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     PicmanRotationType        rotate_type,
                                                     gdouble                 center_x,
                                                     gdouble                 center_y,
                                                     gboolean                clip_result);

GeglBuffer   * picman_drawable_transform_cut          (PicmanDrawable           *drawable,
                                                     PicmanContext            *context,
                                                     gint                   *offset_x,
                                                     gint                   *offset_y,
                                                     gboolean               *new_layer);
PicmanDrawable * picman_drawable_transform_paste        (PicmanDrawable           *drawable,
                                                     GeglBuffer             *buffer,
                                                     gint                    offset_x,
                                                     gint                    offset_y,
                                                     gboolean                new_layer);


#endif  /*  __PICMAN_DRAWABLE_TRANSFORM_H__  */
