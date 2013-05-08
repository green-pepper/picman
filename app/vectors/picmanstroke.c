/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanstroke.c
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

#include "config.h"

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "vectors-types.h"

#include "core/picman-utils.h"
#include "core/picmancoords.h"
#include "core/picmanparamspecs.h"
#include "core/picman-transform-utils.h"

#include "picmananchor.h"
#include "picmanstroke.h"

enum
{
  PROP_0,
  PROP_CONTROL_POINTS,
  PROP_CLOSED
};

/* Prototypes */

static void    picman_stroke_set_property              (GObject      *object,
                                                      guint         property_id,
                                                      const GValue *value,
                                                      GParamSpec   *pspec);
static void    picman_stroke_get_property              (GObject      *object,
                                                      guint         property_id,
                                                      GValue       *value,
                                                      GParamSpec   *pspec);
static void    picman_stroke_finalize                  (GObject      *object);

static gint64  picman_stroke_get_memsize               (PicmanObject   *object,
                                                      gint64       *gui_size);

static PicmanAnchor * picman_stroke_real_anchor_get      (const PicmanStroke *stroke,
                                                      const PicmanCoords *coord);
static PicmanAnchor * picman_stroke_real_anchor_get_next (const PicmanStroke *stroke,
                                                      const PicmanAnchor *prev);
static void         picman_stroke_real_anchor_select   (PicmanStroke       *stroke,
                                                      PicmanAnchor       *anchor,
                                                      gboolean          selected,
                                                      gboolean          exclusive);
static void    picman_stroke_real_anchor_move_relative (PicmanStroke       *stroke,
                                                      PicmanAnchor       *anchor,
                                                      const PicmanCoords *delta,
                                                      PicmanAnchorFeatureType feature);
static void    picman_stroke_real_anchor_move_absolute (PicmanStroke       *stroke,
                                                      PicmanAnchor       *anchor,
                                                      const PicmanCoords *delta,
                                                      PicmanAnchorFeatureType feature);
static void         picman_stroke_real_anchor_convert  (PicmanStroke       *stroke,
                                                      PicmanAnchor       *anchor,
                                                      PicmanAnchorFeatureType  feature);
static void         picman_stroke_real_anchor_delete   (PicmanStroke       *stroke,
                                                      PicmanAnchor       *anchor);
static gboolean     picman_stroke_real_point_is_movable
                                              (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position);
static void         picman_stroke_real_point_move_relative
                                              (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position,
                                               const PicmanCoords      *deltacoord,
                                               PicmanAnchorFeatureType  feature);
static void         picman_stroke_real_point_move_absolute
                                              (PicmanStroke            *stroke,
                                               PicmanAnchor            *predec,
                                               gdouble                position,
                                               const PicmanCoords      *coord,
                                               PicmanAnchorFeatureType  feature);

static void         picman_stroke_real_close           (PicmanStroke       *stroke);
static PicmanStroke * picman_stroke_real_open            (PicmanStroke       *stroke,
                                                      PicmanAnchor       *end_anchor);
static gboolean     picman_stroke_real_anchor_is_insertable
                                                     (PicmanStroke       *stroke,
                                                      PicmanAnchor       *predec,
                                                      gdouble           position);
static PicmanAnchor * picman_stroke_real_anchor_insert   (PicmanStroke       *stroke,
                                                      PicmanAnchor       *predec,
                                                      gdouble           position);

static gboolean     picman_stroke_real_is_extendable   (PicmanStroke       *stroke,
                                                      PicmanAnchor       *neighbor);

static PicmanAnchor * picman_stroke_real_extend (PicmanStroke           *stroke,
                                             const PicmanCoords     *coords,
                                             PicmanAnchor           *neighbor,
                                             PicmanVectorExtendMode  extend_mode);

gboolean     picman_stroke_real_connect_stroke (PicmanStroke          *stroke,
                                              PicmanAnchor          *anchor,
                                              PicmanStroke          *extension,
                                              PicmanAnchor          *neighbor);


static gboolean     picman_stroke_real_is_empty        (const PicmanStroke *stroke);

static gdouble      picman_stroke_real_get_length      (const PicmanStroke *stroke,
                                                      const gdouble     precision);
static gdouble      picman_stroke_real_get_distance    (const PicmanStroke *stroke,
                                                      const PicmanCoords *coord);
static GArray *     picman_stroke_real_interpolate     (const PicmanStroke *stroke,
                                                      gdouble           precision,
                                                      gboolean         *closed);
static PicmanStroke * picman_stroke_real_duplicate       (const PicmanStroke *stroke);
static PicmanBezierDesc * picman_stroke_real_make_bezier (const PicmanStroke *stroke);

static void         picman_stroke_real_translate       (PicmanStroke       *stroke,
                                                      gdouble           offset_x,
                                                      gdouble           offset_y);
