/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvastransformguides.c
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#include "core/picman-transform-utils.h"
#include "core/picman-utils.h"

#include "picmancanvastransformguides.h"
#include "picmandisplayshell.h"


#define SQRT5 2.236067977


enum
{
  PROP_0,
  PROP_TRANSFORM,
  PROP_X1,
  PROP_Y1,
  PROP_X2,
  PROP_Y2,
  PROP_TYPE,
  PROP_N_GUIDES
};


typedef struct _PicmanCanvasTransformGuidesPrivate PicmanCanvasTransformGuidesPrivate;

struct _PicmanCanvasTransformGuidesPrivate
{
  PicmanMatrix3    transform;
  gdouble        x1, y1;
  gdouble        x2, y2;
  PicmanGuidesType type;
  gint           n_guides;
};

#define GET_PRIVATE(transform) \
        G_TYPE_INSTANCE_GET_PRIVATE (transform, \
                                     PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES, \
                                     PicmanCanvasTransformGuidesPrivate)


/*  local function prototypes  */

static void             picman_canvas_transform_guides_set_property (GObject        *object,
                                                                   guint           property_id,
                                                                   const GValue   *value,
                                                                   GParamSpec     *pspec);
static void             picman_canvas_transform_guides_get_property (GObject        *object,
                                                                   guint           property_id,
                                                                   GValue         *value,
                                                                   GParamSpec     *pspec);
static void             picman_canvas_transform_guides_draw         (PicmanCanvasItem *item,
                                                                   cairo_t        *cr);
static cairo_region_t * picman_canvas_transform_guides_get_extents  (PicmanCanvasItem *item);


