/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanenumwidgets.h
 * Copyright (C) 2002-2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_ENUM_WIDGETS_H__
#define __PICMAN_ENUM_WIDGETS_H__

G_BEGIN_DECLS


GtkWidget * picman_enum_radio_box_new               (GType         enum_type,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);
GtkWidget * picman_enum_radio_box_new_with_range    (GType         enum_type,
                                                   gint          minimum,
                                                   gint          maximum,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);

GtkWidget * picman_enum_radio_frame_new             (GType         enum_type,
                                                   GtkWidget    *label_widget,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);
GtkWidget * picman_enum_radio_frame_new_with_range  (GType         enum_type,
                                                   gint          minimum,
                                                   gint          maximum,
                                                   GtkWidget    *label_widget,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);

GtkWidget * picman_enum_stock_box_new               (GType         enum_type,
                                                   const gchar  *stock_prefix,
                                                   GtkIconSize   icon_size,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);
GtkWidget * picman_enum_stock_box_new_with_range    (GType         enum_type,
                                                   gint          minimum,
                                                   gint          maximum,
                                                   const gchar  *stock_prefix,
                                                   GtkIconSize   icon_size,
                                                   GCallback     callback,
                                                   gpointer      callback_data,
                                                   GtkWidget   **first_button);

void        picman_enum_stock_box_set_child_padding (GtkWidget    *stock_box,
                                                   gint          xpad,
                                                   gint          ypad);


G_END_DECLS

#endif  /* __PICMAN_ENUM_WIDGETS_H__ */
