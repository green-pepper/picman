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

#include "paint/picmanpenciloptions.h"

#include "widgets/picmanhelp-ids.h"

#include "picmanpenciltool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


G_DEFINE_TYPE (PicmanPencilTool, picman_pencil_tool, PICMAN_TYPE_PAINTBRUSH_TOOL)


void
picman_pencil_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_PENCIL_TOOL,
                PICMAN_TYPE_PENCIL_OPTIONS,
                picman_paint_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_GRADIENT_MASK,
                "picman-pencil-tool",
                _("Pencil"),
                _("Pencil Tool: Hard edge painting using a brush"),
                N_("Pe_ncil"), "N",
                NULL, PICMAN_HELP_TOOL_PENCIL,
                PICMAN_STOCK_TOOL_PENCIL,
                data);
}

static void
picman_pencil_tool_class_init (PicmanPencilToolClass *klass)
{
}

static void
picman_pencil_tool_init (PicmanPencilTool *pencil)
{
  PicmanTool *tool = PICMAN_TOOL (pencil);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_PENCIL);
}
