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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "tools-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanpaintinfo.h"
#include "core/picmanprojection.h"
#include "core/picmantoolinfo.h"

#include "paint/picmanpaintcore.h"
#include "paint/picmanpaintoptions.h"

#include "widgets/picmandevices.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-selection.h"

#include "picmancoloroptions.h"
#include "picmanpainttool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_paint_tool_constructed    (GObject               *object);
static void   picman_paint_tool_finalize       (GObject               *object);

static void   picman_paint_tool_control        (PicmanTool              *tool,
                                              PicmanToolAction         action,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_button_press   (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanButtonPressType    press_type,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_button_release (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanButtonReleaseType  release_type,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_motion         (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              guint32                time,
                                              GdkModifierType        state,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_modifier_key   (PicmanTool              *tool,
                                              GdkModifierType        key,
                                              gboolean               press,
                                              GdkModifierType        state,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_cursor_update  (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              GdkModifierType        state,
                                              PicmanDisplay           *display);
static void   picman_paint_tool_oper_update    (PicmanTool              *tool,
                                              const PicmanCoords      *coords,
                                              GdkModifierType        state,
                                              gboolean               proximity,
                                              PicmanDisplay           *display);

static void   picman_paint_tool_draw           (PicmanDrawTool          *draw_tool);

static void   picman_paint_tool_hard_notify    (PicmanPaintOptions      *options,
                                              const GParamSpec      *pspec,
                                              PicmanTool              *tool);


G_DEFINE_TYPE (PicmanPaintTool, picman_paint_tool, PICMAN_TYPE_COLOR_TOOL)

#define parent_class picman_paint_tool_parent_class


static void
picman_paint_tool_class_init (PicmanPaintToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed  = picman_paint_tool_constructed;
  object_class->finalize     = picman_paint_tool_finalize;

  tool_class->control        = picman_paint_tool_control;
  tool_class->button_press   = picman_paint_tool_button_press;
  tool_class->button_release = picman_paint_tool_button_release;
  tool_class->motion         = picman_paint_tool_motion;
  tool_class->modifier_key   = picman_paint_tool_modifier_key;
  tool_class->cursor_update  = picman_paint_tool_cursor_update;
  tool_class->oper_update    = picman_paint_tool_oper_update;

  draw_tool_class->draw      = picman_paint_tool_draw;
}

static void
picman_paint_tool_init (PicmanPaintTool *paint_tool)
{
  PicmanTool *tool = PICMAN_TOOL (paint_tool);

  picman_tool_control_set_motion_mode    (tool->control, PICMAN_MOTION_MODE_EXACT);
  picman_tool_control_set_scroll_lock    (tool->control, TRUE);
  picman_tool_control_set_action_value_1 (tool->control,
                                        "context/context-opacity-set");

  paint_tool->pick_colors = FALSE;
  paint_tool->draw_line   = FALSE;

  paint_tool->status      = _("Click to paint");
  paint_tool->status_line = _("Click to draw the line");
  paint_tool->status_ctrl = _("%s to pick a color");

  paint_tool->core        = NULL;
}

static void
picman_paint_tool_constructed (GObject *object)
{
  PicmanTool         *tool       = PICMAN_TOOL (object);
  PicmanPaintTool    *paint_tool = PICMAN_PAINT_TOOL (object);
  PicmanPaintOptions *options    = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanPaintInfo    *paint_info;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_TOOL_INFO (tool->tool_info));
  g_assert (PICMAN_IS_PAINT_INFO (tool->tool_info->paint_info));

  paint_info = tool->tool_info->paint_info;

  g_assert (g_type_is_a (paint_info->paint_type, PICMAN_TYPE_PAINT_CORE));

  paint_tool->core = g_object_new (paint_info->paint_type,
                                   "undo-desc", paint_info->blurb,
                                   NULL);

  g_signal_connect_object (options, "notify::hard",
                           G_CALLBACK (picman_paint_tool_hard_notify),
                           tool, 0);

  picman_paint_tool_hard_notify (options, NULL, tool);
}

static void
picman_paint_tool_finalize (GObject *object)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (object);

  if (paint_tool->core)
    {
      g_object_unref (paint_tool->core);
      paint_tool->core = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * picman_paint_tool_enable_color_picker:
 * @tool: a #PicmanPaintTool
 * @mode: the #PicmanColorPickMode to set
 *
 * This is a convenience function used from the init method of paint
 * tools that want the color picking functionality. The @mode that is
 * set here is used to decide what cursor modifier to draw and if the
 * picked color goes to the foreground or background color.
 **/
void
picman_paint_tool_enable_color_picker (PicmanPaintTool     *tool,
                                     PicmanColorPickMode  mode)
{
  g_return_if_fail (PICMAN_IS_PAINT_TOOL (tool));

  tool->pick_colors = TRUE;

  PICMAN_COLOR_TOOL (tool)->pick_mode = mode;
}

static void
picman_paint_tool_control (PicmanTool       *tool,
                         PicmanToolAction  action,
                         PicmanDisplay    *display)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_paint_core_cleanup (paint_tool->core);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_paint_tool_button_press (PicmanTool            *tool,
                              const PicmanCoords    *coords,
                              guint32              time,
                              GdkModifierType      state,
                              PicmanButtonPressType  press_type,
                              PicmanDisplay         *display)
{
  PicmanDrawTool     *draw_tool     = PICMAN_DRAW_TOOL (tool);
  PicmanPaintTool    *paint_tool    = PICMAN_PAINT_TOOL (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanPaintCore    *core          = paint_tool->core;
  PicmanDisplayShell *shell         = picman_display_get_shell (display);
  PicmanImage        *image         = picman_display_get_image (display);
  PicmanDrawable     *drawable      = picman_image_get_active_drawable (image);
  PicmanCoords        curr_coords;
  gint              off_x, off_y;
  GError           *error = NULL;

  if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                    press_type, display);
      return;
    }

  if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)))
    {
      picman_tool_message_literal (tool, display,
                                 _("Cannot paint on layer groups."));
      return;
    }

  if (picman_item_is_content_locked (PICMAN_ITEM (drawable)))
    {
      picman_tool_message_literal (tool, display,
                                 _("The active layer's pixels are locked."));
      return;
    }

  curr_coords = *coords;

  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  curr_coords.x -= off_x;
  curr_coords.y -= off_y;

  if (picman_draw_tool_is_active (draw_tool))
    picman_draw_tool_stop (draw_tool);

  if (tool->display            &&
      tool->display != display &&
      picman_display_get_image (tool->display) == image)
    {
      /*  if this is a different display, but the same image, HACK around
       *  in tool internals AFTER stopping the current draw_tool, so
       *  straight line drawing works across different views of the
       *  same image.
       */

      tool->display = display;
    }

  if (! picman_paint_core_start (core, drawable, paint_options, &curr_coords,
                               &error))
    {
      picman_tool_message_literal (tool, display, error->message);
      g_clear_error (&error);
      return;
    }

  if ((display != tool->display) || ! paint_tool->draw_line)
    {
      /*  if this is a new image, reinit the core vals  */

      core->start_coords = core->cur_coords;
      core->last_coords  = core->cur_coords;

      core->distance     = 0.0;
      core->pixel_dist   = 0.0;
    }
  else if (paint_tool->draw_line)
    {
      gboolean constrain = (state & picman_get_constrain_behavior_mask ()) != 0;

      /*  If shift is down and this is not the first paint
       *  stroke, then draw a line from the last coords to the pointer
       */
      core->start_coords = core->last_coords;

      picman_paint_core_round_line (core, paint_options, constrain);
    }

  /*  chain up to activate the tool  */
  PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                press_type, display);

  /*  pause the current selection  */
  picman_display_shell_selection_pause (shell);

  /*  Let the specific painting function initialize itself  */
  picman_paint_core_paint (core, drawable, paint_options,
                         PICMAN_PAINT_STATE_INIT, time);

  /*  Paint to the image  */
  if (paint_tool->draw_line)
    {
      picman_paint_core_interpolate (core, drawable, paint_options,
                                   &core->cur_coords, time);
    }
  else
    {
      picman_paint_core_paint (core, drawable, paint_options,
                             PICMAN_PAINT_STATE_MOTION, time);
    }

  picman_projection_flush_now (picman_image_get_projection (image));
  picman_display_flush_now (display);

  picman_draw_tool_start (draw_tool, display);
}