G_DEFINE_TYPE (PicmanCanvasTransformGuides, picman_canvas_transform_guides,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_transform_guides_parent_class


static void
picman_canvas_transform_guides_class_init (PicmanCanvasTransformGuidesClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_transform_guides_set_property;
  object_class->get_property = picman_canvas_transform_guides_get_property;

  item_class->draw           = picman_canvas_transform_guides_draw;
  item_class->get_extents    = picman_canvas_transform_guides_get_extents;

  g_object_class_install_property (object_class, PROP_TRANSFORM,
                                   picman_param_spec_matrix3 ("transform",
                                                            NULL, NULL,
                                                            NULL,
                                                            PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_X1,
                                   g_param_spec_double ("x1",
                                                        NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_Y1,
                                   g_param_spec_double ("y1",
                                                        NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_X2,
                                   g_param_spec_double ("x2",
                                                        NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_Y2,
                                   g_param_spec_double ("y2",
                                                        NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_TYPE,
                                   g_param_spec_enum ("type", NULL, NULL,
                                                      PICMAN_TYPE_GUIDES_TYPE,
                                                      PICMAN_GUIDES_NONE,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_N_GUIDES,
                                   g_param_spec_int ("n-guides", NULL, NULL,
                                                     1, 128, 4,
                                                     PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasTransformGuidesPrivate));
}

static void
picman_canvas_transform_guides_init (PicmanCanvasTransformGuides *transform)
{
}

static void
picman_canvas_transform_guides_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  PicmanCanvasTransformGuidesPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TRANSFORM:
      {
        PicmanMatrix3 *transform = g_value_get_boxed (value);

        if (transform)
          private->transform = *transform;
        else
          picman_matrix3_identity (&private->transform);
      }
      break;

    case PROP_X1:
      private->x1 = g_value_get_double (value);
      break;

    case PROP_Y1:
      private->y1 = g_value_get_double (value);
      break;

    case PROP_X2:
      private->x2 = g_value_get_double (value);
      break;

    case PROP_Y2:
      private->y2 = g_value_get_double (value);
      break;

    case PROP_TYPE:
      private->type = g_value_get_enum (value);
      break;

    case PROP_N_GUIDES:
      private->n_guides = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_transform_guides_get_property (GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  PicmanCanvasTransformGuidesPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TRANSFORM:
      g_value_set_boxed (value, &private->transform);
      break;

    case PROP_X1:
      g_value_set_double (value, private->x1);
      break;

    case PROP_Y1:
      g_value_set_double (value, private->y1);
      break;

    case PROP_X2:
      g_value_set_double (value, private->x2);
      break;

    case PROP_Y2:
      g_value_set_double (value, private->y2);
      break;

    case PROP_TYPE:
      g_value_set_enum (value, private->type);
      break;

    case PROP_N_GUIDES:
      g_value_set_int (value, private->n_guides);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_canvas_transform_guides_transform (PicmanCanvasItem *item,
                                        gdouble        *tx1,
                                        gdouble        *ty1,
                                        gdouble        *tx2,
                                        gdouble        *ty2,
                                        gdouble        *tx3,
                                        gdouble        *ty3,
                                        gdouble        *tx4,
                                        gdouble        *ty4)
{
  PicmanCanvasTransformGuidesPrivate *private = GET_PRIVATE (item);

  picman_matrix3_transform_point (&private->transform,
                                private->x1, private->y1,
                                tx1, ty1);
  picman_matrix3_transform_point (&private->transform,
                                private->x2, private->y1,
                                tx2, ty2);
  picman_matrix3_transform_point (&private->transform,
                                private->x1, private->y2,
                                tx3, ty3);
  picman_matrix3_transform_point (&private->transform,
                                private->x2, private->y2,
                                tx4, ty4);

  return picman_transform_polygon_is_convex (*tx1, *ty1,
                                           *tx2, *ty2,
                                           *tx3, *ty3,
                                           *tx4, *ty4);
}

static void
draw_line (cairo_t        *cr,
           PicmanCanvasItem *item,
           PicmanMatrix3    *transform,
           gdouble         x1,
           gdouble         y1,
           gdouble         x2,
           gdouble         y2)
{
  picman_matrix3_transform_point (transform, x1, y1, &x1, &y1);
  picman_matrix3_transform_point (transform, x2, y2, &x2, &y2);

  picman_canvas_item_transform_xy_f (item, x1, y1, &x1, &y1);
  picman_canvas_item_transform_xy_f (item, x2, y2, &x2, &y2);

  x1 = floor (x1) + 0.5;
  y1 = floor (y1) + 0.5;
  x2 = floor (x2) + 0.5;
  y2 = floor (y2) + 0.5;

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);
}

static void
draw_hline (cairo_t        *cr,
            PicmanCanvasItem *item,
            PicmanMatrix3    *transform,
            gdouble         x1,
            gdouble         x2,
            gdouble         y)
{
  draw_line (cr, item, transform, x1, y, x2, y);
}

static void
draw_vline (cairo_t        *cr,
            PicmanCanvasItem *item,
            PicmanMatrix3    *transform,
            gdouble         y1,
            gdouble         y2,
            gdouble         x)
{
  draw_line (cr, item, transform, x, y1, x, y2);
}

static void
picman_canvas_transform_guides_draw (PicmanCanvasItem *item,
                                   cairo_t        *cr)
{
  PicmanCanvasTransformGuidesPrivate *private = GET_PRIVATE (item);
  gdouble                           x1, y1;
  gdouble                           x2, y2;
  gdouble                           x3, y3;
  gdouble                           x4, y4;
  gboolean                          convex;
  gint                              i;

  convex = picman_canvas_transform_guides_transform (item,
                                                   &x1, &y1,
                                                   &x2, &y2,
                                                   &x3, &y3,
                                                   &x4, &y4);

  picman_canvas_item_transform_xy_f (item, x1, y1, &x1, &y1);
  picman_canvas_item_transform_xy_f (item, x2, y2, &x2, &y2);
  picman_canvas_item_transform_xy_f (item, x3, y3, &x3, &y3);
  picman_canvas_item_transform_xy_f (item, x4, y4, &x4, &y4);

  x1 = floor (x1) + 0.5;
  y1 = floor (y1) + 0.5;
  x2 = floor (x2) + 0.5;
  y2 = floor (y2) + 0.5;
  x3 = floor (x3) + 0.5;
  y3 = floor (y3) + 0.5;
  x4 = floor (x4) + 0.5;
  y4 = floor (y4) + 0.5;

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);
  cairo_line_to (cr, x4, y4);
  cairo_line_to (cr, x3, y3);
  cairo_line_to (cr, x1, y1);

  if (! convex)
    {
      _picman_canvas_item_stroke (item, cr);
      return;
    }

  switch (private->type)
    {
    case PICMAN_GUIDES_NONE:
      break;

    case PICMAN_GUIDES_CENTER_LINES:
      draw_hline (cr, item, &private->transform,
                  private->x1, private->x2, (private->y1 + private->y2) / 2);
      draw_vline (cr, item, &private->transform,
                  private->y1, private->y2, (private->x1 + private->x2) / 2);
      break;

    case PICMAN_GUIDES_THIRDS:
      draw_hline (cr, item, &private->transform,
                  private->x1, private->x2, (2 * private->y1 + private->y2) / 3);
      draw_hline (cr, item, &private->transform,
                  private->x1, private->x2, (private->y1 + 2 * private->y2) / 3);

      draw_vline (cr, item, &private->transform,
                  private->y1, private->y2, (2 * private->x1 + private->x2) / 3);
      draw_vline (cr, item, &private->transform,
                  private->y1, private->y2, (private->x1 + 2 * private->x2) / 3);
      break;

    case PICMAN_GUIDES_FIFTHS:
      for (i = 0; i < 5; i++)
        {
          draw_hline (cr, item, &private->transform,
                      private->x1, private->x2,
                      private->y1 + i * (private->y2 - private->y1) / 5);
          draw_vline (cr, item, &private->transform,
                      private->y1, private->y2,
                      private->x1 + i * (private->x2 - private->x1) / 5);
        }
      break;

    case PICMAN_GUIDES_GOLDEN:
      draw_hline (cr, item, &private->transform,
                  private->x1, private->x2,
                  (2 * private->y1 + (1 + SQRT5) * private->y2) / (3 + SQRT5));
      draw_hline (cr, item, &private->transform,
                  private->x1, private->x2,
                  ((1 + SQRT5) * private->y1 + 2 * private->y2) / (3 + SQRT5));

      draw_vline (cr, item, &private->transform,
                  private->y1, private->y2,
                  (2 * private->x1 + (1 + SQRT5) * private->x2) / (3 + SQRT5));
      draw_vline (cr, item, &private->transform,
                  private->y1, private->y2,
                  ((1 + SQRT5) * private->x1 + 2 * private->x2) / (3 + SQRT5));
      break;

    /* This code implements the method of diagonals discovered by
     * Edwin Westhoff - see http://www.diagonalmethod.info/
     */
    case PICMAN_GUIDES_DIAGONALS:
      {
        /* the side of the largest square that can be
         * fitted in whole into the rectangle (x1, y1), (x2, y2)
         */
        const gdouble square_side = MIN (private->x2 - private->x1,
                                         private->y2 - private->y1);

        /* diagonal from the top-left edge */
        draw_line (cr, item, &private->transform,
                   private->x1, private->y1,
                   private->x1 + square_side,
                   private->y1 + square_side);

        /* diagonal from the top-right edge */
        draw_line (cr, item, &private->transform,
                   private->x2, private->y1,
                   private->x2 - square_side,
                   private->y1 + square_side);

        /* diagonal from the bottom-left edge */
        draw_line (cr, item, &private->transform,
                   private->x1, private->y2,
                   private->x1 + square_side,
                   private->y2 - square_side);

        /* diagonal from the bottom-right edge */
        draw_line (cr, item, &private->transform,
                   private->x2, private->y2,
                   private->x2 - square_side,
                   private->y2 - square_side);
      }
      break;

    case PICMAN_GUIDES_N_LINES:
    case PICMAN_GUIDES_SPACING:
      {
        gint width, height;
        gint ngx, ngy;

        width  = MAX (1, private->x2 - private->x1);
        height = MAX (1, private->y2 - private->y1);

        if (private->type == PICMAN_GUIDES_N_LINES)
          {
            if (width <= height)
              {
                ngx = private->n_guides;
                ngy = ngx * MAX (1, height / width);
              }
            else
              {
                ngy = private->n_guides;
                ngx = ngy * MAX (1, width / height);
              }
          }
        else /* PICMAN_GUIDES_SPACING */
          {
            gint grid_size = MAX (2, private->n_guides);

            ngx = width  / grid_size;
            ngy = height / grid_size;
          }

        for (i = 1; i <= ngx; i++)
          {
            gdouble x = private->x1 + (((gdouble) i) / (ngx + 1) *
                                       (private->x2 - private->x1));

            draw_line (cr, item, &private->transform,
                       x, private->y1,
                       x, private->y2);
          }

        for (i = 1; i <= ngy; i++)
          {
            gdouble y = private->y1 + (((gdouble) i) / (ngy + 1) *
                                       (private->y2 - private->y1));

            draw_line (cr, item, &private->transform,
                       private->x1, y,
                       private->x2, y);
          }
      }
    }

  _picman_canvas_item_stroke (item, cr);
}

static cairo_region_t *
picman_canvas_transform_guides_get_extents (PicmanCanvasItem *item)
{
  gdouble               x1, y1;
  gdouble               x2, y2;
  gdouble               x3, y3;
  gdouble               x4, y4;
  cairo_rectangle_int_t extents;

  picman_canvas_transform_guides_transform (item,
                                          &x1, &y1,
                                          &x2, &y2,
                                          &x3, &y3,
                                          &x4, &y4);

  picman_canvas_item_transform_xy_f (item, x1, y1, &x1, &y1);
  picman_canvas_item_transform_xy_f (item, x2, y2, &x2, &y2);
  picman_canvas_item_transform_xy_f (item, x3, y3, &x3, &y3);
  picman_canvas_item_transform_xy_f (item, x4, y4, &x4, &y4);

  extents.x      = (gint) floor (MIN4 (x1, x2, x3, x4) - 1.5);
  extents.y      = (gint) floor (MIN4 (y1, y2, y3, y4) - 1.5);
  extents.width  = (gint) ceil  (MAX4 (x1, x2, x3, x4) + 1.5);
  extents.height = (gint) ceil  (MAX4 (y1, y2, y3, y4) + 1.5);

  extents.width  -= extents.x;
  extents.height -= extents.y;

  return cairo_region_create_rectangle (&extents);
}

PicmanCanvasItem *
picman_canvas_transform_guides_new (PicmanDisplayShell  *shell,
                                  const PicmanMatrix3 *transform,
                                  gdouble            x1,
                                  gdouble            y1,
                                  gdouble            x2,
                                  gdouble            y2,
                                  PicmanGuidesType     type,
                                  gint               n_guides)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_TRANSFORM_GUIDES,
                       "shell",     shell,
                       "transform", transform,
                       "x1",        x1,
                       "y1",        y1,
                       "x2",        x2,
                       "y2",        y2,
                       "type",      type,
                       "n-guides",  n_guides,
                       NULL);
}

void
picman_canvas_transform_guides_set (PicmanCanvasItem    *guides,
                                  const PicmanMatrix3 *transform,
                                  PicmanGuidesType     type,
                                  gint               n_guides)
{
  g_return_if_fail (PICMAN_IS_CANVAS_TRANSFORM_GUIDES (guides));

  picman_canvas_item_begin_change (guides);

  g_object_set (guides,
                "transform", transform,
                "type",      type,
                "n-guides",  n_guides,
                NULL);

  picman_canvas_item_end_change (guides);
}
