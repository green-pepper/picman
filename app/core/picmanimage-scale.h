/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattisbvf
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

#ifndef __PICMAN_IMAGE_SCALE_H__
#define __PICMAN_IMAGE_SCALE_H__


void   picman_image_scale         (PicmanImage             *image,
                                 gint                   new_width,
                                 gint                   new_height,
                                 PicmanInterpolationType  interpolation_type,
                                 PicmanProgress          *progress);

PicmanImageScaleCheckType
       picman_image_scale_check   (const PicmanImage       *image,
                                 gint                   new_width,
                                 gint                   new_height,
                                 gint64                 max_memsize,
                                 gint64                *new_memsize);


#endif /* __PICMAN_IMAGE_SCALE_H__ */
