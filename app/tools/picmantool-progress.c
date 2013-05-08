/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantool-progress.c
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#include <gtk/gtk.h>

#include "tools-types.h"

#include "core/picmanprogress.h"

#include "widgets/picmanwidgets-utils.h"

#include "display/picmancanvasprogress.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"
#include "display/picmandisplayshell-items.h"
#include "display/picmandisplayshell-transform.h"

#include "picmantool.h"
#include "picmantool-progress.h"


/*  local function prototypes  */

static PicmanProgress * picman_tool_progress_start     (PicmanProgress        *progress,
                                                    const gchar         *message,
                                                    gboolean             cancelable);
static void           picman_tool_progress_end       (PicmanProgress        *progress);
static gboolean       picman_tool_progress_is_active (PicmanProgress        *progress);
static void           picman_tool_progress_set_text  (PicmanProgress        *progress,
                                                    const gchar         *message);
static void           picman_tool_progress_set_value (PicmanProgress        *progress,
                                                    gdouble              percentage);
static gdouble        picman_tool_progress_get_value (PicmanProgress        *progress);
static void           picman_tool_progress_pulse     (PicmanProgress        *progress);
static gboolean       picman_tool_progress_message   (PicmanProgress        *progress,
                                                    Picman                *picman,
                                                    PicmanMessageSeverity  severity,
                                                    const gchar         *domain,
                                                    const gchar         *message);


/*  public functions  */

void
picman_tool_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start     = picman_tool_progress_start;
  iface->end       = picman_tool_progress_end;
  iface->is_active = picman_tool_progress_is_active;
  iface->set_text  = picman_tool_progress_set_text;
  iface->set_value = picman_tool_progress_set_value;
  iface->get_value = picman_tool_progress_get_value;
  iface->pulse     = picman_tool_progress_pulse;
  iface->message   = picman_tool_progress_message;
}


/*  private functions  */

static PicmanProgress *
picman_tool_progress_start (PicmanProgress *progress,
                          const gchar  *message,
                          gboolean      cancelable)
{
  PicmanTool         *tool = PICMAN_TOOL (progress);
  PicmanDisplayShell *shell;
  gint              x, y;

  g_return_val_if_fail (PICMAN_IS_DISPLAY (tool->display), NULL);
  g_return_val_if_fail (tool->progress == NULL, NULL);

  shell = picman_display_get_shell (tool->display);

  x = shell->disp_width  / 2;
  y = shell->disp_height / 2;

  picman_display_shell_unzoom_xy (shell, x, y, &x, &y, FALSE);

  tool->progress = picman_canvas_progress_new (shell,
                                             PICMAN_HANDLE_ANCHOR_CENTER,
                                             x, y);
  picman_display_shell_add_unrotated_item (shell, tool->progress);
  g_object_unref (tool->progress);

  picman_progress_start (PICMAN_PROGRESS (tool->progress),
                       message, FALSE);
  picman_widget_flush_expose (shell->canvas);

  tool->progress_display = tool->display;

  return progress;
}

static void
picman_tool_progress_end (PicmanProgress *progress)
{
  PicmanTool *tool = PICMAN_TOOL (progress);

  if (tool->progress)
    {
      PicmanDisplayShell *shell = picman_display_get_shell (tool->progress_display);

      picman_progress_end (PICMAN_PROGRESS (tool->progress));
      picman_display_shell_remove_unrotated_item (shell, tool->progress);

      tool->progress         = NULL;
      tool->progress_display = NULL;
    }
}

static gboolean
picman_tool_progress_is_active (PicmanProgress *progress)
{
  PicmanTool *tool = PICMAN_TOOL (progress);

  return tool->progress != NULL;
}

static void
picman_tool_progress_set_text (PicmanProgress *progress,
                             const gchar  *message)
{
  PicmanTool *tool = PICMAN_TOOL (progress);

  if (tool->progress)
    {
      PicmanDisplayShell *shell = picman_display_get_shell (tool->progress_display);

      picman_progress_set_text (PICMAN_PROGRESS (tool->progress), message);
      picman_widget_flush_expose (shell->canvas);
    }
}

static void
picman_tool_progress_set_value (PicmanProgress *progress,
                              gdouble       percentage)
{
  PicmanTool *tool = PICMAN_TOOL (progress);

  if (tool->progress)
    {
      PicmanDisplayShell *shell = picman_display_get_shell (tool->progress_display);

      picman_progress_set_value (PICMAN_PROGRESS (tool->progress), percentage);
      picman_widget_flush_expose (shell->canvas);
    }
}

static gdouble
picman_tool_progress_get_value (PicmanProgress *progress)
{
  PicmanTool *tool = PICMAN_TOOL (progress);

  if (tool->progress)
    return picman_progress_get_value (PICMAN_PROGRESS (tool->progress));

  return 0.0;
}

static void
picman_tool_progress_pulse (PicmanProgress *progress)
{
}

static gboolean
picman_tool_progress_message (PicmanProgress        *progress,
                            Picman                *picman,
                            PicmanMessageSeverity  severity,
                            const gchar         *domain,
                            const gchar         *message)
{
  return FALSE;
}
