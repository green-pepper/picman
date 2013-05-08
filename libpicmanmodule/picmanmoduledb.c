/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"

#include "picmanmoduletypes.h"

#include "picmanmodule.h"
#include "picmanmoduledb.h"


/**
 * SECTION: picmanmoduledb
 * @title: PicmanModuleDB
 * @short_description: Keeps a list of #PicmanModule's found in a given
 *                     searchpath.
 *
 * Keeps a list of #PicmanModule's found in a given searchpath.
 **/


enum
{
  ADD,
  REMOVE,
  MODULE_MODIFIED,
  LAST_SIGNAL
};


/*  #define DUMP_DB 1  */


static void         picman_module_db_finalize            (GObject      *object);

static void         picman_module_db_module_initialize   (const PicmanDatafileData *file_data,
                                                        gpointer                user_data);

static PicmanModule * picman_module_db_module_find_by_path (PicmanModuleDB *db,
                                                        const char   *fullpath);

#ifdef DUMP_DB
static void         picman_module_db_dump_module         (gpointer      data,
                                                        gpointer      user_data);
#endif

static void         picman_module_db_module_on_disk_func (gpointer      data,
                                                        gpointer      user_data);
static void         picman_module_db_module_remove_func  (gpointer      data,
                                                        gpointer      user_data);
static void         picman_module_db_module_modified     (PicmanModule   *module,
                                                        PicmanModuleDB *db);


G_DEFINE_TYPE (PicmanModuleDB, picman_module_db, G_TYPE_OBJECT)

#define parent_class picman_module_db_parent_class

static guint db_signals[LAST_SIGNAL] = { 0 };


static void
picman_module_db_class_init (PicmanModuleDBClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  db_signals[ADD] =
    g_signal_new ("add",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanModuleDBClass, add),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_MODULE);

  db_signals[REMOVE] =
    g_signal_new ("remove",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanModuleDBClass, remove),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_MODULE);

  db_signals[MODULE_MODIFIED] =
    g_signal_new ("module-modified",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanModuleDBClass, module_modified),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_MODULE);

  object_class->finalize = picman_module_db_finalize;

  klass->add             = NULL;
  klass->remove          = NULL;
}

static void
picman_module_db_init (PicmanModuleDB *db)
{
  db->modules      = NULL;
  db->load_inhibit = NULL;
  db->verbose      = FALSE;
}

static void
picman_module_db_finalize (GObject *object)
{
  PicmanModuleDB *db = PICMAN_MODULE_DB (object);

  if (db->modules)
    {
      g_list_free (db->modules);
      db->modules = NULL;
    }

  if (db->load_inhibit)
    {
      g_free (db->load_inhibit);
      db->load_inhibit = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * picman_module_db_new:
 * @verbose: Pass %TRUE to enable debugging output.
 *
 * Creates a new #PicmanModuleDB instance. The @verbose parameter will be
 * passed to the created #PicmanModule instances using picman_module_new().
 *
 * Return value: The new #PicmanModuleDB instance.
 **/
PicmanModuleDB *
picman_module_db_new (gboolean verbose)
{
  PicmanModuleDB *db;

  db = g_object_new (PICMAN_TYPE_MODULE_DB, NULL);

  db->verbose = verbose ? TRUE : FALSE;

  return db;
}

static gboolean
is_in_inhibit_list (const gchar *filename,
                    const gchar *inhibit_list)
{
  gchar       *p;
  gint         pathlen;
  const gchar *start;
  const gchar *end;

  if (! inhibit_list || ! strlen (inhibit_list))
    return FALSE;

  p = strstr (inhibit_list, filename);
  if (!p)
    return FALSE;

  /* we have a substring, but check for colons either side */
  start = p;
  while (start != inhibit_list && *start != G_SEARCHPATH_SEPARATOR)
    start--;

  if (*start == G_SEARCHPATH_SEPARATOR)
    start++;

  end = strchr (p, G_SEARCHPATH_SEPARATOR);
  if (! end)
    end = inhibit_list + strlen (inhibit_list);

  pathlen = strlen (filename);

  if ((end - start) == pathlen)
    return TRUE;

  return FALSE;
}

/**
 * picman_module_db_set_load_inhibit:
 * @db:           A #PicmanModuleDB.
 * @load_inhibit: A #G_SEARCHPATH_SEPARATOR delimited list of module
 *                filenames to exclude from auto-loading.
 *
 * Sets the @load_inhibit flag for all #PicmanModule's which are kept
 * by @db (using picman_module_set_load_inhibit()).
 **/
void
picman_module_db_set_load_inhibit (PicmanModuleDB *db,
                                 const gchar  *load_inhibit)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_MODULE_DB (db));

  if (db->load_inhibit)
    g_free (db->load_inhibit);

  db->load_inhibit = g_strdup (load_inhibit);

  for (list = db->modules; list; list = g_list_next (list))
    {
      PicmanModule *module = list->data;

      picman_module_set_load_inhibit (module,
                                    is_in_inhibit_list (module->filename,
                                                        load_inhibit));
    }
}

/**
 * picman_module_db_get_load_inhibit:
 * @db: A #PicmanModuleDB.
 *
 * Return the #G_SEARCHPATH_SEPARATOR delimited list of module filenames
 * which are excluded from auto-loading.
 *
 * Return value: the @db's @load_inhibit string.
 **/
const gchar *
picman_module_db_get_load_inhibit (PicmanModuleDB *db)
{
  g_return_val_if_fail (PICMAN_IS_MODULE_DB (db), NULL);

  return db->load_inhibit;
}

