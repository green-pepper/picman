/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmandrawable.h"

#include "widgets/picmancolorframe.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmantooldialog.h"

#include "picmancolorpickeroptions.h"
#include "picmancolorpickertool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   picman_color_picker_tool_constructed   (GObject             *object);

static void   picman_color_picker_tool_control       (PicmanTool            *tool,
                                                    PicmanToolAction       action,
                                                    PicmanDisplay         *display);
static void   picman_color_picker_tool_modifier_key  (PicmanTool            *tool,
                                                    GdkModifierType      key,
                                                    gboolean             press,
                                                    GdkModifierType      state,
                                                    PicmanDisplay         *display);
static void   picman_color_picker_tool_oper_update   (PicmanTool            *tool,
                                                    const PicmanCoords    *coords,
                                                    GdkModifierType      state,
                                                    gboolean             proximity,
                                                    PicmanDisplay         *display);

static void   picman_color_picker_tool_picked        (PicmanColorTool       *color_tool,
                                                    PicmanColorPickState   pick_state,
                                                    const Babl          *sample_format,
                                                    const PicmanRGB       *color,
                                                    gint                 color_index);

static void   picman_color_picker_tool_info_create   (PicmanColorPickerTool *picker_tool);
static void   picman_color_picker_tool_info_response (GtkWidget           *widget,
                                                    gint                 response_id,
                                                    PicmanColorPickerTool *picker_tool);
static void   picman_color_picker_tool_info_update   (PicmanColorPickerTool *picker_tool,
                                                    const Babl          *sample_format,
                                                    const PicmanRGB       *color,
                                                    gint                 color_index);


G_DEFINE_TYPE (PicmanColorPickerTool, picman_color_picker_tool,
               PICMAN_TYPE_COLOR_TOOL)

#define parent_class picman_color_picker_tool_parent_class


void
picman_color_picker_tool_register (PicmanToolRegisterCallback  callback,
                                 gpointer                  data)
{
  (* callback) (PICMAN_TYPE_COLOR_PICKER_TOOL,
                PICMAN_TYPE_COLOR_PICKER_OPTIONS,
                picman_color_picker_options_gui,
                PICMAN_CONTEXT_FOREGROUND_MASK | PICMAN_CONTEXT_BACKGROUND_MASK,
                "picman-color-picker-tool",
                _("Color Picker"),
                _("Color Picker Tool: Set colors from image pixels"),
                N_("C_olor Picker"), "O",
                NULL, PICMAN_HELP_TOOL_COLOR_PICKER,
                PICMAN_STOCK_TOOL_COLOR_PICKER,
                data);
}

static void
picman_color_picker_tool_class_init (PicmanColorPickerToolClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanToolClass      *tool_class       = PICMAN_TOOL_CLASS (klass);
  PicmanColorToolClass *color_tool_class = PICMAN_COLOR_TOOL_CLASS (klass);

  object_class->constructed = picman_color_picker_tool_constructed;

  tool_class->control       = picman_color_picker_tool_control;
  tool_class->modifier_key  = picman_color_picker_tool_modifier_key;
  tool_class->oper_update   = picman_color_picker_tool_oper_update;

  color_tool_class->picked  = picman_color_picker_tool_picked;
}

static void
picman_color_picker_tool_init (PicmanColorPickerTool *picker_tool)
{
  PicmanTool      *tool       = PICMAN_TOOL (picker_tool);
  PicmanColorTool *color_tool = PICMAN_COLOR_TOOL (picker_tool);

  picman_tool_control_set_motion_mode (tool->control, PICMAN_MOTION_MODE_COMPRESS);

  color_tool->pick_mode = PICMAN_COLOR_PICK_MODE_FOREGROUND;
}

