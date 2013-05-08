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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmandisplayoptions.h"
#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmangrouplayer.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmancolordialog.h"
#include "widgets/picmandock.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwindowstrategy.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmandisplayshell-filter-dialog.h"
#include "display/picmandisplayshell-rotate.h"
#include "display/picmandisplayshell-rotate-dialog.h"
#include "display/picmandisplayshell-scale.h"
#include "display/picmandisplayshell-scale-dialog.h"
#include "display/picmandisplayshell-scroll.h"
#include "display/picmandisplayshell-close.h"
#include "display/picmanimagewindow.h"

#include "actions.h"
#include "view-commands.h"

#include "picman-intl.h"


#define SET_ACTIVE(manager,action_name,active) \
  { PicmanActionGroup *group = \
      picman_ui_manager_get_action_group (manager, "view"); \
    picman_action_group_set_action_active (group, action_name, active); }

#define IS_ACTIVE_DISPLAY(display) \
  ((display) == \
   picman_context_get_display (picman_get_user_context ((display)->picman)))


/*  local function prototypes  */

static void   view_padding_color_dialog_update (PicmanColorDialog      *dialog,
                                                const PicmanRGB        *color,
                                                PicmanColorDialogState  state,
                                                PicmanDisplayShell     *shell);


/*  public functions  */

void
view_new_cmd_callback (GtkAction *action,
                       gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);

  picman_create_display (display->picman,
                       picman_display_get_image (display),
                       shell->unit, picman_zoom_model_get_factor (shell->zoom));
}

void
view_close_cmd_callback (GtkAction *action,
                         gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  PicmanImage        *image;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);
  image = picman_display_get_image (display);

  /* Check for active image so we don't close the last display. */
  if (shell && image)
    picman_display_shell_close (shell, FALSE);
}

void
view_zoom_fit_in_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay *display;
  return_if_no_display (display, data);

  picman_display_shell_scale_fit_in (picman_display_get_shell (display));
}

void
view_zoom_fill_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay *display;
  return_if_no_display (display, data);

  picman_display_shell_scale_fill (picman_display_get_shell (display));
}

void
view_zoom_revert_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay *display;
  return_if_no_display (display, data);

  picman_display_shell_scale_revert (picman_display_get_shell (display));
}

void
view_zoom_cmd_callback (GtkAction *action,
                        gint       value,
                        gpointer   data)
{
  PicmanDisplayShell *shell;
  return_if_no_shell (shell, data);

  switch ((PicmanActionSelectType) value)
    {
    case PICMAN_ACTION_SELECT_FIRST:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_OUT_MAX,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    case PICMAN_ACTION_SELECT_LAST:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_IN_MAX,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    case PICMAN_ACTION_SELECT_PREVIOUS:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_OUT,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    case PICMAN_ACTION_SELECT_NEXT:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_IN,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    case PICMAN_ACTION_SELECT_SKIP_PREVIOUS:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_OUT_MORE,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    case PICMAN_ACTION_SELECT_SKIP_NEXT:
      picman_display_shell_scale (shell,
                                PICMAN_ZOOM_IN_MORE,
                                0.0,
                                PICMAN_ZOOM_FOCUS_BEST_GUESS);
      break;

    default:
      {
        gdouble scale = picman_zoom_model_get_factor (shell->zoom);

        scale = action_select_value ((PicmanActionSelectType) value,
                                     scale,
                                     0.0, 512.0, 1.0,
                                     1.0 / 8.0, 1.0, 16.0, 0.0,
                                     FALSE);

        /* min = 1.0 / 256,  max = 256.0                */
        /* scale = min *  (max / min)**(i/n), i = 0..n  */
        scale = pow (65536.0, scale / 512.0) / 256.0;

        picman_display_shell_scale (shell,
                                  PICMAN_ZOOM_TO,
                                  scale,
                                  PICMAN_ZOOM_FOCUS_BEST_GUESS);
        break;
      }
    }
}

void
view_zoom_explicit_cmd_callback (GtkAction *action,
                                 GtkAction *current,
                                 gpointer   data)
{
  PicmanDisplayShell *shell;
  gint              value;
  return_if_no_shell (shell, data);

  value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  if (value != 0 /* not Other... */)
    {
      if (fabs (value - picman_zoom_model_get_factor (shell->zoom)) > 0.0001)
        picman_display_shell_scale (shell,
                                  PICMAN_ZOOM_TO,
                                  (gdouble) value / 10000,
                                  PICMAN_ZOOM_FOCUS_RETAIN_CENTERING_ELSE_BEST_GUESS);
    }
}

void
view_zoom_other_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanDisplayShell *shell;
  return_if_no_shell (shell, data);

  /* check if we are activated by the user or from
   * view_actions_set_zoom()
   */
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) &&
      shell->other_scale != picman_zoom_model_get_factor (shell->zoom))
    {
      picman_display_shell_scale_dialog (shell);
    }
}

