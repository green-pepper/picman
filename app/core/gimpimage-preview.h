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

#ifndef __PICMAN_IMAGE_PREVIEW_H__
#define __PICMAN_IMAGE_PREVIEW_H__


/*
 *  virtual functions of PicmanImage -- dont't call directly
 */

void          picman_image_get_preview_size (PicmanViewable *viewable,
                                           gint          size,
                                           gboolean      is_popup,
                                           gboolean      dot_for_dot,
                                           gint         *width,
                                           gint         *height);
gboolean      picman_image_get_popup_size   (PicmanViewable *viewable,
                                           gint          width,
                                           gint          height,
                                           gboolean      dot_for_dot,
                                           gint         *popup_width,
                                           gint         *popup_height);
PicmanTempBuf * picman_image_get_new_preview  (PicmanViewable *viewable,
                                           PicmanContext  *context,
                                           gint          width,
                                           gint          height);


#endif /* __PICMAN_IMAGE_PREVIEW_H__ */
