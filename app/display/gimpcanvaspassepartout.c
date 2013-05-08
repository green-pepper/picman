/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvaspassepartout.c
 * Copyright (C) 2010 Sven Neumann <sven@picman.org>
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

#include "display-types.h"

#include "picmancanvas-style.h"
#include "picmancanvaspassepartout.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-scale.h"


/*  local function prototypes  */

static void             picman_canvas_passe_partout_draw        (PicmanCanvasItem *item,
                                                               cairo_t        *cr);
static cairo_region_t * picman_canvas_passe_partout_get_extents (PicmanCanvasItem *item);
static void             picman_canvas_passe_partout_fill        (PicmanCanvasItem *item,
                                                               cairo_t        *cr);


G_DEFINE_TYPE (PicmanCanvasPassePartout, picman_canvas_passe_partout,
               PICMAN_TYPE_CANVAS_RECTANGLE)

#define parent_class picman_canvas_passe_partout_parent_class


static void
picman_canvas_passe_partout_class_init (PicmanCanvasPassePartoutClass *klass)
{
  PicmanCanvasItemClass *item_class = PICMAN_CANVAS_ITEM_CLASS (klass);

  item_class->draw        = picman_canvas_passe_partout_draw;
  item_class->get_extents = picman_canvas_passe_partout_get_extents;
  item_class->fill        = picman_canvas_passe_partout_fill;
}

static void
picman_canvas_passe_partout_init (PicmanCanvasPassePartout *passe_partout)
{
}

static void
picman_canvas_passe_partout_draw (PicmanCanvasItem *item,
                                cairo_t        *cr)
{
  PicmanDisplayShell *shell = picman_canvas_item_get_shell (item);
  gint              w, h;

  picman_display_shell_scale_get_image_size (shell, &w, &h);
  cairo_rectangle (cr, - shell->offset_x, - shell->offset_y, w, h);

  PICMAN_CANVAS_ITEM_CLASS (parent_class)->draw (item, cr);
}

static cairo_region_t *
picman_canvas_passe_partout_get_extents (PicmanCanvasItem *item)
{
  PicmanDisplayShell      *shell = picman_canvas_item_get_shell (item);
  cairo_rectangle_int_t  rectangle;
  cairo_region_t        *inner;
  cairo_region_t        *outer;

  rectangle.x = - shell->offset_x;
  rectangle.y = - shell->offset_y;
  picman_display_shell_scale_get_image_size (shell,
                                           &rectangle.width,
                                           &rectangle.height);

  outer = cairo_region_create_rectangle (&rectangle);

  inner = PICMAN_CANVAS_ITEM_CLASS (parent_class)->get_extents (item);

  cairo_region_subtract (outer, inner);

  return outer;
}

static void
picman_canvas_passe_partout_fill (PicmanCanvasItem *item,
                                cairo_t        *cr)
{
  cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_clip (cr);

  picman_canvas_set_passe_partout_style (picman_canvas_item_get_canvas (item), cr);
  cairo_paint (cr);
}

PicmanCanvasItem *
picman_canvas_passe_partout_new (PicmanDisplayShell *shell,
                               gdouble           x,
                               gdouble           y,
                               gdouble           width,
                               gdouble           height)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS_PASSE_PARTOUT,
                       "shell", shell,
                       "x",      x,
                       "y",      y,
                       "width",  width,
                       "height", height,
                       "filled", TRUE,
                       NULL);
}
