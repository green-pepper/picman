/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanvector.c
 *
 * The picman_vector* functions were taken from:
 * GCK - The General Convenience Kit
 * Copyright (C) 1996 Tom Bech
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

/**********************************************/
/* A little collection of useful vector stuff */
/**********************************************/

#include "config.h"

#include <glib-object.h>

#include "picmanmath.h"


/**
 * SECTION: picmanvector
 * @title: PicmanVector
 * @short_description: Utilities to set up and manipulate vectors.
 * @see_also: #PicmanMatrix2, #PicmanMatrix3, #PicmanMatrix4
 *
 * Utilities to set up and manipulate vectors.
 **/


/*************************/
/* Some useful constants */
/*************************/

static const PicmanVector2 picman_vector2_zero =   { 0.0, 0.0 };
static const PicmanVector2 picman_vector2_unit_x = { 1.0, 0.0 };
static const PicmanVector2 picman_vector2_unit_y = { 0.0, 1.0 };

static const PicmanVector3 picman_vector3_zero =   { 0.0, 0.0, 0.0 };
static const PicmanVector3 picman_vector3_unit_x = { 1.0, 0.0, 0.0 };
static const PicmanVector3 picman_vector3_unit_y = { 0.0, 1.0, 0.0 };
static const PicmanVector3 picman_vector3_unit_z = { 0.0, 0.0, 1.0 };

/**************************************/
/* Two   dimensional vector functions */
/**************************************/

/**
 * picman_vector2_new:
 * @x: the X coordinate.
 * @y: the Y coordinate.
 *
 * Creates a #PicmanVector2 of coordinates @x and @y.
 *
 * Returns: the resulting #PicmanVector2.
 **/
PicmanVector2
picman_vector2_new (gdouble x,
                  gdouble y)
{
  PicmanVector2 vector;

  vector.x = x;
  vector.y = y;

  return vector;
}

/**
 * picman_vector2_set:
 * @vector: a pointer to a #PicmanVector2.
 * @x: the X coordinate.
 * @y: the Y coordinate.
 *
 * Sets the X and Y coordinates of @vector to @x and @y.
 **/
void
picman_vector2_set (PicmanVector2 *vector,
                  gdouble      x,
                  gdouble      y)
{
  vector->x = x;
  vector->y = y;
}

/**
 * picman_vector2_length:
 * @vector: a pointer to a #PicmanVector2.
 *
 * Computes the length of a 2D vector.
 *
 * Returns: the length of @vector (a positive gdouble).
 **/
gdouble
picman_vector2_length (const PicmanVector2 *vector)
{
  return (sqrt (vector->x * vector->x + vector->y * vector->y));
}

/**
 * picman_vector2_length_val:
 * @vector: a #PicmanVector2.
 *
 * This function is identical to picman_vector2_length() but the
 * vector is passed by value rather than by reference.
 *
 * Returns: the length of @vector (a positive gdouble).
 **/
gdouble
picman_vector2_length_val (PicmanVector2 vector)
{
  return (sqrt (vector.x * vector.x + vector.y * vector.y));
}

/**
 * picman_vector2_mul:
 * @vector: a pointer to a #PicmanVector2.
 * @factor: a scalar.
 *
 * Multiplies each component of the @vector by @factor. Note that this
 * is equivalent to multiplying the vectors length by @factor.
 **/
void
picman_vector2_mul (PicmanVector2 *vector,
                  gdouble      factor)
{
  vector->x *= factor;
  vector->y *= factor;
}

/**
 * picman_vector2_mul_val:
 * @vector: a #PicmanVector2.
 * @factor: a scalar.
 *
 * This function is identical to picman_vector2_mul() but the vector is
 * passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector2.
 **/
PicmanVector2
picman_vector2_mul_val (PicmanVector2 vector,
                      gdouble     factor)
{
  PicmanVector2 result;

  result.x = vector.x * factor;
  result.y = vector.y * factor;

  return result;
}


