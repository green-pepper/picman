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

#include "config/picmancoreconfig.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmandrawableundo.h"
#include "picmanimage.h"
#include "picmanimage-private.h"
#include "picmanimage-undo.h"
#include "picmanitem.h"
#include "picmanlist.h"
#include "picmanundostack.h"


/*  local function prototypes  */

static void          picman_image_undo_pop_stack       (PicmanImage     *image,
                                                      PicmanUndoStack *undo_stack,
                                                      PicmanUndoStack *redo_stack,
                                                      PicmanUndoMode   undo_mode);
static void          picman_image_undo_free_space      (PicmanImage     *image);
static void          picman_image_undo_free_redo       (PicmanImage     *image);

static PicmanDirtyMask picman_image_undo_dirty_from_type (PicmanUndoType   undo_type);


/*  public functions  */

gboolean
picman_image_undo_is_enabled (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return (PICMAN_IMAGE_GET_PRIVATE (image)->undo_freeze_count == 0);
}

gboolean
picman_image_undo_enable (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  /*  Free all undo steps as they are now invalidated  */
  picman_image_undo_free (image);

  return picman_image_undo_thaw (image);
}

gboolean
picman_image_undo_disable (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return picman_image_undo_freeze (image);
}

gboolean
picman_image_undo_freeze (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  private->undo_freeze_count++;

  if (private->undo_freeze_count == 1)
    picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_FREEZE, NULL);

  return TRUE;
}

gboolean
picman_image_undo_thaw (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_val_if_fail (private->undo_freeze_count > 0, FALSE);

  private->undo_freeze_count--;

  if (private->undo_freeze_count == 0)
    picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_THAW, NULL);

  return TRUE;
}

gboolean
picman_image_undo (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_val_if_fail (private->pushing_undo_group == PICMAN_UNDO_GROUP_NONE,
                        FALSE);

  picman_image_undo_pop_stack (image,
                             private->undo_stack,
                             private->redo_stack,
                             PICMAN_UNDO_MODE_UNDO);

  return TRUE;
}

gboolean
picman_image_redo (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_val_if_fail (private->pushing_undo_group == PICMAN_UNDO_GROUP_NONE,
                        FALSE);

  picman_image_undo_pop_stack (image,
                             private->redo_stack,
                             private->undo_stack,
                             PICMAN_UNDO_MODE_REDO);

  return TRUE;
}

/*
 * this function continues to undo as long as it only sees certain
 * undo types, in particular visibility changes.
 */
gboolean
picman_image_strong_undo (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanUndo         *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_val_if_fail (private->pushing_undo_group == PICMAN_UNDO_GROUP_NONE,
                        FALSE);

  undo = picman_undo_stack_peek (private->undo_stack);

  picman_image_undo (image);

  while (picman_undo_is_weak (undo))
    {
      undo = picman_undo_stack_peek (private->undo_stack);
      if (picman_undo_is_weak (undo))
        picman_image_undo (image);
    }

  return TRUE;
}

/*
 * this function continues to redo as long as it only sees certain
 * undo types, in particular visibility changes.  Note that the
 * order of events is set up to make it exactly reverse
 * picman_image_strong_undo().
 */
gboolean
picman_image_strong_redo (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanUndo         *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_val_if_fail (private->pushing_undo_group == PICMAN_UNDO_GROUP_NONE,
                        FALSE);

  undo = picman_undo_stack_peek (private->redo_stack);

  picman_image_redo (image);

  while (picman_undo_is_weak (undo))
    {
      undo = picman_undo_stack_peek (private->redo_stack);
      if (picman_undo_is_weak (undo))
        picman_image_redo (image);
    }

  return TRUE;
}

PicmanUndoStack *
picman_image_get_undo_stack (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->undo_stack;
}

PicmanUndoStack *
picman_image_get_redo_stack (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->redo_stack;
}

void
picman_image_undo_free (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  Emit the UNDO_FREE event before actually freeing everything
   *  so the views can properly detach from the undo items
   */
  picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_FREE, NULL);

  picman_undo_free (PICMAN_UNDO (private->undo_stack), PICMAN_UNDO_MODE_UNDO);
  picman_undo_free (PICMAN_UNDO (private->redo_stack), PICMAN_UNDO_MODE_REDO);

  /* If the image was dirty, but could become clean by redo-ing
   * some actions, then it should now become 'infinitely' dirty.
   * This is because we've just nuked the actions that would allow
   * the image to become clean again.
   */
  if (private->dirty < 0)
    private->dirty = 100000;

  /* The same applies to the case where the image would become clean
   * due to undo actions, but since user can't undo without an undo
   * stack, that's not so much a problem.
   */
}

