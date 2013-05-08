/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbezierstroke.c
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

#include <glib-object.h>
#include <cairo.h>

#include "libpicmanmath/picmanmath.h"

#include "vectors-types.h"

#include "core/picmanbezierdesc.h"
#include "core/picmancoords.h"
#include "core/picmancoords-interpolate.h"

#include "picmananchor.h"
#include "picmanbezierstroke.h"


/*  local prototypes  */

static gdouble
    picman_bezier_stroke_nearest_point_get   (const PicmanStroke      *stroke,
                                            const PicmanCoords      *coord,
                                            const gdouble          precision,
                                            PicmanCoords            *ret_point,
                                            PicmanAnchor           **ret_segment_start,
                                            PicmanAnchor           **ret_segment_end,
                                            gdouble               *ret_pos);
static gdouble
    picman_bezier_stroke_segment_nearest_point_get
                                           (const PicmanCoords      *beziercoords,
                                            const PicmanCoords      *coord,
                                            const gdouble          precision,
                                            PicmanCoords            *ret_point,
                                            gdouble               *ret_pos,
                                            gint                   depth);
static gdouble
    picman_bezier_stroke_nearest_tangent_get (const PicmanStroke      *stroke,
                                            const PicmanCoords      *coord1,
                                            const PicmanCoords      *coord2,
                                            const gdouble          precision,
                                            PicmanCoords            *nearest,
                                            PicmanAnchor           **ret_segment_start,
                                            PicmanAnchor           **ret_segment_end,
                                            gdouble               *ret_pos);
static gdouble
    picman_bezier_stroke_segment_nearest_tangent_get
                                           (const PicmanCoords      *beziercoords,
                                            const PicmanCoords      *coord1,
                                            const PicmanCoords      *coord2,
                                            const gdouble          precision,
                                            PicmanCoords            *ret_point,
                                            gdouble               *ret_pos);
static void
    picman_bezier_stroke_anchor_move_relative
                                           (PicmanStroke            *stroke,
                                            PicmanAnchor            *anchor,
                                            const PicmanCoords      *deltacoord,
                                            PicmanAnchorFeatureType  feature);
static void
    picman_bezier_stroke_anchor_move_absolute
                                           (PicmanStroke            *stroke,
                                            PicmanAnchor            *anchor,
                                            const PicmanCoords      *coord,
                                            PicmanAnchorFeatureType  feature);
static void
    picman_bezier_stroke_anchor_convert      (PicmanStroke            *stroke,
                                            PicmanAnchor            *anchor,
                                            PicmanAnchorFeatureType  feature);
static void
    picman_bezier_stroke_anchor_delete       (PicmanStroke            *stroke,
                                            PicmanAnchor            *anchor);
static gboolean
    picman_bezier_stroke_point_is_movable    (PicmanStroke            *stroke,
                                            PicmanAnchor            *predec,
                                            gdouble                position);
static void
    picman_bezier_stroke_point_move_relative (PicmanStroke            *stroke,
                                            PicmanAnchor            *predec,
                                            gdouble                position,
                                            const PicmanCoords      *deltacoord,
                                            PicmanAnchorFeatureType  feature);
static void
    picman_bezier_stroke_point_move_absolute (PicmanStroke            *stroke,
                                            PicmanAnchor            *predec,
                                            gdouble                position,
                                            const PicmanCoords      *coord,
                                            PicmanAnchorFeatureType  feature);

static void picman_bezier_stroke_close       (PicmanStroke            *stroke);

static PicmanStroke *
    picman_bezier_stroke_open                (PicmanStroke            *stroke,
                                            PicmanAnchor            *end_anchor);
static gboolean
    picman_bezier_stroke_anchor_is_insertable
                                           (PicmanStroke            *stroke,
                                            PicmanAnchor            *predec,
                                            gdouble                position);
static PicmanAnchor *
    picman_bezier_stroke_anchor_insert       (PicmanStroke            *stroke,
                                            PicmanAnchor            *predec,
                                            gdouble                position);
static gboolean
    picman_bezier_stroke_is_extendable       (PicmanStroke            *stroke,
                                            PicmanAnchor            *neighbor);
static gboolean
    picman_bezier_stroke_connect_stroke      (PicmanStroke            *stroke,
                                            PicmanAnchor            *anchor,
                                            PicmanStroke            *extension,
                                            PicmanAnchor            *neighbor);
static GArray *
    picman_bezier_stroke_interpolate         (const PicmanStroke      *stroke,
                                            const gdouble          precision,
                                            gboolean              *closed);
static PicmanBezierDesc *
    picman_bezier_stroke_make_bezier         (const PicmanStroke      *stroke);

static void picman_bezier_stroke_finalize    (GObject               *object);


static GList * picman_bezier_stroke_get_anchor_listitem
                                           (GList                 *list);


G_DEFINE_TYPE (PicmanBezierStroke, picman_bezier_stroke, PICMAN_TYPE_STROKE)

#define parent_class picman_bezier_stroke_parent_class


static void
picman_bezier_stroke_class_init (PicmanBezierStrokeClass *klass)
{
  GObjectClass    *object_class = G_OBJECT_CLASS (klass);
  PicmanStrokeClass *stroke_class = PICMAN_STROKE_CLASS (klass);

  object_class->finalize             = picman_bezier_stroke_finalize;

  stroke_class->nearest_point_get    = picman_bezier_stroke_nearest_point_get;
  stroke_class->nearest_tangent_get  = picman_bezier_stroke_nearest_tangent_get;
  stroke_class->nearest_intersection_get = NULL;
  stroke_class->anchor_move_relative = picman_bezier_stroke_anchor_move_relative;
  stroke_class->anchor_move_absolute = picman_bezier_stroke_anchor_move_absolute;
  stroke_class->anchor_convert       = picman_bezier_stroke_anchor_convert;
  stroke_class->anchor_delete        = picman_bezier_stroke_anchor_delete;
  stroke_class->point_is_movable     = picman_bezier_stroke_point_is_movable;
  stroke_class->point_move_relative  = picman_bezier_stroke_point_move_relative;
  stroke_class->point_move_absolute  = picman_bezier_stroke_point_move_absolute;
  stroke_class->close                = picman_bezier_stroke_close;
  stroke_class->open                 = picman_bezier_stroke_open;
  stroke_class->anchor_is_insertable = picman_bezier_stroke_anchor_is_insertable;
  stroke_class->anchor_insert        = picman_bezier_stroke_anchor_insert;
  stroke_class->is_extendable        = picman_bezier_stroke_is_extendable;
  stroke_class->extend               = picman_bezier_stroke_extend;
  stroke_class->connect_stroke       = picman_bezier_stroke_connect_stroke;
  stroke_class->interpolate          = picman_bezier_stroke_interpolate;
  stroke_class->make_bezier          = picman_bezier_stroke_make_bezier;
}

static void
picman_bezier_stroke_init (PicmanBezierStroke *stroke)
{
}

