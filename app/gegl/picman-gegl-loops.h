/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-gegl-loops.h
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_GEGL_LOOPS_H__
#define __PICMAN_GEGL_LOOPS_H__


/*  this is a pretty stupid port of concolve_region() that only works
 *  on a linear source buffer
 */
void   picman_gegl_convolve           (GeglBuffer          *src_buffer,
                                     const GeglRectangle *src_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     const gfloat        *kernel,
                                     gint                 kernel_size,
                                     gdouble              divisor,
                                     PicmanConvolutionType  mode,
                                     gboolean             alpha_weighting);

void   picman_gegl_dodgeburn          (GeglBuffer          *src_buffer,
                                     const GeglRectangle *src_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              exposure,
                                     PicmanDodgeBurnType    type,
                                     PicmanTransferMode     mode);

void   picman_gegl_smudge_blend       (GeglBuffer          *top_buffer,
                                     const GeglRectangle *top_rect,
                                     GeglBuffer          *bottom_buffer,
                                     const GeglRectangle *bottom_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              blend);

void   picman_gegl_apply_mask         (GeglBuffer          *mask_buffer,
                                     const GeglRectangle *mask_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              opacity);

void   picman_gegl_combine_mask       (GeglBuffer          *mask_buffer,
                                     const GeglRectangle *mask_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              opacity);

void   picman_gegl_combine_mask_weird (GeglBuffer          *mask_buffer,
                                     const GeglRectangle *mask_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              opacity,
                                     gboolean             stipple);

void   picman_gegl_replace            (GeglBuffer          *top_buffer,
                                     const GeglRectangle *top_rect,
                                     GeglBuffer          *bottom_buffer,
                                     const GeglRectangle *bottom_rect,
                                     GeglBuffer          *mask_buffer,
                                     const GeglRectangle *mask_rect,
                                     GeglBuffer          *dest_buffer,
                                     const GeglRectangle *dest_rect,
                                     gdouble              opacity,
                                     const gboolean      *affect);


#endif /* __PICMAN_GEGL_LOOPS_H__ */
