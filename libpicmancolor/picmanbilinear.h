/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_COLOR_H_INSIDE__) && !defined (PICMAN_COLOR_COMPILATION)
#error "Only <libpicmancolor/picmancolor.h> can be included directly."
#endif

#ifndef __PICMAN_BILINEAR_H__
#define __PICMAN_BILINEAR_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*  bilinear interpolation functions taken from LibGCK  */


gdouble   picman_bilinear          (gdouble    x,
                                  gdouble    y,
                                  gdouble   *values);
guchar    picman_bilinear_8        (gdouble    x,
                                  gdouble    y,
                                  guchar    *values);
guint16   picman_bilinear_16       (gdouble    x,
                                  gdouble    y,
                                  guint16   *values);
guint32   picman_bilinear_32       (gdouble    x,
                                  gdouble    y,
                                  guint32   *values);
PicmanRGB   picman_bilinear_rgb      (gdouble    x,
                                  gdouble    y,
                                  PicmanRGB   *values);
PicmanRGB   picman_bilinear_rgba     (gdouble    x,
                                  gdouble    y,
                                  PicmanRGB   *values);
void      picman_bilinear_pixels_8 (guchar    *dest,
                                  gdouble    x,
                                  gdouble    y,
                                  guint      bpp,
                                  gboolean   has_alpha,
                                  guchar   **values);

G_END_DECLS

#endif  /* __PICMAN_BILINEAR_H__ */
