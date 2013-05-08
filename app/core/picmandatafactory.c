/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandatafactory.c
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picman.h"
#include "picmancontext.h"
#include "picmandata.h"
#include "picmandatafactory.h"
#include "picmanlist.h"

#include "picman-intl.h"


#define WRITABLE_PATH_KEY "picman-data-factory-writable-path"

/* Data files that have this string in their path are considered
 * obsolete and are only kept around for backwards compatibility
 */
#define PICMAN_OBSOLETE_DATA_DIR_NAME "picman-obsolete-files"


typedef void (* PicmanDataForeachFunc) (PicmanDataFactory *factory,
                                      PicmanData        *data,
                                      gpointer         user_data);


struct _PicmanDataFactoryPriv
{
  Picman                             *picman;
  PicmanContainer                    *container;

  PicmanContainer                    *container_obsolete;

  gchar                            *path_property_name;
  gchar                            *writable_property_name;

  const PicmanDataFactoryLoaderEntry *loader_entries;
  gint                              n_loader_entries;

  PicmanDataNewFunc                   data_new_func;
  PicmanDataGetStandardFunc           data_get_standard_func;
};


static void    picman_data_factory_finalize     (GObject              *object);

static void    picman_data_factory_data_load    (PicmanDataFactory      *factory,
                                               PicmanContext          *context,
                                               GHashTable           *cache);

static gint64  picman_data_factory_get_memsize  (PicmanObject           *object,
                                               gint64               *gui_size);

static gchar * picman_data_factory_get_save_dir (PicmanDataFactory      *factory,
                                               GError              **error);

static void    picman_data_factory_load_data  (const PicmanDatafileData *file_data,
                                             gpointer                data);

static void    picman_data_factory_load_data_recursive (const PicmanDatafileData *file_data,
                                                      gpointer                data);

G_DEFINE_TYPE (PicmanDataFactory, picman_data_factory, PICMAN_TYPE_OBJECT)

#define parent_class picman_data_factory_parent_class


static void
picman_data_factory_class_init (PicmanDataFactoryClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  object_class->finalize         = picman_data_factory_finalize;

  picman_object_class->get_memsize = picman_data_factory_get_memsize;

  g_type_class_add_private (klass, sizeof (PicmanDataFactoryPriv));
}

static void
picman_data_factory_init (PicmanDataFactory *factory)
{
  factory->priv = G_TYPE_INSTANCE_GET_PRIVATE (factory,
                                               PICMAN_TYPE_DATA_FACTORY,
                                               PicmanDataFactoryPriv);

  factory->priv->picman                   = NULL;
  factory->priv->container              = NULL;
  factory->priv->container_obsolete     = NULL;
  factory->priv->path_property_name     = NULL;
  factory->priv->writable_property_name = NULL;
  factory->priv->loader_entries         = NULL;
  factory->priv->n_loader_entries       = 0;
  factory->priv->data_new_func          = NULL;
  factory->priv->data_get_standard_func = NULL;
}

