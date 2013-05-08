/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2002 Spencer Kimball, Peter Mattis, and others
 *
 * picmanpluginmanager-restore.c
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

#include "pdb/picmanpdb.h"
#include "pdb/picmanpdbcontext.h"

#include "picmaninterpreterdb.h"
#include "picmanplugindef.h"
#include "picmanpluginmanager.h"
#define __YES_I_NEED_PICMAN_PLUG_IN_MANAGER_CALL__
#include "picmanpluginmanager-call.h"
#include "picmanpluginmanager-help-domain.h"
#include "picmanpluginmanager-locale-domain.h"
#include "picmanpluginmanager-restore.h"
#include "picmanpluginprocedure.h"
#include "plug-in-rc.h"

#include "picman-intl.h"


static void    picman_plug_in_manager_search            (PicmanPlugInManager      *manager,
                                                       PicmanInitStatusFunc      status_callback);
static gchar * picman_plug_in_manager_get_pluginrc      (PicmanPlugInManager      *manager);
static void    picman_plug_in_manager_read_pluginrc     (PicmanPlugInManager      *manager,
                                                       const gchar            *pluginrc,
                                                       PicmanInitStatusFunc      status_callback);
static void    picman_plug_in_manager_query_new         (PicmanPlugInManager      *manager,
                                                       PicmanContext            *context,
                                                       PicmanInitStatusFunc      status_callback);
static void    picman_plug_in_manager_init_plug_ins     (PicmanPlugInManager      *manager,
                                                       PicmanContext            *context,
                                                       PicmanInitStatusFunc      status_callback);
static void    picman_plug_in_manager_run_extensions    (PicmanPlugInManager      *manager,
                                                       PicmanContext            *context,
                                                       PicmanInitStatusFunc      status_callback);
static void    picman_plug_in_manager_bind_text_domains (PicmanPlugInManager      *manager);
static void    picman_plug_in_manager_add_from_file     (const PicmanDatafileData *file_data,
                                                       gpointer                data);
static void    picman_plug_in_manager_add_from_rc       (PicmanPlugInManager      *manager,
                                                       PicmanPlugInDef          *plug_in_def);
static void     picman_plug_in_manager_add_to_db         (PicmanPlugInManager      *manager,
                                                        PicmanContext            *context,
                                                        PicmanPlugInProcedure    *proc);
static gint     picman_plug_in_manager_file_proc_compare (gconstpointer           a,
                                                        gconstpointer           b,
                                                        gpointer                data);



void
picman_plug_in_manager_restore (PicmanPlugInManager  *manager,
                              PicmanContext        *context,
                              PicmanInitStatusFunc  status_callback)
{
  Picman   *picman;
  gchar  *pluginrc;
  GSList *list;
  GError *error = NULL;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (status_callback != NULL);

  picman = manager->picman;

  /* need a PicmanPDBContext for calling picman_plug_in_manager_run_foo() */
  context = picman_pdb_context_new (picman, context, TRUE);

  /* search for binaries in the plug-in directory path */
  picman_plug_in_manager_search (manager, status_callback);

  /* read the pluginrc file for cached data */
  pluginrc = picman_plug_in_manager_get_pluginrc (manager);

  picman_plug_in_manager_read_pluginrc (manager, pluginrc, status_callback);

  /* query any plug-ins that changed since we last wrote out pluginrc */
  picman_plug_in_manager_query_new (manager, context, status_callback);

  /* initialize the plug-ins */
  picman_plug_in_manager_init_plug_ins (manager, context, status_callback);

  /* add the procedures to manager->plug_in_procedures */
  for (list = manager->plug_in_defs; list; list = list->next)
    {
      PicmanPlugInDef *plug_in_def = list->data;
      GSList        *list2;

      for (list2 = plug_in_def->procedures; list2; list2 = list2->next)
        {
          picman_plug_in_manager_add_procedure (manager, list2->data);
        }
    }

  /* write the pluginrc file if necessary */
  if (manager->write_pluginrc)
    {
      if (picman->be_verbose)
        g_print ("Writing '%s'\n", picman_filename_to_utf8 (pluginrc));

      if (! plug_in_rc_write (manager->plug_in_defs, pluginrc, &error))
        {
          picman_message_literal (picman,
				NULL, PICMAN_MESSAGE_ERROR, error->message);
          g_clear_error (&error);
        }

      manager->write_pluginrc = FALSE;
    }

  g_free (pluginrc);

  /* create locale and help domain lists */
  for (list = manager->plug_in_defs; list; list = list->next)
    {
      PicmanPlugInDef *plug_in_def = list->data;

      if (plug_in_def->locale_domain_name)
        picman_plug_in_manager_add_locale_domain (manager,
                                                plug_in_def->prog,
                                                plug_in_def->locale_domain_name,
                                                plug_in_def->locale_domain_path);
      else
        /* set the default plug-in locale domain */
        picman_plug_in_def_set_locale_domain (plug_in_def,
                                            picman_plug_in_manager_get_locale_domain (manager,
                                                                                    plug_in_def->prog,
                                                                                    NULL),
                                            NULL);

      if (plug_in_def->help_domain_name)
        picman_plug_in_manager_add_help_domain (manager,
                                              plug_in_def->prog,
                                              plug_in_def->help_domain_name,
                                              plug_in_def->help_domain_uri);
    }

  /* we're done with the plug-in-defs */
  g_slist_free_full (manager->plug_in_defs, (GDestroyNotify) g_object_unref);
  manager->plug_in_defs = NULL;

  /* bind plug-in text domains  */
  picman_plug_in_manager_bind_text_domains (manager);

  /* add the plug-in procs to the procedure database */
  for (list = manager->plug_in_procedures; list; list = list->next)
    {
      picman_plug_in_manager_add_to_db (manager, context, list->data);
    }

  /* sort the load, save and export procedures  */
  manager->load_procs =
    g_slist_sort_with_data (manager->load_procs,
                            picman_plug_in_manager_file_proc_compare, manager);
  manager->save_procs =
    g_slist_sort_with_data (manager->save_procs,
                            picman_plug_in_manager_file_proc_compare, manager);
  manager->export_procs =
    g_slist_sort_with_data (manager->export_procs,
                            picman_plug_in_manager_file_proc_compare, manager);

  picman_plug_in_manager_run_extensions (manager, context, status_callback);

  g_object_unref (context);
}


