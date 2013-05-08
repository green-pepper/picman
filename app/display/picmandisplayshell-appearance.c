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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayoptions.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandockcolumns.h"
#include "widgets/picmanrender.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmancanvas.h"
#include "picmancanvasitem.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-selection.h"
#include "picmanimagewindow.h"
#include "picmanstatusbar.h"


/*  local function prototypes  */

static PicmanDisplayOptions *
              appearance_get_options       (PicmanDisplayShell       *shell);
static void   appearance_set_action_active (PicmanDisplayShell       *shell,
                                            const gchar            *action,
                                            gboolean                active);
static void   appearance_set_action_color  (PicmanDisplayShell       *shell,
                                            const gchar            *action,
                                            const PicmanRGB          *color);


/*  public functions  */

void
picman_display_shell_appearance_update (PicmanDisplayShell *shell)
{
  PicmanDisplayOptions *options;
  PicmanImageWindow    *window;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);
  window  = picman_display_shell_get_window (shell);

  if (window)
    {
      PicmanDockColumns *left_docks;
      PicmanDockColumns *right_docks;
      gboolean         fullscreen;
      gboolean         has_grip;

      fullscreen = picman_image_window_get_fullscreen (window);

      appearance_set_action_active (shell, "view-fullscreen", fullscreen);

      left_docks  = picman_image_window_get_left_docks (window);
      right_docks = picman_image_window_get_right_docks (window);

      has_grip = (! fullscreen &&
                  ! (left_docks  && picman_dock_columns_get_docks (left_docks)) &&
                  ! (right_docks && picman_dock_columns_get_docks (right_docks)));

      gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (shell->statusbar),
                                         has_grip);
    }

  picman_display_shell_set_show_menubar       (shell,
                                             options->show_menubar);
  picman_display_shell_set_show_statusbar     (shell,
                                             options->show_statusbar);

  picman_display_shell_set_show_rulers        (shell,
                                             options->show_rulers);
  picman_display_shell_set_show_scrollbars    (shell,
                                             options->show_scrollbars);
  picman_display_shell_set_show_selection     (shell,
                                             options->show_selection);
  picman_display_shell_set_show_layer         (shell,
                                             options->show_layer_boundary);
  picman_display_shell_set_show_guides        (shell,
                                             options->show_guides);
  picman_display_shell_set_show_grid          (shell,
                                             options->show_grid);
  picman_display_shell_set_show_sample_points (shell,
                                             options->show_sample_points);
  picman_display_shell_set_padding            (shell,
                                             options->padding_mode,
                                             &options->padding_color);
}

void
picman_display_shell_set_show_menubar (PicmanDisplayShell *shell,
                                     gboolean          show)
{
  PicmanDisplayOptions *options;
  PicmanImageWindow    *window;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);
  window  = picman_display_shell_get_window (shell);

  g_object_set (options, "show-menubar", show, NULL);

  if (window && picman_image_window_get_active_shell (window) == shell)
    {
      picman_image_window_keep_canvas_pos (picman_display_shell_get_window (shell));
      picman_image_window_set_show_menubar (window, show);
    }

  appearance_set_action_active (shell, "view-show-menubar", show);
}

gboolean
picman_display_shell_get_show_menubar (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_menubar;
}

void
picman_display_shell_set_show_statusbar (PicmanDisplayShell *shell,
                                       gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-statusbar", show, NULL);

  picman_image_window_keep_canvas_pos (picman_display_shell_get_window (shell));
  picman_statusbar_set_visible (PICMAN_STATUSBAR (shell->statusbar), show);

  appearance_set_action_active (shell, "view-show-statusbar", show);
}

gboolean
picman_display_shell_get_show_statusbar (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_statusbar;
}

void
picman_display_shell_set_show_rulers (PicmanDisplayShell *shell,
                                    gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-rulers", show, NULL);

  picman_image_window_keep_canvas_pos (picman_display_shell_get_window (shell));
  gtk_widget_set_visible (shell->origin, show);
  gtk_widget_set_visible (shell->hrule, show);
  gtk_widget_set_visible (shell->vrule, show);

  appearance_set_action_active (shell, "view-show-rulers", show);
}

