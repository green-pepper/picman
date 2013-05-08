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

#ifndef __PICMAN_HSL_H__
#define __PICMAN_HSL_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*
 * PICMAN_TYPE_HSL
 */

#define PICMAN_TYPE_HSL       (picman_hsl_get_type ())

GType   picman_hsl_get_type   (void) G_GNUC_CONST;

void    picman_hsl_set        (PicmanHSL *hsl,
                             gdouble  h,
                             gdouble  s,
                             gdouble  l);


G_END_DECLS

#endif  /* __PICMAN_HSL_H__ */
