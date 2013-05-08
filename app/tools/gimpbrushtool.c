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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "tools-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmanbezierdesc.h"
#include "core/picmanbrush.h"
#include "core/picmanimage.h"
#include "core/picmantoolinfo.h"

#include "paint/picmanbrushcore.h"
#include "paint/picmanpaintoptions.h"

#include "display/picmancanvashandle.h"
#include "display/picmancanvaspath.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "picmanbrushtool.h"
#include "picmantoolcontrol.h"


static void   picman_brush_tool_constructed     (GObject           *object);

static void   picman_brush_tool_motion          (PicmanTool          *tool,
                                               const PicmanCoords  *coords,
                                               guint32            time,
                                               GdkModifierType    state,
                                               PicmanDisplay       *display);
static void   picman_brush_tool_oper_update     (PicmanTool          *tool,
                                               const PicmanCoords  *coords,
                                               GdkModifierType    state,
                                               gboolean           proximity,
                                               PicmanDisplay       *display);
static void   picman_brush_tool_cursor_update   (PicmanTool          *tool,
                                               const PicmanCoords  *coords,
                                               GdkModifierType    state,
                                               PicmanDisplay       *display);
static void   picman_brush_tool_options_notify  (PicmanTool          *tool,
                                               PicmanToolOptions   *options,
                                               const GParamSpec  *pspec);

static void   picman_brush_tool_draw            (PicmanDrawTool      *draw_tool);

static void   picman_brush_tool_brush_changed   (PicmanContext       *context,
                                               PicmanBrush         *brush,
                                               PicmanBrushTool     *brush_tool);
static void   picman_brush_tool_set_brush       (PicmanBrushCore     *brush_core,
                                               PicmanBrush         *brush,
                                               PicmanBrushTool     *brush_tool);
static void   picman_brush_tool_notify_brush    (PicmanDisplayConfig *config,
                                               GParamSpec        *pspec,
                                               PicmanBrushTool     *brush_tool);


G_DEFINE_TYPE (PicmanBrushTool, picman_brush_tool, PICMAN_TYPE_PAINT_TOOL)

#define parent_class picman_brush_tool_parent_class


static void
picman_brush_tool_class_init (PicmanBrushToolClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  object_class->constructed  = picman_brush_tool_constructed;

  tool_class->motion         = picman_brush_tool_motion;
  tool_class->oper_update    = picman_brush_tool_oper_update;
  tool_class->cursor_update  = picman_brush_tool_cursor_update;
  tool_class->options_notify = picman_brush_tool_options_notify;

  draw_tool_class->draw      = picman_brush_tool_draw;
}

static void
picman_brush_tool_init (PicmanBrushTool *brush_tool)
{
  PicmanTool *tool = PICMAN_TOOL (brush_tool);

  picman_tool_control_set_action_value_2 (tool->control,
                                        "tools/tools-paint-brush-size-set");
  picman_tool_control_set_action_value_3  (tool->control,
                                         "context/context-brush-aspect-set");
  picman_tool_control_set_action_value_4  (tool->control,
                                         "context/context-brush-angle-set");
  picman_tool_control_set_action_object_1 (tool->control,
                                         "context/context-brush-select-set");

  brush_tool->show_cursor = TRUE;
  brush_tool->draw_brush  = TRUE;
  brush_tool->brush_x     = 0.0;
  brush_tool->brush_y     = 0.0;
}

static void
picman_brush_tool_constructed (GObject *object)
{
  PicmanTool          *tool       = PICMAN_TOOL (object);
  PicmanPaintTool     *paint_tool = PICMAN_PAINT_TOOL (object);
  PicmanBrushTool     *brush_tool = PICMAN_BRUSH_TOOL (object);
  PicmanDisplayConfig *display_config;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_BRUSH_CORE (paint_tool->core));

  display_config = PICMAN_DISPLAY_CONFIG (tool->tool_info->picman->config);

  brush_tool->show_cursor = display_config->show_paint_tool_cursor;
  brush_tool->draw_brush  = display_config->show_brush_outline;

  g_signal_connect_object (display_config, "notify::show-paint-tool-cursor",
                           G_CALLBACK (picman_brush_tool_notify_brush),
                           brush_tool, 0);
  g_signal_connect_object (display_config, "notify::show-brush-outline",
                           G_CALLBACK (picman_brush_tool_notify_brush),
                           brush_tool, 0);

  g_signal_connect_object (picman_tool_get_options (tool), "brush-changed",
                           G_CALLBACK (picman_brush_tool_brush_changed),
                           brush_tool, 0);

  g_signal_connect_object (paint_tool->core, "set-brush",
                           G_CALLBACK (picman_brush_tool_set_brush),
                           brush_tool, 0);
}

