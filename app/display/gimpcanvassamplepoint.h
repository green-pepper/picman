/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvassamplepoint.h
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

#ifndef __PICMAN_CANVAS_SAMPLE_POINT_H__
#define __PICMAN_CANVAS_SAMPLE_POINT_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_SAMPLE_POINT            (picman_canvas_sample_point_get_type ())
#define PICMAN_CANVAS_SAMPLE_POINT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_SAMPLE_POINT, PicmanCanvasSamplePoint))
#define PICMAN_CANVAS_SAMPLE_POINT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_SAMPLE_POINT, PicmanCanvasSamplePointClass))
#define PICMAN_IS_CANVAS_SAMPLE_POINT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_SAMPLE_POINT))
#define PICMAN_IS_CANVAS_SAMPLE_POINT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_SAMPLE_POINT))
#define PICMAN_CANVAS_SAMPLE_POINT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_SAMPLE_POINT, PicmanCanvasSamplePointClass))


typedef struct _PicmanCanvasSamplePoint      PicmanCanvasSamplePoint;
typedef struct _PicmanCanvasSamplePointClass PicmanCanvasSamplePointClass;

struct _PicmanCanvasSamplePoint
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasSamplePointClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_sample_point_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_sample_point_new      (PicmanDisplayShell *shell,
                                                    gint              x,
                                                    gint              y,
                                                    gint              index,
                                                    gboolean          sample_point_style);

void             picman_canvas_sample_point_set      (PicmanCanvasItem   *sample_point,
                                                    gint              x,
                                                    gint              y);


#endif /* __PICMAN_CANVAS_SAMPLE_POINT_H__ */
