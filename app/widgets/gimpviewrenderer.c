/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrenderer.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 * Copyright (C) 2007 Sven Neumann <sven@picman.org>
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmantempbuf.h"
#include "core/picmanviewable.h"

#include "picmanrender.h"
#include "picmanviewrenderer.h"
#include "picmanviewrenderer-utils.h"
#include "picmanwidgets-utils.h"


enum
{
  UPDATE,
  LAST_SIGNAL
};


static void      picman_view_renderer_dispose           (GObject            *object);
static void      picman_view_renderer_finalize          (GObject            *object);

static gboolean  picman_view_renderer_idle_update       (PicmanViewRenderer   *renderer);
static void      picman_view_renderer_real_set_context  (PicmanViewRenderer   *renderer,
                                                       PicmanContext        *context);
static void      picman_view_renderer_real_invalidate   (PicmanViewRenderer   *renderer);
static void      picman_view_renderer_real_draw         (PicmanViewRenderer   *renderer,
                                                       GtkWidget          *widget,
                                                       cairo_t            *cr,
                                                       gint                available_width,
                                                       gint                available_height);
static void      picman_view_renderer_real_render       (PicmanViewRenderer   *renderer,
                                                       GtkWidget          *widget);

static void      picman_view_renderer_size_changed      (PicmanViewRenderer   *renderer,
                                                       PicmanViewable       *viewable);

static cairo_pattern_t *
                 picman_view_renderer_create_background (PicmanViewRenderer   *renderer,
                                                       GtkWidget          *widget);

static void      picman_view_render_temp_buf_to_surface (PicmanViewRenderer   *renderer,
                                                       PicmanTempBuf        *temp_buf,
                                                       gint                temp_buf_x,
                                                       gint                temp_buf_y,
                                                       gint                channel,
                                                       PicmanViewBG          inside_bg,
                                                       PicmanViewBG          outside_bg,
                                                       cairo_surface_t    *surface,
                                                       gint                dest_width,
                                                       gint                dest_height);



G_DEFINE_TYPE (PicmanViewRenderer, picman_view_renderer, G_TYPE_OBJECT)

#define parent_class picman_view_renderer_parent_class

static guint renderer_signals[LAST_SIGNAL] = { 0 };

static PicmanRGB  black_color;
static PicmanRGB  white_color;
static PicmanRGB  green_color;
static PicmanRGB  red_color;


static void
picman_view_renderer_class_init (PicmanViewRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  renderer_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanViewRendererClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose  = picman_view_renderer_dispose;
  object_class->finalize = picman_view_renderer_finalize;

  klass->update          = NULL;
  klass->set_context     = picman_view_renderer_real_set_context;
  klass->invalidate      = picman_view_renderer_real_invalidate;
  klass->draw            = picman_view_renderer_real_draw;
  klass->render          = picman_view_renderer_real_render;

  klass->frame           = NULL;
  klass->frame_left      = 0;
  klass->frame_right     = 0;
  klass->frame_top       = 0;
  klass->frame_bottom    = 0;

  picman_rgba_set (&black_color, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&white_color, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&green_color, 0.0, 0.94, 0.0, PICMAN_OPACITY_OPAQUE);
  picman_rgba_set (&red_color,   1.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);
}

static void
picman_view_renderer_init (PicmanViewRenderer *renderer)
{
  renderer->context       = NULL;

  renderer->viewable_type = G_TYPE_NONE;
  renderer->viewable      = NULL;

  renderer->width         = 0;
  renderer->height        = 0;
  renderer->border_width  = 0;
  renderer->dot_for_dot   = TRUE;
  renderer->is_popup      = FALSE;

  renderer->border_type   = PICMAN_VIEW_BORDER_BLACK;
  renderer->border_color  = black_color;

  renderer->surface       = NULL;
  renderer->pattern       = NULL;
  renderer->pixbuf        = NULL;
  renderer->bg_stock_id   = NULL;

  renderer->size          = -1;
  renderer->needs_render  = TRUE;
  renderer->idle_id       = 0;
}

