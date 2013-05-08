/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanprogress.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "widgets/picmancontainerentry.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanprogressbox.h"

#include "file-open-location-dialog.h"

#include "picman-intl.h"


static void      file_open_location_response   (GtkDialog          *dialog,
                                                gint                response_id,
                                                Picman               *picman);

static gboolean  file_open_location_completion (GtkEntryCompletion *completion,
                                                const gchar        *key,
                                                GtkTreeIter        *iter,
                                                gpointer            data);


/*  public functions  */

GtkWidget *
file_open_location_dialog_new (Picman *picman)
{
  PicmanContext        *context;
  GtkWidget          *dialog;
  GtkWidget          *hbox;
  GtkWidget          *vbox;
  GtkWidget          *image;
  GtkWidget          *label;
  GtkWidget          *entry;
  GtkEntryCompletion *completion;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  dialog = picman_dialog_new (_("Open Location"),
                            "picman-file-open-location",
                            NULL, 0,
                            picman_standard_help_func,
                            PICMAN_HELP_FILE_OPEN_LOCATION,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OPEN,   GTK_RESPONSE_OK,

                            NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (file_open_location_response),
                    picman);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG(dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  image = gtk_image_new_from_stock (PICMAN_STOCK_WEB, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("Enter location (URI):"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  /* we don't want the context to affect the entry, so create
   * a scratch one instead of using e.g. the user context
   */
  context = picman_context_new (picman, "file-open-location-dialog", NULL);
  entry = picman_container_entry_new (picman->documents, context,
                                    PICMAN_VIEW_SIZE_SMALL, 0);
  g_object_unref (context);

  completion = gtk_entry_get_completion (GTK_ENTRY (entry));
  gtk_entry_completion_set_match_func (completion,
                                       file_open_location_completion,
                                       NULL, NULL);

  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_widget_set_size_request (entry, 400, -1);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (entry);

  g_object_set_data (G_OBJECT (dialog), "location-entry", entry);

  return dialog;
}


/*  private functions  */

static void
file_open_location_response (GtkDialog *dialog,
                             gint       response_id,
                             Picman      *picman)
{
  GtkWidget   *entry;
  GtkWidget   *box;
  const gchar *text = NULL;

  if (response_id != GTK_RESPONSE_OK)
    {
      box = g_object_get_data (G_OBJECT (dialog), "progress-box");

      if (box && PICMAN_PROGRESS_BOX (box)->active)
        picman_progress_cancel (PICMAN_PROGRESS (box));
      else
        gtk_widget_destroy (GTK_WIDGET (dialog));

      return;
    }

  entry = g_object_get_data (G_OBJECT (dialog), "location-entry");

  gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);
  gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_OK, FALSE);

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text && strlen (text))
    {
      PicmanImage         *image;
      gchar             *uri;
      gchar             *filename;
      gchar             *hostname;
      GError            *error  = NULL;
      PicmanPDBStatusType  status;

      filename = g_filename_from_uri (text, &hostname, NULL);

      if (filename)
        {
          uri = g_filename_to_uri (filename, hostname, &error);

          g_free (hostname);
          g_free (filename);
        }
      else
        {
          uri = file_utils_filename_to_uri (picman, text, &error);
        }

      box = picman_progress_box_new ();
      gtk_container_set_border_width (GTK_CONTAINER (box), 12);
      gtk_box_pack_end (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                        box, FALSE, FALSE, 0);

      g_object_set_data (G_OBJECT (dialog), "progress-box", box);

      if (uri)
        {
          gtk_widget_show (box);

          image = file_open_with_proc_and_display (picman,
                                                   picman_get_user_context (picman),
                                                   PICMAN_PROGRESS (box),
                                                   uri, text, FALSE, NULL,
                                                   &status, &error);

          if (image == NULL && status != PICMAN_PDB_CANCEL)
            {
              gchar *filename = file_utils_uri_display_name (uri);

              picman_message (picman, G_OBJECT (box), PICMAN_MESSAGE_ERROR,
                            _("Opening '%s' failed:\n\n%s"),
                            filename, error->message);
              g_clear_error (&error);

              g_free (filename);
            }

          g_free (uri);
        }
      else
        {
          picman_message (picman, G_OBJECT (box), PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed:\n\n%s"),
                        text, error->message);
          g_clear_error (&error);

          gtk_dialog_set_response_sensitive (dialog, GTK_RESPONSE_OK, TRUE);
          gtk_editable_set_editable (GTK_EDITABLE (entry), TRUE);

          return;
        }
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static gboolean
file_open_location_completion (GtkEntryCompletion *completion,
                               const gchar        *key,
                               GtkTreeIter        *iter,
                               gpointer            data)
{
  GtkTreeModel *model = gtk_entry_completion_get_model (completion);
  gchar        *name;
  gchar        *normalized;
  gchar        *case_normalized;
  gboolean      match;

  gtk_tree_model_get (model, iter,
                      1, &name,
                      -1);

  if (! name)
    return FALSE;

  normalized = g_utf8_normalize (name, -1, G_NORMALIZE_ALL);
  case_normalized = g_utf8_casefold (normalized, -1);

  match = (strncmp (key, case_normalized, strlen (key)) == 0);

  if (! match)
    {
      const gchar *colon = strchr (case_normalized, ':');

      if (colon && strlen (colon) > 2 && colon[1] == '/' && colon[2] == '/')
        match = (strncmp (key, colon + 3, strlen (key)) == 0);
    }

  g_free (normalized);
  g_free (case_normalized);
  g_free (name);

  return match;
}