/**
 * picman_vector2_normalize:
 * @vector: a pointer to a #PicmanVector2.
 *
 * Normalizes the @vector so the length of the @vector is 1.0. The nul
 * vector will not be changed.
 **/
void
picman_vector2_normalize (PicmanVector2 *vector)
{
  gdouble len;

  len = picman_vector2_length (vector);

  if (len != 0.0)
    {
      len = 1.0 / len;
      vector->x *= len;
      vector->y *= len;
    }
  else
    {
      *vector = picman_vector2_zero;
    }
}

/**
 * picman_vector2_normalize_val:
 * @vector: a #PicmanVector2.
 *
 * This function is identical to picman_vector2_normalize() but the
 * vector is passed by value rather than by reference.
 *
 * Returns: a #PicmanVector2 parallel to @vector, pointing in the same
 * direction but with a length of 1.0.
 **/
PicmanVector2
picman_vector2_normalize_val (PicmanVector2 vector)
{
  PicmanVector2 normalized;
  gdouble     len;

  len = picman_vector2_length_val (vector);

  if (len != 0.0)
    {
      len = 1.0 / len;
      normalized.x = vector.x * len;
      normalized.y = vector.y * len;
      return normalized;
    }
  else
    {
      return picman_vector2_zero;
    }
}

/**
 * picman_vector2_neg:
 * @vector: a pointer to a #PicmanVector2.
 *
 * Negates the @vector (i.e. negate all its coordinates).
 **/
void
picman_vector2_neg (PicmanVector2 *vector)
{
  vector->x *= -1.0;
  vector->y *= -1.0;
}

/**
 * picman_vector2_neg_val:
 * @vector: a #PicmanVector2.
 *
 * This function is identical to picman_vector2_neg() but the vector
 * is passed by value rather than by reference.
 *
 * Returns: the negated #PicmanVector2.
 **/
PicmanVector2
picman_vector2_neg_val (PicmanVector2 vector)
{
  PicmanVector2 result;

  result.x = vector.x * -1.0;
  result.y = vector.y * -1.0;

  return result;
}

/**
 * picman_vector2_add:
 * @result: destination for the resulting #PicmanVector2.
 * @vector1: a pointer to the first #PicmanVector2.
 * @vector2: a pointer to the second #PicmanVector2.
 *
 * Computes the sum of two 2D vectors. The resulting #PicmanVector2 is
 * stored in @result.
 **/
void
picman_vector2_add (PicmanVector2       *result,
                  const PicmanVector2 *vector1,
                  const PicmanVector2 *vector2)
{
  result->x = vector1->x + vector2->x;
  result->y = vector1->y + vector2->y;
}

/**
 * picman_vector2_add_val:
 * @vector1: the first #PicmanVector2.
 * @vector2: the second #PicmanVector2.
 *
 * This function is identical to picman_vector2_add() but the vectors
 * are passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector2.
 **/
PicmanVector2
picman_vector2_add_val (PicmanVector2 vector1,
                      PicmanVector2 vector2)
{
  PicmanVector2 result;

  result.x = vector1.x + vector2.x;
  result.y = vector1.y + vector2.y;

  return result;
}

/**
 * picman_vector2_sub:
 * @result: the destination for the resulting #PicmanVector2.
 * @vector1: a pointer to the first #PicmanVector2.
 * @vector2: a pointer to the second #PicmanVector2.
 *
 * Computes the difference of two 2D vectors (@vector1 minus @vector2).
 * The resulting #PicmanVector2 is stored in @result.
 **/
void
picman_vector2_sub (PicmanVector2       *result,
                  const PicmanVector2 *vector1,
                  const PicmanVector2 *vector2)
{
  result->x = vector1->x - vector2->x;
  result->y = vector1->y - vector2->y;
}

/**
 * picman_vector2_sub_val:
 * @vector1: the first #PicmanVector2.
 * @vector2: the second #PicmanVector2.
 *
 * This function is identical to picman_vector2_sub() but the vectors
 * are passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector2.
 **/
