/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasguide.c
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

#include "picmancanvas-style.h"
#include "picmancanvasguide.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_ORIENTATION,
  PROP_POSITION,
  PROP_GUIDE_STYLE
};


typedef struct _PicmanCanvasGuidePrivate PicmanCanvasGuidePrivate;

struct _PicmanCanvasGuidePrivate
{
  PicmanOrientationType orientation;
  gint                position;
  gboolean            guide_style;
};

#define GET_PRIVATE(guide) \
        G_TYPE_INSTANCE_GET_PRIVATE (guide, \
                                     PICMAN_TYPE_CANVAS_GUIDE, \
                                     PicmanCanvasGuidePrivate)


/*  local function prototypes  */

static void             picman_canvas_guide_set_property (GObject        *object,
                                                        guint           property_id,
                                                        const GValue   *value,
                                                        GParamSpec     *pspec);
static void             picman_canvas_guide_get_property (GObject        *object,
                                                        guint           property_id,
                                                        GValue         *value,
                                                        GParamSpec     *pspec);
static void             picman_canvas_guide_draw         (PicmanCanvasItem *item,
                                                        cairo_t        *cr);
static cairo_region_t * picman_canvas_guide_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_guide_stroke       (PicmanCanvasItem *item,
                                                        cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasGuide, picman_canvas_guide, PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_guide_parent_class


static void
picman_canvas_guide_class_init (PicmanCanvasGuideClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_guide_set_property;
  object_class->get_property = picman_canvas_guide_get_property;

  item_class->draw           = picman_canvas_guide_draw;
  item_class->get_extents    = picman_canvas_guide_get_extents;
  item_class->stroke         = picman_canvas_guide_stroke;

  g_object_class_install_property (object_class, PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation", NULL, NULL,
                                                      PICMAN_TYPE_ORIENTATION_TYPE,
                                                      PICMAN_ORIENTATION_HORIZONTAL,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_POSITION,
                                   g_param_spec_int ("position", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_GUIDE_STYLE,
                                   g_param_spec_boolean ("guide-style",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasGuidePrivate));
}

static void
picman_canvas_guide_init (PicmanCanvasGuide *guide)
{
}

static void
picman_canvas_guide_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanCanvasGuidePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      private->orientation = g_value_get_enum (value);
      break;
    case PROP_POSITION:
      private->position = g_value_get_int (value);
      break;
    case PROP_GUIDE_STYLE:
      private->guide_style = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_guide_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanCanvasGuidePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, private->orientation);
      break;
    case PROP_POSITION:
      g_value_set_int (value, private->position);
      break;
    case PROP_GUIDE_STYLE:
      g_value_set_boolean (value, private->guide_style);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_guide_transform (PicmanCanvasItem *item,
                             gdouble        *x1,
                             gdouble        *y1,
                             gdouble        *x2,
                             gdouble        *y2)
{
  PicmanCanvasGuidePrivate *private = GET_PRIVATE (item);
  GtkWidget              *canvas  = picman_canvas_item_get_canvas (item);
  GtkAllocation           allocation;
  gint                    max_outside;
  gint                    x, y;

  gtk_widget_get_allocation (canvas, &allocation);

  max_outside = allocation.width + allocation.height;

  *x1 = -max_outside;
  *y1 = -max_outside;
  *x2 = allocation.width  + max_outside;
  *y2 = allocation.height + max_outside;

  switch (private->orientation)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      picman_canvas_item_transform_xy (item, 0, private->position, &x, &y);
      *y1 = *y2 = y + 0.5;
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      picman_canvas_item_transform_xy (item, private->position, 0, &x, &y);
      *x1 = *x2 = x + 0.5;
      break;

    case PICMAN_ORIENTATION_UNKNOWN:
      return;
    }
}

static void
picman_canvas_guide_draw (PicmanCanvasItem *item,
                        cairo_t        *cr)
{
  gdouble x1, y1;
  gdouble x2, y2;

  picman_canvas_guide_transform (item, &x1, &y1, &x2, &y2);

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);

  _picman_canvas_item_stroke (item, cr);
}

static cairo_region_t *
picman_canvas_guide_get_extents (PicmanCanvasItem *item)
{
  cairo_rectangle_int_t rectangle;
  gdouble               x1, y1;
  gdouble               x2, y2;

  picman_canvas_guide_transform (item, &x1, &y1, &x2, &y2);

  rectangle.x      = MIN (x1, x2) - 1.5;
  rectangle.y      = MIN (y1, y2) - 1.5;
  rectangle.width  = ABS (x2 - x1) + 3.0;
  rectangle.height = ABS (y2 - y1) + 3.0;

  return cairo_region_create_rectangle (&rectangle);
}

static void
picman_canvas_guide_stroke (PicmanCanvasItem *item,
                          cairo_t        *cr)
{
  PicmanCanvasGuidePrivate *private = GET_PRIVATE (item);

  if (private->guide_style)
    {
      picman_canvas_set_guide_style (picman_canvas_item_get_canvas (item), cr,
                                   picman_canvas_item_get_highlight (item));
      cairo_stroke (cr);
    }
  else
    {
      PICMAN_CANVAS_ITEM_CLASS (parent_class)->stroke (item, cr);
    }
}

PicmanCanvasItem *
picman_canvas_guide_new (PicmanDisplayShell    *shell,
                       PicmanOrientationType  orientation,
                       gint                 position,
                       gboolean             guide_style)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_GUIDE,
                       "shell",       shell,
                       "orientation", orientation,
                       "position",    position,
                       "guide-style", guide_style,
                       NULL);
}

void
picman_canvas_guide_set (PicmanCanvasItem      *guide,
                       PicmanOrientationType  orientation,
                       gint                 position)
{
  g_return_if_fail (PICMAN_IS_CANVAS_GUIDE (guide));

  picman_canvas_item_begin_change (guide);

  g_object_set (guide,
                "orientation", orientation,
                "position",    position,
                NULL);

  picman_canvas_item_end_change (guide);
}
