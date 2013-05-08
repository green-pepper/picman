/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewrendererdrawable.c
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "widgets-types.h"

#include "core/picmandrawable.h"
#include "core/picmandrawable-preview.h"
#include "core/picmanimage.h"
#include "core/picmantempbuf.h"

#include "picmanviewrendererdrawable.h"


static void   picman_view_renderer_drawable_render (PicmanViewRenderer *renderer,
                                                  GtkWidget        *widget);


G_DEFINE_TYPE (PicmanViewRendererDrawable, picman_view_renderer_drawable,
               PICMAN_TYPE_VIEW_RENDERER)

#define parent_class picman_view_renderer_drawable_parent_class


static void
picman_view_renderer_drawable_class_init (PicmanViewRendererDrawableClass *klass)
{
  PicmanViewRendererClass *renderer_class = PICMAN_VIEW_RENDERER_CLASS (klass);

  renderer_class->render = picman_view_renderer_drawable_render;
}

static void
picman_view_renderer_drawable_init (PicmanViewRendererDrawable *renderer)
{
}

static void
picman_view_renderer_drawable_render (PicmanViewRenderer *renderer,
                                    GtkWidget        *widget)
{
  PicmanDrawable *drawable;
  PicmanItem     *item;
  PicmanImage    *image;
  gint          offset_x;
  gint          offset_y;
  gint          width;
  gint          height;
  gint          view_width;
  gint          view_height;
  gdouble       xres       = 1.0;
  gdouble       yres       = 1.0;
  gboolean      scaling_up;
  PicmanTempBuf  *render_buf = NULL;

  drawable = PICMAN_DRAWABLE (renderer->viewable);
  item     = PICMAN_ITEM (drawable);
  image    = picman_item_get_image (item);

  picman_item_get_offset (item, &offset_x, &offset_y);

  width  = renderer->width;
  height = renderer->height;

  if (image)
    picman_image_get_resolution (image, &xres, &yres);

  if (image && ! renderer->is_popup)
    {
      width  = MAX (1, ROUND ((((gdouble) width /
                                (gdouble) picman_image_get_width (image)) *
                               (gdouble) picman_item_get_width (item))));
      height = MAX (1, ROUND ((((gdouble) height /
                                (gdouble) picman_image_get_height (image)) *
                               (gdouble) picman_item_get_height (item))));

      picman_viewable_calc_preview_size (picman_item_get_width  (item),
                                       picman_item_get_height (item),
                                       width,
                                       height,
                                       renderer->dot_for_dot,
                                       xres,
                                       yres,
                                       &view_width,
                                       &view_height,
                                       &scaling_up);
    }
  else
    {
      picman_viewable_calc_preview_size (picman_item_get_width  (item),
                                       picman_item_get_height (item),
                                       width,
                                       height,
                                       renderer->dot_for_dot,
                                       xres,
                                       yres,
                                       &view_width,
                                       &view_height,
                                       &scaling_up);
    }

  if ((view_width * view_height) <
      (picman_item_get_width (item) * picman_item_get_height (item) * 4))
    scaling_up = FALSE;

  if (scaling_up)
    {
      if (image && ! renderer->is_popup)
        {
          gint src_x, src_y;
          gint src_width, src_height;

          if (picman_rectangle_intersect (0, 0,
                                        picman_item_get_width  (item),
                                        picman_item_get_height (item),
                                        -offset_x, -offset_y,
                                        picman_image_get_width  (image),
                                        picman_image_get_height (image),
                                        &src_x, &src_y,
                                        &src_width, &src_height))
            {
              gint dest_width;
              gint dest_height;

              dest_width  = ROUND (((gdouble) renderer->width /
                                    (gdouble) picman_image_get_width (image)) *
                                   (gdouble) src_width);
              dest_height = ROUND (((gdouble) renderer->height /
                                    (gdouble) picman_image_get_height (image)) *
                                   (gdouble) src_height);

              if (dest_width  < 1) dest_width  = 1;
              if (dest_height < 1) dest_height = 1;

              render_buf = picman_drawable_get_sub_preview (drawable,
                                                          src_x, src_y,
                                                          src_width, src_height,
                                                          dest_width, dest_height);
            }
          else
            {
              const Babl *format = picman_drawable_get_preview_format (drawable);

              render_buf = picman_temp_buf_new (1, 1, format);
              picman_temp_buf_data_clear (render_buf);
            }
        }
      else
        {
          PicmanTempBuf *temp_buf;

          temp_buf = picman_viewable_get_new_preview (renderer->viewable,
                                                    renderer->context,
                                                    picman_item_get_width  (item),
                                                    picman_item_get_height (item));

          if (temp_buf)
            {
              render_buf = picman_temp_buf_scale (temp_buf,
                                                view_width, view_height);
              picman_temp_buf_unref (temp_buf);
            }
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
      gint render_buf_x = 0;
      gint render_buf_y = 0;

      if (image && ! renderer->is_popup)
        {
          if (offset_x != 0)
            render_buf_x =
              ROUND ((((gdouble) renderer->width /
                       (gdouble) picman_image_get_width (image)) *
                      (gdouble) offset_x));

          if (offset_y != 0)
            render_buf_y =
              ROUND ((((gdouble) renderer->height /
                       (gdouble) picman_image_get_height (image)) *
                      (gdouble) offset_y));

          if (scaling_up)
            {
              if (render_buf_x < 0) render_buf_x = 0;
              if (render_buf_y < 0) render_buf_y = 0;
            }
        }
      else
        {
          if (view_width < width)
            render_buf_x = (width - view_width) / 2;

          if (view_height < height)
            render_buf_y = (height - view_height) / 2;
        }

      picman_view_renderer_render_temp_buf (renderer, render_buf,
                                          render_buf_x, render_buf_y,
                                          -1,
                                          PICMAN_VIEW_BG_CHECKS,
                                          PICMAN_VIEW_BG_CHECKS);
      picman_temp_buf_unref (render_buf);
    }
  else
    {
      const gchar *stock_id;

      stock_id = picman_viewable_get_stock_id (renderer->viewable);

      picman_view_renderer_render_stock (renderer, widget, stock_id);
    }
}
