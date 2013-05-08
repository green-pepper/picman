/* PICMAN - The GNU Image Manipulation Program Copyright (C) 1995
 * Spencer Kimball and Peter Mattis
 *
 * picmancanvaspolygon.h
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

#ifndef __PICMAN_CANVAS_POLYGON_H__
#define __PICMAN_CANVAS_POLYGON_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_POLYGON            (picman_canvas_polygon_get_type ())
#define PICMAN_CANVAS_POLYGON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_POLYGON, PicmanCanvasPolygon))
#define PICMAN_CANVAS_POLYGON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_POLYGON, PicmanCanvasPolygonClass))
#define PICMAN_IS_CANVAS_POLYGON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_POLYGON))
#define PICMAN_IS_CANVAS_POLYGON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_POLYGON))
#define PICMAN_CANVAS_POLYGON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_POLYGON, PicmanCanvasPolygonClass))


typedef struct _PicmanCanvasPolygon      PicmanCanvasPolygon;
typedef struct _PicmanCanvasPolygonClass PicmanCanvasPolygonClass;

struct _PicmanCanvasPolygon
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasPolygonClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_polygon_get_type        (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_polygon_new             (PicmanDisplayShell  *shell,
                                                      const PicmanVector2 *points,
                                                      gint               n_points,
                                                      gboolean           filled);
PicmanCanvasItem * picman_canvas_polygon_new_from_coords (PicmanDisplayShell  *shell,
                                                      const PicmanCoords  *coords,
                                                      gint               n_coords,
                                                      gboolean           filled);


#endif /* __PICMAN_CANVAS_POLYGON_H__ */
