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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "display/display-types.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmanimagewindow.h"

#include "widgets/picmanuimanager.h"
#include "widgets/picmandialogfactory.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanlayer.h"

#include "tests.h"

#include "picman-app-test-utils.h"


void
picman_test_utils_set_env_to_subpath (const gchar *root_env_var,
                                    const gchar *subdir,
                                    const gchar *target_env_var)
{
  const gchar *root_dir   = NULL;
  gchar       *target_dir = NULL;

  /* Get root dir */
  root_dir = g_getenv (root_env_var);
  if (! root_dir)
    g_printerr ("*\n"
                "*  The env var %s is not set, you are probably running\n"
                "*  in a debugger. Set it manually, e.g.:\n"
                "*\n"
                "*    set env %s=%s/source/picman\n"
                "*\n",
                root_env_var,
                root_env_var, g_get_home_dir ());

  /* Construct path and setup target env var */
  target_dir = g_build_filename (root_dir, subdir, NULL);
  g_setenv (target_env_var, target_dir, TRUE);
  g_free (target_dir);
}


/**
 * picman_test_utils_set_picman2_directory:
 * @root_env_var: Either "PICMAN_TESTING_ABS_TOP_SRCDIR" or
 *                "PICMAN_TESTING_ABS_TOP_BUILDDIR"
 * @subdir:       Subdir, may be %NULL
 *
 * Sets PICMAN2_DIRECTORY to the source dir @root_env_var/@subdir. The
 * environment variables is set up by the test runner, see Makefile.am
 **/
void
picman_test_utils_set_picman2_directory (const gchar *root_env_var,
                                     const gchar *subdir)
{
  picman_test_utils_set_env_to_subpath (root_env_var,
                                      subdir,
                                      "PICMAN2_DIRECTORY" /*target_env_var*/);
}

/**
 * picman_test_utils_setup_menus_dir:
 *
 * Sets PICMAN_TESTING_MENUS_DIR to "$top_srcdir/menus".
 **/
void
picman_test_utils_setup_menus_dir (void)
{
  /* PICMAN_TESTING_ABS_TOP_SRCDIR is set by the automake test runner,
   * see Makefile.am
   */
  picman_test_utils_set_env_to_subpath ("PICMAN_TESTING_ABS_TOP_SRCDIR" /*root_env_var*/,
                                      "menus" /*subdir*/,
                                      "PICMAN_TESTING_MENUS_DIR" /*target_env_var*/);
}

/**
 * picman_test_utils_create_image:
 * @picman:   A #Picman instance.
 * @width:  Width of image (and layer)
 * @height: Height of image (and layer)
 *
 * Creates a new image of a given size with one layer of same size and
 * a display.
 *
 * Returns: The new #PicmanImage.
 **/
void
picman_test_utils_create_image (Picman *picman,
                              gint  width,
                              gint  height)
{
  PicmanImage *image;
  PicmanLayer *layer;

  image = picman_image_new (picman, width, height,
                          PICMAN_RGB, PICMAN_PRECISION_U8);

  layer = picman_layer_new (image,
                          width,
                          height,
                          picman_image_get_layer_format (image, TRUE),
                          "layer1",
                          1.0,
                          PICMAN_NORMAL_MODE);

  picman_image_add_layer (image,
                        layer,
                        NULL /*parent*/,
                        0 /*position*/,
                        FALSE /*push_undo*/);

  picman_create_display (picman,
                       image,
                       PICMAN_UNIT_PIXEL,
                       1.0 /*scale*/);
}

/**
 * picman_test_utils_synthesize_key_event:
 * @widget: Widget to target.
 * @keyval: Keyval, e.g. GDK_Return
 *
 * Simulates a keypress and release with gdk_test_simulate_key().
 **/
void
picman_test_utils_synthesize_key_event (GtkWidget *widget,
                                      guint      keyval)
{
  gdk_test_simulate_key (gtk_widget_get_window (widget),
                         -1, -1, /*x, y*/
                         keyval,
                         0 /*modifiers*/,
                         GDK_KEY_PRESS);
  gdk_test_simulate_key (gtk_widget_get_window (widget),
                         -1, -1, /*x, y*/
                         keyval,
                         0 /*modifiers*/,
                         GDK_KEY_RELEASE);
}

/**
 * picman_test_utils_get_ui_manager:
 * @picman: The #Picman instance.
 *
 * Returns the "best" #PicmanUIManager to use when performing
 * actions. It gives the ui manager of the empty display if it exists,
 * otherwise it gives it the ui manager of the first display.
 *
 * Returns: The #PicmanUIManager.
 **/
PicmanUIManager *
picman_test_utils_get_ui_manager (Picman *picman)
{
  PicmanDisplay       *display      = NULL;
  PicmanDisplayShell  *shell        = NULL;
  GtkWidget         *toplevel     = NULL;
  PicmanImageWindow   *image_window = NULL;
  PicmanUIManager     *ui_manager   = NULL;

  display = PICMAN_DISPLAY (picman_get_empty_display (picman));

  /* If there were not empty display, assume that there is at least
   * one image display and use that
   */
  if (! display)
    display = PICMAN_DISPLAY (picman_get_display_iter (picman)->data);

  shell            = picman_display_get_shell (display);
  toplevel         = gtk_widget_get_toplevel (GTK_WIDGET (shell));
  image_window     = PICMAN_IMAGE_WINDOW (toplevel);
  ui_manager       = picman_image_window_get_ui_manager (image_window);

  return ui_manager;
}

/**
 * picman_test_utils_create_image_from_dalog:
 * @picman:
 *
 * Creates a new image using the "New image" dialog, and then returns
 * the #PicmanImage created.
 *
 * Returns: The created #PicmanImage.
 **/
PicmanImage *
picman_test_utils_create_image_from_dialog (Picman *picman)
{
  PicmanImage     *image            = NULL;
  GtkWidget     *new_image_dialog = NULL;
  guint          n_initial_images = g_list_length (picman_get_image_iter (picman));
  guint          n_images         = -1;
  gint           tries_left       = 100;
  PicmanUIManager *ui_manager       = picman_test_utils_get_ui_manager (picman);

  /* Bring up the new image dialog */
  picman_ui_manager_activate_action (ui_manager,
                                   "image",
                                   "image-new");
  picman_test_run_mainloop_until_idle ();

  /* Get the GtkWindow of the dialog */
  new_image_dialog =
    picman_dialog_factory_dialog_raise (picman_dialog_factory_get_singleton (),
                                      gdk_screen_get_default (),
                                      "picman-image-new-dialog",
                                      -1 /*view_size*/);

  /* Press the focused widget, it should be the Ok button. It will
   * take a while for the image to be created so loop for a while
   */
  gtk_widget_activate (gtk_window_get_focus (GTK_WINDOW (new_image_dialog)));
  do
    {
      g_usleep (20 * 1000);
      picman_test_run_mainloop_until_idle ();
      n_images = g_list_length (picman_get_image_iter (picman));
    }
  while (tries_left-- &&
         n_images != n_initial_images + 1);

  /* Make sure there now is one image more than initially */
  g_assert_cmpint (n_images,
                   ==,
                   n_initial_images + 1);

  image = PICMAN_IMAGE (picman_get_image_iter (picman)->data);

  return image;
}

