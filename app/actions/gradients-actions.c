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
#include "gradients-actions.h"
#include "gradients-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry gradients_actions[] =
{
  { "gradients-popup", PICMAN_STOCK_GRADIENT,
    NC_("gradients-action", "Gradients Menu"), NULL, NULL, NULL,
    PICMAN_HELP_GRADIENT_DIALOG },

  { "gradients-new", GTK_STOCK_NEW,
    NC_("gradients-action", "_New Gradient"), "",
    NC_("gradients-action", "Create a new gradient"),
    G_CALLBACK (data_new_cmd_callback),
    PICMAN_HELP_GRADIENT_NEW },

  { "gradients-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("gradients-action", "D_uplicate Gradient"), NULL,
    NC_("gradients-action", "Duplicate this gradient"),
    G_CALLBACK (data_duplicate_cmd_callback),
    PICMAN_HELP_GRADIENT_DUPLICATE },

  { "gradients-copy-location", GTK_STOCK_COPY,
    NC_("gradients-action", "Copy Gradient _Location"), "",
    NC_("gradients-action", "Copy gradient file location to clipboard"),
    G_CALLBACK (data_copy_location_cmd_callback),
    PICMAN_HELP_GRADIENT_COPY_LOCATION },

  { "gradients-save-as-pov", GTK_STOCK_SAVE_AS,
    NC_("gradients-action", "Save as _POV-Ray..."), "",
    NC_("gradients-action", "Save gradient as POV-Ray"),
    G_CALLBACK (gradients_save_as_pov_ray_cmd_callback),
    PICMAN_HELP_GRADIENT_SAVE_AS_POV },

  { "gradients-delete", GTK_STOCK_DELETE,
    NC_("gradients-action", "_Delete Gradient"), "",
    NC_("gradients-action", "Delete this gradient"),
    G_CALLBACK (data_delete_cmd_callback),
    PICMAN_HELP_GRADIENT_DELETE },

  { "gradients-refresh", GTK_STOCK_REFRESH,
    NC_("gradients-action", "_Refresh Gradients"), "",
    NC_("gradients-action", "Refresh gradients"),
    G_CALLBACK (data_refresh_cmd_callback),
    PICMAN_HELP_GRADIENT_REFRESH }
};

static const PicmanStringActionEntry gradients_edit_actions[] =
{
  { "gradients-edit", GTK_STOCK_EDIT,
    NC_("gradients-action", "_Edit Gradient..."), NULL,
    NC_("gradients-action", "Edit gradient"),
    "picman-gradient-editor",
    PICMAN_HELP_GRADIENT_EDIT }
};


void
gradients_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "gradients-action",
                                 gradients_actions,
                                 G_N_ELEMENTS (gradients_actions));

  picman_action_group_add_string_actions (group, "gradients-action",
                                        gradients_edit_actions,
                                        G_N_ELEMENTS (gradients_edit_actions),
                                        G_CALLBACK (data_edit_cmd_callback));
}

void
gradients_actions_update (PicmanActionGroup *group,
                          gpointer         user_data)
{
  PicmanContext  *context  = action_data_get_context (user_data);
  PicmanGradient *gradient = NULL;
  PicmanData     *data     = NULL;
  const gchar  *filename = NULL;

  if (context)
    {
      gradient = picman_context_get_gradient (context);

      if (action_data_sel_count (user_data) > 1)
        {
          gradient = NULL;
        }

      if (gradient)
        {
          data = PICMAN_DATA (gradient);

          filename = picman_data_get_filename (data);
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("gradients-edit",          gradient);
  SET_SENSITIVE ("gradients-duplicate",     gradient);
  SET_SENSITIVE ("gradients-save-as-pov",   gradient);
  SET_SENSITIVE ("gradients-copy-location", gradient && filename);
  SET_SENSITIVE ("gradients-delete",        gradient && picman_data_is_deletable (data));

#undef SET_SENSITIVE
}
