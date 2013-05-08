/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanunitcache.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_UNIT_CACHE_H__
#define __PICMAN_UNIT_CACHE_H__

G_BEGIN_DECLS


G_GNUC_INTERNAL gint          _picman_unit_cache_get_number_of_units          (void);
G_GNUC_INTERNAL gint          _picman_unit_cache_get_number_of_built_in_units (void) G_GNUC_CONST;

G_GNUC_INTERNAL PicmanUnit      _picman_unit_cache_new               (gchar   *identifier,
                                                                  gdouble  factor,
                                                                  gint     digits,
                                                                  gchar   *symbol,
                                                                  gchar   *abbreviation,
                                                                  gchar   *singular,
                                                                  gchar   *plural);
G_GNUC_INTERNAL gboolean      _picman_unit_cache_get_deletion_flag (PicmanUnit unit);
G_GNUC_INTERNAL void          _picman_unit_cache_set_deletion_flag (PicmanUnit unit,
                                                                  gboolean deletion_flag);
G_GNUC_INTERNAL gdouble       _picman_unit_cache_get_factor        (PicmanUnit unit);
G_GNUC_INTERNAL gint          _picman_unit_cache_get_digits        (PicmanUnit unit);
G_GNUC_INTERNAL const gchar * _picman_unit_cache_get_identifier    (PicmanUnit unit);
G_GNUC_INTERNAL const gchar * _picman_unit_cache_get_symbol        (PicmanUnit unit);
G_GNUC_INTERNAL const gchar * _picman_unit_cache_get_abbreviation  (PicmanUnit unit);
G_GNUC_INTERNAL const gchar * _picman_unit_cache_get_singular      (PicmanUnit unit);
G_GNUC_INTERNAL const gchar * _picman_unit_cache_get_plural        (PicmanUnit unit);


G_END_DECLS

#endif /*  __PICMAN_UNIT_CACHE_H__ */
