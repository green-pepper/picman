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

#ifndef __PICMAN_GEGL_MASK_COMBINE_H__
#define __PICMAN_GEGL_MASK_COMBINE_H__


gboolean   picman_gegl_mask_combine_rect         (GeglBuffer     *mask,
                                                PicmanChannelOps  op,
                                                gint            x,
                                                gint            y,
                                                gint            w,
                                                gint            h);
gboolean   picman_gegl_mask_combine_ellipse      (GeglBuffer     *mask,
                                                PicmanChannelOps  op,
                                                gint            x,
                                                gint            y,
                                                gint            w,
                                                gint            h,
                                                gboolean        antialias);
gboolean   picman_gegl_mask_combine_ellipse_rect (GeglBuffer     *mask,
                                                PicmanChannelOps  op,
                                                gint            x,
                                                gint            y,
                                                gint            w,
                                                gint            h,
                                                gdouble         a,
                                                gdouble         b,
                                                gboolean        antialias);
gboolean   picman_gegl_mask_combine_buffer       (GeglBuffer     *mask,
                                                GeglBuffer     *add_on,
                                                PicmanChannelOps  op,
                                                gint            off_x,
                                                gint            off_y);


#endif /* __PICMAN_GEGL_MASK_COMBINE_H__ */
