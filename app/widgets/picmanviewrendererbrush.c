/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererbrush.c
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets-types.h"

#include "core/picmanbrushpipe.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmantempbuf.h"

#include "picmanviewrendererbrush.h"


static void   picman_view_renderer_brush_finalize (GObject          *object);
static void   picman_view_renderer_brush_render   (PicmanViewRenderer *renderer,
                                                 GtkWidget        *widget);
static void   picman_view_renderer_brush_draw     (PicmanViewRenderer *renderer,
                                                 GtkWidget        *widget,
                                                 cairo_t          *cr,
                                                 gint              available_width,
                                                 gint              available_height);

static gboolean picman_view_renderer_brush_render_timeout (gpointer    data);


G_DEFINE_TYPE (PicmanViewRendererBrush, picman_view_renderer_brush,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_brush_parent_class


static void
picman_view_renderer_brush_class_init (PicmanViewRendererBrushClass *klass)
{
  GObjectClass          *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  object_class->finalize = picman_view_renderer_brush_finalize;

  renderer_class->render = picman_view_renderer_brush_render;
  renderer_class->draw   = picman_view_renderer_brush_draw;
}

static void
picman_view_renderer_brush_init (PicmanViewRendererBrush *renderer)
{
  renderer->pipe_timeout_id      = 0;
  renderer->pipe_animation_index = 0;
}

static void
picman_view_renderer_brush_finalize (GObject *object)
{
  PicmanViewRendererBrush *renderer = PICMAN_VIEW_RENDERER_BRUSH (object);

  if (renderer->pipe_timeout_id)
    {
      g_source_remove (renderer->pipe_timeout_id);
      renderer->pipe_timeout_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_view_renderer_brush_render (PicmanViewRenderer *renderer,
                                 GtkWidget        *widget)
{
  PicmanViewRendererBrush *renderbrush = PICMAN_VIEW_RENDERER_BRUSH (renderer);
  PicmanTempBuf           *temp_buf;
  gint                   temp_buf_x = 0;
  gint                   temp_buf_y = 0;
  gint                   temp_buf_width;
  gint                   temp_buf_height;

  if (renderbrush->pipe_timeout_id)
    {
      g_source_remove (renderbrush->pipe_timeout_id);
      renderbrush->pipe_timeout_id = 0;
    }

  temp_buf = picman_viewable_get_new_preview (renderer->viewable,
                                            renderer->context,
                                            renderer->width,
                                            renderer->height);

  temp_buf_width  = picman_temp_buf_get_width  (temp_buf);
  temp_buf_height = picman_temp_buf_get_height (temp_buf);

  if (temp_buf_width < renderer->width)
    temp_buf_x = (renderer->width - temp_buf_width) / 2;

  if (temp_buf_height < renderer->height)
    temp_buf_y = (renderer->height - temp_buf_height) / 2;

  if (renderer->is_popup)
    {
      picman_view_renderer_render_temp_buf (renderer, temp_buf,
                                          temp_buf_x, temp_buf_y,
                                          -1,
                                          PICMAN_VIEW_BG_WHITE,
                                          PICMAN_VIEW_BG_WHITE);

      picman_temp_buf_unref (temp_buf);

      if (PICMAN_IS_BRUSH_PIPE (renderer->viewable))
        {
          renderbrush->pipe_animation_index = 0;
          renderbrush->pipe_timeout_id =
            g_timeout_add (300, picman_view_renderer_brush_render_timeout,
                           renderbrush);
        }

      return;
    }

  picman_view_renderer_render_temp_buf (renderer, temp_buf,
                                      temp_buf_x, temp_buf_y,
                                      -1,
                                      PICMAN_VIEW_BG_WHITE,
                                      PICMAN_VIEW_BG_WHITE);

  picman_temp_buf_unref (temp_buf);
}

static gboolean
picman_view_renderer_brush_render_timeout (gpointer data)
{
  PicmanViewRendererBrush *renderbrush = PICMAN_VIEW_RENDERER_BRUSH (data);
  PicmanViewRenderer      *renderer    = PICMAN_VIEW_RENDERER (data);
  PicmanBrushPipe         *brush_pipe;
  PicmanBrush             *brush;
  PicmanTempBuf           *temp_buf;
  gint                   temp_buf_x = 0;
  gint                   temp_buf_y = 0;
  gint                   temp_buf_width;
  gint                   temp_buf_height;

  if (! renderer->viewable)
    {
      renderbrush->pipe_timeout_id      = 0;
      renderbrush->pipe_animation_index = 0;

      return FALSE;
    }

  brush_pipe = PICMAN_BRUSH_PIPE (renderer->viewable);

  renderbrush->pipe_animation_index++;

  if (renderbrush->pipe_animation_index >= brush_pipe->n_brushes)
    renderbrush->pipe_animation_index = 0;

  brush =
    PICMAN_BRUSH (brush_pipe->brushes[renderbrush->pipe_animation_index]);

  temp_buf = picman_viewable_get_new_preview (PICMAN_VIEWABLE (brush),
                                            renderer->context,
                                            renderer->width,
                                            renderer->height);

  temp_buf_width  = picman_temp_buf_get_width  (temp_buf);
  temp_buf_height = picman_temp_buf_get_height (temp_buf);

  if (temp_buf_width < renderer->width)
    temp_buf_x = (renderer->width - temp_buf_width) / 2;

  if (temp_buf_height < renderer->height)
    temp_buf_y = (renderer->height - temp_buf_height) / 2;

  picman_view_renderer_render_temp_buf (renderer, temp_buf,
                                      temp_buf_x, temp_buf_y,
                                      -1,
                                      PICMAN_VIEW_BG_WHITE,
                                      PICMAN_VIEW_BG_WHITE);

  picman_temp_buf_unref (temp_buf);

  picman_view_renderer_update (renderer);

  return TRUE;
}

static void
picman_view_renderer_brush_draw (PicmanViewRenderer *renderer,
                               GtkWidget        *widget,
                               cairo_t          *cr,
                               gint              available_width,
                               gint              available_height)
{
  PICMAN_VIEW_RENDERER_CLASS (parent_class)->draw (renderer, widget, cr,
                                                 available_width,
                                                 available_height);

#define INDICATOR_WIDTH  7
#define INDICATOR_HEIGHT 7

  if (renderer->width  > 2 * INDICATOR_WIDTH &&
      renderer->height > 2 * INDICATOR_HEIGHT)
    {
      gboolean  pipe      = PICMAN_IS_BRUSH_PIPE (renderer->viewable);
      gboolean  generated = PICMAN_IS_BRUSH_GENERATED (renderer->viewable);
      gint      brush_width;
      gint      brush_height;

      if (generated || pipe)
        {
          cairo_move_to (cr, available_width, available_height);
          cairo_rel_line_to (cr, - INDICATOR_WIDTH, 0);
          cairo_rel_line_to (cr, INDICATOR_WIDTH, - INDICATOR_HEIGHT);
          cairo_rel_line_to (cr, 0, INDICATOR_HEIGHT);

          if (pipe)
            cairo_set_source_rgb (cr, 1.0, 0.5, 0.5);
          else
            cairo_set_source_rgb (cr, 0.5, 0.6, 1.0);

          cairo_fill (cr);
        }

      picman_viewable_get_size (renderer->viewable, &brush_width, &brush_height);

      if (renderer->width < brush_width || renderer->height < brush_height)
        {
          cairo_move_to (cr,
                         available_width  - INDICATOR_WIDTH + 1,
                         available_height - INDICATOR_HEIGHT / 2.0);
          cairo_rel_line_to (cr, INDICATOR_WIDTH - 2, 0);

          cairo_move_to (cr,
                         available_width  - INDICATOR_WIDTH / 2.0,
                         available_height - INDICATOR_HEIGHT + 1);
          cairo_rel_line_to (cr, 0, INDICATOR_WIDTH - 2);

          cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
          cairo_set_line_width (cr, 1);
          cairo_stroke (cr);
        }
    }

#undef INDICATOR_WIDTH
#undef INDICATOR_HEIGHT
}
