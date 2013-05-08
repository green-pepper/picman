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

#include "paint/picmandodgeburnoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmandodgeburntool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_dodge_burn_tool_modifier_key  (PicmanTool          *tool,
                                                  GdkModifierType    key,
                                                  gboolean           press,
                                                  GdkModifierType    state,
                                                  PicmanDisplay       *display);
static void   picman_dodge_burn_tool_cursor_update (PicmanTool          *tool,
                                                  const PicmanCoords  *coords,
                                                  GdkModifierType    state,
                                                  PicmanDisplay       *display);
static void   picman_dodge_burn_tool_oper_update   (PicmanTool          *tool,
                                                  const PicmanCoords  *coords,
                                                  GdkModifierType    state,
                                                  gboolean           proximity,
                                                  PicmanDisplay       *display);
static void   picman_dodge_burn_tool_status_update (PicmanTool          *tool,
                                                  PicmanDodgeBurnType  type);

static GtkWidget * picman_dodge_burn_options_gui   (PicmanToolOptions   *tool_options);


G_DEFINE_TYPE (PicmanDodgeBurnTool, picman_dodge_burn_tool, PICMAN_TYPE_BRUSH_TOOL)

#define parent_class picman_dodge_burn_tool_parent_class


void
picman_dodge_burn_tool_register (PicmanToolRegisterCallback  callback,
                               gpointer                  data)
{
  (* callback) (PICMAN_TYPE_DODGE_BURN_TOOL,
                PICMAN_TYPE_DODGE_BURN_OPTIONS,
                picman_dodge_burn_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK,
                "picman-dodge-burn-tool",
                _("Dodge / Burn"),
                _("Dodge / Burn Tool: Selectively lighten or darken using a brush"),
                N_("Dod_ge / Burn"), "<shift>D",
                NULL, PICMAN_HELP_TOOL_DODGE_BURN,
                PICMAN_STOCK_TOOL_DODGE,
                data);
}

static void
picman_dodge_burn_tool_class_init (PicmanDodgeBurnToolClass *klass)
{
  PicmanToolClass *tool_class = PICMAN_TOOL_CLASS (klass);

  tool_class->modifier_key  = picman_dodge_burn_tool_modifier_key;
  tool_class->cursor_update = picman_dodge_burn_tool_cursor_update;
  tool_class->oper_update   = picman_dodge_burn_tool_oper_update;
}

static void
picman_dodge_burn_tool_init (PicmanDodgeBurnTool *dodgeburn)
{
  PicmanTool *tool = PICMAN_TOOL (dodgeburn);

  picman_tool_control_set_tool_cursor        (tool->control,
                                            PICMAN_TOOL_CURSOR_DODGE);
  picman_tool_control_set_toggle_tool_cursor (tool->control,
                                            PICMAN_TOOL_CURSOR_BURN);

  picman_dodge_burn_tool_status_update (tool, PICMAN_BURN);
}

static void
picman_dodge_burn_tool_modifier_key (PicmanTool        *tool,
                                   GdkModifierType  key,
                                   gboolean         press,
                                   GdkModifierType  state,
                                   PicmanDisplay     *display)
{
  PicmanDodgeBurnTool    *dodgeburn = PICMAN_DODGE_BURN_TOOL (tool);
  PicmanDodgeBurnOptions *options   = PICMAN_DODGE_BURN_TOOL_GET_OPTIONS (tool);
  GdkModifierType       toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  if ((key == toggle_mask        &&
      ! (state & GDK_SHIFT_MASK) && /* leave stuff untouched in line draw mode */
       press != dodgeburn->toggled)

      ||

      (key == GDK_SHIFT_MASK && /* toggle back after keypresses CTRL(hold)->  */
       ! press               && /* SHIFT(hold)->CTRL(release)->SHIFT(release) */
       dodgeburn->toggled    &&
       ! (state & toggle_mask)))
    {
      dodgeburn->toggled = press;

      switch (options->type)
        {
        case PICMAN_DODGE:
          g_object_set (options, "type", PICMAN_BURN, NULL);
          break;

        case PICMAN_BURN:
          g_object_set (options, "type", PICMAN_DODGE, NULL);
          break;

        default:
          break;
        }
    }

  PICMAN_TOOL_CLASS (parent_class)->modifier_key (tool, key, press, state,
                                                display);
}

static void
picman_dodge_burn_tool_cursor_update (PicmanTool         *tool,
                                    const PicmanCoords *coords,
                                    GdkModifierType   state,
                                    PicmanDisplay      *display)
{
  PicmanDodgeBurnOptions *options = PICMAN_DODGE_BURN_TOOL_GET_OPTIONS (tool);

  picman_tool_control_set_toggled (tool->control, (options->type == PICMAN_BURN));

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state,
                                                 display);
}

static void
picman_dodge_burn_tool_oper_update (PicmanTool         *tool,
                                  const PicmanCoords *coords,
                                  GdkModifierType   state,
                                  gboolean          proximity,
                                  PicmanDisplay      *display)
{
  PicmanDodgeBurnOptions *options = PICMAN_DODGE_BURN_TOOL_GET_OPTIONS (tool);

  picman_dodge_burn_tool_status_update (tool, options->type);

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);
}

static void
picman_dodge_burn_tool_status_update (PicmanTool          *tool,
                                    PicmanDodgeBurnType  type)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);

  switch (type)
    {
    case PICMAN_DODGE:
      paint_tool->status      = _("Click to dodge");
      paint_tool->status_line = _("Click to dodge the line");
      paint_tool->status_ctrl = _("%s to burn");
      break;

    case PICMAN_BURN:
      paint_tool->status      = _("Click to burn");
      paint_tool->status_line = _("Click to burn the line");
      paint_tool->status_ctrl = _("%s to dodge");
      break;

    default:
      break;
    }
}


/*  tool options stuff  */

static GtkWidget *
picman_dodge_burn_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget       *frame;
  GtkWidget       *scale;
  gchar           *str;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  /* the type (dodge or burn) */
  str = g_strdup_printf (_("Type  (%s)"),
                         picman_get_mod_string (toggle_mask));

  frame = picman_prop_enum_radio_frame_new (config, "type",
                                          str, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_free (str);

  /*  mode (highlights, midtones, or shadows)  */
  frame = picman_prop_enum_radio_frame_new (config, "mode", _("Range"), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  the exposure scale  */
  scale = picman_prop_spin_scale_new (config, "exposure",
                                    _("Exposure"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return vbox;
}
