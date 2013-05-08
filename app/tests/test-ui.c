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
#include "widgets/picmansessioninfo-aux.h"
#include "widgets/picmansessionmanaged.h"
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


#define PICMAN_UI_WINDOW_POSITION_EPSILON 25
#define PICMAN_UI_POSITION_EPSILON        1
#define PICMAN_UI_ZOOM_EPSILON            0.01

#define ADD_TEST(function) \
  g_test_add_data_func ("/picman-ui/" #function, picman, function);


/* Put this in the code below when you want the test to pause so you
 * can do measurements of widgets on the screen for example
 */
#define PICMAN_PAUSE (g_usleep (20 * 1000 * 1000))


typedef gboolean (*PicmanUiTestFunc) (GObject *object);


static void            picman_ui_synthesize_delete_event          (GtkWidget         *widget);
static gboolean        picman_ui_synthesize_click                 (GtkWidget         *widget,
                                                                 gint               x,
                                                                 gint               y,
                                                                 gint               button,
                                                                 GdkModifierType    modifiers);
static GtkWidget     * picman_ui_find_window                      (PicmanDialogFactory *dialog_factory,
                                                                 PicmanUiTestFunc     predicate);
static gboolean        picman_ui_not_toolbox_window               (GObject           *object);
static gboolean        picman_ui_multicolumn_not_toolbox_window   (GObject           *object);
static gboolean        picman_ui_is_picman_layer_list               (GObject           *object);
static int             picman_ui_aux_data_eqiuvalent              (gconstpointer      _a,
                                                                 gconstpointer      _b);
static void            picman_ui_switch_window_mode               (Picman              *picman);


/**
 * tool_options_editor_updates:
 * @data:
 *
 * Makes sure that the tool options editor is updated when the tool
 * changes.
 **/
static void
tool_options_editor_updates (gconstpointer data)
{
  Picman                  *picman         = PICMAN (data);
  PicmanDisplay           *display      = PICMAN_DISPLAY (picman_get_empty_display (picman));
  PicmanDisplayShell      *shell        = picman_display_get_shell (display);
  GtkWidget             *toplevel     = gtk_widget_get_toplevel (GTK_WIDGET (shell));
  PicmanImageWindow       *image_window = PICMAN_IMAGE_WINDOW (toplevel);
  PicmanUIManager         *ui_manager   = picman_image_window_get_ui_manager (image_window);
  GtkWidget             *dockable     = picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                                                        gtk_widget_get_screen (toplevel),
                                                                        NULL /*ui_manager*/,
                                                                        "picman-tool-options",
                                                                        -1 /*view_size*/,
                                                                        FALSE /*present*/);
  PicmanToolOptionsEditor *editor       = PICMAN_TOOL_OPTIONS_EDITOR (gtk_bin_get_child (GTK_BIN (dockable)));

  /* First select the rect select tool */
  picman_ui_manager_activate_action (ui_manager,
                                   "tools",
                                   "tools-rect-select");
  g_assert_cmpstr (PICMAN_HELP_TOOL_RECT_SELECT,
                   ==,
                   picman_tool_options_editor_get_tool_options (editor)->
                   tool_info->
                   help_id);

  /* Change tool and make sure the change is taken into account by the
   * tool options editor
   */
  picman_ui_manager_activate_action (ui_manager,
                                   "tools",
                                   "tools-ellipse-select");
  g_assert_cmpstr (PICMAN_HELP_TOOL_ELLIPSE_SELECT,
                   ==,
                   picman_tool_options_editor_get_tool_options (editor)->
                   tool_info->
                   help_id);
}

static GtkWidget *
picman_ui_get_dialog (const gchar *identifier)
{
  GtkWidget *result = NULL;
  GList     *iter;

  for (iter = picman_dialog_factory_get_open_dialogs (picman_dialog_factory_get_singleton ());
       iter;
       iter = g_list_next (iter))
    {
      GtkWidget *dialog = GTK_WIDGET (iter->data);
      PicmanDialogFactoryEntry *entry = NULL;

      picman_dialog_factory_from_widget (dialog, &entry);

      if (strcmp (entry->identifier, identifier) == 0)
        {
          result = dialog;
          break;
        }
    }

  return result;
}

