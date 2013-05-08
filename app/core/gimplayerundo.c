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
#include "picmanlayer.h"
#include "picmanlayerundo.h"


enum
{
  PROP_0,
  PROP_PREV_PARENT,
  PROP_PREV_POSITION,
  PROP_PREV_LAYER
};


static void     picman_layer_undo_constructed  (GObject             *object);
static void     picman_layer_undo_set_property (GObject             *object,
                                              guint                property_id,
                                              const GValue        *value,
                                              GParamSpec          *pspec);
static void     picman_layer_undo_get_property (GObject             *object,
                                              guint                property_id,
                                              GValue              *value,
                                              GParamSpec          *pspec);

static gint64   picman_layer_undo_get_memsize  (PicmanObject          *object,
                                              gint64              *gui_size);

static void     picman_layer_undo_pop          (PicmanUndo            *undo,
                                              PicmanUndoMode         undo_mode,
                                              PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanLayerUndo, picman_layer_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_layer_undo_parent_class


static void
picman_layer_undo_class_init (PicmanLayerUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_layer_undo_constructed;
  object_class->set_property     = picman_layer_undo_set_property;
  object_class->get_property     = picman_layer_undo_get_property;

  picman_object_class->get_memsize = picman_layer_undo_get_memsize;

  undo_class->pop                = picman_layer_undo_pop;

  g_object_class_install_property (object_class, PROP_PREV_PARENT,
                                   g_param_spec_object ("prev-parent",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_LAYER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_POSITION,
                                   g_param_spec_int ("prev-position", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_LAYER,
                                   g_param_spec_object ("prev-layer", NULL, NULL,
                                                        PICMAN_TYPE_LAYER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_layer_undo_init (PicmanLayerUndo *undo)
{
}

static void
picman_layer_undo_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_LAYER (PICMAN_ITEM_UNDO (object)->item));
}

static void
picman_layer_undo_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanLayerUndo *layer_undo = PICMAN_LAYER_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      layer_undo->prev_parent = g_value_get_object (value);
      break;
    case PROP_PREV_POSITION:
      layer_undo->prev_position = g_value_get_int (value);
      break;
    case PROP_PREV_LAYER:
      layer_undo->prev_layer = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_layer_undo_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanLayerUndo *layer_undo = PICMAN_LAYER_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      g_value_set_object (value, layer_undo->prev_parent);
      break;
    case PROP_PREV_POSITION:
      g_value_set_int (value, layer_undo->prev_position);
      break;
    case PROP_PREV_LAYER:
      g_value_set_object (value, layer_undo->prev_layer);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_layer_undo_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (object);
  gint64        memsize   = 0;

  if (! picman_item_is_attached (item_undo->item))
    memsize += picman_object_get_memsize (PICMAN_OBJECT (item_undo->item),
                                        gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_layer_undo_pop (PicmanUndo            *undo,
                     PicmanUndoMode         undo_mode,
                     PicmanUndoAccumulator *accum)
{
  PicmanLayerUndo *layer_undo = PICMAN_LAYER_UNDO (undo);
  PicmanLayer     *layer      = PICMAN_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if ((undo_mode       == PICMAN_UNDO_MODE_UNDO &&
       undo->undo_type == PICMAN_UNDO_LAYER_ADD) ||
      (undo_mode       == PICMAN_UNDO_MODE_REDO &&
       undo->undo_type == PICMAN_UNDO_LAYER_REMOVE))
    {
      /*  remove layer  */

      /*  record the current parent and position  */
      layer_undo->prev_parent   = picman_layer_get_parent (layer);
      layer_undo->prev_position = picman_item_get_index (PICMAN_ITEM (layer));

      picman_image_remove_layer (undo->image, layer, FALSE,
                               layer_undo->prev_layer);
    }
  else
    {
      /*  restore layer  */

      /*  record the active layer  */
      layer_undo->prev_layer = picman_image_get_active_layer (undo->image);

      picman_image_add_layer (undo->image, layer,
                            layer_undo->prev_parent,
                            layer_undo->prev_position, FALSE);
    }
}
