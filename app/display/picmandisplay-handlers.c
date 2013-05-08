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

#include "display-types.h"

#include "core/picmanimage.h"

#include "picmandisplay.h"
#include "picmandisplay-handlers.h"


/*  local function prototypes  */

static void   picman_display_update_handler (PicmanProjection *projection,
                                           gboolean        now,
                                           gint            x,
                                           gint            y,
                                           gint            w,
                                           gint            h,
                                           PicmanDisplay    *display);
static void   picman_display_flush_handler  (PicmanImage      *image,
                                           gboolean        invalidate_preview,
                                           PicmanDisplay    *display);


/*  public functions  */

void
picman_display_connect (PicmanDisplay *display)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  image = picman_display_get_image (display);

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_connect (picman_image_get_projection (image), "update",
                    G_CALLBACK (picman_display_update_handler),
                    display);

  g_signal_connect (image, "flush",
                    G_CALLBACK (picman_display_flush_handler),
                    display);
}

void
picman_display_disconnect (PicmanDisplay *display)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  image = picman_display_get_image (display);

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  g_signal_handlers_disconnect_by_func (image,
                                        picman_display_flush_handler,
                                        display);

  g_signal_handlers_disconnect_by_func (picman_image_get_projection (image),
                                        picman_display_update_handler,
                                        display);
}


/*  private functions  */

static void
picman_display_update_handler (PicmanProjection *projection,
                             gboolean        now,
                             gint            x,
                             gint            y,
                             gint            w,
                             gint            h,
                             PicmanDisplay    *display)
{
  picman_display_update_area (display, now, x, y, w, h);
}

static void
picman_display_flush_handler (PicmanImage   *image,
                            gboolean     invalidate_preview,
                            PicmanDisplay *display)
{
  picman_display_flush (display);
}