static void
picman_brush_tool_motion (PicmanTool         *tool,
                        const PicmanCoords *coords,
                        guint32           time,
                        GdkModifierType   state,
                        PicmanDisplay      *display)
{
  PicmanBrushTool *brush_tool = PICMAN_BRUSH_TOOL (tool);

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  PICMAN_TOOL_CLASS (parent_class)->motion (tool, coords, time, state, display);

  if (! picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      brush_tool->brush_x = coords->x;
      brush_tool->brush_y = coords->y;
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_brush_tool_oper_update (PicmanTool         *tool,
                             const PicmanCoords *coords,
                             GdkModifierType   state,
                             gboolean          proximity,
                             PicmanDisplay      *display)
{
  PicmanBrushTool    *brush_tool    = PICMAN_BRUSH_TOOL (tool);
  PicmanPaintOptions *paint_options = PICMAN_PAINT_TOOL_GET_OPTIONS (tool);
  PicmanDrawable     *drawable;

  picman_draw_tool_pause (PICMAN_DRAW_TOOL (tool));

  PICMAN_TOOL_CLASS (parent_class)->oper_update (tool, coords, state,
                                               proximity, display);

  drawable = picman_image_get_active_drawable (picman_display_get_image (display));

  if (! picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)) &&
      drawable && proximity)
    {
      PicmanContext   *context    = PICMAN_CONTEXT (paint_options);
      PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);
      PicmanBrushCore *brush_core = PICMAN_BRUSH_CORE (paint_tool->core);

      brush_tool->brush_x = coords->x;
      brush_tool->brush_y = coords->y;

      picman_brush_core_set_brush (brush_core,
                                 picman_context_get_brush (context));

      picman_brush_core_set_dynamics (brush_core,
                                    picman_context_get_dynamics (context));

      if (PICMAN_BRUSH_CORE_GET_CLASS (brush_core)->handles_transforming_brush)
        {
          picman_brush_core_eval_transform_dynamics (brush_core,
                                                   drawable,
                                                   paint_options,
                                                   coords);
        }
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (tool));
}

static void
picman_brush_tool_cursor_update (PicmanTool         *tool,
                               const PicmanCoords *coords,
                               GdkModifierType   state,
                               PicmanDisplay      *display)
{
  PicmanBrushTool *brush_tool = PICMAN_BRUSH_TOOL (tool);
  PicmanBrushCore *brush_core = PICMAN_BRUSH_CORE (PICMAN_PAINT_TOOL (brush_tool)->core);

  if (! picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (tool)))
    {
      if (! brush_core->main_brush || ! brush_core->dynamics)
        {
          picman_tool_set_cursor (tool, display,
                                picman_tool_control_get_cursor (tool->control),
                                picman_tool_control_get_tool_cursor (tool->control),
                                PICMAN_CURSOR_MODIFIER_BAD);
          return;
        }
      else if (! brush_tool->show_cursor &&
               picman_tool_control_get_cursor_modifier (tool->control) !=
               PICMAN_CURSOR_MODIFIER_BAD)
        {
          picman_tool_set_cursor (tool, display,
                                PICMAN_CURSOR_NONE,
                                PICMAN_TOOL_CURSOR_NONE,
                                PICMAN_CURSOR_MODIFIER_NONE);
          return;
        }
    }

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool,  coords, state, display);
}

static void
picman_brush_tool_options_notify (PicmanTool         *tool,
                                PicmanToolOptions  *options,
                                const GParamSpec *pspec)
{
  PICMAN_TOOL_CLASS (parent_class)->options_notify (tool, options, pspec);

  if (! strcmp (pspec->name, "brush-size")  ||
      ! strcmp (pspec->name, "brush-angle") ||
      ! strcmp (pspec->name, "brush-aspect-ratio"))
    {
      PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (tool);
      PicmanBrushCore *brush_core = PICMAN_BRUSH_CORE (paint_tool->core);

      g_signal_emit_by_name (brush_core, "set-brush",
                             brush_core->main_brush);
    }
}

