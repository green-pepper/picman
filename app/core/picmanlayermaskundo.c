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
#include "picmanlayermask.h"
#include "picmanlayermaskundo.h"


enum
{
  PROP_0,
  PROP_LAYER_MASK
};


static void     picman_layer_mask_undo_constructed  (GObject             *object);
static void     picman_layer_mask_undo_set_property (GObject             *object,
                                                   guint                property_id,
                                                   const GValue        *value,
                                                   GParamSpec          *pspec);
static void     picman_layer_mask_undo_get_property (GObject             *object,
                                                   guint                property_id,
                                                   GValue              *value,
                                                   GParamSpec          *pspec);

static gint64   picman_layer_mask_undo_get_memsize  (PicmanObject          *object,
                                                   gint64              *gui_size);

static void     picman_layer_mask_undo_pop          (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode,
                                                   PicmanUndoAccumulator *accum);
static void     picman_layer_mask_undo_free         (PicmanUndo            *undo,
                                                   PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanLayerMaskUndo, picman_layer_mask_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_layer_mask_undo_parent_class


static void
picman_layer_mask_undo_class_init (PicmanLayerMaskUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_layer_mask_undo_constructed;
  object_class->set_property     = picman_layer_mask_undo_set_property;
  object_class->get_property     = picman_layer_mask_undo_get_property;

  picman_object_class->get_memsize = picman_layer_mask_undo_get_memsize;

  undo_class->pop                = picman_layer_mask_undo_pop;
  undo_class->free               = picman_layer_mask_undo_free;

  g_object_class_install_property (object_class, PROP_LAYER_MASK,
                                   g_param_spec_object ("layer-mask", NULL, NULL,
                                                        PICMAN_TYPE_LAYER_MASK,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_layer_mask_undo_init (PicmanLayerMaskUndo *undo)
{
}

static void
picman_layer_mask_undo_constructed (GObject *object)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_LAYER (PICMAN_ITEM_UNDO (object)->item));
  g_assert (PICMAN_IS_LAYER_MASK (layer_mask_undo->layer_mask));
}

static void
picman_layer_mask_undo_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (object);

  switch (property_id)
    {
    case PROP_LAYER_MASK:
      layer_mask_undo->layer_mask = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_layer_mask_undo_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (object);

  switch (property_id)
    {
    case PROP_LAYER_MASK:
      g_value_set_object (value, layer_mask_undo->layer_mask);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_layer_mask_undo_get_memsize (PicmanObject *object,
                                  gint64     *gui_size)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (object);
  PicmanLayer         *layer           = PICMAN_LAYER (PICMAN_ITEM_UNDO (object)->item);
  gint64             memsize         = 0;

  /* don't use !picman_item_is_attached() here */
  if (picman_layer_get_mask (layer) != layer_mask_undo->layer_mask)
    memsize += picman_object_get_memsize (PICMAN_OBJECT (layer_mask_undo->layer_mask),
                                        gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_layer_mask_undo_pop (PicmanUndo            *undo,
                          PicmanUndoMode         undo_mode,
                          PicmanUndoAccumulator *accum)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (undo);
  PicmanLayer         *layer           = PICMAN_LAYER (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if ((undo_mode       == PICMAN_UNDO_MODE_UNDO &&
       undo->undo_type == PICMAN_UNDO_LAYER_MASK_ADD) ||
      (undo_mode       == PICMAN_UNDO_MODE_REDO &&
       undo->undo_type == PICMAN_UNDO_LAYER_MASK_REMOVE))
    {
      /*  remove layer mask  */

      picman_layer_apply_mask (layer, PICMAN_MASK_DISCARD, FALSE);
    }
  else
    {
      /*  restore layer mask  */

      picman_layer_add_mask (layer, layer_mask_undo->layer_mask, FALSE, NULL);
    }
}

static void
picman_layer_mask_undo_free (PicmanUndo     *undo,
                           PicmanUndoMode  undo_mode)
{
  PicmanLayerMaskUndo *layer_mask_undo = PICMAN_LAYER_MASK_UNDO (undo);

  if (layer_mask_undo->layer_mask)
    {
      g_object_unref (layer_mask_undo->layer_mask);
      layer_mask_undo->layer_mask = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