void
view_dot_for_dot_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  gboolean          active;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);

  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (active != shell->dot_for_dot)
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);

      picman_display_shell_scale_set_dot_for_dot (shell, active);

      if (window)
        SET_ACTIVE (picman_image_window_get_ui_manager (window),
                    "view-dot-for-dot", shell->dot_for_dot);

      if (IS_ACTIVE_DISPLAY (display))
        SET_ACTIVE (shell->popup_manager, "view-dot-for-dot",
                    shell->dot_for_dot);
    }
}

void
view_rotate_reset_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);

  picman_display_shell_rotate_to (shell, 0.0);
}

void
view_rotate_cmd_callback (GtkAction *action,
                          gint       value,
                          gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  gdouble           delta = 0.0;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);

  switch ((PicmanRotationType) value)
    {
    case PICMAN_ROTATE_90:   delta =  90; break;
    case PICMAN_ROTATE_180:  delta = 180; break;
    case PICMAN_ROTATE_270:  delta = -90; break;
    }

  picman_display_shell_rotate (shell, delta);
}

void
view_rotate_other_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  return_if_no_display (display, data);

  shell = picman_display_get_shell (display);

  picman_display_shell_rotate_dialog (shell);
}

void
view_scroll_horizontal_cmd_callback (GtkAction *action,
                                     gint       value,
                                     gpointer   data)
{
  PicmanDisplayShell *shell;
  gdouble           offset;
  return_if_no_shell (shell, data);

  offset = action_select_value ((PicmanActionSelectType) value,
                                gtk_adjustment_get_value (shell->hsbdata),
                                gtk_adjustment_get_lower (shell->hsbdata),
                                gtk_adjustment_get_upper (shell->hsbdata) -
                                gtk_adjustment_get_page_size (shell->hsbdata),
                                gtk_adjustment_get_lower (shell->hsbdata),
                                1,
                                gtk_adjustment_get_step_increment (shell->hsbdata),
                                gtk_adjustment_get_page_increment (shell->hsbdata),
                                0,
                                FALSE);

  gtk_adjustment_set_value (shell->hsbdata, offset);
}

void
view_scroll_vertical_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanDisplayShell *shell;
  gdouble           offset;
  return_if_no_shell (shell, data);

  offset = action_select_value ((PicmanActionSelectType) value,
                                gtk_adjustment_get_value (shell->vsbdata),
                                gtk_adjustment_get_lower (shell->vsbdata),
                                gtk_adjustment_get_upper (shell->vsbdata) -
                                gtk_adjustment_get_page_size (shell->vsbdata),
                                gtk_adjustment_get_lower (shell->vsbdata),
                                1,
                                gtk_adjustment_get_step_increment (shell->vsbdata),
                                gtk_adjustment_get_page_increment (shell->vsbdata),
                                0,
                                FALSE);

  gtk_adjustment_set_value (shell->vsbdata, offset);
}

void
view_navigation_window_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  Picman             *picman;
  PicmanDisplayShell *shell;
  return_if_no_picman (picman, data);
  return_if_no_shell (shell, data);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (picman)),
                                             picman,
                                             picman_dialog_factory_get_singleton (),
                                             gtk_widget_get_screen (GTK_WIDGET (shell)),
                                             "picman-navigation-view");
}

void
view_display_filters_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanDisplayShell *shell;
  return_if_no_shell (shell, data);

  if (! shell->filters_dialog)
    {
      shell->filters_dialog = picman_display_shell_filter_dialog_new (shell);

      g_signal_connect (shell->filters_dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &shell->filters_dialog);
    }

  gtk_window_present (GTK_WINDOW (shell->filters_dialog));
}

void
view_toggle_selection_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_selection (shell))
    return;

  picman_display_shell_set_show_selection (shell, active);
}

void
view_toggle_layer_boundary_cmd_callback (GtkAction *action,
                                         gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_layer (shell))
    return;

  picman_display_shell_set_show_layer (shell, active);
}

void
view_toggle_menubar_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_menubar (shell))
    return;

  picman_display_shell_set_show_menubar (shell, active);
}

void
view_toggle_rulers_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_rulers (shell))
    return;

  picman_display_shell_set_show_rulers (shell, active);
}

void
view_toggle_scrollbars_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_scrollbars (shell))
    return;

  picman_display_shell_set_show_scrollbars (shell, active);
}

void
view_toggle_statusbar_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_statusbar (shell))
    return;

  picman_display_shell_set_show_statusbar (shell, active);
}

void
view_toggle_guides_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_guides (shell))
    return;

  picman_display_shell_set_show_guides (shell, active);
}

void
view_toggle_grid_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_grid (shell))
    return;

  picman_display_shell_set_show_grid (shell, active);
}

void
view_toggle_sample_points_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_show_sample_points (shell))
    return;

  picman_display_shell_set_show_sample_points (shell, active);
}

void
view_snap_to_guides_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_snap_to_guides (shell))
    return;

  picman_display_shell_set_snap_to_guides (shell, active);
}

void
view_snap_to_grid_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_snap_to_grid (shell))
    return;

  picman_display_shell_set_snap_to_grid (shell, active);
}

