/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvassamplepoint.c
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

#include "picmancanvas.h"
#include "picmancanvas-style.h"
#include "picmancanvassamplepoint.h"
#include "picmandisplayshell.h"


#define PICMAN_SAMPLE_POINT_DRAW_SIZE 10


enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_INDEX,
  PROP_SAMPLE_POINT_STYLE
};


typedef struct _PicmanCanvasSamplePointPrivate PicmanCanvasSamplePointPrivate;

struct _PicmanCanvasSamplePointPrivate
{
  gint     x;
  gint     y;
  gint     index;
  gboolean sample_point_style;
};

#define GET_PRIVATE(sample_point) \
        G_TYPE_INSTANCE_GET_PRIVATE (sample_point, \
                                     PICMAN_TYPE_CANVAS_SAMPLE_POINT, \
                                     PicmanCanvasSamplePointPrivate)


/*  local function prototypes  */

static void             picman_canvas_sample_point_set_property (GObject        *object,
                                                               guint           property_id,
                                                               const GValue   *value,
                                                               GParamSpec     *pspec);
static void             picman_canvas_sample_point_get_property (GObject        *object,
                                                               guint           property_id,
                                                               GValue         *value,
                                                               GParamSpec     *pspec);
static void             picman_canvas_sample_point_draw         (PicmanCanvasItem *item,
                                                               cairo_t        *cr);
static cairo_region_t * picman_canvas_sample_point_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_sample_point_stroke       (PicmanCanvasItem *item,
                                                               cairo_t        *cr);
