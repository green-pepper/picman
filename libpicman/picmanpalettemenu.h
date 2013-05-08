/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpalettemenu.h
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

#if !defined (__PICMAN_UI_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picmanui.h> can be included directly."
#endif

#ifndef __PICMAN_PALETTE_MENU_H__
#define __PICMAN_PALETTE_MENU_H__

/*  These functions are deprecated and should not be used in newly
 *  written code.
 */

G_BEGIN_DECLS

PICMAN_DEPRECATED_FOR(picman_gradient_select_button_new)
GtkWidget * picman_palette_select_widget_new   (const gchar            *title,
                                              const gchar            *palette_name,
                                              PicmanRunPaletteCallback  callback,
                                              gpointer                data);

PICMAN_DEPRECATED_FOR(picman_select_button_close_popup)
void        picman_palette_select_widget_close (GtkWidget              *widget);
PICMAN_DEPRECATED_FOR(picman_gradient_select_button_set_gradient)
void        picman_palette_select_widget_set   (GtkWidget              *widget,
                                              const gchar            *palette_name);


G_END_DECLS

#endif /* __PICMAN_PALETTE_MENU_H__ */
