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

#ifndef __PICMAN_IMAGE_COLORMAP_H__
#define __PICMAN_IMAGE_COLORMAP_H__


#define PICMAN_IMAGE_COLORMAP_SIZE 768


void           picman_image_colormap_init            (PicmanImage       *image);
void           picman_image_colormap_dispose         (PicmanImage       *image);
void           picman_image_colormap_free            (PicmanImage       *image);

const Babl   * picman_image_colormap_get_rgb_format  (const PicmanImage *image);
const Babl   * picman_image_colormap_get_rgba_format (const PicmanImage *image);

PicmanPalette  * picman_image_get_colormap_palette     (PicmanImage       *image);

const guchar * picman_image_get_colormap             (const PicmanImage *image);
gint           picman_image_get_colormap_size        (const PicmanImage *image);
void           picman_image_set_colormap             (PicmanImage       *image,
                                                    const guchar    *colormap,
                                                    gint             n_colors,
                                                    gboolean         push_undo);

void           picman_image_get_colormap_entry       (PicmanImage       *image,
                                                    gint             color_index,
                                                    PicmanRGB         *color);
void           picman_image_set_colormap_entry       (PicmanImage       *image,
                                                    gint             color_index,
                                                    const PicmanRGB   *color,
                                                    gboolean         push_undo);

void           picman_image_add_colormap_entry       (PicmanImage       *image,
                                                    const PicmanRGB   *color);


#endif /* __PICMAN_IMAGE_COLORMAP_H__ */
