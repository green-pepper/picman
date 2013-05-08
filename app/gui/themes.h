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

#ifndef __THEMES_H__
#define __THEMES_H__


void           themes_init           (Picman        *picman);
void           themes_exit           (Picman        *picman);

gchar       ** themes_list_themes    (Picman        *picman,
                                      gint        *n_themes);
const gchar  * themes_get_theme_dir  (Picman        *picman,
                                      const gchar *theme_name);
gchar        * themes_get_theme_file (Picman        *picman,
                                      const gchar *first_component,
                                      ...) G_GNUC_NULL_TERMINATED;


#endif /* __THEMES_H__ */
