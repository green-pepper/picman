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
#include "core/picmanimage.h"
#include "core/picmanlayermask.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "drawable-actions.h"
#include "drawable-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry drawable_actions[] =
{
  { "drawable-equalize", NULL,
    NC_("drawable-action", "_Equalize"), NULL,
    NC_("drawable-action", "Automatic contrast enhancement"),
    G_CALLBACK (drawable_equalize_cmd_callback),
    PICMAN_HELP_LAYER_EQUALIZE },

  { "drawable-invert", PICMAN_STOCK_INVERT,
    NC_("drawable-action", "In_vert"), NULL,
    NC_("drawable-action", "Invert the colors"),
    G_CALLBACK (drawable_invert_cmd_callback),
    PICMAN_HELP_LAYER_INVERT },

  { "drawable-value-invert", PICMAN_STOCK_GEGL,
    NC_("drawable-action", "_Value Invert"), NULL,
    NC_("drawable-action", "Invert the brightness of each pixel"),
    G_CALLBACK (drawable_value_invert_cmd_callback),
    PICMAN_HELP_LAYER_INVERT },

  { "drawable-levels-stretch", NULL,
    NC_("drawable-action", "_White Balance"), NULL,
    NC_("drawable-action", "Automatic white balance correction"),
    G_CALLBACK (drawable_levels_stretch_cmd_callback),
    PICMAN_HELP_LAYER_WHITE_BALANCE},

  { "drawable-offset", NULL,
    NC_("drawable-action", "_Offset..."), "<primary><shift>O",
    NC_("drawable-action",
        "Shift the pixels, optionally wrapping them at the borders"),
    G_CALLBACK (drawable_offset_cmd_callback),
    PICMAN_HELP_LAYER_OFFSET }
};

static const PicmanToggleActionEntry drawable_toggle_actions[] =
{
  { "drawable-visible", PICMAN_STOCK_VISIBLE,
    NC_("drawable-action", "_Visible"), NULL,
    NC_("drawable-action", "Toggle visibility"),
    G_CALLBACK (drawable_visible_cmd_callback),
    FALSE,
    PICMAN_HELP_LAYER_VISIBLE },

  { "drawable-linked", PICMAN_STOCK_LINKED,
    NC_("drawable-action", "_Linked"), NULL,
    NC_("drawable-action", "Toggle the linked state"),
    G_CALLBACK (drawable_linked_cmd_callback),
    FALSE,
    PICMAN_HELP_LAYER_LINKED },

  { "drawable-lock-content", NULL /* PICMAN_STOCK_LOCK */,
    NC_("drawable-action", "L_ock pixels"), NULL,
    NC_("drawable-action",
        "Keep the pixels on this drawable from being modified"),
    G_CALLBACK (drawable_lock_content_cmd_callback),
    FALSE,
    PICMAN_HELP_LAYER_LOCK_PIXELS },

  { "drawable-lock-position", PICMAN_STOCK_TOOL_MOVE,
    NC_("drawable-action", "L_ock position of channel"), NULL,
    NC_("drawable-action",
        "Keep the position on this drawable from being modified"),
    G_CALLBACK (drawable_lock_position_cmd_callback),
    FALSE,
    PICMAN_HELP_LAYER_LOCK_POSITION },
};

static const PicmanEnumActionEntry drawable_flip_actions[] =
{
  { "drawable-flip-horizontal", PICMAN_STOCK_FLIP_HORIZONTAL,
    NC_("drawable-action", "Flip _Horizontally"), NULL,
    NC_("drawable-action", "Flip horizontally"),
    PICMAN_ORIENTATION_HORIZONTAL, FALSE,
    PICMAN_HELP_LAYER_FLIP_HORIZONTAL },

  { "drawable-flip-vertical", PICMAN_STOCK_FLIP_VERTICAL,
    NC_("drawable-action", "Flip _Vertically"), NULL,
    NC_("drawable-action", "Flip vertically"),
    PICMAN_ORIENTATION_VERTICAL, FALSE,
    PICMAN_HELP_LAYER_FLIP_VERTICAL }
};

static const PicmanEnumActionEntry drawable_rotate_actions[] =
{
  { "drawable-rotate-90", PICMAN_STOCK_ROTATE_90,
    NC_("drawable-action", "Rotate 90° _clockwise"), NULL,
    NC_("drawable-action", "Rotate 90 degrees to the right"),
    PICMAN_ROTATE_90, FALSE,
    PICMAN_HELP_LAYER_ROTATE_90 },

  { "drawable-rotate-180", PICMAN_STOCK_ROTATE_180,
    NC_("drawable-action", "Rotate _180°"), NULL,
    NC_("drawable-action", "Turn upside-down"),
    PICMAN_ROTATE_180, FALSE,
    PICMAN_HELP_LAYER_ROTATE_180 },

  { "drawable-rotate-270", PICMAN_STOCK_ROTATE_270,
    NC_("drawable-action", "Rotate 90° counter-clock_wise"), NULL,
    NC_("drawable-action", "Rotate 90 degrees to the left"),
    PICMAN_ROTATE_270, FALSE,
    PICMAN_HELP_LAYER_ROTATE_270 }
};


