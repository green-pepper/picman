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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "tools-types.h"

#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasrectangle.h"
#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-scale.h"

#include "picmanmagnifyoptions.h"
#include "picmanmagnifytool.h"
#include "picmantoolcontrol.h"

#include "picman-intl.h"


static void   picman_magnify_tool_button_press   (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanButtonPressType    press_type,
                                                PicmanDisplay           *display);
static void   picman_magnify_tool_button_release (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanButtonReleaseType  release_type,
                                                PicmanDisplay           *display);
static void   picman_magnify_tool_motion         (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                guint32                time,
                                                GdkModifierType        state,
                                                PicmanDisplay           *display);
static void   picman_magnify_tool_modifier_key   (PicmanTool              *tool,
                                                GdkModifierType        key,
                                                gboolean               press,
                                                GdkModifierType        state,
                                                PicmanDisplay           *display);
static void   picman_magnify_tool_cursor_update  (PicmanTool              *tool,
                                                const PicmanCoords      *coords,
                                                GdkModifierType        state,
                                                PicmanDisplay           *display);

static void   picman_magnify_tool_draw           (PicmanDrawTool          *draw_tool);

static void   picman_magnify_tool_update_items   (PicmanMagnifyTool       *magnify);


G_DEFINE_TYPE (PicmanMagnifyTool, picman_magnify_tool, PICMAN_TYPE_DRAW_TOOL)

#define parent_class picman_magnify_tool_parent_class


void
picman_magnify_tool_register (PicmanToolRegisterCallback  callback,
                            gpointer                  data)
{
  (* callback) (PICMAN_TYPE_MAGNIFY_TOOL,
                PICMAN_TYPE_MAGNIFY_OPTIONS,
                picman_magnify_options_gui,
                0,
                "picman-zoom-tool",
                _("Zoom"),
                _("Zoom Tool: Adjust the zoom level"),
                N_("_Zoom"), "Z",
                NULL, PICMAN_HELP_TOOL_ZOOM,
                PICMAN_STOCK_TOOL_ZOOM,
                data);
}

static void
picman_magnify_tool_class_init (PicmanMagnifyToolClass *klass)
{
  PicmanToolClass     *tool_class      = PICMAN_TOOL_CLASS (klass);
  PicmanDrawToolClass *draw_tool_class = PICMAN_DRAW_TOOL_CLASS (klass);

  tool_class->button_press   = picman_magnify_tool_button_press;
  tool_class->button_release = picman_magnify_tool_button_release;
  tool_class->motion         = picman_magnify_tool_motion;
  tool_class->modifier_key   = picman_magnify_tool_modifier_key;
  tool_class->cursor_update  = picman_magnify_tool_cursor_update;

  draw_tool_class->draw      = picman_magnify_tool_draw;
}

static void
picman_magnify_tool_init (PicmanMagnifyTool *magnify_tool)
{
  PicmanTool *tool = PICMAN_TOOL (magnify_tool);

  picman_tool_control_set_scroll_lock            (tool->control, TRUE);
  picman_tool_control_set_handle_empty_image     (tool->control, TRUE);
  picman_tool_control_set_wants_click            (tool->control, TRUE);
  picman_tool_control_set_snap_to                (tool->control, FALSE);

  picman_tool_control_set_tool_cursor            (tool->control,
                                                PICMAN_TOOL_CURSOR_ZOOM);
  picman_tool_control_set_cursor_modifier        (tool->control,
                                                PICMAN_CURSOR_MODIFIER_PLUS);
  picman_tool_control_set_toggle_cursor_modifier (tool->control,
                                                PICMAN_CURSOR_MODIFIER_MINUS);

  magnify_tool->x = 0;
  magnify_tool->y = 0;
  magnify_tool->w = 0;
  magnify_tool->h = 0;
}

static void
picman_magnify_tool_button_press (PicmanTool            *tool,
                                const PicmanCoords    *coords,
                                guint32              time,
                                GdkModifierType      state,
                                PicmanButtonPressType  press_type,
                                PicmanDisplay         *display)
{
  PicmanMagnifyTool *magnify = PICMAN_MAGNIFY_TOOL (tool);

  magnify->x = coords->x;
  magnify->y = coords->y;
  magnify->w = 0;
  magnify->h = 0;

  picman_tool_control_activate (tool->control);
  tool->display = display;

  picman_draw_tool_start (PICMAN_DRAW_TOOL (tool), display);
}

