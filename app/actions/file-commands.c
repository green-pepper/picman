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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"
#include "core/picmantemplate.h"

#include "plug-in/picmanpluginmanager.h"

#include "file/file-open.h"
#include "file/file-procedure.h"
#include "file/file-save.h"
#include "file/file-utils.h"
#include "file/picman-file.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanfiledialog.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"

#include "dialogs/file-save-dialog.h"

#include "actions.h"
#include "file-commands.h"

#include "picman-intl.h"


#define REVERT_DATA_KEY "revert-confirm-dialog"


/*  local function prototypes  */

static void     file_open_dialog_show        (Picman                *picman,
                                              GtkWidget           *parent,
                                              const gchar         *title,
                                              PicmanImage           *image,
                                              const gchar         *uri,
                                              gboolean             open_as_layers);
static void     file_save_dialog_show        (Picman                *picman,
                                              PicmanImage           *image,
                                              GtkWidget           *parent,
                                              const gchar         *title,
                                              gboolean             save_a_copy,
                                              gboolean             close_after_saving,
                                              PicmanDisplay         *display);
static void     file_export_dialog_show      (Picman                *picman,
                                              PicmanImage           *image,
                                              GtkWidget           *parent);
static void     file_save_dialog_destroyed   (GtkWidget           *dialog,
                                              PicmanImage           *image);
static void     file_export_dialog_destroyed (GtkWidget           *dialog,
                                              PicmanImage           *image);
static void     file_new_template_callback   (GtkWidget           *widget,
                                              const gchar         *name,
                                              gpointer             data);
static void     file_revert_confirm_response (GtkWidget           *dialog,
                                              gint                 response_id,
                                              PicmanDisplay         *display);



/*  public functions  */


void
file_open_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  Picman        *picman;
  GtkWidget   *widget;
  PicmanImage   *image;
  return_if_no_picman (picman, data);
  return_if_no_widget (widget, data);

  image = action_data_get_image (data);

  file_open_dialog_show (picman, widget,
                         _("Open Image"),
                         image, NULL, FALSE);
}

void
file_open_as_layers_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  Picman        *picman;
  GtkWidget   *widget;
  PicmanDisplay *display;
  PicmanImage   *image = NULL;
  return_if_no_picman (picman, data);
  return_if_no_widget (widget, data);

  display = action_data_get_display (data);

  if (display)
    image = picman_display_get_image (display);

  file_open_dialog_show (picman, widget,
                         _("Open Image as Layers"),
                         image, NULL, TRUE);
}

void
file_open_location_cmd_callback (GtkAction *action,
                                 gpointer   data)
{
  GtkWidget *widget;
  return_if_no_widget (widget, data);

  picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                  gtk_widget_get_screen (widget),
                                  NULL /*ui_manager*/,
                                  "picman-file-open-location-dialog", -1, TRUE);
}

void
file_open_recent_cmd_callback (GtkAction *action,
                               gint       value,
                               gpointer   data)
{
  Picman          *picman;
  PicmanImagefile *imagefile;
  gint           num_entries;
  return_if_no_picman (picman, data);

  num_entries = picman_container_get_n_children (picman->documents);

  if (value >= num_entries)
    return;

  imagefile = (PicmanImagefile *)
    picman_container_get_child_by_index (picman->documents, value);

  if (imagefile)
    {
      PicmanDisplay       *display;
      PicmanProgress      *progress;
      PicmanImage         *image;
      PicmanPDBStatusType  status;
      GError            *error = NULL;
      return_if_no_display (display, data);

      g_object_ref (display);
      g_object_ref (imagefile);

      progress = picman_display_get_image (display) ?
                 NULL : PICMAN_PROGRESS (display);

      image = file_open_with_display (picman, action_data_get_context (data),
                                      progress,
                                      picman_object_get_name (imagefile), FALSE,
                                      &status, &error);

      if (! image && status != PICMAN_PDB_CANCEL)
        {
          gchar *filename =
            file_utils_uri_display_name (picman_object_get_name (imagefile));

          picman_message (picman, G_OBJECT (display), PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed:\n\n%s"),
                        filename, error->message);
          g_clear_error (&error);

          g_free (filename);
        }

      g_object_unref (imagefile);
      g_object_unref (display);
    }
}

