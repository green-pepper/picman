/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrenderervectors.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *                    Simon Budig <simon@picman.org>
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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanimage.h"
#include "core/picmanitem.h"

#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "picmanviewrenderervectors.h"


static void   picman_view_renderer_vectors_draw (PicmanViewRenderer *renderer,
                                               GtkWidget        *widget,
                                               cairo_t          *cr,
                                               gint              available_width,
                                               gint              available_height);


G_DEFINE_TYPE (PicmanViewRendererVectors, picman_view_renderer_vectors,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_vectors_parent_class


static void
picman_view_renderer_vectors_class_init (PicmanViewRendererVectorsClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->draw = picman_view_renderer_vectors_draw;
}

static void
picman_view_renderer_vectors_init (PicmanViewRendererVectors *renderer)
{
}

static void
picman_view_renderer_vectors_draw (PicmanViewRenderer *renderer,
                                 GtkWidget        *widget,
                                 cairo_t          *cr,
                                 gint              available_width,
                                 gint              available_height)
{
  GtkStyle             *style   = gtk_widget_get_style (widget);
  PicmanVectors          *vectors = PICMAN_VECTORS (renderer->viewable);
  const PicmanBezierDesc *desc;

  gdk_cairo_set_source_color (cr, &style->white);

  cairo_translate (cr,
                   (available_width  - renderer->width)  / 2,
                   (available_height - renderer->height) / 2);
  cairo_rectangle (cr, 0, 0, renderer->width, renderer->height);
  cairo_clip_preserve (cr);
  cairo_fill (cr);

  desc = picman_vectors_get_bezier (vectors);

  if (desc)
    {
      gdouble xscale;
      gdouble yscale;

      xscale = ((gdouble) renderer->width /
                (gdouble) picman_item_get_width  (PICMAN_ITEM (vectors)));
      yscale = ((gdouble) renderer->height /
                (gdouble) picman_item_get_height (PICMAN_ITEM (vectors)));

      cairo_scale (cr, xscale, yscale);

      /* determine line width */
      xscale = yscale = 0.5;
      cairo_device_to_user_distance (cr, &xscale, &yscale);

      cairo_set_line_width (cr, MAX (xscale, yscale));
      gdk_cairo_set_source_color (cr, &style->black);

      cairo_append_path (cr, (cairo_path_t *) desc);
      cairo_stroke (cr);
    }
}
