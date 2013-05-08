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
#include <gdk/gdkkeysyms.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmandisplayconfig.h"
#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmanlist.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockwindow.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "dialogs/dialogs.h"

#include "windows-actions.h"
#include "windows-commands.h"

#include "picman-intl.h"


static void  windows_actions_display_add               (PicmanContainer     *container,
                                                        PicmanDisplay       *display,
                                                        PicmanActionGroup   *group);
static void  windows_actions_display_remove            (PicmanContainer     *container,
                                                        PicmanDisplay       *display,
                                                        PicmanActionGroup   *group);
static void  windows_actions_image_notify              (PicmanDisplay       *display,
                                                        const GParamSpec  *unused,
                                                        PicmanActionGroup   *group);
static void  windows_actions_update_display_accels     (PicmanActionGroup   *group);

static void  windows_actions_dock_window_added         (PicmanDialogFactory *factory,
                                                        PicmanDockWindow    *dock_window,
                                                        PicmanActionGroup   *group);
static void  windows_actions_dock_window_removed       (PicmanDialogFactory *factory,
                                                        PicmanDockWindow    *dock_window,
                                                        PicmanActionGroup   *group);
static void  windows_actions_dock_window_notify        (PicmanDockWindow    *dock,
                                                        const GParamSpec  *pspec,
                                                        PicmanActionGroup   *group);
static void  windows_actions_recent_add                (PicmanContainer     *container,
                                                        PicmanSessionInfo   *info,
                                                        PicmanActionGroup   *group);
static void  windows_actions_recent_remove             (PicmanContainer     *container,
                                                        PicmanSessionInfo   *info,
                                                        PicmanActionGroup   *group);
static void  windows_actions_single_window_mode_notify (PicmanDisplayConfig *config,
                                                        GParamSpec        *pspec,
                                                        PicmanActionGroup   *group);


/* The only reason we have "Tab" in the action entries below is to
 * give away the hardcoded keyboard shortcut. If the user changes the
 * shortcut to something else, both that shortcut and Tab will
 * work. The reason we have the shortcut hardcoded is because
 * gtk_accelerator_valid() returns FALSE for GDK_tab.
 */

static const PicmanActionEntry windows_actions[] =
{
  { "windows-menu",         NULL, NC_("windows-action",
                                      "_Windows")               },
  { "windows-docks-menu",   NULL, NC_("windows-action",
                                      "_Recently Closed Docks") },
  { "windows-dialogs-menu", NULL, NC_("windows-action",
                                      "_Dockable Dialogs")      },

  { "windows-show-display-next", NULL,
    NC_("windows-action", "Next Image"), "<alt>Tab",
    NC_("windows-action", "Switch to the next image"),
    G_CALLBACK (windows_show_display_next_cmd_callback),
    NULL },

  { "windows-show-display-previous", NULL,
    NC_("windows-action", "Previous Image"), "<alt><shift>Tab",
    NC_("windows-action", "Switch to the previous image"),
    G_CALLBACK (windows_show_display_previous_cmd_callback),
    NULL }
};

static const PicmanToggleActionEntry windows_toggle_actions[] =
{
  { "windows-hide-docks", NULL,
    NC_("windows-action", "Hide Docks"), "Tab",
    NC_("windows-action", "When enabled docks and other dialogs are hidden, leaving only image windows."),
    G_CALLBACK (windows_hide_docks_cmd_callback),
    FALSE,
    PICMAN_HELP_WINDOWS_HIDE_DOCKS },

  { "windows-use-single-window-mode", NULL,
    NC_("windows-action", "Single-Window Mode"), NULL,
    NC_("windows-action", "When enabled PICMAN is in a single-window mode."),
    G_CALLBACK (windows_use_single_window_mode_cmd_callback),
    FALSE,
    PICMAN_HELP_WINDOWS_USE_SINGLE_WINDOW_MODE }
};


