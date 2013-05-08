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

#include "paint/picmansmudgeoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"

#include "picmansmudgetool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static GtkWidget * picman_smudge_options_gui (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanSmudgeTool, picman_smudge_tool, PICMAN_TYPE_BRUSH_TOOL)


void
picman_smudge_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_SMUDGE_TOOL,
                PICMAN_TYPE_SMUDGE_OPTIONS,
                picman_smudge_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK,
                "picman-smudge-tool",
                _("Smudge"),
                _("Smudge Tool: Smudge selectively using a brush"),
                N_("_Smudge"), "S",
                NULL, PICMAN_HELP_TOOL_SMUDGE,
                PICMAN_STOCK_TOOL_SMUDGE,
                data);
}

static void
picman_smudge_tool_class_init (PicmanSmudgeToolClass *klass)
{
}

static void
picman_smudge_tool_init (PicmanSmudgeTool *smudge)
{
  PicmanTool      *tool       = PICMAN_TOOL (smudge);
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (smudge);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_SMUDGE);

  paint_tool->status      = _("Click to smudge");
  paint_tool->status_line = _("Click to smudge the line");
  paint_tool->status_ctrl = NULL;
}


/*  tool options stuff  */

static GtkWidget *
picman_smudge_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *scale;

  /*  the rate scale  */
  scale = picman_prop_spin_scale_new (config, "rate",
                                    C_("smudge-tool", "Rate"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return vbox;
}