void
file_save_cmd_callback (GtkAction *action,
                        gint       value,
                        gpointer   data)
{
  Picman         *picman;
  PicmanDisplay  *display;
  PicmanImage    *image;
  GtkWidget    *widget;
  PicmanSaveMode  save_mode;
  const gchar  *uri;
  gboolean      saved = FALSE;
  return_if_no_picman (picman, data);
  return_if_no_display (display, data);
  return_if_no_widget (widget, data);

  image = picman_display_get_image (display);

  save_mode = (PicmanSaveMode) value;

  if (! picman_image_get_active_drawable (image))
    return;

  uri = picman_image_get_uri (image);

  switch (save_mode)
    {
    case PICMAN_SAVE_MODE_SAVE:
    case PICMAN_SAVE_MODE_SAVE_AND_CLOSE:
      /*  Only save if the image has been modified, or if it is new.  */
      if ((picman_image_is_dirty (image) ||
           ! PICMAN_GUI_CONFIG (image->picman->config)->trust_dirty_flag) ||
          uri == NULL)
        {
          PicmanPlugInProcedure *save_proc = picman_image_get_save_proc (image);

          if (uri && ! save_proc)
            {
              save_proc =
                file_procedure_find (image->picman->plug_in_manager->save_procs,
                                     uri, NULL);
            }

          if (uri && save_proc)
            {
              saved = file_save_dialog_save_image (PICMAN_PROGRESS (display),
                                                   picman, image, uri,
                                                   save_proc,
                                                   PICMAN_RUN_WITH_LAST_VALS,
                                                   TRUE, FALSE, FALSE, TRUE);
              break;
            }

          /* fall thru */
        }
      else
        {
          picman_message_literal (image->picman,
				G_OBJECT (display), PICMAN_MESSAGE_INFO,
				_("No changes need to be saved"));
          saved = TRUE;
          break;
        }

    case PICMAN_SAVE_MODE_SAVE_AS:
      file_save_dialog_show (picman, image, widget,
                             _("Save Image"), FALSE,
                             save_mode == PICMAN_SAVE_MODE_SAVE_AND_CLOSE, display);
      break;

    case PICMAN_SAVE_MODE_SAVE_A_COPY:
      file_save_dialog_show (picman, image, widget,
                             _("Save a Copy of the Image"), TRUE,
                             FALSE, display);
      break;

    case PICMAN_SAVE_MODE_EXPORT:
      file_export_dialog_show (picman, image, widget);
      break;

    case PICMAN_SAVE_MODE_EXPORT_TO:
    case PICMAN_SAVE_MODE_OVERWRITE:
      {
        const gchar         *uri = NULL;
        PicmanPlugInProcedure *export_proc;
        gboolean             overwrite;

        if (save_mode == PICMAN_SAVE_MODE_EXPORT_TO)
          {
            uri = picman_image_get_exported_uri (image);

            if (! uri)
              {
                /* Behave as if Export... was invoked */
                file_export_dialog_show (picman, image, widget);
                break;
              }

            overwrite = FALSE;
          }
        else if (save_mode == PICMAN_SAVE_MODE_OVERWRITE)
          {
            uri = picman_image_get_imported_uri (image);

            overwrite = TRUE;
          }

        if (uri)
          {
            export_proc =
              file_procedure_find (image->picman->plug_in_manager->export_procs,
                                   uri, NULL);
          }

        if (uri && export_proc)
          {
            char *uri_copy;

            /* The memory that 'uri' points to can be freed by
               file_save_dialog_save_image(), when it eventually calls
               picman_image_set_imported_uri() to reset the imported uri,
               resulting in garbage. So make a duplicate of it here. */

            uri_copy = g_strdup (uri);

            saved = file_save_dialog_save_image (PICMAN_PROGRESS (display),
                                                 picman, image, uri_copy,
                                                 export_proc,
                                                 PICMAN_RUN_WITH_LAST_VALS,
                                                 FALSE,
                                                 overwrite, ! overwrite,
                                                 TRUE);
            g_free (uri_copy);
          }
      }
      break;
    }

  if (save_mode == PICMAN_SAVE_MODE_SAVE_AND_CLOSE &&
      saved &&
      ! picman_image_is_dirty (image))
    {
      picman_display_close (display);
    }
}

void
file_create_template_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  dialog = picman_query_string_box (_("Create New Template"),
                                  GTK_WIDGET (picman_display_get_shell (display)),
                                  picman_standard_help_func,
                                  PICMAN_HELP_FILE_CREATE_TEMPLATE,
                                  _("Enter a name for this template"),
                                  NULL,
                                  G_OBJECT (image), "disconnect",
                                  file_new_template_callback, image);
  gtk_widget_show (dialog);
}

