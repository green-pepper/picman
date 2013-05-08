/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * PicmanRc serialization and deserialization helpers
 * Copyright (C) 2001-2005  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_RC_UNKNOWN_H__
#define __PICMAN_RC_UNKNOWN_H__


typedef void  (* PicmanConfigForeachFunc) (const gchar *key,
                                         const gchar *value,
                                         gpointer     user_data);


void          picman_rc_add_unknown_token     (PicmanConfig            *config,
                                             const gchar           *key,
                                             const gchar           *value);
const gchar * picman_rc_lookup_unknown_token  (PicmanConfig            *config,
                                             const gchar           *key);
void          picman_rc_foreach_unknown_token (PicmanConfig            *config,
                                             PicmanConfigForeachFunc  func,
                                             gpointer               user_data);


#endif  /* __PICMAN_RC_UNKNOWN_H__ */
