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

#include "paint/picmansourceoptions.h"

#include "widgets/picmanhelp-ids.h"

#include "picmanhealtool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static GtkWidget * picman_heal_options_gui (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanHealTool, picman_heal_tool, PICMAN_TYPE_SOURCE_TOOL)


void
picman_heal_tool_register (PicmanToolRegisterCallback  callback,
                         gpointer                  data)
{
  (* callback) (PICMAN_TYPE_HEAL_TOOL,
                PICMAN_TYPE_SOURCE_OPTIONS,
                picman_heal_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK,
                "picman-heal-tool",
                _("Heal"),
                _("Healing Tool: Heal image irregularities"),
                N_("_Heal"),
                "H",
                NULL,
                PICMAN_HELP_TOOL_HEAL,
                PICMAN_STOCK_TOOL_HEAL,
                data);
}

static void
picman_heal_tool_class_init (PicmanHealToolClass *klass)
{
}

static void
picman_heal_tool_init (PicmanHealTool *heal)
{
  PicmanTool       *tool        = PICMAN_TOOL (heal);
  PicmanPaintTool  *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);

  picman_tool_control_set_tool_cursor (tool->control, PICMAN_TOOL_CURSOR_HEAL);

  paint_tool->status      = _("Click to heal");
  paint_tool->status_ctrl = _("%s to set a new heal source");

  source_tool->status_paint           = _("Click to heal");
  /* Translators: the translation of "Click" must be the first word */
  source_tool->status_set_source      = _("Click to set a new heal source");
  source_tool->status_set_source_ctrl = _("%s to set a new heal source");
}


/*  tool options stuff  */

static GtkWidget *
picman_heal_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;

  /* the sample merged checkbox */
  button = picman_prop_check_button_new (config, "sample-merged",
                                       _("Sample merged"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  /* the alignment combo */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Alignment:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_prop_enum_combo_box_new (config, "align-mode", 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  return vbox;
}
