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

#include "picman-utils.h"
#include "picmanitem.h"
#include "picmanitemtree.h"
#include "picmanitempropundo.h"
#include "picmanparasitelist.h"


enum
{
  PROP_0,
  PROP_PARASITE_NAME
};


static void     picman_item_prop_undo_constructed  (GObject             *object);
static void     picman_item_prop_undo_set_property (GObject             *object,
                                                  guint                property_id,
                                                  const GValue        *value,
                                                  GParamSpec          *pspec);
static void     picman_item_prop_undo_get_property (GObject             *object,
                                                  guint                property_id,
                                                  GValue              *value,
                                                  GParamSpec          *pspec);

static gint64   picman_item_prop_undo_get_memsize  (PicmanObject          *object,
                                                  gint64              *gui_size);

static void     picman_item_prop_undo_pop          (PicmanUndo            *undo,
                                                  PicmanUndoMode         undo_mode,
                                                  PicmanUndoAccumulator *accum);
static void     picman_item_prop_undo_free         (PicmanUndo            *undo,
                                                  PicmanUndoMode         undo_mode);


G_DEFINE_TYPE (PicmanItemPropUndo, picman_item_prop_undo, PICMAN_TYPE_ITEM_UNDO)

#define parent_class picman_item_prop_undo_parent_class