static void         picman_stroke_real_scale           (PicmanStroke       *stroke,
                                                      gdouble           scale_x,
                                                      gdouble           scale_y);
static void         picman_stroke_real_rotate          (PicmanStroke *stroke,
                                                      gdouble     center_x,
                                                      gdouble     center_y,
                                                      gdouble     angle);
static void         picman_stroke_real_flip            (PicmanStroke          *stroke,
                                                      PicmanOrientationType  flip_type,
                                                      gdouble              axis);
static void         picman_stroke_real_flip_free       (PicmanStroke          *stroke,
                                                      gdouble              x1,
                                                      gdouble              y1,
                                                      gdouble              x2,
                                                      gdouble              y2);
static void         picman_stroke_real_transform       (PicmanStroke        *stroke,
                                                      const PicmanMatrix3 *matrix);

static GList    * picman_stroke_real_get_draw_anchors  (const PicmanStroke *stroke);
static GList    * picman_stroke_real_get_draw_controls (const PicmanStroke *stroke);
static GArray   * picman_stroke_real_get_draw_lines    (const PicmanStroke *stroke);
static GArray *  picman_stroke_real_control_points_get (const PicmanStroke *stroke,
                                                      gboolean         *ret_closed);
static gboolean   picman_stroke_real_get_point_at_dist (const PicmanStroke *stroke,
                                                      const gdouble     dist,
                                                      const gdouble     precision,
                                                      PicmanCoords       *position,
                                                      gdouble          *slope);


G_DEFINE_TYPE (PicmanStroke, picman_stroke, PICMAN_TYPE_OBJECT)

#define parent_class picman_stroke_parent_class


