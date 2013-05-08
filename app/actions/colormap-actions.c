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
#include "core/picmanimage.h"
#include "core/picmanimage-colormap.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "colormap-actions.h"
#include "colormap-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry colormap_actions[] =
{
  { "colormap-popup", PICMAN_STOCK_COLORMAP,
    NC_("colormap-action", "Colormap Menu"), NULL, NULL, NULL,
    PICMAN_HELP_INDEXED_PALETTE_DIALOG },

  { "colormap-edit-color", GTK_STOCK_EDIT,
    NC_("colormap-action", "_Edit Color..."), NULL,
    NC_("colormap-action", "Edit this color"),
    G_CALLBACK (colormap_edit_color_cmd_callback),
    PICMAN_HELP_INDEXED_PALETTE_EDIT }
};

static const PicmanEnumActionEntry colormap_add_color_actions[] =
{
  { "colormap-add-color-from-fg", GTK_STOCK_ADD,
    NC_("colormap-action", "_Add Color from FG"), "",
    NC_("colormap-action", "Add current foreground color"),
    FALSE, FALSE,
    PICMAN_HELP_INDEXED_PALETTE_ADD },

  { "colormap-add-color-from-bg", GTK_STOCK_ADD,
    NC_("colormap-action", "_Add Color from BG"), "",
    NC_("colormap-action", "Add current background color"),
    TRUE, FALSE,
    PICMAN_HELP_INDEXED_PALETTE_ADD }
};


void
colormap_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "colormap-action",
                                 colormap_actions,
                                 G_N_ELEMENTS (colormap_actions));

  picman_action_group_add_enum_actions (group, "colormap-action",
                                      colormap_add_color_actions,
                                      G_N_ELEMENTS (colormap_add_color_actions),
                                      G_CALLBACK (colormap_add_color_cmd_callback));
}

void
colormap_actions_update (PicmanActionGroup *group,
                         gpointer         data)
{
  PicmanImage   *image      = action_data_get_image (data);
  PicmanContext *context    = action_data_get_context (data);
  gboolean     indexed    = FALSE;
  gint         num_colors = 0;
  PicmanRGB      fg;
  PicmanRGB      bg;

  if (image)
    {
      indexed    = (picman_image_get_base_type (image) == PICMAN_INDEXED);
      num_colors = picman_image_get_colormap_size (image);
    }

  if (context)
    {
      picman_context_get_foreground (context, &fg);
      picman_context_get_background (context, &bg);
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_COLOR(action,color) \
        picman_action_group_set_action_color (group, action, color, FALSE);

  SET_SENSITIVE ("colormap-edit-color",
                 image && indexed);
  SET_SENSITIVE ("colormap-add-color-from-fg",
                 image && indexed && num_colors < 256);
  SET_SENSITIVE ("colormap-add-color-from-bg",
                 image && indexed && num_colors < 256);

  SET_COLOR ("colormap-add-color-from-fg", context ? &fg : NULL);
  SET_COLOR ("colormap-add-color-from-bg", context ? &bg : NULL);

#undef SET_SENSITIVE
#undef SET_COLOR
}
