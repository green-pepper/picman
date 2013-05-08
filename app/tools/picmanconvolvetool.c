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

#include "paint/picmanconvolveoptions.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanwidgets-utils.h"

#include "picmanconvolvetool.h"
#include "picmanpaintoptions-gui.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_convolve_tool_modifier_key  (PicmanTool         *tool,
                                                GdkModifierType   key,
                                                gboolean          press,
                                                GdkModifierType   state,
                                                PicmanDisplay      *display);
static void   picman_convolve_tool_cursor_update (PicmanTool         *tool,
                                                const PicmanCoords *coords,
                                                GdkModifierType   state,
                                                PicmanDisplay      *display);
static void   picman_convolve_tool_oper_update   (PicmanTool         *tool,
                                                const PicmanCoords *coords,
                                                GdkModifierType   state,
                                                gboolean          proximity,
                                                PicmanDisplay      *display);
static void   picman_convolve_tool_status_update (PicmanTool         *tool,
                                                PicmanConvolveType  type);

static GtkWidget * picman_convolve_options_gui   (PicmanToolOptions  *options);


G_DEFINE_TYPE (PicmanConvolveTool, picman_convolve_tool, PICMAN_TYPE_BRUSH_TOOL)

#define parent_class picman_convolve_tool_parent_class


void
picman_convolve_tool_register (PicmanToolRegisterCallback  callback,
                             gpointer                  data)
{
  (* callback) (PICMAN_TYPE_CONVOLVE_TOOL,
                PICMAN_TYPE_CONVOLVE_OPTIONS,
                picman_convolve_options_gui,
                PICMAN_PAINT_OPTIONS_CONTEXT_MASK,
                "picman-convolve-tool",
                _("Blur / Sharpen"),
                _("Blur / Sharpen Tool: Selective blurring or unblurring using a brush"),
                N_("Bl_ur / Sharpen"), "<shift>U",
                NULL, PICMAN_HELP_TOOL_CONVOLVE,
                PICMAN_STOCK_TOOL_BLUR,
                data);
}

static void
picman_convolve_tool_class_init (PicmanConvolveToolClass *klass)
{
  PicmanToolClass *tool_class = PICMAN_TOOL_CLASS (klass);

  tool_class->modifier_key  = picman_convolve_tool_modifier_key;
  tool_class->cursor_update = picman_convolve_tool_cursor_update;
  tool_class->oper_update   = picman_convolve_tool_oper_update;
}

static void
picman_convolve_tool_init (PicmanConvolveTool *convolve)
{
  PicmanTool *tool = PICMAN_TOOL (convolve);

  picman_tool_control_set_tool_cursor            (tool->control,
                                                PICMAN_TOOL_CURSOR_BLUR);
  picman_tool_control_set_toggle_cursor_modifier (tool->control,
                                                PICMAN_CURSOR_MODIFIER_MINUS);

  picman_convolve_tool_status_update (tool, PICMAN_BLUR_CONVOLVE);
}

static void
picman_convolve_tool_modifier_key (PicmanTool        *tool,
                                 GdkModifierType  key,
                                 gboolean         press,
                                 GdkModifierType  state,
                                 PicmanDisplay     *display)
{
  PicmanConvolveTool    *convolve = PICMAN_CONVOLVE_TOOL (tool);
  PicmanConvolveOptions *options  = PICMAN_CONVOLVE_TOOL_GET_OPTIONS (tool);
  GdkModifierType      toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  if (((key == toggle_mask)       &&
       ! (state & GDK_SHIFT_MASK) && /* leave stuff untouched in line draw mode */
       press != convolve->toggled)

      ||

      (key == GDK_SHIFT_MASK && /* toggle back after keypresses CTRL(hold)->  */
       ! press               && /* SHIFT(hold)->CTRL(release)->SHIFT(release) */
       convolve->toggled     &&
       ! (state & toggle_mask)))
    {
      convolve->toggled = press;

      switch (options->type)
        {
        case PICMAN_BLUR_CONVOLVE:
          g_object_set (options, "type", PICMAN_SHARPEN_CONVOLVE, NULL);
          break;

        case PICMAN_SHARPEN_CONVOLVE:
          g_object_set (options, "type", PICMAN_BLUR_CONVOLVE, NULL);
          break;

        default:
          break;
        }
    }
}

static void
picman_convolve_tool_cursor_update (PicmanTool         *tool,
                                  const PicmanCoords *coords,
                                  GdkModifierType   state,
                                  PicmanDisplay      *display)
{
  PicmanConvolveOptions *options = PICMAN_CONVOLVE_TOOL_GET_OPTIONS (tool);

  picman_tool_control_set_toggled (tool->control,
                                 (options->type == PICMAN_SHARPEN_CONVOLVE));

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_convolve_tool_oper_update (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                GdkModifierType   state,
                                gboolean          proximity,
                                PicmanDisplay      *display)
{
  PicmanConvolveOptions *options = PICMAN_CONVOLVE_TOOL_GET_OPTIONS (tool);

  picman_convolve_tool_status_update (tool, options->type);

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);
}

static void
picman_convolve_tool_status_update (PicmanTool         *tool,
                                  PicmanConvolveType  type)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);

  switch (type)
    {
    case PICMAN_BLUR_CONVOLVE:
      paint_tool->status      = _("Click to blur");
      paint_tool->status_line = _("Click to blur the line");
      paint_tool->status_ctrl = _("%s to sharpen");
      break;

    case PICMAN_SHARPEN_CONVOLVE:
      paint_tool->status      = _("Click to sharpen");
      paint_tool->status_line = _("Click to sharpen the line");
      paint_tool->status_ctrl = _("%s to blur");
      break;

    default:
      break;
    }
}


/*  tool options stuff  */

static GtkWidget *
picman_convolve_options_gui (PicmanToolOptions *tool_options)
{
  GObject         *config = G_OBJECT (tool_options);
  GtkWidget       *vbox   = picman_paint_options_gui (tool_options);
  GtkWidget       *frame;
  GtkWidget       *scale;
  gchar           *str;
  GdkModifierType  toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  /*  the type radio box  */
  str = g_strdup_printf (_("Convolve Type  (%s)"),
                         picman_get_mod_string (toggle_mask));

  frame = picman_prop_enum_radio_frame_new (config, "type",
                                          str, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  g_free (str);

  /*  the rate scale  */
  scale = picman_prop_spin_scale_new (config, "rate",
                                    C_("convolve-tool", "Rate"),
                                    1.0, 10.0, 1);
  gtk_box_pack_start (GTK_BOX (vbox), scale, FALSE, FALSE, 0);
  gtk_widget_show (scale);

  return vbox;
}