void
drawable_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "drawable-action",
                                 drawable_actions,
                                 G_N_ELEMENTS (drawable_actions));

  picman_action_group_add_toggle_actions (group, "drawable-action",
                                        drawable_toggle_actions,
                                        G_N_ELEMENTS (drawable_toggle_actions));

  picman_action_group_add_enum_actions (group, "drawable-action",
                                      drawable_flip_actions,
                                      G_N_ELEMENTS (drawable_flip_actions),
                                      G_CALLBACK (drawable_flip_cmd_callback));

  picman_action_group_add_enum_actions (group, "drawable-action",
                                      drawable_rotate_actions,
                                      G_N_ELEMENTS (drawable_rotate_actions),
                                      G_CALLBACK (drawable_rotate_cmd_callback));

#define SET_ALWAYS_SHOW_IMAGE(action,show) \
        picman_action_group_set_action_always_show_image (group, action, show)

  SET_ALWAYS_SHOW_IMAGE ("drawable-rotate-90",  TRUE);
  SET_ALWAYS_SHOW_IMAGE ("drawable-rotate-180", TRUE);
  SET_ALWAYS_SHOW_IMAGE ("drawable-rotate-270", TRUE);

#undef SET_ALWAYS_SHOW_IMAGE
}

void
drawable_actions_update (PicmanActionGroup *group,
                         gpointer         data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable     = NULL;
  gboolean      is_rgb       = FALSE;
  gboolean      visible      = FALSE;
  gboolean      linked       = FALSE;
  gboolean      locked       = FALSE;
  gboolean      can_lock     = FALSE;
  gboolean      locked_pos   = FALSE;
  gboolean      can_lock_pos = FALSE;
  gboolean      writable     = FALSE;
  gboolean      movable      = FALSE;
  gboolean      children     = FALSE;

  image = action_data_get_image (data);

  if (image)
    {
      drawable = picman_image_get_active_drawable (image);

      if (drawable)
        {
          PicmanItem *item;

          is_rgb = picman_drawable_is_rgb (drawable);

          if (PICMAN_IS_LAYER_MASK (drawable))
            item = PICMAN_ITEM (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));
          else
            item = PICMAN_ITEM (drawable);

          visible       = picman_item_get_visible (item);
          linked        = picman_item_get_linked (item);
          locked        = picman_item_get_lock_content (item);
          can_lock      = picman_item_can_lock_content (item);
          writable      = ! picman_item_is_content_locked (item);
          locked_pos    = picman_item_get_lock_position (item);
          can_lock_pos  = picman_item_can_lock_position (item);
          movable       = ! picman_item_is_position_locked (item);

          if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
            children = TRUE;
        }
    }

#define SET_SENSITIVE(action,condition) \
        picman_action_group_set_action_sensitive (group, action, (condition) != 0)
#define SET_ACTIVE(action,condition) \
        picman_action_group_set_action_active (group, action, (condition) != 0)

  SET_SENSITIVE ("drawable-equalize",       writable && !children);
  SET_SENSITIVE ("drawable-invert",         writable && !children);
  SET_SENSITIVE ("drawable-value-invert",   writable && !children);
  SET_SENSITIVE ("drawable-levels-stretch", writable && !children && is_rgb);
  SET_SENSITIVE ("drawable-offset",         writable && !children);

  SET_SENSITIVE ("drawable-visible",       drawable);
  SET_SENSITIVE ("drawable-linked",        drawable);
  SET_SENSITIVE ("drawable-lock-content",  can_lock);
  SET_SENSITIVE ("drawable-lock-position", can_lock_pos);

  SET_ACTIVE ("drawable-visible",       visible);
  SET_ACTIVE ("drawable-linked",        linked);
  SET_ACTIVE ("drawable-lock-content",  locked);
  SET_ACTIVE ("drawable-lock-position", locked_pos);

  SET_SENSITIVE ("drawable-flip-horizontal", writable && movable);
  SET_SENSITIVE ("drawable-flip-vertical",   writable && movable);

  SET_SENSITIVE ("drawable-rotate-90",  writable && movable);
  SET_SENSITIVE ("drawable-rotate-180", writable && movable);
  SET_SENSITIVE ("drawable-rotate-270", writable && movable);

#undef SET_SENSITIVE
#undef SET_ACTIVE
}
