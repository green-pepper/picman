/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfuzzyselecttool.c
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmanimage.h"
#include "core/picmanimage-contiguous-region.h"
#include "core/picmanitem.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanfuzzyselecttool.h"
#include "picmanregionselectoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static GeglBuffer * picman_fuzzy_select_tool_get_mask (PicmanRegionSelectTool *region_select,
                                                     PicmanDisplay          *display);


G_DEFINE_TYPE (PicmanFuzzySelectTool, picman_fuzzy_select_tool,
               PICMAN_TYPE_REGION_SELECT_TOOL)

#define parent_class picman_fuzzy_select_tool_parent_class


void
picman_fuzzy_select_tool_register (PicmanToolRegisterCallback  callback,
                                 gpointer                  data)
{
  (* callback) (PICMAN_TYPE_FUZZY_SELECT_TOOL,
                PICMAN_TYPE_REGION_SELECT_OPTIONS,
                picman_region_select_options_gui,
                0,
                "picman-fuzzy-select-tool",
                _("Fuzzy Select"),
                _("Fuzzy Select Tool: Select a contiguous region on the basis of color"),
                N_("Fu_zzy Select"), "U",
                NULL, PICMAN_HELP_TOOL_FUZZY_SELECT,
                PICMAN_STOCK_TOOL_FUZZY_SELECT,
                data);
}

static void
picman_fuzzy_select_tool_class_init (PicmanFuzzySelectToolClass *klass)
{
  PicmanRegionSelectToolClass *region_class;

  region_class = PICMAN_REGION_SELECT_TOOL_CLASS (klass);

  region_class->undo_desc = C_("command", "Fuzzy Select");
  region_class->get_mask  = picman_fuzzy_select_tool_get_mask;
}

static void
picman_fuzzy_select_tool_init (PicmanFuzzySelectTool *fuzzy_select)
{
  PicmanTool *tool = PICMAN_TOOL (fuzzy_select);

  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_FUZZY_SELECT);
}

static GeglBuffer *
picman_fuzzy_select_tool_get_mask (PicmanRegionSelectTool *region_select,
                                 PicmanDisplay          *display)
{
  PicmanTool                *tool        = PICMAN_TOOL (region_select);
  PicmanSelectionOptions    *sel_options = PICMAN_SELECTION_TOOL_GET_OPTIONS (tool);
  PicmanRegionSelectOptions *options     = PICMAN_REGION_SELECT_TOOL_GET_OPTIONS (tool);
  PicmanImage               *image       = picman_display_get_image (display);
  PicmanDrawable            *drawable    = picman_image_get_active_drawable (image);
  gint                     x, y;

  x = region_select->x;
  y = region_select->y;

  if (! options->sample_merged)
    {
      gint off_x, off_y;

      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

      x -= off_x;
      y -= off_y;
    }

  return picman_image_contiguous_region_by_seed (image, drawable,
                                               options->sample_merged,
                                               sel_options->antialias,
                                               options->threshold / 255.0,
                                               options->select_transparent,
                                               options->select_criterion,
                                               x, y);
}
