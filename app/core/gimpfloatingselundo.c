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

#include "picmanfloatingselundo.h"
#include "picmanimage.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"


static void   picman_floating_sel_undo_constructed (GObject             *object);

static void   picman_floating_sel_undo_pop         (PicmanUndo            *undo,
                                                  PicmanUndoMode         undo_mode,
                                                  PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanFloatingSelUndo, picman_floating_sel_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_floating_sel_undo_parent_class


static void
picman_floating_sel_undo_class_init (PicmanFloatingSelUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed = picman_floating_sel_undo_constructed;

  undo_class->pop           = picman_floating_sel_undo_pop;
}

static void
picman_floating_sel_undo_init (PicmanFloatingSelUndo *undo)
{
}

static void
picman_floating_sel_undo_constructed (GObject *object)
{
  PicmanFloatingSelUndo *floating_sel_undo = PICMAN_FLOATING_SEL_UNDO (object);
  PicmanLayer           *layer;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_LAYER (PICMAN_ITEM_UNDO (object)->item));

  layer = PICMAN_LAYER (PICMAN_ITEM_UNDO (object)->item);

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_FS_TO_LAYER:
      floating_sel_undo->drawable = picman_layer_get_floating_sel_drawable (layer);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_floating_sel_undo_pop (PicmanUndo            *undo,
                            PicmanUndoMode         undo_mode,
                            PicmanUndoAccumulator *accum)
{
  PicmanFloatingSelUndo *floating_sel_undo = PICMAN_FLOATING_SEL_UNDO (undo);
  PicmanLayer           *floating_layer    = PICMAN_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_FS_TO_LAYER:
      if (undo_mode == PICMAN_UNDO_MODE_UNDO)
        {
          /*  Update the preview for the floating sel  */
          picman_viewable_invalidate_preview (PICMAN_VIEWABLE (floating_layer));

          picman_layer_set_floating_sel_drawable (floating_layer,
                                                floating_sel_undo->drawable);
          picman_image_set_active_layer (undo->image, floating_layer);

          picman_drawable_attach_floating_sel (picman_layer_get_floating_sel_drawable (floating_layer),
                                             floating_layer);
        }
      else
        {
          picman_drawable_detach_floating_sel (picman_layer_get_floating_sel_drawable (floating_layer));
          picman_layer_set_floating_sel_drawable (floating_layer, NULL);
        }

      /* When the floating selection is converted to/from a normal
       * layer it does something resembling a name change, so emit the
       * "name-changed" signal
       */
      picman_object_name_changed (PICMAN_OBJECT (floating_layer));

      picman_drawable_update (PICMAN_DRAWABLE (floating_layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (floating_layer)),
                            picman_item_get_height (PICMAN_ITEM (floating_layer)));
      break;

    default:
      g_assert_not_reached ();
    }
}
