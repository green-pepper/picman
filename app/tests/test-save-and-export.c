/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 2009 Martin Nordholts <martinn@src.gnome.org>
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
#include <glib/gstdio.h>
#include <gtk/gtk.h>

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
#include "widgets/picmandocked.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmantoolbox.h"
#include "widgets/picmantooloptionseditor.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "file/file-open.h"
#include "file/file-procedure.h"
#include "file/file-save.h"
#include "file/file-utils.h"

#include "plug-in/picmanpluginmanager.h"

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
  g_test_add_data_func ("/picman-save-and-export/" #function, picman, function);


typedef gboolean (*PicmanUiTestFunc) (GObject *object);


/**
 * new_file_has_no_uris:
 * @data:
 *
 * Tests that the URIs are correct for a newly created image.
 **/
static void
new_file_has_no_uris (gconstpointer    data)
{
  Picman      *picman  = PICMAN (data);
  PicmanImage *image = picman_test_utils_create_image_from_dialog (picman);

  g_assert (picman_image_get_uri (image) == NULL);
  g_assert (picman_image_get_imported_uri (image) == NULL);
  g_assert (picman_image_get_exported_uri (image) == NULL);
}

/**
 * opened_xcf_file_uris:
 * @data:
 *
 * Tests that PicmanImage URIs are correct for an XCF file that has just
 * been opened.
 **/
static void
opened_xcf_file_uris (gconstpointer data)
{
  Picman              *picman = PICMAN (data);
  PicmanImage         *image;
  gchar             *uri;
  gchar             *filename;
  PicmanPDBStatusType  status;

  filename = g_build_filename (g_getenv ("PICMAN_TESTING_ABS_TOP_SRCDIR"),
                               "app/tests/files/picman-2-6-file.xcf",
                               NULL);
  uri = g_filename_to_uri (filename, NULL, NULL);

  image = file_open_image (picman,
                           picman_get_user_context (picman),
                           NULL /*progress*/,
                           uri,
                           filename,
                           FALSE /*as_new*/,
                           NULL /*file_proc*/,
                           PICMAN_RUN_NONINTERACTIVE,
                           &status,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  g_assert_cmpstr (picman_image_get_uri (image), ==, uri);
  g_assert (picman_image_get_imported_uri (image) == NULL);
  g_assert (picman_image_get_exported_uri (image) == NULL);

  /* Don't bother g_free()ing strings */
}

/**
 * imported_file_uris:
 * @data:
 *
 * Tests that URIs are correct for an imported image.
 **/
static void
imported_file_uris (gconstpointer data)
{
  Picman              *picman = PICMAN (data);
  PicmanImage         *image;
  gchar             *uri;
  gchar             *filename;
  PicmanPDBStatusType  status;

  filename = g_build_filename (g_getenv ("PICMAN_TESTING_ABS_TOP_SRCDIR"),
                               "desktop/64x64/picman.png",
                               NULL);
  g_assert (g_file_test (filename, G_FILE_TEST_EXISTS));

  uri = g_filename_to_uri (filename, NULL, NULL);
  image = file_open_image (picman,
                           picman_get_user_context (picman),
                           NULL /*progress*/,
                           uri,
                           filename,
                           FALSE /*as_new*/,
                           NULL /*file_proc*/,
                           PICMAN_RUN_NONINTERACTIVE,
                           &status,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  g_assert (picman_image_get_uri (image) == NULL);
  g_assert_cmpstr (picman_image_get_imported_uri (image), ==, uri);
  g_assert (picman_image_get_exported_uri (image) == NULL);
}

/**
 * saved_imported_file_uris:
 * @data:
 *
 * Tests that the URIs are correct for an image that has been imported
 * and then saved.
 **/
static void
saved_imported_file_uris (gconstpointer data)
{
  Picman                *picman = PICMAN (data);
  PicmanImage           *image;
  gchar               *import_uri;
  gchar               *import_filename;
  gchar               *save_uri;
  gchar               *save_filename;
  PicmanPDBStatusType    status;
  PicmanPlugInProcedure *proc;

  import_filename = g_build_filename (g_getenv ("PICMAN_TESTING_ABS_TOP_SRCDIR"),
                                      "desktop/64x64/picman.png",
                                      NULL);
  import_uri = g_filename_to_uri (import_filename, NULL, NULL);
  save_filename = g_build_filename (g_get_tmp_dir (), "picman-test.xcf", NULL);
  save_uri = g_filename_to_uri (save_filename, NULL, NULL);

  /* Import */
  image = file_open_image (picman,
                           picman_get_user_context (picman),
                           NULL /*progress*/,
                           import_uri,
                           import_filename,
                           FALSE /*as_new*/,
                           NULL /*file_proc*/,
                           PICMAN_RUN_NONINTERACTIVE,
                           &status,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  /* Save */
  proc = file_procedure_find (image->picman->plug_in_manager->save_procs,
                              save_uri,
                              NULL /*error*/);
  file_save (picman,
             image,
             NULL /*progress*/,
             save_uri,
             proc,
             PICMAN_RUN_NONINTERACTIVE,
             TRUE /*change_saved_state*/,
             FALSE /*export_backward*/,
             FALSE /*export_forward*/,
             NULL /*error*/);

  /* Assert */
  g_assert_cmpstr (picman_image_get_uri (image), ==, save_uri);
  g_assert (picman_image_get_imported_uri (image) == NULL);
  g_assert (picman_image_get_exported_uri (image) == NULL);

  g_unlink (save_filename);
}

/**
 * new_file_has_no_uris:
 * @data:
 *
 * Tests that the URIs for an exported, newly created file are
 * correct.
 **/
static void
exported_file_uris (gconstpointer data)
{
  gchar               *save_uri;
  gchar               *save_filename;
  PicmanPlugInProcedure *proc;
  Picman                *picman  = PICMAN (data);
  PicmanImage           *image = picman_test_utils_create_image_from_dialog (picman);

  save_filename = g_build_filename (g_get_tmp_dir (), "picman-test.png", NULL);
  save_uri = g_filename_to_uri (save_filename, NULL, NULL);

  proc = file_procedure_find (image->picman->plug_in_manager->export_procs,
                              save_uri,
                              NULL /*error*/);
  file_save (picman,
             image,
             NULL /*progress*/,
             save_uri,
             proc,
             PICMAN_RUN_NONINTERACTIVE,
             FALSE /*change_saved_state*/,
             FALSE /*export_backward*/,
             TRUE /*export_forward*/,
             NULL /*error*/);

  g_assert (picman_image_get_uri (image) == NULL);
  g_assert (picman_image_get_imported_uri (image) == NULL);
  g_assert_cmpstr (picman_image_get_exported_uri (image), ==, save_uri);

  g_unlink (save_filename);
}

/**
 * clear_import_uri_after_export:
 * @data:
 *
 * Tests that after a XCF file that was imported has been exported,
 * the import URI is cleared. An image can not be considered both
 * imported and exported at the same time.
 **/
static void
clear_import_uri_after_export (gconstpointer data)
{
  Picman                *picman = PICMAN (data);
  PicmanImage           *image;
  gchar               *uri;
  gchar               *filename;
  gchar               *save_uri;
  gchar               *save_filename;
  PicmanPlugInProcedure *proc;
  PicmanPDBStatusType    status;

  filename = g_build_filename (g_getenv ("PICMAN_TESTING_ABS_TOP_SRCDIR"),
                               "desktop/64x64/picman.png",
                               NULL);
  uri = g_filename_to_uri (filename, NULL, NULL);

  image = file_open_image (picman,
                           picman_get_user_context (picman),
                           NULL /*progress*/,
                           uri,
                           filename,
                           FALSE /*as_new*/,
                           NULL /*file_proc*/,
                           PICMAN_RUN_NONINTERACTIVE,
                           &status,
                           NULL /*mime_type*/,
                           NULL /*error*/);

  g_assert (picman_image_get_uri (image) == NULL);
  g_assert_cmpstr (picman_image_get_imported_uri (image), ==, uri);
  g_assert (picman_image_get_exported_uri (image) == NULL);

  save_filename = g_build_filename (g_get_tmp_dir (), "picman-test.png", NULL);
  save_uri = g_filename_to_uri (save_filename, NULL, NULL);

  proc = file_procedure_find (image->picman->plug_in_manager->export_procs,
                              save_uri,
                              NULL /*error*/);
  file_save (picman,
             image,
             NULL /*progress*/,
             save_uri,
             proc,
             PICMAN_RUN_NONINTERACTIVE,
             FALSE /*change_saved_state*/,
             FALSE /*export_backward*/,
             TRUE /*export_forward*/,
             NULL /*error*/);

  g_assert (picman_image_get_uri (image) == NULL);
  g_assert (picman_image_get_imported_uri (image) == NULL);
  g_assert_cmpstr (picman_image_get_exported_uri (image), ==, save_uri);

  g_unlink (save_filename);
}

int main(int argc, char **argv)
{
  Picman *picman   = NULL;
  gint  result = -1;

  picman_test_bail_if_no_display ();
  gtk_test_init (&argc, &argv, NULL);

  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_SRCDIR",
                                       "app/tests/picmandir");
  picman_test_utils_setup_menus_dir ();

  /* Start up PICMAN */
  picman = picman_init_for_gui_testing (TRUE /*show_gui*/);
  picman_test_run_mainloop_until_idle ();

  ADD_TEST (new_file_has_no_uris);
  ADD_TEST (opened_xcf_file_uris);
  ADD_TEST (imported_file_uris);
  ADD_TEST (saved_imported_file_uris);
  ADD_TEST (exported_file_uris);
  ADD_TEST (clear_import_uri_after_export);

  /* Run the tests and return status */
  result = g_test_run ();

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Exit properly so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
