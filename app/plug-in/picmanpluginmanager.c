/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
 *
 * picmanpluginmanager.c
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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "plug-in-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanmarshal.h"

#include "pdb/picmanpdb.h"

#include "picmanenvirontable.h"
#include "picmaninterpreterdb.h"
#include "picmanplugin.h"
#include "picmanplugindebug.h"
#include "picmanplugindef.h"
#include "picmanpluginmanager.h"
#include "picmanpluginmanager-data.h"
#include "picmanpluginmanager-help-domain.h"
#include "picmanpluginmanager-history.h"
#include "picmanpluginmanager-locale-domain.h"
#include "picmanpluginmanager-menu-branch.h"
#include "picmanpluginshm.h"
#include "picmantemporaryprocedure.h"

#include "picman-intl.h"


enum
{
  PLUG_IN_OPENED,
  PLUG_IN_CLOSED,
  MENU_BRANCH_ADDED,
  HISTORY_CHANGED,
  LAST_SIGNAL
};


static void     picman_plug_in_manager_dispose     (GObject    *object);
static void     picman_plug_in_manager_finalize    (GObject    *object);

static gint64   picman_plug_in_manager_get_memsize (PicmanObject *object,
                                                  gint64     *gui_size);


G_DEFINE_TYPE (PicmanPlugInManager, picman_plug_in_manager, PICMAN_TYPE_OBJECT)

#define parent_class picman_plug_in_manager_parent_class

static guint manager_signals[LAST_SIGNAL] = { 0, };


static void
picman_plug_in_manager_class_init (PicmanPlugInManagerClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  manager_signals[PLUG_IN_OPENED] =
    g_signal_new ("plug-in-opened",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanPlugInManagerClass,
                                   plug_in_opened),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PLUG_IN);

  manager_signals[PLUG_IN_CLOSED] =
    g_signal_new ("plug-in-closed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanPlugInManagerClass,
                                   plug_in_closed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_PLUG_IN);

  manager_signals[MENU_BRANCH_ADDED] =
    g_signal_new ("menu-branch-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanPlugInManagerClass,
                                   menu_branch_added),
                  NULL, NULL,
                  picman_marshal_VOID__STRING_STRING_STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING,
                  G_TYPE_STRING,
                  G_TYPE_STRING);

  manager_signals[HISTORY_CHANGED] =
    g_signal_new ("history-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanPlugInManagerClass,
                                   history_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose          = picman_plug_in_manager_dispose;
  object_class->finalize         = picman_plug_in_manager_finalize;

  picman_object_class->get_memsize = picman_plug_in_manager_get_memsize;
}

static void
picman_plug_in_manager_init (PicmanPlugInManager *manager)
{
  manager->picman               = NULL;

  manager->plug_in_defs       = NULL;
  manager->write_pluginrc     = FALSE;

  manager->plug_in_procedures = NULL;
  manager->load_procs         = NULL;
  manager->save_procs         = NULL;
  manager->export_procs       = NULL;

  manager->current_plug_in    = NULL;
  manager->open_plug_ins      = NULL;
  manager->plug_in_stack      = NULL;
  manager->history            = NULL;

  manager->shm                = NULL;
  manager->interpreter_db     = picman_interpreter_db_new ();
  manager->environ_table      = picman_environ_table_new ();
  manager->debug              = NULL;
  manager->data_list          = NULL;
}