static void
automatic_tab_style (gconstpointer data)
{
  GtkWidget    *channel_dockable = picman_ui_get_dialog ("picman-channel-list");
  PicmanDockable *dockable;
  PicmanUIManager *ui_manager;
  g_assert (channel_dockable != NULL);

  dockable = PICMAN_DOCKABLE (channel_dockable);

  picman_test_run_mainloop_until_idle ();

  /* The channel dockable is the only dockable, it has enough space
   * for the icon-blurb
   */
  g_assert_cmpint (PICMAN_TAB_STYLE_ICON_BLURB,
                   ==,
                   picman_dockable_get_actual_tab_style (dockable));

  /* Add some dockables to the channel dockable dockbook */
  ui_manager =
    picman_dockbook_get_ui_manager (picman_dockable_get_dockbook (dockable));
  picman_ui_manager_activate_action (ui_manager,
                                   "dockable",
                                   "dialogs-sample-points");
  picman_ui_manager_activate_action (ui_manager,
                                   "dockable",
                                   "dialogs-vectors");
  picman_test_run_mainloop_until_idle ();

  /* Now there is not enough space to have icon-blurb for channel
   * dockable, make sure it's just an icon now
   */
  g_assert_cmpint (PICMAN_TAB_STYLE_ICON,
                   ==,
                   picman_dockable_get_actual_tab_style (dockable));

  /* Close the two dockables we added */
  picman_ui_manager_activate_action (ui_manager,
                                   "dockable",
                                   "dockable-close-tab");
  picman_ui_manager_activate_action (ui_manager,
                                   "dockable",
                                   "dockable-close-tab");
  picman_test_run_mainloop_until_idle ();

  /* We should now be back on icon-blurb */
  g_assert_cmpint (PICMAN_TAB_STYLE_ICON_BLURB,
                   ==,
                   picman_dockable_get_actual_tab_style (dockable));
}