static void
picman_data_factory_finalize (GObject *object)
{
  PicmanDataFactory *factory = PICMAN_DATA_FACTORY (object);

  if (factory->priv->container)
    {
      g_object_unref (factory->priv->container);
      factory->priv->container = NULL;
    }

  if (factory->priv->container_obsolete)
    {
      g_object_unref (factory->priv->container_obsolete);
      factory->priv->container_obsolete = NULL;
    }

  if (factory->priv->path_property_name)
    {
      g_free (factory->priv->path_property_name);
      factory->priv->path_property_name = NULL;
    }

  if (factory->priv->writable_property_name)
    {
      g_free (factory->priv->writable_property_name);
      factory->priv->writable_property_name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_data_factory_get_memsize (PicmanObject *object,
                               gint64     *gui_size)
{
  PicmanDataFactory *factory = PICMAN_DATA_FACTORY (object);
  gint64           memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (factory->priv->container),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (factory->priv->container_obsolete),
                                      gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

PicmanDataFactory *
picman_data_factory_new (Picman                             *picman,
                       GType                             data_type,
                       const gchar                      *path_property_name,
                       const gchar                      *writable_property_name,
                       const PicmanDataFactoryLoaderEntry *loader_entries,
                       gint                              n_loader_entries,
                       PicmanDataNewFunc                   new_func,
                       PicmanDataGetStandardFunc           get_standard_func)
{
  PicmanDataFactory *factory;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (g_type_is_a (data_type, PICMAN_TYPE_DATA), NULL);
  g_return_val_if_fail (path_property_name != NULL, NULL);
  g_return_val_if_fail (writable_property_name != NULL, NULL);
  g_return_val_if_fail (loader_entries != NULL, NULL);
  g_return_val_if_fail (n_loader_entries > 0, NULL);

  factory = g_object_new (PICMAN_TYPE_DATA_FACTORY, NULL);

  factory->priv->picman                   = picman;
  factory->priv->container              = picman_list_new (data_type, TRUE);
  picman_list_set_sort_func (PICMAN_LIST (factory->priv->container),
			   (GCompareFunc) picman_data_compare);
  factory->priv->container_obsolete     = picman_list_new (data_type, TRUE);
  picman_list_set_sort_func (PICMAN_LIST (factory->priv->container_obsolete),
			   (GCompareFunc) picman_data_compare);

  factory->priv->path_property_name     = g_strdup (path_property_name);
  factory->priv->writable_property_name = g_strdup (writable_property_name);

  factory->priv->loader_entries         = loader_entries;
  factory->priv->n_loader_entries       = n_loader_entries;

  factory->priv->data_new_func          = new_func;
  factory->priv->data_get_standard_func = get_standard_func;

  return factory;
}

void
picman_data_factory_data_init (PicmanDataFactory *factory,
                             PicmanContext     *context,
                             gboolean         no_data)
{
  g_return_if_fail (PICMAN_IS_DATA_FACTORY (factory));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  /*  Freeze and thaw the container even if no_data,
   *  this creates the standard data that serves as fallback.
   */
  picman_container_freeze (factory->priv->container);

  if (! no_data)
    {
      if (factory->priv->picman->be_verbose)
        {
          const gchar *name = picman_object_get_name (factory);

          g_print ("Loading '%s' data\n", name ? name : "???");
        }

      picman_data_factory_data_load (factory, context, NULL);
    }

  picman_container_thaw (factory->priv->container);
}

static void
picman_data_factory_refresh_cache_add (PicmanDataFactory *factory,
                                     PicmanData        *data,
                                     gpointer         user_data)
{
  const gchar *filename = picman_data_get_filename (data);

  if (filename)
    {
      GHashTable *cache = user_data;
      GList      *list;

      g_object_ref (data);

      picman_container_remove (factory->priv->container, PICMAN_OBJECT (data));

      list = g_hash_table_lookup (cache, filename);
      list = g_list_prepend (list, data);

      g_hash_table_insert (cache, (gpointer) filename, list);
    }
}

static gboolean
picman_data_factory_refresh_cache_remove (gpointer key,
                                        gpointer value,
                                        gpointer user_data)
{
  GList *list;

  for (list = value; list; list = list->next)
    g_object_unref (list->data);

  g_list_free (value);

  return TRUE;
}

static void
picman_data_factory_data_foreach (PicmanDataFactory     *factory,
                                gboolean             skip_internal,
                                PicmanDataForeachFunc  callback,
                                gpointer             user_data)
{
  GList *list = PICMAN_LIST (factory->priv->container)->list;

  if (skip_internal)
    {
      while (list && picman_data_is_internal (PICMAN_DATA (list->data)))
        list = g_list_next (list);
    }

  while (list)
    {
      GList *next = g_list_next (list);

      callback (factory, list->data, user_data);

      list = next;
    }
}

typedef struct
{
  PicmanDataFactory *factory;
  PicmanContext     *context;
  GHashTable      *cache;
  const gchar     *top_directory;
} PicmanDataLoadContext;

static void
picman_data_factory_data_load (PicmanDataFactory *factory,
                             PicmanContext     *context,
                             GHashTable      *cache)
{
  gchar *path;
  gchar *writable_path;

  g_object_get (factory->priv->picman->config,
                factory->priv->path_property_name,     &path,
                factory->priv->writable_property_name, &writable_path,
                NULL);

  if (path && strlen (path))
    {
      GList               *writable_list = NULL;
      gchar               *tmp;
      PicmanDataLoadContext  load_context = { 0, };

      load_context.factory = factory;
      load_context.context = context;
      load_context.cache   = cache;

      tmp = picman_config_path_expand (path, TRUE, NULL);
      g_free (path);
      path = tmp;

      if (writable_path)
        {
          tmp = picman_config_path_expand (writable_path, TRUE, NULL);
          g_free (writable_path);
          writable_path = tmp;

          writable_list = picman_path_parse (writable_path, 256, TRUE, NULL);

          g_object_set_data (G_OBJECT (factory),
                             WRITABLE_PATH_KEY, writable_list);
        }

      picman_datafiles_read_directories (path, G_FILE_TEST_IS_REGULAR,
                                       picman_data_factory_load_data,
                                       &load_context);

      picman_datafiles_read_directories (path, G_FILE_TEST_IS_DIR,
                                       picman_data_factory_load_data_recursive,
                                       &load_context);

      if (writable_path)
        {
          picman_path_free (writable_list);
          g_object_set_data (G_OBJECT (factory), WRITABLE_PATH_KEY, NULL);
        }
    }

  g_free (path);
  g_free (writable_path);
}

void
picman_data_factory_data_refresh (PicmanDataFactory *factory,
                                PicmanContext     *context)
{
  GHashTable *cache;

  g_return_if_fail (PICMAN_IS_DATA_FACTORY (factory));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  picman_container_freeze (factory->priv->container);

  /*  First, save all dirty data objects  */
  picman_data_factory_data_save (factory);

  cache = g_hash_table_new (g_str_hash, g_str_equal);

  picman_data_factory_data_foreach (factory, TRUE,
                                  picman_data_factory_refresh_cache_add, cache);

  /*  Now the cache contains a filename => list-of-objects mapping of
   *  the old objects. So we should now traverse the directory and for
   *  each file load it only if its mtime is newer.
   *
   *  Once a file was added, it is removed from the cache, so the only
   *  objects remaining there will be those that are not present on
   *  the disk (that have to be destroyed)
   */
  picman_data_factory_data_load (factory, context, cache);

  /*  Now all the data is loaded. Free what remains in the cache  */
  g_hash_table_foreach_remove (cache,
                               picman_data_factory_refresh_cache_remove, NULL);

  g_hash_table_destroy (cache);

  picman_container_thaw (factory->priv->container);
}

void
picman_data_factory_data_save (PicmanDataFactory *factory)
{
  GList  *list;
  gchar  *writable_dir;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_DATA_FACTORY (factory));

  if (picman_container_is_empty (factory->priv->container))
    return;

  writable_dir = picman_data_factory_get_save_dir (factory, &error);

  if (! writable_dir)
    {
      picman_message (factory->priv->picman, NULL, PICMAN_MESSAGE_ERROR,
                    _("Failed to save data:\n\n%s"),
                    error->message);
      g_clear_error (&error);

      return;
    }

  for (list = PICMAN_LIST (factory->priv->container)->list;
       list;
       list = g_list_next (list))
    {
      PicmanData *data = list->data;

      if (! picman_data_get_filename (data))
        picman_data_create_filename (data, writable_dir);

      if (picman_data_is_dirty (data) &&
          picman_data_is_writable (data))
        {
          GError *error = NULL;

          if (! picman_data_save (data, &error))
            {
              /*  check if there actually was an error (no error
               *  means the data class does not implement save)
               */
              if (error)
                {
                  picman_message (factory->priv->picman, NULL, PICMAN_MESSAGE_ERROR,
                                _("Failed to save data:\n\n%s"),
                                error->message);
                  g_clear_error (&error);
                }
            }
        }
    }

  g_free (writable_dir);
}

static void
picman_data_factory_remove_cb (PicmanDataFactory *factory,
                             PicmanData        *data,
                             gpointer         user_data)
{
  picman_container_remove (factory->priv->container, PICMAN_OBJECT (data));
}

void
picman_data_factory_data_free (PicmanDataFactory *factory)
{
  g_return_if_fail (PICMAN_IS_DATA_FACTORY (factory));

  picman_container_freeze (factory->priv->container);

  picman_data_factory_data_foreach (factory, TRUE,
                                  picman_data_factory_remove_cb, NULL);

  picman_container_thaw (factory->priv->container);
}

PicmanData *
picman_data_factory_data_new (PicmanDataFactory *factory,
                            PicmanContext     *context,
                            const gchar     *name)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (*name != '\0', NULL);

  if (factory->priv->data_new_func)
    {
      PicmanData *data = factory->priv->data_new_func (context, name);

      if (data)
        {
          picman_container_add (factory->priv->container, PICMAN_OBJECT (data));
          g_object_unref (data);

          return data;
        }

      g_warning ("%s: factory->priv->data_new_func() returned NULL", G_STRFUNC);
    }

  return NULL;
}

PicmanData *
picman_data_factory_data_duplicate (PicmanDataFactory *factory,
                                  PicmanData        *data)
{
  PicmanData *new_data;

  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);

  new_data = picman_data_duplicate (data);

  if (new_data)
    {
      const gchar *name = picman_object_get_name (data);
      gchar       *ext;
      gint         copy_len;
      gint         number;
      gchar       *new_name;

      ext      = strrchr (name, '#');
      copy_len = strlen (_("copy"));

      if ((strlen (name) >= copy_len                                 &&
           strcmp (&name[strlen (name) - copy_len], _("copy")) == 0) ||
          (ext && (number = atoi (ext + 1)) > 0                      &&
           ((gint) (log10 (number) + 1)) == strlen (ext + 1)))
        {
          /* don't have redundant "copy"s */
          new_name = g_strdup (name);
        }
      else
        {
          new_name = g_strdup_printf (_("%s copy"), name);
        }

      picman_object_take_name (PICMAN_OBJECT (new_data), new_name);

      picman_container_add (factory->priv->container, PICMAN_OBJECT (new_data));
      g_object_unref (new_data);
    }

  return new_data;
}

