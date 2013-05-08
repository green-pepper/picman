/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushclipboard.h
 * Copyright (C) 2006 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_BRUSH_CLIPBOARD_H__
#define __PICMAN_BRUSH_CLIPBOARD_H__


#include "picmanbrush.h"


#define PICMAN_TYPE_BRUSH_CLIPBOARD            (picman_brush_clipboard_get_type ())
#define PICMAN_BRUSH_CLIPBOARD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_CLIPBOARD, PicmanBrushClipboard))
#define PICMAN_BRUSH_CLIPBOARD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_CLIPBOARD, PicmanBrushClipboardClass))
#define PICMAN_IS_BRUSH_CLIPBOARD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_CLIPBOARD))
#define PICMAN_IS_BRUSH_CLIPBOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_CLIPBOARD))
#define PICMAN_BRUSH_CLIPBOARD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_CLIPBOARD, PicmanBrushClipboardClass))


typedef struct _PicmanBrushClipboardClass PicmanBrushClipboardClass;

struct _PicmanBrushClipboard
{
  PicmanBrush  parent_instance;

  Picman      *picman;
};

struct _PicmanBrushClipboardClass
{
  PicmanBrushClass  parent_class;
};


GType      picman_brush_clipboard_get_type (void) G_GNUC_CONST;

PicmanData * picman_brush_clipboard_new      (Picman *picman);


#endif  /*  __PICMAN_BRUSH_CLIPBOARD_H__  */