static void
create_new_image_via_dialog (gconstpointer data)
{
  Picman      *picman = PICMAN (data);
  PicmanImage *image;
  PicmanLayer *layer;

  image = picman_test_utils_create_image_from_dialog (picman);

  /* Add a layer to the image to make it more useful in later tests */
  layer = picman_layer_new (image,
                          picman_image_get_width (image),
                          picman_image_get_height (image),
                          picman_image_get_layer_format (image, TRUE),
                          "Layer for testing",
                          PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_image_add_layer (image, layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);
  picman_test_run_mainloop_until_idle ();
}

static void
keyboard_zoom_focus (gconstpointer data)
{
  Picman              *picman    = PICMAN (data);
  PicmanDisplay       *display = PICMAN_DISPLAY (picman_get_display_iter (picman)->data);
  PicmanDisplayShell  *shell   = picman_display_get_shell (display);
  PicmanImageWindow   *window  = picman_display_shell_get_window (shell);
  gint               image_x;
  gint               image_y;
  gint               shell_x_before_zoom;
  gint               shell_y_before_zoom;
  gdouble            factor_before_zoom;
  gint               shell_x_after_zoom;
  gint               shell_y_after_zoom;
  gdouble            factor_after_zoom;

  /* We need to use a point that is within the visible (exposed) part
   * of the canvas
   */
  image_x = 400;
  image_y = 50;

  /* Setup zoom focus on the bottom right part of the image. We avoid
   * 0,0 because that's essentially a particularly easy special case.
   */
  picman_display_shell_transform_xy (shell,
                                   image_x,
                                   image_y,
                                   &shell_x_before_zoom,
                                   &shell_y_before_zoom);
  picman_display_shell_push_zoom_focus_pointer_pos (shell,
                                                  shell_x_before_zoom,
                                                  shell_y_before_zoom);
  factor_before_zoom = picman_zoom_model_get_factor (shell->zoom);

  /* Do the zoom */
  picman_test_utils_synthesize_key_event (GTK_WIDGET (window), GDK_KEY_plus);
  picman_test_run_mainloop_until_idle ();

  /* Make sure the zoom focus point remained fixed */
  picman_display_shell_transform_xy (shell,
                                   image_x,
                                   image_y,
                                   &shell_x_after_zoom,
                                   &shell_y_after_zoom);
  factor_after_zoom = picman_zoom_model_get_factor (shell->zoom);

  /* First of all make sure a zoom happened at all. If this assert
   * fails, it means that the zoom didn't happen. Possible causes:
   *
   *  * gdk_test_simulate_key() failed to map 'GDK_KEY_plus' to the proper
   *    'plus' X keysym, probably because it is mapped to a keycode
   *    with modifiers like 'shift'. Run "xmodmap -pk | grep plus" to
   *    find out. Make sure 'plus' is the first keysym for the given
   *    keycode. If not, use "xmodmap <keycode> = plus" to correct it.
   */
  g_assert_cmpfloat (fabs (factor_before_zoom - factor_after_zoom),
                     >=,
                     PICMAN_UI_ZOOM_EPSILON);

#ifdef __GNUC__
#warning disabled zoom test, it fails randomly, no clue how to fix it
#endif
#if 0
  g_assert_cmpint (ABS (shell_x_after_zoom - shell_x_before_zoom),
                   <=,
                   PICMAN_UI_POSITION_EPSILON);
  g_assert_cmpint (ABS (shell_y_after_zoom - shell_y_before_zoom),
                   <=,
                   PICMAN_UI_POSITION_EPSILON);
#endif
}

/**
 * alt_click_is_layer_to_selection:
 * @data:
 *
 * Makes sure that we can alt-click on a layer to do
 * layer-to-selection. Also makes sure that the layer clicked on is
 * not set as the active layer.
 **/
static void
alt_click_is_layer_to_selection (gconstpointer data)
{
#if __GNUC__
#warning FIXME: please fix alt_click_is_layer_to_selection test
#endif
#if 0
  Picman        *picman      = PICMAN (data);
  PicmanImage   *image     = PICMAN_IMAGE (picman_get_image_iter (picman)->data);
  PicmanChannel *selection = picman_image_get_mask (image);
  PicmanLayer   *active_layer;
  GtkWidget   *dockable;
  GtkWidget   *gtk_tree_view;
  gint         assumed_layer_x;
  gint         assumed_empty_layer_y;
  gint         assumed_background_layer_y;

  /* Hardcode assumptions of where the layers are in the
   * GtkTreeView. Doesn't feel worth adding proper API for this. One
   * can just use PICMAN_PAUSE and re-measure new coordinates if we
   * start to layout layers in the GtkTreeView differently
   */
  assumed_layer_x            = 96;
  assumed_empty_layer_y      = 16;
  assumed_background_layer_y = 42;

  /* Store the active layer, it shall not change during the execution
   * of this test
   */
  active_layer = picman_image_get_active_layer (image);

  /* Find the layer tree view to click in. Note that there is a
   * potential problem with gtk_test_find_widget and GtkNotebook: it
   * will return e.g. a GtkTreeView from another page if that page is
   * "on top" of the reference label.
   */
  dockable = picman_ui_find_window (picman_dialog_factory_get_singleton (),
                                  picman_ui_is_picman_layer_list);
  gtk_tree_view = gtk_test_find_widget (dockable,
                                        "Lock:",
                                        GTK_TYPE_TREE_VIEW);
  
  /* First make sure there is no selection */
  g_assert (! picman_channel_bounds (selection,
                                   NULL, NULL, /*x1, y1*/
                                   NULL, NULL  /*x2, y2*/));

  /* Now simulate alt-click on the background layer */
  g_assert (picman_ui_synthesize_click (gtk_tree_view,
                                      assumed_layer_x,
                                      assumed_background_layer_y,
                                      1 /*button*/,
                                      GDK_MOD1_MASK));
  picman_test_run_mainloop_until_idle ();

  /* Make sure we got a selection and that the active layer didn't
   * change
   */
  g_assert (picman_channel_bounds (selection,
                                 NULL, NULL, /*x1, y1*/
                                 NULL, NULL  /*x2, y2*/));
  g_assert (picman_image_get_active_layer (image) == active_layer);

  /* Now simulate alt-click on the empty layer */
  g_assert (picman_ui_synthesize_click (gtk_tree_view,
                                      assumed_layer_x,
                                      assumed_empty_layer_y,
                                      1 /*button*/,
                                      GDK_MOD1_MASK));
  picman_test_run_mainloop_until_idle ();

  /* Make sure that emptied the selection and that the active layer
   * still didn't change
   */
  g_assert (! picman_channel_bounds (selection,
                                   NULL, NULL, /*x1, y1*/
                                   NULL, NULL  /*x2, y2*/));
  g_assert (picman_image_get_active_layer (image) == active_layer);
#endif
}

static void
restore_recently_closed_multi_column_dock (gconstpointer data)
{
  Picman      *picman                          = PICMAN (data);
  GtkWidget *dock_window                   = NULL;
  gint       n_session_infos_before_close  = -1;
  gint       n_session_infos_after_close   = -1;
  gint       n_session_infos_after_restore = -1;
  GList     *session_infos                 = NULL;

  /* Find a non-toolbox dock window */
  dock_window = picman_ui_find_window (picman_dialog_factory_get_singleton (),
                                     picman_ui_multicolumn_not_toolbox_window);
  g_assert (dock_window != NULL);

  /* Count number of docks */
  session_infos = picman_dialog_factory_get_session_infos (picman_dialog_factory_get_singleton ());
  n_session_infos_before_close = g_list_length (session_infos);

  /* Close one of the dock windows */
  picman_ui_synthesize_delete_event (GTK_WIDGET (dock_window));
  picman_test_run_mainloop_until_idle ();

  /* Make sure the number of session infos went down */
  session_infos = picman_dialog_factory_get_session_infos (picman_dialog_factory_get_singleton ());
  n_session_infos_after_close = g_list_length (session_infos);
  g_assert_cmpint (n_session_infos_before_close,
                   >,
                   n_session_infos_after_close);

#ifdef __GNUC__
#warning FIXME test disabled until we depend on GTK+ >= 2.24.11
#endif
#if 0
  /* Restore the (only avaiable) closed dock and make sure the session
   * infos in the global dock factory are increased again
   */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "windows",
                                   /* FIXME: This is severely hardcoded */
                                   "windows-recent-0003");
  picman_test_run_mainloop_until_idle ();
  session_infos = picman_dialog_factory_get_session_infos (picman_dialog_factory_get_singleton ());
  n_session_infos_after_restore = g_list_length (session_infos);
  g_assert_cmpint (n_session_infos_after_close,
                   <,
                   n_session_infos_after_restore);
#endif
}