void
windows_actions_setup (PicmanActionGroup *group)
{
  GList *list;

  picman_action_group_add_actions (group, "windows-action",
                                 windows_actions,
                                 G_N_ELEMENTS (windows_actions));

  picman_action_group_add_toggle_actions (group, "windows-action",
                                        windows_toggle_actions,
                                        G_N_ELEMENTS (windows_toggle_actions));

  picman_action_group_set_action_hide_empty (group, "windows-docks-menu", FALSE);

  g_signal_connect_object (group->picman->displays, "add",
                           G_CALLBACK (windows_actions_display_add),
                           group, 0);
  g_signal_connect_object (group->picman->displays, "remove",
                           G_CALLBACK (windows_actions_display_remove),
                           group, 0);

  for (list = picman_get_display_iter (group->picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;

      windows_actions_display_add (group->picman->displays, display, group);
    }

  g_signal_connect_object (picman_dialog_factory_get_singleton (), "dock-window-added",
                           G_CALLBACK (windows_actions_dock_window_added),
                           group, 0);
  g_signal_connect_object (picman_dialog_factory_get_singleton (), "dock-window-removed",
                           G_CALLBACK (windows_actions_dock_window_removed),
                           group, 0);

  for (list = picman_dialog_factory_get_open_dialogs (picman_dialog_factory_get_singleton ());
       list;
       list = g_list_next (list))
    {
      PicmanDockWindow *dock_window = list->data;

      if (PICMAN_IS_DOCK_WINDOW (dock_window))
        windows_actions_dock_window_added (picman_dialog_factory_get_singleton (),
                                           dock_window,
                                           group);
    }

  g_signal_connect_object (global_recent_docks, "add",
                           G_CALLBACK (windows_actions_recent_add),
                           group, 0);
  g_signal_connect_object (global_recent_docks, "remove",
                           G_CALLBACK (windows_actions_recent_remove),
                           group, 0);

  for (list = PICMAN_LIST (global_recent_docks)->list;
       list;
       list = g_list_next (list))
    {
      PicmanSessionInfo *info = list->data;

      windows_actions_recent_add (global_recent_docks, info, group);
    }

  g_signal_connect_object (group->picman->config, "notify::single-window-mode",
                           G_CALLBACK (windows_actions_single_window_mode_notify),
                           group, 0);
}

void
windows_actions_update (PicmanActionGroup *group,
                        gpointer         data)
{
  PicmanGuiConfig *config = PICMAN_GUI_CONFIG (group->picman->config);

#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  SET_ACTIVE ("windows-use-single-window-mode", config->single_window_mode);
  SET_ACTIVE ("windows-hide-docks", config->hide_docks);

#undef SET_ACTIVE
}

gchar *
windows_actions_dock_window_to_action_name (PicmanDockWindow *dock_window)
{
  return g_strdup_printf ("windows-dock-%04d",
                          picman_dock_window_get_id (dock_window));
}


/*  private functions  */

static void
windows_actions_display_add (PicmanContainer   *container,
                             PicmanDisplay     *display,
                             PicmanActionGroup *group)
{
  g_signal_connect_object (display, "notify::image",
                           G_CALLBACK (windows_actions_image_notify),
                           group, 0);

  if (picman_display_get_image (display))
    windows_actions_image_notify (display, NULL, group);
}

static void
windows_actions_display_remove (PicmanContainer   *container,
                                PicmanDisplay     *display,
                                PicmanActionGroup *group)
{
  GtkAction *action;
  gchar     *action_name = picman_display_get_action_name (display);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (action)
    gtk_action_group_remove_action (GTK_ACTION_GROUP (group), action);

  g_free (action_name);

  windows_actions_update_display_accels (group);
}

static void
windows_actions_image_notify (PicmanDisplay      *display,
                              const GParamSpec *unused,
                              PicmanActionGroup  *group)
{
  PicmanImage *image = picman_display_get_image (display);

  if (image)
    {
      GtkAction *action;
      gchar     *action_name = picman_display_get_action_name (display);

      action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                            action_name);

      if (! action)
        {
          PicmanActionEntry entry;

          entry.name        = action_name;
          entry.stock_id    = PICMAN_STOCK_IMAGE;
          entry.label       = "";
          entry.accelerator = NULL;
          entry.tooltip     = NULL;
          entry.callback    = G_CALLBACK (windows_show_display_cmd_callback);
          entry.help_id     = NULL;

          picman_action_group_add_actions (group, NULL, &entry, 1);

          picman_action_group_set_action_always_show_image (group, action_name,
                                                          TRUE);

          action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                                action_name);

          g_object_set_data (G_OBJECT (action), "display", display);
        }

      {
        const gchar *display_name;
        gchar       *escaped;
        gchar       *title;

        display_name = picman_image_get_display_name (image);
        escaped = picman_escape_uline (display_name);

        title = g_strdup_printf ("%s-%d.%d", escaped,
                                 picman_image_get_ID (image),
                                 picman_display_get_instance (display));
        g_free (escaped);

        g_object_set (action,
                      "label",    title,
                      "tooltip",  picman_image_get_display_path (image),
                      "viewable", image,
                      "context",  picman_get_user_context (group->picman),
                      NULL);

        g_free (title);
      }

      g_free (action_name);

      windows_actions_update_display_accels (group);
    }
  else
    {
      windows_actions_display_remove (group->picman->displays, display, group);
    }
}

static void
windows_actions_update_display_accels (PicmanActionGroup *group)
{
  GList *list;
  gint   i;

  for (list = picman_get_display_iter (group->picman), i = 0;
       list && i < 10;
       list = g_list_next (list), i++)
    {
      PicmanDisplay *display = list->data;
      GtkAction   *action;
      gchar       *action_name;

      if (! picman_display_get_image (display))
        break;

      action_name = picman_display_get_action_name (display);

      action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                            action_name);
      g_free (action_name);

      if (action)
        {
          const gchar *accel_path;
          guint        accel_key;

          accel_path = gtk_action_get_accel_path (action);

          if (i < 9)
            accel_key = GDK_KEY_1 + i;
          else
            accel_key = GDK_KEY_0;

          gtk_accel_map_change_entry (accel_path,
                                      accel_key, GDK_MOD1_MASK,
                                      TRUE);
        }
    }
}