gboolean
picman_display_shell_get_show_rulers (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_rulers;
}

void
picman_display_shell_set_show_scrollbars (PicmanDisplayShell *shell,
                                        gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-scrollbars", show, NULL);

  picman_image_window_keep_canvas_pos (picman_display_shell_get_window (shell));
  gtk_widget_set_visible (shell->nav_ebox, show);
  gtk_widget_set_visible (shell->hsb, show);
  gtk_widget_set_visible (shell->vsb, show);
  gtk_widget_set_visible (shell->quick_mask_button, show);
  gtk_widget_set_visible (shell->zoom_button, show);

  appearance_set_action_active (shell, "view-show-scrollbars", show);
}

gboolean
picman_display_shell_get_show_scrollbars (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_scrollbars;
}

void
picman_display_shell_set_show_selection (PicmanDisplayShell *shell,
                                       gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-selection", show, NULL);

  picman_display_shell_selection_set_show (shell, show);

  appearance_set_action_active (shell, "view-show-selection", show);
}

gboolean
picman_display_shell_get_show_selection (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_selection;
}

void
picman_display_shell_set_show_layer (PicmanDisplayShell *shell,
                                   gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-layer-boundary", show, NULL);

  picman_canvas_item_set_visible (shell->layer_boundary, show);

  appearance_set_action_active (shell, "view-show-layer-boundary", show);
}

gboolean
picman_display_shell_get_show_layer (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_layer_boundary;
}

void
picman_display_shell_set_show_guides (PicmanDisplayShell *shell,
                                    gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-guides", show, NULL);

  picman_canvas_item_set_visible (shell->guides, show);

  appearance_set_action_active (shell, "view-show-guides", show);
}

gboolean
picman_display_shell_get_show_guides (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_guides;
}

void
picman_display_shell_set_show_grid (PicmanDisplayShell *shell,
                                  gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-grid", show, NULL);

  picman_canvas_item_set_visible (shell->grid, show);

  appearance_set_action_active (shell, "view-show-grid", show);
}

gboolean
picman_display_shell_get_show_grid (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_grid;
}

void
picman_display_shell_set_show_sample_points (PicmanDisplayShell *shell,
                                           gboolean          show)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  g_object_set (options, "show-sample-points", show, NULL);

  picman_canvas_item_set_visible (shell->sample_points, show);

  appearance_set_action_active (shell, "view-show-sample-points", show);
}

gboolean
picman_display_shell_get_show_sample_points (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return appearance_get_options (shell)->show_sample_points;
}

void
picman_display_shell_set_snap_to_grid (PicmanDisplayShell *shell,
                                     gboolean          snap)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (snap != shell->snap_to_grid)
    {
      shell->snap_to_grid = snap ? TRUE : FALSE;

      appearance_set_action_active (shell, "view-snap-to-grid", snap);
    }
}

gboolean
picman_display_shell_get_snap_to_grid (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return shell->snap_to_grid;
}

void
picman_display_shell_set_snap_to_guides (PicmanDisplayShell *shell,
                                       gboolean          snap)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (snap != shell->snap_to_guides)
    {
      shell->snap_to_guides = snap ? TRUE : FALSE;

      appearance_set_action_active (shell, "view-snap-to-guides", snap);
    }
}

gboolean
picman_display_shell_get_snap_to_guides (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return shell->snap_to_guides;
}

void
picman_display_shell_set_snap_to_canvas (PicmanDisplayShell *shell,
                                       gboolean          snap)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (snap != shell->snap_to_canvas)
    {
      shell->snap_to_canvas = snap ? TRUE : FALSE;

      appearance_set_action_active (shell, "view-snap-to-canvas", snap);
    }
}

gboolean
picman_display_shell_get_snap_to_canvas (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return shell->snap_to_canvas;
}

void
picman_display_shell_set_snap_to_vectors (PicmanDisplayShell *shell,
                                        gboolean          snap)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (snap != shell->snap_to_vectors)
    {
      shell->snap_to_vectors = snap ? TRUE : FALSE;

      appearance_set_action_active (shell, "view-snap-to-vectors", snap);
    }
}

gboolean
picman_display_shell_get_snap_to_vectors (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);

  return shell->snap_to_vectors;
}

