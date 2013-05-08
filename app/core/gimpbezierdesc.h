/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbezierdesc.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_BEZIER_DESC_H__
#define __PICMAN_BEZIER_DESC_H__


#define          PICMAN_TYPE_BEZIER_DESC     (picman_bezier_desc_get_type ())

GType            picman_bezier_desc_get_type (void) G_GNUC_CONST;


/* takes ownership of "data" */
PicmanBezierDesc * picman_bezier_desc_new                 (cairo_path_data_t    *data,
                                                       gint                  n_data);

/* expects sorted PicmanBoundSegs */
PicmanBezierDesc * picman_bezier_desc_new_from_bound_segs (PicmanBoundSeg         *bound_segs,
                                                       gint                  n_bound_segs,
                                                       gint                  n_bound_groups);

void             picman_bezier_desc_translate           (PicmanBezierDesc       *desc,
                                                       gdouble               offset_x,
                                                       gdouble               offset_y);

PicmanBezierDesc * picman_bezier_desc_copy                (const PicmanBezierDesc *desc);
void             picman_bezier_desc_free                (PicmanBezierDesc       *desc);


#endif /* __PICMAN_BEZIER_DESC_H__ */