static void
picman_view_renderer_dispose (GObject *object)
{
  PicmanViewRenderer *renderer = PICMAN_VIEW_RENDERER (object);

  if (renderer->viewable)
    picman_view_renderer_set_viewable (renderer, NULL);

  if (renderer->context)
    picman_view_renderer_set_context (renderer, NULL);

  picman_view_renderer_remove_idle (renderer);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_view_renderer_finalize (GObject *object)
{
  PicmanViewRenderer *renderer = PICMAN_VIEW_RENDERER (object);

  if (renderer->pattern)
    {
      cairo_pattern_destroy (renderer->pattern);
      renderer->pattern = NULL;
    }

  if (renderer->surface)
    {
      cairo_surface_destroy (renderer->surface);
      renderer->surface = NULL;
    }

  if (renderer->pixbuf)
    {
      g_object_unref (renderer->pixbuf);
      renderer->pixbuf = NULL;
    }

  if (renderer->bg_stock_id)
    {
      g_free (renderer->bg_stock_id);
      renderer->bg_stock_id = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static PicmanViewRenderer *
picman_view_renderer_new_internal (PicmanContext *context,
                                 GType        viewable_type,
                                 gboolean     is_popup)
{
  PicmanViewRenderer *renderer;

  renderer = g_object_new (picman_view_renderer_type_from_viewable_type (viewable_type),
                           NULL);

  renderer->viewable_type = viewable_type;
  renderer->is_popup      = is_popup ? TRUE : FALSE;

  if (context)
    picman_view_renderer_set_context (renderer, context);

  return renderer;
}


/*  public functions  */

PicmanViewRenderer *
picman_view_renderer_new (PicmanContext *context,
                        GType        viewable_type,
                        gint         size,
                        gint         border_width,
                        gboolean     is_popup)
{
  PicmanViewRenderer *renderer;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (g_type_is_a (viewable_type, PICMAN_TYPE_VIEWABLE), NULL);
  g_return_val_if_fail (size >  0 &&
                        size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (border_width >= 0 &&
                        border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH, NULL);

  renderer = picman_view_renderer_new_internal (context, viewable_type,
                                              is_popup);

  picman_view_renderer_set_size (renderer, size, border_width);
  picman_view_renderer_remove_idle (renderer);

  return renderer;
}

PicmanViewRenderer *
picman_view_renderer_new_full (PicmanContext *context,
                             GType        viewable_type,
                             gint         width,
                             gint         height,
                             gint         border_width,
                             gboolean     is_popup)
{
  PicmanViewRenderer *renderer;

  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (g_type_is_a (viewable_type, PICMAN_TYPE_VIEWABLE), NULL);
  g_return_val_if_fail (width >  0 &&
                        width <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (height > 0 &&
                        height <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (border_width >= 0 &&
                        border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH, NULL);

  renderer = picman_view_renderer_new_internal (context, viewable_type,
                                              is_popup);

  picman_view_renderer_set_size_full (renderer, width, height, border_width);
  picman_view_renderer_remove_idle (renderer);

  return renderer;
}

void
picman_view_renderer_set_context (PicmanViewRenderer *renderer,
                                PicmanContext      *context)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context != renderer->context)
    {
      PICMAN_VIEW_RENDERER_GET_CLASS (renderer)->set_context (renderer,
                                                            context);

      if (renderer->viewable)
        picman_view_renderer_invalidate (renderer);
    }
}

static void
picman_view_renderer_weak_notify (PicmanViewRenderer *renderer,
                                PicmanViewable     *viewable)
{
  renderer->viewable = NULL;

  picman_view_renderer_update_idle (renderer);
}

void
picman_view_renderer_set_viewable (PicmanViewRenderer *renderer,
                                 PicmanViewable     *viewable)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable));

  if (viewable)
    g_return_if_fail (g_type_is_a (G_TYPE_FROM_INSTANCE (viewable),
                                   renderer->viewable_type));

  if (viewable == renderer->viewable)
    return;

  if (renderer->surface)
    {
      cairo_surface_destroy (renderer->surface);
      renderer->surface = NULL;
    }

  if (renderer->pixbuf)
    {
      g_object_unref (renderer->pixbuf);
      renderer->pixbuf = NULL;
    }

  if (renderer->viewable)
    {
      g_object_weak_unref (G_OBJECT (renderer->viewable),
                           (GWeakNotify) picman_view_renderer_weak_notify,
                           renderer);

      g_signal_handlers_disconnect_by_func (renderer->viewable,
                                            G_CALLBACK (picman_view_renderer_invalidate),
                                            renderer);

      g_signal_handlers_disconnect_by_func (renderer->viewable,
                                            G_CALLBACK (picman_view_renderer_size_changed),
                                            renderer);
    }

  renderer->viewable = viewable;

  if (renderer->viewable)
    {
      g_object_weak_ref (G_OBJECT (renderer->viewable),
                         (GWeakNotify) picman_view_renderer_weak_notify,
                         renderer);

      g_signal_connect_swapped (renderer->viewable,
                                "invalidate-preview",
                                G_CALLBACK (picman_view_renderer_invalidate),
                                renderer);

      g_signal_connect_swapped (renderer->viewable,
                                "size-changed",
                                G_CALLBACK (picman_view_renderer_size_changed),
                                renderer);

      if (renderer->size != -1)
        picman_view_renderer_set_size (renderer, renderer->size,
                                     renderer->border_width);

      picman_view_renderer_invalidate (renderer);
    }
  else
    {
      picman_view_renderer_update_idle (renderer);
    }
}

void
picman_view_renderer_set_size (PicmanViewRenderer *renderer,
                             gint              view_size,
                             gint              border_width)
{
  gint width;
  gint height;

  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (view_size >  0 &&
                    view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);
  g_return_if_fail (border_width >= 0 &&
                    border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH);

  renderer->size = view_size;

  if (renderer->viewable)
    {
      picman_viewable_get_preview_size (renderer->viewable,
                                      view_size,
                                      renderer->is_popup,
                                      renderer->dot_for_dot,
                                      &width, &height);
    }
  else
    {
      width  = view_size;
      height = view_size;
    }

  picman_view_renderer_set_size_full (renderer, width, height, border_width);
}

void
picman_view_renderer_set_size_full (PicmanViewRenderer *renderer,
                                  gint              width,
                                  gint              height,
                                  gint              border_width)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (width >  0 &&
                    width <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);
  g_return_if_fail (height > 0 &&
                    height <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);
  g_return_if_fail (border_width >= 0 &&
                    border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH);

  if (width        != renderer->width  ||
      height       != renderer->height ||
      border_width != renderer->border_width)
    {
      renderer->width        = width;
      renderer->height       = height;
      renderer->border_width = border_width;

      if (renderer->surface)
        {
          cairo_surface_destroy (renderer->surface);
          renderer->surface = NULL;
        }

      if (renderer->viewable)
        picman_view_renderer_invalidate (renderer);
    }
}

