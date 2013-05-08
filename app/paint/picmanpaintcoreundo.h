/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_PAINT_CORE_UNDO_H__
#define __PICMAN_PAINT_CORE_UNDO_H__


#include "core/picmanundo.h"


#define PICMAN_TYPE_PAINT_CORE_UNDO            (picman_paint_core_undo_get_type ())
#define PICMAN_PAINT_CORE_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINT_CORE_UNDO, PicmanPaintCoreUndo))
#define PICMAN_PAINT_CORE_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINT_CORE_UNDO, PicmanPaintCoreUndoClass))
#define PICMAN_IS_PAINT_CORE_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINT_CORE_UNDO))
#define PICMAN_IS_PAINT_CORE_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINT_CORE_UNDO))
#define PICMAN_PAINT_CORE_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINT_CORE_UNDO, PicmanPaintCoreUndoClass))


typedef struct _PicmanPaintCoreUndoClass PicmanPaintCoreUndoClass;

struct _PicmanPaintCoreUndo
{
  PicmanUndo       parent_instance;

  PicmanPaintCore *paint_core;
  PicmanCoords     last_coords;
};

struct _PicmanPaintCoreUndoClass
{
  PicmanUndoClass  parent_class;
};


GType   picman_paint_core_undo_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_PAINT_CORE_UNDO_H__ */