PicmanVector2
picman_vector2_sub_val (PicmanVector2 vector1,
                      PicmanVector2 vector2)
{
  PicmanVector2 result;

  result.x = vector1.x - vector2.x;
  result.y = vector1.y - vector2.y;

  return result;
}

/**
 * picman_vector2_inner_product:
 * @vector1: a pointer to the first #PicmanVector2.
 * @vector2: a pointer to the second #PicmanVector2.
 *
 * Computes the inner (dot) product of two 2D vectors.
 * This product is zero if and only if the two vectors are orthognal.
 *
 * Returns: The inner product.
 **/
gdouble
picman_vector2_inner_product (const PicmanVector2 *vector1,
                            const PicmanVector2 *vector2)
{
  return (vector1->x * vector2->x + vector1->y * vector2->y);
}

/**
 * picman_vector2_inner_product_val:
 * @vector1: the first #PicmanVector2.
 * @vector2: the second #PicmanVector2.
 *
 * This function is identical to picman_vector2_inner_product() but the
 * vectors are passed by value rather than by reference.
 *
 * Returns: The inner product.
 **/
gdouble
picman_vector2_inner_product_val (PicmanVector2 vector1,
                                PicmanVector2 vector2)
{
  return (vector1.x * vector2.x + vector1.y * vector2.y);
}

/**
 * picman_vector2_cross_product:
 * @vector1: a pointer to the first #PicmanVector2.
 * @vector2: a pointer to the second #PicmanVector2.
 *
 * Compute the cross product of two vectors. The result is a
 * #PicmanVector2 which is orthognal to both @vector1 and @vector2. If
 * @vector1 and @vector2 are parallel, the result will be the nul
 * vector.
 *
 * Note that in 2D, this function is useful to test if two vectors are
 * parallel or not, or to compute the area spawned by two vectors.
 *
 * Returns: The cross product.
 **/
PicmanVector2
picman_vector2_cross_product (const PicmanVector2 *vector1,
                            const PicmanVector2 *vector2)
{
  PicmanVector2 normal;

  normal.x = vector1->x * vector2->y - vector1->y * vector2->x;
  normal.y = vector1->y * vector2->x - vector1->x * vector2->y;

  return normal;
}

/**
 * picman_vector2_cross_product_val:
 * @vector1: the first #PicmanVector2.
 * @vector2: the second #PicmanVector2.
 *
 * This function is identical to picman_vector2_cross_product() but the
 * vectors are passed by value rather than by reference.
 *
 * Returns: The cross product.
 **/
PicmanVector2
picman_vector2_cross_product_val (PicmanVector2 vector1,
                                PicmanVector2 vector2)
{
  PicmanVector2 normal;

  normal.x = vector1.x * vector2.y - vector1.y * vector2.x;
  normal.y = vector1.y * vector2.x - vector1.x * vector2.y;

  return normal;
}

/**
 * picman_vector2_rotate:
 * @vector: a pointer to a #PicmanVector2.
 * @alpha: an angle (in radians).
 *
 * Rotates the @vector counterclockwise by @alpha radians.
 **/
void
picman_vector2_rotate (PicmanVector2 *vector,
                     gdouble      alpha)
{
  PicmanVector2 result;

  result.x = cos (alpha) * vector->x + sin (alpha) * vector->y;
  result.y = cos (alpha) * vector->y - sin (alpha) * vector->x;

  *vector = result;
}

/**
 * picman_vector2_rotate_val:
 * @vector: a #PicmanVector2.
 * @alpha: an angle (in radians).
 *
 * This function is identical to picman_vector2_rotate() but the vector
 * is passed by value rather than by reference.
 *
 * Returns: a #PicmanVector2 representing @vector rotated by @alpha
 * radians.
 **/