/**
 * tab_toggle_dont_change_dock_window_position:
 * @data:
 *
 * Makes sure that when dock windows are hidden with Tab and shown
 * again, their positions and sizes are not changed. We don't really
 * use Tab though, we only simulate its effect.
 **/
static void
tab_toggle_dont_change_dock_window_position (gconstpointer data)
{
  Picman      *picman          = PICMAN (data);
  GtkWidget *dock_window   = NULL;
  gint       x_before_hide = -1;
  gint       y_before_hide = -1;
  gint       w_before_hide = -1;
  gint       h_before_hide = -1;
  gint       x_after_show  = -1;
  gint       y_after_show  = -1;
  gint       w_after_show  = -1;
  gint       h_after_show  = -1;

  /* Find a non-toolbox dock window */
  dock_window = picman_ui_find_window (picman_dialog_factory_get_singleton (),
                                     picman_ui_not_toolbox_window);
  g_assert (dock_window != NULL);
  g_assert (gtk_widget_get_visible (dock_window));

  /* Get the position and size */
  picman_test_run_mainloop_until_idle ();
  gtk_window_get_position (GTK_WINDOW (dock_window),
                           &x_before_hide,
                           &y_before_hide);
  gtk_window_get_size (GTK_WINDOW (dock_window),
                       &w_before_hide,
                       &h_before_hide);

  /* Hide all dock windows */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "windows",
                                   "windows-hide-docks");
  picman_test_run_mainloop_until_idle ();
  g_assert (! gtk_widget_get_visible (dock_window));

  /* Show them again */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "windows",
                                   "windows-hide-docks");
  picman_test_run_mainloop_until_idle ();
  g_assert (gtk_widget_get_visible (dock_window));

  /* Get the position and size again and make sure it's the same as
   * before
   */
  gtk_window_get_position (GTK_WINDOW (dock_window),
                           &x_after_show,
                           &y_after_show);
  gtk_window_get_size (GTK_WINDOW (dock_window),
                       &w_after_show,
                       &h_after_show);
  g_assert_cmpint ((int)abs (x_before_hide - x_after_show), <=, PICMAN_UI_WINDOW_POSITION_EPSILON);
  g_assert_cmpint ((int)abs (y_before_hide - y_after_show), <=, PICMAN_UI_WINDOW_POSITION_EPSILON);
  g_assert_cmpint ((int)abs (w_before_hide - w_after_show), <=, PICMAN_UI_WINDOW_POSITION_EPSILON);
  g_assert_cmpint ((int)abs (h_before_hide - h_after_show), <=, PICMAN_UI_WINDOW_POSITION_EPSILON);
}