static void
picman_stroke_class_init (PicmanStrokeClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  GParamSpec      *param_spec;

  object_class->finalize          = picman_stroke_finalize;
  object_class->get_property      = picman_stroke_get_property;
  object_class->set_property      = picman_stroke_set_property;

  picman_object_class->get_memsize  = picman_stroke_get_memsize;

  klass->changed                  = NULL;
  klass->removed                  = NULL;

  klass->anchor_get               = picman_stroke_real_anchor_get;
  klass->anchor_get_next          = picman_stroke_real_anchor_get_next;
  klass->anchor_select            = picman_stroke_real_anchor_select;
  klass->anchor_move_relative     = picman_stroke_real_anchor_move_relative;
  klass->anchor_move_absolute     = picman_stroke_real_anchor_move_absolute;
  klass->anchor_convert           = picman_stroke_real_anchor_convert;
  klass->anchor_delete            = picman_stroke_real_anchor_delete;

  klass->point_is_movable         = picman_stroke_real_point_is_movable;
  klass->point_move_relative      = picman_stroke_real_point_move_relative;
  klass->point_move_absolute      = picman_stroke_real_point_move_absolute;

  klass->nearest_point_get        = NULL;
  klass->nearest_tangent_get      = NULL;
  klass->nearest_intersection_get = NULL;
  klass->close                    = picman_stroke_real_close;
  klass->open                     = picman_stroke_real_open;
  klass->anchor_is_insertable     = picman_stroke_real_anchor_is_insertable;
  klass->anchor_insert            = picman_stroke_real_anchor_insert;
  klass->is_extendable            = picman_stroke_real_is_extendable;
  klass->extend                   = picman_stroke_real_extend;
  klass->connect_stroke           = picman_stroke_real_connect_stroke;

  klass->is_empty                 = picman_stroke_real_is_empty;
  klass->get_length               = picman_stroke_real_get_length;
  klass->get_distance             = picman_stroke_real_get_distance;
  klass->get_point_at_dist        = picman_stroke_real_get_point_at_dist;
  klass->interpolate              = picman_stroke_real_interpolate;

  klass->duplicate                = picman_stroke_real_duplicate;
  klass->make_bezier              = picman_stroke_real_make_bezier;

  klass->translate                = picman_stroke_real_translate;
  klass->scale                    = picman_stroke_real_scale;
  klass->rotate                   = picman_stroke_real_rotate;
  klass->flip                     = picman_stroke_real_flip;
  klass->flip_free                = picman_stroke_real_flip_free;
  klass->transform                = picman_stroke_real_transform;


  klass->get_draw_anchors         = picman_stroke_real_get_draw_anchors;
  klass->get_draw_controls        = picman_stroke_real_get_draw_controls;
  klass->get_draw_lines           = picman_stroke_real_get_draw_lines;
  klass->control_points_get       = picman_stroke_real_control_points_get;

  param_spec = g_param_spec_boxed ("picman-anchor",
                                   "Picman Anchor",
                                   "The control points of a Stroke",
                                   PICMAN_TYPE_ANCHOR,
                                   PICMAN_PARAM_WRITABLE |
                                   G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (object_class, PROP_CONTROL_POINTS,
                                   picman_param_spec_value_array ("control-points",
                                                                "Control Points",
                                                                "This is an ValueArray "
                                                                "with the initial "
                                                                "control points of "
                                                                "the new Stroke",
                                                                param_spec,
                                                                PICMAN_PARAM_WRITABLE |
                                                                G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CLOSED,
                                   g_param_spec_boolean ("closed",
                                                         "Close Flag",
                                                         "this flag indicates "
                                                         "whether the stroke "
                                                         "is closed or not",
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_stroke_init (PicmanStroke *stroke)
{
  stroke->ID      = 0;
  stroke->anchors = NULL;
  stroke->closed  = FALSE;
}

static void
picman_stroke_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PicmanStroke     *stroke = PICMAN_STROKE (object);
  PicmanValueArray *val_array;
  gint            length;
  gint            i;

  switch (property_id)
    {
    case PROP_CLOSED:
      stroke->closed = g_value_get_boolean (value);
      break;

    case PROP_CONTROL_POINTS:
      g_return_if_fail (stroke->anchors == NULL);
      g_return_if_fail (value != NULL);

      val_array = g_value_get_boxed (value);

      if (val_array == NULL)
        return;

      length = picman_value_array_length (val_array);

      for (i = 0; i < length; i++)
        {
          GValue *item = picman_value_array_index (val_array, i);

          g_return_if_fail (G_VALUE_HOLDS (item, PICMAN_TYPE_ANCHOR));
          stroke->anchors = g_list_append (stroke->anchors,
                                           g_value_dup_boxed (item));
        }

      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_stroke_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PicmanStroke *stroke = PICMAN_STROKE (object);

  switch (property_id)
    {
    case PROP_CLOSED:
      g_value_set_boolean (value, stroke->closed);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_stroke_finalize (GObject *object)
{
  PicmanStroke *stroke = PICMAN_STROKE (object);

  if (stroke->anchors)
    {
      g_list_free_full (stroke->anchors, (GDestroyNotify) picman_anchor_free);
      stroke->anchors = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_stroke_get_memsize (PicmanObject *object,
                         gint64     *gui_size)
{
  PicmanStroke *stroke  = PICMAN_STROKE (object);
  gint64      memsize = 0;

  memsize += picman_g_list_get_memsize (stroke->anchors, sizeof (PicmanAnchor));

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

void
picman_stroke_set_ID (PicmanStroke *stroke,
                    gint        id)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));
  g_return_if_fail (stroke->ID == 0 /* we don't want changing IDs... */);

  stroke->ID = id;
}

gint
picman_stroke_get_ID (const PicmanStroke *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), -1);

  return stroke->ID;
}


PicmanAnchor *
picman_stroke_anchor_get (const PicmanStroke *stroke,
                        const PicmanCoords *coord)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->anchor_get (stroke, coord);
}


gdouble
picman_stroke_nearest_point_get (const PicmanStroke *stroke,
                               const PicmanCoords *coord,
                               const gdouble     precision,
                               PicmanCoords       *ret_point,
                               PicmanAnchor      **ret_segment_start,
                               PicmanAnchor      **ret_segment_end,
                               gdouble          *ret_pos)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);
  g_return_val_if_fail (coord != NULL, FALSE);

  if (PICMAN_STROKE_GET_CLASS (stroke)->nearest_point_get)
    return PICMAN_STROKE_GET_CLASS (stroke)->nearest_point_get (stroke,
                                                              coord,
                                                              precision,
                                                              ret_point,
                                                              ret_segment_start,
                                                              ret_segment_end,
                                                              ret_pos);
  return -1;
}

gdouble
picman_stroke_nearest_tangent_get   (const PicmanStroke      *stroke,
                                   const PicmanCoords      *coords1,
                                   const PicmanCoords      *coords2,
                                   gdouble                precision,
                                   PicmanCoords            *nearest,
                                   PicmanAnchor           **ret_segment_start,
                                   PicmanAnchor           **ret_segment_end,
                                   gdouble               *ret_pos)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);
  g_return_val_if_fail (coords1 != NULL, FALSE);
  g_return_val_if_fail (coords2 != NULL, FALSE);

  if (PICMAN_STROKE_GET_CLASS (stroke)->nearest_tangent_get)
    return PICMAN_STROKE_GET_CLASS (stroke)->nearest_tangent_get (stroke,
                                                                coords1,
                                                                coords2,
                                                                precision,
                                                                nearest,
                                                                ret_segment_start,
                                                                ret_segment_end,
                                                                ret_pos);
  return -1;
}

