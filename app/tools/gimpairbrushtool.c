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

#include "paint/picmanairbrushoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"

#include "picmanairbrushtool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static GtkWidget * picman_airbrush_options_gui (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanAirbrushTool, picman_airbrush_tool, PICMAN_TYPE_PAINTBRUSH_TOOL)


void
picman_airbrush_tool_register (PicmanToolRegisterCallback  callback,
                             gpointer                  data)
{
  (* callback) (PICMAN_TYPE_AIRBRUSH_TOOL,
                PICMAN_TYPE_AIRBRUSH_OPTIONS,
                picman_airbrush_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_GRADIENT_MASK,
                "picman-airbrush-tool",
                _("Airbrush"),
                _("Airbrush Tool: Paint using a brush, with variable pressure"),
                N_("_Airbrush"), "A",
                NULL, PICMAN_HELP_TOOL_AIRBRUSH,
                PICMAN_STOCK_TOOL_AIRBRUSH,
                data);
}

static void
picman_airbrush_tool_class_init (PicmanAirbrushToolClass *klass)
{
}

static void
picman_airbrush_tool_init (PicmanAirbrushTool *airbrush)
{
  PicmanTool *tool = PICMAN_TOOL (airbrush);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_AIRBRUSH);
}


/*  tool options stuff  */

static GtkWidget *
picman_airbrush_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *button;
  GtkWidget *scale;

  button = picman_prop_check_button_new (config, "motion-only", _("Motion only"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  scale = picman_prop_spin_scale_new (config, "rate",
                                    C_("airbrush-tool", "Rate"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  scale = picman_prop_spin_scale_new (config, "flow",
                                    _("Flow"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return vbox;
}
