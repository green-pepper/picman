/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * picmanselection.h
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

#ifndef __PICMAN_SELECTION_H__
#define __PICMAN_SELECTION_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


gint32   picman_selection_float (gint32 image_ID,
                               gint32 drawable_ID,
                               gint   offx,
                               gint   offy);

PICMAN_DEPRECATED_FOR(picman_selection_none)
gboolean picman_selection_clear (gint32 image_ID);


G_END_DECLS

#endif /* __PICMAN_SELECTION_H__ */

