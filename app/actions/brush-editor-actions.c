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

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanbrusheditor.h"

#include "brush-editor-actions.h"
#include "data-editor-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry brush_editor_actions[] =
{
  { "brush-editor-popup", PICMAN_STOCK_BRUSH,
    NC_("brush-editor-action", "Brush Editor Menu"), NULL, NULL, NULL,
    PICMAN_HELP_BRUSH_EDITOR_DIALOG }
};

static const PicmanToggleActionEntry brush_editor_toggle_actions[] =
{
  { "brush-editor-edit-active", PICMAN_STOCK_LINKED,
    NC_("brush-editor-action", "Edit Active Brush"), NULL, NULL,
    G_CALLBACK (data_editor_edit_active_cmd_callback),
    FALSE,
    PICMAN_HELP_BRUSH_EDITOR_EDIT_ACTIVE }
};


void
brush_editor_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "brush-editor-action",
                                 brush_editor_actions,
                                 G_N_ELEMENTS (brush_editor_actions));

  picman_action_group_add_toggle_actions (group, "brush-editor-action",
                                        brush_editor_toggle_actions,
                                        G_N_ELEMENTS (brush_editor_toggle_actions));
}

void
brush_editor_actions_update (PicmanActionGroup *group,
                             gpointer         user_data)
{
  PicmanDataEditor  *data_editor = PICMAN_DATA_EDITOR (user_data);
  gboolean         edit_active = FALSE;

  edit_active = picman_data_editor_get_edit_active (data_editor);

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  SET_ACTIVE ("brush-editor-edit-active", edit_active);

#undef SET_SENSITIVE
#undef SET_ACTIVE
}
