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

#include "config.h"

#include <gegl.h>

#include "core-types.h"

#include "picmanimage.h"
#include "picmanlist.h"
#include "picmanundo.h"
#include "picmanundostack.h"


static void    picman_undo_stack_finalize    (GObject             *object);

static gint64  picman_undo_stack_get_memsize (PicmanObject          *object,
                                            gint64              *gui_size);

static void    picman_undo_stack_pop         (PicmanUndo            *undo,
                                            PicmanUndoMode         undo_mode,
                                            PicmanUndoAccumulator *accum);
static void    picman_undo_stack_free        (PicmanUndo            *undo,
                                            PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanUndoStack, picman_undo_stack, PICMAN_TYPE_UNDO)

#define parent_class picman_undo_stack_parent_class


static void
picman_undo_stack_class_init (PicmanUndoStackClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->finalize         = picman_undo_stack_finalize;

  picman_object_class->get_memsize = picman_undo_stack_get_memsize;

  undo_class->pop                = picman_undo_stack_pop;
  undo_class->free               = picman_undo_stack_free;
}

static void
picman_undo_stack_init (PicmanUndoStack *stack)
{
  stack->undos = picman_list_new (PICMAN_TYPE_UNDO, FALSE);
}

static void
picman_undo_stack_finalize (GObject *object)
{
  PicmanUndoStack *stack = PICMAN_UNDO_STACK (object);

  if (stack->undos)
    {
      g_object_unref (stack->undos);
      stack->undos = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_undo_stack_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanUndoStack *stack   = PICMAN_UNDO_STACK (object);
  gint64         memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (stack->undos), gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_undo_stack_pop (PicmanUndo            *undo,
                     PicmanUndoMode         undo_mode,
                     PicmanUndoAccumulator *accum)
{
  PicmanUndoStack *stack = PICMAN_UNDO_STACK (undo);
  GList         *list;

  for (list = PICMAN_LIST (stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      PicmanUndo *child = list->data;

      picman_undo_pop (child, undo_mode, accum);
    }
}

static void
picman_undo_stack_free (PicmanUndo     *undo,
                      PicmanUndoMode  undo_mode)
{
  PicmanUndoStack *stack = PICMAN_UNDO_STACK (undo);
  GList         *list;

  for (list = PICMAN_LIST (stack->undos)->list;
       list;
       list = g_list_next (list))
    {
      PicmanUndo *child = list->data;

      picman_undo_free (child, undo_mode);
      g_object_unref (child);
    }

  picman_container_clear (stack->undos);
}

PicmanUndoStack *
picman_undo_stack_new (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return g_object_new (PICMAN_TYPE_UNDO_STACK,
                       "image", image,
                       NULL);
}

void
picman_undo_stack_push_undo (PicmanUndoStack *stack,
                           PicmanUndo      *undo)
{
  g_return_if_fail (PICMAN_IS_UNDO_STACK (stack));
  g_return_if_fail (PICMAN_IS_UNDO (undo));

  picman_container_add (stack->undos, PICMAN_OBJECT (undo));
}

PicmanUndo *
picman_undo_stack_pop_undo (PicmanUndoStack       *stack,
                          PicmanUndoMode         undo_mode,
                          PicmanUndoAccumulator *accum)
{
  PicmanUndo *undo;

  g_return_val_if_fail (PICMAN_IS_UNDO_STACK (stack), NULL);
  g_return_val_if_fail (accum != NULL, NULL);

  undo = PICMAN_UNDO (picman_container_get_first_child (stack->undos));

  if (undo)
    {
      picman_container_remove (stack->undos, PICMAN_OBJECT (undo));
      picman_undo_pop (undo, undo_mode, accum);

      return undo;
    }

  return NULL;
}

PicmanUndo *
picman_undo_stack_free_bottom (PicmanUndoStack *stack,
                             PicmanUndoMode   undo_mode)
{
  PicmanUndo *undo;

  g_return_val_if_fail (PICMAN_IS_UNDO_STACK (stack), NULL);

  undo = PICMAN_UNDO (picman_container_get_last_child (stack->undos));

  if (undo)
    {
      picman_container_remove (stack->undos, PICMAN_OBJECT (undo));
      picman_undo_free (undo, undo_mode);

      return undo;
    }

  return NULL;
}

PicmanUndo *
picman_undo_stack_peek (PicmanUndoStack *stack)
{
  g_return_val_if_fail (PICMAN_IS_UNDO_STACK (stack), NULL);

  return PICMAN_UNDO (picman_container_get_first_child (stack->undos));
}

gint
picman_undo_stack_get_depth (PicmanUndoStack *stack)
{
  g_return_val_if_fail (PICMAN_IS_UNDO_STACK (stack), 0);

  return picman_container_get_n_children (stack->undos);
}
