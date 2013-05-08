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

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "templates-actions.h"
#include "templates-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry templates_actions[] =
{
  { "templates-popup", PICMAN_STOCK_TEMPLATE,
    NC_("templates-action", "Templates Menu"), NULL, NULL, NULL,
    PICMAN_HELP_TEMPLATE_DIALOG },

  { "templates-create-image", PICMAN_STOCK_IMAGE,
    NC_("templates-action", "_Create Image from Template"), "",
    NC_("templates-action", "Create a new image from the selected template"),
    G_CALLBACK (templates_create_image_cmd_callback),
    PICMAN_HELP_TEMPLATE_IMAGE_NEW },

  { "templates-new", GTK_STOCK_NEW,
    NC_("templates-action", "_New Template..."), "",
    NC_("templates-action", "Create a new template"),
    G_CALLBACK (templates_new_cmd_callback),
    PICMAN_HELP_TEMPLATE_NEW },

  { "templates-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("templates-action", "D_uplicate Template..."), "",
    NC_("templates-action", "Duplicate this template"),
    G_CALLBACK (templates_duplicate_cmd_callback),
    PICMAN_HELP_TEMPLATE_DUPLICATE },

  { "templates-edit", GTK_STOCK_EDIT,
    NC_("templates-action", "_Edit Template..."), "",
    NC_("templates-action", "Edit this template"),
    G_CALLBACK (templates_edit_cmd_callback),
    PICMAN_HELP_TEMPLATE_EDIT },

  { "templates-delete", GTK_STOCK_DELETE,
    NC_("templates-action", "_Delete Template"), "",
    NC_("templates-action", "Delete this template"),
    G_CALLBACK (templates_delete_cmd_callback),
    PICMAN_HELP_TEMPLATE_DELETE }
};


void
templates_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "templates-action",
                                 templates_actions,
                                 G_N_ELEMENTS (templates_actions));
}

void
templates_actions_update (PicmanActionGroup *group,
                          gpointer         data)
{
  PicmanContext  *context  = action_data_get_context (data);
  PicmanTemplate *template = NULL;

  if (context)
    template = picman_context_get_template (context);

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("templates-create-image", template);
  SET_SENSITIVE ("templates-new",          context);
  SET_SENSITIVE ("templates-duplicate",    template);
  SET_SENSITIVE ("templates-edit",         template);
  SET_SENSITIVE ("templates-delete",       template);

#undef SET_SENSITIVE
}