static void
picman_bezier_stroke_finalize (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/* Bezier specific functions */

PicmanStroke *
picman_bezier_stroke_new (void)
{
  return g_object_new (PICMAN_TYPE_BEZIER_STROKE, NULL);
}


PicmanStroke *
picman_bezier_stroke_new_from_coords (const PicmanCoords *coords,
                                    gint              n_coords,
                                    gboolean          closed)
{
  PicmanStroke *stroke;
  PicmanAnchor *last_anchor;
  gint        count;

  g_return_val_if_fail (coords != NULL, NULL);
  g_return_val_if_fail (n_coords >= 3, NULL);
  g_return_val_if_fail ((n_coords % 3) == 0, NULL);

  stroke = picman_bezier_stroke_new ();

  last_anchor = NULL;

  for (count = 0; count < n_coords; count++)
    last_anchor = picman_bezier_stroke_extend (stroke,
                                             &coords[count],
                                             last_anchor,
                                             EXTEND_SIMPLE);

  if (closed)
    picman_stroke_close (stroke);

  return stroke;
}

static void
picman_bezier_stroke_anchor_delete (PicmanStroke *stroke,
                                  PicmanAnchor *anchor)
{
  GList *list;
  GList *list2;
  gint   i;

  /* Anchors always are surrounded by two handles that have to
   * be deleted too */

  list2 = g_list_find (stroke->anchors, anchor);
  list = g_list_previous(list2);

  for (i=0; i < 3; i++)
    {
      g_return_if_fail (list != NULL);
      list2 = g_list_next (list);
      picman_anchor_free (PICMAN_ANCHOR (list->data));
      stroke->anchors = g_list_delete_link (stroke->anchors, list);
      list = list2;
    }
}

static PicmanStroke *
picman_bezier_stroke_open (PicmanStroke *stroke,
                         PicmanAnchor *end_anchor)
{
  GList      *list;
  GList      *list2;
  PicmanStroke *new_stroke = NULL;

  list = g_list_find (stroke->anchors, end_anchor);

  g_return_val_if_fail (list != NULL && list->next != NULL, NULL);

  list = g_list_next (list);  /* protect the handle... */

  list2 = list->next;
  list->next = NULL;

  if (list2 != NULL)
    {
      list2->prev = NULL;

      if (stroke->closed)
        {
          stroke->anchors = g_list_concat (list2, stroke->anchors);
        }
      else
        {
          new_stroke = picman_bezier_stroke_new ();
          new_stroke->anchors = list2;
        }
    }

  stroke->closed = FALSE;
  g_object_notify (G_OBJECT (stroke), "closed");

  return new_stroke;
}

static gboolean
picman_bezier_stroke_anchor_is_insertable (PicmanStroke *stroke,
                                         PicmanAnchor *predec,
                                         gdouble     position)
{
  return (g_list_find (stroke->anchors, predec) != NULL);
}


static PicmanAnchor *
picman_bezier_stroke_anchor_insert (PicmanStroke *stroke,
                                  PicmanAnchor *predec,
                                  gdouble     position)
{
  GList      *segment_start;
  GList      *list;
  GList      *list2;
  PicmanCoords  subdivided[8];
  PicmanCoords  beziercoords[4];
  gint        i;

  segment_start = g_list_find (stroke->anchors, predec);

  if (! segment_start)
    return NULL;

  list = segment_start;

  for (i = 0; i <= 3; i++)
    {
      beziercoords[i] = PICMAN_ANCHOR (list->data)->position;
      list = g_list_next (list);
      if (!list)
        list = stroke->anchors;
    }

  subdivided[0] = beziercoords[0];
  subdivided[6] = beziercoords[3];

  picman_coords_mix (1-position, &(beziercoords[0]),
                   position,   &(beziercoords[1]),
                   &(subdivided[1]));

  picman_coords_mix (1-position, &(beziercoords[1]),
                   position,   &(beziercoords[2]),
                   &(subdivided[7]));

  picman_coords_mix (1-position, &(beziercoords[2]),
                   position,   &(beziercoords[3]),
                   &(subdivided[5]));

  picman_coords_mix (1-position, &(subdivided[1]),
                   position,   &(subdivided[7]),
                   &(subdivided[2]));

  picman_coords_mix (1-position, &(subdivided[7]),
                   position,   &(subdivided[5]),
                   &(subdivided[4]));

  picman_coords_mix (1-position, &(subdivided[2]),
                   position,   &(subdivided[4]),
                   &(subdivided[3]));

  /* subdivided 0-6 contains the bezier segement subdivided at <position> */

  list = segment_start;

  for (i = 0; i <= 6; i++)
    {
      if (i >= 2 && i <= 4)
        {
          list2 = g_list_append (NULL,
                                 picman_anchor_new ((i == 3 ?
                                                    PICMAN_ANCHOR_ANCHOR:
                                                    PICMAN_ANCHOR_CONTROL),
                                                  &(subdivided[i])));
          /* insert it *before* list manually. */
          list2->next = list;
          list2->prev = list->prev;
          if (list->prev)
            list->prev->next = list2;
          list->prev = list2;

          list = list2;

          if (i == 3)
            segment_start = list;

        }
      else
        {
          PICMAN_ANCHOR (list->data)->position = subdivided[i];
        }

      list = g_list_next (list);
      if (!list)
        list = stroke->anchors;

    }

  stroke->anchors = g_list_first (stroke->anchors);

  return PICMAN_ANCHOR (segment_start->data);
}


static gboolean
picman_bezier_stroke_point_is_movable (PicmanStroke *stroke,
                                     PicmanAnchor *predec,
                                     gdouble     position)
{
  return (g_list_find (stroke->anchors, predec) != NULL);
}


static void
picman_bezier_stroke_point_move_relative (PicmanStroke            *stroke,
                                        PicmanAnchor            *predec,
                                        gdouble                position,
                                        const PicmanCoords      *deltacoord,
                                        PicmanAnchorFeatureType  feature)
{
  PicmanCoords  offsetcoords[2];
  GList      *segment_start;
  GList      *list;
  gint        i;
  gdouble     feel_good;

  segment_start = g_list_find (stroke->anchors, predec);

  g_return_if_fail (segment_start != NULL);

  /* dragging close to endpoints just moves the handle related to
   * the endpoint. Just make sure that feel_good is in the range from
   * 0 to 1. The 1.0 / 6.0 and 5.0 / 6.0 are duplicated in
   * tools/picmanvectortool.c.  */
  if (position <= 1.0 / 6.0)
    feel_good = 0;
  else if (position <= 0.5)
    feel_good = (pow((6 * position - 1) / 2.0, 3)) / 2;
  else if (position <= 5.0 / 6.0)
    feel_good = (1 - pow((6 * (1-position) - 1) / 2.0, 3)) / 2 + 0.5;
  else
    feel_good = 1;

  picman_coords_scale ((1-feel_good)/(3*position*
                                    (1-position)*(1-position)),
                     deltacoord,
                     &(offsetcoords[0]));
  picman_coords_scale (feel_good/(3*position*position*(1-position)),
                     deltacoord,
                     &(offsetcoords[1]));

  list = segment_start;
  list = g_list_next (list);
  if (!list)
    list = stroke->anchors;

  for (i = 0; i <= 1; i++)
    {
      picman_stroke_anchor_move_relative (stroke, PICMAN_ANCHOR (list->data),
                                        &(offsetcoords[i]), feature);
      list = g_list_next (list);
      if (!list)
        list = stroke->anchors;
    }
}


static void
picman_bezier_stroke_point_move_absolute (PicmanStroke            *stroke,
                                        PicmanAnchor            *predec,
                                        gdouble                position,
                                        const PicmanCoords      *coord,
                                        PicmanAnchorFeatureType  feature)
{
  PicmanCoords  deltacoord;
  PicmanCoords  tmp1, tmp2, abs_pos;
  PicmanCoords  beziercoords[4];
  GList      *segment_start;
  GList      *list;
  gint        i;

  segment_start = g_list_find (stroke->anchors, predec);

  g_return_if_fail (segment_start != NULL);

  list = segment_start;

  for (i = 0; i <= 3; i++)
    {
      beziercoords[i] = PICMAN_ANCHOR (list->data)->position;
      list = g_list_next (list);
      if (!list)
        list = stroke->anchors;
    }

  picman_coords_mix ((1-position)*(1-position)*(1-position), &(beziercoords[0]),
                   3*(1-position)*(1-position)*position, &(beziercoords[1]),
                   &tmp1);
  picman_coords_mix (3*(1-position)*position*position, &(beziercoords[2]),
                   position*position*position, &(beziercoords[3]),
                   &tmp2);
  picman_coords_add (&tmp1, &tmp2, &abs_pos);

  picman_coords_difference (coord, &abs_pos, &deltacoord);

  picman_bezier_stroke_point_move_relative (stroke, predec, position,
                                          &deltacoord, feature);
}

static void
picman_bezier_stroke_close (PicmanStroke *stroke)
{
  GList      *start;
  GList      *end;
  PicmanAnchor *anchor;

  start = g_list_first (stroke->anchors);
  end = g_list_last (stroke->anchors);

  g_return_if_fail (start->next != NULL && end->prev != NULL);

  if (start->next != end->prev)
    {
      if (picman_coords_equal (&(PICMAN_ANCHOR (start->next->data)->position),
                             &(PICMAN_ANCHOR (start->data)->position)) &&
          picman_coords_equal (&(PICMAN_ANCHOR (start->data)->position),
                             &(PICMAN_ANCHOR (end->data)->position)) &&
          picman_coords_equal (&(PICMAN_ANCHOR (end->data)->position),
                             &(PICMAN_ANCHOR (end->prev->data)->position)))
        {
          /* redundant segment */

          anchor = PICMAN_ANCHOR (stroke->anchors->data);
          stroke->anchors = g_list_delete_link (stroke->anchors,
                                                stroke->anchors);
          picman_anchor_free (anchor);

          anchor = PICMAN_ANCHOR (stroke->anchors->data);
          stroke->anchors = g_list_delete_link (stroke->anchors,
                                                stroke->anchors);
          picman_anchor_free (anchor);

          anchor = PICMAN_ANCHOR (stroke->anchors->data);
          stroke->anchors = g_list_delete_link (stroke->anchors,
                                                stroke->anchors);

          end = g_list_last (stroke->anchors);
          picman_anchor_free (PICMAN_ANCHOR (end->data));
          end->data = anchor;
        }
    }

  PICMAN_STROKE_CLASS (parent_class)->close (stroke);
}

static gdouble
picman_bezier_stroke_nearest_point_get (const PicmanStroke     *stroke,
                                      const PicmanCoords     *coord,
                                      const gdouble         precision,
                                      PicmanCoords           *ret_point,
                                      PicmanAnchor          **ret_segment_start,
                                      PicmanAnchor          **ret_segment_end,
                                      gdouble              *ret_pos)
{
  gdouble     min_dist, dist, pos;
  PicmanCoords  point;
  PicmanCoords  segmentcoords[4];
  GList      *anchorlist;
  PicmanAnchor *segment_start;
  PicmanAnchor *segment_end = NULL;
  PicmanAnchor *anchor;
  gint        count;

  if (!stroke->anchors)
    return -1.0;

  count = 0;
  min_dist = -1;

  for (anchorlist = stroke->anchors;
       anchorlist && PICMAN_ANCHOR (anchorlist->data)->type != PICMAN_ANCHOR_ANCHOR;
       anchorlist = g_list_next (anchorlist));

  segment_start = anchorlist->data;

  for ( ; anchorlist; anchorlist = g_list_next (anchorlist))
    {
      anchor = anchorlist->data;

      segmentcoords[count] = anchor->position;
      count++;

      if (count == 4)
        {
          segment_end = anchorlist->data;
          dist = picman_bezier_stroke_segment_nearest_point_get (segmentcoords,
                                                               coord, precision,
                                                               &point, &pos,
                                                               10);

          if (dist < min_dist || min_dist < 0)
            {
              min_dist = dist;

              if (ret_pos)
                *ret_pos = pos;
              if (ret_point)
                *ret_point = point;
              if (ret_segment_start)
                *ret_segment_start = segment_start;
              if (ret_segment_end)
                *ret_segment_end = segment_end;
            }

          segment_start = anchorlist->data;
          segmentcoords[0] = segmentcoords[3];
          count = 1;
        }
    }

  if (stroke->closed && stroke->anchors)
    {
      anchorlist = stroke->anchors;

      while (count < 3)
        {
          segmentcoords[count] = PICMAN_ANCHOR (anchorlist->data)->position;
          count++;
        }

      anchorlist = g_list_next (anchorlist);

      if (anchorlist)
        {
          segment_end = PICMAN_ANCHOR (anchorlist->data);
          segmentcoords[3] = segment_end->position;
        }

      dist = picman_bezier_stroke_segment_nearest_point_get (segmentcoords,
                                                           coord, precision,
                                                           &point, &pos,
                                                           10);

      if (dist < min_dist || min_dist < 0)
        {
          min_dist = dist;

          if (ret_pos)
            *ret_pos = pos;
          if (ret_point)
            *ret_point = point;
          if (ret_segment_start)
            *ret_segment_start = segment_start;
          if (ret_segment_end)
            *ret_segment_end = segment_end;
        }
    }

  return min_dist;
}


static gdouble
picman_bezier_stroke_segment_nearest_point_get (const PicmanCoords  *beziercoords,
                                              const PicmanCoords  *coord,
                                              const gdouble      precision,
                                              PicmanCoords        *ret_point,
                                              gdouble           *ret_pos,
                                              gint               depth)
{
  /*
   * beziercoords has to contain four PicmanCoords with the four control points
   * of the bezier segment. We subdivide it at the parameter 0.5.
   */

  PicmanCoords subdivided[8];
  gdouble    dist1, dist2;
  PicmanCoords point1, point2;
  gdouble    pos1, pos2;

  picman_coords_difference (&beziercoords[1], &beziercoords[0], &point1);
  picman_coords_difference (&beziercoords[3], &beziercoords[2], &point2);

  if (!depth || (picman_coords_bezier_is_straight (beziercoords[0],
                                                 beziercoords[1],
                                                 beziercoords[2],
                                                 beziercoords[3],
                                                 precision)
                 && picman_coords_length_squared (&point1) < precision
                 && picman_coords_length_squared (&point2) < precision))
    {
      PicmanCoords line, dcoord;
      gdouble    length2, scalar;
      gint       i;

      picman_coords_difference (&(beziercoords[3]),
                              &(beziercoords[0]),
                              &line);

      picman_coords_difference (coord,
                              &(beziercoords[0]),
                              &dcoord);

      length2 = picman_coords_scalarprod (&line, &line);
      scalar = picman_coords_scalarprod (&line, &dcoord) / length2;

      scalar = CLAMP (scalar, 0.0, 1.0);

      /* lines look the same as bezier curves where the handles
       * sit on the anchors, however, they are parametrized
       * differently. Hence we have to do some weird approximation.  */

      pos1 = pos2 = 0.5;

      for (i = 0; i <= 15; i++)
        {
          pos2 *= 0.5;

          if (3 * pos1 * pos1 * (1-pos1) + pos1 * pos1 * pos1 < scalar)
            pos1 += pos2;
          else
            pos1 -= pos2;
        }

      *ret_pos = pos1;

      picman_coords_mix (1.0, &(beziercoords[0]),
                       scalar, &line,
                       ret_point);

      picman_coords_difference (coord, ret_point, &dcoord);

      return picman_coords_length (&dcoord);
    }

  /* ok, we have to subdivide */

  subdivided[0] = beziercoords[0];
  subdivided[6] = beziercoords[3];

  /* if (!depth) g_printerr ("Hit recursion depth limit!\n"); */

  picman_coords_average (&(beziercoords[0]), &(beziercoords[1]),
                       &(subdivided[1]));

  picman_coords_average (&(beziercoords[1]), &(beziercoords[2]),
                       &(subdivided[7]));

  picman_coords_average (&(beziercoords[2]), &(beziercoords[3]),
                       &(subdivided[5]));

  picman_coords_average (&(subdivided[1]), &(subdivided[7]),
                       &(subdivided[2]));

  picman_coords_average (&(subdivided[7]), &(subdivided[5]),
                       &(subdivided[4]));

  picman_coords_average (&(subdivided[2]), &(subdivided[4]),
                       &(subdivided[3]));

  /*
   * We now have the coordinates of the two bezier segments in
   * subdivided [0-3] and subdivided [3-6]
   */

  dist1 = picman_bezier_stroke_segment_nearest_point_get (&(subdivided[0]),
                                                        coord, precision,
                                                        &point1, &pos1,
                                                        depth - 1);

  dist2 = picman_bezier_stroke_segment_nearest_point_get (&(subdivided[3]),
                                                        coord, precision,
                                                        &point2, &pos2,
                                                        depth - 1);

  if (dist1 <= dist2)
    {
      *ret_point = point1;
      *ret_pos = 0.5 * pos1;
      return dist1;
    }
  else
    {
      *ret_point = point2;
      *ret_pos = 0.5 + 0.5 * pos2;
      return dist2;
    }
}


static gdouble
picman_bezier_stroke_nearest_tangent_get (const PicmanStroke  *stroke,
                                        const PicmanCoords  *coord1,
                                        const PicmanCoords  *coord2,
                                        const gdouble      precision,
                                        PicmanCoords        *nearest,
                                        PicmanAnchor       **ret_segment_start,
                                        PicmanAnchor       **ret_segment_end,
                                        gdouble           *ret_pos)
{
  gdouble     min_dist, dist, pos;
  PicmanCoords  point;
  PicmanCoords  segmentcoords[4];
  GList      *anchorlist;
  PicmanAnchor *segment_start;
  PicmanAnchor *segment_end = NULL;
  PicmanAnchor *anchor;
  gint        count;

  if (!stroke->anchors)
    return -1.0;

  count = 0;
  min_dist = -1;

  for (anchorlist = stroke->anchors;
       anchorlist && PICMAN_ANCHOR (anchorlist->data)->type != PICMAN_ANCHOR_ANCHOR;
       anchorlist = g_list_next (anchorlist));

  segment_start = anchorlist->data;

  for ( ; anchorlist; anchorlist = g_list_next (anchorlist))
    {
      anchor = anchorlist->data;

      segmentcoords[count] = anchor->position;
      count++;

      if (count == 4)
        {
          segment_end = anchorlist->data;
          dist = picman_bezier_stroke_segment_nearest_tangent_get (segmentcoords,
                                                                 coord1, coord2,
                                                                 precision,
                                                                 &point, &pos);

          if (dist >= 0 && (dist < min_dist || min_dist < 0))
            {
              min_dist = dist;

              if (ret_pos)
                *ret_pos = pos;
              if (nearest)
                *nearest = point;
              if (ret_segment_start)
                *ret_segment_start = segment_start;
              if (ret_segment_end)
                *ret_segment_end = segment_end;
            }

          segment_start = anchorlist->data;
          segmentcoords[0] = segmentcoords[3];
          count = 1;
        }
    }

  if (stroke->closed && stroke->anchors)
    {
      anchorlist = stroke->anchors;

      while (count < 3)
        {
          segmentcoords[count] = PICMAN_ANCHOR (anchorlist->data)->position;
          count++;
        }

      anchorlist = g_list_next (anchorlist);

      if (anchorlist)
        {
          segment_end = PICMAN_ANCHOR (anchorlist->data);
          segmentcoords[3] = segment_end->position;
        }

      dist = picman_bezier_stroke_segment_nearest_tangent_get (segmentcoords,
                                                             coord1, coord2,
                                                             precision,
                                                             &point, &pos);

      if (dist >= 0 && (dist < min_dist || min_dist < 0))
        {
          min_dist = dist;

          if (ret_pos)
            *ret_pos = pos;
          if (nearest)
            *nearest = point;
          if (ret_segment_start)
            *ret_segment_start = segment_start;
          if (ret_segment_end)
            *ret_segment_end = segment_end;
        }
    }

  return min_dist;
}

static gdouble
picman_bezier_stroke_segment_nearest_tangent_get (const PicmanCoords *beziercoords,
                                                const PicmanCoords *coord1,
                                                const PicmanCoords *coord2,
                                                const gdouble     precision,
                                                PicmanCoords       *ret_point,
                                                gdouble          *ret_pos)
{
  GArray     *ret_coords;
  GArray     *ret_params;
  PicmanCoords  dir, line, dcoord, min_point;
  gdouble     min_dist = -1;
  gdouble     dist, length2, scalar, ori, ori2;
  gint        i;

  picman_coords_difference (coord2, coord1, &line);

  ret_coords = g_array_new (FALSE, FALSE, sizeof (PicmanCoords));
  ret_params = g_array_new (FALSE, FALSE, sizeof (gdouble));

  g_printerr ("(%.2f, %.2f)-(%.2f,%.2f): ", coord1->x, coord1->y,
              coord2->x, coord2->y);

  picman_coords_interpolate_bezier (beziercoords[0],
                                  beziercoords[1],
                                  beziercoords[2],
                                  beziercoords[3],
                                  precision,
                                  &ret_coords, &ret_params);

  g_return_val_if_fail (ret_coords->len == ret_params->len, -1.0);

  if (ret_coords->len < 2)
    return -1;

  picman_coords_difference (&g_array_index (ret_coords, PicmanCoords, 1),
                          &g_array_index (ret_coords, PicmanCoords, 0),
                          &dir);
  ori = dir.x * line.y - dir.y * line.x;

  for (i = 2; i < ret_coords->len; i++)
    {
      picman_coords_difference (&g_array_index (ret_coords, PicmanCoords, i),
                              &g_array_index (ret_coords, PicmanCoords, i-1),
                              &dir);
      ori2 = dir.x * line.y - dir.y * line.x;

      if (ori * ori2 <= 0)
        {
#if 0
          if (ori2 == 0)
            /* Kandidat finden */;
          else
            /* ret_coords[i] ist der Kandidat */;
#endif

          picman_coords_difference (&g_array_index (ret_coords, PicmanCoords, i),
                                  coord1,
                                  &dcoord);

          length2 = picman_coords_scalarprod (&line, &line);
          scalar = picman_coords_scalarprod (&line, &dcoord) / length2;

          if (scalar >= 0 && scalar <= 1)
            {
              picman_coords_mix (1.0, coord1,
                               scalar, &line,
                               &min_point);
              picman_coords_difference (&min_point,
                                      &g_array_index (ret_coords, PicmanCoords, i),
                                      &dcoord);
              dist = picman_coords_length (&dcoord);

              if (dist < min_dist || min_dist < 0)
                {
                  min_dist   = dist;
                  *ret_point = g_array_index (ret_coords, PicmanCoords, i);
                  *ret_pos   = g_array_index (ret_params, gdouble, i);
                }
            }
        }
      ori = ori2;
    }

  if (min_dist < 0)
    g_printerr ("-\n");
  else
    g_printerr ("%f: (%.2f, %.2f) /%.3f/\n", min_dist,
                (*ret_point).x, (*ret_point).y, *ret_pos);

  g_array_free (ret_coords, TRUE);
  g_array_free (ret_params, TRUE);

  return min_dist;
}

static gboolean
picman_bezier_stroke_is_extendable (PicmanStroke *stroke,
                                  PicmanAnchor *neighbor)
{
  GList *listneighbor;
  gint   loose_end;

  if (stroke->closed)
    return FALSE;

  if (stroke->anchors == NULL)
    return TRUE;

  /* assure that there is a neighbor specified */
  g_return_val_if_fail (neighbor != NULL, FALSE);

  loose_end = 0;
  listneighbor = g_list_last (stroke->anchors);

  /* Check if the neighbor is at an end of the control points */
  if (listneighbor->data == neighbor)
    {
      loose_end = 1;
    }
  else
    {
      listneighbor = g_list_first (stroke->anchors);
      if (listneighbor->data == neighbor)
        {
          loose_end = -1;
        }
      else
        {
          /*
           * It isn't. If we are on a handle go to the nearest
           * anchor and see if we can find an end from it.
           * Yes, this is tedious.
           */

          listneighbor = g_list_find (stroke->anchors, neighbor);

          if (listneighbor && neighbor->type == PICMAN_ANCHOR_CONTROL)
            {
              if (listneighbor->prev &&
                  PICMAN_ANCHOR (listneighbor->prev->data)->type == PICMAN_ANCHOR_ANCHOR)
                {
                  listneighbor = listneighbor->prev;
                }
              else if (listneighbor->next &&
                       PICMAN_ANCHOR (listneighbor->next->data)->type == PICMAN_ANCHOR_ANCHOR)
                {
                  listneighbor = listneighbor->next;
                }
              else
                {
                  loose_end = 0;
                  listneighbor = NULL;
                }
            }

          if (listneighbor)
            /* we found a suitable ANCHOR_ANCHOR now, lets
             * search for its loose end.
             */
            {
              if (listneighbor->prev &&
                  listneighbor->prev->prev == NULL)
                {
                  loose_end = -1;
                }
              else if (listneighbor->next &&
                       listneighbor->next->next == NULL)
                {
                  loose_end = 1;
                }
            }
        }
    }

  return (loose_end != 0);
}

PicmanAnchor *
picman_bezier_stroke_extend (PicmanStroke           *stroke,
                           const PicmanCoords     *coords,
                           PicmanAnchor           *neighbor,
                           PicmanVectorExtendMode  extend_mode)
{
  PicmanAnchor *anchor = NULL;
  GList      *listneighbor;
  gint        loose_end, control_count;

  if (stroke->anchors == NULL)
    {
      /* assure that there is no neighbor specified */
      g_return_val_if_fail (neighbor == NULL, NULL);

      anchor = picman_anchor_new (PICMAN_ANCHOR_CONTROL, coords);

      stroke->anchors = g_list_append (stroke->anchors, anchor);

      switch (extend_mode)
        {
        case EXTEND_SIMPLE:
          break;

        case EXTEND_EDITABLE:
          anchor = picman_bezier_stroke_extend (stroke,
                                              coords, anchor,
                                              EXTEND_SIMPLE);

          /* we return the PICMAN_ANCHOR_ANCHOR */
          picman_bezier_stroke_extend (stroke,
                                     coords, anchor,
                                     EXTEND_SIMPLE);

          break;

        default:
          anchor = NULL;
        }

      return anchor;
    }
  else
    {
      /* assure that there is a neighbor specified */
      g_return_val_if_fail (neighbor != NULL, NULL);

      loose_end = 0;
      listneighbor = g_list_last (stroke->anchors);

      /* Check if the neighbor is at an end of the control points */
      if (listneighbor->data == neighbor)
        {
          loose_end = 1;
        }
      else
        {
          listneighbor = g_list_first (stroke->anchors);
          if (listneighbor->data == neighbor)
            {
              loose_end = -1;
            }
          else
            {
              /*
               * It isn't. If we are on a handle go to the nearest
               * anchor and see if we can find an end from it.
               * Yes, this is tedious.
               */

              listneighbor = g_list_find (stroke->anchors, neighbor);

              if (listneighbor && neighbor->type == PICMAN_ANCHOR_CONTROL)
                {
                  if (listneighbor->prev &&
                      PICMAN_ANCHOR (listneighbor->prev->data)->type == PICMAN_ANCHOR_ANCHOR)
                    {
                      listneighbor = listneighbor->prev;
                    }
                  else if (listneighbor->next &&
                           PICMAN_ANCHOR (listneighbor->next->data)->type == PICMAN_ANCHOR_ANCHOR)
                    {
                      listneighbor = listneighbor->next;
                    }
                  else
                    {
                      loose_end = 0;
                      listneighbor = NULL;
                    }
                }

              if (listneighbor)
                /* we found a suitable ANCHOR_ANCHOR now, lets
                 * search for its loose end.
                 */
                {
                  if (listneighbor->next &&
                           listneighbor->next->next == NULL)
                    {
                      loose_end = 1;
                      listneighbor = listneighbor->next;
                    }
                  else if (listneighbor->prev &&
                      listneighbor->prev->prev == NULL)
                    {
                      loose_end = -1;
                      listneighbor = listneighbor->prev;
                    }
                }
            }
        }

      if (loose_end)
        {
          PicmanAnchorType  type;

          /* We have to detect the type of the point to add... */

          control_count = 0;

          if (loose_end == 1)
            {
              while (listneighbor &&
                     PICMAN_ANCHOR (listneighbor->data)->type == PICMAN_ANCHOR_CONTROL)
                {
                  control_count++;
                  listneighbor = listneighbor->prev;
                }
            }
          else
            {
              while (listneighbor &&
                     PICMAN_ANCHOR (listneighbor->data)->type == PICMAN_ANCHOR_CONTROL)
                {
                  control_count++;
                  listneighbor = listneighbor->next;
                }
            }

          switch (extend_mode)
            {
            case EXTEND_SIMPLE:
              switch (control_count)
                {
                case 0:
                  type = PICMAN_ANCHOR_CONTROL;
                  break;
                case 1:
                  if (listneighbor)  /* only one handle in the path? */
                    type = PICMAN_ANCHOR_CONTROL;
                  else
                    type = PICMAN_ANCHOR_ANCHOR;
                  break;
                case 2:
                  type = PICMAN_ANCHOR_ANCHOR;
                  break;
                default:
                  g_warning ("inconsistent bezier curve: "
                             "%d successive control handles", control_count);
                  type = PICMAN_ANCHOR_ANCHOR;
                }

              anchor = picman_anchor_new (type, coords);

              if (loose_end == 1)
                stroke->anchors = g_list_append (stroke->anchors, anchor);

              if (loose_end == -1)
                stroke->anchors = g_list_prepend (stroke->anchors, anchor);
              break;

            case EXTEND_EDITABLE:
              switch (control_count)
                {
                case 0:
                  neighbor = picman_bezier_stroke_extend (stroke,
                                                        &(neighbor->position),
                                                        neighbor,
                                                        EXTEND_SIMPLE);
                case 1:
                  neighbor = picman_bezier_stroke_extend (stroke,
                                                        coords,
                                                        neighbor,
                                                        EXTEND_SIMPLE);
                case 2:
                  anchor = picman_bezier_stroke_extend (stroke,
                                                      coords,
                                                      neighbor,
                                                      EXTEND_SIMPLE);

                  neighbor = picman_bezier_stroke_extend (stroke,
                                                        coords,
                                                        anchor,
                                                        EXTEND_SIMPLE);
                  break;
                default:
                  g_warning ("inconsistent bezier curve: "
                             "%d successive control handles", control_count);
                }
            }

          return anchor;
        }
      return NULL;
    }
}

static gboolean
picman_bezier_stroke_connect_stroke (PicmanStroke *stroke,
                                   PicmanAnchor *anchor,
                                   PicmanStroke *extension,
                                   PicmanAnchor *neighbor)
{
  GList *list1;
  GList *list2;

  list1 = g_list_find (stroke->anchors, anchor);
  list1 = picman_bezier_stroke_get_anchor_listitem (list1);
  list2 = g_list_find (extension->anchors, neighbor);
  list2 = picman_bezier_stroke_get_anchor_listitem (list2);

  g_return_val_if_fail (list1 != NULL && list2 != NULL, FALSE);

  if (stroke == extension)
    {
      g_return_val_if_fail ((list1->prev && list1->prev->prev == NULL &&
                             list2->next && list2->next->next == NULL) ||
                            (list1->next && list1->next->next == NULL &&
                             list2->prev && list2->prev->prev == NULL), FALSE);
      picman_stroke_close (stroke);
      return TRUE;
    }

  if (list1->prev && list1->prev->prev == NULL)
    {
      stroke->anchors = g_list_reverse (stroke->anchors);
    }

  g_return_val_if_fail (list1->next && list1->next->next == NULL, FALSE);

  if (list2->next && list2->next->next == NULL)
    {
      extension->anchors = g_list_reverse (extension->anchors);
    }

  g_return_val_if_fail (list2->prev && list2->prev->prev == NULL, FALSE);

  stroke->anchors = g_list_concat (stroke->anchors, extension->anchors);
  extension->anchors = NULL;

  return TRUE;
}


static void
picman_bezier_stroke_anchor_move_relative (PicmanStroke            *stroke,
                                         PicmanAnchor            *anchor,
                                         const PicmanCoords      *deltacoord,
                                         PicmanAnchorFeatureType  feature)
{
  PicmanCoords  delta, coord1, coord2;
  GList      *anchor_list;

  delta = *deltacoord;
  delta.pressure = 0;
  delta.xtilt    = 0;
  delta.ytilt    = 0;
  delta.wheel    = 0;

  picman_coords_add (&(anchor->position), &delta, &coord1);
  anchor->position = coord1;

  anchor_list = g_list_find (stroke->anchors, anchor);
  g_return_if_fail (anchor_list != NULL);

  if (anchor->type == PICMAN_ANCHOR_ANCHOR)
    {
      if (g_list_previous (anchor_list))
        {
          coord2 = PICMAN_ANCHOR (g_list_previous (anchor_list)->data)->position;
          picman_coords_add (&coord2, &delta, &coord1);
          PICMAN_ANCHOR (g_list_previous (anchor_list)->data)->position = coord1;
        }

      if (g_list_next (anchor_list))
        {
          coord2 = PICMAN_ANCHOR (g_list_next (anchor_list)->data)->position;
          picman_coords_add (&coord2, &delta, &coord1);
          PICMAN_ANCHOR (g_list_next (anchor_list)->data)->position = coord1;
        }
    }
  else
    {
      if (feature == PICMAN_ANCHOR_FEATURE_SYMMETRIC)
        {
          GList *neighbour = NULL, *opposite = NULL;

          /* search for opposite control point. Sigh. */
          neighbour = g_list_previous (anchor_list);
          if (neighbour &&
              PICMAN_ANCHOR (neighbour->data)->type == PICMAN_ANCHOR_ANCHOR)
            {
              opposite = g_list_previous (neighbour);
            }
          else
            {
              neighbour = g_list_next (anchor_list);
              if (neighbour &&
                  PICMAN_ANCHOR (neighbour->data)->type == PICMAN_ANCHOR_ANCHOR)
                {
                  opposite = g_list_next (neighbour);
                }
            }
          if (opposite &&
              PICMAN_ANCHOR (opposite->data)->type == PICMAN_ANCHOR_CONTROL)
            {
              picman_coords_difference (&(PICMAN_ANCHOR (neighbour->data)->position),
                                      &(anchor->position), &delta);
              picman_coords_add (&(PICMAN_ANCHOR (neighbour->data)->position),
                               &delta, &coord1);
              PICMAN_ANCHOR (opposite->data)->position = coord1;
            }
        }
    }
}


static void
picman_bezier_stroke_anchor_move_absolute (PicmanStroke            *stroke,
                                         PicmanAnchor            *anchor,
                                         const PicmanCoords      *coord,
                                         PicmanAnchorFeatureType  feature)
{
  PicmanCoords deltacoord;

  picman_coords_difference (coord, &anchor->position, &deltacoord);
  picman_bezier_stroke_anchor_move_relative (stroke, anchor,
                                           &deltacoord, feature);
}

static void
picman_bezier_stroke_anchor_convert (PicmanStroke            *stroke,
                                   PicmanAnchor            *anchor,
                                   PicmanAnchorFeatureType  feature)
{
  GList *anchor_list;

  anchor_list = g_list_find (stroke->anchors, anchor);

  g_return_if_fail (anchor_list != NULL);

  switch (feature)
    {
    case PICMAN_ANCHOR_FEATURE_EDGE:
      if (anchor->type == PICMAN_ANCHOR_ANCHOR)
        {
          if (g_list_previous (anchor_list))
            PICMAN_ANCHOR (g_list_previous (anchor_list)->data)->position =
              anchor->position;

          if (g_list_next (anchor_list))
            PICMAN_ANCHOR (g_list_next (anchor_list)->data)->position =
              anchor->position;
        }
      else
        {
          if (g_list_previous (anchor_list) &&
              PICMAN_ANCHOR (g_list_previous (anchor_list)->data)->type == PICMAN_ANCHOR_ANCHOR)
            anchor->position = PICMAN_ANCHOR (g_list_previous (anchor_list)->data)->position;
          if (g_list_next (anchor_list) &&
              PICMAN_ANCHOR (g_list_next (anchor_list)->data)->type == PICMAN_ANCHOR_ANCHOR)
            anchor->position = PICMAN_ANCHOR (g_list_next (anchor_list)->data)->position;
        }

      break;

    default:
      g_warning ("picman_bezier_stroke_anchor_convert: "
                 "unimplemented anchor conversion %d\n", feature);
    }
}


static PicmanBezierDesc *
picman_bezier_stroke_make_bezier (const PicmanStroke *stroke)
{
  GArray            *points;
  GArray            *cmd_array;
  PicmanBezierDesc    *bezdesc;
  cairo_path_data_t  pathdata;
  gint               num_cmds, i;

  points = picman_stroke_control_points_get (stroke, NULL);

  g_return_val_if_fail (points && points->len % 3 == 0, NULL);
  if (points->len < 3)
    return NULL;

  /* Moveto + (n-1) * curveto + (if closed) curveto + closepath */
  num_cmds = 2 + (points->len / 3 - 1) * 4;
  if (stroke->closed)
    num_cmds += 1 + 4;

  cmd_array = g_array_sized_new (FALSE, FALSE,
                                 sizeof (cairo_path_data_t),
                                 num_cmds);

  pathdata.header.type = CAIRO_PATH_MOVE_TO;
  pathdata.header.length = 2;
  g_array_append_val (cmd_array, pathdata);
  pathdata.point.x = g_array_index (points, PicmanAnchor, 1).position.x;
  pathdata.point.y = g_array_index (points, PicmanAnchor, 1).position.y;
  g_array_append_val (cmd_array, pathdata);

  for (i = 2; i+2 < points->len; i += 3)
    {
      pathdata.header.type = CAIRO_PATH_CURVE_TO;
      pathdata.header.length = 4;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, i).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, i).position.y;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, i+1).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, i+1).position.y;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, i+2).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, i+2).position.y;
      g_array_append_val (cmd_array, pathdata);
    }

  if (stroke->closed)
    {
      pathdata.header.type = CAIRO_PATH_CURVE_TO;
      pathdata.header.length = 4;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, i).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, i).position.y;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, 0).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, 0).position.y;
      g_array_append_val (cmd_array, pathdata);

      pathdata.point.x = g_array_index (points, PicmanAnchor, 1).position.x;
      pathdata.point.y = g_array_index (points, PicmanAnchor, 1).position.y;
      g_array_append_val (cmd_array, pathdata);

      pathdata.header.type = CAIRO_PATH_CLOSE_PATH;
      pathdata.header.length = 1;
      g_array_append_val (cmd_array, pathdata);
    }

  if (cmd_array->len != num_cmds)
    g_printerr ("miscalculated path cmd length! (%d vs. %d)\n",
                cmd_array->len, num_cmds);

  bezdesc = picman_bezier_desc_new ((cairo_path_data_t *) cmd_array->data,
                                  cmd_array->len);
  g_array_free (points, TRUE);
  g_array_free (cmd_array, FALSE);

  return bezdesc;
}


