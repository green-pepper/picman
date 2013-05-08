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

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "core/picman-cairo.h"
#include "core/picman-utils.h"

#include "picmancanvas.h"
#include "picmancanvas-style.h"
#include "picmancanvaspath.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-draw.h"
#include "picmandisplayshell-render.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-transform.h"
#include "picmandisplayxfer.h"


/*  public functions  */

void
picman_display_shell_draw_selection_out (PicmanDisplayShell *shell,
                                       cairo_t          *cr,
                                       PicmanSegment      *segs,
                                       gint              n_segs)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (segs != NULL && n_segs > 0);

  picman_canvas_set_selection_out_style (shell->canvas, cr);

  picman_cairo_add_segments (cr, segs, n_segs);
  cairo_stroke (cr);
}

void
picman_display_shell_draw_selection_in (PicmanDisplayShell   *shell,
                                      cairo_t            *cr,
                                      cairo_pattern_t    *mask,
                                      gint                index)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);
  g_return_if_fail (mask != NULL);

  picman_canvas_set_selection_in_style (shell->canvas, cr, index);

  cairo_mask (cr, mask);
}

void
picman_display_shell_draw_background (PicmanDisplayShell *shell,
                                    cairo_t          *cr)
{
  GdkWindow       *window;
  cairo_pattern_t *bg_pattern;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  window     = gtk_widget_get_window (shell->canvas);
  bg_pattern = gdk_window_get_background_pattern (window);

  cairo_set_source (cr, bg_pattern);
  cairo_paint (cr);
}

void
picman_display_shell_draw_checkerboard (PicmanDisplayShell *shell,
                                      cairo_t          *cr)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (cr != NULL);

  if (G_UNLIKELY (! shell->checkerboard))
    {
      PicmanCheckSize  check_size;
      PicmanCheckType  check_type;
      guchar         check_light;
      guchar         check_dark;
      PicmanRGB        light;
      PicmanRGB        dark;

      g_object_get (shell->display->config,
                    "transparency-size", &check_size,
                    "transparency-type", &check_type,
                    NULL);

      picman_checks_get_shades (check_type, &check_light, &check_dark);
      picman_rgb_set_uchar (&light, check_light, check_light, check_light);
      picman_rgb_set_uchar (&dark,  check_dark,  check_dark,  check_dark);

      shell->checkerboard =
        picman_cairo_checkerboard_create (cr,
                                        1 << (check_size + 2), &light, &dark);
    }

  cairo_translate (cr, - shell->offset_x, - shell->offset_y);
  cairo_set_source (cr, shell->checkerboard);
  cairo_paint (cr);
}

void
picman_display_shell_draw_image (PicmanDisplayShell *shell,
                               cairo_t          *cr,
                               gint              x,
                               gint              y,
                               gint              w,
                               gint              h)
{
  gint x1, y1, x2, y2;
  gint i, j;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (picman_display_get_image (shell->display));
  g_return_if_fail (cr != NULL);

  if (shell->rotate_untransform)
    {
      gdouble tx1, ty1;
      gdouble tx2, ty2;
      gint    image_width;
      gint    image_height;

      picman_display_shell_unrotate_bounds (shell,
                                          x, y, x + w, y + h,
                                          &tx1, &ty1, &tx2, &ty2);

      x1 = floor (tx1 - 0.5);
      y1 = floor (ty1 - 0.5);
      x2 = ceil (tx2 + 0.5);
      y2 = ceil (ty2 + 0.5);

      picman_display_shell_scale_get_image_size (shell,
                                               &image_width, &image_height);

      x1 = CLAMP (x1, -shell->offset_x, -shell->offset_x + image_width);
      y1 = CLAMP (y1, -shell->offset_y, -shell->offset_y + image_height);
      x2 = CLAMP (x2, -shell->offset_x, -shell->offset_x + image_width);
      y2 = CLAMP (y2, -shell->offset_y, -shell->offset_y + image_height);

      if (!(x2 > x1) || !(y2 > y1))
        return;
    }
  else
    {
      x1 = x;
      y1 = y;
      x2 = x + w;
      y2 = y + h;
    }

  /*  display the image in RENDER_BUF_WIDTH x RENDER_BUF_HEIGHT
   *  sized chunks
   */
  for (i = y1; i < y2; i += PICMAN_DISPLAY_RENDER_BUF_HEIGHT)
    {
      for (j = x1; j < x2; j += PICMAN_DISPLAY_RENDER_BUF_WIDTH)
        {
          gint dx, dy;

          dx = MIN (x2 - j, PICMAN_DISPLAY_RENDER_BUF_WIDTH);
          dy = MIN (y2 - i, PICMAN_DISPLAY_RENDER_BUF_HEIGHT);

          picman_display_shell_render (shell, cr, j, i, dx, dy);
        }
    }
}
