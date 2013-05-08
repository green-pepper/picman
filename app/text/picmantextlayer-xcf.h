/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_LAYER_XCF_H__
#define __PICMAN_TEXT_LAYER_XCF_H__


gboolean  picman_text_layer_xcf_load_hack    (PicmanLayer     **layer);

void      picman_text_layer_xcf_save_prepare (PicmanTextLayer  *text_layer);

guint32   picman_text_layer_get_xcf_flags    (PicmanTextLayer  *text_layer);
void      picman_text_layer_set_xcf_flags    (PicmanTextLayer  *text_layer,
                                            guint32         flags);


#endif /* __PICMAN_TEXT_LAYER_XCF_H__ */