/* search for binaries in the plug-in directory path */
static void
picman_plug_in_manager_search (PicmanPlugInManager  *manager,
                             PicmanInitStatusFunc  status_callback)
{
  gchar       *path;
  const gchar *pathext = g_getenv ("PATHEXT");

  /*  If PATHEXT is set, we are likely on Windows and need to add
   *  the known file extensions.
   */
  if (pathext)
    {
      gchar *exts;

      exts = picman_interpreter_db_get_extensions (manager->interpreter_db);

      if (exts)
        {
          gchar *value;

          value = g_strconcat (pathext, G_SEARCHPATH_SEPARATOR_S, exts, NULL);

          g_setenv ("PATHEXT", value, TRUE);

          g_free (value);
          g_free (exts);
        }
    }

  status_callback (_("Searching Plug-Ins"), "", 0.0);

  /* Give automatic tests a chance to use plug-ins from the build
   * dir
   */
  path = g_strdup(g_getenv("PICMAN_TESTING_PLUGINDIRS"));

  if (! path) 
    path = picman_config_path_expand (manager->picman->config->plug_in_path,
                                    TRUE, NULL);

  picman_datafiles_read_directories (path,
                                   G_FILE_TEST_IS_EXECUTABLE,
                                   picman_plug_in_manager_add_from_file,
                                   manager);

  g_free (path);
}

static gchar *
picman_plug_in_manager_get_pluginrc (PicmanPlugInManager *manager)
{
  Picman  *picman = manager->picman;
  gchar *pluginrc;

  if (picman->config->plug_in_rc_path)
    {
      pluginrc = picman_config_path_expand (picman->config->plug_in_rc_path,
                                          TRUE, NULL);

      if (! g_path_is_absolute (pluginrc))
        {
          gchar *str = g_build_filename (picman_directory (), pluginrc, NULL);

          g_free (pluginrc);
          pluginrc = str;
        }
    }
  else
    {
      pluginrc = picman_personal_rc_file ("pluginrc");
    }

  return pluginrc;
}

/* read the pluginrc file for cached data */
static void
picman_plug_in_manager_read_pluginrc (PicmanPlugInManager  *manager,
                                    const gchar        *pluginrc,
                                    PicmanInitStatusFunc  status_callback)
{
  GSList *rc_defs;
  GError *error = NULL;

  status_callback (_("Resource configuration"),
                   picman_filename_to_utf8 (pluginrc), 0.0);

  if (manager->picman->be_verbose)
    g_print ("Parsing '%s'\n", picman_filename_to_utf8 (pluginrc));

  rc_defs = plug_in_rc_parse (manager->picman, pluginrc, &error);

  if (rc_defs)
    {
      GSList *list;

      for (list = rc_defs; list; list = g_slist_next (list))
        picman_plug_in_manager_add_from_rc (manager, list->data); /* consumes list->data */

      g_slist_free (rc_defs);
    }
  else if (error)
    {
      if (error->code != PICMAN_CONFIG_ERROR_OPEN_ENOENT)
        picman_message_literal (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
			      error->message);

      g_clear_error (&error);
    }
}

