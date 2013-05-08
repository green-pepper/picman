/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvaspen.c
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
#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "core/picmancontext.h"
#include "core/picmanparamspecs.h"

#include "picmancanvas-style.h"
#include "picmancanvaspen.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_COLOR,
  PROP_WIDTH
};


typedef struct _PicmanCanvasPenPrivate PicmanCanvasPenPrivate;

struct _PicmanCanvasPenPrivate
{
  PicmanRGB color;
  gint    width;
};

#define GET_PRIVATE(pen) \
        G_TYPE_INSTANCE_GET_PRIVATE (pen, \
                                     PICMAN_TYPE_CANVAS_PEN, \
                                     PicmanCanvasPenPrivate)


/*  local function prototypes  */

static void             picman_canvas_pen_set_property (GObject        *object,
                                                      guint           property_id,
                                                      const GValue   *value,
                                                      GParamSpec     *pspec);
static void             picman_canvas_pen_get_property (GObject        *object,
                                                      guint           property_id,
                                                      GValue         *value,
                                                      GParamSpec     *pspec);
static cairo_region_t * picman_canvas_pen_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_pen_stroke       (PicmanCanvasItem *item,
                                                      cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasPen, picman_canvas_pen,
               PICMAN_TYPE_CANVAS_POLYGON)

#define parent_class picman_canvas_pen_parent_class


static void
picman_canvas_pen_class_init (PicmanCanvasPenClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_pen_set_property;
  object_class->get_property = picman_canvas_pen_get_property;

  item_class->get_extents    = picman_canvas_pen_get_extents;
  item_class->stroke         = picman_canvas_pen_stroke;

  g_object_class_install_property (object_class, PROP_COLOR,
                                   picman_param_spec_rgb ("color", NULL, NULL,
                                                        FALSE, NULL,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_WIDTH,
                                   g_param_spec_int ("width", NULL, NULL,
                                                     1, G_MAXINT, 1,
                                                     PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasPenPrivate));
}

static void
picman_canvas_pen_init (PicmanCanvasPen *pen)
{
}

static void
picman_canvas_pen_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanCanvasPenPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_COLOR:
      picman_value_get_rgb (value, &private->color);
      break;
    case PROP_WIDTH:
      private->width = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_pen_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanCanvasPenPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_COLOR:
      picman_value_set_rgb (value, &private->color);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, private->width);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static cairo_region_t *
picman_canvas_pen_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasPenPrivate *private = GET_PRIVATE (item);
  cairo_region_t       *region;

  region = PICMAN_CANVAS_ITEM_CLASS (parent_class)->get_extents (item);

  if (region)
    {
      cairo_rectangle_int_t rectangle;

      cairo_region_get_extents (region, &rectangle);

      rectangle.x      -= ceil (private->width / 2.0);
      rectangle.y      -= ceil (private->width / 2.0);
      rectangle.width  += private->width + 1;
      rectangle.height += private->width + 1;

      cairo_region_union_rectangle (region, &rectangle);
    }

  return region;
}

static void
picman_canvas_pen_stroke (PicmanCanvasItem *item,
                        cairo_t        *cr)
{
  PicmanCanvasPenPrivate *private = GET_PRIVATE (item);

  picman_canvas_set_pen_style (picman_canvas_item_get_canvas (item), cr,
                             &private->color, private->width);
  cairo_stroke (cr);
}

PicmanCanvasItem *
picman_canvas_pen_new (PicmanDisplayShell  *shell,
                     const PicmanVector2 *points,
                     gint               n_points,
                     PicmanContext       *context,
                     PicmanActiveColor    color,
                     gint               width)
{
  PicmanCanvasItem *item;
  PicmanArray      *array;
  PicmanRGB         rgb;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (points != NULL && n_points > 1, NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  array = picman_array_new ((const guint8 *) points,
                          n_points * sizeof (PicmanVector2), TRUE);

  switch (color)
    {
    case PICMAN_ACTIVE_COLOR_FOREGROUND:
      picman_context_get_foreground (context, &rgb);
      break;

    case PICMAN_ACTIVE_COLOR_BACKGROUND:
      picman_context_get_background (context, &rgb);
      break;
    }

  item = g_object_new (PICMAN_TYPE_CANVAS_PEN,
                       "shell",  shell,
                       "points", array,
                       "color",  &rgb,
                       "width",  width,
                       NULL);

  picman_array_free (array);

  return item;
}
