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

#include <string.h>

#include <gegl.h>

#include "paint-types.h"

#include "picmanink.h"
#include "picmanink-blob.h"
#include "picmaninkundo.h"


static void   picman_ink_undo_constructed (GObject             *object);

static void   picman_ink_undo_pop         (PicmanUndo            *undo,
                                         PicmanUndoMode         undo_mode,
                                         PicmanUndoAccumulator *accum);
static void   picman_ink_undo_free        (PicmanUndo            *undo,
                                         PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanInkUndo, picman_ink_undo, PICMAN_TYPE_PAINT_CORE_UNDO)

#define parent_class picman_ink_undo_parent_class


static void
picman_ink_undo_class_init (PicmanInkUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed = picman_ink_undo_constructed;

  undo_class->pop           = picman_ink_undo_pop;
  undo_class->free          = picman_ink_undo_free;
}

static void
picman_ink_undo_init (PicmanInkUndo *undo)
{
}

static void
picman_ink_undo_constructed (GObject *object)
{
  PicmanInkUndo *ink_undo = PICMAN_INK_UNDO (object);
  PicmanInk     *ink;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_INK (PICMAN_PAINT_CORE_UNDO (ink_undo)->paint_core));

  ink = PICMAN_INK (PICMAN_PAINT_CORE_UNDO (ink_undo)->paint_core);

  if (ink->start_blob)
    ink_undo->last_blob = picman_blob_duplicate (ink->start_blob);
}

static void
picman_ink_undo_pop (PicmanUndo              *undo,
                   PicmanUndoMode           undo_mode,
                   PicmanUndoAccumulator   *accum)
{
  PicmanInkUndo *ink_undo = PICMAN_INK_UNDO (undo);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if (PICMAN_PAINT_CORE_UNDO (ink_undo)->paint_core)
    {
      PicmanInk  *ink = PICMAN_INK (PICMAN_PAINT_CORE_UNDO (ink_undo)->paint_core);
      PicmanBlob *tmp_blob;

      tmp_blob = ink->last_blob;
      ink->last_blob = ink_undo->last_blob;
      ink_undo->last_blob = tmp_blob;

    }
}

static void
picman_ink_undo_free (PicmanUndo     *undo,
                    PicmanUndoMode  undo_mode)
{
  PicmanInkUndo *ink_undo = PICMAN_INK_UNDO (undo);

  if (ink_undo->last_blob)
    {
      g_free (ink_undo->last_blob);
      ink_undo->last_blob = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