static void
picman_paint_tool_button_release (PicmanTool              *tool,
                                const PicmanCoords      *coords,
                                guint32                time,
                                GdkModifierType        state,
                                PicmanButtonReleaseType  release_type,
                                PicmanDisplay           *display)
{
  PicmanPaintTool    *paint_tool    = PICMAN_PAINT_TOOL (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanPaintCore    *core          = paint_tool->core;
  PicmanDisplayShell *shell         = picman_display_get_shell (display);
  PicmanImage        *image         = picman_display_get_image (display);
  PicmanDrawable     *drawable      = picman_image_get_active_drawable (image);

  if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time,
                                                      state, release_type,
                                                      display);
      return;
    }

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  /*  Let the specific painting function finish up  */
  picman_paint_core_paint (core, drawable, paint_options,
                         PICMAN_PAINT_STATE_FINISH, time);

  /*  resume the current selection  */
  picman_display_shell_selection_resume (shell);

  /*  chain up to halt the tool */
  PICMAN_TOOL_CLASS (parent_class)->button_release (tool, coords, time, state,
                                                  release_type, display);

  if (release_type == PICMAN_BUTTON_RELEASE_CANCEL)
    picman_paint_core_cancel (core, drawable);
  else
    picman_paint_core_finish (core, drawable, TRUE);

  picman_image_flush (image);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_paint_tool_motion (PicmanTool         *tool,
                        const PicmanCoords *coords,
                        guint32           time,
                        GdkModifierType   state,
                        PicmanDisplay      *display)
{
  PicmanPaintTool    *paint_tool    = PICMAN_PAINT_TOOL (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanPaintCore    *core          = paint_tool->core;
  PicmanImage        *image         = picman_display_get_image (display);
  PicmanDrawable     *drawable      = picman_image_get_active_drawable (image);
  PicmanCoords        curr_coords;
  gint              off_x, off_y;

  PICMAN_TOOL_CLASS (parent_class)->motion (tool, coords, time, state, display);

  if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    return;

  curr_coords = *coords;

  picman_paint_core_smooth_coords (core, paint_options, &curr_coords);

  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  curr_coords.x -= off_x;
  curr_coords.y -= off_y;

  /*  don't paint while the Shift key is pressed for line drawing  */
  if (paint_tool->draw_line)
    {
      picman_paint_core_set_current_coords (core, &curr_coords);
      return;
    }

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  picman_paint_core_interpolate (core, drawable, paint_options,
                               &curr_coords, time);

  picman_projection_flush_now (picman_image_get_projection (image));
  picman_display_flush_now (display);

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_paint_tool_modifier_key (PicmanTool        *tool,
                              GdkModifierType  key,
                              gboolean         press,
                              GdkModifierType  state,
                              PicmanDisplay     *display)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);
  PicmanDrawTool  *draw_tool  = PICMAN_DRAW_TOOL (tool);

  if (key != picman_get_constrain_behavior_mask ())
    return;

  if (paint_tool->pick_colors && ! paint_tool->draw_line)
    {
      if (press)
        {
          PicmanToolInfo *info = picman_get_tool_info (display->picman,
                                                   "picman-color-picker-tool");

          if (PICMAN_IS_TOOL_INFO (info))
            {
              if (picman_draw_tool_is_active (draw_tool))
                picman_draw_tool_stop (draw_tool);

              picman_color_tool_enable (PICMAN_COLOR_TOOL (tool),
                                      PICMAN_COLOR_OPTIONS (info->tool_options));

              switch (PICMAN_COLOR_TOOL (tool)->pick_mode)
                {
                case PICMAN_COLOR_PICK_MODE_FOREGROUND:
                  picman_tool_push_status (tool, display,
                                         _("Click in any image to pick the "
                                           "foreground color"));
                  break;

                case PICMAN_COLOR_PICK_MODE_BACKGROUND:
                  picman_tool_push_status (tool, display,
                                         _("Click in any image to pick the "
                                           "background color"));
                  break;

                default:
                  break;
                }
            }
        }
      else
        {
          if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
            {
              picman_tool_pop_status (tool, display);
              picman_color_tool_disable (PICMAN_COLOR_TOOL (tool));
            }
        }
    }
}

