/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontrollers.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTROLLERS_H__
#define __PICMAN_CONTROLLERS_H__


void             picman_controllers_init           (Picman          *picman);
void             picman_controllers_exit           (Picman          *picman);

void             picman_controllers_restore        (Picman          *picman,
                                                  PicmanUIManager *ui_manager);
void             picman_controllers_save           (Picman          *picman);

PicmanContainer  * picman_controllers_get_list       (Picman          *picman);
PicmanUIManager  * picman_controllers_get_ui_manager (Picman          *picman);
PicmanController * picman_controllers_get_mouse      (Picman          *picman);
PicmanController * picman_controllers_get_wheel      (Picman          *picman);
PicmanController * picman_controllers_get_keyboard   (Picman          *picman);


#endif /* __PICMAN_CONTROLLERS_H__ */