gint
picman_image_get_undo_group_count (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->group_count;
}

gboolean
picman_image_undo_group_start (PicmanImage    *image,
                             PicmanUndoType  undo_type,
                             const gchar  *name)
{
  PicmanImagePrivate *private;
  PicmanUndoStack    *undo_group;
  PicmanDirtyMask     dirty_mask;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (undo_type >  PICMAN_UNDO_GROUP_FIRST &&
                        undo_type <= PICMAN_UNDO_GROUP_LAST, FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (! name)
    name = picman_undo_type_to_name (undo_type);

  dirty_mask = picman_image_undo_dirty_from_type (undo_type);

  /* Notify listeners that the image will be modified */
  if (private->group_count == 0 && dirty_mask != PICMAN_DIRTY_NONE)
    picman_image_dirty (image, dirty_mask);

  if (private->undo_freeze_count > 0)
    return FALSE;

  private->group_count++;

  /*  If we're already in a group...ignore  */
  if (private->group_count > 1)
    return TRUE;

  /*  nuke the redo stack  */
  picman_image_undo_free_redo (image);

  undo_group = picman_undo_stack_new (image);

  picman_object_set_name (PICMAN_OBJECT (undo_group), name);
  PICMAN_UNDO (undo_group)->undo_type  = undo_type;
  PICMAN_UNDO (undo_group)->dirty_mask = dirty_mask;

  picman_undo_stack_push_undo (private->undo_stack, PICMAN_UNDO (undo_group));

  private->pushing_undo_group = undo_type;

  return TRUE;
}

gboolean
picman_image_undo_group_end (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->undo_freeze_count > 0)
    return FALSE;

  g_return_val_if_fail (private->group_count > 0, FALSE);

  private->group_count--;

  if (private->group_count == 0)
    {
      private->pushing_undo_group = PICMAN_UNDO_GROUP_NONE;

      /* Do it here, since undo_push doesn't emit this event while in
       * the middle of a group
       */
      picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_PUSHED,
                             picman_undo_stack_peek (private->undo_stack));

      picman_image_undo_free_space (image);
    }

  return TRUE;
}

PicmanUndo *
picman_image_undo_push (PicmanImage     *image,
                      GType          object_type,
                      PicmanUndoType   undo_type,
                      const gchar   *name,
                      PicmanDirtyMask  dirty_mask,
                      ...)
{
  PicmanImagePrivate *private;
  GParameter       *params   = NULL;
  gint              n_params = 0;
  va_list           args;
  PicmanUndo         *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (g_type_is_a (object_type, PICMAN_TYPE_UNDO), NULL);
  g_return_val_if_fail (undo_type > PICMAN_UNDO_GROUP_LAST, NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /* Does this undo dirty the image?  If so, we always want to mark
   * image dirty, even if we can't actually push the undo.
   */
  if (dirty_mask != PICMAN_DIRTY_NONE)
    picman_image_dirty (image, dirty_mask);

  if (private->undo_freeze_count > 0)
    return NULL;

  if (! name)
    name = picman_undo_type_to_name (undo_type);

  params = picman_parameters_append (object_type, params, &n_params,
                                   "name",       name,
                                   "image",      image,
                                   "undo-type",  undo_type,
                                   "dirty-mask", dirty_mask,
                                   NULL);

  va_start (args, dirty_mask);
  params = picman_parameters_append_valist (object_type, params, &n_params, args);
  va_end (args);

  undo = g_object_newv (object_type, n_params, params);

  picman_parameters_free (params, n_params);

  /*  nuke the redo stack  */
  picman_image_undo_free_redo (image);

  if (private->pushing_undo_group == PICMAN_UNDO_GROUP_NONE)
    {
      picman_undo_stack_push_undo (private->undo_stack, undo);

      picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_PUSHED, undo);

      picman_image_undo_free_space (image);

      /*  freeing undo space may have freed the newly pushed undo  */
      if (picman_undo_stack_peek (private->undo_stack) == undo)
        return undo;
    }
  else
    {
      PicmanUndoStack *undo_group;

      undo_group = PICMAN_UNDO_STACK (picman_undo_stack_peek (private->undo_stack));

      picman_undo_stack_push_undo (undo_group, undo);

      return undo;
    }

  return NULL;
}

PicmanUndo *
picman_image_undo_can_compress (PicmanImage    *image,
                              GType         object_type,
                              PicmanUndoType  undo_type)
{
  PicmanImagePrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (picman_image_is_dirty (image) &&
      ! picman_undo_stack_peek (private->redo_stack))
    {
      PicmanUndo *undo = picman_undo_stack_peek (private->undo_stack);

      if (undo && undo->undo_type == undo_type &&
          g_type_is_a (G_TYPE_FROM_INSTANCE (undo), object_type))
        {
          return undo;
        }
    }

  return NULL;
}

