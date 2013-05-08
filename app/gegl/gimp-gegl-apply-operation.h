/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-apply-operation.h
 * Copyright (C) 2012 Øyvind Kolås <pippin@picman.org>
 *                    Sven Neumann <sven@picman.org>
 *                    Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_GEGL_APPLY_OPERATION_H__
#define __PICMAN_GEGL_APPLY_OPERATION_H__


/*  generic function, also used by the specific ones below  */

void   picman_gegl_apply_operation       (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglNode              *operation,
                                        GeglBuffer            *dest_buffer,
                                        const GeglRectangle   *dest_rect);


/*  apply specific operations  */

void   picman_gegl_apply_color_reduction (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        gint                   bits,
                                        gint                   dither_type);

void   picman_gegl_apply_flatten         (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        const PicmanRGB         *background);

void   picman_gegl_apply_feather         (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        gdouble                radius_x,
                                        gdouble                radius_y);

void   picman_gegl_apply_gaussian_blur   (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        gdouble                std_dev_x,
                                        gdouble                std_dev_y);

void   picman_gegl_apply_invert          (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer);

void   picman_gegl_apply_opacity         (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        GeglBuffer            *mask,
                                        gint                   mask_offset_x,
                                        gint                   mask_offset_y,
                                        gdouble                opacity);

void   picman_gegl_apply_scale           (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        PicmanInterpolationType  interpolation_type,
                                        gdouble                x,
                                        gdouble                y);

void   picman_gegl_apply_set_alpha       (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        gdouble                value);

void   picman_gegl_apply_threshold       (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        gdouble                value);

void   picman_gegl_apply_transform       (GeglBuffer            *src_buffer,
                                        PicmanProgress          *progress,
                                        const gchar           *undo_desc,
                                        GeglBuffer            *dest_buffer,
                                        PicmanInterpolationType  interpolation_type,
                                        PicmanMatrix3           *transform);


#endif /* __PICMAN_GEGL_APPLY_OPERATION_H__ */
