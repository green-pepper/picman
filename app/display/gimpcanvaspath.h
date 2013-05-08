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
#define __PICMAN_CANVAS_PATH_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_PATH            (picman_canvas_path_get_type ())
#define PICMAN_CANVAS_PATH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_PATH, PicmanCanvasPath))
#define PICMAN_CANVAS_PATH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_PATH, PicmanCanvasPathClass))
#define PICMAN_IS_CANVAS_PATH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_PATH))
#define PICMAN_IS_CANVAS_PATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_PATH))
#define PICMAN_CANVAS_PATH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_PATH, PicmanCanvasPathClass))


typedef struct _PicmanCanvasPath      PicmanCanvasPath;
typedef struct _PicmanCanvasPathClass PicmanCanvasPathClass;

struct _PicmanCanvasPath
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasPathClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_path_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_path_new      (PicmanDisplayShell     *shell,
                                            const PicmanBezierDesc *bezier,
                                            gdouble               x,
                                            gdouble               y,
                                            gboolean              filled,
                                            PicmanPathStyle         style);

void             picman_canvas_path_set      (PicmanCanvasItem       *path,
                                            const PicmanBezierDesc *bezier);


#endif /* __PICMAN_CANVAS_PATH_H__ */
