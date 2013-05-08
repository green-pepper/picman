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

#include <time.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "file/file-utils.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmanuimanager.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-close.h"
#include "picmanimagewindow.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void      picman_display_shell_close_dialog       (PicmanDisplayShell *shell,
                                                        PicmanImage        *image);
static void      picman_display_shell_close_name_changed (PicmanImage        *image,
                                                        PicmanMessageBox   *box);
static void      picman_display_shell_close_exported     (PicmanImage        *image,
                                                        const gchar      *uri,
                                                        PicmanMessageBox   *box);
static gboolean  picman_display_shell_close_time_changed (PicmanMessageBox   *box);
static void      picman_display_shell_close_response     (GtkWidget        *widget,
                                                        gboolean          close,
                                                        PicmanDisplayShell *shell);
static void      picman_time_since                       (guint  then,
                                                        gint  *hours,
                                                        gint  *minutes);


/*  public functions  */

void
picman_display_shell_close (PicmanDisplayShell *shell,
                          gboolean          kill_it)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  image = picman_display_get_image (shell->display);

  /*  FIXME: picman_busy HACK not really appropriate here because we only
   *  want to prevent the busy image and display to be closed.  --Mitch
   */
  if (shell->display->picman->busy)
    return;

  /*  If the image has been modified, give the user a chance to save
   *  it before nuking it--this only applies if its the last view
   *  to an image canvas.  (a image with disp_count = 1)
   */
  if (! kill_it                                 &&
      image                                     &&
      picman_image_get_display_count (image) == 1 &&
      picman_image_is_dirty (image))
    {
      /*  If there's a save dialog active for this image, then raise it.
       *  (see bug #511965)
       */
      GtkWidget *dialog = g_object_get_data (G_OBJECT (image),
                                             "picman-file-save-dialog");
      if (dialog)
        {
          gtk_window_present (GTK_WINDOW (dialog));
        }
      else
        {
          picman_display_shell_close_dialog (shell, image);
        }
    }
  else if (image)
    {
      picman_display_close (shell->display);
    }
  else
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);

      if (window)
        {
          PicmanUIManager *manager = picman_image_window_get_ui_manager (window);

          /* Activate the action instead of simply calling picman_exit(), so
           * the quit action's sensitivity is taken into account.
           */
          picman_ui_manager_activate_action (manager, "file", "file-quit");
        }
    }
}


/*  private functions  */

#define RESPONSE_SAVE  1


static void
picman_display_shell_close_dialog (PicmanDisplayShell *shell,
                                 PicmanImage        *image)
{
  GtkWidget      *dialog;
  PicmanMessageBox *box;
  GClosure       *closure;
  GSource        *source;
  gchar          *title;
  const gchar    *uri;

  if (shell->close_dialog)
    {
      gtk_window_present (GTK_WINDOW (shell->close_dialog));
      return;
    }

  uri = picman_image_get_uri (image);

  title = g_strdup_printf (_("Close %s"), picman_image_get_display_name (image));

  shell->close_dialog =
    dialog = picman_message_dialog_new (title, GTK_STOCK_SAVE,
                                      GTK_WIDGET (shell),
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      picman_standard_help_func, NULL,
                                      NULL);
  g_free (title);

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          _("Close _without Saving"), GTK_RESPONSE_CLOSE,
                          GTK_STOCK_CANCEL,           GTK_RESPONSE_CANCEL,
                          (uri ?
                           GTK_STOCK_SAVE :
                           GTK_STOCK_SAVE_AS),        RESPONSE_SAVE,
                          NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_SAVE,
                                           GTK_RESPONSE_CLOSE,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &shell->close_dialog);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (picman_display_shell_close_response),
                    shell);

  box = PICMAN_MESSAGE_DIALOG (dialog)->box;

  g_signal_connect_object (image, "name-changed",
                           G_CALLBACK (picman_display_shell_close_name_changed),
                           box, 0);
  g_signal_connect_object (image, "exported",
                           G_CALLBACK (picman_display_shell_close_exported),
                           box, 0);

  picman_display_shell_close_name_changed (image, box);

  closure =
    g_cclosure_new_object (G_CALLBACK (picman_display_shell_close_time_changed),
                           G_OBJECT (box));

  /*  update every 10 seconds  */
  source = g_timeout_source_new_seconds (10);
  g_source_set_closure (source, closure);
  g_source_attach (source, NULL);
  g_source_unref (source);

  /*  The dialog is destroyed with the shell, so it should be safe
   *  to hold an image pointer for the lifetime of the dialog.
   */
  g_object_set_data (G_OBJECT (box), "picman-image", image);

  picman_display_shell_close_time_changed (box);

  gtk_widget_show (dialog);
}