gboolean
picman_data_factory_data_delete (PicmanDataFactory  *factory,
                               PicmanData         *data,
                               gboolean          delete_from_disk,
                               GError          **error)
{
  gboolean retval = TRUE;

  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), FALSE);
  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_container_have (factory->priv->container, PICMAN_OBJECT (data)))
    {
      g_object_ref (data);

      picman_container_remove (factory->priv->container, PICMAN_OBJECT (data));

      if (delete_from_disk && picman_data_get_filename (data))
        retval = picman_data_delete_from_disk (data, error);

      g_object_unref (data);
    }

  return retval;
}

PicmanData *
picman_data_factory_data_get_standard (PicmanDataFactory *factory,
                                     PicmanContext     *context)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  if (factory->priv->data_get_standard_func)
    return factory->priv->data_get_standard_func (context);

  return NULL;
}

gboolean
picman_data_factory_data_save_single (PicmanDataFactory  *factory,
                                    PicmanData         *data,
                                    GError          **error)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), FALSE);
  g_return_val_if_fail (PICMAN_IS_DATA (data), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! picman_data_is_dirty (data))
    return TRUE;

  if (! picman_data_get_filename (data))
    {
      gchar  *writable_dir;
      GError *my_error = NULL;

      writable_dir = picman_data_factory_get_save_dir (factory, &my_error);

      if (! writable_dir)
        {
          g_set_error (error, PICMAN_DATA_ERROR, 0,
                       _("Failed to save data:\n\n%s"),
                       my_error->message);
          g_clear_error (&my_error);

          return FALSE;
        }

      picman_data_create_filename (data, writable_dir);

      g_free (writable_dir);
    }

  if (! picman_data_is_writable (data))
    return FALSE;

  if (! picman_data_save (data, error))
    {
      /*  check if there actually was an error (no error
       *  means the data class does not implement save)
       */
      if (! error)
        g_set_error (error, PICMAN_DATA_ERROR, 0,
                     _("Failed to save data:\n\n%s"),
                     "Data class does not implement saving");

      return FALSE;
    }

  return TRUE;
}

