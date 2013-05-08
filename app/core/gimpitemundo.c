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
#include "picmanitem.h"
#include "picmanitemundo.h"


enum
{
  PROP_0,
  PROP_ITEM
};


static void   picman_item_undo_constructed  (GObject      *object);
static void   picman_item_undo_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec);
static void   picman_item_undo_get_property (GObject      *object,
                                           guint         property_id,
                                           GValue       *value,
                                           GParamSpec   *pspec);

static void   picman_item_undo_free         (PicmanUndo     *undo,
                                           PicmanUndoMode  undo_mode);


G_DEFINE_TYPE (PicmanItemUndo, picman_item_undo, PICMAN_TYPE_UNDO)

#define parent_class picman_item_undo_parent_class


static void
picman_item_undo_class_init (PicmanItemUndoClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
  PicmanUndoClass *undo_class   = PICMAN_UNDO_CLASS (klass);

  object_class->constructed  = picman_item_undo_constructed;
  object_class->set_property = picman_item_undo_set_property;
  object_class->get_property = picman_item_undo_get_property;

  undo_class->free           = picman_item_undo_free;

  g_object_class_install_property (object_class, PROP_ITEM,
                                   g_param_spec_object ("item", NULL, NULL,
                                                        PICMAN_TYPE_ITEM,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_item_undo_init (PicmanItemUndo *undo)
{
}

static void
picman_item_undo_constructed (GObject *object)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_ITEM (item_undo->item));
}

static void
picman_item_undo_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (object);

  switch (property_id)
    {
    case PROP_ITEM:
      item_undo->item = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_item_undo_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (object);

  switch (property_id)
    {
    case PROP_ITEM:
      g_value_set_object (value, item_undo->item);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_item_undo_free (PicmanUndo     *undo,
                     PicmanUndoMode  undo_mode)
{
  PicmanItemUndo *item_undo = PICMAN_ITEM_UNDO (undo);

  if (item_undo->item)
    {
      g_object_unref (item_undo->item);
      item_undo->item = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
