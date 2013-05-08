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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "display-types.h"

#include "core/picmanboundary.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picman-utils.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-transform.h"


/**
 * picman_display_shell_zoom_coords:
 * @shell:          a #PicmanDisplayShell
 * @image_coords:   image coordinates
 * @display_coords: returns the corresponding display coordinates
 *
 * Zooms from image coordinates to display coordinates, so that
 * objects can be rendered at the correct points on the display.
 **/
void
picman_display_shell_zoom_coords (const PicmanDisplayShell *shell,
                                const PicmanCoords       *image_coords,
                                PicmanCoords             *display_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (image_coords != NULL);
  g_return_if_fail (display_coords != NULL);

  *display_coords = *image_coords;

  display_coords->x = SCALEX (shell, image_coords->x);
  display_coords->y = SCALEY (shell, image_coords->y);

  display_coords->x -= shell->offset_x;
  display_coords->y -= shell->offset_y;
}

/**
 * picman_display_shell_unzoom_coords:
 * @shell:          a #PicmanDisplayShell
 * @display_coords: display coordinates
 * @image_coords:   returns the corresponding image coordinates
 *
 * Zooms from display coordinates to image coordinates, so that
 * points on the display can be mapped to points in the image.
 **/
void
picman_display_shell_unzoom_coords (const PicmanDisplayShell *shell,
                                  const PicmanCoords       *display_coords,
                                  PicmanCoords             *image_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (display_coords != NULL);
  g_return_if_fail (image_coords != NULL);

  *image_coords = *display_coords;

  image_coords->x += shell->offset_x;
  image_coords->y += shell->offset_y;

  image_coords->x /= shell->scale_x;
  image_coords->y /= shell->scale_y;
}

/**
 * picman_display_shell_zoom_xy:
 * @shell:
 * @x:
 * @y:
 * @nx:
 * @ny:
 *
 * Zooms an image coordinate to a shell coordinate.
 **/
void
picman_display_shell_zoom_xy (const PicmanDisplayShell *shell,
                            gdouble                 x,
                            gdouble                 y,
                            gint                   *nx,
                            gint                   *ny)
{
  gint64 tx;
  gint64 ty;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  tx = x * shell->scale_x;
  ty = y * shell->scale_y;

  tx -= shell->offset_x;
  ty -= shell->offset_y;

  /*  The projected coordinates might overflow a gint in the case of
   *  big images at high zoom levels, so we clamp them here to avoid
   *  problems.
   */
  *nx = CLAMP (tx, G_MININT, G_MAXINT);
  *ny = CLAMP (ty, G_MININT, G_MAXINT);
}

/**
 * picman_display_shell_unzoom_xy:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in display coordinates
 * @y:           y coordinate in display coordinates
 * @nx:          returns x oordinate in image coordinates
 * @ny:          returns y coordinate in image coordinates
 * @round:       if %TRUE, round the results to the nearest integer;
 *               if %FALSE, simply cast them to @gint.
 *
 * Zoom from display coordinates to image coordinates, so that
 * points on the display can be mapped to the corresponding points
 * in the image.
 **/
void
picman_display_shell_unzoom_xy (const PicmanDisplayShell *shell,
                              gint                    x,
                              gint                    y,
                              gint                   *nx,
                              gint                   *ny,
                              gboolean                round)
{
  gint64 tx;
  gint64 ty;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  if (round)
    {
      tx = SIGNED_ROUND (((gdouble) x + shell->offset_x) / shell->scale_x);
      ty = SIGNED_ROUND (((gdouble) y + shell->offset_y) / shell->scale_y);
    }
  else
    {
      tx = ((gint64) x + shell->offset_x) / shell->scale_x;
      ty = ((gint64) y + shell->offset_y) / shell->scale_y;
    }

  *nx = CLAMP (tx, G_MININT, G_MAXINT);
  *ny = CLAMP (ty, G_MININT, G_MAXINT);
}

/**
 * picman_display_shell_zoom_xy_f:
 * @shell: a #PicmanDisplayShell
 * @x:     image x coordinate of point
 * @y:     image y coordinate of point
 * @nx:    returned shell canvas x coordinate
 * @ny:    returned shell canvas y coordinate
 *
 * Zooms from image coordinates to display shell canvas
 * coordinates.
 **/