PicmanVector2
picman_vector2_rotate_val (PicmanVector2 vector,
                         gdouble     alpha)
{
  PicmanVector2 result;

  result.x = cos (alpha) * vector.x + sin (alpha) * vector.y;
  result.y = cos (alpha) * vector.y - sin (alpha) * vector.x;

  return result;
}

/**
 * picman_vector2_normal:
 * @vector: a pointer to a #PicmanVector2.
 *
 * Compute a normalized perpendicular vector to @vector
 *
 * Returns: a #PicmanVector2 perpendicular to @vector, with a length of 1.0.
 * 
 * Since: 2.8
 **/
PicmanVector2
picman_vector2_normal (PicmanVector2  *vector)
{
  PicmanVector2 result;

  result.x = - vector->y;
  result.y = vector->x;

  picman_vector2_normalize (&result);

  return result;
}

/**
 * picman_vector2_normal_val:
 * @vector: a #PicmanVector2.
 *
 * This function is identical to picman_vector2_normal() but the vector
 * is passed by value rather than by reference.
 *
 * Returns: a #PicmanVector2 perpendicular to @vector, with a length of 1.0.
 *
 * Since: 2.8
 **/
PicmanVector2
picman_vector2_normal_val (PicmanVector2  vector)
{
  PicmanVector2 result;

  result.x = - vector.y;
  result.y = vector.x;

  picman_vector2_normalize (&result);

  return result;
}
/**************************************/
/* Three dimensional vector functions */
/**************************************/

/**
 * picman_vector3_new:
 * @x: the X coordinate.
 * @y: the Y coordinate.
 * @z: the Z coordinate.
 *
 * Creates a #PicmanVector3 of coordinate @x, @y and @z.
 *
 * Returns: the resulting #PicmanVector3.
 **/
PicmanVector3
picman_vector3_new (gdouble  x,
                  gdouble  y,
                  gdouble  z)
{
  PicmanVector3 vector;

  vector.x = x;
  vector.y = y;
  vector.z = z;

  return vector;
}

/**
 * picman_vector3_set:
 * @vector: a pointer to a #PicmanVector3.
 * @x: the X coordinate.
 * @y: the Y coordinate.
 * @z: the Z coordinate.
 *
 * Sets the X, Y and Z coordinates of @vector to @x, @y and @z.
 **/
void
picman_vector3_set (PicmanVector3 *vector,
                  gdouble      x,
                  gdouble      y,
                  gdouble      z)
{
  vector->x = x;
  vector->y = y;
  vector->z = z;
}

/**
 * picman_vector3_length:
 * @vector: a pointer to a #PicmanVector3.
 *
 * Computes the length of a 3D vector.
 *
 * Returns: the length of @vector (a positive gdouble).
 **/
gdouble
picman_vector3_length (const PicmanVector3 *vector)
{
  return (sqrt (vector->x * vector->x +
                vector->y * vector->y +
                vector->z * vector->z));
}

/**
 * picman_vector3_length_val:
 * @vector: a #PicmanVector3.
 *
 * This function is identical to picman_vector3_length() but the vector
 * is passed by value rather than by reference.
 *
 * Returns: the length of @vector (a positive gdouble).
 **/
gdouble
picman_vector3_length_val (PicmanVector3 vector)
{
  return (sqrt (vector.x * vector.x +
                vector.y * vector.y +
                vector.z * vector.z));
}

/**
 * picman_vector3_mul:
 * @vector: a pointer to a #PicmanVector3.
 * @factor: a scalar.
 *
 * Multiplies each component of the @vector by @factor. Note that
 * this is equivalent to multiplying the vectors length by @factor.
 **/
void
picman_vector3_mul (PicmanVector3 *vector,
                  gdouble      factor)
{
  vector->x *= factor;
  vector->y *= factor;
  vector->z *= factor;
}

/**
 * picman_vector3_mul_val:
 * @vector: a #PicmanVector3.
 * @factor: a scalar.
 *
 * This function is identical to picman_vector3_mul() but the vector is
 * passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector3.
 **/
