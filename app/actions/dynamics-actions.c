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
#include "dynamics-actions.h"

#include "picman-intl.h"


static const PicmanActionEntry dynamics_actions[] =
{
  { "dynamics-popup", PICMAN_STOCK_DYNAMICS,
    NC_("dynamics-action", "Paint Dynamics Menu"), NULL, NULL, NULL,
    PICMAN_HELP_DYNAMICS_DIALOG },

  { "dynamics-new", GTK_STOCK_NEW,
    NC_("dynamics-action", "_New Dynamics"), "",
    NC_("dynamics-action", "Create a new dynamics"),
    G_CALLBACK (data_new_cmd_callback),
    PICMAN_HELP_DYNAMICS_NEW },

  { "dynamics-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("dynamics-action", "D_uplicate Dynamics"), NULL,
    NC_("dynamics-action", "Duplicate this dynamics"),
    G_CALLBACK (data_duplicate_cmd_callback),
    PICMAN_HELP_DYNAMICS_DUPLICATE },

  { "dynamics-copy-location", GTK_STOCK_COPY,
    NC_("dynamics-action", "Copy Dynamics _Location"), "",
    NC_("dynamics-action", "Copy dynamics file location to clipboard"),
    G_CALLBACK (data_copy_location_cmd_callback),
    PICMAN_HELP_DYNAMICS_COPY_LOCATION },

  { "dynamics-delete", GTK_STOCK_DELETE,
    NC_("dynamics-action", "_Delete Dynamics"), "",
    NC_("dynamics-action", "Delete this dynamics"),
    G_CALLBACK (data_delete_cmd_callback),
    PICMAN_HELP_DYNAMICS_DELETE },

  { "dynamics-refresh", GTK_STOCK_REFRESH,
    NC_("dynamics-action", "_Refresh Dynamics"), "",
    NC_("dynamics-action", "Refresh dynamics"),
    G_CALLBACK (data_refresh_cmd_callback),
    PICMAN_HELP_DYNAMICS_REFRESH }
};

static const PicmanStringActionEntry dynamics_edit_actions[] =
{
  { "dynamics-edit", GTK_STOCK_EDIT,
    NC_("dynamics-action", "_Edit Dynamics..."), NULL,
    NC_("dynamics-action", "Edit dynamics"),
    "picman-dynamics-editor",
    PICMAN_HELP_DYNAMICS_EDIT }
};


void
dynamics_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "dynamics-action",
                                 dynamics_actions,
                                 G_N_ELEMENTS (dynamics_actions));

  picman_action_group_add_string_actions (group, "dynamics-action",
                                        dynamics_edit_actions,
                                        G_N_ELEMENTS (dynamics_edit_actions),
                                        G_CALLBACK (data_edit_cmd_callback));
}

void
dynamics_actions_update (PicmanActionGroup *group,
                         gpointer         user_data)
{
  PicmanContext  *context  = action_data_get_context (user_data);
  PicmanDynamics *dynamics = NULL;
  PicmanData     *data     = NULL;
  const gchar  *filename = NULL;

  if (context)
    {
      dynamics = picman_context_get_dynamics (context);

      if (dynamics)
        {
          data = PICMAN_DATA (dynamics);

          filename = picman_data_get_filename (data);
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("dynamics-edit",          dynamics);
  SET_SENSITIVE ("dynamics-duplicate",     dynamics && PICMAN_DATA_GET_CLASS (data)->duplicate);
  SET_SENSITIVE ("dynamics-copy-location", dynamics && filename);
  SET_SENSITIVE ("dynamics-delete",        dynamics && picman_data_is_deletable (data));

#undef SET_SENSITIVE
}
