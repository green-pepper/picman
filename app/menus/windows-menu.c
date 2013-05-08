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
#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanthumb/picmanthumb.h"

#include "menus-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanlist.h"
#include "core/picmanviewable.h"

#include "widgets/picmanaction.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanuimanager.h"

#include "display/picmandisplay.h"

#include "dialogs/dialogs.h"

#include "actions/windows-actions.h"

#include "windows-menu.h"


static void      windows_menu_display_add                (PicmanContainer     *container,
                                                          PicmanDisplay       *display,
                                                          PicmanUIManager     *manager);
static void      windows_menu_display_remove             (PicmanContainer     *container,
                                                          PicmanDisplay       *display,
                                                          PicmanUIManager     *manager);
static void      windows_menu_image_notify               (PicmanDisplay       *display,
                                                          const GParamSpec  *unused,
                                                          PicmanUIManager     *manager);
static void      windows_menu_dock_window_added          (PicmanDialogFactory *factory,
                                                          PicmanDockWindow    *dock_window,
                                                          PicmanUIManager     *manager);
static void      windows_menu_dock_window_removed        (PicmanDialogFactory *factory,
                                                          PicmanDockWindow    *dock_window,
                                                          PicmanUIManager     *manager);
static gchar   * windows_menu_dock_window_to_merge_id    (PicmanDockWindow    *dock_window);
static void      windows_menu_recent_add                 (PicmanContainer     *container,
                                                          PicmanSessionInfo   *info,
                                                          PicmanUIManager     *manager);
static void      windows_menu_recent_remove              (PicmanContainer     *container,
                                                          PicmanSessionInfo   *info,
                                                          PicmanUIManager     *manager);
static gboolean  windows_menu_display_query_tooltip      (GtkWidget         *widget,
                                                          gint               x,
                                                          gint               y,
                                                          gboolean           keyboard_mode,
                                                          GtkTooltip        *tooltip,
                                                          PicmanAction        *action);


void
windows_menu_setup (PicmanUIManager *manager,
                    const gchar   *ui_path)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_UI_MANAGER (manager));
  g_return_if_fail (ui_path != NULL);

  g_object_set_data (G_OBJECT (manager), "image-menu-ui-path",
                     (gpointer) ui_path);

  g_signal_connect_object (manager->picman->displays, "add",
                           G_CALLBACK (windows_menu_display_add),
                           manager, 0);
  g_signal_connect_object (manager->picman->displays, "remove",
                           G_CALLBACK (windows_menu_display_remove),
                           manager, 0);

  for (list = picman_get_display_iter (manager->picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;

      windows_menu_display_add (manager->picman->displays, display, manager);
    }

  g_signal_connect_object (picman_dialog_factory_get_singleton (), "dock-window-added",
                           G_CALLBACK (windows_menu_dock_window_added),
                           manager, 0);
  g_signal_connect_object (picman_dialog_factory_get_singleton (), "dock-window-removed",
                           G_CALLBACK (windows_menu_dock_window_removed),
                           manager, 0);

  for (list = picman_dialog_factory_get_open_dialogs (picman_dialog_factory_get_singleton ());
       list;
       list = g_list_next (list))
    {
      PicmanDockWindow *dock_window = list->data;

      if (PICMAN_IS_DOCK_WINDOW (dock_window))
        windows_menu_dock_window_added (picman_dialog_factory_get_singleton (),
                                        dock_window,
                                        manager);
    }

  g_signal_connect_object (global_recent_docks, "add",
                           G_CALLBACK (windows_menu_recent_add),
                           manager, 0);
  g_signal_connect_object (global_recent_docks, "remove",
                           G_CALLBACK (windows_menu_recent_remove),
                           manager, 0);

  for (list = g_list_last (PICMAN_LIST (global_recent_docks)->list);
       list;
       list = g_list_previous (list))
    {
      PicmanSessionInfo *info = list->data;

      windows_menu_recent_add (global_recent_docks, info, manager);
    }
}


/*  private functions  */

static void
windows_menu_display_add (PicmanContainer *container,
                          PicmanDisplay   *display,
                          PicmanUIManager *manager)
{
  g_signal_connect_object (display, "notify::image",
                           G_CALLBACK (windows_menu_image_notify),
                           manager, 0);

  if (picman_display_get_image (display))
    windows_menu_image_notify (display, NULL, manager);
}

static void
windows_menu_display_remove (PicmanContainer *container,
                             PicmanDisplay   *display,
                             PicmanUIManager *manager)
{
  gchar *merge_key = g_strdup_printf ("windows-display-%04d-merge-id",
                                      picman_display_get_ID (display));
  guint  merge_id;

  merge_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (manager),
                                                  merge_key));

  if (merge_id)
    gtk_ui_manager_remove_ui (GTK_UI_MANAGER (manager), merge_id);

  g_object_set_data (G_OBJECT (manager), merge_key, NULL);

  g_free (merge_key);
}