void
file_revert_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  PicmanDisplay *display;
  PicmanImage   *image;
  GtkWidget   *dialog;
  const gchar *uri;
  return_if_no_display (display, data);

  image = picman_display_get_image (display);

  uri = picman_image_get_uri (image);

  if (! uri)
    uri = picman_image_get_imported_uri (image);

  dialog = g_object_get_data (G_OBJECT (image), REVERT_DATA_KEY);

  if (! uri)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (display), PICMAN_MESSAGE_ERROR,
			    _("Revert failed. "
			      "No file name associated with this image."));
    }
  else if (dialog)
    {
      gtk_window_present (GTK_WINDOW (dialog));
    }
  else
    {
      gchar *filename;

      dialog =
        picman_message_dialog_new (_("Revert Image"), GTK_STOCK_REVERT_TO_SAVED,
                                 GTK_WIDGET (picman_display_get_shell (display)),
                                 0,
                                 picman_standard_help_func, PICMAN_HELP_FILE_REVERT,

                                 GTK_STOCK_CANCEL,          GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_REVERT_TO_SAVED, GTK_RESPONSE_OK,

                                 NULL);

      gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                               GTK_RESPONSE_OK,
                                               GTK_RESPONSE_CANCEL,
                                               -1);

      g_signal_connect_object (display, "disconnect",
                               G_CALLBACK (gtk_widget_destroy),
                               dialog, G_CONNECT_SWAPPED);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (file_revert_confirm_response),
                        display);

      filename = file_utils_uri_display_name (uri);

      picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                         _("Revert '%s' to '%s'?"),
                                         picman_image_get_display_name (image),
                                         filename);
      g_free (filename);

      picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                 _("By reverting the image to the state saved "
                                   "on disk, you will lose all changes, "
                                   "including all undo information."));

      g_object_set_data (G_OBJECT (image), REVERT_DATA_KEY, dialog);

      gtk_widget_show (dialog);
    }
}

void
file_close_all_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  Picman *picman;
  return_if_no_picman (picman, data);

  if (! picman_displays_dirty (picman))
    {
      picman_displays_close (picman);
    }
  else
    {
      GtkWidget *widget;
      return_if_no_widget (widget, data);

      picman_dialog_factory_dialog_raise (picman_dialog_factory_get_singleton (),
                                        gtk_widget_get_screen (widget),
                                        "picman-close-all-dialog", -1);
    }
}

void
file_quit_cmd_callback (GtkAction *action,
                        gpointer   data)
{
  Picman *picman;
  return_if_no_picman (picman, data);

  picman_exit (picman, FALSE);
}

void
file_file_open_dialog (Picman        *picman,
                       const gchar *uri,
                       GtkWidget   *parent)
{
  file_open_dialog_show (picman, parent,
                         _("Open Image"),
                         NULL, uri, FALSE);
}


/*  private functions  */

static void
file_open_dialog_show (Picman        *picman,
                       GtkWidget   *parent,
                       const gchar *title,
                       PicmanImage   *image,
                       const gchar *uri,
                       gboolean     open_as_layers)
{
  GtkWidget *dialog;

  dialog = picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                           gtk_widget_get_screen (parent),
                                           NULL /*ui_manager*/,
                                           "picman-file-open-dialog", -1, FALSE);

  if (dialog)
    {
      if (! uri && image)
        uri = picman_image_get_uri (image);

      if (! uri)
        uri = g_object_get_data (G_OBJECT (picman), PICMAN_FILE_OPEN_LAST_URI_KEY);

      if (uri)
        gtk_file_chooser_set_uri (GTK_FILE_CHOOSER (dialog), uri);
      else if (picman->default_folder)
        gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (dialog),
                                                 picman->default_folder);

      picman_file_dialog_set_open_image (PICMAN_FILE_DIALOG (dialog),
                                       image, open_as_layers);

      gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                    GTK_WINDOW (gtk_widget_get_toplevel (parent)));

      gtk_window_present (GTK_WINDOW (dialog));
    }
}

