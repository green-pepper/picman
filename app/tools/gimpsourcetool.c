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

#include "tools-types.h"

#include "core/picmanchannel.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"

#include "paint/picmansourcecore.h"
#include "paint/picmansourceoptions.h"

#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvashandle.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell-items.h"

#include "picmansourcetool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static gboolean      picman_source_tool_has_display   (PicmanTool            *tool,
                                                     PicmanDisplay         *display);
static PicmanDisplay * picman_source_tool_has_image     (PicmanTool            *tool,
                                                     PicmanImage           *image);
static void          picman_source_tool_control       (PicmanTool            *tool,
                                                     PicmanToolAction       action,
                                                     PicmanDisplay         *display);
static void          picman_source_tool_button_press  (PicmanTool            *tool,
                                                     const PicmanCoords    *coords,
                                                     guint32              time,
                                                     GdkModifierType      state,
                                                     PicmanButtonPressType  press_type,
                                                     PicmanDisplay         *display);
static void          picman_source_tool_motion        (PicmanTool            *tool,
                                                     const PicmanCoords    *coords,
                                                     guint32              time,
                                                     GdkModifierType      state,
                                                     PicmanDisplay         *display);
static void          picman_source_tool_cursor_update (PicmanTool            *tool,
                                                     const PicmanCoords    *coords,
                                                     GdkModifierType      state,
                                                     PicmanDisplay         *display);
static void          picman_source_tool_modifier_key  (PicmanTool            *tool,
                                                     GdkModifierType      key,
                                                     gboolean             press,
                                                     GdkModifierType      state,
                                                     PicmanDisplay         *display);
static void          picman_source_tool_oper_update   (PicmanTool            *tool,
                                                     const PicmanCoords    *coords,
                                                     GdkModifierType      state,
                                                     gboolean             proximity,
                                                     PicmanDisplay         *display);

static void          picman_source_tool_draw          (PicmanDrawTool        *draw_tool);

static void          picman_source_tool_set_src_display (PicmanSourceTool      *source_tool,
                                                       PicmanDisplay         *display);


G_DEFINE_TYPE (PicmanSourceTool, picman_source_tool, PICMAN_TYPE_BRUSH_TOOL)

#define parent_class picman_source_tool_parent_class


static void
picman_source_tool_class_init (PicmanSourceToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->has_display   = picman_source_tool_has_display;
  tool_class->has_image     = picman_source_tool_has_image;
  tool_class->control       = picman_source_tool_control;
  tool_class->button_press  = picman_source_tool_button_press;
  tool_class->motion        = picman_source_tool_motion;
  tool_class->modifier_key  = picman_source_tool_modifier_key;
  tool_class->oper_update   = picman_source_tool_oper_update;
  tool_class->cursor_update = picman_source_tool_cursor_update;

  draw_tool_class->draw     = picman_source_tool_draw;
}

static void
picman_source_tool_init (PicmanSourceTool *source)
{
  source->show_source_outline = TRUE;
}

static gboolean
picman_source_tool_has_display (PicmanTool    *tool,
                              PicmanDisplay *display)
{
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);

  return (display == source_tool->src_display ||
          PICMAN_TOOL_CLASS (parent_class)->has_display (tool, display));
}

static PicmanDisplay *
picman_source_tool_has_image (PicmanTool  *tool,
                            PicmanImage *image)
{
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);
  PicmanDisplay    *display;

  display = PICMAN_TOOL_CLASS (parent_class)->has_image (tool, image);

  if (! display && source_tool->src_display)
    {
      if (image && picman_display_get_image (source_tool->src_display) == image)
        display = source_tool->src_display;

      /*  NULL image means any display  */
      if (! image)
        display = source_tool->src_display;
    }

  return display;
}

static void
picman_source_tool_control (PicmanTool       *tool,
                          PicmanToolAction  action,
                          PicmanDisplay    *display)
{
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);

  switch (action)
    {
    case PICMAN_TOOL_ACTION_PAUSE:
    case PICMAN_TOOL_ACTION_RESUME:
      break;

    case PICMAN_TOOL_ACTION_HALT:
      picman_source_tool_set_src_display (source_tool, NULL);
      g_object_set (PICMAN_PAINT_TOOL (tool)->core,
                    "src-drawable", NULL,
                    NULL);
      break;
    }

  PICMAN_TOOL_CLASS (parent_class)->control (tool, action, display);
}