gdouble
picman_stroke_nearest_intersection_get (const PicmanStroke      *stroke,
                                      const PicmanCoords      *coords1,
                                      const PicmanCoords      *direction,
                                      gdouble                precision,
                                      PicmanCoords            *nearest,
                                      PicmanAnchor           **ret_segment_start,
                                      PicmanAnchor           **ret_segment_end,
                                      gdouble               *ret_pos)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);
  g_return_val_if_fail (coords1 != NULL, FALSE);
  g_return_val_if_fail (direction != NULL, FALSE);

  if (PICMAN_STROKE_GET_CLASS (stroke)->nearest_intersection_get)
    return PICMAN_STROKE_GET_CLASS (stroke)->nearest_intersection_get (stroke,
                                                                     coords1,
                                                                     direction,
                                                                     precision,
                                                                     nearest,
                                                                     ret_segment_start,
                                                                     ret_segment_end,
                                                                     ret_pos);
  return -1;
}

static PicmanAnchor *
picman_stroke_real_anchor_get (const PicmanStroke *stroke,
                             const PicmanCoords *coord)
{
  gdouble     dx, dy;
  gdouble     mindist = -1;
  GList      *anchors;
  GList      *list;
  PicmanAnchor *anchor = NULL;

  anchors = picman_stroke_get_draw_controls (stroke);

  for (list = anchors; list; list = g_list_next (list))
    {
      dx = coord->x - PICMAN_ANCHOR (list->data)->position.x;
      dy = coord->y - PICMAN_ANCHOR (list->data)->position.y;

      if (mindist < 0 || mindist > dx * dx + dy * dy)
        {
          mindist = dx * dx + dy * dy;
          anchor = PICMAN_ANCHOR (list->data);
        }
    }

  g_list_free (anchors);

  anchors = picman_stroke_get_draw_anchors (stroke);

  for (list = anchors; list; list = g_list_next (list))
    {
      dx = coord->x - PICMAN_ANCHOR (list->data)->position.x;
      dy = coord->y - PICMAN_ANCHOR (list->data)->position.y;

      if (mindist < 0 || mindist > dx * dx + dy * dy)
        {
          mindist = dx * dx + dy * dy;
          anchor = PICMAN_ANCHOR (list->data);
        }
    }

  g_list_free (anchors);

  return anchor;
}


PicmanAnchor *
picman_stroke_anchor_get_next (const PicmanStroke *stroke,
                             const PicmanAnchor *prev)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->anchor_get_next (stroke, prev);
}

static PicmanAnchor *
picman_stroke_real_anchor_get_next (const PicmanStroke *stroke,
                                  const PicmanAnchor *prev)
{
  GList *list;

  if (prev)
    {
      list = g_list_find (stroke->anchors, prev);
      if (list)
        list = g_list_next (list);
    }
  else
    {
      list = stroke->anchors;
    }

  if (list)
    return PICMAN_ANCHOR (list->data);

  return NULL;
}


void
picman_stroke_anchor_select (PicmanStroke *stroke,
                           PicmanAnchor *anchor,
                           gboolean    selected,
                           gboolean    exclusive)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->anchor_select (stroke, anchor,
                                                 selected, exclusive);
}

static void
picman_stroke_real_anchor_select (PicmanStroke *stroke,
                                PicmanAnchor *anchor,
                                gboolean    selected,
                                gboolean    exclusive)
{
  GList *list;

  list = stroke->anchors;

  if (exclusive)
    {
      while (list)
        {
          PICMAN_ANCHOR (list->data)->selected = FALSE;
          list = g_list_next (list);
        }
    }

  list = g_list_find (stroke->anchors, anchor);

  if (list)
    PICMAN_ANCHOR (list->data)->selected = selected;
}


void
picman_stroke_anchor_move_relative (PicmanStroke            *stroke,
                                  PicmanAnchor            *anchor,
                                  const PicmanCoords      *delta,
                                  PicmanAnchorFeatureType  feature)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));
  g_return_if_fail (anchor != NULL);
  g_return_if_fail (g_list_find (stroke->anchors, anchor));

  PICMAN_STROKE_GET_CLASS (stroke)->anchor_move_relative (stroke, anchor,
                                                        delta, feature);
}

static void
picman_stroke_real_anchor_move_relative (PicmanStroke            *stroke,
                                       PicmanAnchor            *anchor,
                                       const PicmanCoords      *delta,
                                       PicmanAnchorFeatureType  feature)
{
  anchor->position.x += delta->x;
  anchor->position.y += delta->y;
}


void
picman_stroke_anchor_move_absolute (PicmanStroke            *stroke,
                                  PicmanAnchor            *anchor,
                                  const PicmanCoords      *coord,
                                  PicmanAnchorFeatureType  feature)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));
  g_return_if_fail (anchor != NULL);
  g_return_if_fail (g_list_find (stroke->anchors, anchor));

  PICMAN_STROKE_GET_CLASS (stroke)->anchor_move_absolute (stroke, anchor,
                                                        coord, feature);
}

