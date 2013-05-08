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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>

#ifdef G_OS_WIN32
#include <windows.h>
#include <winnls.h>
#endif

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core/core-types.h"

#include "config/picmanrc.h"

#include "gegl/picman-gegl.h"

#include "core/picman.h"
#include "core/picman-user-install.h"

#include "file/file-open.h"

#ifndef PICMAN_CONSOLE_COMPILATION
#include "dialogs/user-install-dialog.h"

#include "gui/gui.h"
#endif

#include "app.h"
#include "batch.h"
#include "errors.h"
#include "units.h"
#include "language.h"
#include "picman-debug.h"

#include "picman-intl.h"


/*  local prototypes  */

static void       app_init_update_noop    (const gchar *text1,
                                           const gchar *text2,
                                           gdouble      percentage);
static gboolean   app_exit_after_callback (Picman        *picman,
                                           gboolean     kill_it,
                                           GMainLoop   *loop);


/*  public functions  */

void
app_libs_init (GOptionContext *context,
               gboolean        no_interface)
{
  g_type_init ();

  g_option_context_add_group (context, gegl_get_option_group ());

#ifndef PICMAN_CONSOLE_COMPILATION
  if (! no_interface)
    {
      gui_libs_init (context);
    }
#endif
}

void
app_abort (gboolean     no_interface,
           const gchar *abort_message)
{
#ifndef PICMAN_CONSOLE_COMPILATION
  if (no_interface)
#endif
    {
      g_print ("%s\n\n", abort_message);
    }
#ifndef PICMAN_CONSOLE_COMPILATION
  else
    {
      gui_abort (abort_message);
    }
#endif

  app_exit (EXIT_FAILURE);
}

void
app_exit (gint status)
{
  exit (status);
}

void
app_run (const gchar         *full_prog_name,
         const gchar        **filenames,
         const gchar         *alternate_system_picmanrc,
         const gchar         *alternate_picmanrc,
         const gchar         *session_name,
         const gchar         *batch_interpreter,
         const gchar        **batch_commands,
         gboolean             as_new,
         gboolean             no_interface,
         gboolean             no_data,
         gboolean             no_fonts,
         gboolean             no_splash,
         gboolean             be_verbose,
         gboolean             use_shm,
         gboolean             use_cpu_accel,
         gboolean             console_messages,
         gboolean             use_debug_handler,
         PicmanStackTraceMode   stack_trace_mode,
         PicmanPDBCompatMode    pdb_compat_mode)
{
  PicmanInitStatusFunc  update_status_func = NULL;
  Picman               *picman;
  GMainLoop          *loop;
  gchar              *default_folder = NULL;

  if (filenames && filenames[0] && ! filenames[1] &&
      g_file_test (filenames[0], G_FILE_TEST_IS_DIR))
    {
      if (g_path_is_absolute (filenames[0]))
        {
          default_folder = g_filename_to_uri (filenames[0], NULL, NULL);
        }
      else
        {
          gchar *absolute = g_build_path (G_DIR_SEPARATOR_S,
                                          g_get_current_dir (),
                                          filenames[0],
                                          NULL);
          default_folder = g_filename_to_uri (absolute, NULL, NULL);
          g_free (absolute);
        }

      filenames = NULL;
    }

  /*  Create an instance of the "Picman" object which is the root of the
   *  core object system
   */
  picman = picman_new (full_prog_name,
                   session_name,
                   default_folder,
                   be_verbose,
                   no_data,
                   no_fonts,
                   no_interface,
                   use_shm,
                   console_messages,
                   stack_trace_mode,
                   pdb_compat_mode);

  errors_init (picman, full_prog_name, use_debug_handler, stack_trace_mode);

  units_init (picman);

  /*  Check if the user's picman_directory exists
   */
  if (! g_file_test (picman_directory (), G_FILE_TEST_IS_DIR))
    {
      PicmanUserInstall *install = picman_user_install_new (be_verbose);

#ifdef PICMAN_CONSOLE_COMPILATION
      picman_user_install_run (install);
#else
      if (! (no_interface ?
	     picman_user_install_run (install) :
	     user_install_dialog_run (install)))
	exit (EXIT_FAILURE);
#endif

      picman_user_install_free (install);
    }

  picman_load_config (picman, alternate_system_picmanrc, alternate_picmanrc);

  /*  change the locale if a language if specified  */
  language_init (picman->config->language);

  /*  initialize lowlevel stuff  */
  picman_gegl_init (picman);

#ifndef PICMAN_CONSOLE_COMPILATION
  if (! no_interface)
    update_status_func = gui_init (picman, no_splash);
#endif

  if (! update_status_func)
    update_status_func = app_init_update_noop;

  /*  Create all members of the global Picman instance which need an already
   *  parsed picmanrc, e.g. the data factories
   */
  picman_initialize (picman, update_status_func);

  /*  Load all data files
   */
  picman_restore (picman, update_status_func);

  /*  enable autosave late so we don't autosave when the
   *  monitor resolution is set in gui_init()
   */
  picman_rc_set_autosave (PICMAN_RC (picman->edit_config), TRUE);

  /*  Load the images given on the command-line.
   */
  if (filenames)
    {
      gint i;

      for (i = 0; filenames[i] != NULL; i++)
        file_open_from_command_line (picman, filenames[i], as_new);
    }

  batch_run (picman, batch_interpreter, batch_commands);

  loop = g_main_loop_new (NULL, FALSE);

  g_signal_connect_after (picman, "exit",
                          G_CALLBACK (app_exit_after_callback),
                          loop);

  picman_threads_leave (picman);
  g_main_loop_run (loop);
  picman_threads_enter (picman);

  g_main_loop_unref (loop);

  g_object_unref (picman);

  picman_debug_instances ();

  errors_exit ();
  gegl_exit ();
}


/*  private functions  */

static void
app_init_update_noop (const gchar *text1,
                      const gchar *text2,
                      gdouble      percentage)
{
  /*  deliberately do nothing  */
}

static gboolean
app_exit_after_callback (Picman      *picman,
                         gboolean   kill_it,
                         GMainLoop *loop)
{
  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  /*
   *  In stable releases, we simply call exit() here. This speeds up
   *  the process of quitting PICMAN and also works around the problem
   *  that plug-ins might still be running.
   *
   *  In unstable releases, we shut down PICMAN properly in an attempt
   *  to catch possible problems in our finalizers.
   */

#ifdef PICMAN_UNSTABLE

  g_main_loop_quit (loop);

#else

  gegl_exit ();

  exit (EXIT_SUCCESS);

#endif

  return FALSE;
}