void
picman_display_shell_zoom_xy_f (const PicmanDisplayShell *shell,
                              gdouble                 x,
                              gdouble                 y,
                              gdouble                *nx,
                              gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  *nx = SCALEX (shell, x) - shell->offset_x;
  *ny = SCALEY (shell, y) - shell->offset_y;
}

/**
 * picman_display_shell_unzoom_xy_f:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in display coordinates
 * @y:           y coordinate in display coordinates
 * @nx:          place to return x coordinate in image coordinates
 * @ny:          place to return y coordinate in image coordinates
 *
 * This function is identical to picman_display_shell_unzoom_xy(),
 * except that the input and output coordinates are doubles rather than
 * ints, and consequently there is no option related to rounding.
 **/
void
picman_display_shell_unzoom_xy_f (const PicmanDisplayShell *shell,
                                gdouble                 x,
                                gdouble                 y,
                                gdouble                *nx,
                                gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  *nx = (x + shell->offset_x) / shell->scale_x;
  *ny = (y + shell->offset_y) / shell->scale_y;
}

/**
 * picman_display_shell_zoom_segments:
 * @shell:       a #PicmanDisplayShell
 * @src_segs:    array of segments in image coordinates
 * @dest_segs:   returns the corresponding segments in display coordinates
 * @n_segs:      number of segments
 *
 * Zooms from image coordinates to display coordinates, so that
 * objects can be rendered at the correct points on the display.
 **/
void
picman_display_shell_zoom_segments (const PicmanDisplayShell *shell,
                                  const PicmanBoundSeg     *src_segs,
                                  PicmanSegment            *dest_segs,
                                  gint                    n_segs,
                                  gdouble                 offset_x,
                                  gdouble                 offset_y)
{
  gint i;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  for (i = 0; i < n_segs ; i++)
    {
      gdouble x1, x2;
      gdouble y1, y2;

      x1 = src_segs[i].x1 + offset_x;
      x2 = src_segs[i].x2 + offset_x;
      y1 = src_segs[i].y1 + offset_y;
      y2 = src_segs[i].y2 + offset_y;

      dest_segs[i].x1 = SCALEX (shell, x1) - shell->offset_x;
      dest_segs[i].x2 = SCALEX (shell, x2) - shell->offset_x;
      dest_segs[i].y1 = SCALEY (shell, y1) - shell->offset_y;
      dest_segs[i].y2 = SCALEY (shell, y2) - shell->offset_y;
    }
}

/**
 * picman_display_shell_rotate_coords:
 * @shell:          a #PicmanDisplayShell
 * @image_coords:   unrotated display coordinates
 * @display_coords: returns the corresponding rotated display coordinates
 *
 * Rotates from unrotated display coordinates to rotated display
 * coordinates, so that objects can be rendered at the correct points
 * on the display.
 **/
void
picman_display_shell_rotate_coords (const PicmanDisplayShell *shell,
                                  const PicmanCoords       *unrotated_coords,
                                  PicmanCoords             *rotated_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (unrotated_coords != NULL);
  g_return_if_fail (rotated_coords != NULL);

  *rotated_coords = *rotated_coords;

  if (shell->rotate_transform)
    cairo_matrix_transform_point (shell->rotate_transform,
                                  &rotated_coords->x,
                                  &rotated_coords->y);
}

/**
 * picman_display_shell_unrotate_coords:
 * @shell:          a #PicmanDisplayShell
 * @display_coords: rotated display coordinates
 * @image_coords:   returns the corresponding unrotated display coordinates
 *
 * Rotates from rotated display coordinates to unrotated display coordinates.
 **/
void
picman_display_shell_unrotate_coords (const PicmanDisplayShell *shell,
                                    const PicmanCoords       *rotated_coords,
                                    PicmanCoords             *unrotated_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (rotated_coords != NULL);
  g_return_if_fail (unrotated_coords != NULL);

  *unrotated_coords = *rotated_coords;

  if (shell->rotate_untransform)
    cairo_matrix_transform_point (shell->rotate_untransform,
                                  &unrotated_coords->x,
                                  &unrotated_coords->y);
}

/**
 * picman_display_shell_rotate_xy:
 * @shell:
 * @x:
 * @y:
 * @nx:
 * @ny:
 *
 * Rotates an unrotated display coordinate to a rotated shell coordinate.
 **/