GType
picman_data_factory_get_data_type (PicmanDataFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), G_TYPE_NONE);

  return picman_container_get_children_type (factory->priv->container);
}

PicmanContainer *
picman_data_factory_get_container (PicmanDataFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);

  return factory->priv->container;
}

PicmanContainer *
picman_data_factory_get_container_obsolete (PicmanDataFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);

  return factory->priv->container_obsolete;
}

Picman *
picman_data_factory_get_picman (PicmanDataFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);

  return factory->priv->picman;
}

gboolean
picman_data_factory_has_data_new_func (PicmanDataFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), FALSE);

  return factory->priv->data_new_func != NULL;
}


/*  private functions  */

static gchar *
picman_data_factory_get_save_dir (PicmanDataFactory  *factory,
                                GError          **error)
{
  gchar *path;
  gchar *writable_path;
  gchar *tmp;
  GList *path_list;
  GList *writable_list;
  GList *list;
  gchar *writable_dir = NULL;

  g_object_get (factory->priv->picman->config,
                factory->priv->path_property_name,     &path,
                factory->priv->writable_property_name, &writable_path,
                NULL);

  tmp = picman_config_path_expand (path, TRUE, NULL);
  g_free (path);
  path = tmp;

  tmp = picman_config_path_expand (writable_path, TRUE, NULL);
  g_free (writable_path);
  writable_path = tmp;

  path_list     = picman_path_parse (path,          256, FALSE, NULL);
  writable_list = picman_path_parse (writable_path, 256, FALSE, NULL);

  g_free (path);
  g_free (writable_path);

  if (writable_path)
    {
      gboolean found_any = FALSE;

      for (list = writable_list; list; list = g_list_next (list))
        {
          GList *found = g_list_find_custom (path_list,
                                             list->data, (GCompareFunc) strcmp);
          if (found)
            {
              const gchar *dir = found->data;

              found_any = TRUE;

              if (! g_file_test (dir, G_FILE_TEST_IS_DIR))
                {
                  /*  error out only if this is the last chance  */
                  if (! list->next)
                    {
                      gchar *display_name = g_filename_display_name (dir);

                      g_set_error (error, PICMAN_DATA_ERROR, 0,
                                   _("You have a writable data folder "
                                     "configured (%s), but this folder does "
                                     "not exist. Please create the folder or "
                                     "fix your configuation in the "
                                     "Preferences dialog's 'Folders' section."),
                                   display_name);

                      g_free (display_name);
                    }
                }
              else
                {
                  writable_dir = g_strdup (dir);
                  break;
                }
            }
        }

      if (! writable_dir && ! found_any)
        {
          g_set_error (error, PICMAN_DATA_ERROR, 0,
                       _("You have a writable data folder configured, but this "
                         "folder is not part of your data search path. You "
                         "probably edited the picmanrc file manually, "
                         "please fix it in the Preferences dialog's 'Folders' "
                         "section."));
        }
    }
  else
    {
      g_set_error (error, PICMAN_DATA_ERROR, 0,
                   _("You don't have any writable data folder configured."));
    }

  picman_path_free (path_list);
  picman_path_free (writable_list);

  return writable_dir;
}