static void
picman_item_prop_undo_class_init (PicmanItemPropUndoClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanUndoClass   *undo_class        = PICMAN_UNDO_CLASS (klass);

  object_class->constructed      = picman_item_prop_undo_constructed;
  object_class->set_property     = picman_item_prop_undo_set_property;
  object_class->get_property     = picman_item_prop_undo_get_property;

  picman_object_class->get_memsize = picman_item_prop_undo_get_memsize;

  undo_class->pop                = picman_item_prop_undo_pop;
  undo_class->free               = picman_item_prop_undo_free;

  g_object_class_install_property (object_class, PROP_PARASITE_NAME,
                                   g_param_spec_string ("parasite-name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_item_prop_undo_init (PicmanItemPropUndo *undo)
{
}

static void
picman_item_prop_undo_constructed (GObject *object)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (object);
  PicmanItem         *item;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  item = PICMAN_ITEM_UNDO (object)->item;

  switch (PICMAN_UNDO (object)->undo_type)
    {
    case PICMAN_UNDO_ITEM_REORDER:
      item_prop_undo->parent   = picman_item_get_parent (item);
      item_prop_undo->position = picman_item_get_index (item);
      break;

    case PICMAN_UNDO_ITEM_RENAME:
      item_prop_undo->name = g_strdup (picman_object_get_name (item));
      break;

    case PICMAN_UNDO_ITEM_DISPLACE:
      picman_item_get_offset (item,
                            &item_prop_undo->offset_x,
                            &item_prop_undo->offset_y);
      break;

    case PICMAN_UNDO_ITEM_VISIBILITY:
      item_prop_undo->visible = picman_item_get_visible (item);
      break;

    case PICMAN_UNDO_ITEM_LINKED:
      item_prop_undo->linked  = picman_item_get_linked (item);
      break;

    case PICMAN_UNDO_ITEM_LOCK_CONTENT:
      item_prop_undo->lock_content = picman_item_get_lock_content (item);
      break;

    case PICMAN_UNDO_ITEM_LOCK_POSITION:
      item_prop_undo->lock_position = picman_item_get_lock_position (item);
      break;

    case PICMAN_UNDO_PARASITE_ATTACH:
    case PICMAN_UNDO_PARASITE_REMOVE:
      g_assert (item_prop_undo->parasite_name != NULL);

      item_prop_undo->parasite = picman_parasite_copy
        (picman_item_parasite_find (item, item_prop_undo->parasite_name));
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_item_prop_undo_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (object);

  switch (property_id)
    {
    case PROP_PARASITE_NAME:
      item_prop_undo->parasite_name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_item_prop_undo_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (object);

  switch (property_id)
    {
    case PROP_PARASITE_NAME:
      g_value_set_string (value, item_prop_undo->parasite_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_item_prop_undo_get_memsize (PicmanObject *object,
                                 gint64     *gui_size)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (object);
  gint64            memsize        = 0;

  memsize += picman_string_get_memsize (item_prop_undo->name);
  memsize += picman_string_get_memsize (item_prop_undo->parasite_name);
  memsize += picman_parasite_get_memsize (item_prop_undo->parasite, NULL);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_item_prop_undo_pop (PicmanUndo            *undo,
                         PicmanUndoMode         undo_mode,
                         PicmanUndoAccumulator *accum)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (undo);
  PicmanItem         *item           = PICMAN_ITEM_UNDO (undo)->item;

  PICMAN_UNDO_CLASS (parent_class)->pop (undo, undo_mode, accum);

  switch (undo->undo_type)
    {
    case PICMAN_UNDO_ITEM_REORDER:
      {
        PicmanItem *parent;
        gint      position;

        parent   = picman_item_get_parent (item);
        position = picman_item_get_index (item);

        picman_item_tree_reorder_item (picman_item_get_tree (item), item,
                                     item_prop_undo->parent,
                                     item_prop_undo->position,
                                     FALSE, NULL);

        item_prop_undo->parent   = parent;
        item_prop_undo->position = position;
      }
      break;

    case PICMAN_UNDO_ITEM_RENAME:
      {
        gchar *name;

        name = g_strdup (picman_object_get_name (item));

        picman_item_tree_rename_item (picman_item_get_tree (item), item,
                                    item_prop_undo->name,
                                    FALSE, NULL);

        g_free (item_prop_undo->name);
        item_prop_undo->name = name;
      }
      break;

    case PICMAN_UNDO_ITEM_DISPLACE:
      {
        gint offset_x;
        gint offset_y;

        picman_item_get_offset (item, &offset_x, &offset_y);

        picman_item_translate (item,
                             item_prop_undo->offset_x - offset_x,
                             item_prop_undo->offset_y - offset_y,
                             FALSE);

        item_prop_undo->offset_x = offset_x;
        item_prop_undo->offset_y = offset_y;
      }
      break;

    case PICMAN_UNDO_ITEM_VISIBILITY:
      {
        gboolean visible;

        visible = picman_item_get_visible (item);
        picman_item_set_visible (item, item_prop_undo->visible, FALSE);
        item_prop_undo->visible = visible;
      }
      break;

    case PICMAN_UNDO_ITEM_LINKED:
      {
        gboolean linked;

        linked = picman_item_get_linked (item);
        picman_item_set_linked (item, item_prop_undo->linked, FALSE);
        item_prop_undo->linked = linked;
      }
      break;

    case PICMAN_UNDO_ITEM_LOCK_CONTENT:
      {
        gboolean lock_content;

        lock_content = picman_item_get_lock_content (item);
        picman_item_set_lock_content (item, item_prop_undo->lock_content, FALSE);
        item_prop_undo->lock_content = lock_content;
      }
      break;

    case PICMAN_UNDO_ITEM_LOCK_POSITION:
      {
        gboolean lock_position;

        lock_position = picman_item_get_lock_position (item);
        picman_item_set_lock_position (item, item_prop_undo->lock_position, FALSE);
        item_prop_undo->lock_position = lock_position;
      }
      break;

    case PICMAN_UNDO_PARASITE_ATTACH:
    case PICMAN_UNDO_PARASITE_REMOVE:
      {
        PicmanParasite *parasite;

        parasite = item_prop_undo->parasite;

        item_prop_undo->parasite = picman_parasite_copy
          (picman_item_parasite_find (item, item_prop_undo->parasite_name));

        if (parasite)
          picman_item_parasite_attach (item, parasite, FALSE);
        else
          picman_item_parasite_detach (item, item_prop_undo->parasite_name, FALSE);

        if (parasite)
          picman_parasite_free (parasite);
      }
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
picman_item_prop_undo_free (PicmanUndo     *undo,
                          PicmanUndoMode  undo_mode)
{
  PicmanItemPropUndo *item_prop_undo = PICMAN_ITEM_PROP_UNDO (undo);

  if (item_prop_undo->name)
    {
      g_free (item_prop_undo->name);
      item_prop_undo->name = NULL;
    }

  if (item_prop_undo->parasite_name)
    {
      g_free (item_prop_undo->parasite_name);
      item_prop_undo->parasite_name = NULL;
    }

  if (item_prop_undo->parasite)
    {
      picman_parasite_free (item_prop_undo->parasite);
      item_prop_undo->parasite = NULL;
    }

  PICMAN_UNDO_CLASS (parent_class)->free (undo, undo_mode);
}
