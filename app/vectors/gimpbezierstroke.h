/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbezierstroke.h
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#ifndef __PICMAN_BEZIER_STROKE_H__
#define __PICMAN_BEZIER_STROKE_H__

#include "picmanstroke.h"


#define PICMAN_TYPE_BEZIER_STROKE            (picman_bezier_stroke_get_type ())
#define PICMAN_BEZIER_STROKE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BEZIER_STROKE, PicmanBezierStroke))
#define PICMAN_BEZIER_STROKE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BEZIER_STROKE, PicmanBezierStrokeClass))
#define PICMAN_IS_BEZIER_STROKE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BEZIER_STROKE))
#define PICMAN_IS_BEZIER_STROKE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BEZIER_STROKE))
#define PICMAN_BEZIER_STROKE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BEZIER_STROKE, PicmanBezierStrokeClass))


typedef struct _PicmanBezierStrokeClass PicmanBezierStrokeClass;

struct _PicmanBezierStroke
{
  PicmanStroke  parent_instance;
};

struct _PicmanBezierStrokeClass
{
  PicmanStrokeClass  parent_class;
};


GType        picman_bezier_stroke_get_type        (void) G_GNUC_CONST;

PicmanStroke * picman_bezier_stroke_new             (void);
PicmanStroke * picman_bezier_stroke_new_from_coords (const PicmanCoords *coords,
                                                 gint              n_coords,
                                                 gboolean          closed);

PicmanStroke * picman_bezier_stroke_new_moveto      (const PicmanCoords *start);
void         picman_bezier_stroke_lineto          (PicmanStroke       *bez_stroke,
                                                 const PicmanCoords *end);
void         picman_bezier_stroke_conicto         (PicmanStroke       *bez_stroke,
                                                 const PicmanCoords *control,
                                                 const PicmanCoords *end);
void         picman_bezier_stroke_cubicto         (PicmanStroke       *bez_stroke,
                                                 const PicmanCoords *control1,
                                                 const PicmanCoords *control2,
                                                 const PicmanCoords *end);
void         picman_bezier_stroke_arcto           (PicmanStroke       *bez_stroke,
                                                 gdouble           radius_x,
                                                 gdouble           radius_y,
                                                 gdouble           angle_rad,
                                                 gboolean          large_arc,
                                                 gboolean          sweep,
                                                 const PicmanCoords *end);
PicmanStroke * picman_bezier_stroke_new_ellipse     (const PicmanCoords *center,
                                                 gdouble           radius_x,
                                                 gdouble           radius_y,
                                                 gdouble           angle);


PicmanAnchor * picman_bezier_stroke_extend      (PicmanStroke           *stroke,
                                             const PicmanCoords     *coords,
                                             PicmanAnchor           *neighbor,
                                             PicmanVectorExtendMode  extend_mode);


#endif /* __PICMAN_BEZIER_STROKE_H__ */
