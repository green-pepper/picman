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

#ifndef  __PICMAN_DRAWABLE_BLEND_H__
#define  __PICMAN_DRAWABLE_BLEND_H__


void   picman_drawable_blend (PicmanDrawable         *drawable,
                            PicmanContext          *context,
                            PicmanBlendMode         blend_mode,
                            PicmanLayerModeEffects  paint_mode,
                            PicmanGradientType      gradient_type,
                            gdouble               opacity,
                            gdouble               offset,
                            PicmanRepeatMode        repeat,
                            gboolean              reverse,
                            gboolean              supersample,
                            gint                  max_depth,
                            gdouble               threshold,
                            gboolean              dither,
                            gdouble               startx,
                            gdouble               starty,
                            gdouble               endx,
                            gdouble               endy,
                            PicmanProgress         *progress);


#endif /* __PICMAN_DRAWABLE_BLEND_H__ */
