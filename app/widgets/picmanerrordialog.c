/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanerrordialog.c
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "picmanerrordialog.h"
#include "picmanmessagebox.h"

#include "picman-intl.h"

#define PICMAN_ERROR_DIALOG_MAX_MESSAGES 3


static void   picman_error_dialog_finalize (GObject   *object);
static void   picman_error_dialog_response (GtkDialog *dialog,
                                          gint       response_id);


G_DEFINE_TYPE (PicmanErrorDialog, picman_error_dialog, PICMAN_TYPE_DIALOG)

#define parent_class picman_error_dialog_parent_class


static void
picman_error_dialog_class_init (PicmanErrorDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

  object_class->finalize = picman_error_dialog_finalize;

  dialog_class->response = picman_error_dialog_response;
}

static void
picman_error_dialog_init (PicmanErrorDialog *dialog)
{
  gtk_window_set_role (GTK_WINDOW (dialog), "picman-message");

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          GTK_STOCK_OK, GTK_RESPONSE_CLOSE,
                          NULL);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  dialog->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      dialog->vbox, TRUE, TRUE, 0);
  gtk_widget_show (dialog->vbox);

  dialog->last_box     = NULL;
  dialog->last_domain  = NULL;
  dialog->last_message = NULL;
  dialog->num_messages = 0;
}

static void
picman_error_dialog_finalize (GObject *object)
{
  PicmanErrorDialog *dialog = PICMAN_ERROR_DIALOG (object);

  if (dialog->last_domain)
    {
      g_free (dialog->last_domain);
      dialog->last_domain = NULL;
    }
  if (dialog->last_message)
    {
      g_free (dialog->last_message);
      dialog->last_message = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_error_dialog_response (GtkDialog *dialog,
                            gint       response_id)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));
}


/*  public functions  */

GtkWidget *
picman_error_dialog_new (const gchar *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (PICMAN_TYPE_ERROR_DIALOG,
                       "title", title,
                       NULL);
}

void
picman_error_dialog_add (PicmanErrorDialog *dialog,
                       const gchar     *stock_id,
                       const gchar     *domain,
                       const gchar     *message)
{
  GtkWidget *box;
  gboolean   overflow = FALSE;

  g_return_if_fail (PICMAN_IS_ERROR_DIALOG (dialog));
  g_return_if_fail (domain != NULL);
  g_return_if_fail (message != NULL);

  if (dialog->last_box     &&
      dialog->last_domain  && strcmp (dialog->last_domain,  domain)  == 0 &&
      dialog->last_message && strcmp (dialog->last_message, message) == 0)
    {
      if (picman_message_box_repeat (PICMAN_MESSAGE_BOX (dialog->last_box)))
        return;
    }

  if (dialog->num_messages >= PICMAN_ERROR_DIALOG_MAX_MESSAGES)
    {
      g_printerr ("%s: %s\n\n", domain, message);

      overflow = TRUE;
      stock_id = PICMAN_STOCK_WILBER_EEK;
      domain   = _("Too many error messages!");
      message  = _("Messages are redirected to stderr.");

      if (dialog->last_domain  && strcmp (dialog->last_domain,  domain)  == 0 &&
          dialog->last_message && strcmp (dialog->last_message, message) == 0)
        {
          return;
        }
    }

  box = g_object_new (PICMAN_TYPE_MESSAGE_BOX,
                      "stock-id", stock_id,
                      NULL);

  dialog->num_messages++;

  if (overflow)
    picman_message_box_set_primary_text (PICMAN_MESSAGE_BOX (box), "%s", domain);
  else
    picman_message_box_set_primary_text (PICMAN_MESSAGE_BOX (box),
                                       /* %s is a message domain,
                                        * like "PICMAN Message" or
                                        * "PNG Message"
                                        */
                                       _("%s Message"), domain);

  picman_message_box_set_text (PICMAN_MESSAGE_BOX (box), "%s", message);

  gtk_box_pack_start (GTK_BOX (dialog->vbox), box, TRUE, TRUE, 0);
  gtk_widget_show (box);

  dialog->last_box = box;

  g_free (dialog->last_domain);
  dialog->last_domain = g_strdup (domain);

  g_free (dialog->last_message);
  dialog->last_message = g_strdup (message);
}
