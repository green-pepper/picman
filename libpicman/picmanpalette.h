/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * picmanpalette.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picman.h> can be included directly."
#endif

#ifndef __PICMAN_PALETTE_H__
#define __PICMAN_PALETTE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */

PICMAN_DEPRECATED_FOR(picman_context_get_foreground)
gboolean picman_palette_get_foreground     (PicmanRGB       *foreground);
PICMAN_DEPRECATED_FOR(picman_context_get_background)
gboolean picman_palette_get_background     (PicmanRGB       *background);
PICMAN_DEPRECATED_FOR(picman_context_set_foreground)
gboolean picman_palette_set_foreground     (const PicmanRGB *foreground);
PICMAN_DEPRECATED_FOR(picman_context_set_background)
gboolean picman_palette_set_background     (const PicmanRGB *background);
PICMAN_DEPRECATED_FOR(picman_context_set_default_colors)
gboolean picman_palette_set_default_colors (void);
PICMAN_DEPRECATED_FOR(picman_context_swap_colors)
gboolean picman_palette_swap_colors        (void);

G_END_DECLS

#endif /* __PICMAN_PALETTE_H__ */
