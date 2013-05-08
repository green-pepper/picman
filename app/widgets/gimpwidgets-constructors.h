/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_WIDGETS_CONSTRUCTORS_H__
#define __PICMAN_WIDGETS_CONSTRUCTORS_H__


GtkWidget * picman_paint_mode_menu_new (gboolean     with_behind_mode,
                                      gboolean     with_replace_modes);

GtkWidget * picman_stock_button_new    (const gchar *stock_id,
                                      const gchar *label);


#endif  /*  __PICMAN_WIDGETS_CONSTRUCTORS_H__  */