static GArray *
picman_bezier_stroke_interpolate (const PicmanStroke  *stroke,
                                gdouble            precision,
                                gboolean          *ret_closed)
{
  GArray     *ret_coords;
  PicmanAnchor *anchor;
  GList      *anchorlist;
  PicmanCoords  segmentcoords[4];
  gint        count;
  gboolean    need_endpoint = FALSE;

  if (!stroke->anchors)
    {
      if (ret_closed)
        *ret_closed = FALSE;
      return NULL;
    }

  ret_coords = g_array_new (FALSE, FALSE, sizeof (PicmanCoords));

  count = 0;

  for (anchorlist = stroke->anchors;
       anchorlist && PICMAN_ANCHOR (anchorlist->data)->type != PICMAN_ANCHOR_ANCHOR;
       anchorlist = g_list_next (anchorlist));

  for ( ; anchorlist; anchorlist = g_list_next (anchorlist))
    {
      anchor = anchorlist->data;

      segmentcoords[count] = anchor->position;
      count++;

      if (count == 4)
        {
          picman_coords_interpolate_bezier (segmentcoords[0],
                                          segmentcoords[1],
                                          segmentcoords[2],
                                          segmentcoords[3],
                                          precision,
                                          &ret_coords, NULL);
          segmentcoords[0] = segmentcoords[3];
          count = 1;
          need_endpoint = TRUE;
        }
    }

  if (stroke->closed && stroke->anchors)
    {
      anchorlist = stroke->anchors;

      while (count < 3)
        {
          segmentcoords[count] = PICMAN_ANCHOR (anchorlist->data)->position;
          count++;
        }
      anchorlist = g_list_next (anchorlist);
      if (anchorlist)
        segmentcoords[3] = PICMAN_ANCHOR (anchorlist->data)->position;

      picman_coords_interpolate_bezier (segmentcoords[0],
                                      segmentcoords[1],
                                      segmentcoords[2],
                                      segmentcoords[3],
                                      precision,
                                      &ret_coords, NULL);
      need_endpoint = TRUE;

    }

  if (need_endpoint)
    ret_coords = g_array_append_val (ret_coords, segmentcoords[3]);

  if (ret_closed)
    *ret_closed = stroke->closed;

  if (ret_coords->len == 0)
    {
      g_array_free (ret_coords, TRUE);
      ret_coords = NULL;
    }

  return ret_coords;
}

