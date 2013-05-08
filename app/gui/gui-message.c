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

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "gui-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmanprogress.h"

#include "plug-in/picmanplugin.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandockable.h"
#include "widgets/picmanerrorconsole.h"
#include "widgets/picmanerrordialog.h"
#include "widgets/picmanprogressdialog.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanwidgets-utils.h"
#include "widgets/picmanwindowstrategy.h"

#include "gui-message.h"

#include "picman-intl.h"


static gboolean  gui_message_error_console (Picman                *picman,
                                            PicmanMessageSeverity  severity,
                                            const gchar         *domain,
                                            const gchar         *message);
static gboolean  gui_message_error_dialog  (Picman                *picman,
                                            GObject             *handler,
                                            PicmanMessageSeverity  severity,
                                            const gchar         *domain,
                                            const gchar         *message);
static void      gui_message_console       (PicmanMessageSeverity  severity,
                                            const gchar         *domain,
                                            const gchar         *message);


void
gui_message (Picman                *picman,
             GObject             *handler,
             PicmanMessageSeverity  severity,
             const gchar         *domain,
             const gchar         *message)
{
  switch (picman->message_handler)
    {
    case PICMAN_ERROR_CONSOLE:
      if (gui_message_error_console (picman, severity, domain, message))
        return;

      picman->message_handler = PICMAN_MESSAGE_BOX;
      /*  fallthru  */

    case PICMAN_MESSAGE_BOX:
      if (gui_message_error_dialog (picman, handler, severity, domain, message))
        return;

      picman->message_handler = PICMAN_CONSOLE;
      /*  fallthru  */

    case PICMAN_CONSOLE:
      gui_message_console (severity, domain, message);
      break;
    }
}

static gboolean
gui_message_error_console (Picman                *picman,
                           PicmanMessageSeverity  severity,
                           const gchar         *domain,
                           const gchar         *message)
{
  GtkWidget *dockable = NULL;

  /* try to avoid raising the error console for not so severe messages */
  if (severity < PICMAN_MESSAGE_ERROR)
    {
      GtkWidget *widget =
        picman_dialog_factory_find_widget (picman_dialog_factory_get_singleton (),
                                         "picman-error-console");
      if (PICMAN_IS_DOCKABLE (widget))
        dockable = widget;
    }

  if (! dockable)
    dockable =
      picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (picman)),
                                                 picman,
                                                 picman_dialog_factory_get_singleton (),
                                                 gdk_screen_get_default (),
                                                 "picman-error-console");

  if (dockable)
    {
      GtkWidget *child = gtk_bin_get_child (GTK_BIN (dockable));

      picman_error_console_add (PICMAN_ERROR_CONSOLE (child),
                              severity, domain, message);

      return TRUE;
    }

  return FALSE;
}

static void
progress_error_dialog_unset (PicmanProgress *progress)
{
  g_object_set_data (G_OBJECT (progress), "picman-error-dialog", NULL);
}

static GtkWidget *
progress_error_dialog (PicmanProgress *progress)
{
  GtkWidget *dialog;

  g_return_val_if_fail (PICMAN_IS_PROGRESS (progress), NULL);

  dialog = g_object_get_data (G_OBJECT (progress), "picman-error-dialog");

  if (! dialog)
    {
      dialog = picman_error_dialog_new (_("PICMAN Message"));

      g_object_set_data (G_OBJECT (progress), "picman-error-dialog", dialog);

      g_signal_connect_object (dialog, "destroy",
                               G_CALLBACK (progress_error_dialog_unset),
                               progress, G_CONNECT_SWAPPED);

      if (GTK_IS_WIDGET (progress))
        {
          GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (progress));

          if (GTK_IS_WINDOW (toplevel))
            gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                          GTK_WINDOW (toplevel));
        }
      else
        {
          guint32 window_id = picman_progress_get_window_id (progress);

          if (window_id)
            picman_window_set_transient_for (GTK_WINDOW (dialog), window_id);
        }
    }

  return dialog;
}

static GtkWidget *
global_error_dialog (void)
{
  return picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                         gdk_screen_get_default (),
                                         NULL /*ui_manager*/,
                                         "picman-error-dialog", -1,
                                         FALSE);
}

static gboolean
gui_message_error_dialog (Picman                *picman,
                          GObject             *handler,
                          PicmanMessageSeverity  severity,
                          const gchar         *domain,
                          const gchar         *message)
{
  GtkWidget *dialog;

  if (PICMAN_IS_PROGRESS (handler))
    {
      /* If there's already an error dialog associated with this
       * progress, then continue without trying picman_progress_message().
       */
      if (! g_object_get_data (handler, "picman-error-dialog") &&
          picman_progress_message (PICMAN_PROGRESS (handler), picman,
                                 severity, domain, message))
        {
          return TRUE;
        }
    }
  else if (GTK_IS_WIDGET (handler))
    {
      GtkWidget      *parent = GTK_WIDGET (handler);
      GtkMessageType  type   = GTK_MESSAGE_ERROR;

      switch (severity)
        {
        case PICMAN_MESSAGE_INFO:    type = GTK_MESSAGE_INFO;    break;
        case PICMAN_MESSAGE_WARNING: type = GTK_MESSAGE_WARNING; break;
        case PICMAN_MESSAGE_ERROR:   type = GTK_MESSAGE_ERROR;   break;
        }

      dialog =
        gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (parent)),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                type, GTK_BUTTONS_OK,
                                "%s", message);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);

      gtk_widget_show (dialog);

      return TRUE;
    }

  if (PICMAN_IS_PROGRESS (handler) && ! PICMAN_IS_PROGRESS_DIALOG (handler))
    dialog = progress_error_dialog (PICMAN_PROGRESS (handler));
  else
    dialog = global_error_dialog ();

  if (dialog)
    {
      picman_error_dialog_add (PICMAN_ERROR_DIALOG (dialog),
                             picman_get_message_stock_id (severity),
                             domain, message);
      gtk_window_present (GTK_WINDOW (dialog));

      return TRUE;
    }

  return FALSE;
}

static void
gui_message_console (PicmanMessageSeverity  severity,
                     const gchar         *domain,
                     const gchar         *message)
{
  const gchar *desc = "Message";

  picman_enum_get_value (PICMAN_TYPE_MESSAGE_SEVERITY, severity,
                       NULL, NULL, &desc, NULL);
  g_printerr ("%s-%s: %s\n\n", domain, desc, message);
}
