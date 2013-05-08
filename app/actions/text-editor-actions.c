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
#include "widgets/picmantexteditor.h"
#include "widgets/picmanhelp-ids.h"

#include "text-editor-actions.h"
#include "text-editor-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry text_editor_actions[] =
{
  { "text-editor-toolbar", GTK_STOCK_EDIT,
    "Text Editor Toolbar", NULL, NULL, NULL,
    PICMAN_HELP_TEXT_EDITOR_DIALOG },

  { "text-editor-load", GTK_STOCK_OPEN,
    NC_("text-editor-action", "Open"), "",
    NC_("text-editor-action", "Load text from file"),
    G_CALLBACK (text_editor_load_cmd_callback),
    NULL },

  { "text-editor-clear", GTK_STOCK_CLEAR,
    NC_("text-editor-action", "Clear"), "",
    NC_("text-editor-action", "Clear all text"),
    G_CALLBACK (text_editor_clear_cmd_callback),
    NULL }
};

static const PicmanRadioActionEntry text_editor_direction_actions[] =
{
  { "text-editor-direction-ltr", PICMAN_STOCK_TEXT_DIR_LTR,
    NC_("text-editor-action", "LTR"), "",
    NC_("text-editor-action", "From left to right"),
    PICMAN_TEXT_DIRECTION_LTR,
    NULL },

  { "text-editor-direction-rtl", PICMAN_STOCK_TEXT_DIR_RTL,
    NC_("text-editor-action", "RTL"), "",
    NC_("text-editor-action", "From right to left"),
    PICMAN_TEXT_DIRECTION_RTL,
    NULL }
};


void
text_editor_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "text-editor-action",
                                 text_editor_actions,
                                 G_N_ELEMENTS (text_editor_actions));

  picman_action_group_add_radio_actions (group, "text-editor-action",
                                       text_editor_direction_actions,
                                       G_N_ELEMENTS (text_editor_direction_actions),
                                       NULL,
                                       PICMAN_TEXT_DIRECTION_LTR,
                                       G_CALLBACK (text_editor_direction_cmd_callback));
}

void
text_editor_actions_update (PicmanActionGroup *group,
                            gpointer         data)
{
  PicmanTextEditor *editor = PICMAN_TEXT_EDITOR (data);

#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  switch (editor->base_dir)
    {
    case PICMAN_TEXT_DIRECTION_LTR:
      SET_ACTIVE ("text-editor-direction-ltr", TRUE);
      break;

    case PICMAN_TEXT_DIRECTION_RTL:
      SET_ACTIVE ("text-editor-direction-rtl", TRUE);
      break;
    }

#undef SET_ACTIVE
}
