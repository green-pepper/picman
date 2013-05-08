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

#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmandata.h"
#include "core/picmanimage.h"
#include "core/picmanimage-pick-color.h"
#include "core/picmanimage-sample-points.h"
#include "core/picmanitem.h"
#include "core/picmanmarshal.h"
#include "core/picmansamplepoint.h"

#include "widgets/picmancolormapeditor.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandockable.h"
#include "widgets/picmanpaletteeditor.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanwindowstrategy.h"

#include "display/picmancanvasitem.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-appearance.h"
#include "display/picmandisplayshell-selection.h"
#include "display/picmandisplayshell-transform.h"

#include "picmancoloroptions.h"
#include "picmancolortool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


#define SAMPLE_POINT_POSITION_INVALID G_MININT


enum
{
  PICKED,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void   picman_color_tool_finalize       (GObject               *object);

static void   picman_color_tool_button_press   (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanButtonPressType    press_type,
                                              PicmanDisplay           *display);
static void   picman_color_tool_button_release (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanButtonReleaseType  release_type,
                                              PicmanDisplay           *display);
static void   picman_color_tool_motion         (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanDisplay           *display);
static void   picman_color_tool_oper_update    (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              GdkModifierType        state,
                                              gboolean               proximity,
                                              PicmanDisplay           *display);
static void   picman_color_tool_cursor_update  (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              GdkModifierType        state,
                                              PicmanDisplay           *display);

static void   picman_color_tool_draw           (PicmanDrawTool          *draw_tool);

static gboolean   picman_color_tool_real_pick  (PicmanColorTool         *color_tool,
                                              gint                   x,
                                              gint                   y,
                                              const Babl           **sample_format,
                                              PicmanRGB               *color,
                                              gint                  *color_index);
static void   picman_color_tool_pick           (PicmanColorTool         *tool,
                                              PicmanColorPickState     pick_state,
                                              gint                   x,
                                              gint                   y);
static void   picman_color_tool_real_picked    (PicmanColorTool         *color_tool,
                                              PicmanColorPickState     pick_state,
                                              const Babl            *sample_format,
                                              const PicmanRGB         *color,
                                              gint                   color_index);


G_DEFINE_TYPE (PicmanColorTool, picman_color_tool, PICMAN_TYPE_DRAW_TOOL);

#define parent_class picman_color_tool_parent_class

static guint picman_color_tool_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_tool_class_init (PicmanColorToolClass *klass)
{
  GObjectClass      *object_class = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class   = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_class   = PICMAN_DRAW_TOOL_CLASS (klass);

  picman_color_tool_signals[PICKED] =
    g_signal_new ("picked",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorToolClass, picked),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM_POINTER_BOXED_INT,
                  G_TYPE_NONE, 4,
                  PICMAN_TYPE_COLOR_PICK_STATE,
                  G_TYPE_POINTER,
                  PICMAN_TYPE_RGB | G_SIGNAL_TYPE_STATIC_SCOPE,
                  G_TYPE_INT);

  object_class->finalize     = picman_color_tool_finalize;

  tool_class->button_press   = picman_color_tool_button_press;
  tool_class->button_release = picman_color_tool_button_release;
  tool_class->motion         = picman_color_tool_motion;
  tool_class->oper_update    = picman_color_tool_oper_update;
  tool_class->cursor_update  = picman_color_tool_cursor_update;

  draw_class->draw           = picman_color_tool_draw;

  klass->pick                = picman_color_tool_real_pick;
  klass->picked              = picman_color_tool_real_picked;
}

static void
picman_color_tool_init (PicmanColorTool *color_tool)
{
  PicmanTool *tool = PICMAN_TOOL (color_tool);

  picman_tool_control_set_action_value_2 (tool->control,
                                        "tools/tools-color-average-radius-set");

  color_tool->enabled             = FALSE;
  color_tool->center_x            = 0;
  color_tool->center_y            = 0;
  color_tool->pick_mode           = PICMAN_COLOR_PICK_MODE_NONE;

  color_tool->options             = NULL;

  color_tool->sample_point        = NULL;
  color_tool->moving_sample_point = FALSE;
  color_tool->sample_point_x      = SAMPLE_POINT_POSITION_INVALID;
  color_tool->sample_point_y      = SAMPLE_POINT_POSITION_INVALID;
}

