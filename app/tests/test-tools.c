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
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools/tools-types.h"

#include "tools/picmanrectangleoptions.h"
#include "tools/tool_manager.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-callbacks.h"
#include "display/picmandisplayshell-scale.h"
#include "display/picmandisplayshell-tool-events.h"
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

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlayer.h"
#include "core/picmantoolinfo.h"
#include "core/picmantooloptions.h"

#include "tests.h"

#include "picman-app-test-utils.h"


#define PICMAN_TEST_IMAGE_WIDTH            150
#define PICMAN_TEST_IMAGE_HEIGHT           267

/* Put this in the code below when you want the test to pause so you
 * can do measurements of widgets on the screen for example
 */
#define PICMAN_PAUSE (g_usleep (2 * 1000 * 1000))

#define ADD_TEST(function) \
  g_test_add ("/picman-tools/" #function, \
              PicmanTestFixture, \
              picman, \
              picman_tools_setup_image, \
              function, \
              picman_tools_teardown_image);


typedef struct
{
  int avoid_sizeof_zero;
} PicmanTestFixture;


static void               picman_tools_setup_image                         (PicmanTestFixture  *fixture,
                                                                          gconstpointer     data);
static void               picman_tools_teardown_image                      (PicmanTestFixture  *fixture,
                                                                          gconstpointer     data);
static void               picman_tools_synthesize_image_click_drag_release (PicmanDisplayShell *shell,
                                                                          gdouble           start_image_x,
                                                                          gdouble           start_image_y,
                                                                          gdouble           end_image_x,
                                                                          gdouble           end_image_y,
                                                                          gint              button,
                                                                          GdkModifierType   modifiers);
static PicmanDisplay      * picman_test_get_only_display                     (Picman             *picman);
static PicmanImage        * picman_test_get_only_image                       (Picman             *picman);
static PicmanDisplayShell * picman_test_get_only_display_shell               (Picman             *picman);


static void
picman_tools_setup_image (PicmanTestFixture *fixture,
                        gconstpointer    data)
{
  Picman *picman = PICMAN (data);

  picman_test_utils_create_image (picman, 
                                PICMAN_TEST_IMAGE_WIDTH,
                                PICMAN_TEST_IMAGE_HEIGHT);
  picman_test_run_mainloop_until_idle ();
}

static void
picman_tools_teardown_image (PicmanTestFixture *fixture,
                           gconstpointer    data)
{
  Picman *picman = PICMAN (data);

  g_object_unref (picman_test_get_only_image (picman));
  picman_display_close (picman_test_get_only_display (picman));
  picman_test_run_mainloop_until_idle ();
}

/**
 * picman_tools_set_tool:
 * @picman:
 * @tool_id:
 * @display:
 *
 * Makes sure the given tool is the active tool and that the passed
 * display is the focused tool display.
 **/
static void
picman_tools_set_tool (Picman        *picman,
                     const gchar *tool_id,
                     PicmanDisplay *display)
{
  /* Activate tool and setup active display for the new tool */
  picman_context_set_tool (picman_get_user_context (picman),
                         picman_get_tool_info (picman, tool_id));
  tool_manager_focus_display_active (picman, display);
}

/**
 * picman_test_get_only_display:
 * @picman:
 *
 * Asserts that there only is one image and display and then
 * returns the display.
 *
 * Returns: The #PicmanDisplay.
 **/
static PicmanDisplay *
picman_test_get_only_display (Picman *picman)
{
  g_assert (g_list_length (picman_get_image_iter (picman)) == 1);
  g_assert (g_list_length (picman_get_display_iter (picman)) == 1);

  return PICMAN_DISPLAY (picman_get_display_iter (picman)->data);
}

/**
 * picman_test_get_only_display_shell:
 * @picman:
 *
 * Asserts that there only is one image and display shell and then
 * returns the display shell.
 *
 * Returns: The #PicmanDisplayShell.
 **/
static PicmanDisplayShell *
picman_test_get_only_display_shell (Picman *picman)
{
  return picman_display_get_shell (picman_test_get_only_display (picman));
}

/**
 * picman_test_get_only_image:
 * @picman:
 *
 * Asserts that there is only one image and returns that.
 *
 * Returns: The #PicmanImage.
 **/
static PicmanImage *
picman_test_get_only_image (Picman *picman)
{
  g_assert (g_list_length (picman_get_image_iter (picman)) == 1);
  g_assert (g_list_length (picman_get_display_iter (picman)) == 1);

  return PICMAN_IMAGE (picman_get_image_iter (picman)->data);
}

