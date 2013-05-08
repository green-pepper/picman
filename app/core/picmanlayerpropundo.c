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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picmanimage.h"
#include "picmanlayer.h"
#include "picmanlayerpropundo.h"


static void   picman_layer_prop_undo_constructed (GObject             *object);

static void   picman_layer_prop_undo_pop         (PicmanUndo            *undo,
                                                PicmanUndoMode         undo_mode,
                                                PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanLayerPropUndo, picman_layer_prop_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_layer_prop_undo_parent_class


static void
picman_layer_prop_undo_class_init (PicmanLayerPropUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed = picman_layer_prop_undo_constructed;

  undo_class->pop           = picman_layer_prop_undo_pop;
}

static void
picman_layer_prop_undo_init (PicmanLayerPropUndo *undo)
{
}

static void
picman_layer_prop_undo_constructed (GObject *object)
{
  PicmanLayerPropUndo *layer_prop_undo = PICMAN_LAYER_PROP_UNDO (object);
  PicmanLayer         *layer;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_LAYER (PICMAN_ITEM_UNDO (object)->item));

  layer = PICMAN_LAYER (PICMAN_ITEM_UNDO (object)->item);

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_LAYER_MODE:
      layer_prop_undo->mode = picman_layer_get_mode (layer);
      break;

    case PICMAN_UNDO_LAYER_OPACITY:
      layer_prop_undo->opacity = picman_layer_get_opacity (layer);
      break;

    case PICMAN_UNDO_LAYER_LOCK_ALPHA:
      layer_prop_undo->lock_alpha = picman_layer_get_lock_alpha (layer);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_layer_prop_undo_pop (PicmanUndo            *undo,
                          PicmanUndoMode         undo_mode,
                          PicmanUndoAccumulator *accum)
{
  PicmanLayerPropUndo *layer_prop_undo = PICMAN_LAYER_PROP_UNDO (undo);
  PicmanLayer         *layer           = PICMAN_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_LAYER_MODE:
      {
        PicmanLayerModeEffects mode;

        mode = picman_layer_get_mode (layer);
        picman_layer_set_mode (layer, layer_prop_undo->mode, FALSE);
        layer_prop_undo->mode = mode;
      }
      break;

    case PICMAN_UNDO_LAYER_OPACITY:
      {
        gdouble opacity;

        opacity = picman_layer_get_opacity (layer);
        picman_layer_set_opacity (layer, layer_prop_undo->opacity, FALSE);
        layer_prop_undo->opacity = opacity;
      }
      break;

    case PICMAN_UNDO_LAYER_LOCK_ALPHA:
      {
        gboolean lock_alpha;

        lock_alpha = picman_layer_get_lock_alpha (layer);
        picman_layer_set_lock_alpha (layer, layer_prop_undo->lock_alpha, FALSE);
        layer_prop_undo->lock_alpha = lock_alpha;
      }
      break;

    default:
      g_assert_not_reached ();
    }
}