static void
switch_to_single_window_mode (gconstpointer data)
{
  Picman *picman = PICMAN (data);

  /* Switch to single-window mode. We consider this test as passed if
   * we don't get any GLib warnings/errors
   */
  picman_ui_switch_window_mode (picman);
}

static void
picman_ui_toggle_docks_in_single_window_mode (Picman *picman)
{
  PicmanDisplay      *display       = PICMAN_DISPLAY (picman_get_display_iter (picman)->data);
  PicmanDisplayShell *shell         = picman_display_get_shell (display);
  GtkWidget        *toplevel      = GTK_WIDGET (picman_display_shell_get_window (shell));
  gint              x_temp        = -1;
  gint              y_temp        = -1;
  gint              x_before_hide = -1;
  gint              y_before_hide = -1;
  gint              x_after_hide  = -1;
  gint              y_after_hide  = -1;
  g_assert (shell);
  g_assert (toplevel);

  /* Get toplevel coordinate of image origin */
  picman_test_run_mainloop_until_idle ();
  picman_display_shell_transform_xy (shell,
                                   0.0, 0.0,
                                   &x_temp, &y_temp);
  gtk_widget_translate_coordinates (GTK_WIDGET (shell),
                                    toplevel,
                                    x_temp, y_temp,
                                    &x_before_hide, &y_before_hide);

  /* Hide all dock windows */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "windows",
                                   "windows-hide-docks");
  picman_test_run_mainloop_until_idle ();

  /* Get toplevel coordinate of image origin */
  picman_test_run_mainloop_until_idle ();
  picman_display_shell_transform_xy (shell,
                                   0.0, 0.0,
                                   &x_temp, &y_temp);
  gtk_widget_translate_coordinates (GTK_WIDGET (shell),
                                    toplevel,
                                    x_temp, y_temp,
                                    &x_after_hide, &y_after_hide);

  g_assert_cmpint ((int)abs (x_after_hide - x_before_hide), <=, PICMAN_UI_POSITION_EPSILON);
  g_assert_cmpint ((int)abs (y_after_hide - y_before_hide), <=, PICMAN_UI_POSITION_EPSILON);
}

static void
hide_docks_in_single_window_mode (gconstpointer data)
{
  Picman *picman = PICMAN (data);
  picman_ui_toggle_docks_in_single_window_mode (picman);
}

static void
show_docks_in_single_window_mode (gconstpointer data)
{
  Picman *picman = PICMAN (data);
  picman_ui_toggle_docks_in_single_window_mode (picman);
}

