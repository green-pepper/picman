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

#ifndef __PICMAN_DISPLAY_SHELL_TOOL_EVENTS_H__
#define __PICMAN_DISPLAY_SHELL_TOOL_EVENTS_H__


gboolean   picman_display_shell_events                  (GtkWidget        *widget,
                                                       GdkEvent         *event,
                                                       PicmanDisplayShell *shell);

gboolean   picman_display_shell_canvas_tool_events      (GtkWidget        *widget,
                                                       GdkEvent         *event,
                                                       PicmanDisplayShell *shell);
void       picman_display_shell_buffer_stroke           (PicmanMotionBuffer *buffer,
                                                       const PicmanCoords *coords,
                                                       guint32           time,
                                                       GdkModifierType   state,
                                                       PicmanDisplayShell *shell);
void       picman_display_shell_buffer_hover            (PicmanMotionBuffer *buffer,
                                                       const PicmanCoords *coords,
                                                       GdkModifierType   state,
                                                       gboolean          proximity,
                                                       PicmanDisplayShell *shell);

gboolean   picman_display_shell_hruler_button_press     (GtkWidget        *widget,
                                                       GdkEventButton   *bevent,
                                                       PicmanDisplayShell *shell);
gboolean   picman_display_shell_vruler_button_press     (GtkWidget        *widget,
                                                       GdkEventButton   *bevent,
                                                       PicmanDisplayShell *shell);


#endif /* __PICMAN_DISPLAY_SHELL_TOOL_EVENT_H__ */
