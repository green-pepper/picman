/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvashandle.h
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

#ifndef __PICMAN_CANVAS_HANDLE_H__
#define __PICMAN_CANVAS_HANDLE_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_HANDLE            (picman_canvas_handle_get_type ())
#define PICMAN_CANVAS_HANDLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_HANDLE, PicmanCanvasHandle))
#define PICMAN_CANVAS_HANDLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_HANDLE, PicmanCanvasHandleClass))
#define PICMAN_IS_CANVAS_HANDLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_HANDLE))
#define PICMAN_IS_CANVAS_HANDLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_HANDLE))
#define PICMAN_CANVAS_HANDLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_HANDLE, PicmanCanvasHandleClass))


typedef struct _PicmanCanvasHandle      PicmanCanvasHandle;
typedef struct _PicmanCanvasHandleClass PicmanCanvasHandleClass;

struct _PicmanCanvasHandle
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasHandleClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_handle_get_type     (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_handle_new          (PicmanDisplayShell *shell,
                                                  PicmanHandleType    type,
                                                  PicmanHandleAnchor  anchor,
                                                  gdouble           x,
                                                  gdouble           y,
                                                  gint              width,
                                                  gint              height);

void             picman_canvas_handle_set_position (PicmanCanvasItem   *handle,
                                                  gdouble           x,
                                                  gdouble           y);
void             picman_canvas_handle_set_angles   (PicmanCanvasItem   *handle,
                                                  gdouble           start_handle,
                                                  gdouble           slice_handle);


#endif /* __PICMAN_CANVAS_HANDLE_H__ */
