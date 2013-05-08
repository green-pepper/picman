/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 2011 Martin Nordholts
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

#include <gtk/gtk.h>

#include "dialogs/dialogs-types.h"

#include "tests.h"

#include "picman-test-session-utils.h"
#include "picman-app-test-utils.h"


#define ADD_TEST(function) \
  g_test_add_func ("/picman-session-2-8-compatibility-single-window/" #function, \
                   function);


/**
 * Tests that a multi-window sessionrc in PICMAN 2.8 format is loaded
 * and written (thus also interpreted) like we expect.
 **/
static void
read_and_write_session_files (void)
{
  picman_test_session_load_and_write_session_files ("sessionrc-2-8-single-window",
                                                  "dockrc-2-8",
                                                  "sessionrc-expected-single-window",
                                                  "dockrc-expected",
                                                  TRUE /*single_window_mode*/);
}

int main(int argc, char **argv)
{
  picman_test_bail_if_no_display ();
  gtk_test_init (&argc, &argv, NULL);

  ADD_TEST (read_and_write_session_files);

  /* Don't bother freeing stuff, the process is short-lived */
  return g_test_run ();
}
