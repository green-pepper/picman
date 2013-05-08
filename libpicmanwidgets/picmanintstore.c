/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanintstore.c
 * Copyright (C) 2004-2007  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanintstore.h"

#include "libpicman/libpicman-intl.h"


/**
 * SECTION: picmanintstore
 * @title: PicmanIntStore
 * @short_description: A model for integer based name-value pairs
 *                     (e.g. enums)
 *
 * A model for integer based name-value pairs (e.g. enums)
 **/


enum
{
  PROP_0,
  PROP_USER_DATA_TYPE
};

typedef struct
{
  GType  user_data_type;
} PicmanIntStorePrivate;


static void  picman_int_store_tree_model_init (GtkTreeModelIface *iface);

static void  picman_int_store_constructed     (GObject           *object);
static void  picman_int_store_finalize        (GObject           *object);
static void  picman_int_store_set_property    (GObject           *object,
                                             guint              property_id,
                                             const GValue      *value,
                                             GParamSpec        *pspec);
static void  picman_int_store_get_property    (GObject           *object,
                                             guint              property_id,
                                             GValue            *value,
                                             GParamSpec        *pspec);

static void  picman_int_store_row_inserted    (GtkTreeModel      *model,
                                             GtkTreePath       *path,
                                             GtkTreeIter       *iter);
static void  picman_int_store_row_deleted     (GtkTreeModel      *model,
                                             GtkTreePath       *path);
static void  picman_int_store_add_empty       (PicmanIntStore      *store);


G_DEFINE_TYPE_WITH_CODE (PicmanIntStore, picman_int_store, GTK_TYPE_LIST_STORE,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL,
                                                picman_int_store_tree_model_init))

#define PICMAN_INT_STORE_GET_PRIVATE(obj) \
  G_TYPE_INSTANCE_GET_PRIVATE (obj, PICMAN_TYPE_INT_STORE, PicmanIntStorePrivate)

#define parent_class picman_int_store_parent_class

static GtkTreeModelIface *parent_iface = NULL;


