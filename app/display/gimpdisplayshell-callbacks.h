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

#ifndef __PICMAN_DISPLAY_SHELL_CALLBACKS_H__
#define __PICMAN_DISPLAY_SHELL_CALLBACKS_H__


void       picman_display_shell_canvas_realize          (GtkWidget        *widget,
                                                       PicmanDisplayShell *shell);
void       picman_display_shell_canvas_size_allocate    (GtkWidget        *widget,
                                                       GtkAllocation    *alloc,
                                                       PicmanDisplayShell *shell);
gboolean   picman_display_shell_canvas_expose           (GtkWidget        *widget,
                                                       GdkEventExpose   *eevent,
                                                       PicmanDisplayShell *shell);

gboolean   picman_display_shell_origin_button_press     (GtkWidget        *widget,
                                                       GdkEventButton   *bevent,
                                                       PicmanDisplayShell *shell);

gboolean   picman_display_shell_quick_mask_button_press (GtkWidget        *widget,
                                                       GdkEventButton   *bevent,
                                                       PicmanDisplayShell *shell);
void       picman_display_shell_quick_mask_toggled      (GtkWidget        *widget,
                                                       PicmanDisplayShell *shell);

gboolean   picman_display_shell_navigation_button_press (GtkWidget        *widget,
                                                       GdkEventButton   *bevent,
                                                       PicmanDisplayShell *shell);


#endif /* __PICMAN_DISPLAY_SHELL_CALLBACKS_H__ */