void
picman_display_shell_rotate_xy (const PicmanDisplayShell *shell,
                              gdouble                 x,
                              gdouble                 y,
                              gint                   *nx,
                              gint                   *ny)
{
  gint64 tx;
  gint64 ty;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  if (shell->rotate_transform)
    cairo_matrix_transform_point (shell->rotate_transform, &x, &y);

  tx = x;
  ty = y;

  /*  The projected coordinates might overflow a gint in the case of
   *  big images at high zoom levels, so we clamp them here to avoid
   *  problems.
   */
  *nx = CLAMP (tx, G_MININT, G_MAXINT);
  *ny = CLAMP (ty, G_MININT, G_MAXINT);
}

/**
 * picman_display_shell_unrotate_xy:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in rotated display coordinates
 * @y:           y coordinate in rotated display coordinates
 * @nx:          returns x oordinate in unrotated display coordinates
 * @ny:          returns y coordinate in unrotated display coordinates
 *
 * Rotate from rotated display coordinates to unrotated display
 * coordinates.
 **/
void
picman_display_shell_unrotate_xy (const PicmanDisplayShell *shell,
                                gint                    x,
                                gint                    y,
                                gint                   *nx,
                                gint                   *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  if (shell->rotate_untransform)
    {
      gdouble fx = x;
      gdouble fy = y;

      cairo_matrix_transform_point (shell->rotate_untransform, &fy, &fy);

      *nx = CLAMP (fx, G_MININT, G_MAXINT);
      *ny = CLAMP (fy, G_MININT, G_MAXINT);
    }
  else
    {
      *nx = x;
      *ny = y;
    }
}

/**
 * picman_display_shell_rotate_xy_f:
 * @shell: a #PicmanDisplayShell
 * @x:     image x coordinate of point
 * @y:     image y coordinate of point
 * @nx:    returned shell canvas x coordinate
 * @ny:    returned shell canvas y coordinate
 *
 * Rotates from untransformed display coordinates to rotated display
 * coordinates.
 **/
void
picman_display_shell_rotate_xy_f (const PicmanDisplayShell *shell,
                                gdouble                 x,
                                gdouble                 y,
                                gdouble                *nx,
                                gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  *nx = x;
  *ny = y;

  if (shell->rotate_transform)
    cairo_matrix_transform_point (shell->rotate_transform, nx, ny);
}

/**
 * picman_display_shell_unrotate_xy_f:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in rotated display coordinates
 * @y:           y coordinate in rotated display coordinates
 * @nx:          place to return x coordinate in unrotated display coordinates
 * @ny:          place to return y coordinate in unrotated display  coordinates
 *
 * This function is identical to picman_display_shell_unrotate_xy(),
 * except that the input and output coordinates are doubles rather
 * than ints.
 **/
void
picman_display_shell_unrotate_xy_f (const PicmanDisplayShell *shell,
                                  gdouble                 x,
                                  gdouble                 y,
                                  gdouble                *nx,
                                  gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  *nx = x;
  *ny = y;

  if (shell->rotate_untransform)
    cairo_matrix_transform_point (shell->rotate_untransform, nx, ny);
}

void
picman_display_shell_rotate_bounds (PicmanDisplayShell *shell,
                                  gdouble           x1,
                                  gdouble           y1,
                                  gdouble           x2,
                                  gdouble           y2,
                                  gdouble          *nx1,
                                  gdouble          *ny1,
                                  gdouble          *nx2,
                                  gdouble          *ny2)
{

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->rotate_transform)
    {
      gdouble tx1 = x1;
      gdouble ty1 = y1;
      gdouble tx2 = x1;
      gdouble ty2 = y2;
      gdouble tx3 = x2;
      gdouble ty3 = y1;
      gdouble tx4 = x2;
      gdouble ty4 = y2;

      cairo_matrix_transform_point (shell->rotate_transform, &tx1, &ty1);
      cairo_matrix_transform_point (shell->rotate_transform, &tx2, &ty2);
      cairo_matrix_transform_point (shell->rotate_transform, &tx3, &ty3);
      cairo_matrix_transform_point (shell->rotate_transform, &tx4, &ty4);

      *nx1 = MIN4 (tx1, tx2, tx3, tx4);
      *ny1 = MIN4 (ty1, ty2, ty3, ty4);
      *nx2 = MAX4 (tx1, tx2, tx3, tx4);
      *ny2 = MAX4 (ty1, ty2, ty3, ty4);
    }
  else
    {
      *nx1 = x1;
      *ny1 = y1;
      *nx2 = x2;
      *ny2 = y2;
    }
}