static void
picman_color_tool_finalize (GObject *object)
{
  PicmanColorTool *color_tool = PICMAN_COLOR_TOOL (object);

  if (color_tool->options)
    {
      g_object_unref (color_tool->options);
      color_tool->options = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_tool_button_press (PicmanTool            *tool,
                              const PicmanCoords    *coords,
                              guint32              time,
                              GdkModifierType      state,
                              PicmanButtonPressType  press_type,
                              PicmanDisplay         *display)
{
  PicmanColorTool    *color_tool = PICMAN_COLOR_TOOL (tool);
  PicmanDisplayShell *shell      = picman_display_get_shell (display);

  /*  Chain up to activate the tool  */
  PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                press_type, display);

  if (! color_tool->enabled)
    return;

  if (color_tool->sample_point)
    {
      color_tool->moving_sample_point = TRUE;
      color_tool->sample_point_x      = color_tool->sample_point->x;
      color_tool->sample_point_y      = color_tool->sample_point->y;

      picman_tool_control_set_scroll_lock (tool->control, TRUE);

      picman_display_shell_selection_pause (shell);

      if (! picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tool)))
        picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

      picman_tool_push_status_coords (tool, display,
                                    picman_tool_control_get_precision (tool->control),
                                    _("Move Sample Point: "),
                                    color_tool->sample_point_x,
                                    ", ",
                                    color_tool->sample_point_y,
                                    NULL);
    }
  else
    {
      color_tool->center_x = coords->x;
      color_tool->center_y = coords->y;

      picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);

      picman_color_tool_pick (color_tool, PICMAN_COLOR_PICK_STATE_NEW,
                            coords->x, coords->y);
    }
}

static void
picman_color_tool_button_release (PicmanTool              *tool,
                                const PicmanCoords      *coords,
                                guint32                time,
                                GdkModifierType        state,
                                PicmanButtonReleaseType  release_type,
                                PicmanDisplay           *display)
{
  PicmanColorTool    *color_tool = PICMAN_COLOR_TOOL (tool);
  PicmanDisplayShell *shell      = picman_display_get_shell (display);

  /*  Chain up to halt the tool  */
  PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state,
                                                  release_type, display);

  if (! color_tool->enabled)
    return;

  if (color_tool->moving_sample_point)
    {
      PicmanImage *image  = picman_display_get_image (display);
      gint       width  = picman_image_get_width  (image);
      gint       height = picman_image_get_height (image);

      picman_tool_pop_status (tool, display);

      picman_tool_control_set_scroll_lock (tool->control, FALSE);
      picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

      if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
        {
          color_tool->moving_sample_point = FALSE;
          color_tool->sample_point_x      = SAMPLE_POINT_POSITION_INVALID;
          color_tool->sample_point_y      = SAMPLE_POINT_POSITION_INVALID;

          picman_display_shell_selection_resume (shell);
          return;
        }

      if (color_tool->sample_point_x == SAMPLE_POINT_POSITION_INVALID ||
          color_tool->sample_point_x <  0                             ||
          color_tool->sample_point_x >= width                         ||
          color_tool->sample_point_y == SAMPLE_POINT_POSITION_INVALID ||
          color_tool->sample_point_y <  0                             ||
          color_tool->sample_point_y >= height)
        {
          if (color_tool->sample_point)
            {
              picman_image_remove_sample_point (image,
                                              color_tool->sample_point, TRUE);
              color_tool->sample_point = NULL;
            }
        }
      else
        {
          if (color_tool->sample_point)
            {
              picman_image_move_sample_point (image,
                                            color_tool->sample_point,
                                            color_tool->sample_point_x,
                                            color_tool->sample_point_y,
                                            TRUE);
            }
          else
            {
              color_tool->sample_point =
                picman_image_add_sample_point_at_pos (image,
                                                    color_tool->sample_point_x,
                                                    color_tool->sample_point_y,
                                                    TRUE);
            }
        }

      picman_display_shell_selection_resume (shell);
      picman_image_flush (image);

      color_tool->moving_sample_point = FALSE;
      color_tool->sample_point_x      = SAMPLE_POINT_POSITION_INVALID;
      color_tool->sample_point_y      = SAMPLE_POINT_POSITION_INVALID;

      if (color_tool->sample_point)
        picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
    }
  else
    {
      picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));
    }
}