static void
windows_actions_dock_window_added (PicmanDialogFactory *factory,
                                   PicmanDockWindow    *dock_window,
                                   PicmanActionGroup   *group)
{
  GtkAction       *action;
  PicmanActionEntry  entry;
  gchar           *action_name = windows_actions_dock_window_to_action_name (dock_window);

  entry.name        = action_name;
  entry.stock_id    = NULL;
  entry.label       = "";
  entry.accelerator = NULL;
  entry.tooltip     = NULL;
  entry.callback    = G_CALLBACK (windows_show_dock_cmd_callback);
  entry.help_id     = PICMAN_HELP_WINDOWS_SHOW_DOCK;

  picman_action_group_add_actions (group, NULL, &entry, 1);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                        action_name);

  g_object_set (action,
                "ellipsize", PANGO_ELLIPSIZE_END,
                NULL);

  g_object_set_data (G_OBJECT (action), "dock-window", dock_window);

  g_free (action_name);

  g_signal_connect_object (dock_window, "notify::title",
                           G_CALLBACK (windows_actions_dock_window_notify),
                           group, 0);

  if (gtk_window_get_title (GTK_WINDOW (dock_window)))
    windows_actions_dock_window_notify (dock_window, NULL, group);
}

static void
windows_actions_dock_window_removed (PicmanDialogFactory *factory,
                                     PicmanDockWindow    *dock_window,
                                     PicmanActionGroup   *group)
{
  GtkAction *action;
  gchar     *action_name = windows_actions_dock_window_to_action_name (dock_window);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (action)
    gtk_action_group_remove_action (GTK_ACTION_GROUP (group), action);

  g_free (action_name);
}

static void
windows_actions_dock_window_notify (PicmanDockWindow   *dock_window,
                                    const GParamSpec *pspec,
                                    PicmanActionGroup  *group)
{
  GtkAction *action;
  gchar     *action_name;

  action_name = windows_actions_dock_window_to_action_name (dock_window);
  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);
  g_free (action_name);

  if (action)
    g_object_set (action,
                  "label",   gtk_window_get_title (GTK_WINDOW (dock_window)),
                  "tooltip", gtk_window_get_title (GTK_WINDOW (dock_window)),
                  NULL);
}

static void
windows_actions_recent_add (PicmanContainer   *container,
                            PicmanSessionInfo *info,
                            PicmanActionGroup *group)
{
  GtkAction       *action;
  PicmanActionEntry  entry;
  gint             info_id;
  static gint      info_id_counter = 1;
  gchar           *action_name;

  info_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (info),
                                                "recent-action-id"));

  if (! info_id)
    {
      info_id = info_id_counter++;

      g_object_set_data (G_OBJECT (info), "recent-action-id",
                         GINT_TO_POINTER (info_id));
    }

  action_name = g_strdup_printf ("windows-recent-%04d", info_id);

  entry.name        = action_name;
  entry.stock_id    = NULL;
  entry.label       = picman_object_get_name (info);
  entry.accelerator = NULL;
  entry.tooltip     = picman_object_get_name (info);
  entry.callback    = G_CALLBACK (windows_open_recent_cmd_callback);
  entry.help_id     = PICMAN_HELP_WINDOWS_OPEN_RECENT_DOCK;

  picman_action_group_add_actions (group, NULL, &entry, 1);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                        action_name);

  g_object_set (action,
                "ellipsize",       PANGO_ELLIPSIZE_END,
                "max-width-chars", 30,
                NULL);

  g_object_set_data (G_OBJECT (action), "info", info);

  g_free (action_name);
}

static void
windows_actions_recent_remove (PicmanContainer   *container,
                               PicmanSessionInfo *info,
                               PicmanActionGroup *group)
{
  GtkAction *action;
  gint       info_id;
  gchar     *action_name;

  info_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (info),
                                                "recent-action-id"));

  action_name = g_strdup_printf ("windows-recent-%04d", info_id);

  action = gtk_action_group_get_action (GTK_ACTION_GROUP (group), action_name);

  if (action)
    gtk_action_group_remove_action (GTK_ACTION_GROUP (group), action);

  g_free (action_name);
}

static void
windows_actions_single_window_mode_notify (PicmanDisplayConfig *config,
                                           GParamSpec        *pspec,
                                           PicmanActionGroup   *group)
{
  picman_action_group_set_action_active (group,
                                       "windows-use-single-window-mode",
                                       PICMAN_GUI_CONFIG (config)->single_window_mode);
}
