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
#include "widgets/picmanpaletteeditor.h"

#include "data-editor-commands.h"
#include "palette-editor-actions.h"
#include "palette-editor-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry palette_editor_actions[] =
{
  { "palette-editor-popup", PICMAN_STOCK_PALETTE,
    NC_("palette-editor-action", "Palette Editor Menu"), NULL, NULL, NULL,
    PICMAN_HELP_PALETTE_EDITOR_DIALOG },

  { "palette-editor-edit-color", GTK_STOCK_EDIT,
    NC_("palette-editor-action", "_Edit Color..."), "",
    NC_("palette-editor-action", "Edit this entry"),
    G_CALLBACK (palette_editor_edit_color_cmd_callback),
    PICMAN_HELP_PALETTE_EDITOR_EDIT },

  { "palette-editor-delete-color", GTK_STOCK_DELETE,
    NC_("palette-editor-action", "_Delete Color"), "",
    NC_("palette-editor-action", "Delete this entry"),
    G_CALLBACK (palette_editor_delete_color_cmd_callback),
    PICMAN_HELP_PALETTE_EDITOR_DELETE }
};

static const PicmanToggleActionEntry palette_editor_toggle_actions[] =
{
  { "palette-editor-edit-active", PICMAN_STOCK_LINKED,
    NC_("palette-editor-action", "Edit Active Palette"), NULL, NULL,
    G_CALLBACK (data_editor_edit_active_cmd_callback),
    FALSE,
    PICMAN_HELP_PALETTE_EDITOR_EDIT_ACTIVE }
};

static const PicmanEnumActionEntry palette_editor_new_actions[] =
{
  { "palette-editor-new-color-fg", GTK_STOCK_NEW,
    NC_("palette-editor-action", "New Color from _FG"), "",
    NC_("palette-editor-action",
        "Create a new entry from the foreground color"),
    FALSE, FALSE,
    PICMAN_HELP_PALETTE_EDITOR_NEW },

  { "palette-editor-new-color-bg", GTK_STOCK_NEW,
    NC_("palette-editor-action", "New Color from _BG"), "",
    NC_("palette-editor-action",
        "Create a new entry from the background color"),
    TRUE, FALSE,
    PICMAN_HELP_PALETTE_EDITOR_NEW }
};

static const PicmanEnumActionEntry palette_editor_zoom_actions[] =
{
  { "palette-editor-zoom-in", GTK_STOCK_ZOOM_IN,
    N_("Zoom _In"), "",
    N_("Zoom in"),
    PICMAN_ZOOM_IN, FALSE,
    PICMAN_HELP_PALETTE_EDITOR_ZOOM_IN },

  { "palette-editor-zoom-out", GTK_STOCK_ZOOM_OUT,
    N_("Zoom _Out"), "",
    N_("Zoom out"),
    PICMAN_ZOOM_OUT, FALSE,
    PICMAN_HELP_PALETTE_EDITOR_ZOOM_OUT },

  { "palette-editor-zoom-all", GTK_STOCK_ZOOM_FIT,
    N_("Zoom _All"), "",
    N_("Zoom all"),
    PICMAN_ZOOM_OUT_MAX, FALSE,
    PICMAN_HELP_PALETTE_EDITOR_ZOOM_ALL }
};


void
palette_editor_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "palette-editor-action",
                                 palette_editor_actions,
                                 G_N_ELEMENTS (palette_editor_actions));

  picman_action_group_add_toggle_actions (group, "palette-editor-action",
                                        palette_editor_toggle_actions,
                                        G_N_ELEMENTS (palette_editor_toggle_actions));

  picman_action_group_add_enum_actions (group, "palette-editor-action",
                                      palette_editor_new_actions,
                                      G_N_ELEMENTS (palette_editor_new_actions),
                                      G_CALLBACK (palette_editor_new_color_cmd_callback));

  picman_action_group_add_enum_actions (group, NULL,
                                      palette_editor_zoom_actions,
                                      G_N_ELEMENTS (palette_editor_zoom_actions),
                                      G_CALLBACK (palette_editor_zoom_cmd_callback));
}

void
palette_editor_actions_update (PicmanActionGroup *group,
                               gpointer         user_data)
{
  PicmanPaletteEditor *editor      = PICMAN_PALETTE_EDITOR (user_data);
  PicmanDataEditor    *data_editor = PICMAN_DATA_EDITOR (user_data);
  PicmanData          *data;
  gboolean           editable    = FALSE;
  PicmanRGB            fg;
  PicmanRGB            bg;
  gboolean           edit_active = FALSE;

  data = data_editor->data;

  if (data)
    {
      if (data_editor->data_editable)
        editable = TRUE;
    }

  if (data_editor->context)
    {
      picman_context_get_foreground (data_editor->context, &fg);
      picman_context_get_background (data_editor->context, &bg);
    }

  edit_active = picman_data_editor_get_edit_active (data_editor);

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)
#define SET_COLOR(action,color) \
        picman_action_group_set_action_color (group, action, color, FALSE);

  SET_SENSITIVE ("palette-editor-edit-color",   editable && editor->color);
  SET_SENSITIVE ("palette-editor-delete-color", editable && editor->color);

  SET_SENSITIVE ("palette-editor-new-color-fg", editable);
  SET_SENSITIVE ("palette-editor-new-color-bg", editable);

  SET_COLOR ("palette-editor-new-color-fg", data_editor->context ? &fg : NULL);
  SET_COLOR ("palette-editor-new-color-bg", data_editor->context ? &bg : NULL);

  SET_SENSITIVE ("palette-editor-zoom-out", data);
  SET_SENSITIVE ("palette-editor-zoom-in",  data);
  SET_SENSITIVE ("palette-editor-zoom-all", data);

  SET_ACTIVE ("palette-editor-edit-active", edit_active);

#undef SET_SENSITIVE
#undef SET_ACTIVE
#undef SET_COLOR
}