/**
 * picman_module_db_load:
 * @db:          A #PicmanModuleDB.
 * @module_path: A #G_SEARCHPATH_SEPARATOR delimited list of directories
 *               to load modules from.
 *
 * Scans the directories contained in @module_path using
 * picman_datafiles_read_directories() and creates a #PicmanModule
 * instance for every loadable module contained in the directories.
 **/
void
picman_module_db_load (PicmanModuleDB *db,
                     const gchar  *module_path)
{
  g_return_if_fail (PICMAN_IS_MODULE_DB (db));
  g_return_if_fail (module_path != NULL);

  if (g_module_supported ())
    picman_datafiles_read_directories (module_path,
                                     G_FILE_TEST_EXISTS,
                                     picman_module_db_module_initialize,
                                     db);

#ifdef DUMP_DB
  g_list_foreach (db->modules, picman_module_db_dump_module, NULL);
#endif
}

/**
 * picman_module_db_refresh:
 * @db:          A #PicmanModuleDB.
 * @module_path: A #G_SEARCHPATH_SEPARATOR delimited list of directories
 *               to load modules from.
 *
 * Does the same as picman_module_db_load(), plus removes all #PicmanModule
 * instances whose modules have been deleted from disk.
 *
 * Note that the #PicmanModule's will just be removed from the internal
 * list and not freed as this is not possible with #GTypeModule
 * instances which actually implement types.
 **/
void
picman_module_db_refresh (PicmanModuleDB *db,
                        const gchar  *module_path)
{
  GList *kill_list = NULL;

  g_return_if_fail (PICMAN_IS_MODULE_DB (db));
  g_return_if_fail (module_path != NULL);

  /* remove modules we don't have on disk anymore */
  g_list_foreach (db->modules,
                  picman_module_db_module_on_disk_func,
                  &kill_list);
  g_list_foreach (kill_list,
                  picman_module_db_module_remove_func,
                  db);
  g_list_free (kill_list);

  /* walk filesystem and add new things we find */
  picman_datafiles_read_directories (module_path,
                                   G_FILE_TEST_EXISTS,
                                   picman_module_db_module_initialize,
                                   db);
}

static void
picman_module_db_module_initialize (const PicmanDatafileData *file_data,
                                  gpointer                user_data)
{
  PicmanModuleDB *db = PICMAN_MODULE_DB (user_data);
  PicmanModule   *module;
  gboolean      load_inhibit;

  if (! picman_datafiles_check_extension (file_data->filename,
                                        "." G_MODULE_SUFFIX))
    return;

  /* don't load if we already know about it */
  if (picman_module_db_module_find_by_path (db, file_data->filename))
    return;

  load_inhibit = is_in_inhibit_list (file_data->filename,
                                     db->load_inhibit);

  module = picman_module_new (file_data->filename,
                            load_inhibit,
                            db->verbose);

  g_signal_connect (module, "modified",
                    G_CALLBACK (picman_module_db_module_modified),
                    db);

  db->modules = g_list_append (db->modules, module);
  g_signal_emit (db, db_signals[ADD], 0, module);
}

static PicmanModule *
picman_module_db_module_find_by_path (PicmanModuleDB *db,
                                    const char   *fullpath)
{
  GList *list;

  for (list = db->modules; list; list = g_list_next (list))
    {
      PicmanModule *module = list->data;

      if (! strcmp (module->filename, fullpath))
        return module;
    }

  return NULL;
}

#ifdef DUMP_DB
static void
picman_module_db_dump_module (gpointer data,
                            gpointer user_data)
{
  PicmanModule *module = data;

  g_print ("\n%s: %s\n",
           picman_filename_to_utf8 (module->filename),
           picman_module_state_name (module->state));

  g_print ("  module: %p  lasterr: %s  query: %p register: %p\n",
           module->module,
           module->last_module_error ? module->last_module_error : "NONE",
           module->query_module,
           module->register_module);

  if (i->info)
    {
      g_print ("  purpose:   %s\n"
               "  author:    %s\n"
               "  version:   %s\n"
               "  copyright: %s\n"
               "  date:      %s\n",
               module->info->purpose   ? module->info->purpose   : "NONE",
               module->info->author    ? module->info->author    : "NONE",
               module->info->version   ? module->info->version   : "NONE",
               module->info->copyright ? module->info->copyright : "NONE",
               module->info->date      ? module->info->date      : "NONE");
    }
}
#endif

static void
picman_module_db_module_on_disk_func (gpointer data,
                                    gpointer user_data)
{
  PicmanModule  *module    = data;
  GList      **kill_list = user_data;
  gboolean     old_on_disk;

  old_on_disk = module->on_disk;

  module->on_disk = g_file_test (module->filename, G_FILE_TEST_IS_REGULAR);

  /* if it's not on the disk, and it isn't in memory, mark it to be
   * removed later.
   */
  if (! module->on_disk && ! module->module)
    {
      *kill_list = g_list_append (*kill_list, module);
      module = NULL;
    }

  if (module && module->on_disk != old_on_disk)
    picman_module_modified (module);
}

static void
picman_module_db_module_remove_func (gpointer data,
                                   gpointer user_data)
{
  PicmanModule   *module = data;
  PicmanModuleDB *db     = user_data;

  g_signal_handlers_disconnect_by_func (module,
                                        picman_module_db_module_modified,
                                        db);

  db->modules = g_list_remove (db->modules, module);

  g_signal_emit (db, db_signals[REMOVE], 0, module);
}

static void
picman_module_db_module_modified (PicmanModule   *module,
                                PicmanModuleDB *db)
{
  g_signal_emit (db, db_signals[MODULE_MODIFIED], 0, module);
}
