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
#include "picmangrouplayer.h"
#include "picmangrouplayerundo.h"


static void   picman_group_layer_undo_constructed (GObject             *object);

static void   picman_group_layer_undo_pop         (PicmanUndo            *undo,
                                                 PicmanUndoMode         undo_mode,
                                                 PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanGroupLayerUndo, picman_group_layer_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_group_layer_undo_parent_class


static void
picman_group_layer_undo_class_init (PicmanGroupLayerUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed   = picman_group_layer_undo_constructed;

  undo_class->pop             = picman_group_layer_undo_pop;
}

static void
picman_group_layer_undo_init (PicmanGroupLayerUndo *undo)
{
}

static void
picman_group_layer_undo_constructed (GObject *object)
{
  PicmanGroupLayerUndo *group_layer_undo = PICMAN_GROUP_LAYER_UNDO (object);
  PicmanGroupLayer     *group;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_GROUP_LAYER (PICMAN_ITEM_UNDO (object)->item));

  group = PICMAN_GROUP_LAYER (PICMAN_ITEM_UNDO (object)->item);

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_GROUP_LAYER_SUSPEND:
    case PICMAN_UNDO_GROUP_LAYER_RESUME:
      break;

    case PICMAN_UNDO_GROUP_LAYER_CONVERT:
      group_layer_undo->prev_type = picman_drawable_get_base_type (PICMAN_DRAWABLE (group));
      group_layer_undo->prev_precision = picman_drawable_get_precision (PICMAN_DRAWABLE (group));
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_group_layer_undo_pop (PicmanUndo            *undo,
                           PicmanUndoMode         undo_mode,
                           PicmanUndoAccumulator *accum)
{
  PicmanGroupLayerUndo *group_layer_undo = PICMAN_GROUP_LAYER_UNDO (undo);
  PicmanGroupLayer     *group;

  group = PICMAN_GROUP_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_GROUP_LAYER_SUSPEND:
    case PICMAN_UNDO_GROUP_LAYER_RESUME:
      if ((undo_mode       == PICMAN_UNDO_MODE_UNDO &&
           undo->undo_type == PICMAN_UNDO_GROUP_LAYER_SUSPEND) ||
          (undo_mode       == PICMAN_UNDO_MODE_REDO &&
           undo->undo_type == PICMAN_UNDO_GROUP_LAYER_RESUME))
        {
          /*  resume group layer auto-resizing  */

          picman_group_layer_resume_resize (group, FALSE);
        }
      else
        {
          /*  suspend group layer auto-resizing  */

          picman_group_layer_suspend_resize (group, FALSE);
        }
      break;

    case PICMAN_UNDO_GROUP_LAYER_CONVERT:
      {
        PicmanImageBaseType type;
        PicmanPrecision     precision;

        type      = picman_drawable_get_base_type (PICMAN_DRAWABLE (group));
        precision = picman_drawable_get_precision (PICMAN_DRAWABLE (group));

        picman_drawable_convert_type (PICMAN_DRAWABLE (group),
                                    picman_item_get_image (PICMAN_ITEM (group)),
                                    group_layer_undo->prev_type,
                                    group_layer_undo->prev_precision,
                                    0, 0,
                                    FALSE);

        group_layer_undo->prev_type      = type;
        group_layer_undo->prev_precision = precision;
      }
      break;

    default:
      g_assert_not_reached ();
    }
}
