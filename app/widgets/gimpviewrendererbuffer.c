/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererbuffer.c
 * Copyright (C) 2004-2006 Michael Natterer <mitch@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmantempbuf.h"
#include "core/picmanviewable.h"

#include "picmanviewrendererbuffer.h"


static void   picman_view_renderer_buffer_render (PicmanViewRenderer *renderer,
                                                GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererBuffer, picman_view_renderer_buffer,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_buffer_class_init


static void
picman_view_renderer_buffer_class_init (PicmanViewRendererBufferClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->render = picman_view_renderer_buffer_render;
}

static void
picman_view_renderer_buffer_init (PicmanViewRendererBuffer *renderer)
{
}

static void
picman_view_renderer_buffer_render (PicmanViewRenderer *renderer,
                                  GtkWidget        *widget)
{
  gint         buffer_width;
  gint         buffer_height;
  gint         view_width;
  gint         view_height;
  gboolean     scaling_up;
  PicmanTempBuf *render_buf = NULL;

  picman_viewable_get_size (renderer->viewable, &buffer_width, &buffer_height);

  picman_viewable_calc_preview_size (buffer_width,
                                   buffer_height,
                                   renderer->width,
                                   renderer->height,
                                   TRUE, 1.0, 1.0,
                                   &view_width,
                                   &view_height,
                                   &scaling_up);

  if (scaling_up)
    {
      PicmanTempBuf *temp_buf;

      temp_buf = picman_viewable_get_new_preview (renderer->viewable,
                                                renderer->context,
                                                buffer_width, buffer_height);

      if (temp_buf)
        {
          render_buf = picman_temp_buf_scale (temp_buf, view_width, view_height);

          picman_temp_buf_unref (temp_buf);
        }
    }
  else
    {
      render_buf = picman_viewable_get_new_preview (renderer->viewable,
                                                  renderer->context,
                                                  view_width, view_height);
    }

  if (render_buf)
    {
      picman_view_renderer_render_temp_buf_simple (renderer, render_buf);

      picman_temp_buf_unref (render_buf);
    }
  else /* no preview available */
    {
      const gchar  *stock_id;

      stock_id = picman_viewable_get_stock_id (renderer->viewable);

      picman_view_renderer_render_stock (renderer, widget, stock_id);
    }
}
