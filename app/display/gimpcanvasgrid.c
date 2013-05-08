/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasgrid.c
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
#include "libpicmanconfig/picmanconfig.h"

#include "display-types.h"

#include "core/picmangrid.h"
#include "core/picmanimage.h"

#include "picmancanvas-style.h"
#include "picmancanvasgrid.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_GRID,
  PROP_GRID_STYLE
};


typedef struct _PicmanCanvasGridPrivate PicmanCanvasGridPrivate;

struct _PicmanCanvasGridPrivate
{
  PicmanGrid *grid;
  gboolean  grid_style;
};

#define GET_PRIVATE(grid) \
        G_TYPE_INSTANCE_GET_PRIVATE (grid, \
                                     PICMAN_TYPE_CANVAS_GRID, \
                                     PicmanCanvasGridPrivate)


/*  local function prototypes  */

static void             picman_canvas_grid_finalize     (GObject        *object);
static void             picman_canvas_grid_set_property (GObject        *object,
                                                       guint           property_id,
                                                       const GValue   *value,
                                                       GParamSpec     *pspec);
static void             picman_canvas_grid_get_property (GObject        *object,
                                                       guint           property_id,
                                                       GValue         *value,
                                                       GParamSpec     *pspec);
static void             picman_canvas_grid_draw         (PicmanCanvasItem *item,
                                                       cairo_t        *cr);
