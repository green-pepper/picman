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

#ifndef __PICMAN_DISPLAY_SHELL_SCROLL_H__
#define __PICMAN_DISPLAY_SHELL_SCROLL_H__


void   picman_display_shell_scroll_center_image_coordinate    (PicmanDisplayShell       *shell,
                                                             gdouble                 image_x,
                                                             gdouble                 image_y);
void   picman_display_shell_scroll                            (PicmanDisplayShell       *shell,
                                                             gint                    x_offset,
                                                             gint                    y_offset);
void   picman_display_shell_scroll_set_offset                 (PicmanDisplayShell       *shell,
                                                             gint                    offset_x,
                                                             gint                    offset_y);
void   picman_display_shell_scroll_clamp_offsets              (PicmanDisplayShell       *shell);
void   picman_display_shell_scroll_clamp_and_update           (PicmanDisplayShell       *shell);
void   picman_display_shell_scroll_unoverscrollify            (PicmanDisplayShell       *shell,
                                                             gint                    in_offset_x,
                                                             gint                    in_offset_y,
                                                             gint                   *out_offset_x,
                                                             gint                   *out_offset_y);
void   picman_display_shell_scroll_center_image               (PicmanDisplayShell       *shell,
                                                             gboolean                horizontally,
                                                             gboolean                vertically);
void   picman_display_shell_scroll_center_image_on_next_size_allocate
                                                            (PicmanDisplayShell       *shell,
                                                             gboolean                horizontally,
                                                             gboolean                vertically);
void   picman_display_shell_scroll_get_scaled_viewport        (const PicmanDisplayShell *shell,
                                                             gint                   *x,
                                                             gint                   *y,
                                                             gint                   *w,
                                                             gint                   *h);
void   picman_display_shell_scroll_get_viewport               (const PicmanDisplayShell *shell,
                                                             gdouble                *x,
                                                             gdouble                *y,
                                                             gdouble                *w,
                                                             gdouble                *h);
void   picman_display_shell_scroll_setup_hscrollbar           (PicmanDisplayShell       *shell,
                                                             gdouble                 value);
void   picman_display_shell_scroll_setup_vscrollbar           (PicmanDisplayShell       *shell,
                                                             gdouble                 value);


#endif  /*  __PICMAN_DISPLAY_SHELL_SCROLL_H__  */
