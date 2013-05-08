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

#include "core/picmanimage.h"
#include "core/picmanimage-quick-mask.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanhelp-ids.h"

#include "actions.h"
#include "quick-mask-actions.h"
#include "quick-mask-commands.h"

#include "picman-intl.h"


static const PicmanActionEntry quick_mask_actions[] =
{
  { "quick-mask-popup", NULL,
    NC_("quick-mask-action", "Quick Mask Menu"), NULL, NULL, NULL,
    PICMAN_HELP_QUICK_MASK },

  { "quick-mask-configure", NULL,
    NC_("quick-mask-action", "_Configure Color and Opacity..."), NULL, NULL,
    G_CALLBACK (quick_mask_configure_cmd_callback),
    PICMAN_HELP_QUICK_MASK_EDIT }
};

static const PicmanToggleActionEntry quick_mask_toggle_actions[] =
{
  { "quick-mask-toggle", PICMAN_STOCK_QUICK_MASK_ON,
    NC_("quick-mask-action", "Toggle _Quick Mask"), "<shift>Q",
    NC_("quick-mask-action", "Toggle Quick Mask on/off"),
    G_CALLBACK (quick_mask_toggle_cmd_callback),
    FALSE,
    PICMAN_HELP_QUICK_MASK_TOGGLE }
};

static const PicmanRadioActionEntry quick_mask_invert_actions[] =
{
  { "quick-mask-invert-on", NULL,
    NC_("quick-mask-action", "Mask _Selected Areas"), NULL, NULL,
    TRUE,
    PICMAN_HELP_QUICK_MASK_INVERT },

  { "quick-mask-invert-off", NULL,
    NC_("quick-mask-action", "Mask _Unselected Areas"), NULL, NULL,
    FALSE,
    PICMAN_HELP_QUICK_MASK_INVERT }
};


void
quick_mask_actions_setup (PicmanActionGroup *group)
{
  picman_action_group_add_actions (group, "quick-mask-action",
                                 quick_mask_actions,
                                 G_N_ELEMENTS (quick_mask_actions));

  picman_action_group_add_toggle_actions (group, "quick-mask-action",
                                        quick_mask_toggle_actions,
                                        G_N_ELEMENTS (quick_mask_toggle_actions));

  picman_action_group_add_radio_actions (group, "quick-mask-action",
                                       quick_mask_invert_actions,
                                       G_N_ELEMENTS (quick_mask_invert_actions),
                                       NULL,
                                       FALSE,
                                       G_CALLBACK (quick_mask_invert_cmd_callback));
}

void
quick_mask_actions_update (PicmanActionGroup *group,
                           gpointer         data)
{
  PicmanImage *image               = action_data_get_image (data);
  gboolean   quick_mask_state    = FALSE;
  gboolean   quick_mask_inverted = FALSE;
  PicmanRGB    quick_mask_color;

  if (image)
    {
      quick_mask_state    = picman_image_get_quick_mask_state (image);
      quick_mask_inverted = picman_image_get_quick_mask_inverted (image);

      picman_image_get_quick_mask_color (image, &quick_mask_color);
    }

#define SET_SENSITIVE(action,sensitive) \
        picman_action_group_set_action_sensitive (group, action, (sensitive) != 0)
#define SET_ACTIVE(action,active) \
        picman_action_group_set_action_active (group, action, (active) != 0)
#define SET_COLOR(action,color) \
        picman_action_group_set_action_color (group, action, (color), FALSE)

  SET_SENSITIVE ("quick-mask-toggle", image);
  SET_ACTIVE    ("quick-mask-toggle", quick_mask_state);

  SET_SENSITIVE ("quick-mask-invert-on",  image);
  SET_SENSITIVE ("quick-mask-invert-off", image);

  if (quick_mask_inverted)
    SET_ACTIVE ("quick-mask-invert-on", TRUE);
  else
    SET_ACTIVE ("quick-mask-invert-off", TRUE);

  SET_SENSITIVE ("quick-mask-configure", image);

  if (image)
    SET_COLOR ("quick-mask-configure", &quick_mask_color);

#undef SET_SENSITIVE
#undef SET_ACTIVE
#undef SET_COLOR
}