static void
picman_color_picker_tool_constructed (GObject *object)
{
  PicmanTool *tool = PICMAN_TOOL (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_color_tool_enable (PICMAN_COLOR_TOOL (object),
                          PICMAN_COLOR_TOOL_GET_OPTIONS (tool));
}

static void
picman_color_picker_tool_control (PicmanTool       *tool,
                                PicmanToolAction  action,
                                PicmanDisplay    *display)
{
  PicmanColorPickerTool *picker_tool = PICMAN_COLOR_PICKER_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      if (picker_tool->dialog)
        {
          gtk_widget_destroy (picker_tool->dialog);

          picker_tool->dialog       = NULL;
          picker_tool->color_area   = NULL;
          picker_tool->color_frame1 = NULL;
          picker_tool->color_frame2 = NULL;
        }
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_color_picker_tool_modifier_key (PicmanTool        *tool,
                                     GdkModifierType  key,
                                     gboolean         press,
                                     GdkModifierType  state,
                                     PicmanDisplay     *display)
{
  PicmanColorPickerOptions *options = PICMAN_COLOR_PICKER_TOOL_GET_OPTIONS (tool);

  if (key == GDK_SHIFT_MASK)
    {
      g_object_set (options, "use-info-window", ! options->use_info_window,
                    NULL);
    }
  else if (key == picman_get_toggle_behavior_mask ())
    {
      switch (options->pick_mode)
        {
        case PICMAN_COLOR_PICK_MODE_FOREGROUND:
          g_object_set (options, "pick-mode", PICMAN_COLOR_PICK_MODE_BACKGROUND,
                        NULL);
          break;

        case PICMAN_COLOR_PICK_MODE_BACKGROUND:
          g_object_set (options, "pick-mode", PICMAN_COLOR_PICK_MODE_FOREGROUND,
                        NULL);
          break;

        default:
          break;
        }

    }
}

static void
picman_color_picker_tool_oper_update (PicmanTool         *tool,
                                    const PicmanCoords *coords,
                                    GdkModifierType   state,
                                    gboolean          proximity,
                                    PicmanDisplay      *display)
{
  PicmanColorPickerTool    *picker_tool = PICMAN_COLOR_PICKER_TOOL (tool);
  PicmanColorPickerOptions *options = PICMAN_COLOR_PICKER_TOOL_GET_OPTIONS (tool);
  GdkModifierType         toggle_mask;

  toggle_mask = picman_get_toggle_behavior_mask ();

  PICMAN_COLOR_TOOL (tool)->pick_mode = options->pick_mode;

  picman_tool_pop_status (tool, display);
  if (proximity)
    {
      gchar           *status_help = NULL;
      GdkModifierType  shift_mod = 0;

      if (! picker_tool->dialog)
        shift_mod = GDK_SHIFT_MASK;

      switch (options->pick_mode)
        {
        case PICMAN_COLOR_PICK_MODE_NONE:
          status_help = picman_suggest_modifiers (_("Click in any image to view"
                                                  " its color"),
                                                shift_mod & ~state,
                                                NULL, NULL, NULL);
          break;

        case PICMAN_COLOR_PICK_MODE_FOREGROUND:
          status_help = picman_suggest_modifiers (_("Click in any image to pick"
                                                  " the foreground color"),
                                                (shift_mod | toggle_mask) &
                                                ~state,
                                                NULL, NULL, NULL);
          break;

        case PICMAN_COLOR_PICK_MODE_BACKGROUND:
          status_help = picman_suggest_modifiers (_("Click in any image to pick"
                                                  " the background color"),
                                                (shift_mod | toggle_mask) &
                                                ~state,
                                                NULL, NULL, NULL);
          break;

        case PICMAN_COLOR_PICK_MODE_PALETTE:
          status_help = picman_suggest_modifiers (_("Click in any image to add"
                                                  " the color to the palette"),
                                                shift_mod & ~state,
                                                NULL, NULL, NULL);
          break;
        }

      if (status_help != NULL)
        {
          picman_tool_push_status (tool, display, "%s", status_help);
          g_free (status_help);
        }
    }

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);
}

static void
picman_color_picker_tool_picked (PicmanColorTool      *color_tool,
                               PicmanColorPickState  pick_state,
                               const Babl         *sample_format,
                               const PicmanRGB      *color,
                               gint                color_index)
{
  PicmanColorPickerTool    *picker_tool = PICMAN_COLOR_PICKER_TOOL (color_tool);
  PicmanColorPickerOptions *options;

  options = PICMAN_COLOR_PICKER_TOOL_GET_OPTIONS (color_tool);

  if (options->use_info_window && ! picker_tool->dialog)
    picman_color_picker_tool_info_create (picker_tool);

  if (picker_tool->dialog)
    picman_color_picker_tool_info_update (picker_tool, sample_format,
                                        color, color_index);

  PICMAN_COLOR_TOOL_CLASS (parent_class)->picked (color_tool, pick_state,
                                                sample_format, color,
                                                color_index);
}

static void
picman_color_picker_tool_info_create (PicmanColorPickerTool *picker_tool)
{
  PicmanTool  *tool = PICMAN_TOOL (picker_tool);
  GtkWidget *content_area;
  GtkWidget *hbox;
  GtkWidget *frame;
  PicmanRGB    color;

  g_return_if_fail (tool->drawable != NULL);

  picker_tool->dialog = picman_tool_dialog_new (tool->tool_info,
                                              picman_display_get_shell (tool->display),
                                              _("Color Picker Information"),

                                              GTK_STOCK_CLOSE,
                                              GTK_RESPONSE_CLOSE,

                                              NULL);

  gtk_window_set_focus_on_map (GTK_WINDOW (picker_tool->dialog), FALSE);

  picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (picker_tool->dialog),
                                     PICMAN_VIEWABLE (tool->drawable),
                                     PICMAN_CONTEXT (picman_tool_get_options (tool)));

  g_signal_connect (picker_tool->dialog, "response",
                    G_CALLBACK (picman_color_picker_tool_info_response),
                    picker_tool);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (picker_tool->dialog));

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  picker_tool->color_frame1 = picman_color_frame_new ();
  picman_color_frame_set_mode (PICMAN_COLOR_FRAME (picker_tool->color_frame1),
                             PICMAN_COLOR_FRAME_MODE_PIXEL);
  gtk_box_pack_start (GTK_BOX (hbox), picker_tool->color_frame1,
                      FALSE, FALSE, 0);
  gtk_widget_show (picker_tool->color_frame1);

  picker_tool->color_frame2 = picman_color_frame_new ();
  picman_color_frame_set_mode (PICMAN_COLOR_FRAME (picker_tool->color_frame2),
                             PICMAN_COLOR_FRAME_MODE_RGB);
  gtk_box_pack_start (GTK_BOX (hbox), picker_tool->color_frame2,
                      FALSE, FALSE, 0);
  gtk_widget_show (picker_tool->color_frame2);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 0.0);
  picker_tool->color_area =
    picman_color_area_new (&color,
                         picman_drawable_has_alpha (tool->drawable) ?
                         PICMAN_COLOR_AREA_LARGE_CHECKS :
                         PICMAN_COLOR_AREA_FLAT,
                         GDK_BUTTON1_MASK | GDK_BUTTON2_MASK);
  gtk_widget_set_size_request (picker_tool->color_area, 48, -1);
  gtk_drag_dest_unset (picker_tool->color_area);
  gtk_container_add (GTK_CONTAINER (frame), picker_tool->color_area);
  gtk_widget_show (picker_tool->color_area);
}

