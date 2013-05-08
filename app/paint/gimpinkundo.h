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

#ifndef __PICMAN_INK_UNDO_H__
#define __PICMAN_INK_UNDO_H__


#include "picmanpaintcoreundo.h"


#define PICMAN_TYPE_INK_UNDO            (picman_ink_undo_get_type ())
#define PICMAN_INK_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INK_UNDO, PicmanInkUndo))
#define PICMAN_INK_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INK_UNDO, PicmanInkUndoClass))
#define PICMAN_IS_INK_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INK_UNDO))
#define PICMAN_IS_INK_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INK_UNDO))
#define PICMAN_INK_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INK_UNDO, PicmanInkUndoClass))


typedef struct _PicmanInkUndoClass PicmanInkUndoClass;

struct _PicmanInkUndo
{
  PicmanPaintCoreUndo  parent_instance;

  PicmanBlob         *last_blob;
};

struct _PicmanInkUndoClass
{
  PicmanPaintCoreUndoClass  parent_class;
};


GType   picman_ink_undo_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_INK_UNDO_H__ */
