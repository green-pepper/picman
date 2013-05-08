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

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanerrorconsole.h"
#include "widgets/picmanhelp-ids.h"

#include "error-console-actions.h"
#include "error-console-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry error_console_actions[] =
{
  { "error-console-popup", PICMAN_STOCK_WARNING,
    NC_("error-console-action", "Error Console Menu"), NULL, NULL, NULL,
    PICMAN_HELP_ERRORS_DIALOG },

  { "error-console-clear", GTK_STOCK_CLEAR,
    NC_("error-console-action", "_Clear"), "",
    NC_("error-console-action", "Clear error console"),
    G_CALLBACK (error_console_clear_cmd_callback),
    PICMAN_HELP_ERRORS_CLEAR },

  { "error-console-select-all", NULL,
    NC_("error-console-action", "Select _All"), "",
    NC_("error-console-action", "Select all error messages"),
    G_CALLBACK (error_console_select_all_cmd_callback),
    PICMAN_HELP_ERRORS_SELECT_ALL }
};

static const PicmanEnumActionEntry error_console_save_actions[] =
{
  { "error-console-save-all", GTK_STOCK_SAVE_AS,
    NC_("error-console-action", "_Save Error Log to File..."), "",
    NC_("error-console-action", "Write all error messages to a file"),
    FALSE, FALSE,
    PICMAN_HELP_ERRORS_SAVE },

  { "error-console-save-selection", GTK_STOCK_SAVE_AS,
    NC_("error-console-action", "Save S_election to File..."), "",
    NC_("error-console-action", "Write the selected error messages to a file"),
    TRUE, FALSE,
    PICMAN_HELP_ERRORS_SAVE }
};


void
error_console_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "error-console-action",
                                 error_console_actions,
                                 G_N_ELEMENTS (error_console_actions));

  picman_action_group_add_enum_actions (group, "error-console-action",
                                      error_console_save_actions,
                                      G_N_ELEMENTS (error_console_save_actions),
                                      G_CALLBACK (error_console_save_cmd_callback));
}

void
error_console_actions_update (PicmanActionGroup *group,
                              gpointer         data)
{
  PicmanErrorConsole *console = PICMAN_ERROR_CONSOLE (data);
  gboolean          selection;

  selection = gtk_text_buffer_get_selection_bounds (console->text_buffer,
                                                    NULL, NULL);

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("error-console-clear",          TRUE);
  SET_SENSITIVE ("error-console-select-all",     TRUE);
  SET_SENSITIVE ("error-console-save-all",       TRUE);
  SET_SENSITIVE ("error-console-save-selection", selection);

#undef SET_SENSITIVE
}