/* query any plug-ins that changed since we last wrote out pluginrc */
static void
picman_plug_in_manager_query_new (PicmanPlugInManager  *manager,
                                PicmanContext        *context,
                                PicmanInitStatusFunc  status_callback)
{
  GSList *list;
  gint    n_plugins;

  status_callback (_("Querying new Plug-ins"), "", 0.0);

  for (list = manager->plug_in_defs, n_plugins = 0; list; list = list->next)
    {
      PicmanPlugInDef *plug_in_def = list->data;

      if (plug_in_def->needs_query)
        n_plugins++;
    }

  if (n_plugins)
    {
      gint nth;

      manager->write_pluginrc = TRUE;

      for (list = manager->plug_in_defs, nth = 0; list; list = list->next)
        {
          PicmanPlugInDef *plug_in_def = list->data;

          if (plug_in_def->needs_query)
            {
              gchar *basename;

              basename = g_filename_display_basename (plug_in_def->prog);
              status_callback (NULL, basename,
                               (gdouble) nth++ / (gdouble) n_plugins);
              g_free (basename);

              if (manager->picman->be_verbose)
                g_print ("Querying plug-in: '%s'\n",
                         picman_filename_to_utf8 (plug_in_def->prog));

              picman_plug_in_manager_call_query (manager, context, plug_in_def);
            }
        }
    }

  status_callback (NULL, "", 1.0);
}

/* initialize the plug-ins */
static void
picman_plug_in_manager_init_plug_ins (PicmanPlugInManager  *manager,
                                    PicmanContext        *context,
                                    PicmanInitStatusFunc  status_callback)
{
  GSList *list;
  gint    n_plugins;

  status_callback (_("Initializing Plug-ins"), "", 0.0);

  for (list = manager->plug_in_defs, n_plugins = 0; list; list = list->next)
    {
      PicmanPlugInDef *plug_in_def = list->data;

      if (plug_in_def->has_init)
        n_plugins++;
    }

  if (n_plugins)
    {
      gint nth;

      for (list = manager->plug_in_defs, nth = 0; list; list = list->next)
        {
          PicmanPlugInDef *plug_in_def = list->data;

          if (plug_in_def->has_init)
            {
              gchar *basename;

              basename = g_filename_display_basename (plug_in_def->prog);
              status_callback (NULL, basename,
                               (gdouble) nth++ / (gdouble) n_plugins);
              g_free (basename);

              if (manager->picman->be_verbose)
                g_print ("Initializing plug-in: '%s'\n",
                         picman_filename_to_utf8 (plug_in_def->prog));

              picman_plug_in_manager_call_init (manager, context, plug_in_def);
            }
        }
    }

  status_callback (NULL, "", 1.0);
}

/* run automatically started extensions */
static void
picman_plug_in_manager_run_extensions (PicmanPlugInManager  *manager,
                                     PicmanContext        *context,
                                     PicmanInitStatusFunc  status_callback)
{
  Picman   *picman = manager->picman;
  GSList *list;
  GList  *extensions = NULL;
  gint    n_extensions;

  /* build list of automatically started extensions */
  for (list = manager->plug_in_procedures; list; list = list->next)
    {
      PicmanPlugInProcedure *proc = list->data;

      if (proc->prog                                         &&
          PICMAN_PROCEDURE (proc)->proc_type == PICMAN_EXTENSION &&
          PICMAN_PROCEDURE (proc)->num_args  == 0)
        {
          extensions = g_list_prepend (extensions, proc);
        }
    }

  extensions   = g_list_reverse (extensions);
  n_extensions = g_list_length (extensions);

  /* run the available extensions */
  if (extensions)
    {
      GList *list;
      gint   nth;

      status_callback (_("Starting Extensions"), "", 0.0);

      for (list = extensions, nth = 0; list; list = g_list_next (list), nth++)
        {
          PicmanPlugInProcedure *proc = list->data;
          PicmanValueArray      *args;
          GError              *error = NULL;

          if (picman->be_verbose)
            g_print ("Starting extension: '%s'\n", picman_object_get_name (proc));

          status_callback (NULL, picman_object_get_name (proc),
                           (gdouble) nth / (gdouble) n_extensions);

          args = picman_value_array_new (0);

          picman_procedure_execute_async (PICMAN_PROCEDURE (proc),
                                        picman, context, NULL,
                                        args, NULL, &error);

          picman_value_array_unref (args);

          if (error)
            {
              picman_message_literal (picman, NULL, PICMAN_MESSAGE_ERROR,
				    error->message);
              g_clear_error (&error);
            }
        }

      g_list_free (extensions);

      status_callback (NULL, "", 1.0);
    }
}

