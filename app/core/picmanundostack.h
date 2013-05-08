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

#ifndef __PICMAN_UNDO_STACK_H__
#define __PICMAN_UNDO_STACK_H__


#include "picmanundo.h"


#define PICMAN_TYPE_UNDO_STACK            (picman_undo_stack_get_type ())
#define PICMAN_UNDO_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNDO_STACK, PicmanUndoStack))
#define PICMAN_UNDO_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNDO_STACK, PicmanUndoStackClass))
#define PICMAN_IS_UNDO_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UNDO_STACK))
#define PICMAN_IS_UNDO_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNDO_STACK))
#define PICMAN_UNDO_STACK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNDO_STACK, PicmanUndoStackClass))


typedef struct _PicmanUndoStackClass PicmanUndoStackClass;

struct _PicmanUndoStack
{
  PicmanUndo       parent_instance;

  PicmanContainer *undos;
};

struct _PicmanUndoStackClass
{
  PicmanUndoClass  parent_class;
};


GType           picman_undo_stack_get_type    (void) G_GNUC_CONST;

PicmanUndoStack * picman_undo_stack_new         (PicmanImage           *image);

void            picman_undo_stack_push_undo   (PicmanUndoStack       *stack,
                                             PicmanUndo            *undo);
PicmanUndo      * picman_undo_stack_pop_undo    (PicmanUndoStack       *stack,
                                             PicmanUndoMode         undo_mode,
                                             PicmanUndoAccumulator *accum);

PicmanUndo      * picman_undo_stack_free_bottom (PicmanUndoStack       *stack,
                                             PicmanUndoMode         undo_mode);
PicmanUndo      * picman_undo_stack_peek        (PicmanUndoStack       *stack);
gint            picman_undo_stack_get_depth   (PicmanUndoStack       *stack);


#endif /* __PICMAN_UNDO_STACK_H__ */
