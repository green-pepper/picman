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

#ifndef __PICMAN_DISPLAY_XFER_H__
#define __PICMAN_DISPLAY_XFER_H__


/* #define PICMAN_DISPLAY_RENDER_ENABLE_SCALING 1 */

#define PICMAN_DISPLAY_RENDER_BUF_WIDTH  256
#define PICMAN_DISPLAY_RENDER_BUF_HEIGHT 256

#ifdef PICMAN_DISPLAY_RENDER_ENABLE_SCALING
#define PICMAN_DISPLAY_RENDER_MAX_SCALE 2.0
#else
#define PICMAN_DISPLAY_RENDER_MAX_SCALE 1.0
#endif


PicmanDisplayXfer * picman_display_xfer_realize     (GtkWidget       *widget);

cairo_surface_t * picman_display_xfer_get_surface (PicmanDisplayXfer *xfer,
                                                 gint             w,
                                                 gint             h,
                                                 gint            *src_x,
                                                 gint            *src_y);


#endif  /*  __PICMAN_DISPLAY_XFER_H__  */