static void
picman_plug_in_manager_bind_text_domains (PicmanPlugInManager *manager)
{
  gchar **locale_domains;
  gchar **locale_paths;
  gint    n_domains;
  gint    i;

  n_domains = picman_plug_in_manager_get_locale_domains (manager,
                                                       &locale_domains,
                                                       &locale_paths);

  for (i = 0; i < n_domains; i++)
    {
      bindtextdomain (locale_domains[i], locale_paths[i]);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
      bind_textdomain_codeset (locale_domains[i], "UTF-8");
#endif
    }

  g_strfreev (locale_domains);
  g_strfreev (locale_paths);
}

/**
 * picman_plug_in_manager_ignore_plugin_basename:
 * @basename: Basename to test with
 *
 * Checks the environment variable
 * PICMAN_TESTING_PLUGINDIRS_BASENAME_IGNORES for file basenames.
 *
 * Returns: %TRUE if @basename was in PICMAN_TESTING_PLUGINDIRS_BASENAME_IGNORES
 **/
static gboolean
picman_plug_in_manager_ignore_plugin_basename (const gchar *plugin_basename)
{
  const gchar *ignore_basenames_string;
  GList       *ignore_basenames;
  GList       *iter;
  gboolean     ignore = FALSE;

  ignore_basenames_string = g_getenv("PICMAN_TESTING_PLUGINDIRS_BASENAME_IGNORES");
  ignore_basenames        = picman_path_parse (ignore_basenames_string,
                                             256 /*max_paths*/,
                                             FALSE /*check*/,
                                             NULL /*check_failed*/);

  for (iter = ignore_basenames; iter; iter = g_list_next (iter))
    {
      const gchar *ignore_basename = iter->data;

      if (g_ascii_strcasecmp (ignore_basename, plugin_basename) == 0)
        {
          ignore = TRUE;
          break;
        }
    }
  
  picman_path_free (ignore_basenames);

  return ignore;
}

static void
picman_plug_in_manager_add_from_file (const PicmanDatafileData *file_data,
                                    gpointer                data)
{
  PicmanPlugInManager *manager = data;
  PicmanPlugInDef     *plug_in_def;
  GSList            *list;

  /* When we scan build dirs for plug-ins, there will be some
   * executable files that are not plug-ins that we want to ignore,
   * for example plug-ins/common/mkgen.pl if
   * PICMAN_TESTING_PLUGINDIRS=plug-ins/common
   */
  if (picman_plug_in_manager_ignore_plugin_basename (file_data->basename))
      return;

  for (list = manager->plug_in_defs; list; list = list->next)
    {
      gchar *plug_in_name;

      plug_in_def  = list->data;
      plug_in_name = g_path_get_basename (plug_in_def->prog);

      if (g_ascii_strcasecmp (file_data->basename, plug_in_name) == 0)
        {
          g_printerr ("Skipping duplicate plug-in: '%s'\n",
                      picman_filename_to_utf8 (file_data->filename));

          g_free (plug_in_name);

          return;
        }

      g_free (plug_in_name);
    }

  plug_in_def = picman_plug_in_def_new (file_data->filename);

  picman_plug_in_def_set_mtime (plug_in_def, file_data->mtime);
  picman_plug_in_def_set_needs_query (plug_in_def, TRUE);

  manager->plug_in_defs = g_slist_prepend (manager->plug_in_defs, plug_in_def);
}

