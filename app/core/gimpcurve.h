/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_CURVE_H__
#define __PICMAN_CURVE_H__


#include "picmandata.h"


#define PICMAN_TYPE_CURVE            (picman_curve_get_type ())
#define PICMAN_CURVE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CURVE, PicmanCurve))
#define PICMAN_CURVE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CURVE, PicmanCurveClass))
#define PICMAN_IS_CURVE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CURVE))
#define PICMAN_IS_CURVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CURVE))
#define PICMAN_CURVE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CURVE, PicmanCurveClass))


typedef struct _PicmanCurveClass PicmanCurveClass;

struct _PicmanCurve
{
  PicmanData       parent_instance;

  PicmanCurveType  curve_type;

  gint           n_points;
  PicmanVector2   *points;

  gint           n_samples;
  gdouble       *samples;

  gboolean       identity;  /* whether the curve is an identiy mapping */
};

struct _PicmanCurveClass
{
  PicmanDataClass  parent_class;
};


GType           picman_curve_get_type          (void) G_GNUC_CONST;

PicmanData      * picman_curve_new               (const gchar   *name);
PicmanData      * picman_curve_get_standard      (void);

void            picman_curve_reset             (PicmanCurve     *curve,
                                              gboolean       reset_type);

void            picman_curve_set_curve_type    (PicmanCurve     *curve,
                                              PicmanCurveType  curve_type);
PicmanCurveType   picman_curve_get_curve_type    (PicmanCurve     *curve);

gint            picman_curve_get_n_points      (PicmanCurve     *curve);
gint            picman_curve_get_n_samples     (PicmanCurve     *curve);

gint            picman_curve_get_closest_point (PicmanCurve     *curve,
                                              gdouble        x);

void            picman_curve_set_point         (PicmanCurve     *curve,
                                              gint           point,
                                              gdouble        x,
                                              gdouble        y);
void            picman_curve_move_point        (PicmanCurve     *curve,
                                              gint           point,
                                              gdouble        y);
void            picman_curve_delete_point      (PicmanCurve     *curve,
                                              gint           point);
void            picman_curve_get_point         (PicmanCurve     *curve,
                                              gint           point,
                                              gdouble       *x,
                                              gdouble       *y);

void            picman_curve_set_curve         (PicmanCurve     *curve,
                                              gdouble        x,
                                              gdouble        y);

gboolean        picman_curve_is_identity       (PicmanCurve     *curve);

void            picman_curve_get_uchar         (PicmanCurve     *curve,
                                              gint           n_samples,
                                              guchar        *samples);


#endif /* __PICMAN_CURVE_H__ */
