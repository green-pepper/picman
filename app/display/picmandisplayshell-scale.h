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

#ifndef __PICMAN_DISPLAY_SHELL_SCALE_H__
#define __PICMAN_DISPLAY_SHELL_SCALE_H__


void     picman_display_shell_scale_update_scrollbars        (PicmanDisplayShell *shell);
void     picman_display_shell_scale_update_rulers            (PicmanDisplayShell *shell);

gboolean picman_display_shell_scale_revert                   (PicmanDisplayShell *shell);
gboolean picman_display_shell_scale_can_revert               (PicmanDisplayShell *shell);

void     picman_display_shell_scale_set_dot_for_dot          (PicmanDisplayShell *shell,
                                                            gboolean          dot_for_dot);

void     picman_display_shell_get_screen_resolution          (PicmanDisplayShell *shell,
                                                            gdouble          *xres,
                                                            gdouble          *yres);
void     picman_display_shell_scale_get_image_size           (PicmanDisplayShell *shell,
                                                            gint             *w,
                                                            gint             *h);
void     picman_display_shell_scale_get_image_size_for_scale (PicmanDisplayShell *shell,
                                                            gdouble           scale,
                                                            gint             *w,
                                                            gint             *h);

void     picman_display_shell_scale                          (PicmanDisplayShell *shell,
                                                            PicmanZoomType      zoom_type,
                                                            gdouble           scale,
                                                            PicmanZoomFocus     zoom_focus);
void     picman_display_shell_scale_fit_in                   (PicmanDisplayShell *shell);
gboolean picman_display_shell_scale_image_is_within_viewport (PicmanDisplayShell *shell,
                                                            gboolean         *horizontally,
                                                            gboolean         *vertically);
void     picman_display_shell_scale_fill                     (PicmanDisplayShell *shell);
void     picman_display_shell_scale_handle_zoom_revert       (PicmanDisplayShell *shell);
void     picman_display_shell_scale_by_values                (PicmanDisplayShell *shell,
                                                            gdouble           scale,
                                                            gint              offset_x,
                                                            gint              offset_y,
                                                            gboolean          resize_window);
void     picman_display_shell_scale_shrink_wrap              (PicmanDisplayShell *shell,
                                                            gboolean          grow_only);

void     picman_display_shell_scale_resize                   (PicmanDisplayShell *shell,
                                                            gboolean          resize_window,
                                                            gboolean          grow_only);
void     picman_display_shell_calculate_scale_x_and_y        (PicmanDisplayShell *shell,
                                                            gdouble           scale,
                                                            gdouble          *scale_x,
                                                            gdouble          *scale_y);
void     picman_display_shell_set_initial_scale              (PicmanDisplayShell *shell,
                                                            gdouble           scale,
                                                            gint             *display_width,
                                                            gint             *display_height);
void     picman_display_shell_push_zoom_focus_pointer_pos    (PicmanDisplayShell *shell,
                                                            gint              x,
                                                            gint              y);


#endif  /*  __PICMAN_DISPLAY_SHELL_SCALE_H__  */
