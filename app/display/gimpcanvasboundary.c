/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasboundary.c
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
#include "core/picmanboundary.h"
#include "core/picmanparamspecs.h"

#include "picmancanvasboundary.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_SEGS,
  PROP_TRANSFORM,
  PROP_OFFSET_X,
  PROP_OFFSET_Y
};


typedef struct _PicmanCanvasBoundaryPrivate PicmanCanvasBoundaryPrivate;

struct _PicmanCanvasBoundaryPrivate
{
  PicmanBoundSeg *segs;
  gint          n_segs;
  PicmanMatrix3  *transform;
  gdouble       offset_x;
  gdouble       offset_y;
};

#define GET_PRIVATE(boundary) \
        G_TYPE_INSTANCE_GET_PRIVATE (boundary, \
                                     PICMAN_TYPE_CANVAS_BOUNDARY, \
                                     PicmanCanvasBoundaryPrivate)


/*  local function prototypes  */

static void             picman_canvas_boundary_finalize     (GObject        *object);
static void             picman_canvas_boundary_set_property (GObject        *object,
                                                           guint           property_id,
                                                           const GValue   *value,
                                                           GParamSpec     *pspec);
static void             picman_canvas_boundary_get_property (GObject        *object,
                                                           guint           property_id,
                                                           GValue         *value,
                                                           GParamSpec     *pspec);
static void             picman_canvas_boundary_draw         (PicmanCanvasItem *item,
                                                           cairo_t        *cr);
static cairo_region_t * picman_canvas_boundary_get_extents  (PicmanCanvasItem *item);


G_DEFINE_TYPE (PicmanCanvasBoundary, picman_canvas_boundary,
               PICMAN_TYPE_CANVAS_ITEM)

#define parent_class picman_canvas_boundary_parent_class


