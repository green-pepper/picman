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

#ifndef __PICMAN_DISPLAY_SHELL_APPEARANCE_H__
#define __PICMAN_DISPLAY_SHELL_APPEARANCE_H__


void       picman_display_shell_appearance_update      (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_menubar       (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_menubar       (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_statusbar     (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_statusbar     (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_rulers        (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_rulers        (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_scrollbars    (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_scrollbars    (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_selection     (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_selection     (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_layer         (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_layer         (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_grid          (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_grid          (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_guides        (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_guides        (PicmanDisplayShell       *shell);

void       picman_display_shell_set_snap_to_grid       (PicmanDisplayShell       *shell,
                                                      gboolean                snap);
gboolean   picman_display_shell_get_snap_to_grid       (PicmanDisplayShell       *shell);

void       picman_display_shell_set_show_sample_points (PicmanDisplayShell       *shell,
                                                      gboolean                show);
gboolean   picman_display_shell_get_show_sample_points (PicmanDisplayShell       *shell);

void       picman_display_shell_set_snap_to_guides     (PicmanDisplayShell       *shell,
                                                      gboolean                snap);
gboolean   picman_display_shell_get_snap_to_guides     (PicmanDisplayShell       *shell);

void       picman_display_shell_set_snap_to_canvas     (PicmanDisplayShell       *shell,
                                                      gboolean                snap);
gboolean   picman_display_shell_get_snap_to_canvas     (PicmanDisplayShell       *shell);

void       picman_display_shell_set_snap_to_vectors    (PicmanDisplayShell       *shell,
                                                      gboolean                snap);
gboolean   picman_display_shell_get_snap_to_vectors    (PicmanDisplayShell       *shell);

void       picman_display_shell_set_padding            (PicmanDisplayShell       *shell,
                                                      PicmanCanvasPaddingMode   mode,
                                                      const PicmanRGB          *color);
void       picman_display_shell_get_padding            (PicmanDisplayShell       *shell,
                                                      PicmanCanvasPaddingMode  *mode,
                                                      PicmanRGB                *color);


#endif /* __PICMAN_DISPLAY_SHELL_APPEARANCE_H__ */