static void
picman_int_store_class_init (PicmanIntStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_int_store_constructed;
  object_class->finalize     = picman_int_store_finalize;
  object_class->set_property = picman_int_store_set_property;
  object_class->get_property = picman_int_store_get_property;

  /**
   * PicmanIntStore:user-data-type:
   *
   * Sets the #GType for the PICMAN_INT_STORE_USER_DATA column.
   *
   * You need to set this property when constructing the store if you want
   * to use the PICMAN_INT_STORE_USER_DATA column and want to have the store
   * handle ref-counting of your user data.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class,
                                   PROP_USER_DATA_TYPE,
                                   g_param_spec_gtype ("user-data-type",
                                                       NULL, NULL,
                                                       G_TYPE_NONE,
                                                       G_PARAM_CONSTRUCT_ONLY |
                                                       PICMAN_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (PicmanIntStorePrivate));
}

static void
picman_int_store_tree_model_init (GtkTreeModelIface *iface)
{
  parent_iface = g_type_interface_peek_parent (iface);

  iface->row_inserted = picman_int_store_row_inserted;
  iface->row_deleted  = picman_int_store_row_deleted;
}

static void
picman_int_store_init (PicmanIntStore *store)
{
  store->empty_iter = NULL;
}

static void
picman_int_store_constructed (GObject *object)
{
  PicmanIntStore        *store = PICMAN_INT_STORE (object);
  PicmanIntStorePrivate *priv  = PICMAN_INT_STORE_GET_PRIVATE (store);
  GType                types[PICMAN_INT_STORE_NUM_COLUMNS];

  G_OBJECT_CLASS (parent_class)->constructed (object);

  types[PICMAN_INT_STORE_VALUE]     = G_TYPE_INT;
  types[PICMAN_INT_STORE_LABEL]     = G_TYPE_STRING;
  types[PICMAN_INT_STORE_STOCK_ID]  = G_TYPE_STRING;
  types[PICMAN_INT_STORE_PIXBUF]    = GDK_TYPE_PIXBUF;
  types[PICMAN_INT_STORE_USER_DATA] = (priv->user_data_type != G_TYPE_NONE ?
                                     priv->user_data_type : G_TYPE_POINTER);

  gtk_list_store_set_column_types (GTK_LIST_STORE (store),
                                   PICMAN_INT_STORE_NUM_COLUMNS, types);

  picman_int_store_add_empty (store);
}

static void
picman_int_store_finalize (GObject *object)
{
  PicmanIntStore *store = PICMAN_INT_STORE (object);

  if (store->empty_iter)
    {
      gtk_tree_iter_free (store->empty_iter);
      store->empty_iter = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_int_store_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanIntStorePrivate *priv = PICMAN_INT_STORE_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_USER_DATA_TYPE:
      priv->user_data_type = g_value_get_gtype (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_int_store_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanIntStorePrivate *priv = PICMAN_INT_STORE_GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_USER_DATA_TYPE:
      g_value_set_gtype (value, priv->user_data_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_int_store_row_inserted (GtkTreeModel *model,
                             GtkTreePath  *path,
                             GtkTreeIter  *iter)
{
  PicmanIntStore *store = PICMAN_INT_STORE (model);

  if (parent_iface->row_inserted)
    parent_iface->row_inserted (model, path, iter);

  if (store->empty_iter &&
      memcmp (iter, store->empty_iter, sizeof (GtkTreeIter)))
    {
      gtk_list_store_remove (GTK_LIST_STORE (store), store->empty_iter);
    }
}

static void
picman_int_store_row_deleted (GtkTreeModel *model,
                            GtkTreePath  *path)
{
  PicmanIntStore *store = PICMAN_INT_STORE (model);

  if (parent_iface->row_deleted)
    parent_iface->row_deleted (model, path);

  if (store->empty_iter)
    {
      /* freeing here crashes, no clue why. will be freed in finalize() */
      /* gtk_tree_iter_free (store->empty_iter); */
      store->empty_iter = NULL;
    }
}

static void
picman_int_store_add_empty (PicmanIntStore *store)
{
  GtkTreeIter iter = { 0, };

  g_return_if_fail (store->empty_iter == NULL);

  gtk_list_store_prepend (GTK_LIST_STORE (store), &iter);
  gtk_list_store_set (GTK_LIST_STORE (store), &iter,
                      PICMAN_INT_STORE_VALUE, -1,
                      /* This string appears in an empty menu as in
                       * "nothing selected and nothing to select"
                       */
                      PICMAN_INT_STORE_LABEL, (_("(Empty)")),
                      -1);

  store->empty_iter = gtk_tree_iter_copy (&iter);
}

/**
 * picman_int_store_new:
 *
 * Creates a #GtkListStore with a number of useful columns.
 * #PicmanIntStore is especially useful if the items you want to store
 * are identified using an integer value.
 *
 * Return value: a new #PicmanIntStore.
 *
 * Since: PICMAN 2.2
 **/
GtkListStore *
picman_int_store_new (void)
{
  return g_object_new (PICMAN_TYPE_INT_STORE, NULL);
}

/**
 * picman_int_store_lookup_by_value:
 * @model: a #PicmanIntStore
 * @value: an integer value to lookup in the @model
 * @iter:  return location for the iter of the given @value
 *
 * Iterate over the @model looking for @value.
 *
 * Return value: %TRUE if the value has been located and @iter is
 *               valid, %FALSE otherwise.
 *
 * Since: PICMAN 2.2
 **/
gboolean
picman_int_store_lookup_by_value (GtkTreeModel *model,
                                gint          value,
                                GtkTreeIter  *iter)
{
  gboolean  iter_valid;

  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);

  for (iter_valid = gtk_tree_model_get_iter_first (model, iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, iter))
    {
      gint  this;

      gtk_tree_model_get (model, iter,
                          PICMAN_INT_STORE_VALUE, &this,
                          -1);
      if (this == value)
        break;
    }

  return iter_valid;
}
