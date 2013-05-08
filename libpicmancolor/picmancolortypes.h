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

#ifndef __PICMAN_COLOR_TYPES_H__
#define __PICMAN_COLOR_TYPES_H__


#include <libpicmanbase/picmanbasetypes.h>


G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


typedef struct _PicmanColorManaged PicmanColorManaged;  /* dummy typedef */

/*  usually we don't keep the structure definitions in the types file
 *  but PicmanRGB appears in too many header files...
 */

typedef struct _PicmanRGB  PicmanRGB;
typedef struct _PicmanHSV  PicmanHSV;
typedef struct _PicmanHSL  PicmanHSL;
typedef struct _PicmanCMYK PicmanCMYK;

/**
 * PicmanRGB:
 * @r: the red component
 * @g: the green component
 * @b: the blue component
 * @a: the alpha component
 *
 * Used to keep RGB and RGBA colors. All components are in a range of
 * [0.0..1.0].
 **/
struct _PicmanRGB
{
  gdouble r, g, b, a;
};

/**
 * PicmanHSV:
 * @h: the hue component
 * @s: the saturation component
 * @v: the value component
 * @a: the alpha component
 *
 * Used to keep HSV and HSVA colors. All components are in a range of
 * [0.0..1.0].
 **/
struct _PicmanHSV
{
  gdouble h, s, v, a;
};

/**
 * PicmanHSL:
 * @h: the hue component
 * @s: the saturation component
 * @l: the lightness component
 * @a: the alpha component
 *
 * Used to keep HSL and HSLA colors. All components are in a range of
 * [0.0..1.0].
 **/
struct _PicmanHSL
{
  gdouble h, s, l, a;
};

/**
 * PicmanCMYK:
 * @c: the cyan component
 * @m: the magenta component
 * @y: the yellow component
 * @k: the black component
 * @a: the alpha component
 *
 * Used to keep CMYK and CMYKA colors. All components are in a range
 * of [0.0..1.0]. An alpha value is somewhat useless in the CMYK
 * colorspace, but we keep one around anyway so color conversions
 * going to CMYK and back can preserve alpha.
 **/
struct _PicmanCMYK
{
  gdouble c, m, y, k, a;
};


typedef void (* PicmanRenderFunc)   (gdouble   x,
                                   gdouble   y,
                                   PicmanRGB  *color,
                                   gpointer  data);
typedef void (* PicmanPutPixelFunc) (gint      x,
                                   gint      y,
                                   PicmanRGB  *color,
                                   gpointer  data);
typedef void (* PicmanProgressFunc) (gint      min,
                                   gint      max,
                                   gint      current,
                                   gpointer  data);


G_END_DECLS

#endif  /* __PICMAN_COLOR_TYPES_H__ */
