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

#ifndef __PICMAN_DISPLAY_SHELL_ROTATE_H__
#define __PICMAN_DISPLAY_SHELL_ROTATE_H__


void   picman_display_shell_rotate                  (PicmanDisplayShell *shell,
                                                   gdouble           delta);
void   picman_display_shell_rotate_to               (PicmanDisplayShell *shell,
                                                   gdouble           value);
void   picman_display_shell_rotate_drag             (PicmanDisplayShell *shell,
                                                   gdouble           last_x,
                                                   gdouble           last_y,
                                                   gdouble           cur_x,
                                                   gdouble           cur_y,
                                                   gboolean          constrain);

void   picman_display_shell_rotate_update_transform (PicmanDisplayShell *shell);


#endif  /*  __PICMAN_DISPLAY_SHELL_ROTATE_H__  */