static void
maximize_state_in_aux_data (gconstpointer data)
{
  Picman               *picman    = PICMAN (data);
  PicmanDisplay        *display = PICMAN_DISPLAY (picman_get_display_iter (picman)->data);
  PicmanDisplayShell   *shell   = picman_display_get_shell (display);
  PicmanImageWindow    *window  = picman_display_shell_get_window (shell);
  gint                i;

  for (i = 0; i < 2; i++)
    {
      GList              *aux_info = NULL;
      PicmanSessionInfoAux *target_info;
      gboolean            target_max_state;

      if (i == 0)
        {
          target_info = picman_session_info_aux_new ("maximized" , "yes");
          target_max_state = TRUE;
        }
      else
        {
          target_info = picman_session_info_aux_new ("maximized", "no");
          target_max_state = FALSE;
        }

      /* Set the aux info to out target data */
      aux_info = g_list_append (aux_info, target_info);
      picman_session_managed_set_aux_info (PICMAN_SESSION_MANAGED (window), aux_info);
      g_list_free (aux_info);

      /* Give the WM a chance to maximize/unmaximize us */
      picman_test_run_mainloop_until_idle ();
      g_usleep (500 * 1000);
      picman_test_run_mainloop_until_idle ();

      /* Make sure the maximize/unmaximize happened */
      g_assert (picman_image_window_is_maximized (window) == target_max_state);

      /* Make sure we can read out the window state again */
      aux_info = picman_session_managed_get_aux_info (PICMAN_SESSION_MANAGED (window));
      g_assert (g_list_find_custom (aux_info, target_info, picman_ui_aux_data_eqiuvalent));
      g_list_free_full (aux_info,
                        (GDestroyNotify) picman_session_info_aux_free);

      picman_session_info_aux_free (target_info);
    }
}

static void
switch_back_to_multi_window_mode (gconstpointer data)
{
  Picman *picman = PICMAN (data);

  /* Switch back to multi-window mode. We consider this test as passed
   * if we don't get any GLib warnings/errors
   */
  picman_ui_switch_window_mode (picman);
}

static void
close_image (gconstpointer data)
{
  Picman *picman       = PICMAN (data);
  int   undo_count = 4;

  /* Undo all changes so we don't need to find the 'Do you want to
   * save?'-dialog and its 'No' button
   */
  while (undo_count--)
    {
      picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                       "edit",
                                       "edit-undo");
      picman_test_run_mainloop_until_idle ();
    }

  /* Close the image */
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "view",
                                   "view-close");
  picman_test_run_mainloop_until_idle ();

  /* Did it really disappear? */
  g_assert_cmpint (g_list_length (picman_get_image_iter (picman)), ==, 0);
}

/**
 * repeatedly_switch_window_mode:
 * @data:
 *
 * Makes sure that the size of the image window is properly handled
 * when repeatedly switching between window modes.
 **/
static void
repeatedly_switch_window_mode (gconstpointer data)
{
#ifdef __GNUC__
#warning FIXME: plesase fix repeatedly_switch_window_mode test
#endif
#if 0
  Picman             *picman     = PICMAN (data);
  PicmanDisplay      *display  = PICMAN_DISPLAY (picman_get_empty_display (picman));
  PicmanDisplayShell *shell    = picman_display_get_shell (display);
  GtkWidget        *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

  gint expected_initial_height;
  gint expected_initial_width;
  gint expected_second_height;
  gint expected_second_width;
  gint initial_width;
  gint initial_height;
  gint second_width;
  gint second_height;

  /* We need this for some reason */
  picman_test_run_mainloop_until_idle ();

  /* Remember the multi-window mode size */
  gtk_window_get_size (GTK_WINDOW (toplevel),
                       &expected_initial_width,
                       &expected_initial_height);

  /* Switch to single-window mode */
  picman_ui_switch_window_mode (picman);

  /* Rememeber the single-window mode size */
  gtk_window_get_size (GTK_WINDOW (toplevel),
                       &expected_second_width,
                       &expected_second_height);

  /* Make sure they differ, otherwise the test is pointless */
  g_assert_cmpint (expected_initial_width,  !=, expected_second_width);
  g_assert_cmpint (expected_initial_height, !=, expected_second_height);

  /* Switch back to multi-window mode */
  picman_ui_switch_window_mode (picman);

  /* Make sure the size is the same as before */
  gtk_window_get_size (GTK_WINDOW (toplevel), &initial_width, &initial_height);
  g_assert_cmpint (expected_initial_width,  ==, initial_width);
  g_assert_cmpint (expected_initial_height, ==, initial_height);

  /* Switch to single-window mode again... */
  picman_ui_switch_window_mode (picman);

  /* Make sure the size is the same as before */
  gtk_window_get_size (GTK_WINDOW (toplevel), &second_width, &second_height);
  g_assert_cmpint (expected_second_width,  ==, second_width);
  g_assert_cmpint (expected_second_height, ==, second_height);

  /* Finally switch back to multi-window mode since that was the mode
   * when we started
   */
  picman_ui_switch_window_mode (picman);
#endif
}

