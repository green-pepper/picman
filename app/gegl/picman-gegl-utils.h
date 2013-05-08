/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-gegl-utils.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_GEGL_UTILS_H__
#define __PICMAN_GEGL_UTILS_H__


const gchar * picman_interpolation_to_gegl_filter (PicmanInterpolationType  interpolation) G_GNUC_CONST;

GType         picman_gegl_get_op_enum_type        (const gchar           *operation,
                                                 const gchar           *property);

GeglColor   * picman_gegl_color_new               (const PicmanRGB         *rgb);

void          picman_gegl_progress_connect        (GeglNode              *node,
                                                 PicmanProgress          *progress,
                                                 const gchar           *text);


#endif /* __PICMAN_GEGL_UTILS_H__ */
