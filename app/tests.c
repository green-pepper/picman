/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 2009 Martin Nordholts
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "gui/gui-types.h"

#include "gui/gui.h"

#include "actions/actions.h"

#include "menus/menus.h"

#include "widgets/picmansessioninfo.h"

#include "config/picmangeglconfig.h"

#include "core/picman.h"
#include "core/picman-contexts.h"

#include "gegl/picman-gegl.h"

#include "picman-log.h"
#include "tests.h"
#include "units.h"


static void
picman_status_func_dummy (const gchar *text1,
                        const gchar *text2,
                        gdouble      percentage)
{
}

/**
 * picman_init_for_testing:
 *
 * Initialize the PICMAN object system for unit testing. This is a
 * selected subset of the initialization happning in app_run().
 **/
Picman *
picman_init_for_testing (void)
{
  Picman *picman;

  g_type_init();
  picman_log_init ();
  gegl_init (NULL, NULL);

  picman = picman_new ("Unit Tested PICMAN", NULL, NULL, FALSE, TRUE, TRUE, TRUE,
                   FALSE, TRUE, TRUE, FALSE);

  units_init (picman);

  picman_load_config (picman, NULL, NULL);

  picman_gegl_init (picman);
  picman_initialize (picman, picman_status_func_dummy);
  picman_restore (picman, picman_status_func_dummy);

  return picman;
}


#ifndef PICMAN_CONSOLE_COMPILATION

static Picman *
picman_init_for_gui_testing_internal (gboolean     show_gui,
                                    const gchar *picmanrc)
{
  PicmanSessionInfoClass *klass;
  Picman                 *picman;

  /* from main() */
  g_type_init();
  picman_log_init ();
  gegl_init (NULL, NULL);

  /* Introduce an error margin for positions written to sessionrc */
  klass = g_type_class_ref (PICMAN_TYPE_SESSION_INFO);
  picman_session_info_class_set_position_accuracy (klass, 5);

  /* from app_run() */
  picman = picman_new ("Unit Tested PICMAN", NULL, NULL, FALSE, TRUE, TRUE, !show_gui,
                   FALSE, TRUE, TRUE, FALSE);
  picman_set_show_gui (picman, show_gui);
  units_init (picman);
  picman_load_config (picman, picmanrc, NULL);
  picman_gegl_init (picman);
  gui_init (picman, TRUE);
  picman_initialize (picman, picman_status_func_dummy);
  picman_restore (picman, picman_status_func_dummy);

  g_type_class_unref (klass);

  return picman;
}

/**
 * picman_init_for_gui_testing:
 * @show_gui:
 *
 * Initializes a #Picman instance for use in test cases that rely on GUI
 * code to be initialized.
 *
 * Returns: The #Picman instance.
 **/
Picman *
picman_init_for_gui_testing (gboolean show_gui)
{
  return picman_init_for_gui_testing_internal (show_gui, NULL);
}

/**
 * picman_init_for_gui_testing:
 * @show_gui:
 * @picmanrc:
 *
 * Like picman_init_for_gui_testing(), but also allows a custom picmanrc
 * filename to be specified.
 *
 * Returns: The #Picman instance.
 **/
Picman *
picman_init_for_gui_testing_with_rc (gboolean     show_gui,
                                   const gchar *picmanrc)
{
  return picman_init_for_gui_testing_internal (show_gui, picmanrc);
}

#endif /* PICMAN_CONSOLE_COMPILATION */

static gboolean
picman_tests_quit_mainloop (GMainLoop *loop)
{
  g_main_loop_quit (loop);

  return FALSE;
}

/**
 * picman_test_run_temp_mainloop:
 * @running_time: The time to run the main loop.
 *
 * Helper function for tests that wants to run a main loop for a
 * while. Useful when you want PICMAN's state to settle before doing
 * tests.
 **/
void
picman_test_run_temp_mainloop (guint32 running_time)
{
  GMainLoop *loop;
  loop = g_main_loop_new (NULL, FALSE);

  g_timeout_add (running_time,
                 (GSourceFunc) picman_tests_quit_mainloop,
                 loop);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);
}

/**
 * picman_test_run_mainloop_until_idle:
 *
 * Creates and runs a main loop until it is idle, i.e. has no more
 * work to do.
 **/
void
picman_test_run_mainloop_until_idle (void)
{
  GMainLoop *loop = g_main_loop_new (NULL, FALSE);

  g_idle_add ((GSourceFunc) picman_tests_quit_mainloop, loop);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);
}

/**
 * picman_test_bail_if_no_display:
 * @void:
 *
 * If no DISPLAY is set, call exit(EXIT_SUCCESS). There is no use in
 * having UI tests failing in DISPLAY-less environments.
 **/
void
picman_test_bail_if_no_display (void)
{
  if (! g_getenv ("DISPLAY"))
    {
      g_message ("No DISPLAY set, not running UI tests\n");
      exit (EXIT_SUCCESS);
    }
}
