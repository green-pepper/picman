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

#include <gtk/gtk.h>

#include "display-types.h"

#include "core/picmanprogress.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmandisplayshell.h"
#include "picmandisplayshell-progress.h"
#include "picmanstatusbar.h"


static PicmanProgress *
picman_display_shell_progress_start (PicmanProgress *progress,
                                   const gchar  *message,
                                   gboolean      cancelable)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  return picman_progress_start (PICMAN_PROGRESS (statusbar), message, cancelable);
}

static void
picman_display_shell_progress_end (PicmanProgress *progress)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  picman_progress_end (PICMAN_PROGRESS (statusbar));
}

static gboolean
picman_display_shell_progress_is_active (PicmanProgress *progress)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  return picman_progress_is_active (PICMAN_PROGRESS (statusbar));
}

static void
picman_display_shell_progress_set_text (PicmanProgress *progress,
                                      const gchar  *message)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  picman_progress_set_text (PICMAN_PROGRESS (statusbar), message);
}

static void
picman_display_shell_progress_set_value (PicmanProgress *progress,
                                       gdouble       percentage)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  picman_progress_set_value (PICMAN_PROGRESS (statusbar), percentage);
}

static gdouble
picman_display_shell_progress_get_value (PicmanProgress *progress)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  return picman_progress_get_value (PICMAN_PROGRESS (statusbar));
}

static void
picman_display_shell_progress_pulse (PicmanProgress *progress)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  picman_progress_pulse (PICMAN_PROGRESS (statusbar));
}

static guint32
picman_display_shell_progress_get_window_id (PicmanProgress *progress)
{
  GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (progress));

  if (GTK_IS_WINDOW (toplevel))
    return picman_window_get_native_id (GTK_WINDOW (toplevel));

  return 0;
}

static gboolean
picman_display_shell_progress_message (PicmanProgress        *progress,
                                     Picman                *picman,
                                     PicmanMessageSeverity  severity,
                                     const gchar         *domain,
                                     const gchar         *message)
{
  PicmanDisplayShell *shell     = PICMAN_DISPLAY_SHELL (progress);
  PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

  switch (severity)
    {
    case PICMAN_MESSAGE_ERROR:
      /* error messages are never handled here */
      break;

    case PICMAN_MESSAGE_WARNING:
      /* warning messages go to the statusbar, if it's visible */
      if (! picman_statusbar_get_visible (statusbar))
        break;
      else
        return picman_progress_message (PICMAN_PROGRESS (statusbar), picman,
                                      severity, domain, message);

    case PICMAN_MESSAGE_INFO:
      /* info messages go to the statusbar;
       * if they are not handled there, they are swallowed
       */
      picman_progress_message (PICMAN_PROGRESS (statusbar), picman,
                             severity, domain, message);
      return TRUE;
    }

  return FALSE;
}

void
picman_display_shell_progress_iface_init (PicmanProgressInterface *iface)
{
  iface->start         = picman_display_shell_progress_start;
  iface->end           = picman_display_shell_progress_end;
  iface->is_active     = picman_display_shell_progress_is_active;
  iface->set_text      = picman_display_shell_progress_set_text;
  iface->set_value     = picman_display_shell_progress_set_value;
  iface->get_value     = picman_display_shell_progress_get_value;
  iface->pulse         = picman_display_shell_progress_pulse;
  iface->get_window_id = picman_display_shell_progress_get_window_id;
  iface->message       = picman_display_shell_progress_message;
}
