/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvasprogress.h
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

#ifndef __PICMAN_CANVAS_PROGRESS_H__
#define __PICMAN_CANVAS_PROGRESS_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_PROGRESS            (picman_canvas_progress_get_type ())
#define PICMAN_CANVAS_PROGRESS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_PROGRESS, PicmanCanvasProgress))
#define PICMAN_CANVAS_PROGRESS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_PROGRESS, PicmanCanvasProgressClass))
#define PICMAN_IS_CANVAS_PROGRESS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_PROGRESS))
#define PICMAN_IS_CANVAS_PROGRESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_PROGRESS))
#define PICMAN_CANVAS_PROGRESS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_PROGRESS, PicmanCanvasProgressClass))


typedef struct _PicmanCanvasProgress      PicmanCanvasProgress;
typedef struct _PicmanCanvasProgressClass PicmanCanvasProgressClass;

struct _PicmanCanvasProgress
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasProgressClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_progress_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_progress_new      (PicmanDisplayShell *shell,
                                                PicmanHandleAnchor  anchor,
                                                gdouble           x,
                                                gdouble           y);


#endif /* __PICMAN_CANVAS_PROGRESS_H__ */