static void
picman_color_tool_motion (PicmanTool         *tool,
                        const PicmanCoords *coords,
                        guint32           time,
                        GdkModifierType   state,
                        PicmanDisplay      *display)
{
  PicmanColorTool    *color_tool = PICMAN_COLOR_TOOL (tool);
  PicmanDisplayShell *shell      = picman_display_get_shell (display);

  if (! color_tool->enabled)
    return;

  if (color_tool->moving_sample_point)
    {
      gint      tx, ty;
      gboolean  delete_point = FALSE;

      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      picman_display_shell_transform_xy (shell,
                                       coords->x, coords->y,
                                       &tx, &ty);

      if (tx < 0 || tx > shell->disp_width ||
          ty < 0 || ty > shell->disp_height)
        {
          color_tool->sample_point_x = SAMPLE_POINT_POSITION_INVALID;
          color_tool->sample_point_y = SAMPLE_POINT_POSITION_INVALID;

          delete_point = TRUE;
        }
      else
        {
          PicmanImage *image  = picman_display_get_image (display);
          gint       width  = picman_image_get_width  (image);
          gint       height = picman_image_get_height (image);

          color_tool->sample_point_x = floor (coords->x);
          color_tool->sample_point_y = floor (coords->y);

          if (color_tool->sample_point_x <  0     ||
              color_tool->sample_point_x >= width ||
              color_tool->sample_point_y <  0     ||
              color_tool->sample_point_y >= height)
            {
              delete_point = TRUE;
            }
        }

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

      picman_tool_pop_status (tool, display);

      if (delete_point)
        {
          picman_tool_push_status (tool, display,
                                 color_tool->sample_point ?
                                 _("Remove Sample Point") :
                                 _("Cancel Sample Point"));
        }
      else
        {
          picman_tool_push_status_coords (tool, display,
                                        picman_tool_control_get_precision (tool->control),
                                        color_tool->sample_point ?
                                        _("Move Sample Point: ") :
                                        _("Add Sample Point: "),
                                        color_tool->sample_point_x,
                                        ", ",
                                        color_tool->sample_point_y,
                                        NULL);
        }
    }
  else
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      color_tool->center_x = coords->x;
      color_tool->center_y = coords->y;

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));

      picman_color_tool_pick (color_tool, PICMAN_COLOR_PICK_STATE_UPDATE,
                            coords->x, coords->y);
    }
}

static void
picman_color_tool_oper_update (PicmanTool         *tool,
                             const PicmanCoords *coords,
                             GdkModifierType   state,
                             gboolean          proximity,
                             PicmanDisplay      *display)
{
  PicmanColorTool    *color_tool   = PICMAN_COLOR_TOOL (tool);
  PicmanDisplayShell *shell        = picman_display_get_shell (display);
  PicmanImage        *image        = picman_display_get_image (display);
  PicmanSamplePoint  *sample_point = NULL;

  if (color_tool->enabled                               &&
      picman_display_shell_get_show_sample_points (shell) &&
      proximity)
    {
      gint snap_distance = display->config->snap_distance;

      sample_point =
        picman_image_find_sample_point (image,
                                      coords->x, coords->y,
                                      FUNSCALEX (shell, snap_distance),
                                      FUNSCALEY (shell, snap_distance));
    }

  if (color_tool->sample_point != sample_point)
    {
      PicmanDrawTool *draw_tool = PICMAN_DRAW_TOOL (tool);

      picman_draw_tool_pause (draw_tool);

      if (picman_draw_tool_is_active (draw_tool) &&
          draw_tool->display != display)
        picman_draw_tool_stop (draw_tool);

      color_tool->sample_point = sample_point;

      if (! picman_draw_tool_is_active (draw_tool))
        picman_draw_tool_start (draw_tool, display);

      picman_draw_tool_resume (draw_tool);
    }
}