PicmanUndo *
picman_image_undo_get_fadeable (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanUndo         *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  undo = picman_undo_stack_peek (private->undo_stack);

  if (PICMAN_IS_UNDO_STACK (undo) && undo->undo_type == PICMAN_UNDO_GROUP_PAINT)
    {
      PicmanUndoStack *stack = PICMAN_UNDO_STACK (undo);

      if (picman_undo_stack_get_depth (stack) == 2)
        {
          undo = picman_undo_stack_peek (stack);
        }
    }

  if (PICMAN_IS_DRAWABLE_UNDO (undo))
    return undo;

  return NULL;
}


/*  private functions  */

static void
picman_image_undo_pop_stack (PicmanImage     *image,
                           PicmanUndoStack *undo_stack,
                           PicmanUndoStack *redo_stack,
                           PicmanUndoMode   undo_mode)
{
  PicmanUndo            *undo;
  PicmanUndoAccumulator  accum = { 0, };

  g_object_freeze_notify (G_OBJECT (image));

  undo = picman_undo_stack_pop_undo (undo_stack, undo_mode, &accum);

  if (undo)
    {
      if (PICMAN_IS_UNDO_STACK (undo))
        picman_list_reverse (PICMAN_LIST (PICMAN_UNDO_STACK (undo)->undos));

      picman_undo_stack_push_undo (redo_stack, undo);

      if (accum.mode_changed)
        picman_image_mode_changed (image);

      if (accum.precision_changed)
        picman_image_precision_changed (image);

      if (accum.size_changed)
        picman_image_size_changed_detailed (image,
                                          accum.previous_origin_x,
                                          accum.previous_origin_y,
                                          accum.previous_width,
                                          accum.previous_height);

      if (accum.resolution_changed)
        picman_image_resolution_changed (image);

      if (accum.unit_changed)
        picman_image_unit_changed (image);

      /* let others know that we just popped an action */
      picman_image_undo_event (image,
                             (undo_mode == PICMAN_UNDO_MODE_UNDO) ?
                             PICMAN_UNDO_EVENT_UNDO : PICMAN_UNDO_EVENT_REDO,
                             undo);
    }

  g_object_thaw_notify (G_OBJECT (image));
}

static void
picman_image_undo_free_space (PicmanImage *image)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanContainer    *container;
  gint              min_undo_levels;
  gint              max_undo_levels;
  gint64            undo_size;

  container = private->undo_stack->undos;

  min_undo_levels = image->picman->config->levels_of_undo;
  max_undo_levels = 1024; /* FIXME */
  undo_size       = image->picman->config->undo_size;

#ifdef DEBUG_IMAGE_UNDO
  g_printerr ("undo_steps: %d    undo_bytes: %ld\n",
              picman_container_get_n_children (container),
              (glong) picman_object_get_memsize (PICMAN_OBJECT (container), NULL));
#endif

  /*  keep at least min_undo_levels undo steps  */
  if (picman_container_get_n_children (container) <= min_undo_levels)
    return;

  while ((picman_object_get_memsize (PICMAN_OBJECT (container), NULL) > undo_size) ||
         (picman_container_get_n_children (container) > max_undo_levels))
    {
      PicmanUndo *freed = picman_undo_stack_free_bottom (private->undo_stack,
                                                     PICMAN_UNDO_MODE_UNDO);

#ifdef DEBUG_IMAGE_UNDO
      g_printerr ("freed one step: undo_steps: %d    undo_bytes: %ld\n",
                  picman_container_get_n_children (container),
                  (glong) picman_object_get_memsize (PICMAN_OBJECT (container),
                                                   NULL));
#endif

      picman_image_undo_event (image, PICMAN_UNDO_EVENT_UNDO_EXPIRED, freed);

      g_object_unref (freed);

      if (picman_container_get_n_children (container) <= min_undo_levels)
        return;
    }
}

