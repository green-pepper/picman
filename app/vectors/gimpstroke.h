/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanstroke.h
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

#ifndef __PICMAN_STROKE_H__
#define __PICMAN_STROKE_H__

#include "core/picmanobject.h"


#define PICMAN_TYPE_STROKE            (picman_stroke_get_type ())
#define PICMAN_STROKE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STROKE, PicmanStroke))
#define PICMAN_STROKE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STROKE, PicmanStrokeClass))
#define PICMAN_IS_STROKE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STROKE))
#define PICMAN_IS_STROKE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_STROKE))
#define PICMAN_STROKE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_STROKE, PicmanStrokeClass))


typedef struct _PicmanStrokeClass PicmanStrokeClass;

struct _PicmanStroke
{
  PicmanObject  parent_instance;
  gint        ID;

  GList      *anchors;

  gboolean    closed;
};

struct _PicmanStrokeClass
{
  PicmanObjectClass  parent_class;

  void          (* changed)              (PicmanStroke            *stroke);
  void          (* removed)              (PicmanStroke            *stroke);

  PicmanAnchor  * (* anchor_get)           (const PicmanStroke      *stroke,
                                          const PicmanCoords      *coord);
  gdouble       (* nearest_point_get)    (const PicmanStroke      *stroke,
                                          const PicmanCoords      *coord,
                                          const gdouble          precision,
                                          PicmanCoords            *ret_point,
                                          PicmanAnchor           **ret_segment_start,
                                          PicmanAnchor           **ret_segment_end,
                                          gdouble               *ret_pos);
  gdouble       (* nearest_tangent_get)  (const PicmanStroke      *stroke,
                                          const PicmanCoords      *coord1,
                                          const PicmanCoords      *coord2,
                                          const gdouble          precision,
                                          PicmanCoords            *nearest,
                                          PicmanAnchor           **ret_segment_start,
                                          PicmanAnchor           **ret_segment_end,
                                          gdouble               *ret_pos);
  gdouble       (* nearest_intersection_get)
                                         (const PicmanStroke      *stroke,
                                          const PicmanCoords      *coord1,
                                          const PicmanCoords      *direction,
                                          const gdouble          precision,
                                          PicmanCoords            *nearest,
                                          PicmanAnchor           **ret_segment_start,
                                          PicmanAnchor           **ret_segment_end,
                                          gdouble               *ret_pos);
  PicmanAnchor  * (* anchor_get_next)      (const PicmanStroke      *stroke,
                                          const PicmanAnchor      *prev);
  void          (* anchor_select)        (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor,
                                          gboolean               selected,
                                          gboolean               exclusive);
  void          (* anchor_move_relative) (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor,
                                          const PicmanCoords      *deltacoord,
                                          PicmanAnchorFeatureType  feature);
  void          (* anchor_move_absolute) (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor,
                                          const PicmanCoords      *coord,
                                          PicmanAnchorFeatureType  feature);
  void          (* anchor_convert)       (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor,
                                          PicmanAnchorFeatureType  feature);
  void          (* anchor_delete)        (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor);

  gboolean      (* point_is_movable)     (PicmanStroke            *stroke,
                                          PicmanAnchor            *predec,
                                          gdouble                position);
  void          (* point_move_relative)  (PicmanStroke            *stroke,
                                          PicmanAnchor            *predec,
                                          gdouble                position,
                                          const PicmanCoords      *deltacoord,
                                          PicmanAnchorFeatureType  feature);
  void          (* point_move_absolute)  (PicmanStroke            *stroke,
                                          PicmanAnchor            *predec,
                                          gdouble                position,
                                          const PicmanCoords      *coord,
                                          PicmanAnchorFeatureType  feature);

  void          (* close)                (PicmanStroke            *stroke);
  PicmanStroke  * (* open)                 (PicmanStroke            *stroke,
                                          PicmanAnchor            *end_anchor);
  gboolean      (* anchor_is_insertable) (PicmanStroke            *stroke,
                                          PicmanAnchor            *predec,
                                          gdouble                position);
  PicmanAnchor  * (* anchor_insert)        (PicmanStroke            *stroke,
                                          PicmanAnchor            *predec,
                                          gdouble                position);
  gboolean      (* is_extendable)        (PicmanStroke            *stroke,
                                          PicmanAnchor            *neighbor);
  PicmanAnchor  * (* extend)               (PicmanStroke            *stroke,
                                          const PicmanCoords      *coords,
                                          PicmanAnchor            *neighbor,
                                          PicmanVectorExtendMode   extend_mode);
  gboolean      (* connect_stroke)       (PicmanStroke            *stroke,
                                          PicmanAnchor            *anchor,
                                          PicmanStroke            *extension,
                                          PicmanAnchor            *neighbor);

