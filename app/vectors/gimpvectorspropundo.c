/* Picman - The GNU Image Manipulation Program
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

#include "core/picmanimage.h"

#include "picmanvectors.h"
#include "picmanvectorspropundo.h"


static void   picman_vectors_prop_undo_constructed (GObject             *object);

static void   picman_vectors_prop_undo_pop         (PicmanUndo            *undo,
                                                  PicmanUndoMode         undo_mode,
                                                  PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanVectorsPropUndo, picman_vectors_prop_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_vectors_prop_undo_parent_class


static void
picman_vectors_prop_undo_class_init (PicmanVectorsPropUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed = picman_vectors_prop_undo_constructed;

  undo_class->pop           = picman_vectors_prop_undo_pop;
}

static void
picman_vectors_prop_undo_init (PicmanVectorsPropUndo *undo)
{
}

static void
picman_vectors_prop_undo_constructed (GObject *object)
{
  /* PicmanVectors *vectors; */

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_VECTORS (PICMAN_ITEM_UNDO (object)->item));

  /* vectors = PICMAN_VECTORS (PICMAN_ITEM_UNDO (object)->item); */

  switch (PICMAN_UNDO (object)->undo_type)
    {
    default:
      g_assert_not_reached ();
    }
}

static void
picman_vectors_prop_undo_pop (PicmanUndo            *undo,
                            PicmanUndoMode         undo_mode,
                            PicmanUndoAccumulator *accum)
{
#if 0
  PicmanVectorsPropUndo *vectors_prop_undo = PICMAN_VECTORS_PROP_UNDO (undo);
  PicmanVectors         *vectors           = PICMAN_VECTORS (PICMAN_ITEM_UNDO (undo)->item);
#endif

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    default:
      g_assert_not_reached ();
    }
}
