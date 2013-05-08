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

#include "paint/picmanpaintoptions.h"

#include "widgets/picmanhelp-ids.h"

#include "picmanpaintbrushtool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


G_DEFINE_TYPE (PicmanPaintbrushTool, picman_paintbrush_tool, PICMAN_TYPE_BRUSH_TOOL)


void
picman_paintbrush_tool_register (PicmanToolRegisterCallback  callback,
                               gpointer                  data)
{
  (* callback) (PICMAN_TYPE_PAINTBRUSH_TOOL,
                PICMAN_TYPE_PAINT_OPTIONS,
                picman_paint_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_GRADIENT_MASK,
                "picman-paintbrush-tool",
                _("Paintbrush"),
                _("Paintbrush Tool: Paint smooth strokes using a brush"),
                N_("_Paintbrush"), "P",
                NULL, PICMAN_HELP_TOOL_PAINTBRUSH,
                PICMAN_STOCK_TOOL_PAINTBRUSH,
                data);
}

static void
picman_paintbrush_tool_class_init (PicmanPaintbrushToolClass *klass)
{
}

static void
picman_paintbrush_tool_init (PicmanPaintbrushTool *paintbrush)
{
  PicmanTool *tool = PICMAN_TOOL (paintbrush);

  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_PAINTBRUSH);

  picman_paint_tool_enable_color_picker (PICMAN_PAINT_TOOL (paintbrush),
                                       PICMAN_COLOR_PICK_MODE_FOREGROUND);
}