PicmanStroke *
picman_bezier_stroke_new_moveto (const PicmanCoords *start)
{
  PicmanStroke *stroke;

  stroke = picman_bezier_stroke_new ();

  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     start));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_ANCHOR,
                                                     start));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     start));
  return stroke;
}

void
picman_bezier_stroke_lineto (PicmanStroke       *stroke,
                           const PicmanCoords *end)
{
  g_return_if_fail (PICMAN_IS_BEZIER_STROKE (stroke));
  g_return_if_fail (stroke->closed == FALSE);
  g_return_if_fail (stroke->anchors != NULL);

  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     end));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_ANCHOR,
                                                     end));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     end));
}

void
picman_bezier_stroke_conicto (PicmanStroke       *stroke,
                            const PicmanCoords *control,
                            const PicmanCoords *end)
{
  PicmanCoords start, coords;

  g_return_if_fail (PICMAN_IS_BEZIER_STROKE (stroke));
  g_return_if_fail (stroke->closed == FALSE);
  g_return_if_fail (stroke->anchors != NULL);
  g_return_if_fail (stroke->anchors->next != NULL);

  start = PICMAN_ANCHOR (stroke->anchors->next->data)->position;

  picman_coords_mix (2.0 / 3.0, control, 1.0 / 3.0, &start, &coords);

  PICMAN_ANCHOR (stroke->anchors->data)->position = coords;

  picman_coords_mix (2.0 / 3.0, control, 1.0 / 3.0, end, &coords);

  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     &coords));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_ANCHOR,
                                                     end));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     end));
}

