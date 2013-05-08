/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "gegl/picman-gegl-utils.h"

#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"
#include "core/picmanprojectable.h"
#include "core/picmanprojection.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-transform.h"
#include "picmandisplayshell-filter.h"
#include "picmandisplayshell-render.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayxfer.h"


void
picman_display_shell_render (PicmanDisplayShell *shell,
                           cairo_t          *cr,
                           gint              x,
                           gint              y,
                           gint              w,
                           gint              h)
{
  PicmanImage       *image;
  PicmanProjection  *projection;
  GeglBuffer      *buffer;
  gdouble          window_scale = 1.0;
  gint             viewport_offset_x;
  gint             viewport_offset_y;
  gint             viewport_width;
  gint             viewport_height;
  cairo_surface_t *xfer;
  gint             src_x;
  gint             src_y;
  gint             mask_src_x = 0;
  gint             mask_src_y = 0;
  gint             stride;
  guchar          *data;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (w > 0 && h > 0);

  image      = picman_display_get_image (shell->display);
  projection = picman_image_get_projection (image);
  buffer     = picman_pickable_get_buffer (PICMAN_PICKABLE (projection));

#ifdef PICMAN_DISPLAY_RENDER_ENABLE_SCALING
  /* if we had this future API, things would look pretty on hires (retina) */
  window_scale = gdk_window_get_scale_factor (gtk_widget_get_window (gtk_widget_get_toplevel (GTK_WIDGET (shell))));
#endif

  window_scale = MIN (window_scale, PICMAN_DISPLAY_RENDER_MAX_SCALE);

  picman_display_shell_scroll_get_scaled_viewport (shell,
                                                 &viewport_offset_x,
                                                 &viewport_offset_y,
                                                 &viewport_width,
                                                 &viewport_height);
  if (shell->rotate_transform)
    {
      xfer = cairo_surface_create_similar_image (cairo_get_target (cr),
                                                 CAIRO_FORMAT_ARGB32,
                                                 w * window_scale,
                                                 h * window_scale);
      cairo_surface_mark_dirty (xfer);
      src_x = 0;
      src_y = 0;
    }
  else
    {
      xfer = picman_display_xfer_get_surface (shell->xfer,
                                            w * window_scale,
                                            h * window_scale,
                                            &src_x, &src_y);
    }

  stride = cairo_image_surface_get_stride (xfer);
  data = cairo_image_surface_get_data (xfer);
  data += src_y * stride + src_x * 4;

  gegl_buffer_get (buffer,
                   GEGL_RECTANGLE ((x + viewport_offset_x) * window_scale,
                                   (y + viewport_offset_y) * window_scale,
                                   w * window_scale,
                                   h * window_scale),
                   shell->scale_x * window_scale,
                   babl_format ("cairo-ARGB32"),
		   data, stride,
                   GEGL_ABYSS_NONE);

  /*  apply filters to the rendered projection  */
  if (shell->filter_stack)
    {
      cairo_surface_t *image =
	cairo_image_surface_create_for_data (data, CAIRO_FORMAT_ARGB32,
					     w * window_scale,
					     h * window_scale,
					     stride);
      picman_color_display_stack_convert_surface (shell->filter_stack, image);
      cairo_surface_destroy (image);
    }

  if (shell->mask)
    {
      if (! shell->mask_surface)
        {
          shell->mask_surface =
            cairo_image_surface_create (CAIRO_FORMAT_A8,
                                        PICMAN_DISPLAY_RENDER_BUF_WIDTH  *
                                        PICMAN_DISPLAY_RENDER_MAX_SCALE,
                                        PICMAN_DISPLAY_RENDER_BUF_HEIGHT *
                                        PICMAN_DISPLAY_RENDER_MAX_SCALE);
        }

      cairo_surface_mark_dirty (shell->mask_surface);

      buffer = picman_drawable_get_buffer (shell->mask);

      stride = cairo_image_surface_get_stride (shell->mask_surface);
      data = cairo_image_surface_get_data (shell->mask_surface);
      data += mask_src_y * stride + mask_src_x * 4;

      gegl_buffer_get (buffer,
                       GEGL_RECTANGLE ((x + viewport_offset_x) * window_scale,
                                       (y + viewport_offset_y) * window_scale,
                                       w * window_scale,
                                       h * window_scale),
                       shell->scale_x * window_scale,
                       babl_format ("Y u8"),
                       data, stride,
                       GEGL_ABYSS_NONE);
    }

  /*  put it to the screen  */
  cairo_save (cr);

  cairo_rectangle (cr, x, y, w, h);

  cairo_scale (cr, 1.0 / window_scale, 1.0 / window_scale);

  cairo_set_source_surface (cr, xfer,
                            (x - src_x) * window_scale,
                            (y - src_y) * window_scale);

  if (shell->rotate_transform)
    {
      cairo_pattern_t *pattern;

      pattern = cairo_get_source (cr);
      cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

      cairo_set_line_width (cr, 1.0);
      cairo_stroke_preserve (cr);

      cairo_surface_destroy (xfer);
    }

  cairo_clip (cr);
  cairo_paint (cr);

  if (shell->mask)
    {
      picman_cairo_set_source_rgba (cr, &shell->mask_color);
      cairo_mask_surface (cr, shell->mask_surface,
                          (x - mask_src_x) * window_scale,
                          (y - mask_src_y) * window_scale);
    }

  cairo_restore (cr);
}
