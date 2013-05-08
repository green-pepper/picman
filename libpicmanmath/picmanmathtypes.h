/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmathtypes.h
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

#ifndef __PICMAN_MATH_TYPES_H__
#define __PICMAN_MATH_TYPES_H__


#include <libpicmanbase/picmanbasetypes.h>


G_BEGIN_DECLS

typedef struct _PicmanMatrix2 PicmanMatrix2;
typedef struct _PicmanMatrix3 PicmanMatrix3;
typedef struct _PicmanMatrix4 PicmanMatrix4;

/**
 * PicmanMatrix2
 * @coeff: the coefficients
 *
 * A two by two matrix.
 **/
struct _PicmanMatrix2
{
  gdouble coeff[2][2];
};

/**
 * PicmanMatrix3
 * @coeff: the coefficients
 *
 * A three by three matrix.
 **/
struct _PicmanMatrix3
{
  gdouble coeff[3][3];
};

/**
 * PicmanMatrix4
 * @coeff: the coefficients
 *
 * A four by four matrix.
 **/
struct _PicmanMatrix4
{
  gdouble coeff[4][4];
};


typedef struct _PicmanVector2 PicmanVector2;
typedef struct _PicmanVector3 PicmanVector3;
typedef struct _PicmanVector4 PicmanVector4;

/**
 * PicmanVector2:
 * @x: the x axis
 * @y: the y axis
 *
 * A two dimensional vector.
 **/
struct _PicmanVector2
{
  gdouble x, y;
};

/**
 * PicmanVector3:
 * @x: the x axis
 * @y: the y axis
 * @z: the z axis
 *
 * A three dimensional vector.
 **/
struct _PicmanVector3
{
  gdouble x, y, z;
};

/**
 * PicmanVector4:
 * @x: the x axis
 * @y: the y axis
 * @z: the z axis
 * @w: the w axis
 *
 * A four dimensional vector.
 **/
struct _PicmanVector4
{
  gdouble x, y, z, w;
};


G_END_DECLS

#endif /* __PICMAN_MATH_TYPES_H__ */