void
picman_view_renderer_set_dot_for_dot (PicmanViewRenderer *renderer,
                                    gboolean             dot_for_dot)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (dot_for_dot != renderer->dot_for_dot)
    {
      renderer->dot_for_dot = dot_for_dot ? TRUE: FALSE;

      if (renderer->size != -1)
        picman_view_renderer_set_size (renderer, renderer->size,
                                     renderer->border_width);

      picman_view_renderer_invalidate (renderer);
    }
}

void
picman_view_renderer_set_border_type (PicmanViewRenderer   *renderer,
                                    PicmanViewBorderType  border_type)
{
  PicmanRGB *border_color = &black_color;

  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  renderer->border_type = border_type;

  switch (border_type)
    {
    case PICMAN_VIEW_BORDER_BLACK:
      border_color = &black_color;
      break;
    case PICMAN_VIEW_BORDER_WHITE:
      border_color = &white_color;
      break;
    case PICMAN_VIEW_BORDER_GREEN:
      border_color = &green_color;
      break;
    case PICMAN_VIEW_BORDER_RED:
      border_color = &red_color;
      break;
    }

  picman_view_renderer_set_border_color (renderer, border_color);
}

void
picman_view_renderer_set_border_color (PicmanViewRenderer *renderer,
                                     const PicmanRGB    *color)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (color != NULL);

  if (picman_rgb_distance (&renderer->border_color, color))
    {
      renderer->border_color = *color;

      picman_view_renderer_update_idle (renderer);
    }
}