void
picman_display_shell_unrotate_bounds (PicmanDisplayShell *shell,
                                    gdouble           x1,
                                    gdouble           y1,
                                    gdouble           x2,
                                    gdouble           y2,
                                    gdouble          *nx1,
                                    gdouble          *ny1,
                                    gdouble          *nx2,
                                    gdouble          *ny2)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->rotate_untransform)
    {
      gdouble tx1 = x1;
      gdouble ty1 = y1;
      gdouble tx2 = x1;
      gdouble ty2 = y2;
      gdouble tx3 = x2;
      gdouble ty3 = y1;
      gdouble tx4 = x2;
      gdouble ty4 = y2;

      cairo_matrix_transform_point (shell->rotate_untransform, &tx1, &ty1);
      cairo_matrix_transform_point (shell->rotate_untransform, &tx2, &ty2);
      cairo_matrix_transform_point (shell->rotate_untransform, &tx3, &ty3);
      cairo_matrix_transform_point (shell->rotate_untransform, &tx4, &ty4);

      *nx1 = MIN4 (tx1, tx2, tx3, tx4);
      *ny1 = MIN4 (ty1, ty2, ty3, ty4);
      *nx2 = MAX4 (tx1, tx2, tx3, tx4);
      *ny2 = MAX4 (ty1, ty2, ty3, ty4);
    }
  else
    {
      *nx1 = x1;
      *ny1 = y1;
      *nx2 = x2;
      *ny2 = y2;
    }
}

/**
 * picman_display_shell_transform_coords:
 * @shell:          a #PicmanDisplayShell
 * @image_coords:   image coordinates
 * @display_coords: returns the corresponding display coordinates
 *
 * Transforms from image coordinates to display coordinates, so that
 * objects can be rendered at the correct points on the display.
 **/
void
picman_display_shell_transform_coords (const PicmanDisplayShell *shell,
                                     const PicmanCoords       *image_coords,
                                     PicmanCoords             *display_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (image_coords != NULL);
  g_return_if_fail (display_coords != NULL);

  *display_coords = *image_coords;

  display_coords->x = SCALEX (shell, image_coords->x);
  display_coords->y = SCALEY (shell, image_coords->y);

  display_coords->x -= shell->offset_x;
  display_coords->y -= shell->offset_y;

  if (shell->rotate_transform)
    cairo_matrix_transform_point (shell->rotate_transform,
                                  &display_coords->x,
                                  &display_coords->y);
}

/**
 * picman_display_shell_untransform_coords:
 * @shell:          a #PicmanDisplayShell
 * @display_coords: display coordinates
 * @image_coords:   returns the corresponding image coordinates
 *
 * Transforms from display coordinates to image coordinates, so that
 * points on the display can be mapped to points in the image.
 **/
void
picman_display_shell_untransform_coords (const PicmanDisplayShell *shell,
                                       const PicmanCoords       *display_coords,
                                       PicmanCoords             *image_coords)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (display_coords != NULL);
  g_return_if_fail (image_coords != NULL);

  *image_coords = *display_coords;

  if (shell->rotate_untransform)
    cairo_matrix_transform_point (shell->rotate_untransform,
                                  &image_coords->x,
                                  &image_coords->y);

  image_coords->x += shell->offset_x;
  image_coords->y += shell->offset_y;

  image_coords->x /= shell->scale_x;
  image_coords->y /= shell->scale_y;
}

/**
 * picman_display_shell_transform_xy:
 * @shell:
 * @x:
 * @y:
 * @nx:
 * @ny:
 *
 * Transforms an image coordinate to a shell coordinate.
 **/
