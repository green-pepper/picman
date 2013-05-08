/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanvector.h
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

#if !defined (__PICMAN_MATH_H_INSIDE__) && !defined (PICMAN_MATH_COMPILATION)
#error "Only <libpicmanmath/picmanmath.h> can be included directly."
#endif

#ifndef __PICMAN_VECTOR_H__
#define __PICMAN_VECTOR_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/* Two dimensional vector functions */
/* ================================ */

PicmanVector2 picman_vector2_new               (gdouble            x,
                                            gdouble            y);
void        picman_vector2_set               (PicmanVector2       *vector,
                                            gdouble            x,
                                            gdouble            y);
gdouble     picman_vector2_length            (const PicmanVector2 *vector);
gdouble     picman_vector2_length_val        (PicmanVector2        vector);
void        picman_vector2_mul               (PicmanVector2       *vector,
                                            gdouble            factor);
PicmanVector2 picman_vector2_mul_val           (PicmanVector2        vector,
                                            gdouble            factor);
void        picman_vector2_normalize         (PicmanVector2       *vector);
PicmanVector2 picman_vector2_normalize_val     (PicmanVector2        vector);
void        picman_vector2_neg               (PicmanVector2       *vector);
PicmanVector2 picman_vector2_neg_val           (PicmanVector2        vector);
void        picman_vector2_add               (PicmanVector2       *result,
                                            const PicmanVector2 *vector1,
                                            const PicmanVector2 *vector2);
PicmanVector2 picman_vector2_add_val           (PicmanVector2        vector1,
                                            PicmanVector2        vector2);
void        picman_vector2_sub               (PicmanVector2       *result,
                                            const PicmanVector2 *vector1,
                                            const PicmanVector2 *vector2);
PicmanVector2 picman_vector2_sub_val           (PicmanVector2        vector1,
                                            PicmanVector2        vector2);
gdouble     picman_vector2_inner_product     (const PicmanVector2 *vector1,
                                            const PicmanVector2 *vector2);
gdouble     picman_vector2_inner_product_val (PicmanVector2        vector1,
                                            PicmanVector2        vector2);
PicmanVector2 picman_vector2_cross_product     (const PicmanVector2 *vector1,
                                            const PicmanVector2 *vector2);
PicmanVector2 picman_vector2_cross_product_val (PicmanVector2        vector1,
                                            PicmanVector2        vector2);
void        picman_vector2_rotate            (PicmanVector2       *vector,
                                            gdouble            alpha);
PicmanVector2 picman_vector2_rotate_val        (PicmanVector2        vector,
                                            gdouble            alpha);
PicmanVector2 picman_vector2_normal            (PicmanVector2       *vector);
PicmanVector2 picman_vector2_normal_val        (PicmanVector2        vector);

/* Three dimensional vector functions */
/* ================================== */

PicmanVector3 picman_vector3_new               (gdouble            x,
                                            gdouble            y,
                                            gdouble            z);
void        picman_vector3_set               (PicmanVector3       *vector,
                                            gdouble            x,
                                            gdouble            y,
                                            gdouble            z);
gdouble     picman_vector3_length            (const PicmanVector3 *vector);
gdouble     picman_vector3_length_val        (PicmanVector3        vector);
void        picman_vector3_mul               (PicmanVector3       *vector,
                                            gdouble            factor);
PicmanVector3 picman_vector3_mul_val           (PicmanVector3        vector,
                                            gdouble            factor);
void        picman_vector3_normalize         (PicmanVector3       *vector);
PicmanVector3 picman_vector3_normalize_val     (PicmanVector3        vector);
void        picman_vector3_neg               (PicmanVector3       *vector);
PicmanVector3 picman_vector3_neg_val           (PicmanVector3        vector);
void        picman_vector3_add               (PicmanVector3       *result,
                                            const PicmanVector3 *vector1,
                                            const PicmanVector3 *vector2);
PicmanVector3 picman_vector3_add_val           (PicmanVector3        vector1,
                                            PicmanVector3        vector2);
void        picman_vector3_sub               (PicmanVector3       *result,
                                            const PicmanVector3 *vector1,
                                            const PicmanVector3 *vector2);
PicmanVector3 picman_vector3_sub_val           (PicmanVector3        vector1,
                                            PicmanVector3        vector2);
gdouble     picman_vector3_inner_product     (const PicmanVector3 *vector1,
                                            const PicmanVector3 *vector2);
gdouble     picman_vector3_inner_product_val (PicmanVector3        vector1,
                                            PicmanVector3        vector2);
PicmanVector3 picman_vector3_cross_product     (const PicmanVector3 *vector1,
                                            const PicmanVector3 *vector2);
PicmanVector3 picman_vector3_cross_product_val (PicmanVector3        vector1,
                                            PicmanVector3        vector2);
void        picman_vector3_rotate            (PicmanVector3       *vector,
                                            gdouble            alpha,
                                            gdouble            beta,
                                            gdouble            gamma);
PicmanVector3 picman_vector3_rotate_val        (PicmanVector3        vector,
                                            gdouble            alpha,
                                            gdouble            beta,
                                            gdouble            gamma);

/* 2d <-> 3d Vector projection functions */
/* ===================================== */

void        picman_vector_2d_to_3d           (gint               sx,
                                            gint               sy,
                                            gint               w,
                                            gint               h,
                                            gint               x,
                                            gint               y,
                                            const PicmanVector3 *vp,
                                            PicmanVector3       *p);

PicmanVector3 picman_vector_2d_to_3d_val       (gint               sx,
                                            gint               sy,
                                            gint               w,
                                            gint               h,
                                            gint               x,
                                            gint               y,
                                            PicmanVector3        vp,
                                            PicmanVector3        p);

void        picman_vector_3d_to_2d           (gint               sx,
                                            gint               sy,
                                            gint               w,
                                            gint               h,
                                            gdouble           *x,
                                            gdouble           *y,
                                            const PicmanVector3 *vp,
                                            const PicmanVector3 *p);


G_END_DECLS

#endif  /* __PICMAN_VECTOR_H__ */
