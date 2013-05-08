/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancanvascursor.h
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

#ifndef __PICMAN_CANVAS_CURSOR_H__
#define __PICMAN_CANVAS_CURSOR_H__


#include "picmancanvasitem.h"


#define PICMAN_TYPE_CANVAS_CURSOR            (picman_canvas_cursor_get_type ())
#define PICMAN_CANVAS_CURSOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CANVAS_CURSOR, PicmanCanvasCursor))
#define PICMAN_CANVAS_CURSOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CANVAS_CURSOR, PicmanCanvasCursorClass))
#define PICMAN_IS_CANVAS_CURSOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CANVAS_CURSOR))
#define PICMAN_IS_CANVAS_CURSOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CANVAS_CURSOR))
#define PICMAN_CANVAS_CURSOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CANVAS_CURSOR, PicmanCanvasCursorClass))


typedef struct _PicmanCanvasCursor      PicmanCanvasCursor;
typedef struct _PicmanCanvasCursorClass PicmanCanvasCursorClass;

struct _PicmanCanvasCursor
{
  PicmanCanvasItem  parent_instance;
};

struct _PicmanCanvasCursorClass
{
  PicmanCanvasItemClass  parent_class;
};


GType            picman_canvas_cursor_get_type (void) G_GNUC_CONST;

PicmanCanvasItem * picman_canvas_cursor_new      (PicmanDisplayShell *shell);

void             picman_canvas_cursor_set      (PicmanCanvasItem   *cursor,
                                              gdouble           x,
                                              gdouble           y);


#endif /* __PICMAN_CANVAS_CURSOR_H__ */