static void
picman_brush_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanBrushTool  *brush_tool = PICMAN_BRUSH_TOOL (draw_tool);
  PicmanCanvasItem *item;

  PICMAN_DRAW_TOOL_CLASS (parent_class)->draw (draw_tool);

  if (picman_color_tool_is_enabled (PICMAN_COLOR_TOOL (draw_tool)))
    return;

  item = picman_brush_tool_create_outline (brush_tool,
                                         draw_tool->display,
                                         brush_tool->brush_x,
                                         brush_tool->brush_y,
                                         ! brush_tool->show_cursor);

  if (item)
    {
      picman_draw_tool_add_item (draw_tool, item);
      g_object_unref (item);
    }
}

PicmanCanvasItem *
picman_brush_tool_create_outline (PicmanBrushTool *brush_tool,
                                PicmanDisplay   *display,
                                gdouble        x,
                                gdouble        y,
                                gboolean       draw_fallback)
{
  PicmanBrushCore        *brush_core;
  PicmanPaintOptions     *options;
  PicmanDisplayShell     *shell;
  const PicmanBezierDesc *boundary = NULL;
  gint                  width    = 0;
  gint                  height   = 0;

  g_return_val_if_fail (PICMAN_IS_BRUSH_TOOL (brush_tool), NULL);
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);

  if (! brush_tool->draw_brush)
    return NULL;

  brush_core = PICMAN_BRUSH_CORE (PICMAN_PAINT_TOOL (brush_tool)->core);
  options    = PICMAN_PAINT_TOOL_GET_OPTIONS (brush_tool);
  shell      = picman_display_get_shell (display);

  if (! brush_core->main_brush || ! brush_core->dynamics)
    return NULL;

  if (brush_core->scale > 0.0)
    boundary = picman_brush_transform_boundary (brush_core->main_brush,
                                              brush_core->scale,
                                              brush_core->aspect_ratio,
                                              brush_core->angle,
                                              brush_core->hardness,
                                              &width,
                                              &height);

  /*  don't draw the boundary if it becomes too small  */
  if (boundary                   &&
      SCALEX (shell, width)  > 4 &&
      SCALEY (shell, height) > 4)
    {
      x -= width  / 2.0;
      y -= height / 2.0;

      if (picman_paint_options_get_brush_mode (options) == PICMAN_BRUSH_HARD)
        {
#define EPSILON 0.000001
          /*  Add EPSILON before rounding since e.g.
           *  (5.0 - 0.5) may end up at (4.499999999....)
           *  due to floating point fnords
           */
          x = RINT (x + EPSILON);
          y = RINT (y + EPSILON);
#undef EPSILON
        }

      return picman_canvas_path_new (shell, boundary, x, y, FALSE,
                                   PICMAN_PATH_STYLE_OUTLINE);
    }
  else if (draw_fallback)
    {
      return picman_canvas_handle_new (shell,
                                     PICMAN_HANDLE_CROSS,
                                     PICMAN_HANDLE_ANCHOR_CENTER,
                                     x, y,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL,
                                     PICMAN_TOOL_HANDLE_SIZE_SMALL);
    }

  return NULL;
}

static void
picman_brush_tool_brush_changed (PicmanContext   *context,
                               PicmanBrush     *brush,
                               PicmanBrushTool *brush_tool)
{
  PicmanPaintTool *paint_tool = PICMAN_PAINT_TOOL (brush_tool);
  PicmanBrushCore *brush_core = PICMAN_BRUSH_CORE (paint_tool->core);

  picman_brush_core_set_brush (brush_core, brush);

}

static void
picman_brush_tool_set_brush (PicmanBrushCore *brush_core,
                           PicmanBrush     *brush,
                           PicmanBrushTool *brush_tool)
{
  picman_draw_tool_pause (PICMAN_DRAW_TOOL (brush_tool));

  if (PICMAN_BRUSH_CORE_GET_CLASS (brush_core)->handles_transforming_brush)
    {
      PicmanPaintCore *paint_core = PICMAN_PAINT_CORE (brush_core);

      picman_brush_core_eval_transform_dynamics (brush_core,
                                               NULL,
                                               PICMAN_PAINT_TOOL_GET_OPTIONS (brush_tool),
                                               &paint_core->cur_coords);
    }

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (brush_tool));
}

static void
picman_brush_tool_notify_brush (PicmanDisplayConfig *config,
                              GParamSpec        *pspec,
                              PicmanBrushTool     *brush_tool)
{
  picman_draw_tool_pause (PICMAN_DRAW_TOOL (brush_tool));

  brush_tool->show_cursor = config->show_paint_tool_cursor;
  brush_tool->draw_brush  = config->show_brush_outline;

  picman_draw_tool_resume (PICMAN_DRAW_TOOL (brush_tool));
}
