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

#include "core/picmancontext.h"
#include "core/picmandata.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "data-commands.h"
#include "patterns-actions.h"

#include "picman-intl.h"


static const PicmanActionEntry patterns_actions[] =
{
  { "patterns-popup", PICMAN_STOCK_PATTERN,
    NC_("patterns-action", "Patterns Menu"), NULL, NULL, NULL,
    PICMAN_HELP_PATTERN_DIALOG },

  { "patterns-open-as-image", GTK_STOCK_OPEN,
    NC_("patterns-action", "_Open Pattern as Image"), "",
    NC_("patterns-action", "Open this pattern as an image"),
    G_CALLBACK (data_open_as_image_cmd_callback),
    PICMAN_HELP_PATTERN_OPEN_AS_IMAGE },

  { "patterns-new", GTK_STOCK_NEW,
    NC_("patterns-action", "_New Pattern"), "",
    NC_("patterns-action", "Create a new pattern"),
    G_CALLBACK (data_new_cmd_callback),
    PICMAN_HELP_PATTERN_NEW },

  { "patterns-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("patterns-action", "D_uplicate Pattern"), NULL,
    NC_("patterns-action", "Duplicate this pattern"),
    G_CALLBACK (data_duplicate_cmd_callback),
    PICMAN_HELP_PATTERN_DUPLICATE },

  { "patterns-copy-location", GTK_STOCK_COPY,
    NC_("patterns-action", "Copy Pattern _Location"), "",
    NC_("patterns-action", "Copy pattern file location to clipboard"),
    G_CALLBACK (data_copy_location_cmd_callback),
    PICMAN_HELP_PATTERN_COPY_LOCATION },

  { "patterns-delete", GTK_STOCK_DELETE,
    NC_("patterns-action", "_Delete Pattern"), "",
    NC_("patterns-action", "Delete this pattern"),
    G_CALLBACK (data_delete_cmd_callback),
    PICMAN_HELP_PATTERN_DELETE },

  { "patterns-refresh", GTK_STOCK_REFRESH,
    NC_("patterns-action", "_Refresh Patterns"), "",
    NC_("patterns-action", "Refresh patterns"),
    G_CALLBACK (data_refresh_cmd_callback),
    PICMAN_HELP_PATTERN_REFRESH }
};

static const PicmanStringActionEntry patterns_edit_actions[] =
{
  { "patterns-edit", GTK_STOCK_EDIT,
    NC_("patterns-action", "_Edit Pattern..."), NULL,
    NC_("patterns-action", "Edit pattern"),
    "picman-pattern-editor",
    PICMAN_HELP_PATTERN_EDIT }
};


void
patterns_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "patterns-action",
                                 patterns_actions,
                                 G_N_ELEMENTS (patterns_actions));

  picman_action_group_add_string_actions (group, "patterns-action",
                                        patterns_edit_actions,
                                        G_N_ELEMENTS (patterns_edit_actions),
                                        G_CALLBACK (data_edit_cmd_callback));
}

void
patterns_actions_update (PicmanActionGroup *group,
                         gpointer         user_data)
{
  PicmanContext *context  = action_data_get_context (user_data);
  PicmanPattern *pattern  = NULL;
  PicmanData    *data     = NULL;
  const gchar *filename = NULL;

  if (context)
    {
      pattern = picman_context_get_pattern (context);

      if (action_data_sel_count (user_data) > 1)
        {
          pattern = NULL;
        }

      if (pattern)
        {
          data = PICMAN_DATA (pattern);

          filename = picman_data_get_filename (data);
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("patterns-edit",          pattern && FALSE);
  SET_SENSITIVE ("patterns-open-as-image", pattern && filename);
  SET_SENSITIVE ("patterns-duplicate",     pattern && PICMAN_DATA_GET_CLASS (data)->duplicate);
  SET_SENSITIVE ("patterns-copy-location", pattern && filename);
  SET_SENSITIVE ("patterns-delete",        pattern && picman_data_is_deletable (data));

#undef SET_SENSITIVE
}
