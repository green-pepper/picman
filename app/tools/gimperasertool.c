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

#include "paint/picmaneraseroptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanerasertool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_eraser_tool_modifier_key  (PicmanTool         *tool,
                                              GdkModifierType   key,
                                              gboolean          press,
                                              GdkModifierType   state,
                                              PicmanDisplay      *display);
static void   picman_eraser_tool_cursor_update (PicmanTool         *tool,
                                              const PicmanCoords *coords,
                                              GdkModifierType   state,
                                              PicmanDisplay      *display);

static GtkWidget * picman_eraser_options_gui   (PicmanToolOptions *tool_options);


G_DEFINE_TYPE (PicmanEraserTool, picman_eraser_tool, PICMAN_TYPE_BRUSH_TOOL)

#define parent_class picman_eraser_tool_parent_class


void
picman_eraser_tool_register (PicmanToolRegisterCallback  callback,
                           gpointer                  data)
{
  (* callback) (PICMAN_TYPE_ERASER_TOOL,
                PICMAN_TYPE_ERASER_OPTIONS,
                picman_eraser_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK,
                "picman-eraser-tool",
                _("Eraser"),
                _("Eraser Tool: Erase to background or transparency using a brush"),
                N_("_Eraser"), "<shift>E",
                NULL, PICMAN_HELP_TOOL_ERASER,
                PICMAN_STOCK_TOOL_ERASER,
                data);
}

static void
picman_eraser_tool_class_init (PicmanEraserToolClass *klass)
{
  PicmanToolClass *tool_class = PICMAN_TOOL_CLASS (klass);

  tool_class->modifier_key  = picman_eraser_tool_modifier_key;
  tool_class->cursor_update = picman_eraser_tool_cursor_update;
}

static void
picman_eraser_tool_init (PicmanEraserTool *eraser)
{
  PicmanTool      *tool       = PICMAN_TOOL (eraser);
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (eraser);

  picman_tool_control_set_tool_cursor            (tool->control,
                                                PICMAN_TOOL_CURSOR_ERASER);
  picman_tool_control_set_toggle_cursor_modifier (tool->control,
                                                PICMAN_CURSOR_MODIFIER_MINUS);

  picman_paint_tool_enable_color_picker (paint_tool,
                                       PICMAN_COLOR_PICK_MODE_BACKGROUND);

  paint_tool->status      = _("Click to erase");
  paint_tool->status_line = _("Click to erase the line");
  paint_tool->status_ctrl = _("%s to pick a background color");
}

static void
picman_eraser_tool_modifier_key (PicmanTool        *tool,
                               GdkModifierType  key,
                               gboolean         press,
                               GdkModifierType  state,
                               PicmanDisplay     *display)
{
  if (key == GDK_MOD1_MASK)
    {
      PicmanEraserOptions *options = PICMAN_ERASER_TOOL_GET_OPTIONS (tool);

      g_object_set (options,
                    "anti-erase", ! options->anti_erase,
                    NULL);
    }

  PICMAN_TOOL_CLASS (parent_class)->modifier_key (tool, key, press, state, display);
}

static void
picman_eraser_tool_cursor_update (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                GdkModifierType   state,
                                PicmanDisplay      *display)
{
  PicmanEraserOptions *options = PICMAN_ERASER_TOOL_GET_OPTIONS (tool);

  picman_tool_control_set_toggled (tool->control, options->anti_erase);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}


/*  tool options stuff  */

static GtkWidget *
picman_eraser_options_gui (PicmanToolOptions *tool_options)
{
  GObject   *config = G_OBJECT (tool_options);
  GtkWidget *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget *button;
  gchar     *str;

  /* the anti_erase toggle */
  str = g_strdup_printf (_("Anti erase  (%s)"),
                         picman_get_mod_string (GDK_MOD1_MASK));

  button = picman_prop_check_button_new (config, "anti-erase", str);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_free (str);

  return vbox;
}