PicmanVector3
picman_vector3_mul_val (PicmanVector3 vector,
                      gdouble     factor)
{
  PicmanVector3 result;

  result.x = vector.x * factor;
  result.y = vector.y * factor;
  result.z = vector.z * factor;

  return result;
}

/**
 * picman_vector3_normalize:
 * @vector: a pointer to a #PicmanVector3.
 *
 * Normalizes the @vector so the length of the @vector is 1.0. The nul
 * vector will not be changed.
 **/
void
picman_vector3_normalize (PicmanVector3 *vector)
{
  gdouble len;

  len = picman_vector3_length (vector);

  if (len != 0.0)
    {
      len = 1.0 / len;
      vector->x *= len;
      vector->y *= len;
      vector->z *= len;
    }
  else
    {
      *vector = picman_vector3_zero;
    }
}

/**
 * picman_vector3_normalize_val:
 * @vector: a #PicmanVector3.
 *
 * This function is identical to picman_vector3_normalize() but the
 * vector is passed by value rather than by reference.
 *
 * Returns: a #PicmanVector3 parallel to @vector, pointing in the same
 * direction but with a length of 1.0.
 **/
PicmanVector3
picman_vector3_normalize_val (PicmanVector3 vector)
{
  PicmanVector3 result;
  gdouble     len;

  len = picman_vector3_length_val (vector);

  if (len != 0.0)
    {
      len = 1.0 / len;
      result.x = vector.x * len;
      result.y = vector.y * len;
      result.z = vector.z * len;
      return result;
    }
  else
    {
      return picman_vector3_zero;
    }
}

/**
 * picman_vector3_neg:
 * @vector: a pointer to a #PicmanVector3.
 *
 * Negates the @vector (i.e. negate all its coordinates).
 **/
void
picman_vector3_neg (PicmanVector3 *vector)
{
  vector->x *= -1.0;
  vector->y *= -1.0;
  vector->z *= -1.0;
}

/**
 * picman_vector3_neg_val:
 * @vector: a #PicmanVector3.
 *
 * This function is identical to picman_vector3_neg() but the vector
 * is passed by value rather than by reference.
 *
 * Returns: the negated #PicmanVector3.
 **/
PicmanVector3
picman_vector3_neg_val (PicmanVector3 vector)
{
  PicmanVector3 result;

  result.x = vector.x * -1.0;
  result.y = vector.y * -1.0;
  result.z = vector.z * -1.0;

  return result;
}

/**
 * picman_vector3_add:
 * @result: destination for the resulting #PicmanVector3.
 * @vector1: a pointer to the first #PicmanVector3.
 * @vector2: a pointer to the second #PicmanVector3.
 *
 * Computes the sum of two 3D vectors. The resulting #PicmanVector3 is
 * stored in @result.
 **/
void
picman_vector3_add (PicmanVector3       *result,
                  const PicmanVector3 *vector1,
                  const PicmanVector3 *vector2)
{
  result->x = vector1->x + vector2->x;
  result->y = vector1->y + vector2->y;
  result->z = vector1->z + vector2->z;
}

/**
 * picman_vector3_add_val:
 * @vector1: a #PicmanVector3.
 * @vector2: a #PicmanVector3.
 *
 * This function is identical to picman_vector3_add() but the vectors
 * are passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector3.
 **/
PicmanVector3
picman_vector3_add_val (PicmanVector3 vector1,
                      PicmanVector3 vector2)
{
  PicmanVector3 result;

  result.x = vector1.x + vector2.x;
  result.y = vector1.y + vector2.y;
  result.z = vector1.z + vector2.z;

  return result;
}

/**
 * picman_vector3_sub:
 * @result: the destination for the resulting #PicmanVector3.
 * @vector1: a pointer to the first #PicmanVector3.
 * @vector2: a pointer to the second #PicmanVector3.
 *
 * Computes the difference of two 3D vectors (@vector1 minus @vector2).
 * The resulting #PicmanVector3 is stored in @result.
 **/
