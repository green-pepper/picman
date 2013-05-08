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

#include "display-types.h"

#include "config/picmanguiconfig.h"

#include "core/picmanimage.h"

#include "widgets/picmancursor.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmansessioninfo.h"

#include "picmancanvascursor.h"
#include "picmandisplay.h"
#include "picmancursorview.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-cursor.h"
#include "picmandisplayshell-transform.h"
#include "picmanstatusbar.h"


static void  picman_display_shell_real_set_cursor (PicmanDisplayShell   *shell,
                                                 PicmanCursorType      cursor_type,
                                                 PicmanToolCursorType  tool_cursor,
                                                 PicmanCursorModifier  modifier,
                                                 gboolean            always_install);


/*  public functions  */

void
picman_display_shell_set_cursor (PicmanDisplayShell   *shell,
                               PicmanCursorType      cursor_type,
                               PicmanToolCursorType  tool_cursor,
                               PicmanCursorModifier  modifier)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (! shell->using_override_cursor)
    {
      picman_display_shell_real_set_cursor (shell,
                                          cursor_type,
                                          tool_cursor,
                                          modifier,
                                          FALSE);
    }
}

void
picman_display_shell_unset_cursor (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (! shell->using_override_cursor)
    {
      picman_display_shell_real_set_cursor (shell,
                                          (PicmanCursorType) -1, 0, 0, FALSE);
    }
}

void
picman_display_shell_set_override_cursor (PicmanDisplayShell *shell,
                                        PicmanCursorType    cursor_type)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (! shell->using_override_cursor ||
      (shell->using_override_cursor &&
       shell->override_cursor != cursor_type))
    {
      shell->override_cursor       = cursor_type;
      shell->using_override_cursor = TRUE;

      picman_cursor_set (shell->canvas,
                       shell->cursor_handedness,
                       cursor_type,
                       PICMAN_TOOL_CURSOR_NONE,
                       PICMAN_CURSOR_MODIFIER_NONE);
    }
}

void
picman_display_shell_unset_override_cursor (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->using_override_cursor)
    {
      shell->using_override_cursor = FALSE;

      picman_display_shell_real_set_cursor (shell,
                                          shell->current_cursor,
                                          shell->tool_cursor,
                                          shell->cursor_modifier,
                                          TRUE);
    }
}

void
picman_display_shell_update_software_cursor (PicmanDisplayShell    *shell,
                                           PicmanCursorPrecision  precision,
                                           gint                 display_x,
                                           gint                 display_y,
                                           gdouble              image_x,
                                           gdouble              image_y)
{
  PicmanStatusbar   *statusbar;
  GtkWidget       *widget;
  PicmanImage       *image;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  image = picman_display_get_image (shell->display);

  if (shell->draw_cursor &&
      shell->proximity   &&
      display_x >= 0     &&
      display_y >= 0)
    {
      picman_canvas_item_begin_change (shell->cursor);

      picman_canvas_cursor_set (shell->cursor,
                              display_x,
                              display_y);
      picman_canvas_item_set_visible (shell->cursor, TRUE);

      picman_canvas_item_end_change (shell->cursor);
    }
  else
    {
      picman_canvas_item_set_visible (shell->cursor, FALSE);
    }

  /*  use the passed image_coords for the statusbar because they are
   *  possibly snapped...
   */
  statusbar = picman_display_shell_get_statusbar (shell);

  picman_statusbar_update_cursor (statusbar, precision, image_x, image_y);

  widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                            "picman-cursor-view");
  if (widget)
    {
      GtkWidget *cursor_view;

      cursor_view = gtk_bin_get_child (GTK_BIN (widget));

      if (cursor_view)
        {
          gint t_x = -1;
          gint t_y = -1;

          /*  ...but use the unsnapped display_coords for the info window  */
          if (display_x >= 0 && display_y >= 0)
            picman_display_shell_untransform_xy (shell, display_x, display_y,
                                               &t_x, &t_y, FALSE);

          picman_cursor_view_update_cursor (PICMAN_CURSOR_VIEW (cursor_view),
                                          image, shell->unit,
                                          t_x, t_y);
        }
    }
}

void
picman_display_shell_clear_software_cursor (PicmanDisplayShell *shell)
{
  PicmanStatusbar *statusbar;
  GtkWidget     *widget;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_canvas_item_set_visible (shell->cursor, FALSE);

  statusbar = picman_display_shell_get_statusbar (shell);

  picman_statusbar_clear_cursor (statusbar);

  widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                            "picman-cursor-view");
  if (widget)
    {
      GtkWidget *cursor_view;

      cursor_view = gtk_bin_get_child (GTK_BIN (widget));

      if (cursor_view)
        picman_cursor_view_clear_cursor (PICMAN_CURSOR_VIEW (cursor_view));
    }
}


/*  private functions  */

static void
picman_display_shell_real_set_cursor (PicmanDisplayShell   *shell,
                                    PicmanCursorType      cursor_type,
                                    PicmanToolCursorType  tool_cursor,
                                    PicmanCursorModifier  modifier,
                                    gboolean            always_install)
{
  PicmanHandedness cursor_handedness;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (cursor_type == (PicmanCursorType) -1)
    {
      shell->current_cursor = cursor_type;

      if (gtk_widget_is_drawable (shell->canvas))
        gdk_window_set_cursor (gtk_widget_get_window (shell->canvas), NULL);

      return;
    }

  if (cursor_type != PICMAN_CURSOR_NONE &&
      cursor_type != PICMAN_CURSOR_BAD)
    {
      switch (shell->display->config->cursor_mode)
        {
        case PICMAN_CURSOR_MODE_TOOL_ICON:
          break;

        case PICMAN_CURSOR_MODE_TOOL_CROSSHAIR:
          if (cursor_type < PICMAN_CURSOR_CORNER_TOP ||
              cursor_type > PICMAN_CURSOR_SIDE_TOP_LEFT)
            {
              /* the corner and side cursors count as crosshair, so leave
               * them and override everything else
               */
              cursor_type = PICMAN_CURSOR_CROSSHAIR_SMALL;
            }
          break;

        case PICMAN_CURSOR_MODE_CROSSHAIR:
          cursor_type = PICMAN_CURSOR_CROSSHAIR;
          tool_cursor = PICMAN_TOOL_CURSOR_NONE;

          if (modifier != PICMAN_CURSOR_MODIFIER_BAD)
            {
              /* the bad modifier is always shown */
              modifier = PICMAN_CURSOR_MODIFIER_NONE;
            }
          break;
        }
    }

  cursor_type = picman_cursor_rotate (cursor_type, shell->rotate_angle);

  cursor_handedness = PICMAN_GUI_CONFIG (shell->display->config)->cursor_handedness;

  if (shell->cursor_handedness != cursor_handedness ||
      shell->current_cursor    != cursor_type       ||
      shell->tool_cursor       != tool_cursor       ||
      shell->cursor_modifier   != modifier          ||
      always_install)
    {
      shell->cursor_handedness = cursor_handedness;
      shell->current_cursor    = cursor_type;
      shell->tool_cursor       = tool_cursor;
      shell->cursor_modifier   = modifier;

      picman_cursor_set (shell->canvas,
                       cursor_handedness,
                       cursor_type, tool_cursor, modifier);
    }
}