static cairo_region_t * picman_canvas_grid_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_grid_stroke       (PicmanCanvasItem *item,
                                                       cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasGrid, picman_canvas_grid, PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_grid_parent_class


static void
picman_canvas_grid_class_init (PicmanCanvasGridClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = picman_canvas_grid_finalize;
  object_class->set_property = picman_canvas_grid_set_property;
  object_class->get_property = picman_canvas_grid_get_property;

  item_class->draw           = picman_canvas_grid_draw;
  item_class->get_extents    = picman_canvas_grid_get_extents;
  item_class->stroke         = picman_canvas_grid_stroke;

  g_object_class_install_property (object_class, PROP_GRID,
                                   g_param_spec_object ("grid", NULL, NULL,
                                                        PICMAN_TYPE_GRID,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_GRID_STYLE,
                                   g_param_spec_boolean ("grid-style",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasGridPrivate));
}

static void
picman_canvas_grid_init (PicmanCanvasGrid *grid)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (grid);

  private->grid = g_object_new (PICMAN_TYPE_GRID, NULL);
}

static void
picman_canvas_grid_finalize (GObject *object)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (object);

  if (private->grid)
    {
      g_object_unref (private->grid);
      private->grid = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_grid_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_GRID:
      {
        PicmanGrid *grid = g_value_get_object (value);
        if (grid)
          picman_config_sync (G_OBJECT (grid), G_OBJECT (private->grid), 0);
      }
      break;
    case PROP_GRID_STYLE:
      private->grid_style = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_grid_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_GRID:
      g_value_set_object (value, private->grid);
      break;
    case PROP_GRID_STYLE:
      g_value_set_boolean (value, private->grid_style);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_grid_draw (PicmanCanvasItem *item,
                       cairo_t        *cr)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (item);
  PicmanDisplayShell      *shell   = picman_canvas_item_get_shell (item);
  PicmanImage             *image   = picman_canvas_item_get_image (item);
  gdouble                x, y;
  gdouble                dx1, dy1, dx2, dy2;
  gint                   x0, x1, x2, x3;
  gint                   y0, y1, y2, y3;
  gint                   x_real, y_real;
  gdouble                x_offset, y_offset;
  gint                   width, height;

#define CROSSHAIR 2

  g_return_if_fail (private->grid->xspacing > 0 &&
                    private->grid->yspacing > 0);

  /*  skip grid drawing when the space between grid lines starts
   *  disappearing, see bug #599267.
   */
  if (private->grid->xspacing * shell->scale_x < 2.0 ||
      private->grid->yspacing * shell->scale_y < 2.0)
    return;

  cairo_clip_extents (cr, &dx1, &dy1, &dx2, &dy2);

  x1 = floor (dx1);
  y1 = floor (dy1);
  x2 = ceil (dx2);
  y2 = ceil (dy2);

  width  = picman_image_get_width  (image);
  height = picman_image_get_height (image);

  x_offset = private->grid->xoffset;
  while (x_offset > 0)
    x_offset -= private->grid->xspacing;

  y_offset = private->grid->yoffset;
  while (y_offset > 0)
    y_offset -= private->grid->yspacing;

  switch (private->grid->style)
    {
    case PICMAN_GRID_DOTS:
      for (x = x_offset; x <= width; x += private->grid->xspacing)
        {
          if (x < 0)
            continue;

          picman_canvas_item_transform_xy (item, x, 0, &x_real, &y_real);

          if (x_real < x1 || x_real >= x2)
            continue;

          for (y = y_offset; y <= height; y += private->grid->yspacing)
            {
              if (y < 0)
                continue;

              picman_canvas_item_transform_xy (item, x, y, &x_real, &y_real);

              if (y_real >= y1 && y_real < y2)
                {
                  cairo_move_to (cr, x_real,     y_real + 0.5);
                  cairo_line_to (cr, x_real + 1, y_real + 0.5);
                }
            }
        }
      break;

    case PICMAN_GRID_INTERSECTIONS:
      for (x = x_offset; x <= width; x += private->grid->xspacing)
        {
          if (x < 0)
            continue;

          picman_canvas_item_transform_xy (item, x, 0, &x_real, &y_real);

          if (x_real + CROSSHAIR < x1 || x_real - CROSSHAIR >= x2)
            continue;

          for (y = y_offset; y <= height; y += private->grid->yspacing)
            {
              if (y < 0)
                continue;

              picman_canvas_item_transform_xy (item, x, y, &x_real, &y_real);

              if (y_real + CROSSHAIR < y1 || y_real - CROSSHAIR >= y2)
                continue;

              if (x_real >= x1 && x_real < x2)
                {
                  cairo_move_to (cr,
                                 x_real + 0.5,
                                 CLAMP (y_real - CROSSHAIR,
                                        y1, y2 - 1));
                  cairo_line_to (cr,
                                 x_real + 0.5,
                                 CLAMP (y_real + CROSSHAIR,
                                        y1, y2 - 1) + 1);
                }

              if (y_real >= y1 && y_real < y2)
                {
                  cairo_move_to (cr,
                                 CLAMP (x_real - CROSSHAIR,
                                        x1, x2 - 1),
                                 y_real + 0.5);
                  cairo_line_to (cr,
                                 CLAMP (x_real + CROSSHAIR,
                                        x1, x2 - 1) + 1,
                                 y_real + 0.5);
                }
            }
        }
      break;

    case PICMAN_GRID_ON_OFF_DASH:
    case PICMAN_GRID_DOUBLE_DASH:
    case PICMAN_GRID_SOLID:
      picman_canvas_item_transform_xy (item, 0, 0, &x0, &y0);
      picman_canvas_item_transform_xy (item, width, height, &x3, &y3);

      for (x = x_offset; x < width; x += private->grid->xspacing)
        {
          if (x < 0)
            continue;

          picman_canvas_item_transform_xy (item, x, 0, &x_real, &y_real);

          if (x_real >= x1 && x_real < x2)
            {
              cairo_move_to (cr, x_real + 0.5, y0);
              cairo_line_to (cr, x_real + 0.5, y3 + 1);
            }
        }

      for (y = y_offset; y < height; y += private->grid->yspacing)
        {
          if (y < 0)
            continue;

          picman_canvas_item_transform_xy (item, 0, y, &x_real, &y_real);

          if (y_real >= y1 && y_real < y2)
            {
              cairo_move_to (cr, x0,     y_real + 0.5);
              cairo_line_to (cr, x3 + 1, y_real + 0.5);
            }
        }
      break;
    }

  _picman_canvas_item_stroke (item, cr);
}

static cairo_region_t *
picman_canvas_grid_get_extents (PicmanCanvasItem *item)
{
  PicmanImage             *image = picman_canvas_item_get_image (item);
  cairo_rectangle_int_t  rectangle;
  gdouble                x1, y1;
  gdouble                x2, y2;
  gint                   w, h;

  if (! image)
    return NULL;

  w = picman_image_get_width  (image);
  h = picman_image_get_height (image);

  picman_canvas_item_transform_xy_f (item, 0, 0, &x1, &y1);
  picman_canvas_item_transform_xy_f (item, w, h, &x2, &y2);

  rectangle.x      = floor (x1);
  rectangle.y      = floor (y1);
  rectangle.width  = ceil (x2) - rectangle.x;
  rectangle.height = ceil (y2) - rectangle.y;

  return cairo_region_create_rectangle (&rectangle);
}

static void
picman_canvas_grid_stroke (PicmanCanvasItem *item,
                         cairo_t        *cr)
{
  PicmanCanvasGridPrivate *private = GET_PRIVATE (item);

  if (private->grid_style)
    {
      picman_canvas_set_grid_style (picman_canvas_item_get_canvas (item), cr,
                                  private->grid);
      cairo_stroke (cr);
    }
  else
    {
      PICMAN_CANVAS_ITEM_CLASS (parent_class)->stroke (item, cr);
    }
}

PicmanCanvasItem *
picman_canvas_grid_new (PicmanDisplayShell *shell,
                      PicmanGrid         *grid)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);
  g_return_val_if_fail (grid == NULL || PICMAN_IS_GRID (grid), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_GRID,
                       "shell", shell,
                       "grid",  grid,
                       NULL);
}