void
view_snap_to_canvas_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_snap_to_canvas (shell))
    return;

  picman_display_shell_set_snap_to_canvas (shell, active);
}

void
view_snap_to_vectors_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanDisplayShell *shell;
  gboolean          active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  return_if_no_shell (shell, data);

  if (active == picman_display_shell_get_snap_to_vectors (shell))
    return;

  picman_display_shell_set_snap_to_vectors (shell, active);
}

void
view_padding_color_cmd_callback (GtkAction *action,
                                 gint       value,
                                 gpointer   data)
{
  PicmanDisplay        *display;
  PicmanImageWindow    *window;
  PicmanDisplayShell   *shell;
  PicmanDisplayOptions *options;
  gboolean            fullscreen;
  return_if_no_display (display, data);

  shell  = picman_display_get_shell (display);
  window = picman_display_shell_get_window (shell);

  if (window)
    fullscreen = picman_image_window_get_fullscreen (window);
  else
    fullscreen = FALSE;

  if (fullscreen)
    options = shell->fullscreen_options;
  else
    options = shell->options;

  switch ((PicmanCanvasPaddingMode) value)
    {
    case PICMAN_CANVAS_PADDING_MODE_DEFAULT:
    case PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK:
    case PICMAN_CANVAS_PADDING_MODE_DARK_CHECK:
      g_object_set_data (G_OBJECT (shell), "padding-color-dialog", NULL);

      options->padding_mode_set = TRUE;

      picman_display_shell_set_padding (shell, (PicmanCanvasPaddingMode) value,
                                      &options->padding_color);
      break;

    case PICMAN_CANVAS_PADDING_MODE_CUSTOM:
      {
        GtkWidget *color_dialog;

        color_dialog = g_object_get_data (G_OBJECT (shell),
                                          "padding-color-dialog");

        if (! color_dialog)
          {
            PicmanImage        *image = picman_display_get_image (display);
            PicmanDisplayShell *shell = picman_display_get_shell (display);

            color_dialog =
              picman_color_dialog_new (PICMAN_VIEWABLE (image),
                                     action_data_get_context (data),
                                     _("Set Canvas Padding Color"),
                                     GTK_STOCK_SELECT_COLOR,
                                     _("Set Custom Canvas Padding Color"),
                                     GTK_WIDGET (shell),
                                     NULL, NULL,
                                     &options->padding_color,
                                     FALSE, FALSE);

            g_signal_connect (color_dialog, "update",
                              G_CALLBACK (view_padding_color_dialog_update),
                              shell);

            g_object_set_data_full (G_OBJECT (shell), "padding-color-dialog",
                                    color_dialog,
                                    (GDestroyNotify) gtk_widget_destroy);
          }

        gtk_window_present (GTK_WINDOW (color_dialog));
      }
      break;

    case PICMAN_CANVAS_PADDING_MODE_RESET:
      g_object_set_data (G_OBJECT (shell), "padding-color-dialog", NULL);

      {
        PicmanDisplayOptions *default_options;

        options->padding_mode_set = FALSE;

        if (fullscreen)
          default_options = display->config->default_fullscreen_view;
        else
          default_options = display->config->default_view;

        picman_display_shell_set_padding (shell,
                                        default_options->padding_mode,
                                        &default_options->padding_color);
      }
      break;
    }
}

void
view_shrink_wrap_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanDisplayShell *shell;
  return_if_no_shell (shell, data);

  picman_display_shell_scale_shrink_wrap (shell, FALSE);
}

void
view_fullscreen_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanDisplay      *display;
  PicmanDisplayShell *shell;
  PicmanImageWindow  *window;
  return_if_no_display (display, data);

  shell  = picman_display_get_shell (display);
  window = picman_display_shell_get_window (shell);

  if (window)
    {
      gboolean active;

      active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

      picman_image_window_set_fullscreen (window, active);
    }
}


/*  private functions  */

static void
view_padding_color_dialog_update (PicmanColorDialog      *dialog,
                                  const PicmanRGB        *color,
                                  PicmanColorDialogState  state,
                                  PicmanDisplayShell     *shell)
{
  PicmanImageWindow    *window;
  PicmanDisplayOptions *options;
  gboolean            fullscreen;

  window = picman_display_shell_get_window (shell);

  if (window)
    fullscreen = picman_image_window_get_fullscreen (window);
  else
    fullscreen = FALSE;

  if (fullscreen)
    options = shell->fullscreen_options;
  else
    options = shell->options;

  switch (state)
    {
    case PICMAN_COLOR_DIALOG_OK:
      options->padding_mode_set = TRUE;

      picman_display_shell_set_padding (shell, PICMAN_CANVAS_PADDING_MODE_CUSTOM,
                                      color);
      /* fallthru */

    case PICMAN_COLOR_DIALOG_CANCEL:
      g_object_set_data (G_OBJECT (shell), "padding-color-dialog", NULL);
      break;

    default:
      break;
    }
}
