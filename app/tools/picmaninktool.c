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

#include "tools-types.h"

#include "paint/picmaninkoptions.h"

#include "widgets/picmanhelp-ids.h"

#include "picmaninkoptions-gui.h"
#include "picmaninktool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


G_DEFINE_TYPE (PicmanInkTool, picman_ink_tool, PICMAN_TYPE_PAINT_TOOL)

#define parent_class picman_ink_tool_parent_class


void
picman_ink_tool_register (PicmanToolRegisterCallback  callback,
                        gpointer                  data)
{
  (* callback) (PICMAN_TYPE_INK_TOOL,
                PICMAN_TYPE_INK_OPTIONS,
                picman_ink_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK |
                PICMAN_CONTEXT_BACKGROUND_MASK |
                PICMAN_CONTEXT_OPACITY_MASK    |
                PICMAN_CONTEXT_PAINT_MODE_MASK,
                "picman-ink-tool",
                _("Ink"),
                _("Ink Tool: Calligraphy-style painting"),
                N_("In_k"), "K",
                NULL, PICMAN_HELP_TOOL_INK,
                PICMAN_STOCK_TOOL_INK,
                data);
}

static void
picman_ink_tool_class_init (PicmanInkToolClass *klass)
{
}

static void
picman_ink_tool_init (PicmanInkTool *ink_tool)
{
  PicmanTool *tool = PICMAN_TOOL (ink_tool);

  picman_tool_control_set_tool_cursor    (tool->control, PICMAN_TOOL_CURSOR_INK);
  picman_tool_control_set_action_value_2 (tool->control,
                                        "tools/tools-ink-blob-size-set");
  picman_tool_control_set_action_value_3 (tool->control,
                                        "tools/tools-ink-blob-aspect-set");
  picman_tool_control_set_action_value_4 (tool->control,
                                        "tools/tools-ink-blob-angle-set");

  picman_paint_tool_enable_color_picker (PICMAN_PAINT_TOOL (ink_tool),
                                       PICMAN_COLOR_PICK_MODE_FOREGROUND);
}