static void
picman_color_picker_tool_info_response (GtkWidget           *widget,
                                      gint                 response_id,
                                      PicmanColorPickerTool *picker_tool)
{
  PicmanTool *tool = PICMAN_TOOL (picker_tool);

  picman_tool_control (tool, PICMAN_TOOL_ACTION_HALT, tool->display);
}

static void
picman_color_picker_tool_info_update (PicmanColorPickerTool *picker_tool,
                                    const Babl          *sample_format,
                                    const PicmanRGB       *color,
                                    gint                 color_index)
{
  PicmanTool *tool = PICMAN_TOOL (picker_tool);

  picman_tool_dialog_set_shell (PICMAN_TOOL_DIALOG (picker_tool->dialog),
                              picman_display_get_shell (tool->display));
  picman_viewable_dialog_set_viewable (PICMAN_VIEWABLE_DIALOG (picker_tool->dialog),
                                     PICMAN_VIEWABLE (tool->drawable),
                                     PICMAN_CONTEXT (picman_tool_get_options (tool)));

  picman_color_area_set_color (PICMAN_COLOR_AREA (picker_tool->color_area),
                             color);

  picman_color_frame_set_color (PICMAN_COLOR_FRAME (picker_tool->color_frame1),
                              sample_format, color, color_index);
  picman_color_frame_set_color (PICMAN_COLOR_FRAME (picker_tool->color_frame2),
                              sample_format, color, color_index);

  /*  don't use gtk_window_present() because it would focus the dialog  */
  if (gtk_widget_get_visible (picker_tool->dialog))
    gdk_window_show (gtk_widget_get_window (picker_tool->dialog));
  else
    gtk_widget_show (picker_tool->dialog);
}