void
picman_bezier_stroke_cubicto (PicmanStroke       *stroke,
                            const PicmanCoords *control1,
                            const PicmanCoords *control2,
                            const PicmanCoords *end)
{
  g_return_if_fail (PICMAN_IS_BEZIER_STROKE (stroke));
  g_return_if_fail (stroke->closed == FALSE);
  g_return_if_fail (stroke->anchors != NULL);

  PICMAN_ANCHOR (stroke->anchors->data)->position = *control1;

  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     control2));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_ANCHOR,
                                                     end));
  stroke->anchors = g_list_prepend (stroke->anchors,
                                    picman_anchor_new (PICMAN_ANCHOR_CONTROL,
                                                     end));
}

static gdouble
arcto_circleparam (gdouble  h,
                   gdouble *y)
{
  gdouble t0 = 0.5;
  gdouble dt = 0.25;
  gdouble pt0;
  gdouble y01, y12, y23, y012, y123, y0123;   /* subdividing y[] */

  while (dt >= 0.00001)
    {
      pt0 = (    y[0] * (1-t0) * (1-t0) * (1-t0) +
             3 * y[1] * (1-t0) * (1-t0) * t0     +
             3 * y[2] * (1-t0) * t0     * t0     +
                 y[3] * t0     * t0     * t0 );

      if (pt0 > h)
        t0 = t0 - dt;
      else if (pt0 < h)
        t0 = t0 + dt;
      else
        break;
      dt = dt/2;
    }

  y01   = y[0] * (1-t0) + y[1] * t0;
  y12   = y[1] * (1-t0) + y[2] * t0;
  y23   = y[2] * (1-t0) + y[3] * t0;
  y012  = y01  * (1-t0) + y12  * t0;
  y123  = y12  * (1-t0) + y23  * t0;
  y0123 = y012 * (1-t0) + y123 * t0;

  y[0] = y0123; y[1] = y123; y[2] = y23; /* y[3] unchanged */

  return t0;
}