void
picman_view_renderer_set_background (PicmanViewRenderer *renderer,
                                   const gchar      *stock_id)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (renderer->bg_stock_id)
    g_free (renderer->bg_stock_id);

  renderer->bg_stock_id = g_strdup (stock_id);

  if (renderer->pattern)
    {
      g_object_unref (renderer->pattern);
      renderer->pattern = NULL;
    }
}

void
picman_view_renderer_invalidate (PicmanViewRenderer *renderer)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (renderer->idle_id)
    {
      g_source_remove (renderer->idle_id);
      renderer->idle_id = 0;
    }

  PICMAN_VIEW_RENDERER_GET_CLASS (renderer)->invalidate (renderer);

  renderer->idle_id =
    g_idle_add_full (PICMAN_VIEWABLE_PRIORITY_IDLE,
                     (GSourceFunc) picman_view_renderer_idle_update,
                     renderer, NULL);
}

void
picman_view_renderer_update (PicmanViewRenderer *renderer)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (renderer->idle_id)
    {
      g_source_remove (renderer->idle_id);
      renderer->idle_id = 0;
    }

  g_signal_emit (renderer, renderer_signals[UPDATE], 0);
}

void
picman_view_renderer_update_idle (PicmanViewRenderer *renderer)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (renderer->idle_id)
    g_source_remove (renderer->idle_id);

  renderer->idle_id =
    g_idle_add_full (PICMAN_VIEWABLE_PRIORITY_IDLE,
                     (GSourceFunc) picman_view_renderer_idle_update,
                     renderer, NULL);
}

void
picman_view_renderer_remove_idle (PicmanViewRenderer *renderer)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));

  if (renderer->idle_id)
    {
      g_source_remove (renderer->idle_id);
      renderer->idle_id = 0;
    }
}

void
picman_view_renderer_draw (PicmanViewRenderer *renderer,
                         GtkWidget        *widget,
                         cairo_t          *cr,
                         gint              available_width,
                         gint              available_height)
{
  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (cr != NULL);

  if (G_UNLIKELY (renderer->context == NULL))
    g_warning ("%s: renderer->context is NULL", G_STRFUNC);

  if (! gtk_widget_is_drawable (widget))
    return;

  if (renderer->viewable)
    {
      cairo_save (cr);

      PICMAN_VIEW_RENDERER_GET_CLASS (renderer)->draw (renderer, widget, cr,
                                                     available_width,
                                                     available_height);

      cairo_restore (cr);
    }
  else
    {
      PicmanViewableClass *viewable_class;

      viewable_class = g_type_class_ref (renderer->viewable_type);

      picman_view_renderer_render_stock (renderer,
                                       widget,
                                       viewable_class->default_stock_id);

      g_type_class_unref (viewable_class);

      picman_view_renderer_real_draw (renderer, widget, cr,
                                    available_width,
                                    available_height);
    }

  if (renderer->border_width > 0)
    {
      gint    width  = renderer->width  + renderer->border_width;
      gint    height = renderer->height + renderer->border_width;
      gdouble x, y;

      cairo_set_line_width (cr, renderer->border_width);
      cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
      picman_cairo_set_source_rgb (cr, &renderer->border_color);

      x = (available_width  - width)  / 2.0;
      y = (available_height - height) / 2.0;

      cairo_rectangle (cr, x, y, width, height);
      cairo_stroke (cr);
    }
}