static void
picman_test_synthesize_tool_button_event (PicmanDisplayShell *shell,
                                       gint              x,
                                       gint              y,
                                       gint              button,
                                       gint              modifiers,
                                       GdkEventType      button_event_type)
{
  GdkEvent   *event   = gdk_event_new (button_event_type);
  GdkWindow  *window  = gtk_widget_get_window (GTK_WIDGET (shell->canvas));
  GdkDisplay *display = gdk_window_get_display (window);

  g_assert (button_event_type == GDK_BUTTON_PRESS ||
            button_event_type == GDK_BUTTON_RELEASE);

  event->button.window     = g_object_ref (window);
  event->button.send_event = TRUE;
  event->button.time       = gtk_get_current_event_time ();
  event->button.x          = x;
  event->button.y          = y;
  event->button.axes       = NULL;
  event->button.state      = 0;
  event->button.button     = button;
  event->button.device     = gdk_display_get_core_pointer (display);
  event->button.x_root     = -1;
  event->button.y_root     = -1;

  picman_display_shell_canvas_tool_events (shell->canvas,
                                         event,
                                         shell);
  gdk_event_free (event);
}

static void
picman_test_synthesize_tool_motion_event (PicmanDisplayShell *shell,
                                        gint              x,
                                        gint              y,
                                        gint              modifiers)
{
  GdkEvent   *event   = gdk_event_new (GDK_MOTION_NOTIFY);
  GdkWindow  *window  = gtk_widget_get_window (GTK_WIDGET (shell->canvas));
  GdkDisplay *display = gdk_window_get_display (window);

  event->motion.window     = g_object_ref (window);
  event->motion.send_event = TRUE;
  event->motion.time       = gtk_get_current_event_time ();
  event->motion.x          = x;
  event->motion.y          = y;
  event->motion.axes       = NULL;
  event->motion.state      = GDK_BUTTON1_MASK | modifiers;
  event->motion.is_hint    = FALSE;
  event->motion.device     = gdk_display_get_core_pointer (display);
  event->motion.x_root     = -1;
  event->motion.y_root     = -1;

  picman_display_shell_canvas_tool_events (shell->canvas,
                                         event,
                                         shell);
  gdk_event_free (event);
}

static void
picman_test_synthesize_tool_crossing_event (PicmanDisplayShell *shell,
                                          gint              x,
                                          gint              y,
                                          gint              modifiers,
                                          GdkEventType      crossing_event_type)
{
  GdkEvent   *event   = gdk_event_new (crossing_event_type);
  GdkWindow  *window  = gtk_widget_get_window (GTK_WIDGET (shell->canvas));

  g_assert (crossing_event_type == GDK_ENTER_NOTIFY ||
            crossing_event_type == GDK_LEAVE_NOTIFY);

  event->crossing.window     = g_object_ref (window);
  event->crossing.send_event = TRUE;
  event->crossing.subwindow  = NULL;
  event->crossing.time       = gtk_get_current_event_time ();
  event->crossing.x          = x;
  event->crossing.y          = y;
  event->crossing.x_root     = -1;
  event->crossing.y_root     = -1;
  event->crossing.mode       = GDK_CROSSING_NORMAL;
  event->crossing.detail     = GDK_NOTIFY_UNKNOWN;
  event->crossing.focus      = TRUE;
  event->crossing.state      = modifiers;

  picman_display_shell_canvas_tool_events (shell->canvas,
                                         event,
                                         shell);
  gdk_event_free (event);
}

static void
picman_tools_synthesize_image_click_drag_release (PicmanDisplayShell *shell,
                                                gdouble           start_image_x,
                                                gdouble           start_image_y,
                                                gdouble           end_image_x,
                                                gdouble           end_image_y,
                                                gint              button /*1..3*/,
                                                GdkModifierType   modifiers)
{
  gdouble start_canvas_x  = -1.0;
  gdouble start_canvas_y  = -1.0;
  gdouble middle_canvas_x = -1.0;
  gdouble middle_canvas_y = -1.0;
  gdouble end_canvas_x    = -1.0;
  gdouble end_canvas_y    = -1.0;

  /* Transform coordinates */
  picman_display_shell_transform_xy_f (shell,
                                     start_image_x,
                                     start_image_y,
                                     &start_canvas_x,
                                     &start_canvas_y);
  picman_display_shell_transform_xy_f (shell,
                                     end_image_x,
                                     end_image_y,
                                     &end_canvas_x,
                                     &end_canvas_y);
  middle_canvas_x = (start_canvas_x + end_canvas_x) / 2;
  middle_canvas_y = (start_canvas_y + end_canvas_y) / 2;

  /* Enter notify */
  picman_test_synthesize_tool_crossing_event (shell,
                                            (int)start_canvas_x,
                                            (int)start_canvas_y,
                                            modifiers,
                                            GDK_ENTER_NOTIFY);

  /* Button press */
  picman_test_synthesize_tool_button_event (shell,
                                          (int)start_canvas_x,
                                          (int)start_canvas_y,
                                          button,
                                          modifiers,
                                          GDK_BUTTON_PRESS);

  /* Move events */
  picman_test_synthesize_tool_motion_event (shell,
                                          (int)start_canvas_x,
                                          (int)start_canvas_y,
                                          modifiers);
  picman_test_synthesize_tool_motion_event (shell,
                                          (int)middle_canvas_x,
                                          (int)middle_canvas_y,
                                          modifiers);
  picman_test_synthesize_tool_motion_event (shell,
                                          (int)end_canvas_x,
                                          (int)end_canvas_y,
                                          modifiers);

  /* Button release */
  picman_test_synthesize_tool_button_event (shell,
                                          (int)end_canvas_x,
                                          (int)end_canvas_y,
                                          button,
                                          modifiers,
                                          GDK_BUTTON_RELEASE);

  /* Leave notify */
  picman_test_synthesize_tool_crossing_event (shell,
                                            (int)start_canvas_x,
                                            (int)start_canvas_y,
                                            modifiers,
                                            GDK_LEAVE_NOTIFY);

  /* Process them */
  picman_test_run_mainloop_until_idle ();
}

