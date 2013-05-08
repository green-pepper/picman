/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvaspath.c
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

#include "core/picmanbezierdesc.h"
#include "core/picmanparamspecs.h"

#include "picmancanvas-style.h"
#include "picmancanvaspath.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_PATH,
  PROP_X,
  PROP_Y,
  PROP_FILLED,
  PROP_PATH_STYLE
};


typedef struct _PicmanCanvasPathPrivate PicmanCanvasPathPrivate;

struct _PicmanCanvasPathPrivate
{
  cairo_path_t *path;
  gdouble       x;
  gdouble       y;
  gboolean      filled;
  PicmanPathStyle path_style;
};

#define GET_PRIVATE(path) \
        G_TYPE_INSTANCE_GET_PRIVATE (path, \
                                     PICMAN_TYPE_CANVAS_PATH, \
                                     PicmanCanvasPathPrivate)

/*  local function prototypes  */

static void             picman_canvas_path_finalize     (GObject        *object);
static void             picman_canvas_path_set_property (GObject        *object,
                                                       guint           property_id,
                                                       const GValue   *value,
                                                       GParamSpec     *pspec);
static void             picman_canvas_path_get_property (GObject        *object,
                                                       guint           property_id,
                                                       GValue         *value,
                                                       GParamSpec     *pspec);
static void             picman_canvas_path_draw         (PicmanCanvasItem *item,
                                                       cairo_t        *cr);
static cairo_region_t * picman_canvas_path_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_path_stroke       (PicmanCanvasItem *item,
                                                       cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasPath, picman_canvas_path,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_path_parent_class


static void
picman_canvas_path_class_init (PicmanCanvasPathClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = picman_canvas_path_finalize;
  object_class->set_property = picman_canvas_path_set_property;
  object_class->get_property = picman_canvas_path_get_property;

  item_class->draw           = picman_canvas_path_draw;
  item_class->get_extents    = picman_canvas_path_get_extents;
  item_class->stroke         = picman_canvas_path_stroke;

  g_object_class_install_property (object_class, PROP_PATH,
                                   g_param_spec_boxed ("path", NULL, NULL,
                                                       PICMAN_TYPE_BEZIER_DESC,
                                                       PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_X,
                                   g_param_spec_double ("x", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_Y,
                                   g_param_spec_double ("y", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_FILLED,
                                   g_param_spec_boolean ("filled", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));


  g_object_class_install_property (object_class, PROP_PATH_STYLE,
                                   g_param_spec_enum ("path-style", NULL, NULL,
                                                      PICMAN_TYPE_PATH_STYLE,
                                                      PICMAN_PATH_STYLE_DEFAULT,
                                                      PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasPathPrivate));
}

static void
picman_canvas_path_init (PicmanCanvasPath *path)
{
}

static void
picman_canvas_path_finalize (GObject *object)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (object);

  if (private->path)
    {
      picman_bezier_desc_free (private->path);
      private->path = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_path_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PATH:
      if (private->path)
        picman_bezier_desc_free (private->path);
      private->path = g_value_dup_boxed (value);
      break;
    case PROP_X:
      private->x = g_value_get_double (value);
      break;
    case PROP_Y:
      private->y = g_value_get_double (value);
      break;
    case PROP_FILLED:
      private->filled = g_value_get_boolean (value);
      break;
    case PROP_PATH_STYLE:
      private->path_style = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_path_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PATH:
      g_value_set_boxed (value, private->path);
      break;
    case PROP_X:
      g_value_set_double (value, private->x);
      break;
    case PROP_Y:
      g_value_set_double (value, private->y);
      break;
    case PROP_FILLED:
      g_value_set_boolean (value, private->filled);
      break;
    case PROP_PATH_STYLE:
      g_value_set_enum (value, private->path_style);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_path_draw (PicmanCanvasItem *item,
                       cairo_t        *cr)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (item);

  if (private->path)
    {
      cairo_save (cr);
      picman_canvas_item_transform (item, cr);
      cairo_translate (cr, private->x, private->y);

      cairo_append_path (cr, private->path);
      cairo_restore (cr);

      if (private->filled)
        _picman_canvas_item_fill (item, cr);
      else
        _picman_canvas_item_stroke (item, cr);
    }
}

static cairo_region_t *
picman_canvas_path_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (item);
  GtkWidget             *canvas  = picman_canvas_item_get_canvas (item);

  if (private->path && gtk_widget_get_realized (canvas))
    {
      cairo_t               *cr;
      cairo_rectangle_int_t  rectangle;
      gdouble                x1, y1, x2, y2;

      cr = gdk_cairo_create (gtk_widget_get_window (canvas));

      cairo_save (cr);
      picman_canvas_item_transform (item, cr);
      cairo_translate (cr, private->x, private->y);

      cairo_append_path (cr, private->path);
      cairo_restore (cr);

      cairo_path_extents (cr, &x1, &y1, &x2, &y2);

      cairo_destroy (cr);

      if (private->filled)
        {
          rectangle.x      = floor (x1 - 1.0);
          rectangle.y      = floor (y1 - 1.0);
          rectangle.width  = ceil (x2 + 1.0) - rectangle.x;
          rectangle.height = ceil (y2 + 1.0) - rectangle.y;
        }
      else
        {
          rectangle.x      = floor (x1 - 1.5);
          rectangle.y      = floor (y1 - 1.5);
          rectangle.width  = ceil (x2 + 1.5) - rectangle.x;
          rectangle.height = ceil (y2 + 1.5) - rectangle.y;
        }

      return cairo_region_create_rectangle (&rectangle);
    }

  return NULL;
}

static void
picman_canvas_path_stroke (PicmanCanvasItem *item,
                         cairo_t        *cr)
{
  PicmanCanvasPathPrivate *private = GET_PRIVATE (item);
  GtkWidget             *canvas  = picman_canvas_item_get_canvas (item);
  gboolean               active;

  switch (private->path_style)
    {
    case PICMAN_PATH_STYLE_VECTORS:
      active = picman_canvas_item_get_highlight (item);

      picman_canvas_set_vectors_bg_style (canvas, cr, active);
      cairo_stroke_preserve (cr);

      picman_canvas_set_vectors_fg_style (canvas, cr, active);
      cairo_stroke (cr);
      break;

    case PICMAN_PATH_STYLE_OUTLINE:
      picman_canvas_set_outline_bg_style (canvas, cr);
      cairo_stroke_preserve (cr);

      picman_canvas_set_outline_fg_style (canvas, cr);
      cairo_stroke (cr);
      break;

    case PICMAN_PATH_STYLE_DEFAULT:
      PICMAN_CANVAS_ITEM_CLASS (parent_class)->stroke (item, cr);
      break;
    }
}

PicmanCanvasItem *
picman_canvas_path_new (PicmanDisplayShell     *shell,
                      const PicmanBezierDesc *bezier,
                      gdouble               x,
                      gdouble               y,
                      gboolean              filled,
                      PicmanPathStyle         style)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_PATH,
                       "shell",      shell,
                       "path",       bezier,
                       "x",          x,
                       "y",          y,
                       "filled",     filled,
                       "path-style", style,
                       NULL);
}

void
picman_canvas_path_set (PicmanCanvasItem       *path,
                      const PicmanBezierDesc *bezier)
{
  g_return_if_fail (PICMAN_IS_CANVAS_PATH (path));

  picman_canvas_item_begin_change (path);

  g_object_set (path,
                "path", bezier,
                NULL);

  picman_canvas_item_end_change (path);
}
