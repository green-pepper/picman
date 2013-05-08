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

#include "core/picmanchannel-select.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "picmanellipseselecttool.h"
#include "picmanrectangleselectoptions.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_ellipse_select_tool_draw   (PicmanDrawTool            *draw_tool);

static void   picman_ellipse_select_tool_select (PicmanRectangleSelectTool *rect_tool,
                                               PicmanChannelOps           operation,
                                               gint                     x,
                                               gint                     y,
                                               gint                     w,
                                               gint                     h);


G_DEFINE_TYPE (PicmanEllipseSelectTool, picman_ellipse_select_tool,
               PICMAN_TYPE_RECTANGLE_SELECT_TOOL)

#define parent_class picman_ellipse_select_tool_parent_class


void
picman_ellipse_select_tool_register (PicmanToolRegisterCallback  callback,
                                   gpointer                  data)
{
  (* callback) (PICMAN_TYPE_ELLIPSE_SELECT_TOOL,
                PICMAN_TYPE_RECTANGLE_SELECT_OPTIONS,
                picman_rectangle_select_options_gui,
                0,
                "picman-ellipse-select-tool",
                _("Ellipse Select"),
                _("Ellipse Select Tool: Select an elliptical region"),
                N_("_Ellipse Select"), "E",
                NULL, PICMAN_HELP_TOOL_ELLIPSE_SELECT,
                PICMAN_STOCK_TOOL_ELLIPSE_SELECT,
                data);
}

static void
picman_ellipse_select_tool_class_init (PicmanEllipseSelectToolClass *klass)
{
  PicmanDrawToolClass            *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);
  PicmanRectangleSelectToolClass *rect_tool_class = PICMAN_RECTANGLE_SELECT_TOOL_CLASS (klass);

  draw_tool_class->draw   = picman_ellipse_select_tool_draw;

  rect_tool_class->select = picman_ellipse_select_tool_select;
}

static void
picman_ellipse_select_tool_init (PicmanEllipseSelectTool *ellipse_select)
{
  PicmanTool *tool = PICMAN_TOOL (ellipse_select);

  picman_tool_control_set_tool_cursor (tool->control,
                                     PICMAN_TOOL_CURSOR_ELLIPSE_SELECT);
}

static void
picman_ellipse_select_tool_draw (PicmanDrawTool *draw_tool)
{
  gint x1, y1, x2, y2;

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);

  g_object_get (draw_tool,
                "x1", &x1,
                "y1", &y1,
                "x2", &x2,
                "y2", &y2,
                NULL);

  picman_draw_tool_add_arc (draw_tool,
                          FALSE,
                          x1, y1,
                          x2 - x1, y2 - y1,
                          0.0, 2 * G_PI);
}

static void
picman_ellipse_select_tool_select (PicmanRectangleSelectTool *rect_tool,
                                 PicmanChannelOps           operation,
                                 gint                     x,
                                 gint                     y,
                                 gint                     w,
                                 gint                     h)
{
  PicmanTool             *tool    = PICMAN_TOOL (rect_tool);
  PicmanSelectionOptions *options = PICMAN_SELECTION_TOOL_GET_OPTIONS (rect_tool);
  PicmanImage            *image   = picman_display_get_image (tool->display);

  picman_channel_select_ellipse (picman_image_get_mask (image),
                               x, y, w, h,
                               operation,
                               options->antialias,
                               options->feather,
                               options->feather_radius,
                               options->feather_radius,
                               TRUE);
}