static void
picman_image_undo_free_redo (PicmanImage *image)
{
  PicmanImagePrivate *private   = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanContainer    *container = private->redo_stack->undos;

#ifdef DEBUG_IMAGE_UNDO
  g_printerr ("redo_steps: %d    redo_bytes: %ld\n",
              picman_container_get_n_children (container),
              (glong) picman_object_get_memsize (PICMAN_OBJECT (container), NULL));
#endif

  if (picman_container_is_empty (container))
    return;

  while (picman_container_get_n_children (container) > 0)
    {
      PicmanUndo *freed = picman_undo_stack_free_bottom (private->redo_stack,
                                                     PICMAN_UNDO_MODE_REDO);

#ifdef DEBUG_IMAGE_UNDO
      g_printerr ("freed one step: redo_steps: %d    redo_bytes: %ld\n",
                  picman_container_get_n_children (container),
                  (glong )picman_object_get_memsize (PICMAN_OBJECT (container),
                                                   NULL));
#endif

      picman_image_undo_event (image, PICMAN_UNDO_EVENT_REDO_EXPIRED, freed);

      g_object_unref (freed);
    }

  /* We need to use <= here because the undo counter has already been
   * incremented at this point.
   */
  if (private->dirty <= 0)
    {
      /* If the image was dirty, but could become clean by redo-ing
       * some actions, then it should now become 'infinitely' dirty.
       * This is because we've just nuked the actions that would allow
       * the image to become clean again.
       */
      private->dirty = 100000;
    }
}

static PicmanDirtyMask
picman_image_undo_dirty_from_type (PicmanUndoType undo_type)
{
  switch (undo_type)
    {
    case PICMAN_UNDO_GROUP_IMAGE_SCALE:
    case PICMAN_UNDO_GROUP_IMAGE_RESIZE:
    case PICMAN_UNDO_GROUP_IMAGE_FLIP:
    case PICMAN_UNDO_GROUP_IMAGE_ROTATE:
    case PICMAN_UNDO_GROUP_IMAGE_CROP:
      return PICMAN_DIRTY_IMAGE | PICMAN_DIRTY_IMAGE_SIZE;

    case PICMAN_UNDO_GROUP_IMAGE_CONVERT:
      return PICMAN_DIRTY_IMAGE | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE:
      return PICMAN_DIRTY_IMAGE_STRUCTURE | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE:
      return PICMAN_DIRTY_IMAGE_STRUCTURE | PICMAN_DIRTY_VECTORS;

    case PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK: /* FIXME */
      return PICMAN_DIRTY_IMAGE_STRUCTURE | PICMAN_DIRTY_SELECTION;

    case PICMAN_UNDO_GROUP_IMAGE_GRID:
    case PICMAN_UNDO_GROUP_GUIDE:
      return PICMAN_DIRTY_IMAGE_META;

    case PICMAN_UNDO_GROUP_DRAWABLE:
    case PICMAN_UNDO_GROUP_DRAWABLE_MOD:
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_MASK: /* FIXME */
      return PICMAN_DIRTY_SELECTION;

    case PICMAN_UNDO_GROUP_ITEM_VISIBILITY:
    case PICMAN_UNDO_GROUP_ITEM_LINKED:
    case PICMAN_UNDO_GROUP_ITEM_PROPERTIES:
      return PICMAN_DIRTY_ITEM_META;

    case PICMAN_UNDO_GROUP_ITEM_DISPLACE: /* FIXME */
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE | PICMAN_DIRTY_VECTORS;

    case PICMAN_UNDO_GROUP_ITEM_SCALE: /* FIXME */
    case PICMAN_UNDO_GROUP_ITEM_RESIZE: /* FIXME */
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE | PICMAN_DIRTY_VECTORS;

    case PICMAN_UNDO_GROUP_LAYER_ADD_MASK:
    case PICMAN_UNDO_GROUP_LAYER_APPLY_MASK:
      return PICMAN_DIRTY_IMAGE_STRUCTURE;

    case PICMAN_UNDO_GROUP_FS_TO_LAYER:
    case PICMAN_UNDO_GROUP_FS_FLOAT:
    case PICMAN_UNDO_GROUP_FS_ANCHOR:
      return PICMAN_DIRTY_IMAGE_STRUCTURE;

    case PICMAN_UNDO_GROUP_EDIT_PASTE:
      return PICMAN_DIRTY_IMAGE_STRUCTURE;

    case PICMAN_UNDO_GROUP_EDIT_CUT:
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_TEXT:
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_TRANSFORM: /* FIXME */
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE | PICMAN_DIRTY_VECTORS;

    case PICMAN_UNDO_GROUP_PAINT:
      return PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE;

    case PICMAN_UNDO_GROUP_PARASITE_ATTACH:
    case PICMAN_UNDO_GROUP_PARASITE_REMOVE:
      return PICMAN_DIRTY_IMAGE_META | PICMAN_DIRTY_ITEM_META;

    case PICMAN_UNDO_GROUP_VECTORS_IMPORT:
      return PICMAN_DIRTY_IMAGE_STRUCTURE | PICMAN_DIRTY_VECTORS;

    case PICMAN_UNDO_GROUP_MISC:
      return PICMAN_DIRTY_ALL;

    default:
      break;
    }

  return PICMAN_DIRTY_ALL;
}