  gboolean      (* is_empty)             (const PicmanStroke      *stroke);
  gdouble       (* get_length)           (const PicmanStroke      *stroke,
                                          const gdouble          precision);
  gdouble       (* get_distance)         (const PicmanStroke      *stroke,
                                          const PicmanCoords      *coord);
  gboolean      (* get_point_at_dist)    (const PicmanStroke      *stroke,
                                          const gdouble          dist,
                                          const gdouble          precision,
                                          PicmanCoords            *position,
                                          gdouble               *slope);

  GArray      * (* interpolate)          (const PicmanStroke      *stroke,
                                          const gdouble          precision,
                                          gboolean              *ret_closed);

  PicmanStroke  * (* duplicate)            (const PicmanStroke      *stroke);

  PicmanBezierDesc * (* make_bezier)       (const PicmanStroke      *stroke);

  void          (* translate)            (PicmanStroke            *stroke,
                                          gdouble                offset_x,
                                          gdouble                offset_y);
  void          (* scale)                (PicmanStroke            *stroke,
                                          gdouble                scale_x,
                                          gdouble                scale_y);
  void          (* rotate)               (PicmanStroke            *stroke,
                                          gdouble                center_x,
                                          gdouble                center_y,
                                          gdouble                angle);
  void          (* flip)                 (PicmanStroke             *stroke,
                                          PicmanOrientationType    flip_type,
                                          gdouble                axis);
  void          (* flip_free)            (PicmanStroke            *stroke,
                                          gdouble                x1,
                                          gdouble                y1,
                                          gdouble                x2,
                                          gdouble                y2);
  void          (* transform)            (PicmanStroke            *stroke,
                                          const PicmanMatrix3     *matrix);

  GList       * (* get_draw_anchors)     (const PicmanStroke      *stroke);
  GList       * (* get_draw_controls)    (const PicmanStroke      *stroke);
  GArray      * (* get_draw_lines)       (const PicmanStroke      *stroke);
  GArray      * (* control_points_get)   (const PicmanStroke      *stroke,
                                          gboolean              *ret_closed);
};


GType        picman_stroke_get_type             (void) G_GNUC_CONST;

void         picman_stroke_set_ID               (PicmanStroke            *stroke,
                                               gint                   id);
gint         picman_stroke_get_ID               (const PicmanStroke      *stroke);


/* accessing / modifying the anchors */

GArray     * picman_stroke_control_points_get   (const PicmanStroke      *stroke,
                                               gboolean              *closed);

PicmanAnchor * picman_stroke_anchor_get           (const PicmanStroke      *stroke,
                                               const PicmanCoords      *coord);

gdouble      picman_stroke_nearest_point_get    (const PicmanStroke      *stroke,
                                               const PicmanCoords      *coord,
                                               const gdouble          precision,
                                               PicmanCoords            *ret_point,
                                               PicmanAnchor           **ret_segment_start,
                                               PicmanAnchor           **ret_segment_end,
                                               gdouble               *ret_pos);
gdouble     picman_stroke_nearest_tangent_get   (const PicmanStroke      *stroke,
                                               const PicmanCoords      *coords1,
                                               const PicmanCoords      *coords2,
                                               gdouble                precision,
                                               PicmanCoords            *nearest,
                                               PicmanAnchor           **ret_segment_start,
                                               PicmanAnchor           **ret_segment_end,
                                               gdouble               *ret_pos);
gdouble  picman_stroke_nearest_intersection_get (const PicmanStroke      *stroke,
                                               const PicmanCoords      *coords1,
                                               const PicmanCoords      *direction,
                                               gdouble                precision,
                                               PicmanCoords            *nearest,
                                               PicmanAnchor           **ret_segment_start,
                                               PicmanAnchor           **ret_segment_end,
                                               gdouble               *ret_pos);


/* prev == NULL: "first" anchor */
PicmanAnchor * picman_stroke_anchor_get_next      (const PicmanStroke      *stroke,
                                               const PicmanAnchor      *prev);

