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

#ifndef __PICMAN_CMYK_H__
#define __PICMAN_CMYK_H__

G_BEGIN_DECLS


/* For information look into the C source or the html documentation */


/*
 * PICMAN_TYPE_CMYK
 */

#define PICMAN_TYPE_CMYK       (picman_cmyk_get_type ())

GType   picman_cmyk_get_type   (void) G_GNUC_CONST;

void    picman_cmyk_set        (PicmanCMYK       *cmyk,
                              gdouble         cyan,
                              gdouble         magenta,
                              gdouble         yellow,
                              gdouble         black);
void    picman_cmyk_set_uchar  (PicmanCMYK       *cmyk,
                              guchar          cyan,
                              guchar          magenta,
                              guchar          yellow,
                              guchar          black);
void    picman_cmyk_get_uchar  (const PicmanCMYK *cmyk,
                              guchar         *cyan,
                              guchar         *magenta,
                              guchar         *yellow,
                              guchar         *black);

void    picman_cmyka_set       (PicmanCMYK       *cmyka,
                              gdouble         cyan,
                              gdouble         magenta,
                              gdouble         yellow,
                              gdouble         black,
                              gdouble         alpha);
void    picman_cmyka_set_uchar (PicmanCMYK       *cmyka,
                              guchar          cyan,
                              guchar          magenta,
                              guchar          yellow,
                              guchar          black,
                              guchar          alpha);
void    picman_cmyka_get_uchar (const PicmanCMYK *cmyka,
                              guchar         *cyan,
                              guchar         *magenta,
                              guchar         *yellow,
                              guchar         *black,
                              guchar         *alpha);


G_END_DECLS

#endif  /* __PICMAN_CMYK_H__ */
