/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasrectangleguides.h
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CANVAS_RECTANGLE_GUIDES_H__
#define __PICMAN_CANVAS_RECTANGLE_GUIDES_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES            (picman_canvas_rectangle_guides_get_type ())
#define PICMAN_CANVAS_RECTANGLE_GUIDES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES, PicmanCanvasRectangleGuides))
#define PICMAN_CANVAS_RECTANGLE_GUIDES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES, PicmanCanvasRectangleGuidesClass))
#define PICMAN_IS_CANVAS_RECTANGLE_GUIDES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES))
#define PICMAN_IS_CANVAS_RECTANGLE_GUIDES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES))
#define PICMAN_CANVAS_RECTANGLE_GUIDES_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_RECTANGLE_GUIDES, PicmanCanvasRectangleGuidesClass))


typedef struct _PicmanCanvasRectangleGuides      PicmanCanvasRectangleGuides;
typedef struct _PicmanCanvasRectangleGuidesClass PicmanCanvasRectangleGuidesClass;

struct _PicmanCanvasRectangleGuides
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasRectangleGuidesClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_rectangle_guides_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_rectangle_guides_new      (PicmanDisplayShell *shell,
                                                        gdouble           x,
                                                        gdouble           y,
                                                        gdouble           width,
                                                        gdouble           height,
                                                        PicmanGuidesType    type,
                                                        gint              n_guides);

void             picman_canvas_rectangle_guides_set      (PicmanCanvasItem   *rectangle,
                                                        gdouble           x,
                                                        gdouble           y,
                                                        gdouble           width,
                                                        gdouble           height,
                                                        PicmanGuidesType    type,
                                                        gint              n_guides);


#endif /* __PICMAN_CANVAS_RECTANGLE_GUIDES_H__ */
