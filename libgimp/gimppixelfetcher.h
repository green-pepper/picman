/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpixelfetcher.h
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

#ifndef __PICMAN_PIXEL_FETCHER_H__
#define __PICMAN_PIXEL_FETCHER_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


typedef enum
{
  PICMAN_PIXEL_FETCHER_EDGE_NONE,
  PICMAN_PIXEL_FETCHER_EDGE_WRAP,
  PICMAN_PIXEL_FETCHER_EDGE_SMEAR,
  PICMAN_PIXEL_FETCHER_EDGE_BLACK,
  PICMAN_PIXEL_FETCHER_EDGE_BACKGROUND
} PicmanPixelFetcherEdgeMode;


typedef struct _PicmanPixelFetcher PicmanPixelFetcher;


PICMAN_DEPRECATED
PicmanPixelFetcher * picman_pixel_fetcher_new       (PicmanDrawable     *drawable,
                                                 gboolean          shadow);
PICMAN_DEPRECATED
void               picman_pixel_fetcher_destroy   (PicmanPixelFetcher *pf);

PICMAN_DEPRECATED
void   picman_pixel_fetcher_set_edge_mode (PicmanPixelFetcher         *pf,
                                         PicmanPixelFetcherEdgeMode  mode);
PICMAN_DEPRECATED
void   picman_pixel_fetcher_set_bg_color  (PicmanPixelFetcher         *pf,
                                         const PicmanRGB            *color);

PICMAN_DEPRECATED
void   picman_pixel_fetcher_get_pixel     (PicmanPixelFetcher         *pf,
                                         gint                      x,
                                         gint                      y,
                                         guchar                   *pixel);
PICMAN_DEPRECATED
void   picman_pixel_fetcher_put_pixel     (PicmanPixelFetcher         *pf,
                                         gint                      x,
                                         gint                      y,
                                         const guchar             *pixel);

G_END_DECLS

#endif /* __PICMAN_PIXEL_FETCHER_H__ */