static void
picman_stroke_real_anchor_move_absolute (PicmanStroke            *stroke,
                                       PicmanAnchor            *anchor,
                                       const PicmanCoords      *coord,
                                       PicmanAnchorFeatureType  feature)
{
  anchor->position.x = coord->x;
  anchor->position.y = coord->y;
}

gboolean
picman_stroke_point_is_movable (PicmanStroke *stroke,
                              PicmanAnchor *predec,
                              gdouble     position)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

  return PICMAN_STROKE_GET_CLASS (stroke)->point_is_movable (stroke, predec,
                                                           position);
}


static gboolean
picman_stroke_real_point_is_movable (PicmanStroke *stroke,
                                   PicmanAnchor *predec,
                                   gdouble     position)
{
  return FALSE;
}


void
picman_stroke_point_move_relative (PicmanStroke            *stroke,
                                 PicmanAnchor            *predec,
                                 gdouble                position,
                                 const PicmanCoords      *deltacoord,
                                 PicmanAnchorFeatureType  feature)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->point_move_relative (stroke, predec,
                                                       position, deltacoord,
                                                       feature);
}


static void
picman_stroke_real_point_move_relative (PicmanStroke           *stroke,
                                      PicmanAnchor           *predec,
                                      gdouble               position,
                                      const PicmanCoords     *deltacoord,
                                      PicmanAnchorFeatureType feature)
{
  g_printerr ("picman_stroke_point_move_relative: default implementation\n");
}


void
picman_stroke_point_move_absolute (PicmanStroke            *stroke,
                                 PicmanAnchor            *predec,
                                 gdouble                position,
                                 const PicmanCoords      *coord,
                                 PicmanAnchorFeatureType  feature)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->point_move_absolute (stroke, predec,
                                                       position, coord,
                                                       feature);
}

static void
picman_stroke_real_point_move_absolute (PicmanStroke           *stroke,
                                      PicmanAnchor           *predec,
                                      gdouble               position,
                                      const PicmanCoords     *coord,
                                      PicmanAnchorFeatureType feature)
{
  g_printerr ("picman_stroke_point_move_absolute: default implementation\n");
}


void
picman_stroke_close (PicmanStroke *stroke)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));
  g_return_if_fail (stroke->anchors != NULL);

  PICMAN_STROKE_GET_CLASS (stroke)->close (stroke);
}

static void
picman_stroke_real_close (PicmanStroke *stroke)
{
  stroke->closed = TRUE;
  g_object_notify (G_OBJECT (stroke), "closed");
}


void
picman_stroke_anchor_convert (PicmanStroke            *stroke,
                            PicmanAnchor            *anchor,
                            PicmanAnchorFeatureType  feature)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->anchor_convert (stroke, anchor, feature);
}

static void
picman_stroke_real_anchor_convert (PicmanStroke            *stroke,
                                 PicmanAnchor            *anchor,
                                 PicmanAnchorFeatureType  feature)
{
  g_printerr ("picman_stroke_anchor_convert: default implementation\n");
}


void
picman_stroke_anchor_delete (PicmanStroke *stroke,
                           PicmanAnchor *anchor)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));
  g_return_if_fail (anchor && anchor->type == PICMAN_ANCHOR_ANCHOR);

  PICMAN_STROKE_GET_CLASS (stroke)->anchor_delete (stroke, anchor);
}

static void
picman_stroke_real_anchor_delete (PicmanStroke *stroke,
                                PicmanAnchor *anchor)
{
  g_printerr ("picman_stroke_anchor_delete: default implementation\n");
}

PicmanStroke *
picman_stroke_open (PicmanStroke *stroke,
                  PicmanAnchor *end_anchor)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);
  g_return_val_if_fail (end_anchor &&
                        end_anchor->type == PICMAN_ANCHOR_ANCHOR, NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->open (stroke, end_anchor);
}

static PicmanStroke *
picman_stroke_real_open (PicmanStroke *stroke,
                       PicmanAnchor *end_anchor)
{
  g_printerr ("picman_stroke_open: default implementation\n");
  return NULL;
}

gboolean
picman_stroke_anchor_is_insertable (PicmanStroke *stroke,
                                  PicmanAnchor *predec,
                                  gdouble     position)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

  return PICMAN_STROKE_GET_CLASS (stroke)->anchor_is_insertable (stroke,
                                                               predec,
                                                               position);
}

static gboolean
picman_stroke_real_anchor_is_insertable (PicmanStroke *stroke,
                                       PicmanAnchor *predec,
                                       gdouble     position)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

  return FALSE;
}

PicmanAnchor *
picman_stroke_anchor_insert (PicmanStroke *stroke,
                           PicmanAnchor *predec,
                           gdouble     position)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);
  g_return_val_if_fail (predec->type == PICMAN_ANCHOR_ANCHOR, NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->anchor_insert (stroke,
                                                        predec, position);
}

static PicmanAnchor *
picman_stroke_real_anchor_insert (PicmanStroke *stroke,
                                PicmanAnchor *predec,
                                gdouble     position)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return NULL;
}