static void             picman_canvas_sample_point_fill         (PicmanCanvasItem *item,
                                                               cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasSamplePoint, picman_canvas_sample_point,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_sample_point_parent_class


static void
picman_canvas_sample_point_class_init (PicmanCanvasSamplePointClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_sample_point_set_property;
  object_class->get_property = picman_canvas_sample_point_get_property;

  item_class->draw           = picman_canvas_sample_point_draw;
  item_class->get_extents    = picman_canvas_sample_point_get_extents;
  item_class->stroke         = picman_canvas_sample_point_stroke;
  item_class->fill           = picman_canvas_sample_point_fill;

  g_object_class_install_property (object_class, PROP_X,
                                   g_param_spec_int ("x", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_Y,
                                   g_param_spec_int ("y", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_INDEX,
                                   g_param_spec_int ("index", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_SAMPLE_POINT_STYLE,
                                   g_param_spec_boolean ("sample-point-style",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasSamplePointPrivate));
}

static void
picman_canvas_sample_point_init (PicmanCanvasSamplePoint *sample_point)
{
}

static void
picman_canvas_sample_point_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_X:
      private->x = g_value_get_int (value);
      break;
    case PROP_Y:
      private->y = g_value_get_int (value);
      break;
    case PROP_INDEX:
      private->index = g_value_get_int (value);
      break;
    case PROP_SAMPLE_POINT_STYLE:
      private->sample_point_style = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_sample_point_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_X:
      g_value_set_int (value, private->x);
      break;
    case PROP_Y:
      g_value_set_int (value, private->x);
      break;
    case PROP_INDEX:
      g_value_set_int (value, private->x);
      break;
    case PROP_SAMPLE_POINT_STYLE:
      g_value_set_boolean (value, private->sample_point_style);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_sample_point_transform (PicmanCanvasItem *item,
                                    gdouble        *x,
                                    gdouble        *y)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (item);

  picman_canvas_item_transform_xy_f (item,
                                   private->x + 0.5,
                                   private->y + 0.5,
                                   x, y);

  *x = floor (*x) + 0.5;
  *y = floor (*y) + 0.5;
}

#define HALF_SIZE (PICMAN_SAMPLE_POINT_DRAW_SIZE / 2)

static void
picman_canvas_sample_point_draw (PicmanCanvasItem *item,
                               cairo_t        *cr)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (item);
  GtkWidget                    *canvas  = picman_canvas_item_get_canvas (item);
  PangoLayout                  *layout;
  gdouble                       x, y;
  gint                          x1, x2, y1, y2;

  picman_canvas_sample_point_transform (item, &x, &y);

  x1 = x - PICMAN_SAMPLE_POINT_DRAW_SIZE;
  x2 = x + PICMAN_SAMPLE_POINT_DRAW_SIZE;
  y1 = y - PICMAN_SAMPLE_POINT_DRAW_SIZE;
  y2 = y + PICMAN_SAMPLE_POINT_DRAW_SIZE;

  cairo_move_to (cr, x, y1);
  cairo_line_to (cr, x, y1 + HALF_SIZE);

  cairo_move_to (cr, x, y2);
  cairo_line_to (cr, x, y2 - HALF_SIZE);

  cairo_move_to (cr, x1,             y);
  cairo_line_to (cr, x1 + HALF_SIZE, y);

  cairo_move_to (cr, x2,             y);
  cairo_line_to (cr, x2 - HALF_SIZE, y);

  cairo_arc_negative (cr, x, y, HALF_SIZE, 0.0, 0.5 * G_PI);

  _picman_canvas_item_stroke (item, cr);

  layout = picman_canvas_get_layout (PICMAN_CANVAS (canvas),
                                   "%d", private->index);

  cairo_move_to (cr, x + 2.5, y + 2.5);
  pango_cairo_show_layout (cr, layout);

  _picman_canvas_item_fill (item, cr);
}

static cairo_region_t *
picman_canvas_sample_point_get_extents (PicmanCanvasItem *item)
{
  cairo_rectangle_int_t rectangle;
  gdouble               x, y;
  gint                  x1, x2, y1, y2;

  picman_canvas_sample_point_transform (item, &x, &y);

  x1 = floor (x - PICMAN_SAMPLE_POINT_DRAW_SIZE);
  x2 = ceil  (x + PICMAN_SAMPLE_POINT_DRAW_SIZE);
  y1 = floor (y - PICMAN_SAMPLE_POINT_DRAW_SIZE);
  y2 = ceil  (y + PICMAN_SAMPLE_POINT_DRAW_SIZE);

  rectangle.x      = x1 - 1.5;
  rectangle.y      = y1 - 1.5;
  rectangle.width  = x2 - x1 + 3.0;
  rectangle.height = y2 - y1 + 3.0;

  /* HACK: add 5 so the number gets cleared too */
  rectangle.width  += 5;
  rectangle.height += 5;

  return cairo_region_create_rectangle (&rectangle);
}

static void
picman_canvas_sample_point_stroke (PicmanCanvasItem *item,
                                 cairo_t        *cr)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (item);

  if (private->sample_point_style)
    {
      picman_canvas_set_sample_point_style (picman_canvas_item_get_canvas (item), cr,
                                          picman_canvas_item_get_highlight (item));
      cairo_stroke (cr);
    }
  else
    {
      PICMAN_CANVAS_ITEM_CLASS (parent_class)->stroke (item, cr);
    }
}

static void
picman_canvas_sample_point_fill (PicmanCanvasItem *item,
                               cairo_t        *cr)
{
  PicmanCanvasSamplePointPrivate *private = GET_PRIVATE (item);

  if (private->sample_point_style)
    {
      picman_canvas_set_sample_point_style (picman_canvas_item_get_canvas (item), cr,
                                          picman_canvas_item_get_highlight (item));
      cairo_fill (cr);
    }
  else
    {
      PICMAN_CANVAS_ITEM_CLASS (parent_class)->fill (item, cr);
    }
}

PicmanCanvasItem *
picman_canvas_sample_point_new (PicmanDisplayShell *shell,
                              gint              x,
                              gint              y,
                              gint              index,
                              gboolean          sample_point_style)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_SAMPLE_POINT,
                       "shell",              shell,
                       "x",                  x,
                       "y",                  y,
                       "index",              index,
                       "sample-point-style", sample_point_style,
                       NULL);
}

void
picman_canvas_sample_point_set (PicmanCanvasItem *sample_point,
                              gint            x,
                              gint            y)
{
  g_return_if_fail (PICMAN_IS_CANVAS_SAMPLE_POINT (sample_point));

  picman_canvas_item_begin_change (sample_point);

  g_object_set (sample_point,
                "x", x,
                "y", y,
                NULL);

  picman_canvas_item_end_change (sample_point);
}
