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

#include "core/picmanbrushgenerated.h"
#include "core/picmancontext.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "brushes-actions.h"
#include "data-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry brushes_actions[] =
{
  { "brushes-popup", PICMAN_STOCK_BRUSH,
    NC_("brushes-action", "Brushes Menu"), NULL, NULL, NULL,
    PICMAN_HELP_BRUSH_DIALOG },

  { "brushes-open-as-image", GTK_STOCK_OPEN,
    NC_("brushes-action", "_Open Brush as Image"), "",
    NC_("brushes-action", "Open brush as image"),
    G_CALLBACK (data_open_as_image_cmd_callback),
    PICMAN_HELP_BRUSH_OPEN_AS_IMAGE },

  { "brushes-new", GTK_STOCK_NEW,
    NC_("brushes-action", "_New Brush"), "",
    NC_("brushes-action", "Create a new brush"),
    G_CALLBACK (data_new_cmd_callback),
    PICMAN_HELP_BRUSH_NEW },

  { "brushes-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("brushes-action", "D_uplicate Brush"), NULL,
    NC_("brushes-action", "Duplicate this brush"),
    G_CALLBACK (data_duplicate_cmd_callback),
    PICMAN_HELP_BRUSH_DUPLICATE },

  { "brushes-copy-location", GTK_STOCK_COPY,
    NC_("brushes-action", "Copy Brush _Location"), "",
    NC_("brushes-action", "Copy brush file location to clipboard"),
    G_CALLBACK (data_copy_location_cmd_callback),
    PICMAN_HELP_BRUSH_COPY_LOCATION },

  { "brushes-delete", GTK_STOCK_DELETE,
    NC_("brushes-action", "_Delete Brush"), "",
    NC_("brushes-action", "Delete this brush"),
    G_CALLBACK (data_delete_cmd_callback),
    PICMAN_HELP_BRUSH_DELETE },

  { "brushes-refresh", GTK_STOCK_REFRESH,
    NC_("brushes-action", "_Refresh Brushes"), "",
    NC_("brushes-action", "Refresh brushes"),
    G_CALLBACK (data_refresh_cmd_callback),
    PICMAN_HELP_BRUSH_REFRESH }
};

static const PicmanStringActionEntry brushes_edit_actions[] =
{
  { "brushes-edit", GTK_STOCK_EDIT,
    NC_("brushes-action", "_Edit Brush..."), NULL,
    NC_("brushes-action", "Edit this brush"),
    "picman-brush-editor",
    PICMAN_HELP_BRUSH_EDIT }
};


void
brushes_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "brushes-action",
                                 brushes_actions,
                                 G_N_ELEMENTS (brushes_actions));

  picman_action_group_add_string_actions (group, "brushes-action",
                                        brushes_edit_actions,
                                        G_N_ELEMENTS (brushes_edit_actions),
                                        G_CALLBACK (data_edit_cmd_callback));
}

void
brushes_actions_update (PicmanActionGroup *group,
                        gpointer         user_data)
{
  PicmanContext *context  = action_data_get_context (user_data);
  PicmanBrush   *brush    = NULL;
  PicmanData    *data     = NULL;
  const gchar *filename = NULL;

  if (context)
    {
      brush = picman_context_get_brush (context);

      if (action_data_sel_count (user_data) > 1)
        {
          brush = NULL;
        }

      if (brush)
        {
          data = PICMAN_DATA (brush);

          filename = picman_data_get_filename (data);
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)

  SET_SENSITIVE ("brushes-edit",          brush);
  SET_SENSITIVE ("brushes-open-as-image", brush && filename && ! PICMAN_IS_BRUSH_GENERATED (brush));
  SET_SENSITIVE ("brushes-duplicate",     brush && PICMAN_DATA_GET_CLASS (data)->duplicate);
  SET_SENSITIVE ("brushes-copy-location", brush && filename);
  SET_SENSITIVE ("brushes-delete",        brush && picman_data_is_deletable (data));

#undef SET_SENSITIVE
}
