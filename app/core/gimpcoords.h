/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancoords.h
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#ifndef __PICMAN_COORDS_H__
#define __PICMAN_COORDS_H__


void     picman_coords_mix            (const gdouble     amul,
                                     const PicmanCoords *a,
                                     const gdouble     bmul,
                                     const PicmanCoords *b,
                                     PicmanCoords       *ret_val);
void     picman_coords_average        (const PicmanCoords *a,
                                     const PicmanCoords *b,
                                     PicmanCoords       *ret_average);
void     picman_coords_add            (const PicmanCoords *a,
                                     const PicmanCoords *b,
                                     PicmanCoords       *ret_add);
void     picman_coords_difference     (const PicmanCoords *a,
                                     const PicmanCoords *b,
                                     PicmanCoords       *difference);
void     picman_coords_scale          (const gdouble     f,
                                     const PicmanCoords *a,
                                     PicmanCoords       *ret_product);

gdouble  picman_coords_scalarprod     (const PicmanCoords *a,
                                     const PicmanCoords *b);
gdouble  picman_coords_length         (const PicmanCoords *a);
gdouble  picman_coords_length_squared (const PicmanCoords *a);
gdouble  picman_coords_manhattan_dist (const PicmanCoords *a,
                                     const PicmanCoords *b);

gboolean picman_coords_equal          (const PicmanCoords *a,
                                     const PicmanCoords *b);

gdouble  picman_coords_direction      (const PicmanCoords *a,
                                     const PicmanCoords *b);


#endif /* __PICMAN_COORDS_H__ */