/**
 * crop_tool_can_crop:
 * @fixture:
 * @data:
 *
 * Make sure it's possible to crop at all. Regression test for
 * "Bug 315255 - SIGSEGV, while doing a crop".
 **/
static void
crop_tool_can_crop (PicmanTestFixture *fixture,
                    gconstpointer    data)
{
  Picman             *picman  = PICMAN (data);
  PicmanImage        *image = picman_test_get_only_image (picman);
  PicmanDisplayShell *shell = picman_test_get_only_display_shell (picman);

  gint cropped_x = 10;
  gint cropped_y = 10;
  gint cropped_w = 20;
  gint cropped_h = 30;

  /* Fit display and pause and let it stabalize (two idlings seems to
   * always be enough)
   */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "view",
                                   "view-shrink-wrap");
  picman_test_run_mainloop_until_idle ();
  picman_test_run_mainloop_until_idle ();

  /* Activate crop tool */
  picman_tools_set_tool (picman, "picman-crop-tool", shell->display);

  /* Do the crop rect */
  picman_tools_synthesize_image_click_drag_release (shell,
                                                  cropped_x,
                                                  cropped_y,
                                                  cropped_x + cropped_w,
                                                  cropped_y + cropped_h,
                                                  1 /*button*/,
                                                  0 /*modifiers*/);

  /* Crop */
  picman_test_utils_synthesize_key_event (GTK_WIDGET (shell), GDK_KEY_Return);
  picman_test_run_mainloop_until_idle ();

  /* Make sure the new image has the expected size */
  g_assert_cmpint (cropped_w, ==, picman_image_get_width (image));
  g_assert_cmpint (cropped_h, ==, picman_image_get_height (image));
}

/**
 * crop_tool_can_crop:
 * @fixture:
 * @data:
 *
 * Make sure it's possible to change width of crop rect in tool
 * options without there being a pending rectangle. Regression test
 * for "Bug 322396 - Crop dimension entering causes crash".
 **/
static void
crop_set_width_without_pending_rect (PicmanTestFixture *fixture,
                                     gconstpointer    data)
{
  Picman                 *picman    = PICMAN (data);
  PicmanDisplay          *display = picman_test_get_only_display (picman);
  PicmanToolInfo         *tool_info;
  PicmanRectangleOptions *rectangle_options;
  GtkWidget            *tool_options_gui;
  GtkWidget            *size_entry;

  /* Activate crop tool */
  picman_tools_set_tool (picman, "picman-crop-tool", display);

  /* Get tool options */
  tool_info         = picman_get_tool_info (picman, "picman-crop-tool");
  tool_options_gui  = picman_tools_get_tool_options_gui (tool_info->tool_options);
  rectangle_options = PICMAN_RECTANGLE_OPTIONS (tool_info->tool_options);

  /* Find 'Width' or 'Height' GtkTextEntry in tool options */
  size_entry = picman_rectangle_options_get_width_entry (rectangle_options);

  /* Set arbitrary non-0 value */
  picman_size_entry_set_value (PICMAN_SIZE_ENTRY (size_entry),
                             0 /*field*/,
                             42.0 /*lower*/);

  /* If we don't crash, everything s fine */
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

  /* Add tests */
  ADD_TEST (crop_tool_can_crop);
  ADD_TEST (crop_set_width_without_pending_rect);

  /* Run the tests and return status */
  result = g_test_run ();

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Exit properly so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