static void
picman_paint_tool_cursor_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display)
{
  PicmanCursorModifier  modifier;
  PicmanCursorModifier  toggle_modifier;
  PicmanCursorModifier  old_modifier;
  PicmanCursorModifier  old_toggle_modifier;

  modifier        = tool->control->cursor_modifier;
  toggle_modifier = tool->control->toggle_cursor_modifier;

  old_modifier        = modifier;
  old_toggle_modifier = toggle_modifier;

  if (! picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      PicmanImage    *image    = picman_display_get_image (display);
      PicmanDrawable *drawable = picman_image_get_active_drawable (image);

      if (picman_viewable_get_children (PICMAN_VIEWABLE (drawable)) ||
          picman_item_is_content_locked (PICMAN_ITEM (drawable)))
        {
          modifier        = PICMAN_CURSOR_MODIFIER_BAD;
          toggle_modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }

      picman_tool_control_set_cursor_modifier        (tool->control,
                                                    modifier);
      picman_tool_control_set_toggle_cursor_modifier (tool->control,
                                                    toggle_modifier);
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state,
                                                 display);

  /*  reset old stuff here so we are not interferring with the modifiers
   *  set by our subclasses
   */
  picman_tool_control_set_cursor_modifier        (tool->control,
                                                old_modifier);
  picman_tool_control_set_toggle_cursor_modifier (tool->control,
                                                old_toggle_modifier);
}