static void
picman_plug_in_manager_add_from_rc (PicmanPlugInManager *manager,
                                  PicmanPlugInDef     *plug_in_def)
{
  GSList *list;
  gchar  *basename1;

  g_return_if_fail (PICMAN_IS_PLUG_IN_MANAGER (manager));
  g_return_if_fail (plug_in_def != NULL);
  g_return_if_fail (plug_in_def->prog != NULL);

  if (! g_path_is_absolute (plug_in_def->prog))
    {
      g_warning ("plug_ins_def_add_from_rc: filename not absolute (skipping)");
      g_object_unref (plug_in_def);
      return;
    }

  basename1 = g_path_get_basename (plug_in_def->prog);

  /*  If this is a file load or save plugin, make sure we have
   *  something for one of the extensions, prefixes, or magic number.
   *  Other bits of code rely on detecting file plugins by the
   *  presence of one of these things, but the raw plug-in needs to be
   *  able to register no extensions, prefixes or magics.
   */
  for (list = plug_in_def->procedures; list; list = list->next)
    {
      PicmanPlugInProcedure *proc = list->data;

      if (! proc->extensions &&
          ! proc->prefixes   &&
          ! proc->magics     &&
          proc->menu_paths   &&
          (g_str_has_prefix (proc->menu_paths->data, "<Load>") ||
           g_str_has_prefix (proc->menu_paths->data, "<Save>")))
        {
          proc->extensions = g_strdup ("");
        }
    }

  /*  Check if the entry mentioned in pluginrc matches an executable
   *  found in the plug_in_path.
   */
  for (list = manager->plug_in_defs; list; list = list->next)
    {
      PicmanPlugInDef *ondisk_plug_in_def = list->data;
      gchar         *basename2;

      basename2 = g_path_get_basename (ondisk_plug_in_def->prog);

      if (! strcmp (basename1, basename2))
        {
          if (! g_ascii_strcasecmp (plug_in_def->prog,
                                    ondisk_plug_in_def->prog) &&
              (plug_in_def->mtime == ondisk_plug_in_def->mtime))
            {
              /* Use pluginrc entry, deleting on-disk entry */
              list->data = plug_in_def;
              g_object_unref (ondisk_plug_in_def);
            }
          else
            {
              /* Use on-disk entry, deleting pluginrc entry */
              g_object_unref (plug_in_def);
            }

          g_free (basename2);
          g_free (basename1);

          return;
        }

      g_free (basename2);
    }

  g_free (basename1);

  manager->write_pluginrc = TRUE;

  if (manager->picman->be_verbose)
    {
      g_printerr ("pluginrc lists '%s', but it wasn't found\n",
                  picman_filename_to_utf8 (plug_in_def->prog));
    }

  g_object_unref (plug_in_def);
}


static void
picman_plug_in_manager_add_to_db (PicmanPlugInManager   *manager,
                                PicmanContext         *context,
                                PicmanPlugInProcedure *proc)
{
  picman_pdb_register_procedure (manager->picman->pdb, PICMAN_PROCEDURE (proc));

  if (proc->file_proc)
    {
      PicmanValueArray *return_vals;
      GError         *error = NULL;

      if (proc->image_types)
        {
          return_vals =
            picman_pdb_execute_procedure_by_name (manager->picman->pdb,
                                                context, NULL, &error,
                                                "picman-register-save-handler",
                                                G_TYPE_STRING, picman_object_get_name (proc),
                                                G_TYPE_STRING, proc->extensions,
                                                G_TYPE_STRING, proc->prefixes,
                                                G_TYPE_NONE);
        }
      else
        {
          return_vals =
            picman_pdb_execute_procedure_by_name (manager->picman->pdb,
                                                context, NULL, &error,
                                                "picman-register-magic-load-handler",
                                                G_TYPE_STRING, picman_object_get_name (proc),
                                                G_TYPE_STRING, proc->extensions,
                                                G_TYPE_STRING, proc->prefixes,
                                                G_TYPE_STRING, proc->magics,
                                                G_TYPE_NONE);
        }

      picman_value_array_unref (return_vals);

      if (error)
        {
          picman_message_literal (manager->picman, NULL, PICMAN_MESSAGE_ERROR,
			        error->message);
          g_error_free (error);
        }
    }
}

static gint
picman_plug_in_manager_file_proc_compare (gconstpointer a,
                                        gconstpointer b,
                                        gpointer      data)
{
  PicmanPlugInProcedure *proc_a = PICMAN_PLUG_IN_PROCEDURE (a);
  PicmanPlugInProcedure *proc_b = PICMAN_PLUG_IN_PROCEDURE (b);
  const gchar         *label_a;
  const gchar         *label_b;
  gint                 retval = 0;

  if (g_str_has_prefix (proc_a->prog, "picman-xcf"))
    return -1;

  if (g_str_has_prefix (proc_b->prog, "picman-xcf"))
    return 1;

  label_a = picman_plug_in_procedure_get_label (proc_a);
  label_b = picman_plug_in_procedure_get_label (proc_b);

  if (label_a && label_b)
    retval = g_utf8_collate (label_a, label_b);

  return retval;
}
