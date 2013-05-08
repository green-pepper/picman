/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvaspolygon.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "core/picmanparamspecs.h"

#include "picmancanvaspolygon.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_POINTS,
  PROP_FILLED
};


typedef struct _PicmanCanvasPolygonPrivate PicmanCanvasPolygonPrivate;

struct _PicmanCanvasPolygonPrivate
{
  PicmanVector2 *points;
  gint         n_points;
  gboolean     filled;
};

#define GET_PRIVATE(polygon) \
        G_TYPE_INSTANCE_GET_PRIVATE (polygon, \
                                     PICMAN_TYPE_CANVAS_POLYGON, \
                                     PicmanCanvasPolygonPrivate)


/*  local function prototypes  */

static void             picman_canvas_polygon_finalize     (GObject        *object);
static void             picman_canvas_polygon_set_property (GObject        *object,
                                                          guint           property_id,
                                                          const GValue   *value,
                                                          GParamSpec     *pspec);
static void             picman_canvas_polygon_get_property (GObject        *object,
                                                          guint           property_id,
                                                          GValue         *value,
                                                          GParamSpec     *pspec);
static void             picman_canvas_polygon_draw         (PicmanCanvasItem *item,
                                                          cairo_t        *cr);
static cairo_region_t * picman_canvas_polygon_get_extents  (PicmanCanvasItem *item);


G_DEFINE_TYPE (PicmanCanvasPolygon, picman_canvas_polygon,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_polygon_parent_class


static void
picman_canvas_polygon_class_init (PicmanCanvasPolygonClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = picman_canvas_polygon_finalize;
  object_class->set_property = picman_canvas_polygon_set_property;
  object_class->get_property = picman_canvas_polygon_get_property;

  item_class->draw           = picman_canvas_polygon_draw;
  item_class->get_extents    = picman_canvas_polygon_get_extents;

  g_object_class_install_property (object_class, PROP_POINTS,
                                   picman_param_spec_array ("points", NULL, NULL,
                                                          PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_FILLED,
                                   g_param_spec_boolean ("filled", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasPolygonPrivate));
}

static void
picman_canvas_polygon_init (PicmanCanvasPolygon *polygon)
{
}

static void
picman_canvas_polygon_finalize (GObject *object)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (object);

  if (private->points)
    {
      g_free (private->points);
      private->points = NULL;
      private->n_points = 0;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_polygon_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_POINTS:
      {
        PicmanArray *array = g_value_get_boxed (value);

        g_free (private->points);
        private->points = NULL;
        private->n_points = 0;

        if (array)
          {
            private->points = g_memdup (array->data, array->length);
            private->n_points = array->length / sizeof (PicmanVector2);
          }
      }
      break;
    case PROP_FILLED:
      private->filled = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_polygon_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_POINTS:
      if (private->points)
        {
          PicmanArray *array;

          array = picman_array_new ((const guint8 *) private->points,
                                  private->n_points * sizeof (PicmanVector2),
                                  FALSE);
          g_value_take_boxed (value, array);
        }
      else
        {
          g_value_set_boxed (value, NULL);
        }
      break;
    case PROP_FILLED:
      g_value_set_boolean (value, private->filled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_polygon_transform (PicmanCanvasItem *item,
                               PicmanVector2    *points)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (item);
  gint                      i;

  for (i = 0; i < private->n_points; i++)
    {
      picman_canvas_item_transform_xy_f (item,
                                       private->points[i].x,
                                       private->points[i].y,
                                       &points[i].x,
                                       &points[i].y);

      points[i].x = floor (points[i].x) + 0.5;
      points[i].y = floor (points[i].y) + 0.5;
    }
}

static void
picman_canvas_polygon_draw (PicmanCanvasItem *item,
                          cairo_t        *cr)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (item);
  PicmanVector2              *points;
  gint                      i;

  points = g_new0 (PicmanVector2, private->n_points);

  picman_canvas_polygon_transform (item, points);

  cairo_move_to (cr, points[0].x, points[0].y);

  for (i = 1; i < private->n_points; i++)
    {
      cairo_line_to (cr, points[i].x, points[i].y);
    }

  if (private->filled)
    _picman_canvas_item_fill (item, cr);
  else
    _picman_canvas_item_stroke (item, cr);

  g_free (points);
}

static cairo_region_t *
picman_canvas_polygon_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasPolygonPrivate *private = GET_PRIVATE (item);
  cairo_rectangle_int_t     rectangle;
  PicmanVector2              *points;
  gint                      x1, y1, x2, y2;
  gint                      i;

  points = g_new0 (PicmanVector2, private->n_points);

  picman_canvas_polygon_transform (item, points);

  x1 = floor (points[0].x - 1.5);
  y1 = floor (points[0].y - 1.5);
  x2 = x1 + 3;
  y2 = y1 + 3;

  for (i = 1; i < private->n_points; i++)
    {
      gint x3 = floor (points[i].x - 1.5);
      gint y3 = floor (points[i].y - 1.5);
      gint x4 = x3 + 3;
      gint y4 = y3 + 3;

      x1 = MIN (x1, x3);
      y1 = MIN (y1, y3);
      x2 = MAX (x2, x4);
      y2 = MAX (y2, y4);
    }

  g_free (points);

  rectangle.x      = x1;
  rectangle.y      = y1;
  rectangle.width  = x2 - x1;
  rectangle.height = y2 - y1;

  return cairo_region_create_rectangle (&rectangle);
}

PicmanCanvasItem *
picman_canvas_polygon_new (PicmanDisplayShell  *shell,
                         const PicmanVector2 *points,
                         gint               n_points,
                         gboolean           filled)
{
  PicmanCanvasItem *item;
  PicmanArray      *array;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (points != NULL && n_points > 1, NULL);

  array = picman_array_new ((const guint8 *) points,
                          n_points * sizeof (PicmanVector2), TRUE);

  item = g_object_new (PICMAN_TYPE_CANVAS_POLYGON,
                       "shell",  shell,
                       "filled", filled,
                       "points", array,
                       NULL);

  picman_array_free (array);

  return item;
}

PicmanCanvasItem *
picman_canvas_polygon_new_from_coords (PicmanDisplayShell *shell,
                                     const PicmanCoords *coords,
                                     gint              n_coords,
                                     gboolean          filled)
{
  PicmanCanvasItem *item;
  PicmanVector2    *points;
  PicmanArray      *array;
  gint            i;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (coords != NULL && n_coords > 1, NULL);

  points = g_new (PicmanVector2, n_coords);

  for (i = 0; i < n_coords; i++)
    {
      points[i].x = coords[i].x;
      points[i].y = coords[i].y;
    }

  array = picman_array_new ((const guint8 *) points,
                          n_coords * sizeof (PicmanVector2), TRUE);

  item = g_object_new (PICMAN_TYPE_CANVAS_POLYGON,
                       "shell",  shell,
                       "filled", filled,
                       "points", array,
                       NULL);

  picman_array_free (array);
  g_free (points);

  return item;
}
