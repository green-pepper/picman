/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanwidgets-error.h
 * Copyright (C) 2008  Martin Nordholts <martinn@svn.gnome.org>
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

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_WIDGETS_ERROR_H__
#define __PICMAN_WIDGETS_ERROR_H__

G_BEGIN_DECLS


typedef enum
{
  PICMAN_WIDGETS_PARSE_ERROR
} PicmanWidgetsError;


/**
 * PICMAN_WIDGETS_ERROR:
 *
 * The PICMAN widgets error domain.
 *
 * Since: PICMAN 2.8
 */
#define PICMAN_WIDGETS_ERROR (picman_widgets_error_quark ())

GQuark  picman_widgets_error_quark (void) G_GNUC_CONST;


G_END_DECLS

#endif  /* __PICMAN_WIDGETS_ERROR_H__ */
