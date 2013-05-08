/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrenderergradient.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmangradient.h"

#include "picmanviewrenderergradient.h"


static void   picman_view_renderer_gradient_set_context (PicmanViewRenderer *renderer,
                                                       PicmanContext      *context);
static void   picman_view_renderer_gradient_invalidate  (PicmanViewRenderer *renderer);
static void   picman_view_renderer_gradient_render      (PicmanViewRenderer *renderer,
                                                       GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererGradient, picman_view_renderer_gradient,
               PICMAN_TYPE_VIEW_RENDERER);

#define parent_class picman_view_renderer_gradient_parent_class


static void
picman_view_renderer_gradient_class_init (PicmanViewRendererGradientClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->set_context = picman_view_renderer_gradient_set_context;
  renderer_class->invalidate  = picman_view_renderer_gradient_invalidate;
  renderer_class->render      = picman_view_renderer_gradient_render;
}

static void
picman_view_renderer_gradient_init (PicmanViewRendererGradient *renderer)
{
  renderer->left    = 0.0;
  renderer->right   = 1.0;
  renderer->reverse = FALSE;
}

static void
picman_view_renderer_gradient_fg_bg_changed (PicmanContext      *context,
                                           const PicmanRGB    *color,
                                           PicmanViewRenderer *renderer)
{
#if 0
  g_printerr ("%s: invalidating %s\n", G_STRFUNC,
              picman_object_get_name (renderer->viewable));
#endif

  picman_view_renderer_invalidate (renderer);
}

static void
picman_view_renderer_gradient_set_context (PicmanViewRenderer *renderer,
                                         PicmanContext      *context)
{
  PicmanViewRendererGradient *rendergrad;

  rendergrad = PICMAN_VIEW_RENDERER_GRADIENT (renderer);

  if (renderer->context && rendergrad->has_fg_bg_segments)
    {
      g_signal_handlers_disconnect_by_func (renderer->context,
                                            picman_view_renderer_gradient_fg_bg_changed,
                                            renderer);
    }

  PICMAN_VIEW_RENDERER_CLASS (parent_class)->set_context (renderer, context);

  if (renderer->context && rendergrad->has_fg_bg_segments)
    {
      g_signal_connect (renderer->context, "foreground-changed",
                        G_CALLBACK (picman_view_renderer_gradient_fg_bg_changed),
                        renderer);
      g_signal_connect (renderer->context, "background-changed",
                        G_CALLBACK (picman_view_renderer_gradient_fg_bg_changed),
                        renderer);

      picman_view_renderer_gradient_fg_bg_changed (renderer->context,
                                                 NULL,
                                                 renderer);
    }
}

static void
picman_view_renderer_gradient_invalidate (PicmanViewRenderer *renderer)
{
  PicmanViewRendererGradient *rendergrad;
  gboolean                  has_fg_bg_segments = FALSE;

  rendergrad = PICMAN_VIEW_RENDERER_GRADIENT (renderer);

  if (renderer->viewable)
    has_fg_bg_segments =
      picman_gradient_has_fg_bg_segments (PICMAN_GRADIENT (renderer->viewable));

  if (rendergrad->has_fg_bg_segments != has_fg_bg_segments)
    {
      if (renderer->context)
        {
          if (rendergrad->has_fg_bg_segments)
            {
              g_signal_handlers_disconnect_by_func (renderer->context,
                                                    picman_view_renderer_gradient_fg_bg_changed,
                                                    renderer);
            }
          else
            {
              g_signal_connect (renderer->context, "foreground-changed",
                                G_CALLBACK (picman_view_renderer_gradient_fg_bg_changed),
                                renderer);
              g_signal_connect (renderer->context, "background-changed",
                                G_CALLBACK (picman_view_renderer_gradient_fg_bg_changed),
                                renderer);
            }
        }

      rendergrad->has_fg_bg_segments = has_fg_bg_segments;
    }

  PICMAN_VIEW_RENDERER_CLASS (parent_class)->invalidate (renderer);
}

static void
picman_view_renderer_gradient_render (PicmanViewRenderer *renderer,
                                    GtkWidget        *widget)
{
  PicmanViewRendererGradient *rendergrad = PICMAN_VIEW_RENDERER_GRADIENT (renderer);
  PicmanGradient             *gradient   = PICMAN_GRADIENT (renderer->viewable);
  PicmanGradientSegment      *seg        = NULL;
  guchar                   *buf;
  guchar                   *dest;
  gint                      dest_stride;
  gint                      x;
  gint                      y;
  gdouble                   dx, cur_x;
  PicmanRGB                   color;

  buf   = g_alloca (4 * renderer->width);
  dx    = (rendergrad->right - rendergrad->left) / (renderer->width - 1);
  cur_x = rendergrad->left;

  for (x = 0, dest = buf; x < renderer->width; x++, dest += 4)
    {
      guchar r, g, b, a;

      seg = picman_gradient_get_color_at (gradient, renderer->context, seg,
                                        cur_x, rendergrad->reverse, &color);
      cur_x += dx;

      picman_rgba_get_uchar (&color, &r, &g, &b, &a);

      PICMAN_CAIRO_ARGB32_SET_PIXEL (dest, r, g, b, a);
    }

  if (! renderer->surface)
    renderer->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                    renderer->width,
                                                    renderer->height);

  cairo_surface_flush (renderer->surface);

  dest        = cairo_image_surface_get_data (renderer->surface);
  dest_stride = cairo_image_surface_get_stride (renderer->surface);

  for (y = 0; y < renderer->height; y++, dest += dest_stride)
    {
      memcpy (dest, buf, renderer->width * 4);
    }

  cairo_surface_mark_dirty (renderer->surface);

  renderer->needs_render = FALSE;
}

void
picman_view_renderer_gradient_set_offsets (PicmanViewRendererGradient *renderer,
                                         gdouble                   left,
                                         gdouble                   right)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER_GRADIENT (renderer));

  left  = CLAMP (left, 0.0, 1.0);
  right = CLAMP (right, left, 1.0);

  if (left != renderer->left || right != renderer->right)
    {
      renderer->left  = left;
      renderer->right = right;

      picman_view_renderer_invalidate (PICMAN_VIEW_RENDERER (renderer));
    }
}

void
picman_view_renderer_gradient_set_reverse (PicmanViewRendererGradient *renderer,
                                         gboolean                  reverse)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER_GRADIENT (renderer));

  if (reverse != renderer->reverse)
    {
      renderer->reverse = reverse ? TRUE : FALSE;

      picman_view_renderer_invalidate (PICMAN_VIEW_RENDERER (renderer));
      picman_view_renderer_update (PICMAN_VIEW_RENDERER (renderer));
    }
}
