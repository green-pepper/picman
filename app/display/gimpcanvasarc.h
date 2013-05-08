/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasarc.h
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

#ifndef __PICMAN_CANVAS_ARC_H__
#define __PICMAN_CANVAS_ARC_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_ARC            (picman_canvas_arc_get_type ())
#define PICMAN_CANVAS_ARC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_ARC, PicmanCanvasArc))
#define PICMAN_CANVAS_ARC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_ARC, PicmanCanvasArcClass))
#define PICMAN_IS_CANVAS_ARC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_ARC))
#define PICMAN_IS_CANVAS_ARC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_ARC))
#define PICMAN_CANVAS_ARC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_ARC, PicmanCanvasArcClass))


typedef struct _PicmanCanvasArc      PicmanCanvasArc;
typedef struct _PicmanCanvasArcClass PicmanCanvasArcClass;

struct _PicmanCanvasArc
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasArcClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_arc_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_arc_new      (PicmanDisplayShell *shell,
                                           gdouble          center_x,
                                           gdouble          center_y,
                                           gdouble          radius_x,
                                           gdouble          radius_y,
                                           gdouble          start_angle,
                                           gdouble          slice_angle,
                                           gboolean         filled);


#endif /* __PICMAN_CANVAS_ARC_H__ */
