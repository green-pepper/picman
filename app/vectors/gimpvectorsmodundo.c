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

#include "vectors-types.h"

#include "picmanvectors.h"
#include "picmanvectorsmodundo.h"


static void     picman_vectors_mod_undo_constructed (GObject             *object);

static gint64   picman_vectors_mod_undo_get_memsize (PicmanObject          *object,
                                                   gint64              *gui_size);

static void     picman_vectors_mod_undo_pop         (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode,
                                                   PicmanUndoAccumulator *accum);
static void     picman_vectors_mod_undo_free        (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanVectorsModUndo, picman_vectors_mod_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_vectors_mod_undo_parent_class


static void
picman_vectors_mod_undo_class_init (PicmanVectorsModUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_vectors_mod_undo_constructed;

  picman_object_class->get_memsize = picman_vectors_mod_undo_get_memsize;

  undo_class->pop                = picman_vectors_mod_undo_pop;
  undo_class->free               = picman_vectors_mod_undo_free;
}

static void
picman_vectors_mod_undo_init (PicmanVectorsModUndo *undo)
{
}

static void
picman_vectors_mod_undo_constructed (GObject *object)
{
  PicmanVectorsModUndo *vectors_mod_undo = PICMAN_VECTORS_MOD_UNDO (object);
  PicmanVectors        *vectors;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_VECTORS (PICMAN_ITEM_UNDO (object)->item));

  vectors = PICMAN_VECTORS (PICMAN_ITEM_UNDO (object)->item);

  vectors_mod_undo->vectors =
    PICMAN_VECTORS (picman_item_duplicate (PICMAN_ITEM (vectors),
                                       G_TYPE_FROM_INSTANCE (vectors)));
}

static gint64
picman_vectors_mod_undo_get_memsize (PicmanObject *object,
                                   gint64     *gui_size)
{
  PicmanVectorsModUndo *vectors_mod_undo = PICMAN_VECTORS_MOD_UNDO (object);
  gint64              memsize          = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (vectors_mod_undo->vectors),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_vectors_mod_undo_pop (PicmanUndo            *undo,
                           PicmanUndoMode         undo_mode,
                           PicmanUndoAccumulator *accum)
{
  PicmanVectorsModUndo *vectors_mod_undo = PICMAN_VECTORS_MOD_UNDO (undo);
  PicmanVectors        *vectors          = PICMAN_VECTORS (PICMAN_ITEM_UNDO (undo)->item);
  PicmanVectors        *temp;
  gint                offset_x;
  gint                offset_y;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  temp = vectors_mod_undo->vectors;

  vectors_mod_undo->vectors =
    PICMAN_VECTORS (picman_item_duplicate (PICMAN_ITEM (vectors),
                                       G_TYPE_FROM_INSTANCE (vectors)));

  picman_vectors_freeze (vectors);

  picman_vectors_copy_strokes (temp, vectors);

  picman_item_get_offset (PICMAN_ITEM (temp), &offset_x, &offset_y);
  picman_item_set_offset (PICMAN_ITEM (vectors), offset_x, offset_y);

  picman_item_set_size (PICMAN_ITEM (vectors),
                      picman_item_get_width  (PICMAN_ITEM (temp)),
                      picman_item_get_height (PICMAN_ITEM (temp)));

  g_object_unref (temp);

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_mod_undo_free (PicmanUndo     *undo,
                            PicmanUndoMode  undo_mode)
{
  PicmanVectorsModUndo *vectors_mod_undo = PICMAN_VECTORS_MOD_UNDO (undo);

  if (vectors_mod_undo->vectors)
    {
      g_object_unref (vectors_mod_undo->vectors);
      vectors_mod_undo->vectors = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
