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

#ifndef __PICMAN_PAINT_CORE_STROKE_H__
#define __PICMAN_PAINT_CORE_STROKE_H__


gboolean   picman_paint_core_stroke          (PicmanPaintCore      *core,
                                            PicmanDrawable       *drawable,
                                            PicmanPaintOptions   *paint_options,
                                            PicmanCoords         *strokes,
                                            gint                n_strokes,
                                            gboolean            push_undo,
                                            GError            **error);
gboolean   picman_paint_core_stroke_boundary (PicmanPaintCore      *core,
                                            PicmanDrawable       *drawable,
                                            PicmanPaintOptions   *paint_options,
                                            gboolean            emulate_dynamics,
                                            const PicmanBoundSeg *bound_segs,
                                            gint                n_bound_segs,
                                            gint                offset_x,
                                            gint                offset_y,
                                            gboolean            push_undo,
                                            GError            **error);
gboolean   picman_paint_core_stroke_vectors  (PicmanPaintCore      *core,
                                            PicmanDrawable       *drawable,
                                            PicmanPaintOptions   *paint_options,
                                            gboolean            emulate_dynamics,
                                            PicmanVectors        *vectors,
                                            gboolean            push_undo,
                                            GError            **error);


#endif  /*  __PICMAN_PAINT_CORE_STROKE_H__  */
