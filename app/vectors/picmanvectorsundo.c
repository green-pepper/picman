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

#include "core/picmanimage.h"

#include "picmanvectors.h"
#include "picmanvectorsundo.h"


enum
{
  PROP_0,
  PROP_PREV_PARENT,
  PROP_PREV_POSITION,
  PROP_PREV_VECTORS
};


static void     picman_vectors_undo_constructed  (GObject             *object);
static void     picman_vectors_undo_set_property (GObject             *object,
                                                guint                property_id,
                                                const GValue        *value,
                                                GParamSpec          *pspec);
static void     picman_vectors_undo_get_property (GObject             *object,
                                                guint                property_id,
                                                GValue              *value,
                                                GParamSpec          *pspec);

static gint64   picman_vectors_undo_get_memsize  (PicmanObject          *object,
                                                gint64              *gui_size);

static void     picman_vectors_undo_pop          (PicmanUndo            *undo,
                                                PicmanUndoMode         undo_mode,
                                                PicmanUndoAccumulator *accum);


G_DEFINE_TYPE (PicmanVectorsUndo, picman_vectors_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_vectors_undo_parent_class


static void
picman_vectors_undo_class_init (PicmanVectorsUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_vectors_undo_constructed;
  object_class->set_property     = picman_vectors_undo_set_property;
  object_class->get_property     = picman_vectors_undo_get_property;

  picman_object_class->get_memsize = picman_vectors_undo_get_memsize;

  undo_class->pop                = picman_vectors_undo_pop;

  g_object_class_install_property (object_class, PROP_PREV_PARENT,
                                   g_param_spec_object ("prev-parent",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_VECTORS,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_POSITION,
                                   g_param_spec_int ("prev-position", NULL, NULL,
                                                     0, G_MAXINT, 0,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_PREV_VECTORS,
                                   g_param_spec_object ("prev-vectors", NULL, NULL,
                                                        PICMAN_TYPE_VECTORS,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_vectors_undo_init (PicmanVectorsUndo *undo)
{
}

static void
picman_vectors_undo_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_VECTORS (PICMAN_ITEM_UNDO (object)->item));
}

static void
picman_vectors_undo_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanVectorsUndo *vectors_undo = PICMAN_VECTORS_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      vectors_undo->prev_parent = g_value_get_object (value);
      break;
    case PROP_PREV_POSITION:
      vectors_undo->prev_position = g_value_get_int (value);
      break;
    case PROP_PREV_VECTORS:
      vectors_undo->prev_vectors = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_vectors_undo_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanVectorsUndo *vectors_undo = PICMAN_VECTORS_UNDO (object);

  switch (property_id)
    {
    case PROP_PREV_PARENT:
      g_value_set_object (value, vectors_undo->prev_parent);
      break;
    case PROP_PREV_POSITION:
      g_value_set_int (value, vectors_undo->prev_position);
      break;
    case PROP_PREV_VECTORS:
      g_value_set_object (value, vectors_undo->prev_vectors);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_vectors_undo_get_memsize (PicmanObject *object,
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
picman_vectors_undo_pop (PicmanUndo            *undo,
                       PicmanUndoMode         undo_mode,
                       PicmanUndoAccumulator *accum)
{
  PicmanVectorsUndo *vectors_undo = PICMAN_VECTORS_UNDO (undo);
  PicmanVectors     *vectors      = PICMAN_VECTORS (PICMAN_ITEM_UNDO (undo)->item);

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  if ((undo_mode       == PICMAN_UNDO_MODE_UNDO &&
       undo->undo_type == PICMAN_UNDO_VECTORS_ADD) ||
      (undo_mode       == PICMAN_UNDO_MODE_REDO &&
       undo->undo_type == PICMAN_UNDO_VECTORS_REMOVE))
    {
      /*  remove vectors  */

      /*  record the current parent and position  */
      vectors_undo->prev_parent   = picman_vectors_get_parent (vectors);
      vectors_undo->prev_position = picman_item_get_index (PICMAN_ITEM (vectors));

      picman_image_remove_vectors (undo->image, vectors, FALSE,
                                 vectors_undo->prev_vectors);
    }
  else
    {
      /*  restore vectors  */

      /*  record the active vectors  */
      vectors_undo->prev_vectors = picman_image_get_active_vectors (undo->image);

      picman_image_add_vectors (undo->image, vectors,
                              vectors_undo->prev_parent,
                              vectors_undo->prev_position, FALSE);
    }
}