void
picman_vector3_sub (PicmanVector3       *result,
                  const PicmanVector3 *vector1,
                  const PicmanVector3 *vector2)
{
  result->x = vector1->x - vector2->x;
  result->y = vector1->y - vector2->y;
  result->z = vector1->z - vector2->z;
}

/**
 * picman_vector3_sub_val:
 * @vector1: a #PicmanVector3.
 * @vector2: a #PicmanVector3.
 *
 * This function is identical to picman_vector3_sub() but the vectors
 * are passed by value rather than by reference.
 *
 * Returns: the resulting #PicmanVector3.
 **/
PicmanVector3
picman_vector3_sub_val (PicmanVector3 vector1,
                     PicmanVector3 vector2)
{
  PicmanVector3 result;

  result.x = vector1.x - vector2.x;
  result.y = vector1.y - vector2.y;
  result.z = vector1.z - vector2.z;

  return result;
}

/**
 * picman_vector3_inner_product:
 * @vector1: a pointer to the first #PicmanVector3.
 * @vector2: a pointer to the second #PicmanVector3.
 *
 * Computes the inner (dot) product of two 3D vectors. This product
 * is zero if and only if the two vectors are orthognal.
 *
 * Returns: The inner product.
 **/
gdouble
picman_vector3_inner_product (const PicmanVector3 *vector1,
                            const PicmanVector3 *vector2)
{
  return (vector1->x * vector2->x +
          vector1->y * vector2->y +
          vector1->z * vector2->z);
}

/**
 * picman_vector3_inner_product_val:
 * @vector1: the first #PicmanVector3.
 * @vector2: the second #PicmanVector3.
 *
 * This function is identical to picman_vector3_inner_product() but the
 * vectors are passed by value rather than by reference.
 *
 * Returns: The inner product.
 **/
gdouble
picman_vector3_inner_product_val (PicmanVector3 vector1,
                                PicmanVector3 vector2)
{
  return (vector1.x * vector2.x +
          vector1.y * vector2.y +
          vector1.z * vector2.z);
}

/**
 * picman_vector3_cross_product:
 * @vector1: a pointer to the first #PicmanVector3.
 * @vector2: a pointer to the second #PicmanVector3.
 *
 * Compute the cross product of two vectors. The result is a
 * #PicmanVector3 which is orthognal to both @vector1 and @vector2. If
 * @vector1 and @vector2 and parallel, the result will be the nul
 * vector.
 *
 * This function can be used to compute the normal of the plane
 * defined by @vector1 and @vector2.
 *
 * Returns: The cross product.
 **/
PicmanVector3
picman_vector3_cross_product (const PicmanVector3 *vector1,
                            const PicmanVector3 *vector2)
{
  PicmanVector3 normal;

  normal.x = vector1->y * vector2->z - vector1->z * vector2->y;
  normal.y = vector1->z * vector2->x - vector1->x * vector2->z;
  normal.z = vector1->x * vector2->y - vector1->y * vector2->x;

  return normal;
}

/**
 * picman_vector3_cross_product_val:
 * @vector1: the first #PicmanVector3.
 * @vector2: the second #PicmanVector3.
 *
 * This function is identical to picman_vector3_cross_product() but the
 * vectors are passed by value rather than by reference.
 *
 * Returns: The cross product.
 **/
PicmanVector3
picman_vector3_cross_product_val (PicmanVector3 vector1,
                                PicmanVector3 vector2)
{
  PicmanVector3 normal;

  normal.x = vector1.y * vector2.z - vector1.z * vector2.y;
  normal.y = vector1.z * vector2.x - vector1.x * vector2.z;
  normal.z = vector1.x * vector2.y - vector1.y * vector2.x;

  return normal;
}

