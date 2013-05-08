/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvaslayerboundary.c
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

#include "core/picmanchannel.h"
#include "core/picmanlayer.h"
#include "core/picmanlayer-floating-sel.h"

#include "picmancanvas-style.h"
#include "picmancanvaslayerboundary.h"
#include "picmandisplayshell.h"


enum
{
  PROP_0,
  PROP_LAYER,
  PROP_EDIT_MASK
};


typedef struct _PicmanCanvasLayerBoundaryPrivate PicmanCanvasLayerBoundaryPrivate;

struct _PicmanCanvasLayerBoundaryPrivate
{
  PicmanLayer *layer;
  gboolean   edit_mask;
};

#define GET_PRIVATE(layer_boundary) \
        G_TYPE_INSTANCE_GET_PRIVATE (layer_boundary, \
                                     PICMAN_TYPE_CANVAS_LAYER_BOUNDARY, \
                                     PicmanCanvasLayerBoundaryPrivate)


/*  local function prototypes  */

static void             picman_canvas_layer_boundary_set_property (GObject        *object,
                                                                 guint           property_id,
                                                                 const GValue   *value,
                                                                 GParamSpec     *pspec);
static void             picman_canvas_layer_boundary_get_property (GObject        *object,
                                                                 guint           property_id,
                                                                 GValue         *value,
                                                                 GParamSpec     *pspec);
static void             picman_canvas_layer_boundary_draw         (PicmanCanvasItem *item,
                                                                 cairo_t        *cr);
static cairo_region_t * picman_canvas_layer_boundary_get_extents  (PicmanCanvasItem *item);
static void             picman_canvas_layer_boundary_stroke       (PicmanCanvasItem *item,
                                                                 cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasLayerBoundary, picman_canvas_layer_boundary,
               PICMAN_TYPE_CANVAS_RECTANGLE)

#define parent_class picman_canvas_layer_boundary_parent_class


static void
picman_canvas_layer_boundary_class_init (PicmanCanvasLayerBoundaryClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  PicmanCanvasItemClass *item_class   = PICMAN_CANVAS_ITEM_CLASS (klass);

  object_class->set_property = picman_canvas_layer_boundary_set_property;
  object_class->get_property = picman_canvas_layer_boundary_get_property;

  item_class->draw           = picman_canvas_layer_boundary_draw;
  item_class->get_extents    = picman_canvas_layer_boundary_get_extents;
  item_class->stroke         = picman_canvas_layer_boundary_stroke;

  g_object_class_install_property (object_class, PROP_LAYER,
                                   g_param_spec_object ("layer", NULL, NULL,
                                                        PICMAN_TYPE_LAYER,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_EDIT_MASK,
                                   g_param_spec_boolean ("edit-mask", NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanCanvasLayerBoundaryPrivate));
}

static void
picman_canvas_layer_boundary_init (PicmanCanvasLayerBoundary *layer_boundary)
{
}

static void
picman_canvas_layer_boundary_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  PicmanCanvasLayerBoundaryPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_LAYER:
      private->layer = g_value_get_object (value); /* don't ref */
      break;
    case PROP_EDIT_MASK:
      private->edit_mask = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_layer_boundary_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  PicmanCanvasLayerBoundaryPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_LAYER:
      g_value_set_object (value, private->layer);
      break;
    case PROP_EDIT_MASK:
      g_value_set_boolean (value, private->edit_mask);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_layer_boundary_draw (PicmanCanvasItem *item,
                                 cairo_t        *cr)
{
  PicmanCanvasLayerBoundaryPrivate *private = GET_PRIVATE (item);

  if (private->layer)
    PICMAN_CANVAS_ITEM_CLASS (parent_class)->draw (item, cr);
}

static cairo_region_t *
picman_canvas_layer_boundary_get_extents (PicmanCanvasItem *item)
{
  PicmanCanvasLayerBoundaryPrivate *private = GET_PRIVATE (item);

  if (private->layer)
    return PICMAN_CANVAS_ITEM_CLASS (parent_class)->get_extents (item);

  return NULL;
}

static void
picman_canvas_layer_boundary_stroke (PicmanCanvasItem *item,
                                   cairo_t        *cr)
{
  PicmanCanvasLayerBoundaryPrivate *private = GET_PRIVATE (item);

  picman_canvas_set_layer_style (picman_canvas_item_get_canvas (item), cr,
                               private->layer);
  cairo_stroke (cr);
}

PicmanCanvasItem *
picman_canvas_layer_boundary_new (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_LAYER_BOUNDARY,
                       "shell", shell,
                       NULL);
}

void
picman_canvas_layer_boundary_set_layer (PicmanCanvasLayerBoundary *boundary,
                                      PicmanLayer               *layer)
{
  PicmanCanvasLayerBoundaryPrivate *private;

  g_return_if_fail (PICMAN_IS_CANVAS_LAYER_BOUNDARY (boundary));
  g_return_if_fail (layer == NULL || PICMAN_IS_LAYER (layer));

  private = GET_PRIVATE (boundary);

  if (layer && picman_layer_is_floating_sel (layer))
    {
      PicmanDrawable *drawable;

      drawable = picman_layer_get_floating_sel_drawable (layer);

      if (PICMAN_IS_CHANNEL (drawable))
        {
          /*  if the owner drawable is a channel, show no outline  */

          layer = NULL;
        }
      else
        {
          /*  otherwise, set the layer to the owner drawable  */

          layer = PICMAN_LAYER (drawable);
        }
    }

  if (layer != private->layer)
    {
      gboolean edit_mask = FALSE;

      picman_canvas_item_begin_change (PICMAN_CANVAS_ITEM (boundary));

      if (layer)
        {
          PicmanItem *item = PICMAN_ITEM (layer);

          edit_mask = (picman_layer_get_mask (layer) &&
                       picman_layer_get_edit_mask (layer));

          g_object_set (boundary,
                        "x",      (gdouble) picman_item_get_offset_x (item),
                        "y",      (gdouble) picman_item_get_offset_y (item),
                        "width",  (gdouble) picman_item_get_width  (item),
                        "height", (gdouble) picman_item_get_height (item),
                        NULL);
        }

      g_object_set (boundary,
                    "layer",     layer,
                    "edit-mask", edit_mask,
                    NULL);

      picman_canvas_item_end_change (PICMAN_CANVAS_ITEM (boundary));
    }
  else if (layer && layer == private->layer)
    {
      PicmanItem *item = PICMAN_ITEM (layer);
      gint      lx, ly, lw, lh;
      gdouble   x, y, w ,h;
      gboolean  edit_mask;

      lx = picman_item_get_offset_x (item);
      ly = picman_item_get_offset_y (item);
      lw = picman_item_get_width  (item);
      lh = picman_item_get_height (item);

      edit_mask = (picman_layer_get_mask (layer) &&
                   picman_layer_get_edit_mask (layer));

      g_object_get (boundary,
                    "x",      &x,
                    "y",      &y,
                    "width",  &w,
                    "height", &h,
                    NULL);

      if (lx        != (gint) x ||
          ly        != (gint) y ||
          lw        != (gint) w ||
          lh        != (gint) h ||
          edit_mask != private->edit_mask)
        {
          picman_canvas_item_begin_change (PICMAN_CANVAS_ITEM (boundary));

          g_object_set (boundary,
                        "x",         (gdouble) lx,
                        "y",         (gdouble) ly,
                        "width",     (gdouble) lw,
                        "height",    (gdouble) lh,
                        "edit-mask", edit_mask,
                        NULL);

          picman_canvas_item_end_change (PICMAN_CANVAS_ITEM (boundary));
        }
    }
}
