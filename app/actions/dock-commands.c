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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "widgets/picmandockwindow.h"
#include "widgets/picmandockwindow.h"

#include "actions.h"
#include "dock-commands.h"


static PicmanDockWindow *
dock_commands_get_dock_window_from_widget (GtkWidget *widget)
{
  GtkWidget      *toplevel    = gtk_widget_get_toplevel (widget);
  PicmanDockWindow *dock_window = NULL;

  if (PICMAN_IS_DOCK_WINDOW (toplevel))
    dock_window = PICMAN_DOCK_WINDOW (toplevel);

  return dock_window;
}


/*  public functions  */

void
dock_toggle_image_menu_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  GtkWidget      *widget      = NULL;
  PicmanDockWindow *dock_window = NULL;
  return_if_no_widget (widget, data);

  dock_window = dock_commands_get_dock_window_from_widget (widget);

  if (dock_window)
    {
      gboolean active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
      picman_dock_window_set_show_image_menu (dock_window, active);
    }
}

void
dock_toggle_auto_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  GtkWidget      *widget      = NULL;
  PicmanDockWindow *dock_window = NULL;
  return_if_no_widget (widget, data);

  dock_window = dock_commands_get_dock_window_from_widget (widget);

  if (dock_window)
    {
      gboolean active = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
      picman_dock_window_set_auto_follow_active (dock_window, active);
    }
}
