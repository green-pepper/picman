/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaninputdevicestore-gudev.c
 * Input device store based on GUdev, the hardware abstraction layer.
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
 *               2011  Michael Natterer <mitch@picman.org>
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

#include <string.h>

#include <gtk/gtk.h>

#include "picmaninputdevicestore.h"

#include "libpicmanmodule/picmanmodule.h"


#ifdef HAVE_LIBGUDEV

#include <gudev/gudev.h>

enum
{
  COLUMN_IDENTIFIER,
  COLUMN_LABEL,
  COLUMN_DEVICE_FILE,
  NUM_COLUMNS
};

enum
{
  PROP_0,
  PROP_CONSTRUCT_ERROR
};

enum
{
  DEVICE_ADDED,
  DEVICE_REMOVED,
  LAST_SIGNAL
};

typedef struct _PicmanInputDeviceStoreClass PicmanInputDeviceStoreClass;

struct _PicmanInputDeviceStore
{
  GtkListStore  parent_instance;

  GUdevClient  *client;
  GError       *error;
};


struct _PicmanInputDeviceStoreClass
{
  GtkListStoreClass   parent_class;

  void  (* device_added)   (PicmanInputDeviceStore *store,
                            const gchar          *identifier);
  void  (* device_removed) (PicmanInputDeviceStore *store,
                            const gchar          *identifier);
};


static void      picman_input_device_store_finalize   (GObject              *object);

static gboolean  picman_input_device_store_add        (PicmanInputDeviceStore *store,
                                                     GUdevDevice          *device);
static gboolean  picman_input_device_store_remove     (PicmanInputDeviceStore *store,
                                                     GUdevDevice          *device);

static void      picman_input_device_store_uevent     (GUdevClient          *client,
                                                     const gchar          *action,
                                                     GUdevDevice          *device,
                                                     PicmanInputDeviceStore *store);


G_DEFINE_DYNAMIC_TYPE (PicmanInputDeviceStore, picman_input_device_store,
                       GTK_TYPE_LIST_STORE)

static guint store_signals[LAST_SIGNAL] = { 0 };


void
picman_input_device_store_register_types (GTypeModule *module)
{
  picman_input_device_store_register_type (module);
}