static void
picman_display_shell_close_name_changed (PicmanImage      *image,
                                       PicmanMessageBox *box)
{
  GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET (box));

  if (GTK_IS_WINDOW (window))
    {
      gchar *title = g_strdup_printf (_("Close %s"),
				      picman_image_get_display_name (image));

      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title);
    }

  picman_message_box_set_primary_text (box,
                                     _("Save the changes to image '%s' "
                                       "before closing?"),
                                     picman_image_get_display_name (image));
}

static void
picman_display_shell_close_exported (PicmanImage      *image,
                                   const gchar    *uri,
                                   PicmanMessageBox *box)
{
  picman_display_shell_close_time_changed (box);
}

static gboolean
picman_display_shell_close_time_changed (PicmanMessageBox *box)
{
  PicmanImage   *image       = g_object_get_data (G_OBJECT (box), "picman-image");
  gint         dirty_time  = picman_image_get_dirty_time (image);
  gchar       *time_text   = NULL;
  gchar       *export_text = NULL;

  if (dirty_time)
    {
      gint hours   = 0;
      gint minutes = 0;

      picman_time_since (dirty_time, &hours, &minutes);

      if (hours > 0)
        {
          if (hours > 1 || minutes == 0)
            {
              time_text =
                g_strdup_printf (ngettext ("If you don't save the image, "
                                           "changes from the last hour "
                                           "will be lost.",
                                           "If you don't save the image, "
                                           "changes from the last %d "
                                           "hours will be lost.",
                                           hours), hours);
            }
          else
            {
              time_text =
                g_strdup_printf (ngettext ("If you don't save the image, "
                                           "changes from the last hour "
                                           "and %d minute will be lost.",
                                           "If you don't save the image, "
                                           "changes from the last hour "
                                           "and %d minutes will be lost.",
                                           minutes), minutes);
            }
        }
      else
        {
          time_text =
            g_strdup_printf (ngettext ("If you don't save the image, "
                                       "changes from the last minute "
                                       "will be lost.",
                                       "If you don't save the image, "
                                       "changes from the last %d "
                                       "minutes will be lost.",
                                       minutes), minutes);
        }
    }

  if (! picman_image_is_export_dirty (image))
    {
      const gchar *exported_uri = picman_image_get_exported_uri (image);

      if (! exported_uri)
        exported_uri = picman_image_get_imported_uri (image);

      export_text =
        g_strdup_printf (_("The image has been exported to '%s'."),
                         exported_uri);
    }

  if (time_text && export_text)
    picman_message_box_set_text (box, "%s\n\n%s", time_text, export_text);
  else if (time_text || export_text)
    picman_message_box_set_text (box, "%s", time_text ? time_text : export_text);
  else
    picman_message_box_set_text (box, "%s", time_text);

  g_free (time_text);
  g_free (export_text);

  return TRUE;
}

static void
picman_display_shell_close_response (GtkWidget        *widget,
                                   gint              response_id,
                                   PicmanDisplayShell *shell)
{
  gtk_widget_destroy (widget);

  switch (response_id)
    {
    case GTK_RESPONSE_CLOSE:
      picman_display_close (shell->display);
      break;

    case RESPONSE_SAVE:
      {
        PicmanImageWindow *window = picman_display_shell_get_window (shell);

        if (window)
          {
            PicmanUIManager *manager = picman_image_window_get_ui_manager (window);

            picman_image_window_set_active_shell (window, shell);

            picman_ui_manager_activate_action (manager,
                                             "file", "file-save-and-close");
          }
      }
      break;

    default:
      break;
    }
}

static void
picman_time_since (guint  then,
                 gint  *hours,
                 gint  *minutes)
{
  guint now  = time (NULL);
  guint diff = 1 + now - then;

  g_return_if_fail (now >= then);

  /*  first round up to the nearest minute  */
  diff = (diff + 59) / 60;

  /*  then optionally round minutes to multiples of 5 or 10  */
  if (diff > 50)
    diff = ((diff + 8) / 10) * 10;
  else if (diff > 20)
    diff = ((diff + 3) / 5) * 5;

  /*  determine full hours  */
  if (diff >= 60)
    {
      *hours = diff / 60;
      diff = (diff % 60);
    }

  /*  round up to full hours for 2 and more  */
  if (*hours > 1 && diff > 0)
    {
      *hours += 1;
      diff = 0;
    }

  *minutes = diff;
}
