/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_COMPAT_H__
#define __PICMAN_TEXT_COMPAT_H__


/* convenience functions that provide the 1.2 API, only used by the PDB */

PicmanLayer * text_render      (PicmanImage    *image,
                              PicmanDrawable *drawable,
                              PicmanContext  *context,
                              gint          text_x,
                              gint          text_y,
                              const gchar  *fontname,
                              const gchar  *text,
                              gint          border,
                              gboolean      antialias);
gboolean    text_get_extents (const gchar  *fontname,
                              const gchar  *text,
                              gint         *width,
                              gint         *height,
                              gint         *ascent,
                              gint         *descent);


#endif /* __PICMAN_TEXT_COMPAT_H__ */