gboolean
picman_stroke_is_extendable (PicmanStroke *stroke,
                           PicmanAnchor *neighbor)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

  return PICMAN_STROKE_GET_CLASS (stroke)->is_extendable (stroke, neighbor);
}

static gboolean
picman_stroke_real_is_extendable (PicmanStroke *stroke,
                                PicmanAnchor *neighbor)
{
  return FALSE;
}


PicmanAnchor *
picman_stroke_extend (PicmanStroke           *stroke,
                    const PicmanCoords     *coords,
                    PicmanAnchor           *neighbor,
                    PicmanVectorExtendMode  extend_mode)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);
  g_return_val_if_fail (!stroke->closed, NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->extend (stroke, coords,
                                                 neighbor, extend_mode);
}

static PicmanAnchor *
picman_stroke_real_extend (PicmanStroke           *stroke,
                         const PicmanCoords     *coords,
                         PicmanAnchor           *neighbor,
                         PicmanVectorExtendMode  extend_mode)
{
  g_printerr ("picman_stroke_extend: default implementation\n");
  return NULL;
}

gboolean
picman_stroke_connect_stroke (PicmanStroke *stroke,
                            PicmanAnchor *anchor,
                            PicmanStroke *extension,
                            PicmanAnchor *neighbor)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);
  g_return_val_if_fail (PICMAN_IS_STROKE (extension), FALSE);
  g_return_val_if_fail (stroke->closed == FALSE &&
                        extension->closed == FALSE, FALSE);

  return PICMAN_STROKE_GET_CLASS (stroke)->connect_stroke (stroke, anchor,
                                                         extension, neighbor);
}

gboolean
picman_stroke_real_connect_stroke (PicmanStroke *stroke,
                                 PicmanAnchor *anchor,
                                 PicmanStroke *extension,
                                 PicmanAnchor *neighbor)
{
  g_printerr ("picman_stroke_connect_stroke: default implementation\n");
  return FALSE;
}

gboolean
picman_stroke_is_empty (const PicmanStroke *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

  return PICMAN_STROKE_GET_CLASS (stroke)->is_empty (stroke);
}

static gboolean
picman_stroke_real_is_empty (const PicmanStroke *stroke)
{
  return stroke->anchors == NULL;
}


gdouble
picman_stroke_get_length (const PicmanStroke *stroke,
                        const gdouble     precision)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), 0.0);

  return PICMAN_STROKE_GET_CLASS (stroke)->get_length (stroke, precision);
}

static gdouble
picman_stroke_real_get_length (const PicmanStroke *stroke,
                             const gdouble     precision)
{
  GArray     *points;
  gint        i;
  gdouble     length;
  PicmanCoords  difference;

  if (!stroke->anchors)
    return -1;

  points = picman_stroke_interpolate (stroke, precision, NULL);
  if (points == NULL)
    return -1;

  length = 0;

  for (i = 0; i < points->len - 1; i++ )
    {
       picman_coords_difference (&(g_array_index (points, PicmanCoords, i)),
                               &(g_array_index (points, PicmanCoords, i+1)),
                               &difference);
       length += picman_coords_length (&difference);
    }

  g_array_free(points, TRUE);

  return length;
}


gdouble
picman_stroke_get_distance (const PicmanStroke *stroke,
                          const PicmanCoords  *coord)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), 0.0);

  return PICMAN_STROKE_GET_CLASS (stroke)->get_distance (stroke, coord);
}

static gdouble
picman_stroke_real_get_distance (const PicmanStroke *stroke,
                               const PicmanCoords  *coord)
{
  g_printerr ("picman_stroke_get_distance: default implementation\n");

  return 0.0;
}


GArray *
picman_stroke_interpolate (const PicmanStroke *stroke,
                         gdouble           precision,
                         gboolean         *ret_closed)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->interpolate (stroke, precision,
                                                      ret_closed);
}

static GArray *
picman_stroke_real_interpolate (const PicmanStroke  *stroke,
                              gdouble            precision,
                              gboolean          *ret_closed)
{
  g_printerr ("picman_stroke_interpolate: default implementation\n");

  return NULL;
}

PicmanStroke *
picman_stroke_duplicate (const PicmanStroke *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->duplicate (stroke);
}

static PicmanStroke *
picman_stroke_real_duplicate (const PicmanStroke *stroke)
{
  PicmanStroke *new_stroke;
  GList      *list;

  new_stroke = g_object_new (G_TYPE_FROM_INSTANCE (stroke),
                             "name", picman_object_get_name (stroke),
                             NULL);

  new_stroke->anchors = g_list_copy (stroke->anchors);

  for (list = new_stroke->anchors; list; list = g_list_next (list))
    {
      list->data = picman_anchor_copy (PICMAN_ANCHOR (list->data));
    }

  new_stroke->closed = stroke->closed;
  /* we do *not* copy the ID! */

  return new_stroke;
}


