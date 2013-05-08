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

#include "core/picmanchannel.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "vectors-actions.h"
#include "vectors-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry vectors_actions[] =
{
  { "vectors-popup", PICMAN_STOCK_PATHS,
    NC_("vectors-action", "Paths Menu"), NULL, NULL, NULL,
    PICMAN_HELP_PATH_DIALOG },

  { "vectors-path-tool", PICMAN_STOCK_TOOL_PATH,
    NC_("vectors-action", "Path _Tool"), NULL, NULL,
    G_CALLBACK (vectors_vectors_tool_cmd_callback),
    PICMAN_HELP_TOOL_VECTORS },

  { "vectors-edit-attributes", GTK_STOCK_EDIT,
    NC_("vectors-action", "_Edit Path Attributes..."), NULL,
    NC_("vectors-action", "Edit path attributes"),
    G_CALLBACK (vectors_edit_attributes_cmd_callback),
    PICMAN_HELP_PATH_EDIT },

  { "vectors-new", GTK_STOCK_NEW,
    NC_("vectors-action", "_New Path..."), "",
    NC_("vectors-action", "Create a new path..."),
    G_CALLBACK (vectors_new_cmd_callback),
    PICMAN_HELP_PATH_NEW },

  { "vectors-new-last-values", GTK_STOCK_NEW,
    NC_("vectors-action", "_New Path with last values"), "",
    NC_("vectors-action", "Create a new path with last used values"),
    G_CALLBACK (vectors_new_last_vals_cmd_callback),
    PICMAN_HELP_PATH_NEW },

  { "vectors-duplicate", PICMAN_STOCK_DUPLICATE,
    NC_("vectors-action", "D_uplicate Path"), NULL,
    NC_("vectors-action", "Duplicate this path"),
    G_CALLBACK (vectors_duplicate_cmd_callback),
    PICMAN_HELP_PATH_DUPLICATE },

  { "vectors-delete", GTK_STOCK_DELETE,
    NC_("vectors-action", "_Delete Path"), "",
    NC_("vectors-action", "Delete this path"),
    G_CALLBACK (vectors_delete_cmd_callback),
    PICMAN_HELP_PATH_DELETE },

  { "vectors-merge-visible", NULL,
    NC_("vectors-action", "Merge _Visible Paths"), NULL, NULL,
    G_CALLBACK (vectors_merge_visible_cmd_callback),
    PICMAN_HELP_PATH_MERGE_VISIBLE },

  { "vectors-raise", GTK_STOCK_GO_UP,
    NC_("vectors-action", "_Raise Path"), "",
    NC_("vectors-action", "Raise this path"),
    G_CALLBACK (vectors_raise_cmd_callback),
    PICMAN_HELP_PATH_RAISE },

  { "vectors-raise-to-top", GTK_STOCK_GOTO_TOP,
    NC_("vectors-action", "Raise Path to _Top"), "",
    NC_("vectors-action", "Raise this path to the top"),
    G_CALLBACK (vectors_raise_to_top_cmd_callback),
    PICMAN_HELP_PATH_RAISE_TO_TOP },

  { "vectors-lower", GTK_STOCK_GO_DOWN,
    NC_("vectors-action", "_Lower Path"), "",
    NC_("vectors-action", "Lower this path"),
    G_CALLBACK (vectors_lower_cmd_callback),
    PICMAN_HELP_PATH_LOWER },

  { "vectors-lower-to-bottom", GTK_STOCK_GOTO_BOTTOM,
    NC_("vectors-action", "Lower Path to _Bottom"), "",
    NC_("vectors-action", "Lower this path to the bottom"),
    G_CALLBACK (vectors_lower_to_bottom_cmd_callback),
    PICMAN_HELP_PATH_LOWER_TO_BOTTOM },

  { "vectors-stroke", PICMAN_STOCK_PATH_STROKE,
    NC_("vectors-action", "Stro_ke Path..."), NULL,
    NC_("vectors-action", "Paint along the path"),
    G_CALLBACK (vectors_stroke_cmd_callback),
    PICMAN_HELP_PATH_STROKE },

  { "vectors-stroke-last-values", PICMAN_STOCK_PATH_STROKE,
    NC_("vectors-action", "Stro_ke Path"), NULL,
    NC_("vectors-action", "Paint along the path with last values"),
    G_CALLBACK (vectors_stroke_last_vals_cmd_callback),
    PICMAN_HELP_PATH_STROKE },

  { "vectors-copy", GTK_STOCK_COPY,
    NC_("vectors-action", "Co_py Path"), "", NULL,
    G_CALLBACK (vectors_copy_cmd_callback),
    PICMAN_HELP_PATH_COPY },

  { "vectors-paste", GTK_STOCK_PASTE,
    NC_("vectors-action", "Paste Pat_h"), "", NULL,
    G_CALLBACK (vectors_paste_cmd_callback),
    PICMAN_HELP_PATH_PASTE },

  { "vectors-export", GTK_STOCK_SAVE,
    NC_("vectors-action", "E_xport Path..."), "", NULL,
    G_CALLBACK (vectors_export_cmd_callback),
    PICMAN_HELP_PATH_EXPORT },

  { "vectors-import", GTK_STOCK_OPEN,
    NC_("vectors-action", "I_mport Path..."), "", NULL,
    G_CALLBACK (vectors_import_cmd_callback),
    PICMAN_HELP_PATH_IMPORT }
};