/*  private functions  */

static gboolean
picman_view_renderer_idle_update (PicmanViewRenderer *renderer)
{
  renderer->idle_id = 0;

  picman_view_renderer_update (renderer);

  return FALSE;
}

static void
picman_view_renderer_real_set_context (PicmanViewRenderer *renderer,
                                     PicmanContext      *context)
{
  if (renderer->context)
    g_object_unref (renderer->context);

  renderer->context = context;

  if (renderer->context)
    g_object_ref (renderer->context);
}

static void
picman_view_renderer_real_invalidate (PicmanViewRenderer *renderer)
{
  renderer->needs_render = TRUE;
}

static void
picman_view_renderer_real_draw (PicmanViewRenderer *renderer,
                              GtkWidget        *widget,
                              cairo_t          *cr,
                              gint              available_width,
                              gint              available_height)
{
  if (renderer->needs_render)
    PICMAN_VIEW_RENDERER_GET_CLASS (renderer)->render (renderer, widget);

  if (renderer->pixbuf)
    {
      gint  width  = gdk_pixbuf_get_width  (renderer->pixbuf);
      gint  height = gdk_pixbuf_get_height (renderer->pixbuf);
      gint  x, y;

      if (renderer->bg_stock_id)
        {
          if (! renderer->pattern)
            {
              renderer->pattern = picman_view_renderer_create_background (renderer,
                                                                        widget);
            }

          cairo_set_source (cr, renderer->pattern);
          cairo_paint (cr);
        }

      x = (available_width  - width)  / 2;
      y = (available_height - height) / 2;

      gdk_cairo_set_source_pixbuf (cr, renderer->pixbuf, x, y);
      cairo_rectangle (cr, x, y, width, height);
      cairo_fill (cr);
    }
  else if (renderer->surface)
    {
      cairo_content_t content  = cairo_surface_get_content (renderer->surface);
      gint            width    = renderer->width;
      gint            height   = renderer->height;
      gint            offset_x = (available_width  - width)  / 2;
      gint            offset_y = (available_height - height) / 2;

      cairo_translate (cr, offset_x, offset_y);

      cairo_rectangle (cr, 0, 0, width, height);

      if (content == CAIRO_CONTENT_COLOR_ALPHA)
        {
          if (! renderer->pattern)
            renderer->pattern =
              picman_cairo_checkerboard_create (cr, PICMAN_CHECK_SIZE_SM,
                                              picman_render_light_check_color (),
                                              picman_render_dark_check_color ());

          cairo_set_source (cr, renderer->pattern);
          cairo_fill_preserve (cr);
        }

      cairo_set_source_surface (cr, renderer->surface, 0, 0);
      cairo_fill (cr);

      cairo_translate (cr, - offset_x, - offset_y);
    }
}

static void
picman_view_renderer_real_render (PicmanViewRenderer *renderer,
                                GtkWidget        *widget)
{
  GdkPixbuf   *pixbuf;
  PicmanTempBuf *temp_buf;
  const gchar *stock_id;

  pixbuf = picman_viewable_get_pixbuf (renderer->viewable,
                                     renderer->context,
                                     renderer->width,
                                     renderer->height);
  if (pixbuf)
    {
      picman_view_renderer_render_pixbuf (renderer, pixbuf);
      return;
    }

  temp_buf = picman_viewable_get_preview (renderer->viewable,
                                        renderer->context,
                                        renderer->width,
                                        renderer->height);
  if (temp_buf)
    {
      picman_view_renderer_render_temp_buf_simple (renderer, temp_buf);
      return;
    }

  stock_id = picman_viewable_get_stock_id (renderer->viewable);
  picman_view_renderer_render_stock (renderer, widget, stock_id);
}