static void
picman_paint_tool_oper_update (PicmanTool         *tool,
                             const PicmanCoords *coords,
                             GdkModifierType   state,
                             gboolean          proximity,
                             PicmanDisplay      *display)
{
  PicmanPaintTool    *paint_tool    = PICMAN_PAINT_TOOL (tool);
  PicmanDrawTool     *draw_tool     = PICMAN_DRAW_TOOL (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanPaintCore    *core          = paint_tool->core;
  PicmanDisplayShell *shell         = picman_display_get_shell (display);
  PicmanImage        *image         = picman_display_get_image (display);
  PicmanDrawable     *drawable      = picman_image_get_active_drawable (image);

  if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state,
                                                   proximity, display);
      return;
    }

  picman_draw_tool_pause (draw_tool);

  if (picman_draw_tool_is_active (draw_tool) &&
      draw_tool->display != display)
    picman_draw_tool_stop (draw_tool);

  picman_tool_pop_status (tool, display);

  if (tool->display            &&
      tool->display != display &&
      picman_display_get_image (tool->display) == image)
    {
      /*  if this is a different display, but the same image, HACK around
       *  in tool internals AFTER stopping the current draw_tool, so
       *  straight line drawing works across different views of the
       *  same image.
       */

      tool->display = display;
    }

  if (drawable && proximity)
    {
      gboolean constrain_mask = picman_get_constrain_behavior_mask ();

      if (display == tool->display && (state & GDK_SHIFT_MASK))
        {
          /*  If shift is down and this is not the first paint stroke,
           *  draw a line.
           */

          gchar   *status_help;
          gdouble  dx, dy, dist;
          gint     off_x, off_y;

          core->cur_coords = *coords;

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

          core->cur_coords.x -= off_x;
          core->cur_coords.y -= off_y;

          picman_paint_core_round_line (core, paint_options,
                                      (state & constrain_mask) != 0);

          dx = core->cur_coords.x - core->last_coords.x;
          dy = core->cur_coords.y - core->last_coords.y;

          status_help = picman_suggest_modifiers (paint_tool->status_line,
                                                constrain_mask & ~state,
                                                NULL,
                                                _("%s for constrained angles"),
                                                NULL);

          /*  show distance in statusbar  */
          if (shell->unit == PICMAN_UNIT_PIXEL)
            {
              dist = sqrt (SQR (dx) + SQR (dy));

              picman_tool_push_status (tool, display, "%.1f %s.  %s",
                                     dist, _("pixels"), status_help);
            }
          else
            {
              gdouble xres;
              gdouble yres;
              gchar   format_str[64];

              picman_image_get_resolution (image, &xres, &yres);

              g_snprintf (format_str, sizeof (format_str), "%%.%df %s.  %%s",
                          picman_unit_get_digits (shell->unit),
                          picman_unit_get_symbol (shell->unit));

              dist = (picman_unit_get_factor (shell->unit) *
                      sqrt (SQR (dx / xres) +
                            SQR (dy / yres)));

              picman_tool_push_status (tool, display, format_str,
                                     dist, status_help);
            }

          g_free (status_help);

          paint_tool->draw_line = TRUE;
        }
      else
        {
          gchar           *status;
          GdkModifierType  modifiers = 0;

          /* HACK: A paint tool may set status_ctrl to NULL to indicate that
           * it ignores the Ctrl modifier (temporarily or permanently), so
           * it should not be suggested.  This is different from how
           * picman_suggest_modifiers() would interpret this parameter.
           */
          if (paint_tool->status_ctrl != NULL)
            modifiers |= constrain_mask;

          /* suggest drawing lines only after the first point is set
           */
          if (display == tool->display)
            modifiers |= GDK_SHIFT_MASK;

          status = picman_suggest_modifiers (paint_tool->status,
                                           modifiers & ~state,
                                           _("%s for a straight line"),
                                           paint_tool->status_ctrl,
                                           NULL);
          picman_tool_push_status (tool, display, "%s", status);
          g_free (status);

          paint_tool->draw_line = FALSE;
        }

      if (! picman_draw_tool_is_active (draw_tool))
        picman_draw_tool_start (draw_tool, display);
    }
  else if (picman_draw_tool_is_active (draw_tool))
    {
      picman_draw_tool_stop (draw_tool);
    }

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);

  picman_draw_tool_resume (draw_tool);
}

