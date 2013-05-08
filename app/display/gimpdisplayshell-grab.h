/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandisplayshell-grab.h
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

#ifndef __PICMAN_DISPLAY_SHELL_GRAB_H__
#define __PICMAN_DISPLAY_SHELL_GRAB_H__


gboolean   picman_display_shell_pointer_grab    (PicmanDisplayShell *shell,
                                               const GdkEvent   *event,
                                               GdkEventMask      event_mask);
void       picman_display_shell_pointer_ungrab  (PicmanDisplayShell *shell,
                                               const GdkEvent   *event);

gboolean   picman_display_shell_keyboard_grab   (PicmanDisplayShell *shell,
                                               const GdkEvent   *event);
void       picman_display_shell_keyboard_ungrab (PicmanDisplayShell *shell,
                                               const GdkEvent   *event);


#endif /* __PICMAN_DISPLAY_SHELL_GRAB_H__ */