PicmanBezierDesc *
picman_stroke_make_bezier (const PicmanStroke *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->make_bezier (stroke);
}

static PicmanBezierDesc *
picman_stroke_real_make_bezier (const PicmanStroke *stroke)
{
  g_printerr ("picman_stroke_make_bezier: default implementation\n");

  return NULL;
}


void
picman_stroke_translate (PicmanStroke *stroke,
                       gdouble     offset_x,
                       gdouble     offset_y)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->translate (stroke, offset_x, offset_y);
}

static void
picman_stroke_real_translate (PicmanStroke *stroke,
                            gdouble     offset_x,
                            gdouble     offset_y)
{
  GList *list;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      anchor->position.x += offset_x;
      anchor->position.y += offset_y;
    }
}


void
picman_stroke_scale (PicmanStroke *stroke,
                   gdouble     scale_x,
                   gdouble     scale_y)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->scale (stroke, scale_x, scale_y);
}

static void
picman_stroke_real_scale (PicmanStroke *stroke,
                        gdouble     scale_x,
                        gdouble     scale_y)
{
  GList *list;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      anchor->position.x *= scale_x;
      anchor->position.y *= scale_y;
    }
}

void
picman_stroke_rotate (PicmanStroke *stroke,
                    gdouble     center_x,
                    gdouble     center_y,
                    gdouble     angle)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->rotate (stroke, center_x, center_y, angle);
}

static void
picman_stroke_real_rotate (PicmanStroke *stroke,
                         gdouble     center_x,
                         gdouble     center_y,
                         gdouble     angle)
{
  PicmanMatrix3  matrix;

  angle = angle / 180.0 * G_PI;
  picman_matrix3_identity (&matrix);
  picman_transform_matrix_rotate_center (&matrix, center_x, center_y, angle);

  picman_stroke_transform (stroke, &matrix);
}

void
picman_stroke_flip   (PicmanStroke          *stroke,
                    PicmanOrientationType  flip_type,
                    gdouble                axis)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->flip (stroke, flip_type, axis);
}

static void
picman_stroke_real_flip   (PicmanStroke          *stroke,
                         PicmanOrientationType  flip_type,
                         gdouble              axis)
{
  PicmanMatrix3  matrix;

  picman_matrix3_identity (&matrix);
  picman_transform_matrix_flip (&matrix, flip_type, axis);
  picman_stroke_transform (stroke, &matrix);
}

void
picman_stroke_flip_free   (PicmanStroke          *stroke,
                         gdouble              x1,
                         gdouble              y1,
                         gdouble              x2,
                         gdouble              y2)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->flip_free (stroke, x1, y1, x2, y2);
}

static void
picman_stroke_real_flip_free   (PicmanStroke          *stroke,
                              gdouble              x1,
                              gdouble              y1,
                              gdouble              x2,
                              gdouble              y2)
{
  /* x, y, width and height parameter in picman_transform_matrix_flip_free are unused */
  PicmanMatrix3  matrix;

  picman_matrix3_identity (&matrix);
  picman_transform_matrix_flip_free (&matrix, x1, y1, x2, y2);

  picman_stroke_transform (stroke, &matrix);
}

void
picman_stroke_transform (PicmanStroke        *stroke,
                       const PicmanMatrix3 *matrix)
{
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  PICMAN_STROKE_GET_CLASS (stroke)->transform (stroke, matrix);
}

static void
picman_stroke_real_transform (PicmanStroke        *stroke,
                            const PicmanMatrix3 *matrix)
{
  GList *list;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      picman_matrix3_transform_point (matrix,
                                    anchor->position.x,
                                    anchor->position.y,
                                    &anchor->position.x,
                                    &anchor->position.y);
    }
}


GList *
picman_stroke_get_draw_anchors (const PicmanStroke  *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->get_draw_anchors (stroke);
}

static GList *
picman_stroke_real_get_draw_anchors (const PicmanStroke  *stroke)
{
  GList *list;
  GList *ret_list = NULL;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      if (PICMAN_ANCHOR (list->data)->type == PICMAN_ANCHOR_ANCHOR)
        ret_list = g_list_prepend (ret_list, list->data);
    }

  return g_list_reverse (ret_list);
}


GList *
picman_stroke_get_draw_controls (const PicmanStroke  *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->get_draw_controls (stroke);
}

