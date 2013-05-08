/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererimage.c
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanimage.h"
#include "core/picmantempbuf.h"

#include "picmanviewrendererimage.h"


static void   picman_view_renderer_image_render (PicmanViewRenderer *renderer,
                                               GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererImage, picman_view_renderer_image,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_image_parent_class


static void
picman_view_renderer_image_class_init (PicmanViewRendererImageClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->render = picman_view_renderer_image_render;
}

static void
picman_view_renderer_image_init (PicmanViewRendererImage *renderer)
{
  renderer->channel = -1;
}

static void
picman_view_renderer_image_render (PicmanViewRenderer *renderer,
                                 GtkWidget        *widget)
{
  PicmanViewRendererImage *rendererimage = PICMAN_VIEW_RENDERER_IMAGE (renderer);
  PicmanImage             *image         = PICMAN_IMAGE (renderer->viewable);
  const gchar           *stock_id;

  /* The conditions checked here are mostly a hack to hide the fact that
   * we are creating the channel preview from the image preview and turning
   * off visibility of a channel has the side-effect of painting the channel
   * preview all black. See bug #459518 for details.
   */
  if (rendererimage->channel == -1 ||
      (picman_image_get_component_visible (image, rendererimage->channel) &&
       picman_image_get_component_visible (image, PICMAN_ALPHA_CHANNEL)))
    {
      gint         view_width;
      gint         view_height;
      gdouble      xres;
      gdouble      yres;
      gboolean     scaling_up;
      PicmanTempBuf *render_buf = NULL;

      picman_image_get_resolution (image, &xres, &yres);

      picman_viewable_calc_preview_size (picman_image_get_width  (image),
                                       picman_image_get_height (image),
                                       renderer->width,
                                       renderer->height,
                                       renderer->dot_for_dot,
                                       xres,
                                       yres,
                                       &view_width,
                                       &view_height,
                                       &scaling_up);

      if (scaling_up)
        {
          PicmanTempBuf *temp_buf;

          temp_buf = picman_viewable_get_new_preview (renderer->viewable,
                                                    renderer->context,
                                                    picman_image_get_width  (image),
                                                    picman_image_get_height (image));

          if (temp_buf)
            {
              render_buf = picman_temp_buf_scale (temp_buf,
                                                view_width, view_height);
              picman_temp_buf_unref (temp_buf);
            }
        }
      else
        {
          render_buf = picman_viewable_get_new_preview (renderer->viewable,
                                                      renderer->context,
                                                      view_width,
                                                      view_height);
        }

      if (render_buf)
        {
          gint render_buf_x    = 0;
          gint render_buf_y    = 0;
          gint component_index = -1;

          /*  xresolution != yresolution */
          if (view_width > renderer->width || view_height > renderer->height)
            {
              PicmanTempBuf *temp_buf;

              temp_buf = picman_temp_buf_scale (render_buf,
                                              renderer->width, renderer->height);
              picman_temp_buf_unref (render_buf);
              render_buf = temp_buf;
            }

          if (view_width  < renderer->width)
            render_buf_x = (renderer->width  - view_width)  / 2;

          if (view_height < renderer->height)
            render_buf_y = (renderer->height - view_height) / 2;

          if (rendererimage->channel != -1)
            component_index =
              picman_image_get_component_index (image, rendererimage->channel);

          picman_view_renderer_render_temp_buf (renderer, render_buf,
                                              render_buf_x, render_buf_y,
                                              component_index,
                                              PICMAN_VIEW_BG_CHECKS,
                                              PICMAN_VIEW_BG_WHITE);
          picman_temp_buf_unref (render_buf);

          return;
        }
    }

  switch (rendererimage->channel)
    {
    case PICMAN_RED_CHANNEL:     stock_id = PICMAN_STOCK_CHANNEL_RED;     break;
    case PICMAN_GREEN_CHANNEL:   stock_id = PICMAN_STOCK_CHANNEL_GREEN;   break;
    case PICMAN_BLUE_CHANNEL:    stock_id = PICMAN_STOCK_CHANNEL_BLUE;    break;
    case PICMAN_GRAY_CHANNEL:    stock_id = PICMAN_STOCK_CHANNEL_GRAY;    break;
    case PICMAN_INDEXED_CHANNEL: stock_id = PICMAN_STOCK_CHANNEL_INDEXED; break;
    case PICMAN_ALPHA_CHANNEL:   stock_id = PICMAN_STOCK_CHANNEL_ALPHA;   break;

    default:
      stock_id = picman_viewable_get_stock_id (renderer->viewable);
      break;
    }

  picman_view_renderer_render_stock (renderer, widget, stock_id);
}
