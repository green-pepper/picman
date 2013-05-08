/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanink-blob.h: routines for manipulating scan converted convex polygons.
 * Copyright 1998, Owen Taylor <otaylor@gtk.org>
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
 *
*/

#ifndef __PICMAN_INK_BLOB_H__
#define __PICMAN_INK_BLOB_H__


typedef struct _PicmanBlobPoint PicmanBlobPoint;
typedef struct _PicmanBlobSpan  PicmanBlobSpan;
typedef struct _PicmanBlob      PicmanBlob;

typedef PicmanBlob * (* PicmanBlobFunc) (gdouble xc,
                                     gdouble yc,
                                     gdouble xp,
                                     gdouble yp,
                                     gdouble xq,
                                     gdouble yq);

struct _PicmanBlobPoint
{
  gint x;
  gint y;
};

struct _PicmanBlobSpan
{
  gint left;
  gint right;
};

struct _PicmanBlob
{
  gint         y;
  gint         height;
  PicmanBlobSpan data[1];
};


PicmanBlob * picman_blob_polygon      (PicmanBlobPoint *points,
                                   gint           n_points);
PicmanBlob * picman_blob_square       (gdouble        xc,
                                   gdouble        yc,
                                   gdouble        xp,
                                   gdouble        yp,
                                   gdouble        xq,
                                   gdouble        yq);
PicmanBlob * picman_blob_diamond      (gdouble        xc,
                                   gdouble        yc,
                                   gdouble        xp,
                                   gdouble        yp,
                                   gdouble        xq,
                                   gdouble        yq);
PicmanBlob * picman_blob_ellipse      (gdouble        xc,
                                   gdouble        yc,
                                   gdouble        xp,
                                   gdouble        yp,
                                   gdouble        xq,
                                   gdouble        yq);
void       picman_blob_bounds       (PicmanBlob      *b,
                                   gint          *x,
                                   gint          *y,
                                   gint          *width,
                                   gint          *height);
PicmanBlob * picman_blob_convex_union (PicmanBlob      *b1,
                                   PicmanBlob      *b2);
PicmanBlob * picman_blob_duplicate    (PicmanBlob      *b);


#endif /* __PICMAN_INK_BLOB_H__ */
