/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixbuf.h
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picman.h> can be included directly."
#endif

#ifndef __LIBPICMAN_PICMAN_PIXBUF_H__
#define __LIBPICMAN_PICMAN_PIXBUF_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


typedef enum
{
  PICMAN_PIXBUF_KEEP_ALPHA,
  PICMAN_PIXBUF_SMALL_CHECKS,
  PICMAN_PIXBUF_LARGE_CHECKS
} PicmanPixbufTransparency;


GdkPixbuf * picman_image_get_thumbnail        (gint32                  image_ID,
                                             gint                    width,
                                             gint                    height,
                                             PicmanPixbufTransparency  alpha);
GdkPixbuf * picman_drawable_get_thumbnail     (gint32                  drawable_ID,
                                             gint                    width,
                                             gint                    height,
                                             PicmanPixbufTransparency  alpha);
GdkPixbuf * picman_drawable_get_sub_thumbnail (gint32                  drawable_ID,
                                             gint                    src_x,
                                             gint                    src_y,
                                             gint                    src_width,
                                             gint                    src_height,
                                             gint                    dest_width,
                                             gint                    dest_height,
                                             PicmanPixbufTransparency  alpha);

G_END_DECLS

#endif /* __LIBPICMAN_PICMAN_PIXBUF_H__ */