static void
picman_plug_in_manager_dispose (GObject *object)
{
  PicmanPlugInManager *manager = PICMAN_PLUG_IN_MANAGER (object);

  picman_plug_in_manager_history_clear (manager);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_plug_in_manager_finalize (GObject *object)
{
  PicmanPlugInManager *manager = PICMAN_PLUG_IN_MANAGER (object);

  if (manager->load_procs)
    {
      g_slist_free (manager->load_procs);
      manager->load_procs = NULL;
    }

  if (manager->save_procs)
    {
      g_slist_free (manager->save_procs);
      manager->save_procs = NULL;
    }

  if (manager->export_procs)
    {
      g_slist_free (manager->export_procs);
      manager->export_procs = NULL;
    }

  if (manager->plug_in_procedures)
    {
      g_slist_free_full (manager->plug_in_procedures,
                         (GDestroyNotify) g_object_unref);
      manager->plug_in_procedures = NULL;
    }

  if (manager->plug_in_defs)
    {
      g_slist_free_full (manager->plug_in_defs,
                         (GDestroyNotify) g_object_unref);
      manager->plug_in_defs = NULL;
    }

  if (manager->environ_table)
    {
      g_object_unref (manager->environ_table);
      manager->environ_table = NULL;
    }

  if (manager->interpreter_db)
    {
      g_object_unref (manager->interpreter_db);
      manager->interpreter_db = NULL;
    }

  if (manager->debug)
    {
      picman_plug_in_debug_free (manager->debug);
      manager->debug = NULL;
    }

  picman_plug_in_manager_menu_branch_exit (manager);
  picman_plug_in_manager_locale_domain_exit (manager);
  picman_plug_in_manager_help_domain_exit (manager);
  picman_plug_in_manager_data_free (manager);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_plug_in_manager_get_memsize (PicmanObject *object,
                                  gint64     *gui_size)
{
  PicmanPlugInManager *manager = PICMAN_PLUG_IN_MANAGER (object);
  gint64             memsize = 0;

  memsize += picman_g_slist_get_memsize_foreach (manager->plug_in_defs,
                                               (PicmanMemsizeFunc)
                                               picman_object_get_memsize,
                                               gui_size);

  memsize += picman_g_slist_get_memsize (manager->plug_in_procedures, 0);
  memsize += picman_g_slist_get_memsize (manager->load_procs, 0);
  memsize += picman_g_slist_get_memsize (manager->save_procs, 0);
  memsize += picman_g_slist_get_memsize (manager->export_procs, 0);

  memsize += picman_g_slist_get_memsize (manager->menu_branches,  0 /* FIXME */);
  memsize += picman_g_slist_get_memsize (manager->locale_domains, 0 /* FIXME */);
  memsize += picman_g_slist_get_memsize (manager->help_domains,   0 /* FIXME */);

  memsize += picman_g_slist_get_memsize_foreach (manager->open_plug_ins,
                                               (PicmanMemsizeFunc)
                                               picman_object_get_memsize,
                                               gui_size);
  memsize += picman_g_slist_get_memsize (manager->plug_in_stack, 0);
  memsize += picman_g_slist_get_memsize (manager->history,       0);

  memsize += 0; /* FIXME manager->shm */
  memsize += picman_object_get_memsize (PICMAN_OBJECT (manager->interpreter_db),
                                      gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (manager->environ_table),
                                      gui_size);
  memsize += 0; /* FIXME manager->plug_in_debug */
  memsize += picman_g_list_get_memsize (manager->data_list, 0 /* FIXME */);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

PicmanPlugInManager *
picman_plug_in_manager_new (Picman *picman)
{
  PicmanPlugInManager *manager;

  manager = g_object_new (PICMAN_TYPE_PLUG_IN_MANAGER, NULL);

  manager->picman = picman;

  return manager;
}

void
picman_plug_in_manager_initialize (PicmanPlugInManager  *manager,
                                 PicmanInitStatusFunc  status_callback)
{
  gchar *path;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (status_callback != NULL);

  status_callback (NULL, _("Plug-In Interpreters"), 0.8);

  path = picman_config_path_expand (manager->picman->config->interpreter_path,
                                  TRUE, NULL);
  picman_interpreter_db_load (manager->interpreter_db, path);
  g_free (path);

  status_callback (NULL, _("Plug-In Environment"), 0.9);

  path = picman_config_path_expand (manager->picman->config->environ_path,
                                  TRUE, NULL);
  picman_environ_table_load (manager->environ_table, path);
  g_free (path);

  /*  allocate a piece of shared memory for use in transporting tiles
   *  to plug-ins. if we can't allocate a piece of shared memory then
   *  we'll fall back on sending the data over the pipe.
   */
  if (manager->picman->use_shm)
    manager->shm = picman_plug_in_shm_new ();

  manager->debug = picman_plug_in_debug_new ();
}

void
picman_plug_in_manager_exit (PicmanPlugInManager *manager)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));

  while (manager->open_plug_ins)
    picman_plug_in_close (manager->open_plug_ins->data, TRUE);

  /*  need to deatch from shared memory, we can't rely on exit()
   *  cleaning up behind us (see bug #609026)
   */
  if (manager->shm)
    {
      picman_plug_in_shm_free (manager->shm);
      manager->shm = NULL;
    }
}

