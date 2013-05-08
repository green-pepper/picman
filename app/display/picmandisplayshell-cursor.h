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

#ifndef __PICMAN_DISPLAY_SHELL_CURSOR_H__
#define __PICMAN_DISPLAY_SHELL_CURSOR_H__


/*  functions dealing with the normal windowing system cursor  */

void   picman_display_shell_set_cursor             (PicmanDisplayShell    *shell,
                                                  PicmanCursorType       cursor_type,
                                                  PicmanToolCursorType   tool_cursor,
                                                  PicmanCursorModifier   modifier);
void   picman_display_shell_unset_cursor           (PicmanDisplayShell    *shell);
void   picman_display_shell_set_override_cursor    (PicmanDisplayShell    *shell,
                                                  PicmanCursorType       cursor_type);
void   picman_display_shell_unset_override_cursor  (PicmanDisplayShell    *shell);


/*  functions dealing with the software cursor that is drawn to the
 *  canvas by PICMAN
 */

void   picman_display_shell_update_software_cursor (PicmanDisplayShell    *shell,
                                                  PicmanCursorPrecision  precision,
                                                  gint                 display_x,
                                                  gint                 display_y,
                                                  gdouble              image_x,
                                                  gdouble              image_y);
void   picman_display_shell_clear_software_cursor  (PicmanDisplayShell    *shell);


#endif /* __PICMAN_DISPLAY_SHELL_CURSOR_H__ */
