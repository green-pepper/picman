/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvashandle.c
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

#include "core/picman-cairo.h"

#include "picmancanvashandle.h"
#include "picmancanvasitem-utils.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_TYPE,
  PROP_ANCHOR,
  PROP_X,
  PROP_Y,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_START_ANGLE,
  PROP_SLICE_ANGLE
};


typedef struct _PicmanCanvasHandlePrivate PicmanCanvasHandlePrivate;

struct _PicmanCanvasHandlePrivate
{
  PicmanHandleType   type;
  PicmanHandleAnchor anchor;
  gdouble          x;
  gdouble          y;
  gint             width;
  gint             height;
  gdouble          start_angle;
  gdouble          slice_angle;
};

#define GET_PRIVATE(handle) \
        G_TYPE_INSTANCE_GET_PRIVATE (handle, \
                                     PICMAN_TYPE_CANVAS_HANDLE, \
                                     PicmanCanvasHandlePrivate)


/*  local function prototypes  */

static void             picman_canvas_handle_set_property (GObject        *object,
                                                         guint           property_id,
                                                         const GValue   *value,
                                                         GParamSpec     *pspec);
static void             picman_canvas_handle_get_property (GObject        *object,
                                                         guint           property_id,
                                                         GValue         *value,
                                                         GParamSpec     *pspec);
static void             picman_canvas_handle_draw         (PicmanCanvasItem *item,
                                                         cairo_t        *cr);
static cairo_region_t * picman_canvas_handle_get_extents  (PicmanCanvasItem *item);
static gboolean         picman_canvas_handle_hit          (PicmanCanvasItem *item,
                                                         gdouble         x,
                                                         gdouble         y);