static void
arcto_subdivide (gdouble     t,
                 gint        part,
                 PicmanCoords *p)
{
  PicmanCoords p01, p12, p23, p012, p123, p0123;

  picman_coords_mix (1-t, &(p[0]), t, &(p[1]), &p01  );
  picman_coords_mix (1-t, &(p[1]), t, &(p[2]), &p12  );
  picman_coords_mix (1-t, &(p[2]), t, &(p[3]), &p23  );
  picman_coords_mix (1-t,  &p01  , t,  &p12  , &p012 );
  picman_coords_mix (1-t,  &p12  , t,  &p23  , &p123 );
  picman_coords_mix (1-t,  &p012 , t,  &p123 , &p0123);

  if (part == 0)
    {
      /* p[0] unchanged */
      p[1] = p01;
      p[2] = p012;
      p[3] = p0123;
    }
  else
    {
      p[0] = p0123;
      p[1] = p123;
      p[2] = p23;
      /* p[3] unchanged */
    }
}

static void
arcto_ellipsesegment (gdouble     radius_x,
                      gdouble     radius_y,
                      gdouble     phi0,
                      gdouble     phi1,
                      PicmanCoords *ellips)
{
  const PicmanCoords  template    = PICMAN_COORDS_DEFAULT_VALUES;
  const gdouble     circlemagic = 4.0 * (G_SQRT2 - 1.0) / 3.0;

  gdouble  phi_s, phi_e;
  gdouble  y[4];
  gdouble  h0, h1;
  gdouble  t0, t1;

  g_return_if_fail (ellips != NULL);

  y[0] = 0.0;
  y[1] = circlemagic;
  y[2] = 1.0;
  y[3] = 1.0;

  ellips[0] = template;
  ellips[1] = template;
  ellips[2] = template;
  ellips[3] = template;

  if (phi0 < phi1)
    {
      phi_s = floor (phi0 / G_PI_2) * G_PI_2;
      while (phi_s < 0) phi_s += 2 * G_PI;
      phi_e = phi_s + G_PI_2;
    }
  else
    {
      phi_e = floor (phi1 / G_PI_2) * G_PI_2;
      while (phi_e < 0) phi_e += 2 * G_PI;
      phi_s = phi_e + G_PI_2;
    }

  h0 = sin (fabs (phi0-phi_s));
  h1 = sin (fabs (phi1-phi_s));

  ellips[0].x = cos (phi_s); ellips[0].y = sin (phi_s);
  ellips[3].x = cos (phi_e); ellips[3].y = sin (phi_e);

  picman_coords_mix (1, &(ellips[0]), circlemagic, &(ellips[3]), &(ellips[1]));
  picman_coords_mix (circlemagic, &(ellips[0]), 1, &(ellips[3]), &(ellips[2]));

  if (h0 > y[0])
    {
      t0 = arcto_circleparam (h0, y); /* also subdivides y[] at t0 */
      arcto_subdivide (t0, 1, ellips);
    }

  if (h1 < y[3])
    {
      t1 = arcto_circleparam (h1, y);
      arcto_subdivide (t1, 0, ellips);
    }

  ellips[0].x *= radius_x ; ellips[0].y *= radius_y;
  ellips[1].x *= radius_x ; ellips[1].y *= radius_y;
  ellips[2].x *= radius_x ; ellips[2].y *= radius_y;
  ellips[3].x *= radius_x ; ellips[3].y *= radius_y;
}

