/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanidtable.c
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
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

#include "picmanidtable.h"
#include "picman-utils.h"


#define PICMAN_ID_TABLE_START_ID 1
#define PICMAN_ID_TABLE_END_ID   G_MAXINT


struct _PicmanIdTablePriv
{
  GHashTable *id_table;
  gint        next_id;
};


static void    picman_id_table_finalize    (GObject    *object);
static gint64  picman_id_table_get_memsize (PicmanObject *object,
                                          gint64     *gui_size);


G_DEFINE_TYPE (PicmanIdTable, picman_id_table, PICMAN_TYPE_OBJECT)

#define parent_class picman_id_table_parent_class


static void
picman_id_table_class_init (PicmanIdTableClass *klass)
{
  GObjectClass     *object_class        = G_OBJECT_CLASS (klass);
  PicmanObjectClass  *picman_object_class   = PICMAN_OBJECT_CLASS (klass);
  PicmanIdTableClass *picman_id_table_class = PICMAN_ID_TABLE_CLASS (klass);

  object_class->finalize         = picman_id_table_finalize;

  picman_object_class->get_memsize = picman_id_table_get_memsize;

  g_type_class_add_private (picman_id_table_class,
                            sizeof (PicmanIdTablePriv));
}

static void
picman_id_table_init (PicmanIdTable *id_table)
{
  id_table->priv = G_TYPE_INSTANCE_GET_PRIVATE (id_table,
                                                PICMAN_TYPE_ID_TABLE,
                                                PicmanIdTablePriv);

  id_table->priv->id_table = g_hash_table_new (g_direct_hash, NULL);
  id_table->priv->next_id  = PICMAN_ID_TABLE_START_ID;
}

static void
picman_id_table_finalize (GObject *object)
{
  PicmanIdTable *id_table = PICMAN_ID_TABLE (object);

  if (id_table->priv->id_table)
    {
      g_hash_table_unref (id_table->priv->id_table);
      id_table->priv->id_table = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_id_table_get_memsize (PicmanObject *object,
                           gint64     *gui_size)
{
  PicmanIdTable *id_table = PICMAN_ID_TABLE (object);
  gint64       memsize  = 0;

  memsize += picman_g_hash_table_get_memsize (id_table->priv->id_table, 0);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

/**
 * picman_id_table_new:
 *
 * Returns: A new #PicmanIdTable.
 **/
PicmanIdTable *
picman_id_table_new (void)
{
  return g_object_new (PICMAN_TYPE_ID_TABLE, NULL);
}

/**
 * picman_id_table_insert:
 * @id_table: A #PicmanIdTable
 * @data: Data to insert and assign an id to
 *
 * Insert data in the id table. The data will get an, in this table,
 * unused ID assigned to it that can be used to later lookup the data.
 *
 * Returns: The assigned ID.
 **/
gint
picman_id_table_insert (PicmanIdTable *id_table, gpointer data)
{
  gint new_id;

  g_return_val_if_fail (PICMAN_IS_ID_TABLE (id_table), 0);

  do
    {
      new_id = id_table->priv->next_id++;

      if (id_table->priv->next_id == PICMAN_ID_TABLE_END_ID)
        id_table->priv->next_id = PICMAN_ID_TABLE_START_ID;
    }
  while (picman_id_table_lookup (id_table, new_id));

  return picman_id_table_insert_with_id (id_table, new_id, data);
}

/**
 * picman_id_table_insert_with_id:
 * @id_table: An #PicmanIdTable
 * @id: The ID to use. Must be greater than 0.
 * @data: The data to associate with the id
 *
 * Insert data in the id table with a specific ID. If data already
 * exsts with the given ID, this function fails.
 *
 * Returns: The used ID if successful, -1 if it was already in use.
 **/
gint
picman_id_table_insert_with_id (PicmanIdTable *id_table, gint id, gpointer data)
{
  g_return_val_if_fail (PICMAN_IS_ID_TABLE (id_table), 0);
  g_return_val_if_fail (id > 0, 0);

  if (picman_id_table_lookup (id_table, id))
    return -1;

  g_hash_table_insert (id_table->priv->id_table, GINT_TO_POINTER (id), data);

  return id;
}

/**
 * picman_id_table_replace:
 * @id_table: An #PicmanIdTable
 * @id: The ID to use. Must be greater than 0.
 * @data: The data to insert/replace
 *
 * Replaces (if an item with the given ID exists) or inserts a new
 * entry in the id table.
 **/
void
picman_id_table_replace (PicmanIdTable *id_table, gint id, gpointer data)
{
  g_return_if_fail (PICMAN_IS_ID_TABLE (id_table));
  g_return_if_fail (id > 0);

  g_hash_table_replace (id_table->priv->id_table, GINT_TO_POINTER (id), data);
}

/**
 * picman_id_table_lookup:
 * @id_table: An #PicmanIdTable
 * @id: The ID of the data to lookup
 *
 * Lookup data based on ID.
 *
 * Returns: The data, or NULL if no data with the given ID was found.
 **/
gpointer
picman_id_table_lookup (PicmanIdTable *id_table, gint id)
{
  g_return_val_if_fail (PICMAN_IS_ID_TABLE (id_table), NULL);

  return g_hash_table_lookup (id_table->priv->id_table, GINT_TO_POINTER (id));
}


/**
 * picman_id_table_remove:
 * @id_table: An #PicmanIdTable
 * @id: The ID of the data to remove.
 *
 * Remove the data from the table with the given ID.
 *
 * Returns: %TRUE if data with the ID existed and was successfully
 *          removed, %FALSE otherwise.
 **/
gboolean
picman_id_table_remove (PicmanIdTable *id_table, gint id)
{
  g_return_val_if_fail (PICMAN_IS_ID_TABLE (id_table), FALSE);

  g_return_val_if_fail (id_table != NULL, FALSE);

  return g_hash_table_remove (id_table->priv->id_table, GINT_TO_POINTER (id));
}
