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

#ifndef __PICMAN_DRAWABLE__PREVIEW_H__
#define __PICMAN_DRAWABLE__PREVIEW_H__


/*
 *  virtual function of PicmanDrawable -- dont't call directly
 */
PicmanTempBuf * picman_drawable_get_new_preview    (PicmanViewable *viewable,
                                                PicmanContext  *context,
                                                gint          width,
                                                gint          height);

/*
 *  normal functions (no virtuals)
 */
const Babl  * picman_drawable_get_preview_format (PicmanDrawable *drawable);
PicmanTempBuf * picman_drawable_get_sub_preview    (PicmanDrawable *drawable,
                                                gint          src_x,
                                                gint          src_y,
                                                gint          src_width,
                                                gint          src_height,
                                                gint          dest_width,
                                                gint          dest_height);


#endif /* __PICMAN_DRAWABLE__PREVIEW_H__ */