/**
 * window_roles:
 * @data:
 *
 * Makes sure that different windows have the right roles specified.
 **/
static void
window_roles (gconstpointer data)
{
  GtkWidget      *dock           = NULL;
  GtkWidget      *toolbox        = NULL;
  PicmanDockWindow *dock_window    = NULL;
  PicmanDockWindow *toolbox_window = NULL;

  dock           = picman_dock_with_window_new (picman_dialog_factory_get_singleton (),
                                              gdk_screen_get_default (),
                                              FALSE /*toolbox*/);
  toolbox        = picman_dock_with_window_new (picman_dialog_factory_get_singleton (),
                                              gdk_screen_get_default (),
                                              TRUE /*toolbox*/);
  dock_window    = picman_dock_window_from_dock (PICMAN_DOCK (dock));
  toolbox_window = picman_dock_window_from_dock (PICMAN_DOCK (toolbox));

  g_assert_cmpint (g_str_has_prefix (gtk_window_get_role (GTK_WINDOW (dock_window)), "picman-dock-"), ==,
                   TRUE);
  g_assert_cmpint (g_str_has_prefix (gtk_window_get_role (GTK_WINDOW (toolbox_window)), "picman-toolbox-"), ==,
                   TRUE);

  /* When we get here we have a ref count of one, but the signals we
   * emit cause the reference count to become less than zero for some
   * reason. So we're lazy and simply ignore to unref these
  g_object_unref (toolbox);
  g_object_unref (dock);
   */
}

static void
paintbrush_is_standard_tool (gconstpointer data)
{
  Picman         *picman         = PICMAN (data);
  PicmanContext  *user_context = picman_get_user_context (picman);
  PicmanToolInfo *tool_info    = picman_context_get_tool (user_context);

  g_assert_cmpstr (tool_info->help_id,
                   ==,
                   "picman-tool-paintbrush");
}

/**
 * picman_ui_synthesize_delete_event:
 * @widget:
 *
 * Synthesize a delete event to @widget.
 **/
static void
picman_ui_synthesize_delete_event (GtkWidget *widget)
{
  GdkWindow *window = NULL;
  GdkEvent *event = NULL;

  window = gtk_widget_get_window (widget);
  g_assert (window);

  event = gdk_event_new (GDK_DELETE);
  event->any.window     = g_object_ref (window);
  event->any.send_event = TRUE;
  gtk_main_do_event (event);
  gdk_event_free (event);
}

static gboolean
picman_ui_synthesize_click (GtkWidget       *widget,
                          gint             x,
                          gint             y,
                          gint             button, /*1..3*/
                          GdkModifierType  modifiers)
{
  return (gdk_test_simulate_button (gtk_widget_get_window (widget),
                                    x, y,
                                    button,
                                    modifiers,
                                    GDK_BUTTON_PRESS) &&
          gdk_test_simulate_button (gtk_widget_get_window (widget),
                                    x, y,
                                    button,
                                    modifiers,
                                    GDK_BUTTON_RELEASE));
}

