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

#include "display-types.h"

#include "widgets/picmandeviceinfo.h"
#include "widgets/picmandeviceinfo-coords.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-autoscroll.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-transform.h"

#include "tools/tools-types.h"
#include "tools/tool_manager.h"
#include "tools/picmantool.h"
#include "tools/picmantoolcontrol.h"


#define AUTOSCROLL_DT  20
#define AUTOSCROLL_DX  0.1


typedef struct
{
  GdkEventMotion  *mevent;
  PicmanDeviceInfo  *device;
  guint32          time;
  GdkModifierType  state;
  guint            timeout_id;
} ScrollInfo;


/*  local function prototypes  */

static gboolean picman_display_shell_autoscroll_timeout (gpointer data);


/*  public functions  */

void
picman_display_shell_autoscroll_start (PicmanDisplayShell *shell,
                                     GdkModifierType   state,
                                     GdkEventMotion   *mevent)
{
  ScrollInfo *info;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->scroll_info)
    return;

  info = g_slice_new0 (ScrollInfo);

  info->mevent     = mevent;
  info->device     = picman_device_info_get_by_device (mevent->device);
  info->time       = gdk_event_get_time ((GdkEvent *) mevent);
  info->state      = state;
  info->timeout_id = g_timeout_add (AUTOSCROLL_DT,
                                    picman_display_shell_autoscroll_timeout,
                                    shell);

  shell->scroll_info = info;
}

void
picman_display_shell_autoscroll_stop (PicmanDisplayShell *shell)
{
  ScrollInfo *info;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (! shell->scroll_info)
    return;

  info = shell->scroll_info;

  if (info->timeout_id)
    {
      g_source_remove (info->timeout_id);
      info->timeout_id = 0;
    }

  g_slice_free (ScrollInfo, info);
  shell->scroll_info = NULL;
}


/*  private functions  */

static gboolean
picman_display_shell_autoscroll_timeout (gpointer data)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (data);
  ScrollInfo       *info  = shell->scroll_info;
  PicmanCoords        device_coords;
  PicmanCoords        image_coords;
  gint              dx = 0;
  gint              dy = 0;

  picman_device_info_get_device_coords (info->device,
                                      gtk_widget_get_window (shell->canvas),
                                      &device_coords);

  if (device_coords.x < 0)
    dx = device_coords.x;
  else if (device_coords.x > shell->disp_width)
    dx = device_coords.x - shell->disp_width;

  if (device_coords.y < 0)
    dy = device_coords.y;
  else if (device_coords.y > shell->disp_height)
    dy = device_coords.y - shell->disp_height;

  if (dx || dy)
    {
      PicmanDisplay *display         = shell->display;
      PicmanTool    *active_tool     = tool_manager_get_active (display->picman);
      gint         scroll_amount_x = AUTOSCROLL_DX * dx;
      gint         scroll_amount_y = AUTOSCROLL_DX * dy;

      info->time += AUTOSCROLL_DT;

      picman_display_shell_scroll_unoverscrollify (shell,
                                                 scroll_amount_x,
                                                 scroll_amount_y,
                                                 &scroll_amount_x,
                                                 &scroll_amount_y);

      picman_display_shell_scroll (shell,
                                 scroll_amount_x,
                                 scroll_amount_y);

      picman_display_shell_untransform_coords (shell,
                                             &device_coords,
                                             &image_coords);

      if (picman_tool_control_get_snap_to (active_tool->control))
        {
          gint x, y, width, height;

          picman_tool_control_get_snap_offsets (active_tool->control,
                                              &x, &y, &width, &height);

          picman_display_shell_snap_coords (shell,
                                          &image_coords,
                                          x, y, width, height);
        }

      tool_manager_motion_active (display->picman,
                                  &image_coords,
                                  info->time, info->state,
                                  display);

      return TRUE;
    }
  else
    {
      g_slice_free (ScrollInfo, info);
      shell->scroll_info = NULL;

      return FALSE;
    }
}