/**
 * picman_vector3_rotate:
 * @vector: a pointer to a #PicmanVector3.
 * @alpha: the angle (in radian) of rotation around the Z axis.
 * @beta: the angle (in radian) of rotation around the Y axis.
 * @gamma: the angle (in radian) of rotation around the X axis.
 *
 * Rotates the @vector around the three axis (Z, Y, and X) by @alpha,
 * @beta and @gamma, respectively.
 *
 * Note that the order of the rotation is very important. If you
 * expect a vector to be rotated around X, and then around Y, you will
 * have to call this function twice. Also, it is often wise to call
 * this function with only one of @alpha, @beta and @gamma non-zero.
 **/
void
picman_vector3_rotate (PicmanVector3 *vector,
                     gdouble      alpha,
                     gdouble      beta,
                     gdouble      gamma)
{
  PicmanVector3 s, t;

  /* First we rotate it around the Z axis (XY plane).. */
  /* ================================================= */

  s.x = cos (alpha) * vector->x + sin (alpha) * vector->y;
  s.y = cos (alpha) * vector->y - sin (alpha) * vector->x;

  /* ..then around the Y axis (XZ plane).. */
  /* ===================================== */

  t = s;

  vector->x = cos (beta) *t.x       + sin (beta) * vector->z;
  s.z       = cos (beta) *vector->z - sin (beta) * t.x;

  /* ..and at last around the X axis (YZ plane) */
  /* ========================================== */

  vector->y = cos (gamma) * t.y + sin (gamma) * s.z;
  vector->z = cos (gamma) * s.z - sin (gamma) * t.y;
}

/**
 * picman_vector3_rotate_val:
 * @vector: a #PicmanVector3.
 * @alpha: the angle (in radian) of rotation around the Z axis.
 * @beta: the angle (in radian) of rotation around the Y axis.
 * @gamma: the angle (in radian) of rotation around the X axis.
 *
 * This function is identical to picman_vector3_rotate() but the vectors
 * are passed by value rather than by reference.
 *
 * Returns: the rotated vector.
 **/
PicmanVector3
picman_vector3_rotate_val (PicmanVector3 vector,
                         gdouble     alpha,
                         gdouble     beta,
                         gdouble     gamma)
{
  PicmanVector3 s, t, result;

  /* First we rotate it around the Z axis (XY plane).. */
  /* ================================================= */

  s.x = cos (alpha) * vector.x + sin (alpha) * vector.y;
  s.y = cos (alpha) * vector.y - sin (alpha) * vector.x;

  /* ..then around the Y axis (XZ plane).. */
  /* ===================================== */

  t = s;

  result.x = cos (beta) *t.x      + sin (beta) * vector.z;
  s.z      = cos (beta) *vector.z - sin (beta) * t.x;

  /* ..and at last around the X axis (YZ plane) */
  /* ========================================== */

  result.y = cos (gamma) * t.y + sin (gamma) * s.z;
  result.z = cos (gamma) * s.z - sin (gamma) * t.y;

  return result;
}

/**
 * picman_vector_2d_to_3d:
 * @sx: the abscisse of the upper-left screen rectangle.
 * @sy: the ordinate of the upper-left screen rectangle.
 * @w: the width of the screen rectangle.
 * @h: the height of the screen rectangle.
 * @x: the abscisse of the point in the screen rectangle to map.
 * @y: the ordinate of the point in the screen rectangle to map.
 * @vp: the position of the observer.
 * @p: the resulting point.
 *
 * \"Compute screen (sx, sy) - (sx + w, sy + h) to 3D unit square
 * mapping. The plane to map to is given in the z field of p. The
 * observer is located at position vp (vp->z != 0.0).\"
 *
 * In other words, this computes the projection of the point (@x, @y)
 * to the plane z = @p->z (parallel to XY), from the @vp point of view
 * through the screen (@sx, @sy)->(@sx + @w, @sy + @h)
 **/

