/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixelrgn.h
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

#ifndef __PICMAN_PIXEL_RGN_H__
#define __PICMAN_PIXEL_RGN_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


struct _PicmanPixelRgn
{
  guchar       *data;          /* pointer to region data */
  PicmanDrawable *drawable;      /* pointer to drawable */
  gint          bpp;           /* bytes per pixel */
  gint          rowstride;     /* bytes per pixel row */
  gint          x, y;          /* origin */
  gint          w, h;          /* width and height of region */
  guint         dirty : 1;     /* will this region be dirtied? */
  guint         shadow : 1;    /* will this region use the shadow or normal tiles */
  gint          process_count; /* used internally */
};


PICMAN_DEPRECATED_FOR(picman_drawable_get_buffer)
void      picman_pixel_rgn_init       (PicmanPixelRgn  *pr,
                                     PicmanDrawable  *drawable,
                                     gint           x,
                                     gint           y,
                                     gint           width,
                                     gint           height,
                                     gint           dirty,
                                     gint           shadow);
PICMAN_DEPRECATED
void      picman_pixel_rgn_resize     (PicmanPixelRgn  *pr,
                                     gint           x,
                                     gint           y,
                                     gint           width,
                                     gint           height);
PICMAN_DEPRECATED_FOR(gegl_buffer_sample)
void      picman_pixel_rgn_get_pixel  (PicmanPixelRgn  *pr,
                                     guchar        *buf,
                                     gint           x,
                                     gint           y);
PICMAN_DEPRECATED_FOR(gegl_buffer_get)
void      picman_pixel_rgn_get_row    (PicmanPixelRgn  *pr,
                                     guchar        *buf,
                                     gint           x,
                                     gint           y,
                                     gint           width);
PICMAN_DEPRECATED_FOR(gegl_buffer_get)
void      picman_pixel_rgn_get_col    (PicmanPixelRgn  *pr,
                                     guchar        *buf,
                                     gint           x,
                                     gint           y,
                                     gint           height);
PICMAN_DEPRECATED_FOR(gegl_buffer_get)
void      picman_pixel_rgn_get_rect   (PicmanPixelRgn  *pr,
                                     guchar        *buf,
                                     gint           x,
                                     gint           y,
                                     gint           width,
                                     gint           height);
PICMAN_DEPRECATED_FOR(gegl_buffer_set)
void      picman_pixel_rgn_set_pixel  (PicmanPixelRgn  *pr,
                                     const guchar  *buf,
                                     gint           x,
                                     gint           y);
PICMAN_DEPRECATED_FOR(gegl_buffer_set)
void      picman_pixel_rgn_set_row    (PicmanPixelRgn  *pr,
                                     const guchar  *buf,
                                     gint           x,
                                     gint           y,
                                     gint           width);
PICMAN_DEPRECATED_FOR(gegl_buffer_set)
void      picman_pixel_rgn_set_col    (PicmanPixelRgn  *pr,
                                     const guchar  *buf,
                                     gint           x,
                                     gint           y,
                                     gint           height);
PICMAN_DEPRECATED_FOR(gegl_buffer_set)
void      picman_pixel_rgn_set_rect   (PicmanPixelRgn  *pr,
                                     const guchar  *buf,
                                     gint           x,
                                     gint           y,
                                     gint           width,
                                     gint           height);
PICMAN_DEPRECATED_FOR(gegl_buffer_iterator_new)
gpointer  picman_pixel_rgns_register  (gint           nrgns,
                                     ...);
PICMAN_DEPRECATED_FOR(gegl_buffer_iterator_new)
gpointer  picman_pixel_rgns_register2 (gint           nrgns,
                                     PicmanPixelRgn **prs);
PICMAN_DEPRECATED_FOR(gegl_buffer_iterator_next)
gpointer  picman_pixel_rgns_process   (gpointer       pri_ptr);


G_END_DECLS

#endif /* __PICMAN_PIXEL_RGN_H__ */