static void
picman_view_renderer_size_changed (PicmanViewRenderer *renderer,
                                 PicmanViewable     *viewable)
{
  if (renderer->size != -1)
    picman_view_renderer_set_size (renderer, renderer->size,
                                 renderer->border_width);

  picman_view_renderer_invalidate (renderer);
}


/*  protected functions  */

void
picman_view_renderer_render_temp_buf_simple (PicmanViewRenderer *renderer,
                                           PicmanTempBuf      *temp_buf)
{
  gint temp_buf_x = 0;
  gint temp_buf_y = 0;
  gint temp_buf_width;
  gint temp_buf_height;

  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (temp_buf != NULL);

  temp_buf_width  = picman_temp_buf_get_width  (temp_buf);
  temp_buf_height = picman_temp_buf_get_height (temp_buf);

  if (temp_buf_width < renderer->width)
    temp_buf_x = (renderer->width - temp_buf_width)  / 2;

  if (temp_buf_height < renderer->height)
    temp_buf_y = (renderer->height - temp_buf_height) / 2;

  picman_view_renderer_render_temp_buf (renderer, temp_buf,
                                      temp_buf_x, temp_buf_y,
                                      -1,
                                      PICMAN_VIEW_BG_CHECKS,
                                      PICMAN_VIEW_BG_WHITE);
}

void
picman_view_renderer_render_temp_buf (PicmanViewRenderer *renderer,
                                    PicmanTempBuf      *temp_buf,
                                    gint              temp_buf_x,
                                    gint              temp_buf_y,
                                    gint              channel,
                                    PicmanViewBG        inside_bg,
                                    PicmanViewBG        outside_bg)
{
  if (renderer->pixbuf)
    {
      g_object_unref (renderer->pixbuf);
      renderer->pixbuf = NULL;
    }

  if (! renderer->surface)
    renderer->surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                    renderer->width,
                                                    renderer->height);

  picman_view_render_temp_buf_to_surface (renderer,
                                        temp_buf,
                                        temp_buf_x,
                                        temp_buf_y,
                                        channel,
                                        inside_bg,
                                        outside_bg,
                                        renderer->surface,
                                        renderer->width,
                                        renderer->height);

  renderer->needs_render = FALSE;
}


void
picman_view_renderer_render_pixbuf (PicmanViewRenderer *renderer,
                                  GdkPixbuf        *pixbuf)
{
  if (renderer->surface)
    {
      cairo_surface_destroy (renderer->surface);
      renderer->surface = NULL;
    }

  g_object_ref (pixbuf);

  if (renderer->pixbuf)
    g_object_unref (renderer->pixbuf);

  renderer->pixbuf = pixbuf;

  renderer->needs_render = FALSE;
}

void
picman_view_renderer_render_stock (PicmanViewRenderer *renderer,
                                 GtkWidget        *widget,
                                 const gchar      *stock_id)
{
  GdkPixbuf   *pixbuf = NULL;
  GtkIconSize  icon_size;

  g_return_if_fail (PICMAN_IS_VIEW_RENDERER (renderer));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (stock_id != NULL);

  if (renderer->pixbuf)
    {
      g_object_unref (renderer->pixbuf);
      renderer->pixbuf = NULL;
    }

  if (renderer->surface)
    {
      cairo_surface_destroy (renderer->surface);
      renderer->surface = NULL;
    }

  icon_size = picman_get_icon_size (widget, stock_id, GTK_ICON_SIZE_INVALID,
                                  renderer->width, renderer->height);

  if (icon_size)
    pixbuf = gtk_widget_render_icon (widget, stock_id, icon_size, NULL);

  if (pixbuf)
    {
      gint  width  = gdk_pixbuf_get_width (pixbuf);
      gint  height = gdk_pixbuf_get_height (pixbuf);

      if (width > renderer->width || height > renderer->height)
        {
          GdkPixbuf *scaled_pixbuf;

          picman_viewable_calc_preview_size (width, height,
                                           renderer->width, renderer->height,
                                           TRUE, 1.0, 1.0,
                                           &width, &height,
                                           NULL);

          scaled_pixbuf = gdk_pixbuf_scale_simple (pixbuf,
                                                   width, height,
                                                   GDK_INTERP_BILINEAR);

          g_object_unref (pixbuf);
          pixbuf = scaled_pixbuf;
        }

      renderer->pixbuf = pixbuf;
    }

  renderer->needs_render = FALSE;
}