static void
picman_source_tool_button_press (PicmanTool            *tool,
                               const PicmanCoords    *coords,
                               guint32              time,
                               GdkModifierType      state,
                               PicmanButtonPressType  press_type,
                               PicmanDisplay         *display)
{
  PicmanPaintTool  *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);
  PicmanSourceCore *source      = PICMAN_SOURCE_CORE (paint_tool->core);
  GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  if ((state & (toggle_mask | GDK_SHIFT_MASK)) == toggle_mask)
    {
      source->set_source = TRUE;

      picman_source_tool_set_src_display (source_tool, display);
    }
  else
    {
      source->set_source = FALSE;
    }

  PICMAN_TOOL_CLASS (parent_class)->button_press (tool, coords, time, state,
                                                press_type, display);

  source_tool->src_x = source->src_x;
  source_tool->src_y = source->src_y;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_source_tool_motion (PicmanTool         *tool,
                         const PicmanCoords *coords,
                         guint32           time,
                         GdkModifierType   state,
                         PicmanDisplay      *display)
{
  PicmanSourceTool *source_tool = PICMAN_SOURCE_TOOL (tool);
  PicmanPaintTool  *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceCore *source      = PICMAN_SOURCE_CORE (paint_tool->core);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  PICMAN_TOOL_CLASS (parent_class)->motion (tool, coords, time, state, display);

  source_tool->src_x = source->src_x;
  source_tool->src_y = source->src_y;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_source_tool_modifier_key (PicmanTool        *tool,
                               GdkModifierType  key,
                               gboolean         press,
                               GdkModifierType  state,
                               PicmanDisplay     *display)
{
  PicmanSourceTool    *source_tool = PICMAN_SOURCE_TOOL (tool);
  PicmanPaintTool     *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceOptions *options     = PICMAN_SOURCE_TOOL_GET_OPTIONS (tool);

  if (picman_source_core_use_source (PICMAN_SOURCE_CORE (paint_tool->core),
                                   options) &&
      key == picman_get_toggle_behavior_mask ())
    {
      picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

      if (press)
        {
          paint_tool->status = source_tool->status_set_source;

          source_tool->show_source_outline = FALSE;
        }
      else
        {
          paint_tool->status = source_tool->status_paint;

          source_tool->show_source_outline = TRUE;
        }

      picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
    }

  PICMAN_TOOL_CLASS (parent_class)->modifier_key (tool, key, press, state,
                                                display);
}

static void
picman_source_tool_cursor_update (PicmanTool         *tool,
                                const PicmanCoords *coords,
                                GdkModifierType   state,
                                PicmanDisplay      *display)
{
  PicmanPaintTool      *paint_tool = PICMAN_PAINT_TOOL (tool);
  PicmanSourceOptions  *options    = PICMAN_SOURCE_TOOL_GET_OPTIONS (tool);
  PicmanCursorType      cursor     = PICMAN_CURSOR_MOUSE;
  PicmanCursorModifier  modifier   = PICMAN_CURSOR_MODIFIER_NONE;

  if (picman_source_core_use_source (PICMAN_SOURCE_CORE (paint_tool->core),
                                   options))
    {
      GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

      if ((state & (toggle_mask | GDK_SHIFT_MASK)) == toggle_mask)
        {
          cursor = PICMAN_CURSOR_CROSSHAIR_SMALL;
        }
      else if (! PICMAN_SOURCE_CORE (PICMAN_PAINT_TOOL (tool)->core)->src_drawable)
        {
          modifier = PICMAN_CURSOR_MODIFIER_BAD;
        }
    }

  picman_tool_control_set_cursor          (tool->control, cursor);
  picman_tool_control_set_cursor_modifier (tool->control, modifier);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_source_tool_oper_update (PicmanTool         *tool,
                              const PicmanCoords *coords,
                              GdkModifierType   state,
                              gboolean          proximity,
                              PicmanDisplay      *display)
{
  PicmanPaintTool     *paint_tool  = PICMAN_PAINT_TOOL (tool);
  PicmanSourceTool    *source_tool = PICMAN_SOURCE_TOOL (tool);
  PicmanSourceOptions *options     = PICMAN_SOURCE_TOOL_GET_OPTIONS (tool);
  PicmanSourceCore    *source;

  source = PICMAN_SOURCE_CORE (PICMAN_PAINT_TOOL (tool)->core);

  if (proximity)
    {
      if (picman_source_core_use_source (source, options))
        paint_tool->status_ctrl = source_tool->status_set_source_ctrl;
      else
        paint_tool->status_ctrl = NULL;
    }

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state, proximity,
                                               display);

  if (picman_source_core_use_source (source, options))
    {
      if (source->src_drawable == NULL)
        {
          GdkModifierType toggle_mask = picman_get_toggle_behavior_mask ();

          if (state & toggle_mask)
            {
              picman_tool_replace_status (tool, display, "%s",
                                        source_tool->status_set_source);
            }
          else
            {
              picman_tool_replace_status (tool, display, "%s-%s",
                                        picman_get_mod_string (toggle_mask),
                                        source_tool->status_set_source);
            }
        }
      else
        {
          picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

          source_tool->src_x = source->src_x;
          source_tool->src_y = source->src_y;

          if (! source->first_stroke)
            {
              switch (options->align_mode)
                {
                case PICMAN_SOURCE_ALIGN_YES:
                  source_tool->src_x = coords->x + source->offset_x;
                  source_tool->src_y = coords->y + source->offset_y;
                  break;

                case PICMAN_SOURCE_ALIGN_REGISTERED:
                  source_tool->src_x = coords->x;
                  source_tool->src_y = coords->y;
                  break;

                default:
                  break;
                }
            }

          picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
        }
    }
}

static void
picman_source_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanSourceTool    *source_tool = PICMAN_SOURCE_TOOL (draw_tool);
  PicmanSourceOptions *options     = PICMAN_SOURCE_TOOL_GET_OPTIONS (draw_tool);
  PicmanSourceCore    *source;

  source = PICMAN_SOURCE_CORE (PICMAN_PAINT_TOOL (draw_tool)->core);

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);

  if (picman_source_core_use_source (source, options) &&
      source->src_drawable && source_tool->src_display)
    {
      PicmanDisplayShell *src_shell;
      gint              off_x;
      gint              off_y;

      src_shell = picman_display_get_shell (source_tool->src_display);

      picman_item_get_offset (PICMAN_ITEM (source->src_drawable), &off_x, &off_y);

      if (source_tool->src_outline)
        {
          picman_display_shell_remove_tool_item (src_shell,
                                               source_tool->src_outline);
          source_tool->src_outline = NULL;
        }

      if (source_tool->show_source_outline)
        {
          source_tool->src_outline =
            picman_brush_tool_create_outline (PICMAN_BRUSH_TOOL (source_tool),
                                            source_tool->src_display,
                                            source_tool->src_x + off_x,
                                            source_tool->src_y + off_y,
                                            FALSE);

          if (source_tool->src_outline)
            {
              picman_display_shell_add_tool_item (src_shell,
                                                source_tool->src_outline);
              g_object_unref (source_tool->src_outline);
            }
        }

      if (! source_tool->src_handle)
        {
          source_tool->src_handle =
            picman_canvas_handle_new (src_shell,
                                    PICMAN_HANDLE_CROSS,
                                    PICMAN_HANDLE_ANCHOR_CENTER,
                                    source_tool->src_x + off_x,
                                    source_tool->src_y + off_y,
                                    PICMAN_TOOL_HANDLE_SIZE_CROSS,
                                    PICMAN_TOOL_HANDLE_SIZE_CROSS);
          picman_display_shell_add_tool_item (src_shell,
                                            source_tool->src_handle);
          g_object_unref (source_tool->src_handle);
        }
      else
        {
          picman_canvas_handle_set_position (source_tool->src_handle,
                                           source_tool->src_x + off_x,
                                           source_tool->src_y + off_y);
        }
    }
}

static void
picman_source_tool_set_src_display (PicmanSourceTool *source_tool,
                                  PicmanDisplay    *display)
{
  if (source_tool->src_display != display)
    {
      if (source_tool->src_display)
        {
          PicmanDisplayShell *src_shell;

          src_shell = picman_display_get_shell (source_tool->src_display);

          if (source_tool->src_handle)
            {
              picman_display_shell_remove_tool_item (src_shell,
                                                   source_tool->src_handle);
              source_tool->src_handle = NULL;
            }

          if (source_tool->src_outline)
            {
              picman_display_shell_remove_tool_item (src_shell,
                                                   source_tool->src_outline);
              source_tool->src_outline = NULL;
            }
        }

      source_tool->src_display = display;
    }
}