void
picman_display_shell_transform_xy (const PicmanDisplayShell *shell,
                                 gdouble                 x,
                                 gdouble                 y,
                                 gint                   *nx,
                                 gint                   *ny)
{
  gint64 tx;
  gint64 ty;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  tx = x * shell->scale_x;
  ty = y * shell->scale_y;

  tx -= shell->offset_x;
  ty -= shell->offset_y;

  if (shell->rotate_transform)
    {
      gdouble fx = tx;
      gdouble fy = ty;

      cairo_matrix_transform_point (shell->rotate_transform, &fx, &fy);

      tx = fx;
      ty = fy;
    }

  /*  The projected coordinates might overflow a gint in the case of
   *  big images at high zoom levels, so we clamp them here to avoid
   *  problems.
   */
  *nx = CLAMP (tx, G_MININT, G_MAXINT);
  *ny = CLAMP (ty, G_MININT, G_MAXINT);
}

/**
 * picman_display_shell_untransform_xy:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in display coordinates
 * @y:           y coordinate in display coordinates
 * @nx:          returns x oordinate in image coordinates
 * @ny:          returns y coordinate in image coordinates
 * @round:       if %TRUE, round the results to the nearest integer;
 *               if %FALSE, simply cast them to @gint.
 *
 * Transform from display coordinates to image coordinates, so that
 * points on the display can be mapped to the corresponding points
 * in the image.
 **/
void
picman_display_shell_untransform_xy (const PicmanDisplayShell *shell,
                                   gint                    x,
                                   gint                    y,
                                   gint                   *nx,
                                   gint                   *ny,
                                   gboolean                round)
{
  gint64 tx;
  gint64 ty;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  if (shell->rotate_untransform)
    {
      gdouble fx = x;
      gdouble fy = y;

      cairo_matrix_transform_point (shell->rotate_untransform, &fy, &fy);

      x = fx;
      y = fy;
    }

  if (round)
    {
      tx = SIGNED_ROUND (((gdouble) x + shell->offset_x) / shell->scale_x);
      ty = SIGNED_ROUND (((gdouble) y + shell->offset_y) / shell->scale_y);
    }
  else
    {
      tx = ((gint64) x + shell->offset_x) / shell->scale_x;
      ty = ((gint64) y + shell->offset_y) / shell->scale_y;
    }

  *nx = CLAMP (tx, G_MININT, G_MAXINT);
  *ny = CLAMP (ty, G_MININT, G_MAXINT);
}

/**
 * picman_display_shell_transform_xy_f:
 * @shell: a #PicmanDisplayShell
 * @x:     image x coordinate of point
 * @y:     image y coordinate of point
 * @nx:    returned shell canvas x coordinate
 * @ny:    returned shell canvas y coordinate
 *
 * Transforms from image coordinates to display shell canvas
 * coordinates.
 **/
void
picman_display_shell_transform_xy_f  (const PicmanDisplayShell *shell,
                                    gdouble                 x,
                                    gdouble                 y,
                                    gdouble                *nx,
                                    gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  *nx = SCALEX (shell, x) - shell->offset_x;
  *ny = SCALEY (shell, y) - shell->offset_y;

  if (shell->rotate_transform)
    cairo_matrix_transform_point (shell->rotate_transform, nx, ny);
}

/**
 * picman_display_shell_untransform_xy_f:
 * @shell:       a #PicmanDisplayShell
 * @x:           x coordinate in display coordinates
 * @y:           y coordinate in display coordinates
 * @nx:          place to return x coordinate in image coordinates
 * @ny:          place to return y coordinate in image coordinates
 *
 * This function is identical to picman_display_shell_untransform_xy(),
 * except that the input and output coordinates are doubles rather than
 * ints, and consequently there is no option related to rounding.
 **/
void
picman_display_shell_untransform_xy_f (const PicmanDisplayShell *shell,
                                     gdouble                 x,
                                     gdouble                 y,
                                     gdouble                *nx,
                                     gdouble                *ny)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx != NULL);
  g_return_if_fail (ny != NULL);

  if (shell->rotate_untransform)
    cairo_matrix_transform_point (shell->rotate_untransform, &x, &y);

  *nx = (x + shell->offset_x) / shell->scale_x;
  *ny = (y + shell->offset_y) / shell->scale_y;
}