void
picman_vector_2d_to_3d (gint               sx,
                      gint               sy,
                      gint               w,
                      gint               h,
                      gint               x,
                      gint               y,
                      const PicmanVector3 *vp,
                      PicmanVector3       *p)
{
  gdouble t = 0.0;

  if (vp->x != 0.0)
    t = (p->z - vp->z) / vp->z;

  if (t != 0.0)
    {
      p->x = vp->x + t * (vp->x - ((gdouble) (x - sx) / (gdouble) w));
      p->y = vp->y + t * (vp->y - ((gdouble) (y - sy) / (gdouble) h));
    }
  else
    {
      p->x = (gdouble) (x - sx) / (gdouble) w;
      p->y = (gdouble) (y - sy) / (gdouble) h;
    }
}

/**
 * picman_vector_2d_to_3d_val:
 * @sx: the abscisse of the upper-left screen rectangle.
 * @sy: the ordinate of the upper-left screen rectangle.
 * @w: the width of the screen rectangle.
 * @h: the height of the screen rectangle.
 * @x: the abscisse of the point in the screen rectangle to map.
 * @y: the ordinate of the point in the screen rectangle to map.
 * @vp: position of the observer.
 * @p: the resulting point.
 *
 * This function is identical to picman_vector_2d_to_3d() but the
 * position of the @observer and the resulting point @p are passed by
 * value rather than by reference.
 *
 * Returns: the computed #PicmanVector3 point.
 **/
PicmanVector3
picman_vector_2d_to_3d_val (gint        sx,
                          gint        sy,
                          gint        w,
                          gint        h,
                          gint        x,
                          gint        y,
                          PicmanVector3 vp,
                          PicmanVector3 p)
{
  PicmanVector3 result;
  gdouble     t = 0.0;

  if (vp.x != 0.0)
    t = (p.z - vp.z) / vp.z;

  if (t != 0.0)
    {
      result.x = vp.x + t * (vp.x - ((gdouble) (x - sx) / (gdouble) w));
      result.y = vp.y + t * (vp.y - ((gdouble) (y - sy) / (gdouble) h));
    }
  else
    {
      result.x = (gdouble) (x - sx) / (gdouble) w;
      result.y = (gdouble) (y - sy) / (gdouble) h;
    }

  result.z = 0;
  return result;
}

/**
 * picman_vector_3d_to_2d:
 * @sx: the abscisse of the upper-left screen rectangle.
 * @sy: the ordinate of the upper-left screen rectangle.
 * @w: the width of the screen rectangle.
 * @h: the height of the screen rectangle.
 * @x: the abscisse of the point in the screen rectangle to map (return value).
 * @y: the ordinate of the point in the screen rectangle to map (return value).
 * @vp: position of the observer.
 * @p: the 3D point to project to the plane.
 *
 * Convert the given 3D point to 2D (project it onto the viewing
 * plane, (sx, sy, 0) - (sx + w, sy + h, 0). The input is assumed to
 * be in the unit square (0, 0, z) - (1, 1, z). The viewpoint of the
 * observer is passed in vp.
 *
 * This is basically the opposite of picman_vector_2d_to_3d().
 **/
void
picman_vector_3d_to_2d (gint               sx,
                      gint               sy,
                      gint               w,
                      gint               h,
                      gdouble           *x,
                      gdouble           *y,
                      const PicmanVector3 *vp,
                      const PicmanVector3 *p)
{
  gdouble     t;
  PicmanVector3 dir;

  picman_vector3_sub (&dir, p, vp);
  picman_vector3_normalize (&dir);

  if (dir.z != 0.0)
    {
      t = (-1.0 * vp->z) / dir.z;
      *x = (gdouble) sx + ((vp->x + t * dir.x) * (gdouble) w);
      *y = (gdouble) sy + ((vp->y + t * dir.y) * (gdouble) h);
    }
  else
    {
      *x = (gdouble) sx + (p->x * (gdouble) w);
      *y = (gdouble) sy + (p->y * (gdouble) h);
    }
}