static GList *
picman_stroke_real_get_draw_controls (const PicmanStroke  *stroke)
{
  GList *list;
  GList *ret_list = NULL;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      if (anchor->type == PICMAN_ANCHOR_CONTROL)
        {
          PicmanAnchor *next = list->next ? list->next->data : NULL;
          PicmanAnchor *prev = list->prev ? list->prev->data : NULL;

          if (next && next->type == PICMAN_ANCHOR_ANCHOR && next->selected)
            {
              /* Ok, this is a hack.
               * The idea is to give control points at the end of a
               * stroke a higher priority for the interactive tool. */
              if (prev)
                ret_list = g_list_prepend (ret_list, anchor);
              else
                ret_list = g_list_append (ret_list, anchor);
            }
          else if (prev && prev->type == PICMAN_ANCHOR_ANCHOR && prev->selected)
            {
              /* same here... */
              if (next)
                ret_list = g_list_prepend (ret_list, anchor);
              else
                ret_list = g_list_append (ret_list, anchor);
            }
        }
    }

  return g_list_reverse (ret_list);
}


GArray *
picman_stroke_get_draw_lines (const PicmanStroke  *stroke)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->get_draw_lines (stroke);
}

static GArray *
picman_stroke_real_get_draw_lines (const PicmanStroke  *stroke)
{
  GList  *list;
  GArray *ret_lines = NULL;
  gint    count = 0;

  for (list = stroke->anchors; list; list = g_list_next (list))
    {
      PicmanAnchor *anchor = list->data;

      if (anchor->type == PICMAN_ANCHOR_ANCHOR && anchor->selected)
        {
          if (list->next)
            {
              PicmanAnchor *next = list->next->data;

              if (count == 0)
                ret_lines = g_array_new (FALSE, FALSE, sizeof (PicmanCoords));

              ret_lines = g_array_append_val (ret_lines, anchor->position);
              ret_lines = g_array_append_val (ret_lines, next->position);
              count += 1;
            }

          if (list->prev)
            {
              PicmanAnchor *prev = list->prev->data;

              if (count == 0)
                ret_lines = g_array_new (FALSE, FALSE, sizeof (PicmanCoords));

              ret_lines = g_array_append_val (ret_lines, anchor->position);
              ret_lines = g_array_append_val (ret_lines, prev->position);
              count += 1;
            }
        }
    }

  return ret_lines;
}

GArray *
picman_stroke_control_points_get (const PicmanStroke *stroke,
                                gboolean         *ret_closed)
{
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), NULL);

  return PICMAN_STROKE_GET_CLASS (stroke)->control_points_get (stroke,
                                                             ret_closed);
}

static GArray *
picman_stroke_real_control_points_get (const PicmanStroke *stroke,
                                     gboolean         *ret_closed)
{
  guint num_anchors;
  GArray *ret_array;
  GList *list;

  num_anchors = g_list_length (stroke->anchors);
  ret_array = g_array_sized_new (FALSE, FALSE,
                                 sizeof (PicmanAnchor), num_anchors);

  for (list = g_list_first (stroke->anchors); list; list = g_list_next (list))
    {
      g_array_append_vals (ret_array, list->data, 1);
    }

  if (ret_closed)
    *ret_closed = stroke->closed;

  return ret_array;
}

gboolean
picman_stroke_get_point_at_dist (const PicmanStroke *stroke,
                               const gdouble     dist,
                               const gdouble     precision,
                               PicmanCoords       *position,
                               gdouble          *slope)
{
   g_return_val_if_fail (PICMAN_IS_STROKE (stroke), FALSE);

   return PICMAN_STROKE_GET_CLASS (stroke)->get_point_at_dist (stroke,
                                                             dist,
                                                             precision,
                                                             position,
                                                             slope);
}


static gboolean
picman_stroke_real_get_point_at_dist (const PicmanStroke *stroke,
                                    const gdouble     dist,
                                    const gdouble     precision,
                                    PicmanCoords       *position,
                                    gdouble          *slope)
{
  GArray     *points;
  gint        i;
  gdouble     length;
  gdouble     segment_length;
  gboolean    ret = FALSE;
  PicmanCoords  difference;

  points = picman_stroke_interpolate (stroke, precision, NULL);
  if (points == NULL)
    return ret;

  length = 0;
  for (i=0; i < points->len - 1; i++)
    {
      picman_coords_difference (&(g_array_index (points, PicmanCoords , i)),
                              &(g_array_index (points, PicmanCoords , i+1)),
                              &difference);
      segment_length = picman_coords_length (&difference);

      if (segment_length == 0 || length + segment_length < dist )
        {
          length += segment_length;
        }
      else
        {
          /* x = x1 + (x2 - x1 ) u  */
          /* x   = x1 (1-u) + u x2  */

          gdouble u = (dist - length) / segment_length;

          picman_coords_mix (1 - u, &(g_array_index (points, PicmanCoords , i)),
                               u, &(g_array_index (points, PicmanCoords , i+1)),
                           position);

          if (difference.x == 0)
            *slope = G_MAXDOUBLE;
          else
            *slope = difference.y / difference.x;

          ret = TRUE;
          break;
        }
    }

  g_array_free (points, TRUE);

  return ret;
}