static void
picman_input_device_store_class_init (PicmanInputDeviceStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  store_signals[DEVICE_ADDED] =
    g_signal_new ("device-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanInputDeviceStoreClass, device_added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  store_signals[DEVICE_REMOVED] =
    g_signal_new ("device-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanInputDeviceStoreClass, device_removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  object_class->finalize = picman_input_device_store_finalize;

  klass->device_added    = NULL;
  klass->device_removed  = NULL;
}

static void
picman_input_device_store_class_finalize (PicmanInputDeviceStoreClass *klass)
{
}

static void
picman_input_device_store_init (PicmanInputDeviceStore *store)
{
  GType        types[]      = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING };
  const gchar *subsystems[] = { "input", NULL };
  GList       *devices;
  GList       *list;

  gtk_list_store_set_column_types (GTK_LIST_STORE (store),
                                   G_N_ELEMENTS (types), types);

  store->client = g_udev_client_new (subsystems);

  devices = g_udev_client_query_by_subsystem (store->client, "input");

  for (list = devices; list; list = g_list_next (list))
    {
      GUdevDevice *device = list->data;

      picman_input_device_store_add (store, device);
      g_object_unref (device);
    }

  g_list_free (devices);

  g_signal_connect (store->client, "uevent",
                    G_CALLBACK (picman_input_device_store_uevent),
                    store);
}

static void
picman_input_device_store_finalize (GObject *object)
{
  PicmanInputDeviceStore *store = PICMAN_INPUT_DEVICE_STORE (object);

  if (store->client)
    {
      g_object_unref (store->client);
      store->client = NULL;
    }

  if (store->error)
    {
      g_error_free (store->error);
      store->error = NULL;
    }

  G_OBJECT_CLASS (picman_input_device_store_parent_class)->finalize (object);
}

static gboolean
picman_input_device_store_lookup (PicmanInputDeviceStore *store,
                                const gchar          *identifier,
                                GtkTreeIter          *iter)
{
  GtkTreeModel *model = GTK_TREE_MODEL (store);
  GValue        value = { 0, };
  gboolean      iter_valid;

  for (iter_valid = gtk_tree_model_get_iter_first (model, iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, iter))
    {
      const gchar *str;

      gtk_tree_model_get_value (model, iter, COLUMN_IDENTIFIER, &value);

      str = g_value_get_string (&value);

      if (strcmp (str, identifier) == 0)
        {
          g_value_unset (&value);
          break;
        }

      g_value_unset (&value);
    }

  return iter_valid;
}

/*  insert in alphabetic order  */
static void
picman_input_device_store_insert (PicmanInputDeviceStore *store,
                                const gchar          *identifier,
                                const gchar          *label,
                                const gchar          *device_file)
{
  GtkTreeModel *model = GTK_TREE_MODEL (store);
  GtkTreeIter   iter;
  GValue        value = { 0, };
  gint          pos   = 0;
  gboolean      iter_valid;

  for (iter_valid = gtk_tree_model_get_iter_first (model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (model, &iter), pos++)
    {
      const gchar *str;

      gtk_tree_model_get_value (model, &iter, COLUMN_LABEL, &value);

      str = g_value_get_string (&value);

      if (g_utf8_collate (label, str) < 0)
        {
          g_value_unset (&value);
          break;
        }

      g_value_unset (&value);
    }

  gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, pos,
                                     COLUMN_IDENTIFIER,  identifier,
                                     COLUMN_LABEL,       label,
                                     COLUMN_DEVICE_FILE, device_file,
                                     -1);
}

static gboolean
picman_input_device_store_add (PicmanInputDeviceStore *store,
                             GUdevDevice          *device)
{
  const gchar *device_file = g_udev_device_get_device_file (device);
#if 0
  const gchar *path        = g_udev_device_get_sysfs_path (device);
#endif
  const gchar *name        = g_udev_device_get_sysfs_attr (device, "name");

#if 0
  g_printerr ("\ndevice added: %s, %s, %s\n",
              name ? name : "NULL",
              device_file ? device_file : "NULL",
              path);
#endif

  if (device_file)
    {
      if (name)
        {
          GtkTreeIter unused;

          if (! picman_input_device_store_lookup (store, name, &unused))
            {
              picman_input_device_store_insert (store, name, name, device_file);

              g_signal_emit (store, store_signals[DEVICE_ADDED], 0,
                             name);

              return TRUE;
            }
        }
      else
        {
          GUdevDevice *parent = g_udev_device_get_parent (device);

          if (parent)
            {
              const gchar *parent_name;

              parent_name = g_udev_device_get_sysfs_attr (parent, "name");

              if (parent_name)
                {
                  GtkTreeIter unused;

                  if (! picman_input_device_store_lookup (store, parent_name,
                                                        &unused))
                    {
                      picman_input_device_store_insert (store,
                                                      parent_name, parent_name,
                                                      device_file);

                      g_signal_emit (store, store_signals[DEVICE_ADDED], 0,
                                     parent_name);

                      g_object_unref (parent);
                      return TRUE;
                    }
                }

              g_object_unref (parent);
            }
        }
    }

  return FALSE;
}

static gboolean
picman_input_device_store_remove (PicmanInputDeviceStore *store,
                                GUdevDevice          *device)
{
  const gchar *name = g_udev_device_get_sysfs_attr (device, "name");
  GtkTreeIter  iter;

  if (name)
    {
      if (picman_input_device_store_lookup (store, name, &iter))
        {
          gtk_list_store_remove (GTK_LIST_STORE (store), &iter);

          g_signal_emit (store, store_signals[DEVICE_REMOVED], 0, name);

          return TRUE;
        }
    }

  return FALSE;
}

static void
picman_input_device_store_uevent (GUdevClient          *client,
                                const gchar          *action,
                                GUdevDevice          *device,
                                PicmanInputDeviceStore *store)
{
  if (! strcmp (action, "add"))
    {
      picman_input_device_store_add (store, device);
    }
  else if (! strcmp (action, "remove"))
    {
      picman_input_device_store_remove (store, device);
    }
}

PicmanInputDeviceStore *
picman_input_device_store_new (void)
{
  return g_object_new (PICMAN_TYPE_INPUT_DEVICE_STORE, NULL);
}

gchar *
picman_input_device_store_get_device_file (PicmanInputDeviceStore *store,
                                         const gchar          *identifier)
{
  GtkTreeIter iter;

  g_return_val_if_fail (PICMAN_IS_INPUT_DEVICE_STORE (store), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  if (! store->client)
    return NULL;

  if (picman_input_device_store_lookup (store, identifier, &iter))
    {
      GtkTreeModel *model = GTK_TREE_MODEL (store);
      gchar        *device_file;

      gtk_tree_model_get (model, &iter,
                          COLUMN_DEVICE_FILE, &device_file,
                          -1);

      return device_file;
    }

  return NULL;
}

GError *
picman_input_device_store_get_error (PicmanInputDeviceStore  *store)
{
  g_return_val_if_fail (PICMAN_IS_INPUT_DEVICE_STORE (store), NULL);

  return store->error ? g_error_copy (store->error) : NULL;
}

#else /* HAVE_LIBGUDEV */

void
picman_input_device_store_register_types (GTypeModule *module)
{
}

GType
picman_input_device_store_get_type (void)
{
  return G_TYPE_NONE;
}

PicmanInputDeviceStore *
picman_input_device_store_new (void)
{
  return NULL;
}

gchar *
picman_input_device_store_get_device_file (PicmanInputDeviceStore *store,
                                         const gchar          *identifier)
{
  return NULL;
}

GError *
picman_input_device_store_get_error (PicmanInputDeviceStore  *store)
{
  return NULL;
}

#endif /* HAVE_LIBGUDEV */
