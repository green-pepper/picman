/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_IMAGE_QUICK_MASK_H__
#define __PICMAN_IMAGE_QUICK_MASK_H__


/*  don't change this string, it's used to identify the Quick Mask
 *  when opening files.
 */
#define PICMAN_IMAGE_QUICK_MASK_NAME "Qmask"


void          picman_image_set_quick_mask_state    (PicmanImage       *image,
                                                  gboolean         active);
gboolean      picman_image_get_quick_mask_state    (const PicmanImage *image);

void          picman_image_set_quick_mask_color    (PicmanImage       *image,
                                                  const PicmanRGB   *color);
void          picman_image_get_quick_mask_color    (const PicmanImage *image,
                                                  PicmanRGB         *color);

PicmanChannel * picman_image_get_quick_mask          (const PicmanImage *image);

void          picman_image_quick_mask_invert       (PicmanImage       *image);
gboolean      picman_image_get_quick_mask_inverted (const PicmanImage *image);


#endif /* __PICMAN_IMAGE_QUICK_MASK_H__ */
