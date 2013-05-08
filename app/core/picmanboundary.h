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

#ifndef  __PICMAN_BOUNDARY_H__
#define  __PICMAN_BOUNDARY_H__


/* half intensity for mask */
#define PICMAN_BOUNDARY_HALF_WAY 0.5


typedef enum
{
  PICMAN_BOUNDARY_WITHIN_BOUNDS,
  PICMAN_BOUNDARY_IGNORE_BOUNDS
} PicmanBoundaryType;


struct _PicmanBoundSeg
{
  gint   x1;
  gint   y1;
  gint   x2;
  gint   y2;
  guint  open    : 1;
  guint  visited : 1;
};


PicmanBoundSeg * picman_boundary_find      (GeglBuffer          *buffer,
                                        const GeglRectangle *region,
                                        const Babl          *format,
                                        PicmanBoundaryType     type,
                                        gint                 x1,
                                        gint                 y1,
                                        gint                 x2,
                                        gint                 y2,
                                        gfloat               threshold,
                                        gint                *num_segs);
PicmanBoundSeg * picman_boundary_sort      (const PicmanBoundSeg  *segs,
                                        gint                 num_segs,
                                        gint                *num_groups);
PicmanBoundSeg * picman_boundary_simplify  (PicmanBoundSeg        *sorted_segs,
                                        gint                 num_groups,
                                        gint                *num_segs);

/* offsets in-place */
void       picman_boundary_offset        (PicmanBoundSeg        *segs,
                                        gint                 num_segs,
                                        gint                 off_x,
                                        gint                 off_y);


#endif  /*  __PICMAN_BOUNDARY_H__  */