void
picman_display_shell_transform_bounds (const PicmanDisplayShell *shell,
                                     gdouble                 x1,
                                     gdouble                 y1,
                                     gdouble                 x2,
                                     gdouble                 y2,
                                     gdouble                *nx1,
                                     gdouble                *ny1,
                                     gdouble                *nx2,
                                     gdouble                *ny2)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx1 != NULL);
  g_return_if_fail (ny1 != NULL);
  g_return_if_fail (nx2 != NULL);
  g_return_if_fail (ny2 != NULL);

  if (shell->rotate_transform)
    {
      gdouble tx1, ty1;
      gdouble tx2, ty2;
      gdouble tx3, ty3;
      gdouble tx4, ty4;

      picman_display_shell_transform_xy_f (shell, x1, y1, &tx1, &ty1);
      picman_display_shell_transform_xy_f (shell, x1, y2, &tx2, &ty2);
      picman_display_shell_transform_xy_f (shell, x2, y1, &tx3, &ty3);
      picman_display_shell_transform_xy_f (shell, x2, y2, &tx4, &ty4);

      *nx1 = MIN4 (tx1, tx2, tx3, tx4);
      *ny1 = MIN4 (ty1, ty2, ty3, ty4);
      *nx2 = MAX4 (tx1, tx2, tx3, tx4);
      *ny2 = MAX4 (ty1, ty2, ty3, ty4);
    }
  else
    {
      picman_display_shell_transform_xy_f (shell, x1, y1, nx1, ny1);
      picman_display_shell_transform_xy_f (shell, x2, y2, nx2, ny2);
    }
}

void
picman_display_shell_untransform_bounds (const PicmanDisplayShell *shell,
                                       gdouble                 x1,
                                       gdouble                 y1,
                                       gdouble                 x2,
                                       gdouble                 y2,
                                       gdouble                *nx1,
                                       gdouble                *ny1,
                                       gdouble                *nx2,
                                       gdouble                *ny2)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (nx1 != NULL);
  g_return_if_fail (ny1 != NULL);
  g_return_if_fail (nx2 != NULL);
  g_return_if_fail (ny2 != NULL);

  if (shell->rotate_untransform)
    {
      gdouble tx1, ty1;
      gdouble tx2, ty2;
      gdouble tx3, ty3;
      gdouble tx4, ty4;

      picman_display_shell_untransform_xy_f (shell, x1, y1, &tx1, &ty1);
      picman_display_shell_untransform_xy_f (shell, x1, y2, &tx2, &ty2);
      picman_display_shell_untransform_xy_f (shell, x2, y1, &tx3, &ty3);
      picman_display_shell_untransform_xy_f (shell, x2, y2, &tx4, &ty4);

      *nx1 = MIN4 (tx1, tx2, tx3, tx4);
      *ny1 = MIN4 (ty1, ty2, ty3, ty4);
      *nx2 = MAX4 (tx1, tx2, tx3, tx4);
      *ny2 = MAX4 (ty1, ty2, ty3, ty4);
    }
  else
    {
      picman_display_shell_untransform_xy_f (shell, x1, y1, nx1, ny1);
      picman_display_shell_untransform_xy_f (shell, x2, y2, nx2, ny2);
    }
}

/**
 * picman_display_shell_untransform_viewport:
 * @shell:  a #PicmanDisplayShell
 * @x:      returns image x coordinate of display upper left corner
 * @y:      returns image y coordinate of display upper left corner
 * @width:  returns width of display measured in image coordinates
 * @height: returns height of display measured in image coordinates
 *
 * This function calculates the part of the image, im image coordinates,
 * that corresponds to the display viewport.
 **/
void
picman_display_shell_untransform_viewport (const PicmanDisplayShell *shell,
                                         gint                   *x,
                                         gint                   *y,
                                         gint                   *width,
                                         gint                   *height)
{
  PicmanImage *image;
  gint       x1, y1, x2, y2;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_untransform_xy (shell,
                                     0, 0,
                                     &x1, &y1,
                                     FALSE);
  picman_display_shell_untransform_xy (shell,
                                     shell->disp_width, shell->disp_height,
                                     &x2, &y2,
                                     FALSE);

  image = picman_display_get_image (shell->display);

  if (x1 < 0)
    x1 = 0;

  if (y1 < 0)
    y1 = 0;

  if (x2 > picman_image_get_width (image))
    x2 = picman_image_get_width (image);

  if (y2 > picman_image_get_height (image))
    y2 = picman_image_get_height (image);

  if (x)      *x      = x1;
  if (y)      *y      = y1;
  if (width)  *width  = x2 - x1;
  if (height) *height = y2 - y1;
}
