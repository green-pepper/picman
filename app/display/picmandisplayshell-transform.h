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

#ifndef __PICMAN_DISPLAY_SHELL_TRANSFORM_H__
#define __PICMAN_DISPLAY_SHELL_TRANSFORM_H__


/*  zoom: functions to transform from image space to unrotated display
 *  space and back, taking into account scroll offset and scale
 */

void  picman_display_shell_zoom_coords          (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *image_coords,
                                               PicmanCoords             *display_coords);
void  picman_display_shell_unzoom_coords        (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *display_coords,
                                               PicmanCoords             *image_coords);

void  picman_display_shell_zoom_xy              (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gint                   *nx,
                                               gint                   *ny);
void  picman_display_shell_unzoom_xy            (const PicmanDisplayShell *shell,
                                               gint                    x,
                                               gint                    y,
                                               gint                   *nx,
                                               gint                   *ny,
                                               gboolean                round);

void  picman_display_shell_zoom_xy_f            (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);
void  picman_display_shell_unzoom_xy_f          (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);

void  picman_display_shell_zoom_segments        (const PicmanDisplayShell *shell,
                                               const PicmanBoundSeg     *src_segs,
                                               PicmanSegment            *dest_segs,
                                               gint                    n_segs,
                                               gdouble                 offset_x,
                                               gdouble                 offset_y);


/*  rotate: functions to transform from unrotated but zoomed display
 *  space to rotated display space and back
 */

void  picman_display_shell_rotate_coords        (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *image_coords,
                                               PicmanCoords             *display_coords);
void  picman_display_shell_unrotate_coords      (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *display_coords,
                                               PicmanCoords             *image_coords);

void  picman_display_shell_rotate_xy            (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gint                   *nx,
                                               gint                   *ny);
void  picman_display_shell_unrotate_xy          (const PicmanDisplayShell *shell,
                                               gint                    x,
                                               gint                    y,
                                               gint                   *nx,
                                               gint                   *ny);

void  picman_display_shell_rotate_xy_f          (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);
void  picman_display_shell_unrotate_xy_f        (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);

void  picman_display_shell_rotate_bounds        (PicmanDisplayShell       *shell,
                                               gdouble                 x1,
                                               gdouble                 y1,
                                               gdouble                 x2,
                                               gdouble                 y2,
                                               gdouble                *nx1,
                                               gdouble                *ny1,
                                               gdouble                *nx2,
                                               gdouble                *ny2);
void  picman_display_shell_unrotate_bounds      (PicmanDisplayShell       *shell,
                                               gdouble                 x1,
                                               gdouble                 y1,
                                               gdouble                 x2,
                                               gdouble                 y2,
                                               gdouble                *nx1,
                                               gdouble                *ny1,
                                               gdouble                *nx2,
                                               gdouble                *ny2);


/*  transform: functions to transform from image space to rotated display
 *  space and back, taking into account scroll offset, scale, and rotation
 */

void  picman_display_shell_transform_coords     (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *image_coords,
                                               PicmanCoords             *display_coords);
void  picman_display_shell_untransform_coords   (const PicmanDisplayShell *shell,
                                               const PicmanCoords       *display_coords,
                                               PicmanCoords             *image_coords);

void  picman_display_shell_transform_xy         (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gint                   *nx,
                                               gint                   *ny);
void  picman_display_shell_untransform_xy       (const PicmanDisplayShell *shell,
                                               gint                    x,
                                               gint                    y,
                                               gint                   *nx,
                                               gint                   *ny,
                                               gboolean                round);

void  picman_display_shell_transform_xy_f       (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);
void  picman_display_shell_untransform_xy_f     (const PicmanDisplayShell *shell,
                                               gdouble                 x,
                                               gdouble                 y,
                                               gdouble                *nx,
                                               gdouble                *ny);

void  picman_display_shell_transform_bounds     (const PicmanDisplayShell *shell,
                                               gdouble                 x1,
                                               gdouble                 y1,
                                               gdouble                 x2,
                                               gdouble                 y2,
                                               gdouble                *nx1,
                                               gdouble                *ny1,
                                               gdouble                *nx2,
                                               gdouble                *ny2);
void  picman_display_shell_untransform_bounds   (const PicmanDisplayShell *shell,
                                               gdouble                 x1,
                                               gdouble                 y1,
                                               gdouble                 x2,
                                               gdouble                 y2,
                                               gdouble                *nx1,
                                               gdouble                *ny1,
                                               gdouble                *nx2,
                                               gdouble                *ny2);

void  picman_display_shell_untransform_viewport (const PicmanDisplayShell *shell,
                                               gint                   *x,
                                               gint                   *y,
                                               gint                   *width,
                                               gint                   *height);


#endif /* __PICMAN_DISPLAY_SHELL_TRANSFORM_H__ */