void
picman_bezier_stroke_arcto (PicmanStroke       *bez_stroke,
                          gdouble           radius_x,
                          gdouble           radius_y,
                          gdouble           angle_rad,
                          gboolean          large_arc,
                          gboolean          sweep,
                          const PicmanCoords *end)
{
  PicmanCoords  start;
  PicmanCoords  middle;   /* between start and end */
  PicmanCoords  trans_delta;
  PicmanCoords  trans_center;
  PicmanCoords  tmp_center;
  PicmanCoords  center;
  PicmanCoords  ellips[4]; /* control points of untransformed ellipse segment */
  PicmanCoords  ctrl[4];   /* control points of next bezier segment           */

  PicmanMatrix3 anglerot;

  gdouble     lambda;
  gdouble     phi0, phi1, phi2;
  gdouble     tmpx, tmpy;

  g_return_if_fail (PICMAN_IS_BEZIER_STROKE (bez_stroke));
  g_return_if_fail (bez_stroke->closed == FALSE);
  g_return_if_fail (bez_stroke->anchors != NULL);
  g_return_if_fail (bez_stroke->anchors->next != NULL);

  if (radius_x == 0 || radius_y == 0)
    {
      picman_bezier_stroke_lineto (bez_stroke, end);
      return;
    }

  start = PICMAN_ANCHOR (bez_stroke->anchors->next->data)->position;

  picman_matrix3_identity (&anglerot);
  picman_matrix3_rotate (&anglerot, -angle_rad);

  picman_coords_mix (0.5, &start, -0.5, end, &trans_delta);
  picman_matrix3_transform_point (&anglerot,
                                trans_delta.x, trans_delta.y,
                                &tmpx, &tmpy);
  trans_delta.x = tmpx;
  trans_delta.y = tmpy;

  lambda = (SQR (trans_delta.x) / SQR (radius_x) +
            SQR (trans_delta.y) / SQR (radius_y));

  if (lambda < 0.00001)
    {
      /* dont bother with it - endpoint is too close to startpoint */
      return;
    }

  trans_center = trans_delta;

  if (lambda > 1.0)
    {
      /* The radii are too small for a matching ellipse. We expand them
       * so that they fit exactly (center of the ellipse between the
       * start- and endpoint
       */
      radius_x *= sqrt (lambda);
      radius_y *= sqrt (lambda);
      trans_center.x = 0.0;
      trans_center.y = 0.0;
    }
  else
    {
      gdouble factor = sqrt ((1.0 - lambda) / lambda);

      trans_center.x =   trans_delta.y * radius_x / radius_y * factor;
      trans_center.y = - trans_delta.x * radius_y / radius_x * factor;
    }

  if ((large_arc && sweep) || (!large_arc && !sweep))
    {
      trans_center.x *= -1;
      trans_center.y *= -1;
    }

  picman_matrix3_identity (&anglerot);
  picman_matrix3_rotate (&anglerot, angle_rad);

  tmp_center = trans_center;
  picman_matrix3_transform_point (&anglerot,
                                tmp_center.x, tmp_center.y,
                                &tmpx, &tmpy);
  tmp_center.x = tmpx;
  tmp_center.y = tmpy;

  picman_coords_mix (0.5, &start, 0.5, end, &middle);
  picman_coords_add (&tmp_center, &middle, &center);

  phi1 = atan2 ((trans_delta.y - trans_center.y) / radius_y,
                (trans_delta.x - trans_center.x) / radius_x);

  phi2 = atan2 ((- trans_delta.y - trans_center.y) / radius_y,
                (- trans_delta.x - trans_center.x) / radius_x);

  if (phi1 < 0)
    phi1 += 2 * G_PI;

  if (phi2 < 0)
    phi2 += 2 * G_PI;

  if (sweep)
    {
      while (phi2 < phi1)
        phi2 += 2 * G_PI;

      phi0 = floor (phi1 / G_PI_2) * G_PI_2;

      while (phi0 < phi2)
        {
          arcto_ellipsesegment (radius_x, radius_y,
                                MAX (phi0, phi1), MIN (phi0 + G_PI_2, phi2),
                                ellips);

          picman_matrix3_transform_point (&anglerot, ellips[0].x, ellips[0].y,
                                        &tmpx, &tmpy);
          ellips[0].x = tmpx; ellips[0].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[1].x, ellips[1].y,
                                        &tmpx, &tmpy);
          ellips[1].x = tmpx; ellips[1].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[2].x, ellips[2].y,
                                        &tmpx, &tmpy);
          ellips[2].x = tmpx; ellips[2].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[3].x, ellips[3].y,
                                        &tmpx, &tmpy);
          ellips[3].x = tmpx; ellips[3].y = tmpy;

          picman_coords_add (&center, &(ellips[1]), &(ctrl[1]));
          picman_coords_add (&center, &(ellips[2]), &(ctrl[2]));
          picman_coords_add (&center, &(ellips[3]), &(ctrl[3]));

          picman_bezier_stroke_cubicto (bez_stroke,
                                      &(ctrl[1]), &(ctrl[2]), &(ctrl[3]));
          phi0 += G_PI_2;
        }
    }
  else
    {
      while (phi1 < phi2)
        phi1 += 2 * G_PI;

      phi0 = ceil (phi1 / G_PI_2) * G_PI_2;

      while (phi0 > phi2)
        {
          arcto_ellipsesegment (radius_x, radius_y,
                                MIN (phi0, phi1), MAX (phi0 - G_PI_2, phi2),
                                ellips);

          picman_matrix3_transform_point (&anglerot, ellips[0].x, ellips[0].y,
                                        &tmpx, &tmpy);
          ellips[0].x = tmpx; ellips[0].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[1].x, ellips[1].y,
                                        &tmpx, &tmpy);
          ellips[1].x = tmpx; ellips[1].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[2].x, ellips[2].y,
                                        &tmpx, &tmpy);
          ellips[2].x = tmpx; ellips[2].y = tmpy;
          picman_matrix3_transform_point (&anglerot, ellips[3].x, ellips[3].y,
                                        &tmpx, &tmpy);
          ellips[3].x = tmpx; ellips[3].y = tmpy;

          picman_coords_add (&center, &(ellips[1]), &(ctrl[1]));
          picman_coords_add (&center, &(ellips[2]), &(ctrl[2]));
          picman_coords_add (&center, &(ellips[3]), &(ctrl[3]));

          picman_bezier_stroke_cubicto (bez_stroke,
                                      &(ctrl[1]), &(ctrl[2]), &(ctrl[3]));
          phi0 -= G_PI_2;
        }
    }
}

