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

#ifndef __PICMAN_IMAGE_PICK_LAYER_H__
#define __PICMAN_IMAGE_PICK_LAYER_H__


PicmanLayer     * picman_image_pick_layer           (const PicmanImage *image,
                                                 gint             x,
                                                 gint             y);
PicmanLayer     * picman_image_pick_layer_by_bounds (const PicmanImage *image,
                                                 gint             x,
                                                 gint             y);
PicmanTextLayer * picman_image_pick_text_layer      (const PicmanImage *image,
                                                 gint             x,
                                                 gint             y);


#endif /* __PICMAN_IMAGE_PICK_LAYER_H__ */
