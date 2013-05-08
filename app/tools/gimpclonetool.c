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

#include "paint/picmanclone.h"
#include "paint/picmancloneoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"

#include "picmanclonetool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static GtkWidget * picman_clone_options_gui (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanCloneTool, picman_clone_tool, PICMAN_TYPE_SOURCE_TOOL)

#define parent_class picman_clone_tool_parent_class


void
picman_clone_tool_register (PicmanToolRegisterCallback  callback,
                          gpointer                  data)
{
  (* callback) (PICMAN_TYPE_CLONE_TOOL,
                PICMAN_TYPE_CLONE_OPTIONS,
                picman_clone_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK |
                PICMAN_CONTEXT_PATTERN_MASK,
                "picman-clone-tool",
                _("Clone"),
                _("Clone Tool: Selectively copy from an image or pattern, using a brush"),
                N_("_Clone"), "C",
                NULL, PICMAN_HELP_TOOL_CLONE,
                PICMAN_STOCK_TOOL_CLONE,
                data);
}

static void
picman_clone_tool_class_init (PicmanCloneToolClass *klass)
{
}

static void
picman_clone_tool_init (PicmanCloneTool *clone)
{
  PicmanTool       *tool        = PICMAN_TOOL (clone);
  PicmanPaintTool  *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);

  picman_tool_control_set_tool_cursor     (tool->control,
                                         PICMAN_TOOL_CURSOR_CLONE);
  picman_tool_control_set_action_object_2 (tool->control,
                                         "context/context-pattern-select-set");

  paint_tool->status      = _("Click to clone");
  paint_tool->status_ctrl = _("%s to set a new clone source");

  source_tool->status_paint           = _("Click to clone");
  /* Translators: the translation of "Click" must be the first word */
  source_tool->status_set_source      = _("Click to set a new clone source");
  source_tool->status_set_source_ctrl = _("%s to set a new clone source");
}


/*  tool options stuff  */

static GtkWidget *
picman_clone_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *hbox;
  GtkWidget *combo;
  GtkWidget *label;

  frame = picman_prop_enum_radio_frame_new (config, "clone-type",
                                          _("Source"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = picman_prop_check_button_new (config, "sample-merged",
                                       _("Sample merged"));
  picman_enum_radio_frame_add (GTK_FRAME (frame), button,
                             PICMAN_IMAGE_CLONE, TRUE);

  hbox = picman_prop_pattern_box_new (NULL, PICMAN_CONTEXT (tool_options),
                                    NULL, 2,
                                    "pattern-view-type", "pattern-view-size");
  picman_enum_radio_frame_add (GTK_FRAME (frame), hbox,
                             PICMAN_PATTERN_CLONE, TRUE);

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
