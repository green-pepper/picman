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

#include "core-types.h"

#include "picmanlayer.h"
#include "picmanlayermaskpropundo.h"


static void   picman_layer_mask_prop_undo_constructed (GObject             *object);

static void   picman_layer_mask_prop_undo_pop         (PicmanUndo            *undo,
                                                     PicmanUndoMode         undo_mode,
                                                     PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanLayerMaskPropUndo, picman_layer_mask_prop_undo,
               PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_layer_mask_prop_undo_parent_class


static void
picman_layer_mask_prop_undo_class_init (PicmanLayerMaskPropUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed = picman_layer_mask_prop_undo_constructed;

  undo_class->pop           = picman_layer_mask_prop_undo_pop;
}

static void
picman_layer_mask_prop_undo_init (PicmanLayerMaskPropUndo *undo)
{
}

static void
picman_layer_mask_prop_undo_constructed (GObject *object)
{
  PicmanLayerMaskPropUndo *layer_mask_prop_undo;
  PicmanLayer             *layer;

  layer_mask_prop_undo = PICMAN_LAYER_MASK_PROP_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_LAYER (PICMAN_ITEM_UNDO (object)->item));

  layer = PICMAN_LAYER (PICMAN_ITEM_UNDO (object)->item);

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_LAYER_MASK_APPLY:
      layer_mask_prop_undo->apply = picman_layer_get_apply_mask (layer);
      break;

    case PICMAN_UNDO_LAYER_MASK_SHOW:
      layer_mask_prop_undo->show = picman_layer_get_show_mask (layer);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_layer_mask_prop_undo_pop (PicmanUndo            *undo,
                               PicmanUndoMode         undo_mode,
                               PicmanUndoAccumulator *accum)
{
  PicmanLayerMaskPropUndo *layer_mask_prop_undo = PICMAN_LAYER_MASK_PROP_UNDO (undo);
  PicmanLayer             *layer                = PICMAN_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_LAYER_MASK_APPLY:
      {
        gboolean apply;

        apply = picman_layer_get_apply_mask (layer);
        picman_layer_set_apply_mask (layer, layer_mask_prop_undo->apply, FALSE);
        layer_mask_prop_undo->apply = apply;
      }
      break;

    case PICMAN_UNDO_LAYER_MASK_SHOW:
      {
        gboolean show;

        show = picman_layer_get_show_mask (layer);
        picman_layer_set_show_mask (layer, layer_mask_prop_undo->show, FALSE);
        layer_mask_prop_undo->show = show;
      }
      break;

    default:
      g_assert_not_reached ();
    }
}