void
picman_plug_in_manager_add_procedure (PicmanPlugInManager   *manager,
                                    PicmanPlugInProcedure *procedure)
{
  GSList *list;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN_PROCEDURE (procedure));

  for (list = manager->plug_in_procedures; list; list = list->next)
    {
      PicmanPlugInProcedure *tmp_proc = list->data;

      if (strcmp (picman_object_get_name (procedure),
                  picman_object_get_name (tmp_proc)) == 0)
        {
          GSList *list2;

          list->data = g_object_ref (procedure);

          g_printerr ("Removing duplicate PDB procedure '%s' "
                      "registered by '%s'\n",
                      picman_object_get_name (tmp_proc),
                      picman_filename_to_utf8 (tmp_proc->prog));

          /* search the plugin list to see if any plugins had references to
           * the tmp_proc.
           */
          for (list2 = manager->plug_in_defs; list2; list2 = list2->next)
            {
              PicmanPlugInDef *plug_in_def = list2->data;

              if (g_slist_find (plug_in_def->procedures, tmp_proc))
                picman_plug_in_def_remove_procedure (plug_in_def, tmp_proc);
            }

          /* also remove it from the lists of load, save and export procs */
          manager->load_procs   = g_slist_remove (manager->load_procs,   tmp_proc);
          manager->save_procs   = g_slist_remove (manager->save_procs,   tmp_proc);
          manager->export_procs = g_slist_remove (manager->export_procs, tmp_proc);

          /* and from the history */
          picman_plug_in_manager_history_remove (manager, tmp_proc);

          g_object_unref (tmp_proc);

          return;
        }
    }

  manager->plug_in_procedures = g_slist_prepend (manager->plug_in_procedures,
                                                 g_object_ref (procedure));
}

void
picman_plug_in_manager_add_temp_proc (PicmanPlugInManager      *manager,
                                    PicmanTemporaryProcedure *procedure)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (procedure));

  picman_pdb_register_procedure (manager->picman->pdb, PICMAN_PROCEDURE (procedure));

  manager->plug_in_procedures = g_slist_prepend (manager->plug_in_procedures,
                                                 g_object_ref (procedure));
}

void
picman_plug_in_manager_remove_temp_proc (PicmanPlugInManager      *manager,
                                       PicmanTemporaryProcedure *procedure)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_TEMPORARY_PROCEDURE (procedure));

  manager->plug_in_procedures = g_slist_remove (manager->plug_in_procedures,
                                                procedure);

  picman_plug_in_manager_history_remove (manager,
                                       PICMAN_PLUG_IN_PROCEDURE (procedure));

  picman_pdb_unregister_procedure (manager->picman->pdb,
                                 PICMAN_PROCEDURE (procedure));

  g_object_unref (procedure);
}

void
picman_plug_in_manager_add_open_plug_in (PicmanPlugInManager *manager,
                                       PicmanPlugIn        *plug_in)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  manager->open_plug_ins = g_slist_prepend (manager->open_plug_ins,
                                            g_object_ref (plug_in));

  g_signal_emit (manager, manager_signals[PLUG_IN_OPENED], 0,
                 plug_in);
}

void
picman_plug_in_manager_remove_open_plug_in (PicmanPlugInManager *manager,
                                          PicmanPlugIn        *plug_in)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  manager->open_plug_ins = g_slist_remove (manager->open_plug_ins, plug_in);

  g_signal_emit (manager, manager_signals[PLUG_IN_CLOSED], 0,
                 plug_in);

  g_object_unref (plug_in);
}

void
picman_plug_in_manager_plug_in_push (PicmanPlugInManager *manager,
                                   PicmanPlugIn        *plug_in)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_PLUG_IN (plug_in));

  manager->current_plug_in = plug_in;

  manager->plug_in_stack = g_slist_prepend (manager->plug_in_stack,
                                            manager->current_plug_in);
}

void
picman_plug_in_manager_plug_in_pop (PicmanPlugInManager *manager)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));

  if (manager->current_plug_in)
    manager->plug_in_stack = g_slist_remove (manager->plug_in_stack,
                                             manager->plug_in_stack->data);

  if (manager->plug_in_stack)
    manager->current_plug_in = manager->plug_in_stack->data;
  else
    manager->current_plug_in = NULL;
}

void
picman_plug_in_manager_history_changed (PicmanPlugInManager *manager)
{
  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));

  g_signal_emit (manager, manager_signals[HISTORY_CHANGED], 0);
}
