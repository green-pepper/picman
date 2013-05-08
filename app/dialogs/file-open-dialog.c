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
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanlayer.h"
#include "core/picmanprogress.h"

#include "file/file-open.h"
#include "file/file-utils.h"
#include "file/picman-file.h"

#include "widgets/picmanfiledialog.h"
#include "widgets/picmanhelp-ids.h"

#include "file-open-dialog.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void       file_open_dialog_response    (GtkWidget           *open_dialog,
                                                gint                 response_id,
                                                Picman                *picman);
static PicmanImage *file_open_dialog_open_image  (GtkWidget           *open_dialog,
                                                Picman                *picman,
                                                const gchar         *uri,
                                                PicmanPlugInProcedure *load_proc);
static gboolean   file_open_dialog_open_layers (GtkWidget           *open_dialog,
                                                PicmanImage           *image,
                                                const gchar         *uri,
                                                PicmanPlugInProcedure *load_proc);


/*  public functions  */

GtkWidget *
file_open_dialog_new (Picman *picman)
{
  GtkWidget           *dialog;
  PicmanFileDialogState *state;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  dialog = picman_file_dialog_new (picman,
                                 PICMAN_FILE_CHOOSER_ACTION_OPEN,
                                 _("Open Image"), "picman-file-open",
                                 GTK_STOCK_OPEN,
                                 PICMAN_HELP_FILE_OPEN);

  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), TRUE);

  state = g_object_get_data (G_OBJECT (picman), "picman-file-open-dialog-state");

  if (state)
    picman_file_dialog_set_state (PICMAN_FILE_DIALOG (dialog), state);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (file_open_dialog_response),
                    picman);

  return dialog;
}


/*  private functions  */

static void
file_open_dialog_response (GtkWidget *open_dialog,
                           gint       response_id,
                           Picman      *picman)
{
  PicmanFileDialog *dialog  = PICMAN_FILE_DIALOG (open_dialog);
  GSList         *uris;
  GSList         *list;
  gboolean        success = FALSE;

  g_object_set_data_full (G_OBJECT (picman), "picman-file-open-dialog-state",
                          picman_file_dialog_get_state (dialog),
                          (GDestroyNotify) picman_file_dialog_state_destroy);

  if (response_id != GTK_RESPONSE_OK)
    {
      if (! dialog->busy)
        gtk_widget_destroy (open_dialog);

      return;
    }

  uris = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (open_dialog));

  if (uris)
    g_object_set_data_full (G_OBJECT (picman), PICMAN_FILE_OPEN_LAST_URI_KEY,
                            g_strdup (uris->data), (GDestroyNotify) g_free);

  picman_file_dialog_set_sensitive (dialog, FALSE);

  /* When we are going to open new image windows, unset the transient
   * window. We don't need it since we will use gdk_window_raise() to
   * keep the dialog on top. And if we don't do it, then the dialog
   * will pull the image window it was invoked from on top of all the
   * new opened image windows, and we don't want that to happen.
   */
  if (! dialog->open_as_layers)
    gtk_window_set_transient_for (GTK_WINDOW (open_dialog), NULL);

  for (list = uris; list; list = g_slist_next (list))
    {
      gchar *filename = file_utils_filename_from_uri (list->data);

      if (filename)
        {
          gboolean regular = g_file_test (filename, G_FILE_TEST_IS_REGULAR);

          g_free (filename);

          if (! regular)
            continue;
        }

      if (dialog->open_as_layers)
        {
          if (! dialog->image)
            {
              dialog->image = file_open_dialog_open_image (open_dialog,
                                                           picman,
                                                           list->data,
                                                           dialog->file_proc);

              if (dialog->image)
                success = TRUE;
            }
          else if (file_open_dialog_open_layers (open_dialog,
                                                 dialog->image,
                                                 list->data,
                                                 dialog->file_proc))
            {
              success = TRUE;
            }
        }
      else
        {
          if (file_open_dialog_open_image (open_dialog,
                                           picman,
                                           list->data,
                                           dialog->file_proc))
            {
              success = TRUE;

              /* Make the dialog stay on top of all images we open if
               * we open say 10 at once
               */
              gdk_window_raise (gtk_widget_get_window (open_dialog));
            }
        }

      if (dialog->canceled)
        break;
    }

  if (success)
    {
      if (dialog->open_as_layers && dialog->image)
        picman_image_flush (dialog->image);

      gtk_widget_destroy (open_dialog);
    }
  else
    {
      picman_file_dialog_set_sensitive (dialog, TRUE);
    }

  g_slist_free_full (uris, (GDestroyNotify) g_free);
}

static PicmanImage *
file_open_dialog_open_image (GtkWidget           *open_dialog,
                             Picman                *picman,
                             const gchar         *uri,
                             PicmanPlugInProcedure *load_proc)
{
  PicmanImage         *image;
  PicmanPDBStatusType  status;
  GError            *error = NULL;

  image = file_open_with_proc_and_display (picman,
                                           picman_get_user_context (picman),
                                           PICMAN_PROGRESS (open_dialog),
                                           uri, uri, FALSE,
                                           load_proc,
                                           &status, &error);

  if (! image && status != PICMAN_PDB_CANCEL)
    {
      gchar *filename = file_utils_uri_display_name (uri);

      picman_message (picman, G_OBJECT (open_dialog), PICMAN_MESSAGE_ERROR,
                    _("Opening '%s' failed:\n\n%s"), filename, error->message);
      g_clear_error (&error);

      g_free (filename);
    }

  return image;
}

static gboolean
file_open_dialog_open_layers (GtkWidget           *open_dialog,
                              PicmanImage           *image,
                              const gchar         *uri,
                              PicmanPlugInProcedure *load_proc)
{
  GList             *new_layers;
  PicmanPDBStatusType  status;
  GError            *error = NULL;

  new_layers = file_open_layers (image->picman,
                                 picman_get_user_context (image->picman),
                                 PICMAN_PROGRESS (open_dialog),
                                 image, FALSE,
                                 uri, PICMAN_RUN_INTERACTIVE, load_proc,
                                 &status, &error);

  if (new_layers)
    {
      picman_image_add_layers (image, new_layers,
                             PICMAN_IMAGE_ACTIVE_PARENT, -1,
                             0, 0,
                             picman_image_get_width (image),
                             picman_image_get_height (image),
                             _("Open layers"));

      g_list_free (new_layers);

      return TRUE;
    }
  else if (status != PICMAN_PDB_CANCEL)
    {
      gchar *filename = file_utils_uri_display_name (uri);

      picman_message (image->picman, G_OBJECT (open_dialog), PICMAN_MESSAGE_ERROR,
                    _("Opening '%s' failed:\n\n%s"), filename, error->message);
      g_clear_error (&error);

      g_free (filename);
    }

  return FALSE;
}