static void
windows_menu_image_notify (PicmanDisplay      *display,
                           const GParamSpec *unused,
                           PicmanUIManager    *manager)
{
  if (picman_display_get_image (display))
    {
      gchar *merge_key = g_strdup_printf ("windows-display-%04d-merge-id",
                                          picman_display_get_ID (display));
      guint  merge_id;

      merge_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (manager),
                                                      merge_key));

      if (! merge_id)
        {
          GtkWidget   *widget;
          const gchar *ui_path;
          gchar       *action_name;
          gchar       *action_path;
          gchar       *full_path;

          ui_path = g_object_get_data (G_OBJECT (manager),
                                       "image-menu-ui-path");

          action_name = picman_display_get_action_name (display);
          action_path = g_strdup_printf ("%s/Windows/Images", ui_path);

          merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

          g_object_set_data (G_OBJECT (manager), merge_key,
                             GUINT_TO_POINTER (merge_id));

          gtk_ui_manager_add_ui (GTK_UI_MANAGER (manager), merge_id,
                                 action_path, action_name, action_name,
                                 GTK_UI_MANAGER_MENUITEM,
                                 FALSE);

          full_path = g_strconcat (action_path, "/", action_name, NULL);

          widget = gtk_ui_manager_get_widget (GTK_UI_MANAGER (manager),
                                              full_path);

          if (widget)
            {
              GtkAction *action;

              action = picman_ui_manager_find_action (manager,
                                                    "windows", action_name);

              g_signal_connect_object (widget, "query-tooltip",
                                       G_CALLBACK (windows_menu_display_query_tooltip),
                                       action, 0);
            }

          g_free (action_name);
          g_free (action_path);
          g_free (full_path);
        }

      g_free (merge_key);
    }
  else
    {
      windows_menu_display_remove (manager->picman->displays, display, manager);
    }
}

static void
windows_menu_dock_window_added (PicmanDialogFactory *factory,
                                PicmanDockWindow    *dock_window,
                                PicmanUIManager     *manager)
{
  const gchar *ui_path;
  gchar       *action_name;
  gchar       *action_path;
  gchar       *merge_key;
  guint        merge_id;

  ui_path = g_object_get_data (G_OBJECT (manager), "image-menu-ui-path");

  action_name = windows_actions_dock_window_to_action_name (dock_window);
  action_path = g_strdup_printf ("%s/Windows/Docks",
                                 ui_path);

  merge_key = windows_menu_dock_window_to_merge_id (dock_window);
  merge_id  = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

  g_object_set_data (G_OBJECT (manager), merge_key,
                     GUINT_TO_POINTER (merge_id));

  gtk_ui_manager_add_ui (GTK_UI_MANAGER (manager), merge_id,
                         action_path, action_name, action_name,
                         GTK_UI_MANAGER_MENUITEM,
                         FALSE);

  g_free (merge_key);
  g_free (action_path);
  g_free (action_name);
}

static void
windows_menu_dock_window_removed (PicmanDialogFactory *factory,
                                  PicmanDockWindow    *dock_window,
                                  PicmanUIManager     *manager)
{
  gchar *merge_key = windows_menu_dock_window_to_merge_id (dock_window);
  guint  merge_id  = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (manager),
                                                          merge_key));
  if (merge_id)
    gtk_ui_manager_remove_ui (GTK_UI_MANAGER (manager), merge_id);

  g_object_set_data (G_OBJECT (manager), merge_key, NULL);

  g_free (merge_key);
}

static gchar *
windows_menu_dock_window_to_merge_id (PicmanDockWindow *dock_window)
{
  return g_strdup_printf ("windows-dock-%04d-merge-id",
                          picman_dock_window_get_id (dock_window));
}

static void
windows_menu_recent_add (PicmanContainer   *container,
                         PicmanSessionInfo *info,
                         PicmanUIManager   *manager)
{
  const gchar *ui_path;
  gchar       *action_name;
  gchar       *action_path;
  gint         info_id;
  gchar       *merge_key;
  guint        merge_id;

  ui_path = g_object_get_data (G_OBJECT (manager), "image-menu-ui-path");

  info_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (info),
                                                "recent-action-id"));

  action_name = g_strdup_printf ("windows-recent-%04d", info_id);
  action_path = g_strdup_printf ("%s/Windows/Recently Closed Docks", ui_path);

  merge_key = g_strdup_printf ("windows-recent-%04d-merge-id", info_id);
  merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

  g_object_set_data (G_OBJECT (manager), merge_key,
                     GUINT_TO_POINTER (merge_id));

  gtk_ui_manager_add_ui (GTK_UI_MANAGER (manager), merge_id,
                         action_path, action_name, action_name,
                         GTK_UI_MANAGER_MENUITEM,
                         TRUE);

  g_free (merge_key);
  g_free (action_path);
  g_free (action_name);
}

static void
windows_menu_recent_remove (PicmanContainer   *container,
                            PicmanSessionInfo *info,
                            PicmanUIManager   *manager)
{
  gint   info_id;
  gchar *merge_key;
  guint  merge_id;

  info_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (info),
                                                "recent-action-id"));

  merge_key = g_strdup_printf ("windows-recent-%04d-merge-id", info_id);

  merge_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (manager),
                                                  merge_key));

  if (merge_id)
    gtk_ui_manager_remove_ui (GTK_UI_MANAGER (manager), merge_id);

  g_object_set_data (G_OBJECT (manager), merge_key, NULL);

  g_free (merge_key);
}

static gboolean
windows_menu_display_query_tooltip (GtkWidget  *widget,
                                    gint        x,
                                    gint        y,
                                    gboolean    keyboard_mode,
                                    GtkTooltip *tooltip,
                                    PicmanAction *action)
{
  PicmanImage *image = PICMAN_IMAGE (action->viewable);
  gchar     *text;
  gdouble    xres;
  gdouble    yres;
  gint       width;
  gint       height;

  text = gtk_widget_get_tooltip_text (widget);
  gtk_tooltip_set_text (tooltip, text);
  g_free (text);

  picman_image_get_resolution (image, &xres, &yres);

  picman_viewable_calc_preview_size (picman_image_get_width  (image),
                                   picman_image_get_height (image),
                                   PICMAN_VIEW_SIZE_HUGE, PICMAN_VIEW_SIZE_HUGE,
                                   FALSE, xres, yres,
                                   &width, &height, NULL);

  gtk_tooltip_set_icon (tooltip,
                        picman_viewable_get_pixbuf (action->viewable,
                                                  action->context,
                                                  width, height));

  return TRUE;
}