static gboolean
picman_data_factory_is_dir_writable (const gchar *dirname,
                                   GList       *writable_path)
{
  GList *list;

  for (list = writable_path; list; list = g_list_next (list))
    {
      if (g_str_has_prefix (dirname, list->data))
        return TRUE;
    }

  return FALSE;
}

static void
picman_data_factory_load_data_recursive (const PicmanDatafileData *file_data,
                                       gpointer                data)
{
  PicmanDataLoadContext *context = data;
  gboolean             top_set = FALSE;

  /*  When processing subdirectories, set the top_directory if it's
   *  unset. This way me make sure picman_data_set_folder_tags()'
   *  calling convention is honored: pass NULL when processing the
   *  toplevel directory itself, and pass the toplevel directory when
   *  processing any folder inside.
   */
  if (! context->top_directory)
    {
      context->top_directory = file_data->dirname;
      top_set = TRUE;
    }

  picman_datafiles_read_directories (file_data->filename, G_FILE_TEST_IS_REGULAR,
                                   picman_data_factory_load_data, context);

  picman_datafiles_read_directories (file_data->filename, G_FILE_TEST_IS_DIR,
                                   picman_data_factory_load_data_recursive,
                                   context);

  /*  Unset, the string is only valid within this function, and will
   *  be set again for the next subdirectory.
   */
  if (top_set)
    context->top_directory = NULL;
}