static void
picman_color_tool_cursor_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display)
{
  PicmanColorTool *color_tool = PICMAN_COLOR_TOOL (tool);
  PicmanImage     *image      = picman_display_get_image (display);

  if (color_tool->enabled)
    {
      if (color_tool->sample_point)
        {
          picman_tool_set_cursor (tool, display,
                                PICMAN_CURSOR_MOUSE,
                                PICMAN_TOOL_CURSOR_COLOR_PICKER,
                                PICMAN_CURSOR_MODIFIER_MOVE);
        }
      else
        {
          PicmanCursorModifier modifier = PICMAN_CURSOR_MODIFIER_BAD;

          if (picman_image_coords_in_active_pickable (image, coords,
                                                    color_tool->options->sample_merged,
                                                    FALSE))
            {
              switch (color_tool->pick_mode)
                {
                case PICMAN_COLOR_PICK_MODE_NONE:
                  modifier = PICMAN_CURSOR_MODIFIER_NONE;
                  break;
                case PICMAN_COLOR_PICK_MODE_FOREGROUND:
                  modifier = PICMAN_CURSOR_MODIFIER_FOREGROUND;
                  break;
                case PICMAN_COLOR_PICK_MODE_BACKGROUND:
                  modifier = PICMAN_CURSOR_MODIFIER_BACKGROUND;
                  break;
                case PICMAN_COLOR_PICK_MODE_PALETTE:
                  modifier = PICMAN_CURSOR_MODIFIER_PLUS;
                  break;
                }
            }

          picman_tool_set_cursor (tool, display,
                                PICMAN_CURSOR_COLOR_PICKER,
                                PICMAN_TOOL_CURSOR_COLOR_PICKER,
                                modifier);
        }

      return;  /*  don't chain up  */
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_color_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanColorTool *color_tool = PICMAN_COLOR_TOOL (draw_tool);

  if (color_tool->enabled)
    {
      if (color_tool->sample_point)
        {
          PicmanImage      *image = picman_display_get_image (draw_tool->display);
          PicmanCanvasItem *item;
          gint            index;

          index = g_list_index (picman_image_get_sample_points (image),
                                color_tool->sample_point) + 1;

          item = picman_draw_tool_add_sample_point (draw_tool,
                                                  color_tool->sample_point->x,
                                                  color_tool->sample_point->y,
                                                  index);
          picman_canvas_item_set_highlight (item, TRUE);
        }

      if (color_tool->moving_sample_point)
        {
          if (color_tool->sample_point_x != SAMPLE_POINT_POSITION_INVALID &&
              color_tool->sample_point_y != SAMPLE_POINT_POSITION_INVALID)
            {
              picman_draw_tool_add_crosshair (draw_tool,
                                            color_tool->sample_point_x,
                                            color_tool->sample_point_y);
            }
        }
      else if (color_tool->options->sample_average &&
               picman_tool_control_is_active (PICMAN_TOOL (draw_tool)->control))
        {
          gdouble radius = color_tool->options->average_radius;

          picman_draw_tool_add_rectangle (draw_tool,
                                        FALSE,
                                        color_tool->center_x - radius,
                                        color_tool->center_y - radius,
                                        2 * radius + 1,
                                        2 * radius + 1);
        }
    }
}

static gboolean
picman_color_tool_real_pick (PicmanColorTool  *color_tool,
                           gint            x,
                           gint            y,
                           const Babl    **sample_format,
                           PicmanRGB        *color,
                           gint           *color_index)
{
  PicmanTool  *tool  = PICMAN_TOOL (color_tool);
  PicmanImage *image = picman_display_get_image (tool->display);

  g_return_val_if_fail (tool->display != NULL, FALSE);
  g_return_val_if_fail (tool->drawable != NULL, FALSE);

  return picman_image_pick_color (image, tool->drawable, x, y,
                                color_tool->options->sample_merged,
                                color_tool->options->sample_average,
                                color_tool->options->average_radius,
                                sample_format,
                                color,
                                color_index);
}

static void
picman_color_tool_real_picked (PicmanColorTool      *color_tool,
                             PicmanColorPickState  pick_state,
                             const Babl         *sample_format,
                             const PicmanRGB      *color,
                             gint                color_index)
{
  PicmanTool          *tool = PICMAN_TOOL (color_tool);
  PicmanContext       *context;

  /*  use this tool's own options here (NOT color_tool->options)  */
  context = PICMAN_CONTEXT (picman_tool_get_options (tool));

  if (color_tool->pick_mode == PICMAN_COLOR_PICK_MODE_FOREGROUND ||
      color_tool->pick_mode == PICMAN_COLOR_PICK_MODE_BACKGROUND)
    {
      GtkWidget *widget;

      if (babl_format_is_palette (sample_format))
        {
          widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                                    "picman-indexed-palette");
          if (widget)
            {
              PicmanColormapEditor *editor;

              editor = PICMAN_COLORMAP_EDITOR (gtk_bin_get_child (GTK_BIN (widget)));

              picman_colormap_editor_set_index (editor, color_index, NULL);
            }
        }

      if (TRUE)
        {
          widget = picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                                    "picman-palette-editor");
          if (widget)
            {
              PicmanPaletteEditor *editor;
              gint               index;

              editor = PICMAN_PALETTE_EDITOR (gtk_bin_get_child (GTK_BIN (widget)));

              index = picman_palette_editor_get_index (editor, color);
              if (index != -1)
                picman_palette_editor_set_index (editor, index, NULL);
            }
        }
    }

  switch (color_tool->pick_mode)
    {
    case PICMAN_COLOR_PICK_MODE_NONE:
      break;

    case PICMAN_COLOR_PICK_MODE_FOREGROUND:
      picman_context_set_foreground (context, color);
      break;

    case PICMAN_COLOR_PICK_MODE_BACKGROUND:
      picman_context_set_background (context, color);
      break;

    case PICMAN_COLOR_PICK_MODE_PALETTE:
      {
        PicmanDisplayShell *shell  = picman_display_get_shell (tool->display);
        GdkScreen        *screen = gtk_widget_get_screen (GTK_WIDGET (shell));
        GtkWidget        *dockable;

        dockable =
          picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (tool->display->picman)),
                                                     tool->display->picman,
                                                     picman_dialog_factory_get_singleton (),
                                                     screen,
                                                     "picman-palette-editor");

        if (dockable)
          {
            GtkWidget *palette_editor;
            PicmanData  *data;

            /* don't blink like mad when updating */
            if (pick_state == PICMAN_COLOR_PICK_STATE_UPDATE)
              picman_dockable_blink_cancel (PICMAN_DOCKABLE (dockable));

            palette_editor = gtk_bin_get_child (GTK_BIN (dockable));

            data = picman_data_editor_get_data (PICMAN_DATA_EDITOR (palette_editor));

            if (! data)
              {
                data = PICMAN_DATA (picman_context_get_palette (context));

                picman_data_editor_set_data (PICMAN_DATA_EDITOR (palette_editor),
                                           data);
              }

            picman_palette_editor_pick_color (PICMAN_PALETTE_EDITOR (palette_editor),
                                            color, pick_state);
          }
      }
      break;
    }
}