static void
picman_canvas_boundary_class_init (PicmanCanvasBoundaryClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->finalize     = picman_canvas_boundary_finalize;
  object_class->set_property = picman_canvas_boundary_set_property;
  object_class->get_property = picman_canvas_boundary_get_property;

  item_class->draw           = picman_canvas_boundary_draw;
  item_class->get_extents    = picman_canvas_boundary_get_extents;

  g_object_class_install_property (object_class, PROP_SEGS,
                                   picman_param_spec_array ("segs", NULL, NULL,
                                                          PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_TRANSFORM,
                                   g_param_spec_pointer ("transform", NULL, NULL,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_OFFSET_X,
                                   g_param_spec_double ("offset-x", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_OFFSET_Y,
                                   g_param_spec_double ("offset-y", NULL, NULL,
                                                        -PICMAN_MAX_IMAGE_SIZE,
                                                        PICMAN_MAX_IMAGE_SIZE, 0,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasBoundaryPrivate));
}

static void
picman_canvas_boundary_init (PicmanCanvasBoundary *boundary)
{
  picman_canvas_item_set_line_cap (PICMAN_CANVAS_ITEM (boundary),
                                 CAIRO_LINE_CAP_SQUARE);
}

static void
picman_canvas_boundary_finalize (GObject *object)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (object);

  if (private->segs)
    {
      g_free (private->segs);
      private->segs = NULL;
      private->n_segs = 0;
    }

  if (private->transform)
    {
      g_free (private->transform);
      private->transform = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_canvas_boundary_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_SEGS:
      break;
    case PROP_TRANSFORM:
      {
        PicmanMatrix3 *transform = g_value_get_pointer (value);
        if (private->transform)
          g_free (private->transform);
        if (transform)
          private->transform = g_memdup (transform, sizeof (PicmanMatrix3));
        else
          private->transform = NULL;
      }
      break;
    case PROP_OFFSET_X:
      private->offset_x = g_value_get_double (value);
      break;
    case PROP_OFFSET_Y:
      private->offset_y = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_boundary_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_SEGS:
      break;
    case PROP_TRANSFORM:
      g_value_set_pointer (value, private->transform);
      break;
    case PROP_OFFSET_X:
      g_value_set_double (value, private->offset_x);
      break;
    case PROP_OFFSET_Y:
      g_value_set_double (value, private->offset_y);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_boundary_transform (PicmanCanvasItem *item,
                                PicmanSegment    *segs)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (item);
  gint                       i;

  if (private->transform)
    {
      for (i = 0; i < private->n_segs; i++)
        {
          gdouble tx, ty;

          picman_matrix3_transform_point (private->transform,
                                        private->segs[i].x1,
                                        private->segs[i].y1,
                                        &tx, &ty);
          picman_canvas_item_transform_xy (item,
                                         tx + private->offset_x,
                                         ty + private->offset_y,
                                         &segs[i].x1, &segs[i].y1);

          picman_matrix3_transform_point (private->transform,
                                        private->segs[i].x2,
                                        private->segs[i].y2,
                                        &tx, &ty);
          picman_canvas_item_transform_xy (item,
                                         tx + private->offset_x,
                                         ty + private->offset_y,
                                         &segs[i].x2, &segs[i].y2);
        }
    }
  else
    {
      for (i = 0; i < private->n_segs; i++)
        {
          picman_canvas_item_transform_xy (item,
                                         private->segs[i].x1 + private->offset_x,
                                         private->segs[i].y1 + private->offset_y,
                                         &segs[i].x1,
                                         &segs[i].y1);
          picman_canvas_item_transform_xy (item,
                                         private->segs[i].x2 + private->offset_x,
                                         private->segs[i].y2 + private->offset_y,
                                         &segs[i].x2,
                                         &segs[i].y2);

          /*  If this segment is a closing segment && the segments lie inside
           *  the region, OR if this is an opening segment and the segments
           *  lie outside the region...
           *  we need to transform it by one display pixel
           */
          if (! private->segs[i].open)
            {
              /*  If it is vertical  */
              if (segs[i].x1 == segs[i].x2)
                {
                  segs[i].x1 -= 1;
                  segs[i].x2 -= 1;
                }
              else
                {
                  segs[i].y1 -= 1;
                  segs[i].y2 -= 1;
                }
            }
        }
    }
}

static void
picman_canvas_boundary_draw (PicmanCanvasItem *item,
                           cairo_t        *cr)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (item);
  PicmanSegment               *segs;

  segs = g_new0 (PicmanSegment, private->n_segs);

  picman_canvas_boundary_transform (item, segs);

  picman_cairo_add_segments (cr, segs, private->n_segs);

  _picman_canvas_item_stroke (item, cr);

  g_free (segs);
}

static cairo_region_t *
picman_canvas_boundary_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasBoundaryPrivate *private = GET_PRIVATE (item);
  cairo_rectangle_int_t      rectangle;
  PicmanSegment               *segs;
  gint                       x1, y1, x2, y2;
  gint                       i;

  segs = g_new0 (PicmanSegment, private->n_segs);

  picman_canvas_boundary_transform (item, segs);

  x1 = MIN (segs[0].x1, segs[0].x2);
  y1 = MIN (segs[0].y1, segs[0].y2);
  x2 = MAX (segs[0].x1, segs[0].x2);
  y2 = MAX (segs[0].y1, segs[0].y2);

  for (i = 1; i < private->n_segs; i++)
    {
      gint x3 = MIN (segs[i].x1, segs[i].x2);
      gint y3 = MIN (segs[i].y1, segs[i].y2);
      gint x4 = MAX (segs[i].x1, segs[i].x2);
      gint y4 = MAX (segs[i].y1, segs[i].y2);

      x1 = MIN (x1, x3);
      y1 = MIN (y1, y3);
      x2 = MAX (x2, x4);
      y2 = MAX (y2, y4);
    }

  g_free (segs);

  rectangle.x      = x1 - 2;
  rectangle.y      = y1 - 2;
  rectangle.width  = x2 - x1 + 4;
  rectangle.height = y2 - y1 + 4;

  return cairo_region_create_rectangle (&rectangle);
}

PicmanCanvasItem *
picman_canvas_boundary_new (PicmanDisplayShell   *shell,
                          const PicmanBoundSeg *segs,
                          gint                n_segs,
                          PicmanMatrix3        *transform,
                          gdouble             offset_x,
                          gdouble             offset_y)
{
  PicmanCanvasItem            *item;
  PicmanCanvasBoundaryPrivate *private;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  item = g_object_new (PICMAN_TYPE_CANVAS_BOUNDARY,
                       "shell",     shell,
                       "transform", transform,
                       "offset-x",  offset_x,
                       "offset-y",  offset_y,
                       NULL);
  private = GET_PRIVATE (item);

  /* puke */
  private->segs   = g_memdup (segs, n_segs * sizeof (PicmanBoundSeg));
  private->n_segs = n_segs;

  return item;
}
