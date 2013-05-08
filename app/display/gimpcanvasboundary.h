/* PICMAN - The GNU Image Manipulation Program Copyright (C) 1995
 * Spencer Kimball and Peter Mattis
 *
 * picmancanvasboundary.h
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

#ifndef __PICMAN_CANVAS_BOUNDARY_H__
#define __PICMAN_CANVAS_BOUNDARY_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_BOUNDARY            (picman_canvas_boundary_get_type ())
#define PICMAN_CANVAS_BOUNDARY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_BOUNDARY, PicmanCanvasBoundary))
#define PICMAN_CANVAS_BOUNDARY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_BOUNDARY, PicmanCanvasBoundaryClass))
#define PICMAN_IS_CANVAS_BOUNDARY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_BOUNDARY))
#define PICMAN_IS_CANVAS_BOUNDARY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_BOUNDARY))
#define PICMAN_CANVAS_BOUNDARY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_BOUNDARY, PicmanCanvasBoundaryClass))


typedef struct _PicmanCanvasBoundary      PicmanCanvasBoundary;
typedef struct _PicmanCanvasBoundaryClass PicmanCanvasBoundaryClass;

struct _PicmanCanvasBoundary
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasBoundaryClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_boundary_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_boundary_new      (PicmanDisplayShell   *shell,
                                                const PicmanBoundSeg *segs,
                                                gint                n_segs,
                                                PicmanMatrix3        *transform,
                                                gdouble             offset_x,
                                                gdouble             offset_y);


#endif /* __PICMAN_CANVAS_BOUNDARY_H__ */
