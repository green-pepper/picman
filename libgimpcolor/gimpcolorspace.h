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

#ifndef __PICMAN_COLOR_SPACE_H__
#define __PICMAN_COLOR_SPACE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*  Color conversion routines  */


/*  PicmanRGB function  */

void   picman_rgb_to_hsv          (const PicmanRGB  *rgb,
                                 PicmanHSV        *hsv);
void   picman_rgb_to_hsl          (const PicmanRGB  *rgb,
                                 PicmanHSL        *hsl);
void   picman_rgb_to_cmyk         (const PicmanRGB  *rgb,
                                 gdouble         pullout,
                                 PicmanCMYK       *cmyk);

void   picman_hsv_to_rgb          (const PicmanHSV  *hsv,
                                 PicmanRGB        *rgb);
void   picman_hsl_to_rgb          (const PicmanHSL  *hsl,
                                 PicmanRGB        *rgb);
void   picman_cmyk_to_rgb         (const PicmanCMYK *cmyk,
                                 PicmanRGB        *rgb);

void   picman_rgb_to_hwb          (const PicmanRGB  *rgb,
                                 gdouble        *hue,
                                 gdouble        *whiteness,
                                 gdouble        *blackness);
void   picman_hwb_to_rgb          (gdouble         hue,
                                 gdouble         whiteness,
                                 gdouble         blackness,
                                 PicmanRGB        *rgb);


/*  gint functions  */

void    picman_rgb_to_hsv_int     (gint    *red         /* returns hue        */,
                                 gint    *green       /* returns saturation */,
                                 gint    *blue        /* returns value      */);
void    picman_hsv_to_rgb_int     (gint    *hue         /* returns red        */,
                                 gint    *saturation  /* returns green      */,
                                 gint    *value       /* returns blue       */);

void    picman_rgb_to_cmyk_int    (gint    *red         /* returns cyan       */,
                                 gint    *green       /* returns magenta    */,
                                 gint    *blue        /* returns yellow     */,
                                 gint    *pullout     /* returns black      */);
void    picman_cmyk_to_rgb_int    (gint    *cyan        /* returns red        */,
                                 gint    *magenta     /* returns green      */,
                                 gint    *yellow      /* returns blue       */,
                                 gint    *black       /* not changed        */);

void    picman_rgb_to_hsl_int     (gint    *red         /* returns hue        */,
                                 gint    *green       /* returns saturation */,
                                 gint    *blue        /* returns lightness  */);
gint    picman_rgb_to_l_int       (gint     red,
                                 gint     green,
                                 gint     blue);
void    picman_hsl_to_rgb_int     (gint    *hue         /* returns red        */,
                                 gint    *saturation  /* returns green      */,
                                 gint    *lightness   /* returns blue       */);


/*  gdouble functions  */

void    picman_rgb_to_hsv4        (const guchar *rgb,
                                 gdouble      *hue,
                                 gdouble      *saturation,
                                 gdouble      *value);
void    picman_hsv_to_rgb4        (guchar       *rgb,
                                 gdouble       hue,
                                 gdouble       saturation,
                                 gdouble       value);


G_END_DECLS

#endif  /* __PICMAN_COLOR_SPACE_H__ */