void
picman_display_shell_set_padding (PicmanDisplayShell      *shell,
                                PicmanCanvasPaddingMode  padding_mode,
                                const PicmanRGB         *padding_color)
{
  PicmanDisplayOptions *options;
  PicmanRGB             color;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (padding_color != NULL);

  options = appearance_get_options (shell);
  color   = *padding_color;

  switch (padding_mode)
    {
    case PICMAN_CANVAS_PADDING_MODE_DEFAULT:
      if (shell->canvas)
        {
          GtkStyle *style;

          gtk_widget_ensure_style (shell->canvas);

          style = gtk_widget_get_style (shell->canvas);

          picman_rgb_set_gdk_color (&color, style->bg + GTK_STATE_NORMAL);
        }
      break;

    case PICMAN_CANVAS_PADDING_MODE_LIGHT_CHECK:
      color = *picman_render_light_check_color ();
      break;

    case PICMAN_CANVAS_PADDING_MODE_DARK_CHECK:
      color = *picman_render_dark_check_color ();
      break;

    case PICMAN_CANVAS_PADDING_MODE_CUSTOM:
    case PICMAN_CANVAS_PADDING_MODE_RESET:
      break;
    }

  g_object_set (options,
                "padding-mode",  padding_mode,
                "padding-color", &color,
                NULL);

  picman_canvas_set_bg_color (PICMAN_CANVAS (shell->canvas), &color);

  appearance_set_action_color (shell, "view-padding-color-menu",
                               &options->padding_color);
}

void
picman_display_shell_get_padding (PicmanDisplayShell       *shell,
                                PicmanCanvasPaddingMode  *padding_mode,
                                PicmanRGB                *padding_color)
{
  PicmanDisplayOptions *options;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  options = appearance_get_options (shell);

  if (padding_mode)
    *padding_mode = options->padding_mode;

  if (padding_color)
    *padding_color = options->padding_color;
}


/*  private functions  */

static PicmanDisplayOptions *
appearance_get_options (PicmanDisplayShell *shell)
{
  if (picman_display_get_image (shell->display))
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);

      if (window && picman_image_window_get_fullscreen (window))
        return shell->fullscreen_options;
      else
        return shell->options;
    }

  return shell->no_image_options;
}

static void
appearance_set_action_active (PicmanDisplayShell *shell,
                              const gchar      *action,
                              gboolean          active)
{
  PicmanImageWindow *window = picman_display_shell_get_window (shell);
  PicmanContext     *context;

  if (window && picman_image_window_get_active_shell (window) == shell)
    {
      PicmanUIManager   *manager = picman_image_window_get_ui_manager (window);
      PicmanActionGroup *action_group;

      action_group = picman_ui_manager_get_action_group (manager, "view");

      if (action_group)
        picman_action_group_set_action_active (action_group, action, active);
    }

  context = picman_get_user_context (shell->display->picman);

  if (shell->display == picman_context_get_display (context))
    {
      PicmanActionGroup *action_group;

      action_group = picman_ui_manager_get_action_group (shell->popup_manager,
                                                       "view");

      if (action_group)
        picman_action_group_set_action_active (action_group, action, active);
    }
}

static void
appearance_set_action_color (PicmanDisplayShell *shell,
                             const gchar      *action,
                             const PicmanRGB    *color)
{
  PicmanImageWindow *window = picman_display_shell_get_window (shell);
  PicmanContext     *context;

  if (window && picman_image_window_get_active_shell (window) == shell)
    {
      PicmanUIManager   *manager = picman_image_window_get_ui_manager (window);
      PicmanActionGroup *action_group;

      action_group = picman_ui_manager_get_action_group (manager, "view");

      if (action_group)
        picman_action_group_set_action_color (action_group, action, color, FALSE);
    }

  context = picman_get_user_context (shell->display->picman);

  if (shell->display == picman_context_get_display (context))
    {
      PicmanActionGroup *action_group;

      action_group = picman_ui_manager_get_action_group (shell->popup_manager,
                                                       "view");

      if (action_group)
        picman_action_group_set_action_color (action_group, action, color, FALSE);
    }
}
