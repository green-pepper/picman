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

#ifndef __MENUS_H__
#define __MENUS_H__


extern PicmanMenuFactory *global_menu_factory;


void       menus_init    (Picman               *picman,
                          PicmanActionFactory  *action_factory);
void       menus_exit    (Picman               *picman);

void       menus_restore (Picman               *picman);
void       menus_save    (Picman               *picman,
                          gboolean            always_save);

gboolean   menus_clear   (Picman               *picman,
                          GError            **error);
void       menus_remove  (Picman               *picman);


#endif /* __MENUS_H__ */
