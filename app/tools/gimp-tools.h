/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis and others
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

#ifndef __PICMAN_TOOLS_H__
#define __PICMAN_TOOLS_H__


void       picman_tools_init              (Picman      *picman);
void       picman_tools_exit              (Picman      *picman);

void       picman_tools_restore           (Picman      *picman);
void       picman_tools_save              (Picman      *picman,
                                         gboolean   save_tool_options,
                                         gboolean   always_save);

gboolean   picman_tools_clear             (Picman      *picman,
                                         GError   **error);

GList    * picman_tools_get_default_order (Picman      *picman);


#endif  /* __PICMAN_TOOLS_H__ */
