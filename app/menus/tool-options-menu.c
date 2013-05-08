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

#include "libpicmanwidgets/picmanwidgets.h"

#include "menus-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanuimanager.h"

#include "tool-options-menu.h"


/*  local function prototypes  */

static void   tool_options_menu_update         (PicmanUIManager *manager,
                                                gpointer       update_data,
                                                const gchar   *ui_path);
static void   tool_options_menu_update_after   (PicmanUIManager *manager,
                                                gpointer       update_data,
                                                const gchar   *ui_path);
static void   tool_options_menu_update_presets (PicmanUIManager *manager,
                                                guint          merge_id,
                                                const gchar   *ui_path,
                                                const gchar   *menu_path,
                                                const gchar   *which_action,
                                                PicmanContainer *presets);


/*  public functions  */

void
tool_options_menu_setup (PicmanUIManager *manager,
                         const gchar   *ui_path)
{
  g_signal_connect (manager, "update",
                    G_CALLBACK (tool_options_menu_update),
                    (gpointer) ui_path);
  g_signal_connect_after (manager, "update",
                          G_CALLBACK (tool_options_menu_update_after),
                          (gpointer) ui_path);
}


/*  private functions  */

static void
tool_options_menu_update (PicmanUIManager *manager,
                          gpointer       update_data,
                          const gchar   *ui_path)
{
  guint merge_id;

  merge_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (manager),
                                                  "tool-options-merge-id"));

  if (merge_id)
    {
      gtk_ui_manager_remove_ui (GTK_UI_MANAGER (manager), merge_id);

      g_object_set_data (G_OBJECT (manager), "tool-options-merge-id",
                         GINT_TO_POINTER (0));

      gtk_ui_manager_ensure_update (GTK_UI_MANAGER (manager));
    }
}

static void
tool_options_menu_update_after (PicmanUIManager *manager,
                                gpointer       update_data,
                                const gchar   *ui_path)
{
  PicmanContext  *context;
  PicmanToolInfo *tool_info;
  guint         merge_id;

  context   = picman_get_user_context (manager->picman);
  tool_info = picman_context_get_tool (context);

  if (! tool_info->presets)
    return;

  merge_id = gtk_ui_manager_new_merge_id (GTK_UI_MANAGER (manager));

  g_object_set_data (G_OBJECT (manager), "tool-options-merge-id",
                     GUINT_TO_POINTER (merge_id));

  tool_options_menu_update_presets (manager, merge_id, ui_path,
                                    "Save", "save",
                                    tool_info->presets);

  tool_options_menu_update_presets (manager, merge_id, ui_path,
                                    "Restore", "restore",
                                    tool_info->presets);

  tool_options_menu_update_presets (manager, merge_id, ui_path,
                                    "Edit", "edit",
                                    tool_info->presets);

  tool_options_menu_update_presets (manager, merge_id, ui_path,
                                    "Delete", "delete",
                                    tool_info->presets);

  gtk_ui_manager_ensure_update (GTK_UI_MANAGER (manager));
}

static void
tool_options_menu_update_presets (PicmanUIManager *manager,
                                  guint          merge_id,
                                  const gchar   *ui_path,
                                  const gchar   *menu_path,
                                  const gchar   *which_action,
                                  PicmanContainer *presets)
{
  gint n_children;
  gint i;

  n_children = picman_container_get_n_children (presets);

  for (i = 0; i < n_children; i++)
    {
      gchar *action_name;
      gchar *path;

      action_name = g_strdup_printf ("tool-options-%s-preset-%03d",
                                     which_action, i);
      path        = g_strdup_printf ("%s/%s", ui_path, menu_path);

      gtk_ui_manager_add_ui (GTK_UI_MANAGER (manager), merge_id,
                             path, action_name, action_name,
                             GTK_UI_MANAGER_MENUITEM,
                             FALSE);

      g_free (action_name);
      g_free (path);
    }
}