void         picman_stroke_anchor_select        (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor,
                                               gboolean               selected,
                                               gboolean               exclusive);

/* type will be an xorable enum:
 * VECTORS_NONE, VECTORS_FIX_ANGLE, VECTORS_FIX_RATIO, VECTORS_RESTRICT_ANGLE
 *  or so.
 */
void         picman_stroke_anchor_move_relative (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor,
                                               const PicmanCoords      *delta,
                                               PicmanAnchorFeatureType  feature);
void         picman_stroke_anchor_move_absolute (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor,
                                               const PicmanCoords      *coord,
                                               PicmanAnchorFeatureType  feature);

gboolean     picman_stroke_point_is_movable     (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position);
void         picman_stroke_point_move_relative  (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position,
                                               const PicmanCoords      *deltacoord,
                                               PicmanAnchorFeatureType  feature);
void         picman_stroke_point_move_absolute  (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position,
                                               const PicmanCoords      *coord,
                                               PicmanAnchorFeatureType  feature);

void         picman_stroke_close                (PicmanStroke            *stroke);

void         picman_stroke_anchor_convert       (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor,
                                               PicmanAnchorFeatureType  feature);

void         picman_stroke_anchor_delete        (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor);

PicmanStroke * picman_stroke_open                 (PicmanStroke            *stroke,
                                               PicmanAnchor            *end_anchor);
gboolean     picman_stroke_anchor_is_insertable (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position);
PicmanAnchor * picman_stroke_anchor_insert        (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position);

gboolean     picman_stroke_is_extendable        (PicmanStroke            *stroke,
                                               PicmanAnchor            *neighbor);

PicmanAnchor * picman_stroke_extend               (PicmanStroke            *stroke,
                                               const PicmanCoords      *coords,
                                               PicmanAnchor            *neighbor,
                                               PicmanVectorExtendMode   extend_mode);

gboolean     picman_stroke_connect_stroke       (PicmanStroke            *stroke,
                                               PicmanAnchor            *anchor,
                                               PicmanStroke            *extension,
                                               PicmanAnchor            *neighbor);

gboolean     picman_stroke_is_empty             (const PicmanStroke      *stroke);

/* accessing the shape of the curve */

gdouble      picman_stroke_get_length           (const PicmanStroke      *stroke,
                                               const gdouble          precision);
gdouble      picman_stroke_get_distance         (const PicmanStroke      *stroke,
                                               const PicmanCoords      *coord);

gboolean     picman_stroke_get_point_at_dist    (const PicmanStroke      *stroke,
                                               const gdouble          dist,
                                               const gdouble          precision,
                                               PicmanCoords            *position,
                                               gdouble               *slope);

/* returns an array of valid coordinates */
GArray     * picman_stroke_interpolate          (const PicmanStroke      *stroke,
                                               const gdouble          precision,
                                               gboolean              *closed);

PicmanStroke * picman_stroke_duplicate            (const PicmanStroke      *stroke);

/* creates a bezier approximation. */
PicmanBezierDesc * picman_stroke_make_bezier      (const PicmanStroke      *stroke);

void         picman_stroke_translate            (PicmanStroke            *stroke,
                                               gdouble                offset_x,
                                               gdouble                offset_y);
void         picman_stroke_scale                (PicmanStroke            *stroke,
                                               gdouble                scale_x,
                                               gdouble                scale_y);
void         picman_stroke_rotate               (PicmanStroke            *stroke,
                                               gdouble                center_x,
                                               gdouble                center_y,
                                               gdouble                angle);
void         picman_stroke_flip                 (PicmanStroke             *stroke,
                                               PicmanOrientationType    flip_type,
                                               gdouble                axis);
void         picman_stroke_flip_free            (PicmanStroke            *stroke,
                                               gdouble                x1,
                                               gdouble                y1,
                                               gdouble                x2,
                                               gdouble                y2);
void         picman_stroke_transform            (PicmanStroke            *stroke,
                                               const PicmanMatrix3     *matrix);


GList      * picman_stroke_get_draw_anchors     (const PicmanStroke      *stroke);
GList      * picman_stroke_get_draw_controls    (const PicmanStroke      *stroke);
GArray     * picman_stroke_get_draw_lines       (const PicmanStroke      *stroke);

#endif /* __PICMAN_STROKE_H__ */

