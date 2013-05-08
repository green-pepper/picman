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

#ifndef __TOOL_MANAGER_H__
#define __TOOL_MANAGER_H__


void       tool_manager_init                       (Picman             *picman);
void       tool_manager_exit                       (Picman             *picman);

PicmanTool * tool_manager_get_active                 (Picman             *picman);

void       tool_manager_select_tool                (Picman             *picman,
                                                    PicmanTool         *tool);

void       tool_manager_push_tool                  (Picman             *picman,
                                                    PicmanTool         *tool);
void       tool_manager_pop_tool                   (Picman             *picman);


gboolean   tool_manager_initialize_active          (Picman             *picman,
                                                    PicmanDisplay      *display);
void       tool_manager_control_active             (Picman             *picman,
                                                    PicmanToolAction    action,
                                                    PicmanDisplay      *display);
void       tool_manager_button_press_active        (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    guint32           time,
                                                    GdkModifierType   state,
                                                    PicmanButtonPressType press_type,
                                                    PicmanDisplay      *display);
void       tool_manager_button_release_active      (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    guint32           time,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display);
void       tool_manager_motion_active              (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    guint32           time,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display);
gboolean   tool_manager_key_press_active           (Picman             *picman,
                                                    GdkEventKey      *kevent,
                                                    PicmanDisplay      *display);
gboolean   tool_manager_key_release_active         (Picman             *picman,
                                                    GdkEventKey      *kevent,
                                                    PicmanDisplay      *display);

void       tool_manager_focus_display_active       (Picman             *picman,
                                                    PicmanDisplay      *display);
void       tool_manager_modifier_state_active      (Picman             *picman,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display);

void     tool_manager_active_modifier_state_active (Picman             *picman,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display);

void       tool_manager_oper_update_active         (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    GdkModifierType   state,
                                                    gboolean          proximity,
                                                    PicmanDisplay      *display);
void       tool_manager_cursor_update_active       (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display);

PicmanUIManager * tool_manager_get_popup_active      (Picman             *picman,
                                                    const PicmanCoords *coords,
                                                    GdkModifierType   state,
                                                    PicmanDisplay      *display,
                                                    const gchar     **ui_path);


#endif  /*  __TOOL_MANAGER_H__  */