static GtkWidget *
picman_ui_find_window (PicmanDialogFactory *dialog_factory,
                     PicmanUiTestFunc     predicate)
{
  GList     *iter        = NULL;
  GtkWidget *dock_window = NULL;

  g_return_val_if_fail (predicate != NULL, NULL);

  for (iter = picman_dialog_factory_get_session_infos (dialog_factory);
       iter;
       iter = g_list_next (iter))
    {
      GtkWidget *widget = picman_session_info_get_widget (iter->data);

      if (predicate (G_OBJECT (widget)))
        {
          dock_window = widget;
          break;
        }
    }

  return dock_window;
}

static gboolean
picman_ui_not_toolbox_window (GObject *object)
{
  return (PICMAN_IS_DOCK_WINDOW (object) &&
          ! picman_dock_window_has_toolbox (PICMAN_DOCK_WINDOW (object)));
}

static gboolean
picman_ui_multicolumn_not_toolbox_window (GObject *object)
{
  gboolean           not_toolbox_window;
  PicmanDockWindow    *dock_window;
  PicmanDockContainer *dock_container;
  GList             *docks;

  if (! PICMAN_IS_DOCK_WINDOW (object))
    return FALSE;

  dock_window    = PICMAN_DOCK_WINDOW (object);
  dock_container = PICMAN_DOCK_CONTAINER (object);
  docks          = picman_dock_container_get_docks (dock_container);

  not_toolbox_window = (! picman_dock_window_has_toolbox (dock_window) &&
                        g_list_length (docks) > 1);

  g_list_free (docks);

  return not_toolbox_window;
}

static gboolean
picman_ui_is_picman_layer_list (GObject *object)
{
  PicmanDialogFactoryEntry *entry = NULL;

  if (! GTK_IS_WIDGET (object))
    return FALSE;

  picman_dialog_factory_from_widget (GTK_WIDGET (object), &entry);

  return strcmp (entry->identifier, "picman-layer-list") == 0;
}

static int
picman_ui_aux_data_eqiuvalent (gconstpointer _a, gconstpointer _b)
{
  PicmanSessionInfoAux *a = (PicmanSessionInfoAux*) _a;
  PicmanSessionInfoAux *b = (PicmanSessionInfoAux*) _b;
  return (strcmp (a->name, b->name) || strcmp (a->value, b->value));
}

static void
picman_ui_switch_window_mode (Picman *picman)
{
  picman_ui_manager_activate_action (picman_test_utils_get_ui_manager (picman),
                                   "windows",
                                   "windows-use-single-window-mode");
  picman_test_run_mainloop_until_idle ();

  /* Add a small sleep to let things stabilize */
  g_usleep (500 * 1000);
  picman_test_run_mainloop_until_idle ();
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

  /* Add tests. Note that the order matters. For example,
   * 'paintbrush_is_standard_tool' can't be after
   * 'tool_options_editor_updates'
   */
  ADD_TEST (paintbrush_is_standard_tool);
  ADD_TEST (tool_options_editor_updates);
  ADD_TEST (automatic_tab_style);
  ADD_TEST (create_new_image_via_dialog);
  ADD_TEST (keyboard_zoom_focus);
  ADD_TEST (alt_click_is_layer_to_selection);
  ADD_TEST (restore_recently_closed_multi_column_dock);
  ADD_TEST (tab_toggle_dont_change_dock_window_position);
  ADD_TEST (switch_to_single_window_mode);
  ADD_TEST (hide_docks_in_single_window_mode);
  ADD_TEST (show_docks_in_single_window_mode);
#warning FIXME: maximize_state_in_aux_data doesn't work without WM
#if 0
  ADD_TEST (maximize_state_in_aux_data);
#endif
  ADD_TEST (switch_back_to_multi_window_mode);
  ADD_TEST (close_image);
  ADD_TEST (repeatedly_switch_window_mode);
  ADD_TEST (window_roles);

  /* Run the tests and return status */
  result = g_test_run ();

  /* Don't write files to the source dir */
  picman_test_utils_set_picman2_directory ("PICMAN_TESTING_ABS_TOP_BUILDDIR",
                                       "app/tests/picmandir-output");

  /* Exit properly so we don't break script-fu plug-in wire */
  picman_exit (picman, TRUE);

  return result;
}