static void
picman_color_tool_pick (PicmanColorTool      *tool,
                      PicmanColorPickState  pick_state,
                      gint                x,
                      gint                y)
{
  PicmanColorToolClass *klass;
  const Babl         *sample_format;
  PicmanRGB             color;
  gint                color_index;

  klass = PICMAN_COLOR_TOOL_GET_CLASS (tool);

  if (klass->pick &&
      klass->pick (tool, x, y, &sample_format, &color, &color_index))
    {
      g_signal_emit (tool, picman_color_tool_signals[PICKED], 0,
                     pick_state, sample_format, &color, color_index);
    }
}


void
picman_color_tool_enable (PicmanColorTool    *color_tool,
                        PicmanColorOptions *options)
{
  PicmanTool *tool;

  g_return_if_fail (PICMAN_IS_COLOR_TOOL (color_tool));
  g_return_if_fail (PICMAN_IS_COLOR_OPTIONS (options));

  tool = PICMAN_TOOL (color_tool);

  if (picman_tool_control_is_active (tool->control))
    {
      g_warning ("Trying to enable PicmanColorTool while it is active.");
      return;
    }

  if (color_tool->options)
    g_object_unref (color_tool->options);

  color_tool->options = g_object_ref (options);

  color_tool->enabled = TRUE;
}

void
picman_color_tool_disable (PicmanColorTool *color_tool)
{
  PicmanTool *tool;

  g_return_if_fail (PICMAN_IS_COLOR_TOOL (color_tool));

  tool = PICMAN_TOOL (color_tool);

  if (picman_tool_control_is_active (tool->control))
    {
      g_warning ("Trying to disable PicmanColorTool while it is active.");
      return;
    }

  if (color_tool->options)
    {
      g_object_unref (color_tool->options);
      color_tool->options = NULL;
    }

  color_tool->enabled = FALSE;
}

gboolean
picman_color_tool_is_enabled (PicmanColorTool *color_tool)
{
  return color_tool->enabled;
}

void
picman_color_tool_start_sample_point (PicmanTool    *tool,
                                    PicmanDisplay *display)
{
  PicmanColorTool *color_tool;

  g_return_if_fail (PICMAN_IS_COLOR_TOOL (tool));
  g_return_if_fail (PICMAN_IS_DISPLAY (display));

  color_tool = PICMAN_COLOR_TOOL (tool);

  picman_display_shell_selection_pause (picman_display_get_shell (display));

  tool->display = display;
  picman_tool_control_activate (tool->control);
  picman_tool_control_set_scroll_lock (tool->control, TRUE);

  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (tool)))
    picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  color_tool->sample_point        = NULL;
  color_tool->moving_sample_point = TRUE;
  color_tool->sample_point_x      = SAMPLE_POINT_POSITION_INVALID;
  color_tool->sample_point_y      = SAMPLE_POINT_POSITION_INVALID;

  picman_tool_set_cursor (tool, display,
                        PICMAN_CURSOR_MOUSE,
                        PICMAN_TOOL_CURSOR_COLOR_PICKER,
                        PICMAN_CURSOR_MODIFIER_MOVE);

  picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
}
