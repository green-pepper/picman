/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectors-compat.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VECTORS_COMPAT_H__
#define __PICMAN_VECTORS_COMPAT_H__


typedef struct _PicmanVectorsCompatPoint PicmanVectorsCompatPoint;

struct _PicmanVectorsCompatPoint
{
  guint32 type;
  gdouble x;
  gdouble y;
};


PicmanVectors * picman_vectors_compat_new (PicmanImage              *image,
                                       const gchar            *name,
                                       PicmanVectorsCompatPoint *points,
                                       gint                    n_points,
                                       gboolean                closed);

gboolean              picman_vectors_compat_is_compatible (PicmanImage   *image);

PicmanVectorsCompatPoint * picman_vectors_compat_get_points (PicmanVectors *vectors,
                                                         gint32      *n_points,
                                                         gint32      *closed);


#endif /* __PICMAN_VECTORS_COMPAT_H__ */
