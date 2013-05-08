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

#include "libpicmanthumb/picmanthumb.h"

#include "menus-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanviewable.h"

#include "widgets/picmanaction.h"
#include "widgets/picmanuimanager.h"

#include "file-menu.h"


static gboolean file_menu_open_recent_query_tooltip (GtkWidget  *widget,
                                                     gint        x,
                                                     gint        y,
                                                     gboolean    keyboard_mode,
                                                     GtkTooltip *tooltip,
                                                     PicmanAction *action);


void
file_menu_setup (PicmanUIManager *manager,
                 const gchar   *ui_path)
{
  GtkUIManager *ui_manager;
  gint          n_entries;
  guint         merge_id;
  gint          i;

  g_return_if_fail (PICMAN_IS_UI_MANAGER (manager));
  g_return_if_fail (ui_path != NULL);

  ui_manager = GTK_UI_MANAGER (manager);

  n_entries = PICMAN_GUI_CONFIG (manager->picman->config)->last_opened_size;

  merge_id = gtk_ui_manager_new_merge_id (ui_manager);

  for (i = 0; i < n_entries; i++)
    {
      GtkWidget *widget;
      gchar     *action_name;
      gchar     *action_path;
      gchar     *full_path;

      action_name = g_strdup_printf ("file-open-recent-%02d", i + 1);
      action_path = g_strdup_printf ("%s/File/Open Recent/Files", ui_path);

      gtk_ui_manager_add_ui (ui_manager, merge_id,
                             action_path, action_name, action_name,
                             GTK_UI_MANAGER_MENUITEM,
                             FALSE);

      full_path = g_strconcat (action_path, "/", action_name, NULL);

      widget = gtk_ui_manager_get_widget (ui_manager, full_path);

      if (widget)
        {
          GtkAction *action;

          action = picman_ui_manager_find_action (manager, "file", action_name);

          g_signal_connect_object (widget, "query-tooltip",
                                   G_CALLBACK (file_menu_open_recent_query_tooltip),
                                   action, 0);
        }

      g_free (action_name);
      g_free (action_path);
      g_free (full_path);
    }
}

static gboolean
file_menu_open_recent_query_tooltip (GtkWidget  *widget,
                                     gint        x,
                                     gint        y,
                                     gboolean    keyboard_mode,
                                     GtkTooltip *tooltip,
                                     PicmanAction *action)
{
  gchar *text;

  text = gtk_widget_get_tooltip_text (widget);
  gtk_tooltip_set_text (tooltip, text);
  g_free (text);

  gtk_tooltip_set_icon (tooltip,
                        picman_viewable_get_pixbuf (action->viewable,
                                                  action->context,
                                                  PICMAN_THUMB_SIZE_NORMAL,
                                                  PICMAN_THUMB_SIZE_NORMAL));

  return TRUE;
}
