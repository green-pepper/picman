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

#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmandisplayconfig.h"
#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "dialogs/dialogs.h"

#include "actions.h"
#include "dialogs-actions.h"
#include "windows-commands.h"

#include "picman-intl.h"


void
windows_hide_docks_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  gboolean  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  Picman     *picman;
  return_if_no_picman (picman, data);

  if (PICMAN_GUI_CONFIG (picman->config)->hide_docks == active)
    return;

  g_object_set (picman->config,
                "hide-docks", active,
                NULL);
}

void
windows_use_single_window_mode_cmd_callback (GtkAction *action,
                                             gpointer   data)
{
  gboolean  active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  Picman     *picman;
  return_if_no_picman (picman, data);

  if (PICMAN_GUI_CONFIG (picman->config)->single_window_mode == active)
    return;

  g_object_set (picman->config,
                "single-window-mode", active,
                NULL);
}

void
windows_show_display_next_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanDisplay *display;
  Picman        *picman;
  gint         index;
  return_if_no_display (display, data);
  return_if_no_picman (picman, data);

  index = picman_container_get_child_index (picman->displays,
                                          PICMAN_OBJECT (display));
  index++;

  if (index >= picman_container_get_n_children (picman->displays))
    index = 0;

  display = PICMAN_DISPLAY (picman_container_get_child_by_index (picman->displays,
                                                             index));
  picman_display_shell_present (picman_display_get_shell (display));
}

void
windows_show_display_previous_cmd_callback (GtkAction *action,
                                            gpointer   data)
{
  PicmanDisplay *display;
  Picman        *picman;
  gint         index;
  return_if_no_display (display, data);
  return_if_no_picman (picman, data);

  index = picman_container_get_child_index (picman->displays,
                                          PICMAN_OBJECT (display));
  index--;

  if (index < 0)
    index = picman_container_get_n_children (picman->displays) - 1;

  display = PICMAN_DISPLAY (picman_container_get_child_by_index (picman->displays,
                                                             index));
  picman_display_shell_present (picman_display_get_shell (display));
}

void
windows_show_display_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanDisplay *display = g_object_get_data (G_OBJECT (action), "display");

  picman_display_shell_present (picman_display_get_shell (display));
}

void
windows_show_dock_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  GtkWindow *dock_window = g_object_get_data (G_OBJECT (action), "dock-window");

  gtk_window_present (dock_window);
}

void
windows_open_recent_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanSessionInfo        *info;
  PicmanDialogFactoryEntry *entry;
  Picman                   *picman;
  return_if_no_picman (picman, data);

  info  = g_object_get_data (G_OBJECT (action), "info");
  entry = picman_session_info_get_factory_entry (info);

  if (entry && strcmp ("picman-toolbox-window", entry->identifier) == 0 &&
      dialogs_actions_toolbox_exists (picman))
    {
      picman_message (picman,
                    G_OBJECT (action_data_get_widget (data)),
                    PICMAN_MESSAGE_WARNING,
                    _("The chosen recent dock contains a toolbox. Please "
                      "close the currently open toolbox and try again."));
      return;
    }

  g_object_ref (info);

  picman_container_remove (global_recent_docks, PICMAN_OBJECT (info));

  picman_dialog_factory_add_session_info (picman_dialog_factory_get_singleton (),
                                        info);

  picman_session_info_restore (info, picman_dialog_factory_get_singleton ());

  g_object_unref (info);
}