static void
picman_view_render_temp_buf_to_surface (PicmanViewRenderer *renderer,
                                      PicmanTempBuf      *temp_buf,
                                      gint              temp_buf_x,
                                      gint              temp_buf_y,
                                      gint              channel,
                                      PicmanViewBG        inside_bg,
                                      PicmanViewBG        outside_bg,
                                      cairo_surface_t  *surface,
                                      gint              surface_width,
                                      gint              surface_height)
{
  cairo_t    *cr;
  gint        x, y;
  gint        width, height;
  const Babl *temp_buf_format;
  gint        temp_buf_width;
  gint        temp_buf_height;

  g_return_if_fail (temp_buf != NULL);
  g_return_if_fail (surface != NULL);

  temp_buf_format = picman_temp_buf_get_format (temp_buf);
  temp_buf_width  = picman_temp_buf_get_width  (temp_buf);
  temp_buf_height = picman_temp_buf_get_height (temp_buf);

  /*  Here are the different cases this functions handles correctly:
   *  1)  Offset temp_buf which does not necessarily cover full image area
   *  2)  Color conversion of temp_buf if it is gray and image is color
   *  3)  Background check buffer for transparent temp_bufs
   *  4)  Using the optional "channel" argument, one channel can be extracted
   *      from a multi-channel temp_buf and composited as a grayscale
   *  Prereqs:
   *  1)  Grayscale temp_bufs have bytes == {1, 2}
   *  2)  Color temp_bufs have bytes == {3, 4}
   *  3)  If image is gray, then temp_buf should have bytes == {1, 2}
   */

  cr = cairo_create (surface);

  if (outside_bg == PICMAN_VIEW_BG_CHECKS ||
      inside_bg  == PICMAN_VIEW_BG_CHECKS)
    {
      if (! renderer->pattern)
        renderer->pattern =
          picman_cairo_checkerboard_create (cr, PICMAN_CHECK_SIZE_SM,
                                          picman_render_light_check_color (),
                                          picman_render_dark_check_color ());
    }

  switch (outside_bg)
    {
    case PICMAN_VIEW_BG_CHECKS:
      cairo_set_source (cr, renderer->pattern);
      break;

    case PICMAN_VIEW_BG_WHITE:
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
      break;
    }

  cairo_paint (cr);

  if (! picman_rectangle_intersect (0, 0,
                                  surface_width, surface_height,
                                  temp_buf_x, temp_buf_y,
                                  temp_buf_width, temp_buf_height,
                                  &x, &y,
                                  &width, &height))
    {
      cairo_destroy (cr);
      return;
    }

  if (inside_bg != outside_bg &&
      babl_format_has_alpha (temp_buf_format) && channel == -1)
    {
      cairo_rectangle (cr, x, y, width, height);

      switch (inside_bg)
        {
        case PICMAN_VIEW_BG_CHECKS:
          cairo_set_source (cr, renderer->pattern);
          break;

        case PICMAN_VIEW_BG_WHITE:
          cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
          break;
        }

      cairo_fill (cr);
    }

  if (babl_format_has_alpha (temp_buf_format) && channel == -1)
    {
      GeglBuffer      *src_buffer;
      GeglBuffer      *dest_buffer;
      cairo_surface_t *alpha_surface;

      alpha_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                  width, height);

      src_buffer  = picman_temp_buf_create_buffer (temp_buf);
      dest_buffer = picman_cairo_surface_create_buffer (alpha_surface);

      gegl_buffer_copy (src_buffer,
                        GEGL_RECTANGLE (x - temp_buf_x,
                                        y - temp_buf_y,
                                        width, height),
                        dest_buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));

      g_object_unref (src_buffer);
      g_object_unref (dest_buffer);

      cairo_surface_mark_dirty (alpha_surface);

      cairo_translate (cr, x, y);
      cairo_rectangle (cr, 0, 0, width, height);
      cairo_set_source_surface (cr, alpha_surface, 0, 0);
      cairo_fill (cr);

      cairo_surface_destroy (alpha_surface);
    }
  else if (channel == -1)
    {
      GeglBuffer *src_buffer;
      GeglBuffer *dest_buffer;

      cairo_surface_flush (surface);

      src_buffer  = picman_temp_buf_create_buffer (temp_buf);
      dest_buffer = picman_cairo_surface_create_buffer (surface);

      gegl_buffer_copy (src_buffer,
                        GEGL_RECTANGLE (x - temp_buf_x,
                                        y - temp_buf_y,
                                        width, height),
                        dest_buffer,
                        GEGL_RECTANGLE (x, y, 0, 0));

      g_object_unref (src_buffer);
      g_object_unref (dest_buffer);

      cairo_surface_mark_dirty (surface);
    }
  else
    {
      const Babl   *fish;
      const guchar *src;
      guchar       *dest;
      gint          dest_stride;
      gint          bytes;
      gint          rowstride;
      gint          i;

      cairo_surface_flush (surface);

      bytes     = babl_format_get_bytes_per_pixel (temp_buf_format);
      rowstride = temp_buf_width * bytes;

      src = picman_temp_buf_get_data (temp_buf) + ((y - temp_buf_y) * rowstride +
                                                 (x - temp_buf_x) * bytes);

      dest        = cairo_image_surface_get_data (surface);
      dest_stride = cairo_image_surface_get_stride (surface);

      dest += y * dest_stride + x * 4;

      fish = babl_fish (temp_buf_format,
                        babl_format ("cairo-RGB24"));

      for (i = y; i < (y + height); i++)
        {
          const guchar *s = src;
          guchar       *d = dest;
          gint          j;

          for (j = x; j < (x + width); j++, d += 4, s += bytes)
            {
              if (bytes > 2)
                {
                  guchar pixel[4] = { s[channel], s[channel], s[channel], 255 };

                  babl_process (fish, pixel, d, 1);
                }
              else
                {
                  guchar pixel[2] = { s[channel], 255 };

                  babl_process (fish, pixel, d, 1);
                }
            }

          src += rowstride;
          dest += dest_stride;
        }

      cairo_surface_mark_dirty (surface);
    }

  cairo_destroy (cr);
}

/* This function creates a background pattern from a stock icon
 * if renderer->bg_stock_id is set.
 */
static cairo_pattern_t *
picman_view_renderer_create_background (PicmanViewRenderer *renderer,
                                      GtkWidget        *widget)
{
  cairo_pattern_t *pattern = NULL;

  if (renderer->bg_stock_id)
    {
      GdkPixbuf *pixbuf = gtk_widget_render_icon (widget,
                                                  renderer->bg_stock_id,
                                                  GTK_ICON_SIZE_DIALOG, NULL);

      if (pixbuf)
        {
          cairo_surface_t *surface;

          surface = picman_cairo_surface_create_from_pixbuf (pixbuf);

          g_object_unref (pixbuf);

          pattern = cairo_pattern_create_for_surface (surface);
          cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

          cairo_surface_destroy (surface);
        }
    }

  return pattern;
}