PicmanStroke *
picman_bezier_stroke_new_ellipse (const PicmanCoords *center,
                                gdouble           radius_x,
                                gdouble           radius_y,
                                gdouble           angle)
{
  PicmanStroke    *stroke;
  PicmanCoords     p1 = *center;
  PicmanCoords     p2 = *center;
  PicmanCoords     p3 = *center;
  PicmanCoords     dx = { 0, };
  PicmanCoords     dy = { 0, };
  const gdouble  circlemagic = 4.0 * (G_SQRT2 - 1.0) / 3.0;
  PicmanAnchor    *handle;

  dx.x =   radius_x * cos (angle);
  dx.y = - radius_x * sin (angle);
  dy.x =   radius_y * sin (angle);
  dy.y =   radius_y * cos (angle);

  picman_coords_mix (1.0, center, 1.0, &dx, &p1);
  stroke = picman_bezier_stroke_new_moveto (&p1);

  handle = g_list_last (PICMAN_STROKE (stroke)->anchors)->data;
  picman_coords_mix (1.0,    &p1, -circlemagic, &dy, &handle->position);

  picman_coords_mix (1.0,    &p1,  circlemagic, &dy, &p1);
  picman_coords_mix (1.0, center,          1.0, &dy, &p3);
  picman_coords_mix (1.0,    &p3,  circlemagic, &dx, &p2);
  picman_bezier_stroke_cubicto (stroke, &p1, &p2, &p3);

  picman_coords_mix (1.0,    &p3, -circlemagic, &dx, &p1);
  picman_coords_mix (1.0, center,         -1.0, &dx, &p3);
  picman_coords_mix (1.0,    &p3,  circlemagic, &dy, &p2);
  picman_bezier_stroke_cubicto (stroke, &p1, &p2, &p3);

  picman_coords_mix (1.0,    &p3, -circlemagic, &dy, &p1);
  picman_coords_mix (1.0, center,         -1.0, &dy, &p3);
  picman_coords_mix (1.0,    &p3, -circlemagic, &dx, &p2);
  picman_bezier_stroke_cubicto (stroke, &p1, &p2, &p3);

  handle = g_list_first (PICMAN_STROKE (stroke)->anchors)->data;
  picman_coords_mix (1.0,    &p3,  circlemagic, &dx, &handle->position);

  picman_stroke_close (stroke);

  return stroke;
}


/* helper function to get the associated anchor of a listitem */

static GList *
picman_bezier_stroke_get_anchor_listitem (GList *list)
{
  if (!list)
    return NULL;

  if (PICMAN_ANCHOR (list->data)->type == PICMAN_ANCHOR_ANCHOR)
    return list;

  if (list->prev && PICMAN_ANCHOR (list->prev->data)->type == PICMAN_ANCHOR_ANCHOR)
    return list->prev;

  if (list->next && PICMAN_ANCHOR (list->next->data)->type == PICMAN_ANCHOR_ANCHOR)
    return list->next;

  g_return_val_if_fail (/* bezier stroke inconsistent! */ FALSE, NULL);

  return NULL;
}