static void
picman_data_factory_load_data (const PicmanDatafileData *file_data,
                             gpointer                data)
{
  PicmanDataLoadContext              *context = data;
  PicmanDataFactory                  *factory = context->factory;
  GHashTable                       *cache   = context->cache;
  const PicmanDataFactoryLoaderEntry *loader  = NULL;
  GError                           *error   = NULL;
  GList                            *data_list;
  gint                              i;

  for (i = 0; i < factory->priv->n_loader_entries; i++)
    {
      loader = &factory->priv->loader_entries[i];

      /* a loder matches if its extension matches, or if it doesn't
       * have an extension, which is the case for the fallback loader,
       * which must be last in the loader array
       */
      if (! loader->extension ||
          picman_datafiles_check_extension (file_data->filename,
                                          loader->extension))
        goto insert;
    }

  return;

 insert:
  if (cache)
    {
      GList *cached_data;

      cached_data = g_hash_table_lookup (cache, file_data->filename);

      if (cached_data &&
          picman_data_get_mtime (cached_data->data) != 0 &&
          picman_data_get_mtime (cached_data->data) == file_data->mtime)
        {
          GList *list;

          for (list = cached_data; list; list = g_list_next (list))
            picman_container_add (factory->priv->container, list->data);

          return;
        }
    }

  data_list = loader->load_func (context->context, file_data->filename, &error);

  if (G_LIKELY (data_list))
    {
      GList    *list;
      gboolean  obsolete;
      gboolean  writable  = FALSE;
      gboolean  deletable = FALSE;

      obsolete = (strstr (file_data->dirname,
                          PICMAN_OBSOLETE_DATA_DIR_NAME) != 0);

      /* obsolete files are immutable, don't check their writability */
      if (! obsolete)
        {
          GList *writable_list;

          writable_list = g_object_get_data (G_OBJECT (factory),
                                             WRITABLE_PATH_KEY);

          deletable = (g_list_length (data_list) == 1 &&
                       picman_data_factory_is_dir_writable (file_data->dirname,
                                                          writable_list));

          writable = (deletable && loader->writable);
        }

      for (list = data_list; list; list = g_list_next (list))
        {
          PicmanData *data = list->data;

          picman_data_set_filename (data, file_data->filename,
                                  writable, deletable);
          picman_data_set_mtime (data, file_data->mtime);

          picman_data_clean (data);

          if (obsolete)
            {
              picman_container_add (factory->priv->container_obsolete,
                                  PICMAN_OBJECT (data));
            }
          else
            {
              picman_data_set_folder_tags (data, context->top_directory);

              picman_container_add (factory->priv->container,
                                  PICMAN_OBJECT (data));
            }

          g_object_unref (data);
        }

      g_list_free (data_list);
    }

  if (G_UNLIKELY (error))
    {
      picman_message (factory->priv->picman, NULL, PICMAN_MESSAGE_ERROR,
                    _("Failed to load data:\n\n%s"), error->message);
      g_clear_error (&error);
    }
}