static void
picman_paint_tool_draw (PicmanDrawTool *draw_tool)
{
  if (! picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (draw_tool)))
    {
      PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (draw_tool);

      if (paint_tool->draw_line &&
          ! picman_tool_control_is_active (PICMAN_TOOL (draw_tool)->control))
        {
          PicmanPaintCore *core       = paint_tool->core;
          PicmanImage     *image      = picman_display_get_image (draw_tool->display);
          PicmanDrawable  *drawable   = picman_image_get_active_drawable (image);
          gint           off_x, off_y;

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

          /*  Draw the line between the start and end coords  */
          picman_draw_tool_add_line (draw_tool,
                                   core->last_coords.x + off_x,
                                   core->last_coords.y + off_y,
                                   core->cur_coords.x + off_x,
                                   core->cur_coords.y + off_y);

          /*  Draw start target  */
          picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_CROSS,
                                     core->last_coords.x + off_x,
                                     core->last_coords.y + off_y,
                                     PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                     PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                     PICMAN_HANDLE_ANCHOR_CENTER);

          /*  Draw end target  */
          picman_draw_tool_add_handle (draw_tool,
                                     PICMAN_HANDLE_CROSS,
                                     core->cur_coords.x + off_x,
                                     core->cur_coords.y + off_y,
                                     PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                     PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                     PICMAN_HANDLE_ANCHOR_CENTER);
        }
    }

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);
}

static void
picman_paint_tool_hard_notify (PicmanPaintOptions *options,
                             const GParamSpec *pspec,
                             PicmanTool         *tool)
{
  picman_tool_control_set_precision (tool->control,
                                   options->hard ?
                                   PICMAN_CURSOR_PRECISION_PIXEL_CENTER :
                                   PICMAN_CURSOR_PRECISION_SUBPIXEL);
}
