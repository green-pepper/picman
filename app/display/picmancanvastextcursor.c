/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvastextcursor.c
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

#include "picmancanvastextcursor.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_OVERWRITE
};


typedef struct _PicmanCanvasTextCursorPrivate PicmanCanvasTextCursorPrivate;

struct _PicmanCanvasTextCursorPrivate
{
  gint     x;
  gint     y;
  gint     width;
  gint     height;
  gboolean overwrite;
};

#define GET_PRIVATE(text_cursor) \
        G_TYPE_INSTANCE_GET_PRIVATE (text_cursor, \
                                     PICMAN_TYPE_CANVAS_TEXT_CURSOR, \
                                     PicmanCanvasTextCursorPrivate)


/*  local function prototypes  */

static void             picman_canvas_text_cursor_set_property (GObject        *object,
                                                              guint           property_id,
                                                              const GValue   *value,
                                                              GParamSpec     *pspec);
static void             picman_canvas_text_cursor_get_property (GObject        *object,
                                                              guint           property_id,
                                                              GValue         *value,
                                                              GParamSpec     *pspec);
static void             picman_canvas_text_cursor_draw         (PicmanCanvasItem *item,
                                                              cairo_t        *cr);
static cairo_region_t * picman_canvas_text_cursor_get_extents  (PicmanCanvasItem *item);


G_DEFINE_TYPE (PicmanCanvasTextCursor, picman_canvas_text_cursor,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_text_cursor_parent_class


static void
picman_canvas_text_cursor_class_init (PicmanCanvasTextCursorClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_text_cursor_set_property;
  object_class->get_property = picman_canvas_text_cursor_get_property;

  item_class->draw           = picman_canvas_text_cursor_draw;
  item_class->get_extents    = picman_canvas_text_cursor_get_extents;

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

  g_object_class_install_property (object_class, PROP_WIDTH,
                                   g_param_spec_int ("width", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_HEIGHT,
                                   g_param_spec_int ("height", NULL, NULL,
                                                     -PICMAN_MAX_IMAGE_SIZE,
                                                     PICMAN_MAX_IMAGE_SIZE, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_OVERWRITE,
                                   g_param_spec_boolean ("overwrite", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasTextCursorPrivate));
}

static void
picman_canvas_text_cursor_init (PicmanCanvasTextCursor *text_cursor)
{
}

static void
picman_canvas_text_cursor_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PicmanCanvasTextCursorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_X:
      private->x = g_value_get_int (value);
      break;
    case PROP_Y:
      private->y = g_value_get_int (value);
      break;
    case PROP_WIDTH:
      private->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      private->height = g_value_get_int (value);
      break;
    case PROP_OVERWRITE:
      private->overwrite = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_text_cursor_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PicmanCanvasTextCursorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_X:
      g_value_set_int (value, private->x);
      break;
    case PROP_Y:
      g_value_set_int (value, private->y);
      break;
    case PROP_WIDTH:
      g_value_set_int (value, private->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int (value, private->height);
      break;
    case PROP_OVERWRITE:
      g_value_set_boolean (value, private->overwrite);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_text_cursor_transform (PicmanCanvasItem *item,
                                   gdouble        *x,
                                   gdouble        *y,
                                   gdouble        *w,
                                   gdouble        *h)
{
  PicmanCanvasTextCursorPrivate *private = GET_PRIVATE (item);

  picman_canvas_item_transform_xy_f (item,
                                   MIN (private->x,
                                        private->x + private->width),
                                   MIN (private->y,
                                        private->y + private->height),
                                   x, y);
  picman_canvas_item_transform_xy_f (item,
                                   MAX (private->x,
                                        private->x + private->width),
                                   MAX (private->y,
                                        private->y + private->height),
                                   w, h);

  *w -= *x;
  *h -= *y;

  *x = floor (*x) + 0.5;
  *y = floor (*y) + 0.5;

  if (private->overwrite)
    {
      *w = ceil (*w) - 1.0;
      *h = ceil (*h) - 1.0;
    }
  else
    {
      *w = 0;
      *h = ceil (*h) - 1.0;
    }
}

static void
picman_canvas_text_cursor_draw (PicmanCanvasItem *item,
                              cairo_t        *cr)
{
  PicmanCanvasTextCursorPrivate *private = GET_PRIVATE (item);
  gdouble                      x, y;
  gdouble                      w, h;

  picman_canvas_text_cursor_transform (item, &x, &y, &w, &h);

  if (private->overwrite)
    {
      cairo_rectangle (cr, x, y, w, h);
    }
  else
    {
      cairo_move_to (cr, x, y);
      cairo_line_to (cr, x, y + h);

      cairo_move_to (cr, x - 3.0, y);
      cairo_line_to (cr, x + 3.0, y);

      cairo_move_to (cr, x - 3.0, y + h);
      cairo_line_to (cr, x + 3.0, y + h);
    }

  _picman_canvas_item_stroke (item, cr);
}

static cairo_region_t *
picman_canvas_text_cursor_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasTextCursorPrivate *private = GET_PRIVATE (item);
  cairo_rectangle_int_t        rectangle;
  gdouble                      x, y;
  gdouble                      w, h;

  picman_canvas_text_cursor_transform (item, &x, &y, &w, &h);

  if (private->overwrite)
    {
      rectangle.x      = floor (x - 1.5);
      rectangle.y      = floor (y - 1.5);
      rectangle.width  = ceil (w + 3.0);
      rectangle.height = ceil (h + 3.0);
    }
  else
    {
      rectangle.x      = floor (x - 4.5);
      rectangle.y      = floor (y - 1.5);
      rectangle.width  = ceil (9.0);
      rectangle.height = ceil (h + 3.0);
    }

  return cairo_region_create_rectangle (&rectangle);
}

PicmanCanvasItem *
picman_canvas_text_cursor_new (PicmanDisplayShell *shell,
                             PangoRectangle   *cursor,
                             gboolean          overwrite)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (cursor != NULL, NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_TEXT_CURSOR,
                       "shell",     shell,
                       "x",         cursor->x,
                       "y",         cursor->y,
                       "width",     cursor->width,
                       "height",    cursor->height,
                       "overwrite", overwrite,
                       NULL);
}