static void
picman_magnify_tool_button_release (PicmanTool              *tool,
                                  const PicmanCoords      *coords,
                                  guint32                time,
                                  GdkModifierType        state,
                                  PicmanButtonReleaseType  release_type,
                                  PicmanDisplay           *display)
{
  PicmanMagnifyTool    *magnify = PICMAN_MAGNIFY_TOOL (tool);
  PicmanMagnifyOptions *options = PICMAN_MAGNIFY_TOOL_GET_OPTIONS (tool);
  PicmanDisplayShell   *shell   = picman_display_get_shell (tool->display);

  picman_draw_tool_stop (PICMAN_DRAW_TOOL (tool));

  picman_tool_control_halt (tool->control);

  switch (release_type)
    {
    case PICMAN_BUTTON_RELEASE_CLICK:
    case PICMAN_BUTTON_RELEASE_NO_MOTION:
      picman_display_shell_scale (shell,
                                options->zoom_type,
                                0.0,
                                PICMAN_ZOOM_FOCUS_POINTER);
      break;

    case PICMAN_BUTTON_RELEASE_NORMAL:
      {
        gdouble x1, y1, x2, y2;
        gdouble width, height;
        gdouble current_scale;
        gdouble new_scale;
        gdouble display_width;
        gdouble display_height;
        gdouble factor = 1.0;

        x1     = (magnify->w < 0) ?  magnify->x + magnify->w : magnify->x;
        y1     = (magnify->h < 0) ?  magnify->y + magnify->h : magnify->y;
        width  = (magnify->w < 0) ? -magnify->w : magnify->w;
        height = (magnify->h < 0) ? -magnify->h : magnify->h;
        x2     = x1 + width;
        y2     = y1 + height;

        width  = MAX (1.0, width);
        height = MAX (1.0, height);

        current_scale = picman_zoom_model_get_factor (shell->zoom);

        display_width  = FUNSCALEX (shell, shell->disp_width);
        display_height = FUNSCALEY (shell, shell->disp_height);

        switch (options->zoom_type)
          {
          case PICMAN_ZOOM_IN:
            factor = MIN ((display_width  / width),
                          (display_height / height));
            break;

          case PICMAN_ZOOM_OUT:
            factor = MAX ((width  / display_width),
                          (height / display_height));
            break;

          default:
            break;
          }

        new_scale = current_scale * factor;

        if (new_scale != current_scale)
          {
            gint    offset_x = 0;
            gint    offset_y = 0;
            gdouble xres;
            gdouble yres;
            gdouble screen_xres;
            gdouble screen_yres;

            picman_image_get_resolution (picman_display_get_image (display),
                                       &xres, &yres);
            picman_display_shell_get_screen_resolution (shell,
                                                      &screen_xres, &screen_yres);

            switch (options->zoom_type)
              {
              case PICMAN_ZOOM_IN:
                /*  move the center of the rectangle to the center of the
                 *  viewport:
                 *
                 *  new_offset = center of rectangle in new scale screen coords
                 *               including offset
                 *               -
                 *               center of viewport in screen coords without
                 *               offset
                 */
                offset_x = RINT (new_scale * ((x1 + x2) / 2.0) *
                                 screen_xres / xres -
                                 (shell->disp_width / 2.0));

                offset_y = RINT (new_scale * ((y1 + y2) / 2.0) *
                                 screen_yres / yres -
                                 (shell->disp_height / 2.0));
                break;

              case PICMAN_ZOOM_OUT:
                /*  move the center of the viewport to the center of the
                 *  rectangle:
                 *
                 *  new_offset = center of viewport in new scale screen coords
                 *               including offset
                 *               -
                 *               center of rectangle in screen coords without
                 *               offset
                 */
                offset_x = RINT (new_scale * UNSCALEX (shell,
                                                       shell->offset_x +
                                                       shell->disp_width / 2.0) *
                                 screen_xres / xres -
                                 (SCALEX (shell, (x1 + x2) / 2.0) -
                                  shell->offset_x));

                offset_y = RINT (new_scale * UNSCALEY (shell,
                                                       shell->offset_y +
                                                       shell->disp_height / 2.0) *
                                 screen_yres / yres -
                                 (SCALEY (shell, (y1 + y2) / 2.0) -
                                  shell->offset_y));
                break;

              default:
                break;
              }

            picman_display_shell_scale_by_values (shell,
                                                new_scale,
                                                offset_x, offset_y,
                                                options->auto_resize);
          }
      }
      break;

    default:
      break;
    }
}

static void
picman_magnify_tool_motion (PicmanTool         *tool,
                          const PicmanCoords *coords,
                          guint32           time,
                          GdkModifierType   state,
                          PicmanDisplay      *display)
{
  PicmanMagnifyTool *magnify = PICMAN_MAGNIFY_TOOL (tool);

  magnify->w = coords->x - magnify->x;
  magnify->h = coords->y - magnify->y;

  picman_magnify_tool_update_items (magnify);
}

static void
picman_magnify_tool_modifier_key (PicmanTool        *tool,
                                GdkModifierType  key,
                                gboolean         press,
                                GdkModifierType  state,
                                PicmanDisplay     *display)
{
  PicmanMagnifyOptions *options = PICMAN_MAGNIFY_TOOL_GET_OPTIONS (tool);

  if (key == picman_get_toggle_behavior_mask ())
    {
      switch (options->zoom_type)
        {
        case PICMAN_ZOOM_IN:
          g_object_set (options, "zoom-type", PICMAN_ZOOM_OUT, NULL);
          break;

        case PICMAN_ZOOM_OUT:
          g_object_set (options, "zoom-type", PICMAN_ZOOM_IN, NULL);
          break;

        default:
          break;
        }
    }
}

static void
picman_magnify_tool_cursor_update (PicmanTool         *tool,
                                 const PicmanCoords *coords,
                                 GdkModifierType   state,
                                 PicmanDisplay      *display)
{
  PicmanMagnifyOptions *options = PICMAN_MAGNIFY_TOOL_GET_OPTIONS (tool);

  picman_tool_control_set_toggled (tool->control,
                                 options->zoom_type == PICMAN_ZOOM_OUT);

  PICMAN_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, display);
}

static void
picman_magnify_tool_draw (PicmanDrawTool *draw_tool)
{
  PicmanMagnifyTool *magnify = PICMAN_MAGNIFY_TOOL (draw_tool);

  magnify->rectangle =
    picman_draw_tool_add_rectangle (draw_tool, FALSE,
                                  magnify->x,
                                  magnify->y,
                                  magnify->w,
                                  magnify->h);
}

static void
picman_magnify_tool_update_items (PicmanMagnifyTool *magnify)
{
  if (picman_draw_tool_is_active (PICMAN_DRAW_TOOL (magnify)))
    {
      picman_canvas_rectangle_set (magnify->rectangle,
                                 magnify->x,
                                 magnify->y,
                                 magnify->w,
                                 magnify->h);
    }
}
