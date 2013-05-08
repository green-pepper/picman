/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanregioniterator.h
 * Contains all kinds of miscellaneous routines factored out from different
 * plug-ins. They stay here until their API has crystalized a bit and we can
 * put them into the file where they belong (Maurits Rijk
 * <lpeek.mrijk@consunet.nl> if you want to blame someone for this mess)
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

#ifndef __PICMAN_REGION_ITERATOR_H__
#define __PICMAN_REGION_ITERATOR_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */

typedef struct _PicmanRgnIterator  PicmanRgnIterator;

typedef void   (* PicmanRgnFunc1)       (const guchar *src,
                                       gint          bpp,
                                       gpointer      data);
typedef void   (* PicmanRgnFunc2)       (const guchar *src,
                                       guchar       *dest,
                                       gint          bpp,
                                       gpointer      data);
typedef void   (* PicmanRgnFuncSrc)     (gint          x,
                                       gint          y,
                                       const guchar *src,
                                       gint          bpp,
                                       gpointer      data);
typedef void   (* PicmanRgnFuncDest)    (gint          x,
                                       gint          y,
                                       guchar       *dest,
                                       gint          bpp,
                                       gpointer      data);
typedef void   (* PicmanRgnFuncSrcDest) (gint          x,
                                       gint          y,
                                       const guchar *src,
                                       guchar       *dest,
                                       gint          bpp,
                                       gpointer      data);

PICMAN_DEPRECATED_FOR(GeglBufferIterator)
PicmanRgnIterator * picman_rgn_iterator_new      (PicmanDrawable      *drawable,
                                              PicmanRunMode        unused);
PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterator_free     (PicmanRgnIterator   *iter);
PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterator_src      (PicmanRgnIterator   *iter,
                                              PicmanRgnFuncSrc     func,
                                              gpointer           data);
PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterator_dest     (PicmanRgnIterator   *iter,
                                              PicmanRgnFuncDest    func,
                                              gpointer           data);
PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterator_src_dest (PicmanRgnIterator   *iter,
                                              PicmanRgnFuncSrcDest func,
                                              gpointer           data);


PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterate1          (PicmanDrawable      *drawable,
                                              PicmanRunMode        unused,
                                              PicmanRgnFunc1       func,
                                              gpointer           data);

PICMAN_DEPRECATED_FOR(GeglBufferIterator)
void              picman_rgn_iterate2          (PicmanDrawable      *drawable,
                                              PicmanRunMode        unused,
                                              PicmanRgnFunc2       func,
                                              gpointer           data);

G_END_DECLS

#endif /* __PICMAN_REGION_ITERATOR_H__ */
