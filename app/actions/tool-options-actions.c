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

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanlist.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "tool-options-actions.h"
#include "tool-options-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void tool_options_actions_update_presets (PicmanActionGroup *group,
                                                 const gchar     *action_prefix,
                                                 GCallback        callback,
                                                 const gchar     *help_id,
                                                 PicmanContainer   *presets,
                                                 gboolean         need_writable,
                                                 gboolean         need_deletable);


/*  global variables  */

static const PicmanActionEntry tool_options_actions[] =
{
  { "tool-options-popup", PICMAN_STOCK_TOOL_OPTIONS,
    NC_("tool-options-action", "Tool Options Menu"), NULL, NULL, NULL,
    PICMAN_HELP_TOOL_OPTIONS_DIALOG },

  { "tool-options-save-preset-menu", GTK_STOCK_SAVE,
    NC_("tool-options-action", "_Save Tool Preset"), "", NULL, NULL,
    PICMAN_HELP_TOOL_OPTIONS_SAVE },

  { "tool-options-restore-preset-menu", GTK_STOCK_REVERT_TO_SAVED,
    NC_("tool-options-action", "_Restore Tool Preset"), "", NULL, NULL,
    PICMAN_HELP_TOOL_OPTIONS_RESTORE },

  { "tool-options-edit-preset-menu", GTK_STOCK_EDIT,
    NC_("tool-options-action", "E_dit Tool Preset"), NULL, NULL, NULL,
    PICMAN_HELP_TOOL_OPTIONS_EDIT },

  { "tool-options-delete-preset-menu", GTK_STOCK_DELETE,
    NC_("tool-options-action", "_Delete Tool Preset"), "", NULL, NULL,
    PICMAN_HELP_TOOL_OPTIONS_DELETE },

  { "tool-options-save-new-preset", GTK_STOCK_NEW,
    NC_("tool-options-action", "_New Tool Preset..."), "", NULL,
    G_CALLBACK (tool_options_save_new_preset_cmd_callback),
    PICMAN_HELP_TOOL_OPTIONS_SAVE },

  { "tool-options-reset", PICMAN_STOCK_RESET,
    NC_("tool-options-action", "R_eset Tool Options"), "",
    NC_("tool-options-action", "Reset to default values"),
    G_CALLBACK (tool_options_reset_cmd_callback),
    PICMAN_HELP_TOOL_OPTIONS_RESET },

  { "tool-options-reset-all", PICMAN_STOCK_RESET,
    NC_("tool-options-action", "Reset _all Tool Options"), "",
    NC_("tool-options-action", "Reset all tool options"),
    G_CALLBACK (tool_options_reset_all_cmd_callback),
    PICMAN_HELP_TOOL_OPTIONS_RESET }
};


/*  public functions  */

#define SET_VISIBLE(action,condition) \
        picman_action_group_set_action_visible (group, action, (condition) != 0)
#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_HIDE_EMPTY(action,condition) \
        picman_action_group_set_action_hide_empty (group, action, (condition) != 0)

void
tool_options_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "tool-options-action",
                                 tool_options_actions,
                                 G_N_ELEMENTS (tool_options_actions));

  SET_HIDE_EMPTY ("tool-options-restore-preset-menu", FALSE);
  SET_HIDE_EMPTY ("tool-options-edit-preset-menu",    FALSE);
  SET_HIDE_EMPTY ("tool-options-delete-preset-menu",  FALSE);
}

void
tool_options_actions_update (PicmanActionGroup *group,
                             gpointer         data)
{
  PicmanContext  *context   = picman_get_user_context (group->picman);
  PicmanToolInfo *tool_info = picman_context_get_tool (context);

  SET_VISIBLE ("tool-options-save-preset-menu",    tool_info->presets);
  SET_VISIBLE ("tool-options-restore-preset-menu", tool_info->presets);
  SET_VISIBLE ("tool-options-edit-preset-menu",    tool_info->presets);
  SET_VISIBLE ("tool-options-delete-preset-menu",  tool_info->presets);

  tool_options_actions_update_presets (group, "tool-options-save-preset",
                                       G_CALLBACK (tool_options_save_preset_cmd_callback),
                                       PICMAN_HELP_TOOL_OPTIONS_SAVE,
                                       tool_info->presets,
                                       TRUE /* writable */,
                                       FALSE /* deletable */);

  tool_options_actions_update_presets (group, "tool-options-restore-preset",
                                       G_CALLBACK (tool_options_restore_preset_cmd_callback),
                                       PICMAN_HELP_TOOL_OPTIONS_RESTORE,
                                       tool_info->presets,
                                       FALSE /* writable */,
                                       FALSE /* deletable */);

  tool_options_actions_update_presets (group, "tool-options-edit-preset",
                                       G_CALLBACK (tool_options_edit_preset_cmd_callback),
                                       PICMAN_HELP_TOOL_OPTIONS_EDIT,
                                       tool_info->presets,
                                       FALSE /* writable */,
                                       FALSE /* deletable */);

  tool_options_actions_update_presets (group, "tool-options-delete-preset",
                                       G_CALLBACK (tool_options_delete_preset_cmd_callback),
                                       PICMAN_HELP_TOOL_OPTIONS_DELETE,
                                       tool_info->presets,
                                       FALSE /* writable */,
                                       TRUE /* deletable */);
}


/*  private function  */

static void
tool_options_actions_update_presets (PicmanActionGroup *group,
                                     const gchar     *action_prefix,
                                     GCallback        callback,
                                     const gchar     *help_id,
                                     PicmanContainer   *presets,
                                     gboolean         need_writable,
                                     gboolean         need_deletable)
{
  GList *list;
  gint   n_children = 0;
  gint   i;

  for (i = 0; ; i++)
    {
      gchar     *action_name;
      GtkAction *action;

      action_name = g_strdup_printf ("%s-%03d", action_prefix, i);
      action = gtk_action_group_get_action (GTK_ACTION_GROUP (group),
                                            action_name);
      g_free (action_name);

      if (! action)
        break;

      gtk_action_group_remove_action (GTK_ACTION_GROUP (group), action);
    }

  if (presets)
    n_children = picman_container_get_n_children (presets);

  if (n_children > 0)
    {
      PicmanEnumActionEntry entry;

      entry.name           = NULL;
      entry.label          = NULL;
      entry.accelerator    = "";
      entry.tooltip        = NULL;
      entry.value          = 0;
      entry.value_variable = FALSE;
      entry.help_id        = help_id;

      for (list = PICMAN_LIST (presets)->list, i = 0;
           list;
           list = g_list_next (list), i++)
        {
          PicmanObject *preset = list->data;

          entry.name     = g_strdup_printf ("%s-%03d", action_prefix, i);
          entry.label    = picman_object_get_name (preset);
          entry.stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (preset));
          entry.value    = i;

          picman_action_group_add_enum_actions (group, NULL, &entry, 1, callback);

          if (need_writable)
            SET_SENSITIVE (entry.name,
                           picman_data_is_writable (PICMAN_DATA (preset)));

          if (need_deletable)
            SET_SENSITIVE (entry.name,
                           picman_data_is_deletable (PICMAN_DATA (preset)));

          g_free ((gchar *) entry.name);
        }
    }
}

#undef SET_VISIBLE
#undef SET_SENSITIVE
#undef SET_HIDE_EMPTY
