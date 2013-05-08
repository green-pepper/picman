/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanmatrix.h
 * Copyright (C) 1998 Jay Cox <jaycox@picman.org>
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

#ifndef __PICMAN_MATRIX_H__
#define __PICMAN_MATRIX_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


/*****************/
/*  PicmanMatrix2  */
/*****************/

#define PICMAN_TYPE_MATRIX2               (picman_matrix2_get_type ())
#define PICMAN_VALUE_HOLDS_MATRIX2(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_MATRIX2))

GType         picman_matrix2_get_type        (void) G_GNUC_CONST;


#define PICMAN_TYPE_PARAM_MATRIX2            (picman_param_matrix2_get_type ())
#define PICMAN_IS_PARAM_SPEC_MATRIX2(pspec)  (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_MATRIX2))

GType         picman_param_matrix2_get_type  (void) G_GNUC_CONST;

GParamSpec *  picman_param_spec_matrix2      (const gchar        *name,
                                            const gchar        *nick,
                                            const gchar        *blurb,
                                            const PicmanMatrix2  *default_value,
                                            GParamFlags         flags);


void          picman_matrix2_identity        (PicmanMatrix2       *matrix);
void          picman_matrix2_mult            (const PicmanMatrix2 *matrix1,
                                            PicmanMatrix2       *matrix2);


/*****************/
/*  PicmanMatrix3  */
/*****************/

#define PICMAN_TYPE_MATRIX3               (picman_matrix3_get_type ())
#define PICMAN_VALUE_HOLDS_MATRIX3(value) (G_TYPE_CHECK_VALUE_TYPE ((value), PICMAN_TYPE_MATRIX3))

GType         picman_matrix3_get_type        (void) G_GNUC_CONST;


#define PICMAN_TYPE_PARAM_MATRIX3            (picman_param_matrix3_get_type ())
#define PICMAN_IS_PARAM_SPEC_MATRIX3(pspec)  (G_TYPE_CHECK_INSTANCE_TYPE ((pspec), PICMAN_TYPE_PARAM_MATRIX3))

GType         picman_param_matrix3_get_type  (void) G_GNUC_CONST;

GParamSpec *  picman_param_spec_matrix3      (const gchar        *name,
                                            const gchar        *nick,
                                            const gchar        *blurb,
                                            const PicmanMatrix3  *default_value,
                                            GParamFlags         flags);


void          picman_matrix3_identity        (PicmanMatrix3       *matrix);
void          picman_matrix3_mult            (const PicmanMatrix3 *matrix1,
                                            PicmanMatrix3       *matrix2);
void          picman_matrix3_translate       (PicmanMatrix3       *matrix,
                                            gdouble            x,
                                            gdouble            y);
void          picman_matrix3_scale           (PicmanMatrix3       *matrix,
                                            gdouble            x,
                                            gdouble            y);
void          picman_matrix3_rotate          (PicmanMatrix3       *matrix,
                                            gdouble            theta);
void          picman_matrix3_xshear          (PicmanMatrix3       *matrix,
                                            gdouble            amount);
void          picman_matrix3_yshear          (PicmanMatrix3       *matrix,
                                            gdouble            amount);
void          picman_matrix3_affine          (PicmanMatrix3       *matrix,
                                            gdouble            a,
                                            gdouble            b,
                                            gdouble            c,
                                            gdouble            d,
                                            gdouble            e,
                                            gdouble            f);

gdouble       picman_matrix3_determinant     (const PicmanMatrix3 *matrix);
void          picman_matrix3_invert          (PicmanMatrix3       *matrix);

gboolean      picman_matrix3_is_identity     (const PicmanMatrix3 *matrix);
gboolean      picman_matrix3_is_diagonal     (const PicmanMatrix3 *matrix);
gboolean      picman_matrix3_is_affine       (const PicmanMatrix3 *matrix);
gboolean      picman_matrix3_is_simple       (const PicmanMatrix3 *matrix);

void          picman_matrix3_transform_point (const PicmanMatrix3 *matrix,
                                            gdouble            x,
                                            gdouble            y,
                                            gdouble           *newx,
                                            gdouble           *newy);


/*****************/
/*  PicmanMatrix4  */

/*****************/
void          picman_matrix4_to_deg          (const PicmanMatrix4 *matrix,
                                            gdouble           *a,
                                            gdouble           *b,
                                            gdouble           *c);


G_END_DECLS

#endif /* __PICMAN_MATRIX_H__ */
