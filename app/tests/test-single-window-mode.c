/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * test-single-window-mode.c
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs/dialogs-types.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-scale.h"
#include "display/picmandisplayshell-transform.h"
#include "display/picmanimagewindow.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockable.h"
#include "widgets/picmandockbook.h"
#include "widgets/picmandockcontainer.h"
#include "widgets/picmandocked.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmantoolbox.h"
#include "widgets/picmantooloptionseditor.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlayer.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"

#include "tests.h"

#include "picman-app-test-utils.h"


#define ADD_TEST(function) \
  g_test_add_data_func ("/picman-single-window-mode/" #function, picman, function);


/* Put this in the code below when you want the test to pause so you
 * can do measurements of widgets on the screen for example
 */
#define PICMAN_PAUSE (g_usleep (20 * 1000 * 1000))


/**
 * new_dockable_not_in_new_window:
 * @data:
 *
 * Test that in single-window mode, new dockables are not put in new
 * windows (they should end up in the single image window).
 **/
static void
new_dockable_not_in_new_window (gconstpointer data)
{
  Picman              *picman             = PICMAN (data);
  PicmanDialogFactory *factory          = picman_dialog_factory_get_singleton ();
  gint               dialogs_before   = 0;
  gint               toplevels_before = 0;
  gint               dialogs_after    = 0;
  gint               toplevels_after  = 0;
  GList             *dialogs;
  GList             *iter;

  picman_test_run_mainloop_until_idle ();

  /* Count dialogs before we create the dockable */
  dialogs        = picman_dialog_factory_get_open_dialogs (factory);
  dialogs_before = g_list_length (dialogs);
  for (iter = dialogs; iter; iter = g_list_next (iter))
    {
      if (gtk_widget_is_toplevel (iter->data))
        toplevels_before++;
    }

  /* Create a dockable */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "dialogs",
                                   "dialogs-undo-history");
  picman_test_run_mainloop_until_idle ();

  /* Count dialogs after we created the dockable */
  dialogs        = picman_dialog_factory_get_open_dialogs (factory);
  dialogs_after = g_list_length (dialogs);
  for (iter = dialogs; iter; iter = g_list_next (iter))
    {
      if (gtk_widget_is_toplevel (iter->data))
        toplevels_after++;
    }

  /* We got one more session managed dialog ... */
  g_assert_cmpint (dialogs_before + 1, ==, dialogs_after);
  /* ... but no new toplevels */
  g_assert_cmpint (toplevels_before, ==, toplevels_after);
}

int main(int argc, char **argv)
{
  Picman  *picman   = NULL;
  gint   result = -1;

  picman_test_bail_if_no_display ();
  gtk_test_init (&argc, &argv, NULL);

  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_SRCDIR",
                                       "app/tests/picmandir");
  picman_test_utils_setup_menus_dir ();

  /* Launch PICMAN in single-window mode */
  g_setenv ("PICMAN_TESTING_SESSIONRC_NAME", "sessionrc-2-8-single-window", TRUE /*overwrite*/);
  picman = picman_init_for_gui_testing (TRUE /*show_gui*/);
  picman_test_run_mainloop_until_idle ();

  ADD_TEST (new_dockable_not_in_new_window);

  /* Run the tests and return status */
  result = g_test_run ();

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Exit properly so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
