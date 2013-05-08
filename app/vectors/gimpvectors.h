/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectors.h
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

#ifndef __PICMAN_VECTORS_H__
#define __PICMAN_VECTORS_H__

#include "core/picmanitem.h"

#define PICMAN_TYPE_VECTORS            (picman_vectors_get_type ())
#define PICMAN_VECTORS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VECTORS, PicmanVectors))
#define PICMAN_VECTORS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VECTORS, PicmanVectorsClass))
#define PICMAN_IS_VECTORS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VECTORS))
#define PICMAN_IS_VECTORS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VECTORS))
#define PICMAN_VECTORS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VECTORS, PicmanVectorsClass))


typedef struct _PicmanVectorsClass  PicmanVectorsClass;

struct _PicmanVectors
{
  PicmanItem        parent_instance;

  GList          *strokes;        /* The List of PicmanStrokes      */
  gint            last_stroke_ID;

  gint            freeze_count;
  gdouble         precision;

  PicmanBezierDesc *bezier_desc;    /* Cached bezier representation */

  gboolean        bounds_valid;   /* Cached bounding box          */
  gboolean        bounds_empty;
  gdouble         bounds_x1;
  gdouble         bounds_y1;
  gdouble         bounds_x2;
  gdouble         bounds_y2;
};

struct _PicmanVectorsClass
{
  PicmanItemClass  parent_class;

  /*  signals  */
  void          (* freeze)            (PicmanVectors       *vectors);
  void          (* thaw)              (PicmanVectors       *vectors);

  /*  virtual functions  */
  void          (* stroke_add)        (PicmanVectors       *vectors,
                                       PicmanStroke        *stroke);
  void          (* stroke_remove)     (PicmanVectors       *vectors,
                                       PicmanStroke        *stroke);
  PicmanStroke  * (* stroke_get)        (const PicmanVectors *vectors,
                                       const PicmanCoords  *coord);
  PicmanStroke  * (* stroke_get_next)   (const PicmanVectors *vectors,
                                       const PicmanStroke  *prev);
  gdouble       (* stroke_get_length) (const PicmanVectors *vectors,
                                       const PicmanStroke  *stroke);
  PicmanAnchor  * (* anchor_get)        (const PicmanVectors *vectors,
                                       const PicmanCoords  *coord,
                                       PicmanStroke       **ret_stroke);
  void          (* anchor_delete)     (PicmanVectors       *vectors,
                                       PicmanAnchor        *anchor);
  gdouble       (* get_length)        (const PicmanVectors *vectors,
                                       const PicmanAnchor  *start);
  gdouble       (* get_distance)      (const PicmanVectors *vectors,
                                       const PicmanCoords  *coord);
  gint          (* interpolate)       (const PicmanVectors *vectors,
                                       const PicmanStroke  *stroke,
                                       gdouble            precision,
                                       gint               max_points,
                                       PicmanCoords        *ret_coords);
  PicmanBezierDesc * (* make_bezier)    (const PicmanVectors *vectors);
};


/*  vectors utility functions  */

GType           picman_vectors_get_type           (void) G_GNUC_CONST;

PicmanVectors   * picman_vectors_new                (PicmanImage         *image,
                                                 const gchar       *name);

PicmanVectors   * picman_vectors_get_parent         (PicmanVectors       *vectors);

void            picman_vectors_freeze             (PicmanVectors       *vectors);
void            picman_vectors_thaw               (PicmanVectors       *vectors);

void            picman_vectors_copy_strokes       (const PicmanVectors *src_vectors,
                                                 PicmanVectors       *dest_vectors);
void            picman_vectors_add_strokes        (const PicmanVectors *src_vectors,
                                                 PicmanVectors       *dest_vectors);


/* accessing / modifying the anchors */

PicmanAnchor    * picman_vectors_anchor_get         (const PicmanVectors *vectors,
                                                 const PicmanCoords  *coord,
                                                 PicmanStroke       **ret_stroke);

/* prev == NULL: "first" anchor */
PicmanAnchor    * picman_vectors_anchor_get_next    (const PicmanVectors  *vectors,
                                                 const PicmanAnchor   *prev);

/* type will be an xorable enum:
 * VECTORS_NONE, VECTORS_FIX_ANGLE, VECTORS_FIX_RATIO, VECTORS_RESTRICT_ANGLE
 *  or so.
 */
void          picman_vectors_anchor_move_relative (PicmanVectors        *vectors,
                                                 PicmanAnchor         *anchor,
                                                 const PicmanCoords   *deltacoord,
                                                 gint                type);
void          picman_vectors_anchor_move_absolute (PicmanVectors        *vectors,
                                                 PicmanAnchor         *anchor,
                                                 const PicmanCoords   *coord,
                                                 gint                type);

void          picman_vectors_anchor_delete        (PicmanVectors        *vectors,
                                                 PicmanAnchor         *anchor);

void          picman_vectors_anchor_select        (PicmanVectors        *vectors,
                                                 PicmanStroke         *target_stroke,
                                                 PicmanAnchor         *anchor,
                                                 gboolean            selected,
                                                 gboolean            exclusive);


/* PicmanStroke is a connected component of a PicmanVectors object */

void            picman_vectors_stroke_add         (PicmanVectors        *vectors,
                                                 PicmanStroke         *stroke);
void            picman_vectors_stroke_remove      (PicmanVectors        *vectors,
                                                 PicmanStroke         *stroke);
gint            picman_vectors_get_n_strokes      (const PicmanVectors  *vectors);
PicmanStroke    * picman_vectors_stroke_get         (const PicmanVectors  *vectors,
                                                 const PicmanCoords   *coord);
PicmanStroke    * picman_vectors_stroke_get_by_ID   (const PicmanVectors  *vectors,
                                                 gint                id);

/* prev == NULL: "first" stroke */
PicmanStroke    * picman_vectors_stroke_get_next    (const PicmanVectors  *vectors,
                                                 const PicmanStroke   *prev);
gdouble         picman_vectors_stroke_get_length  (const PicmanVectors  *vectors,
                                                 const PicmanStroke   *stroke);

/* accessing the shape of the curve */

gdouble         picman_vectors_get_length         (const PicmanVectors  *vectors,
                                                 const PicmanAnchor   *start);
gdouble         picman_vectors_get_distance       (const PicmanVectors  *vectors,
                                                 const PicmanCoords   *coord);

gboolean        picman_vectors_bounds             (PicmanVectors        *vectors,
                                                 gdouble            *x1,
                                                 gdouble            *y1,
                                                 gdouble            *x2,
                                                 gdouble            *y2);


/* returns the number of valid coordinates */
gint            picman_vectors_interpolate        (const PicmanVectors  *vectors,
                                                 const PicmanStroke   *stroke,
                                                 gdouble             precision,
                                                 gint                max_points,
                                                 PicmanCoords         *ret_coords);

/* usually overloaded */

/* returns a bezier representation */
const PicmanBezierDesc * picman_vectors_get_bezier  (PicmanVectors        *vectors);


#endif /* __PICMAN_VECTORS_H__ */