static void
file_save_dialog_show (Picman        *picman,
                       PicmanImage   *image,
                       GtkWidget   *parent,
                       const gchar *title,
                       gboolean     save_a_copy,
                       gboolean     close_after_saving,
                       PicmanDisplay *display)
{
  GtkWidget *dialog;

  dialog = g_object_get_data (G_OBJECT (image), "picman-file-save-dialog");

  if (! dialog)
    {
      dialog = picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                               gtk_widget_get_screen (parent),
                                               NULL /*ui_manager*/,
                                               "picman-file-save-dialog",
                                               -1, FALSE);

      if (dialog)
        {
          gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                        GTK_WINDOW (gtk_widget_get_toplevel (parent)));

          g_object_set_data_full (G_OBJECT (image),
                                  "picman-file-save-dialog", dialog,
                                  (GDestroyNotify) gtk_widget_destroy);
          g_signal_connect (dialog, "destroy",
                            G_CALLBACK (file_save_dialog_destroyed),
                            image);
        }
    }

  if (dialog)
    {
      gtk_window_set_title (GTK_WINDOW (dialog), title);

      picman_file_dialog_set_save_image (PICMAN_FILE_DIALOG (dialog),
                                       picman, image, save_a_copy, FALSE,
                                       close_after_saving, PICMAN_OBJECT (display));

      gtk_window_present (GTK_WINDOW (dialog));
    }
}

static void
file_save_dialog_destroyed (GtkWidget *dialog,
                            PicmanImage *image)
{
  if (PICMAN_FILE_DIALOG (dialog)->image == image)
    g_object_set_data (G_OBJECT (image), "picman-file-save-dialog", NULL);
}

static void
file_export_dialog_show (Picman      *picman,
                         PicmanImage *image,
                         GtkWidget *parent)
{
  GtkWidget *dialog;

  dialog = g_object_get_data (G_OBJECT (image), "picman-file-export-dialog");

  if (! dialog)
    {
      dialog = picman_dialog_factory_dialog_new (picman_dialog_factory_get_singleton (),
                                               gtk_widget_get_screen (parent),
                                               NULL /*ui_manager*/,
                                               "picman-file-export-dialog",
                                               -1, FALSE);

      if (dialog)
        {
          gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                        GTK_WINDOW (gtk_widget_get_toplevel (parent)));

          g_object_set_data_full (G_OBJECT (image),
                                  "picman-file-export-dialog", dialog,
                                  (GDestroyNotify) gtk_widget_destroy);
          g_signal_connect (dialog, "destroy",
                            G_CALLBACK (file_export_dialog_destroyed),
                            image);
        }
    }

  if (dialog)
    {
      picman_file_dialog_set_save_image (PICMAN_FILE_DIALOG (dialog),
                                       picman,
                                       image,
                                       FALSE,
                                       TRUE,
                                       FALSE,
                                       NULL);

      gtk_window_present (GTK_WINDOW (dialog));
    }
}

static void
file_export_dialog_destroyed (GtkWidget *dialog,
                              PicmanImage *image)
{
  if (PICMAN_FILE_DIALOG (dialog)->image == image)
    g_object_set_data (G_OBJECT (image), "picman-file-export-dialog", NULL);
}

static void
file_new_template_callback (GtkWidget   *widget,
                            const gchar *name,
                            gpointer     data)
{
  PicmanTemplate *template;
  PicmanImage    *image;

  image = (PicmanImage *) data;

  if (! (name && strlen (name)))
    name = _("(Unnamed Template)");

  template = picman_template_new (name);
  picman_template_set_from_image (template, image);
  picman_container_add (image->picman->templates, PICMAN_OBJECT (template));
  g_object_unref (template);
}

static void
file_revert_confirm_response (GtkWidget   *dialog,
                              gint         response_id,
                              PicmanDisplay *display)
{
  PicmanImage *old_image = picman_display_get_image (display);

  gtk_widget_destroy (dialog);

  g_object_set_data (G_OBJECT (old_image), REVERT_DATA_KEY, NULL);

  if (response_id == GTK_RESPONSE_OK)
    {
      Picman              *picman = old_image->picman;
      PicmanImage         *new_image;
      const gchar       *uri;
      PicmanPDBStatusType  status;
      GError            *error = NULL;

      uri = picman_image_get_uri (old_image);

      if (! uri)
        uri = picman_image_get_imported_uri (old_image);

      new_image = file_open_image (picman, picman_get_user_context (picman),
                                   PICMAN_PROGRESS (display),
                                   uri, uri, FALSE, NULL,
                                   PICMAN_RUN_INTERACTIVE,
                                   &status, NULL, &error);

      if (new_image)
        {
          picman_displays_reconnect (picman, old_image, new_image);
          picman_image_flush (new_image);

          /*  the displays own the image now  */
          g_object_unref (new_image);
        }
      else if (status != PICMAN_PDB_CANCEL)
        {
          gchar *filename = file_utils_uri_display_name (uri);

          picman_message (picman, G_OBJECT (display), PICMAN_MESSAGE_ERROR,
                        _("Reverting to '%s' failed:\n\n%s"),
                        filename, error->message);
          g_clear_error (&error);

          g_free (filename);
        }
    }
}