G_DEFINE_TYPE (PicmanCanvasHandle, picman_canvas_handle,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_handle_parent_class


static void
picman_canvas_handle_class_init (PicmanCanvasHandleClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_handle_set_property;
  object_class->get_property = picman_canvas_handle_get_property;

  item_class->draw           = picman_canvas_handle_draw;
  item_class->get_extents    = picman_canvas_handle_get_extents;
  item_class->hit            = picman_canvas_handle_hit;

  g_object_class_install_property (object_class, PROP_TYPE,
                                   g_param_spec_enum ("type", NULL, NULL,
                                                      PICMAN_TYPE_HANDLE_TYPE,
                                                      PICMAN_HANDLE_CROSS,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_ANCHOR,
                                   g_param_spec_enum ("anchor", NULL, NULL,
                                                      PICMAN_TYPE_HANDLE_ANCHOR,
                                                      PICMAN_HANDLE_ANCHOR_CENTER,
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

  g_object_class_install_property (object_class, PROP_WIDTH,
                                   g_param_spec_int ("width", NULL, NULL,
                                                     3, 1001, 7,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_HEIGHT,
                                   g_param_spec_int ("height", NULL, NULL,
                                                     3, 1001, 7,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_START_ANGLE,
                                   g_param_spec_double ("start-angle", NULL, NULL,
                                                        -1000, 1000, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_SLICE_ANGLE,
                                   g_param_spec_double ("slice-angle", NULL, NULL,
                                                        -1000, 1000, 2 * G_PI,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasHandlePrivate));
}

static void
picman_canvas_handle_init (PicmanCanvasHandle *handle)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (handle);

  picman_canvas_item_set_line_cap (PICMAN_CANVAS_ITEM (handle),
                                 CAIRO_LINE_CAP_SQUARE);

  private->start_angle = 0.0;
  private->slice_angle = 2.0 * G_PI;
}

static void
picman_canvas_handle_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TYPE:
      private->type = g_value_get_enum (value);
      break;
    case PROP_ANCHOR:
      private->anchor = g_value_get_enum (value);
      break;
    case PROP_X:
      private->x = g_value_get_double (value);
      break;
    case PROP_Y:
      private->y = g_value_get_double (value);
      break;
    case PROP_WIDTH:
      private->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      private->height = g_value_get_int (value);
      break;
    case PROP_START_ANGLE:
      private->start_angle = g_value_get_double (value);
      break;
    case PROP_SLICE_ANGLE:
      private->slice_angle = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_handle_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TYPE:
      g_value_set_enum (value, private->type);
      break;
    case PROP_ANCHOR:
      g_value_set_enum (value, private->anchor);
      break;
    case PROP_X:
      g_value_set_double (value, private->x);
      break;
    case PROP_Y:
      g_value_set_double (value, private->y);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, private->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, private->height);
      break;
    case PROP_START_ANGLE:
      g_value_set_double (value, private->start_angle);
      break;
    case PROP_SLICE_ANGLE:
      g_value_set_double (value, private->slice_angle);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_handle_transform (PicmanCanvasItem *item,
                              gdouble        *x,
                              gdouble        *y)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (item);

  picman_canvas_item_transform_xy_f (item,
                                   private->x, private->y,
                                   x, y);

  switch (private->type)
    {
    case PICMAN_HANDLE_SQUARE:
    case PICMAN_HANDLE_FILLED_SQUARE:
      picman_canvas_item_shift_to_north_west (private->anchor,
                                            *x, *y,
                                            private->width,
                                            private->height,
                                            x, y);
      break;

    case PICMAN_HANDLE_CIRCLE:
    case PICMAN_HANDLE_FILLED_CIRCLE:
    case PICMAN_HANDLE_CROSS:
    case PICMAN_HANDLE_DIAMOND:
    case PICMAN_HANDLE_FILLED_DIAMOND:
      picman_canvas_item_shift_to_center (private->anchor,
                                        *x, *y,
                                        private->width,
                                        private->height,
                                        x, y);
      break;

    default:
      break;
    }

  *x = floor (*x) + 0.5;
  *y = floor (*y) + 0.5;
}

static void
picman_canvas_handle_draw (PicmanCanvasItem *item,
                         cairo_t        *cr)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (item);
  gdouble                  x, y, tx, ty;

  picman_canvas_handle_transform (item, &x, &y);

  picman_canvas_item_transform_xy_f (item,
                                   private->x, private->y,
                                   &tx, &ty);

  switch (private->type)
    {
    case PICMAN_HANDLE_SQUARE:
    case PICMAN_HANDLE_FILLED_SQUARE:
    case PICMAN_HANDLE_DIAMOND:
    case PICMAN_HANDLE_FILLED_DIAMOND:
      cairo_save (cr);
      cairo_translate (cr, tx, ty);
      cairo_rotate (cr, private->start_angle);
      cairo_translate (cr, -tx, -ty);

      switch (private->type)
        {
        case PICMAN_HANDLE_SQUARE:
          cairo_rectangle (cr, x, y, private->width - 1.0, private->height - 1.0);
          _picman_canvas_item_stroke (item, cr);
          break;
        case PICMAN_HANDLE_FILLED_SQUARE:
          cairo_rectangle (cr, x - 0.5, y - 0.5, private->width, private->height);
          _picman_canvas_item_fill (item, cr);
          break;
        case PICMAN_HANDLE_DIAMOND:
        case PICMAN_HANDLE_FILLED_DIAMOND:
          cairo_move_to (cr, x, y - (gdouble) private->height / 2.0);
          cairo_line_to (cr, x + (gdouble) private->width / 2.0, y);
          cairo_line_to (cr, x, y + (gdouble) private->height / 2.0);
          cairo_line_to (cr, x - (gdouble) private->width / 2.0, y);
          cairo_line_to (cr, x, y - (gdouble) private->height / 2.0);
          if (private->type == PICMAN_HANDLE_DIAMOND)
            _picman_canvas_item_stroke (item, cr);
          else
            _picman_canvas_item_fill (item, cr);
          break;
        default:
          g_assert_not_reached ();
        }
      cairo_restore (cr);
      break;

    case PICMAN_HANDLE_CIRCLE:
      picman_cairo_add_arc (cr, x, y, private->width / 2,
                          private->start_angle,
                          private->slice_angle);

      _picman_canvas_item_stroke (item, cr);
      break;

    case PICMAN_HANDLE_FILLED_CIRCLE:
      cairo_move_to (cr, x, y);

      picman_cairo_add_arc (cr, x, y, (gdouble) private->width / 2.0,
                          private->start_angle,
                          private->slice_angle);

      _picman_canvas_item_fill (item, cr);
      break;

    case PICMAN_HANDLE_CROSS:
      cairo_move_to (cr, x - private->width / 2, y);
      cairo_line_to (cr, x + private->width / 2 - 0.5, y);

      cairo_move_to (cr, x, y - private->height / 2);
      cairo_line_to (cr, x, y + private->height / 2 - 0.5);

      _picman_canvas_item_stroke (item, cr);
      break;

    default:
      break;
    }
}

static cairo_region_t *
picman_canvas_handle_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (item);
  cairo_rectangle_int_t    rectangle;
  gdouble                  x, y;
  gdouble                  w, h;

  picman_canvas_handle_transform (item, &x, &y);

  switch (private->type)
    {
    case PICMAN_HANDLE_SQUARE:
    case PICMAN_HANDLE_FILLED_SQUARE:
      w = private->width * (sqrt(2) - 1) / 2;
      h = private->height * (sqrt(2) - 1) / 2;
      rectangle.x      = x - 1.5 - w;
      rectangle.y      = y - 1.5 - h;
      rectangle.width  = private->width  + 3.0 + w * 2;
      rectangle.height = private->height + 3.0 + h * 2;
      break;

    case PICMAN_HANDLE_CIRCLE:
    case PICMAN_HANDLE_FILLED_CIRCLE:
    case PICMAN_HANDLE_CROSS:
    case PICMAN_HANDLE_DIAMOND:
    case PICMAN_HANDLE_FILLED_DIAMOND:
      rectangle.x      = x - private->width  / 2 - 2.0;
      rectangle.y      = y - private->height / 2 - 2.0;
      rectangle.width  = private->width  + 4.0;
      rectangle.height = private->height + 4.0;
      break;

    default:
      break;
    }

  return cairo_region_create_rectangle (&rectangle);
}

static gboolean
picman_canvas_handle_hit (PicmanCanvasItem *item,
                        gdouble         x,
                        gdouble         y)
{
  PicmanCanvasHandlePrivate *private = GET_PRIVATE (item);
  gdouble                  handle_tx, handle_ty;
  gdouble                  mx, my, tx, ty, mmx, mmy;
  gdouble                  diamond_offset_x = 0.0;
  gdouble                  diamond_offset_y = 0.0;
  gdouble                  angle            = -private->start_angle;

  picman_canvas_handle_transform (item, &handle_tx, &handle_ty);

  picman_canvas_item_transform_xy_f (item,
                                   x, y,
                                   &mx, &my);

  switch (private->type)
    {
    case PICMAN_HANDLE_DIAMOND:
    case PICMAN_HANDLE_FILLED_DIAMOND:
      angle -= G_PI / 4.0;
      diamond_offset_x = private->width / 2.0;
      diamond_offset_y = private->height / 2.0;
    case PICMAN_HANDLE_SQUARE:
    case PICMAN_HANDLE_FILLED_SQUARE:
      picman_canvas_item_transform_xy_f (item,
                                       private->x, private->y,
                                       &tx, &ty);
      mmx = mx - tx; mmy = my - ty;
      mx = cos (angle) * mmx - sin (angle) * mmy + tx + diamond_offset_x;
      my = sin (angle) * mmx + cos (angle) * mmy + ty + diamond_offset_y;
      return mx > handle_tx && mx < handle_tx + private->width &&
             my > handle_ty && my < handle_ty + private->height;

    case PICMAN_HANDLE_CIRCLE:
    case PICMAN_HANDLE_FILLED_CIRCLE:
    case PICMAN_HANDLE_CROSS:
      {
        gint width = private->width;

        if (width != private->height)
          width = (width + private->height) / 2;

        width /= 2;

        return ((SQR (handle_tx - mx) + SQR (handle_ty - my)) < SQR (width));
      }

    default:
      break;
    }

  return FALSE;
}

PicmanCanvasItem *
picman_canvas_handle_new (PicmanDisplayShell *shell,
                        PicmanHandleType    type,
                        PicmanHandleAnchor  anchor,
                        gdouble           x,
                        gdouble           y,
                        gint              width,
                        gint              height)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_HANDLE,
                       "shell",  shell,
                       "type",   type,
                       "anchor", anchor,
                       "x",      x,
                       "y",      y,
                       "width",  width,
                       "height", height,
                       NULL);
}

void
picman_canvas_handle_set_position (PicmanCanvasItem *handle,
                                 gdouble         x,
                                 gdouble         y)
{
  g_return_if_fail (PICMAN_IS_CANVAS_HANDLE (handle));

  picman_canvas_item_begin_change (handle);

  g_object_set (handle,
                "x", x,
                "y", y,
                NULL);

  picman_canvas_item_end_change (handle);
}

void
picman_canvas_handle_set_angles (PicmanCanvasItem *handle,
                               gdouble         start_angle,
                               gdouble         slice_angle)
{
  g_return_if_fail (PICMAN_IS_CANVAS_HANDLE (handle));

  picman_canvas_item_begin_change (handle);

  g_object_set (handle,
                "start-angle", start_angle,
                "slice-angle", slice_angle,
                NULL);

  picman_canvas_item_end_change (handle);
}