static const PicmanToggleActionEntry vectors_toggle_actions[] =
{
  { "vectors-visible", PICMAN_STOCK_VISIBLE,
    NC_("vectors-action", "_Visible"), NULL, NULL,
    G_CALLBACK (vectors_visible_cmd_callback),
    FALSE,
    PICMAN_HELP_PATH_VISIBLE },

  { "vectors-linked", PICMAN_STOCK_LINKED,
    NC_("vectors-action", "_Linked"), NULL, NULL,
    G_CALLBACK (vectors_linked_cmd_callback),
    FALSE,
    PICMAN_HELP_PATH_LINKED },

  { "vectors-lock-content", NULL /* PICMAN_STOCK_LOCK */,
    NC_("vectors-action", "L_ock strokes"), NULL, NULL,
    G_CALLBACK (vectors_lock_content_cmd_callback),
    FALSE,
    PICMAN_HELP_PATH_LOCK_STROKES },

  { "vectors-lock-position", PICMAN_STOCK_TOOL_MOVE,
    NC_("vectors-action", "L_ock position"), NULL, NULL,
    G_CALLBACK (vectors_lock_position_cmd_callback),
    FALSE,
    PICMAN_HELP_PATH_LOCK_POSITION }
};

static const PicmanEnumActionEntry vectors_to_selection_actions[] =
{
  { "vectors-selection-replace", PICMAN_STOCK_SELECTION_REPLACE,
    NC_("vectors-action", "Path to Sele_ction"), NULL,
    NC_("vectors-action", "Path to selection"),
    PICMAN_CHANNEL_OP_REPLACE, FALSE,
    PICMAN_HELP_PATH_SELECTION_REPLACE },

  { "vectors-selection-from-vectors", PICMAN_STOCK_SELECTION_REPLACE,
    NC_("vectors-action", "Fr_om Path"), "<shift>V",
    NC_("vectors-action", "Replace selection with path"),
    PICMAN_CHANNEL_OP_REPLACE, FALSE,
    PICMAN_HELP_PATH_SELECTION_REPLACE },

  { "vectors-selection-add", PICMAN_STOCK_SELECTION_ADD,
    NC_("vectors-action", "_Add to Selection"), NULL,
    NC_("vectors-action", "Add path to selection"),
    PICMAN_CHANNEL_OP_ADD, FALSE,
    PICMAN_HELP_PATH_SELECTION_ADD },

  { "vectors-selection-subtract", PICMAN_STOCK_SELECTION_SUBTRACT,
    NC_("vectors-action", "_Subtract from Selection"), NULL,
    NC_("vectors-action", "Subtract path from selection"),
    PICMAN_CHANNEL_OP_SUBTRACT, FALSE,
    PICMAN_HELP_PATH_SELECTION_SUBTRACT },

  { "vectors-selection-intersect", PICMAN_STOCK_SELECTION_INTERSECT,
    NC_("vectors-action", "_Intersect with Selection"), NULL,
    NC_("vectors-action", "Intersect path with selection"),
    PICMAN_CHANNEL_OP_INTERSECT, FALSE,
    PICMAN_HELP_PATH_SELECTION_INTERSECT }
};

static const PicmanEnumActionEntry vectors_selection_to_vectors_actions[] =
{
  { "vectors-selection-to-vectors", PICMAN_STOCK_SELECTION_TO_PATH,
    NC_("vectors-action", "Selecti_on to Path"), NULL,
    NC_("vectors-action", "Selection to path"),
    FALSE, FALSE,
    PICMAN_HELP_SELECTION_TO_PATH },

  { "vectors-selection-to-vectors-short", PICMAN_STOCK_SELECTION_TO_PATH,
    NC_("vectors-action", "To _Path"), NULL,
    NC_("vectors-action", "Selection to path"),
    FALSE, FALSE,
    PICMAN_HELP_SELECTION_TO_PATH },

  { "vectors-selection-to-vectors-advanced", PICMAN_STOCK_SELECTION_TO_PATH,
    NC_("vectors-action", "Selection to Path (_Advanced)"), NULL,
    NC_("vectors-action", "Advanced options"),
    TRUE, FALSE,
    PICMAN_HELP_SELECTION_TO_PATH }
};


