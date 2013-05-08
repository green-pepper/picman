/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * user-install-dialog.c
 * Copyright (C) 2000-2006 Michael Natterer and Sven Neumann
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman-user-install.h"

#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "user-install-dialog.h"

#include "picman-intl.h"


static GtkWidget * user_install_dialog_new (PicmanUserInstall *install);
static void        user_install_dialog_log (const gchar     *message,
					    gboolean         error,
					    gpointer         data);


gboolean
user_install_dialog_run (PicmanUserInstall *install)
{
  GtkWidget *dialog;
  gboolean   success;

  g_return_val_if_fail (install != NULL, FALSE);

  dialog = user_install_dialog_new (install);

  success = picman_user_install_run (install);

  if (! success)
    {
      g_signal_connect (dialog, "response",
			G_CALLBACK (gtk_main_quit),
			NULL);

      gtk_widget_show (dialog);

      gtk_main ();
    }

  gtk_widget_destroy (dialog);

  return success;
}

static GtkWidget *
user_install_dialog_new (PicmanUserInstall *install)
{
  GtkWidget     *dialog;
  GtkWidget     *frame;
  GtkWidget     *scrolled;
  GtkTextBuffer *buffer;
  GtkWidget     *view;

  picman_stock_init ();

  dialog = picman_message_dialog_new (_("PICMAN User Installation"),
				    PICMAN_STOCK_WILBER_EEK,
				    NULL, 0, NULL, NULL,

				    GTK_STOCK_QUIT, GTK_RESPONSE_OK,

				    NULL);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
				     _("User installation failed!"));
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
			     _("The PICMAN user installation failed; "
			       "see the log for details."));

  frame = picman_frame_new (_("Installation Log"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scrolled);
  gtk_widget_show (scrolled);

  buffer = gtk_text_buffer_new (NULL);

  gtk_text_buffer_create_tag (buffer, "bold",
                              "weight", PANGO_WEIGHT_BOLD,
                              NULL);

  view = gtk_text_view_new_with_buffer (buffer);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
  gtk_widget_set_size_request (view, -1, 200);
  gtk_container_add (GTK_CONTAINER (scrolled), view);
  gtk_widget_show (view);

  g_object_unref (buffer);

  picman_user_install_set_log_handler (install, user_install_dialog_log, buffer);

  return dialog;
}

static void
user_install_dialog_log (const gchar *message,
                         gboolean     error,
                         gpointer     data)
{
  GtkTextBuffer *buffer = GTK_TEXT_BUFFER (data);
  GtkTextIter    cursor;

  gtk_text_buffer_get_end_iter (buffer, &cursor);

  if (error && message)
    {
      gtk_text_buffer_insert_with_tags_by_name (buffer, &cursor, message, -1,
                                                "bold", NULL);
    }
  else if (message)
    {
      gtk_text_buffer_insert (buffer, &cursor, message, -1);
    }

  gtk_text_buffer_insert (buffer, &cursor, "\n", -1);
}