void
vectors_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "vectors-action",
                                 vectors_actions,
                                 G_N_ELEMENTS (vectors_actions));

  picman_action_group_add_toggle_actions (group, "vectors-action",
                                        vectors_toggle_actions,
                                        G_N_ELEMENTS (vectors_toggle_actions));

  picman_action_group_add_enum_actions (group, "vectors-action",
                                      vectors_to_selection_actions,
                                      G_N_ELEMENTS (vectors_to_selection_actions),
                                      G_CALLBACK (vectors_to_selection_cmd_callback));

  picman_action_group_add_enum_actions (group, "vectors-action",
                                      vectors_selection_to_vectors_actions,
                                      G_N_ELEMENTS (vectors_selection_to_vectors_actions),
                                      G_CALLBACK (vectors_selection_to_vectors_cmd_callback));
}

void
vectors_actions_update (PicmanActionGroup *group,
                        gpointer         data)
{
  PicmanImage    *image        = action_data_get_image (data);
  PicmanVectors  *vectors      = NULL;
  PicmanDrawable *drawable     = NULL;
  gint          n_vectors    = 0;
  gboolean      mask_empty   = TRUE;
  gboolean      visible      = FALSE;
  gboolean      linked       = FALSE;
  gboolean      locked       = FALSE;
  gboolean      can_lock     = FALSE;
  gboolean      locked_pos   = FALSE;
  gboolean      can_lock_pos = FALSE;
  gboolean      dr_writable  = FALSE;
  gboolean      dr_children  = FALSE;
  GList        *next         = NULL;
  GList        *prev         = NULL;

  if (image)
    {
      n_vectors  = picman_image_get_n_vectors (image);
      mask_empty = picman_channel_is_empty (picman_image_get_mask (image));

      vectors = picman_image_get_active_vectors (image);

      if (vectors)
        {
          PicmanItem *item = PICMAN_ITEM (vectors);
          GList    *vectors_list;
          GList    *list;

          visible      = picman_item_get_visible (item);
          linked       = picman_item_get_linked (item);
          locked       = picman_item_get_lock_content (item);
          can_lock     = picman_item_can_lock_content (item);
          locked_pos   = picman_item_get_lock_position (item);
          can_lock_pos = picman_item_can_lock_position (item);
          vectors_list = picman_item_get_container_iter (item);

          list = g_list_find (vectors_list, vectors);

          if (list)
            {
              prev = g_list_previous (list);
              next = g_list_next (list);
            }
        }

      drawable = picman_image_get_active_drawable (image);

      if (drawable)
        {
          PicmanItem *item = PICMAN_ITEM (drawable);

          dr_writable = ! picman_item_is_content_locked (item);

          if (picman_viewable_get_children (PICMAN_VIEWABLE (item)))
            dr_children = TRUE;
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  SET_SENSITIVE ("vectors-path-tool",       vectors);
  SET_SENSITIVE ("vectors-edit-attributes", vectors);

  SET_SENSITIVE ("vectors-new",             image);
  SET_SENSITIVE ("vectors-new-last-values", image);
  SET_SENSITIVE ("vectors-duplicate",       vectors);
  SET_SENSITIVE ("vectors-delete",          vectors);
  SET_SENSITIVE ("vectors-merge-visible",   n_vectors > 1);

  SET_SENSITIVE ("vectors-raise",           vectors && prev);
  SET_SENSITIVE ("vectors-raise-to-top",    vectors && prev);
  SET_SENSITIVE ("vectors-lower",           vectors && next);
  SET_SENSITIVE ("vectors-lower-to-bottom", vectors && next);

  SET_SENSITIVE ("vectors-copy",   vectors);
  SET_SENSITIVE ("vectors-paste",  image);
  SET_SENSITIVE ("vectors-export", vectors);
  SET_SENSITIVE ("vectors-import", image);

  SET_SENSITIVE ("vectors-visible",       vectors);
  SET_SENSITIVE ("vectors-linked",        vectors);
  SET_SENSITIVE ("vectors-lock-content",  can_lock);
  SET_SENSITIVE ("vectors-lock-position", can_lock_pos);

  SET_ACTIVE ("vectors-visible",       visible);
  SET_ACTIVE ("vectors-linked",        linked);
  SET_ACTIVE ("vectors-lock-content",  locked);
  SET_ACTIVE ("vectors-lock-position", locked_pos);

  SET_SENSITIVE ("vectors-selection-to-vectors",          image && !mask_empty);
  SET_SENSITIVE ("vectors-selection-to-vectors-short",    image && !mask_empty);
  SET_SENSITIVE ("vectors-selection-to-vectors-advanced", image && !mask_empty);
  SET_SENSITIVE ("vectors-stroke",                        vectors &&
                                                          dr_writable &&
                                                          !dr_children);
  SET_SENSITIVE ("vectors-stroke-last-values",            vectors &&
                                                          dr_writable &&
                                                          !dr_children);

  SET_SENSITIVE ("vectors-selection-replace",      vectors);
  SET_SENSITIVE ("vectors-selection-from-vectors", vectors);
  SET_SENSITIVE ("vectors-selection-add",          vectors);
  SET_SENSITIVE ("vectors-selection-subtract",     vectors);
  SET_SENSITIVE ("vectors-selection-intersect",    vectors);

#undef SET_SENSITIVE
#undef SET_ACTIVE
}
