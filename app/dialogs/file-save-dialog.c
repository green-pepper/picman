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
#include "core/picmanprogress.h"

#include "plug-in/picmanpluginmanager.h"
#include "plug-in/picmanpluginprocedure.h"

#include "file/file-procedure.h"
#include "file/file-save.h"
#include "file/file-utils.h"
#include "file/picman-file.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmanfiledialog.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "display/picmandisplay.h"
#include "display/picmandisplayshell.h"

#include "file-save-dialog.h"

#include "picman-log.h"
#include "picman-intl.h"


/*  local function prototypes  */

static GtkFileChooserConfirmation
                 file_save_dialog_confirm_overwrite         (GtkWidget            *save_dialog,
                                                             Picman                 *picman);
static void      file_save_dialog_response                  (GtkWidget            *save_dialog,
                                                             gint                  response_id,
                                                             Picman                 *picman);
static gboolean  file_save_dialog_check_uri                 (GtkWidget            *save_dialog,
                                                             Picman                 *picman,
                                                             gchar               **ret_uri,
                                                             gchar               **ret_basename,
                                                             PicmanPlugInProcedure **ret_save_proc);
static gboolean  file_save_dialog_no_overwrite_confirmation (PicmanFileDialog       *dialog,
                                                             Picman                 *picman);
static gchar *   file_save_dialog_get_uri                   (PicmanFileDialog       *dialog);
static GSList *  file_save_dialog_get_procs                 (PicmanFileDialog       *dialog,
                                                             Picman                 *picman);
static void      file_save_dialog_unknown_ext_msg           (PicmanFileDialog       *dialog,
                                                             Picman                 *picman,
                                                             const gchar          *basename);
static gboolean  file_save_dialog_use_extension             (GtkWidget            *save_dialog,
                                                             const gchar          *uri);


/*  public functions  */

GtkWidget *
file_save_dialog_new (Picman     *picman,
                      gboolean  export)
{
  GtkWidget           *dialog;
  PicmanFileDialogState *state;

  if (! export)
    {
      dialog = picman_file_dialog_new (picman,
                                     PICMAN_FILE_CHOOSER_ACTION_SAVE,
                                     _("Save Image"), "picman-file-save",
                                     GTK_STOCK_SAVE,
                                     PICMAN_HELP_FILE_SAVE);

      state = g_object_get_data (G_OBJECT (picman), "picman-file-save-dialog-state");
    }
  else
    {
      dialog = picman_file_dialog_new (picman,
                                     PICMAN_FILE_CHOOSER_ACTION_EXPORT,
                                     _("Export Image"), "picman-file-export",
                                     _("_Export"),
                                     PICMAN_HELP_FILE_EXPORT);

      state = g_object_get_data (G_OBJECT (picman), "picman-file-export-dialog-state");
    }

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  if (state)
    picman_file_dialog_set_state (PICMAN_FILE_DIALOG (dialog), state);

  g_signal_connect (dialog, "confirm-overwrite",
                    G_CALLBACK (file_save_dialog_confirm_overwrite),
                    picman);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (file_save_dialog_response),
                    picman);

  return dialog;
}


/*  private functions  */

static GtkFileChooserConfirmation
file_save_dialog_confirm_overwrite (GtkWidget *save_dialog,
                                    Picman      *picman)
{
  PicmanFileDialog *dialog = PICMAN_FILE_DIALOG (save_dialog);

  if (file_save_dialog_no_overwrite_confirmation (dialog, picman))
    /* The URI will not be accepted whatever happens, so don't
     * bother asking the user about overwriting files
     */
    return GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
  else
    return GTK_FILE_CHOOSER_CONFIRMATION_CONFIRM;
}

static void
file_save_dialog_response (GtkWidget *save_dialog,
                           gint       response_id,
                           Picman      *picman)
{
  PicmanFileDialog      *dialog = PICMAN_FILE_DIALOG (save_dialog);
  gchar               *uri;
  gchar               *basename;
  PicmanPlugInProcedure *save_proc;
  gulong               handler_id;

  if (! dialog->export)
    {
      g_object_set_data_full (G_OBJECT (picman), "picman-file-save-dialog-state",
                              picman_file_dialog_get_state (dialog),
                              (GDestroyNotify) picman_file_dialog_state_destroy);
    }
  else
    {
      g_object_set_data_full (G_OBJECT (picman), "picman-file-export-dialog-state",
                              picman_file_dialog_get_state (dialog),
                              (GDestroyNotify) picman_file_dialog_state_destroy);
    }

  if (response_id != GTK_RESPONSE_OK)
    {
      if (! dialog->busy)
        gtk_widget_destroy (save_dialog);

      return;
    }

  handler_id = g_signal_connect (dialog, "destroy",
                                 G_CALLBACK (gtk_widget_destroyed),
                                 &dialog);

  if (file_save_dialog_check_uri (save_dialog, picman,
                                  &uri, &basename, &save_proc))
    {
      picman_file_dialog_set_sensitive (dialog, FALSE);

      if (file_save_dialog_save_image (PICMAN_PROGRESS (save_dialog),
                                       picman,
                                       dialog->image,
                                       uri,
                                       save_proc,
                                       PICMAN_RUN_INTERACTIVE,
                                       ! dialog->save_a_copy && ! dialog->export,
                                       FALSE,
                                       dialog->export,
                                       FALSE))
        {
          /* Save was successful, now store the URI in a couple of
           * places that depend on it being the user that made a
           * save. Lower-level URI management is handled in
           * file_save()
           */
          if (dialog->save_a_copy)
            picman_image_set_save_a_copy_uri (dialog->image, uri);

          if (! dialog->export)
            g_object_set_data_full (G_OBJECT (dialog->image->picman),
                                    PICMAN_FILE_SAVE_LAST_URI_KEY,
                                    g_strdup (uri), (GDestroyNotify) g_free);
          else
            g_object_set_data_full (G_OBJECT (dialog->image->picman),
                                    PICMAN_FILE_EXPORT_LAST_URI_KEY,
                                    g_strdup (uri), (GDestroyNotify) g_free);

          /*  make sure the menus are updated with the keys we've just set  */
          picman_image_flush (dialog->image);

          /* Handle close-after-saving */
          if (dialog->close_after_saving && dialog->display_to_close)
            {
              PicmanDisplay *display = PICMAN_DISPLAY (dialog->display_to_close);
              if (display && ! picman_image_is_dirty (picman_display_get_image (display)))
                picman_display_close (display);
            }

          gtk_widget_destroy (save_dialog);
        }

      g_free (uri);
      g_free (basename);

      if (dialog)
        picman_file_dialog_set_sensitive (dialog, TRUE);
    }

  if (dialog)
    g_signal_handler_disconnect (dialog, handler_id);
}

/*
 * IMPORTANT: When changing this function, keep
 * file_save_dialog_no_overwrite_confirmation() up to date. It is difficult to
 * move logic to a common place due to how the dialog is implemented
 * in GTK+ in combination with how we use it.
 */
static gboolean
file_save_dialog_check_uri (GtkWidget            *save_dialog,
                            Picman                 *picman,
                            gchar               **ret_uri,
                            gchar               **ret_basename,
                            PicmanPlugInProcedure **ret_save_proc)
{
  PicmanFileDialog      *dialog = PICMAN_FILE_DIALOG (save_dialog);
  gchar               *uri;
  gchar               *basename;
  PicmanPlugInProcedure *save_proc;
  PicmanPlugInProcedure *uri_proc;
  PicmanPlugInProcedure *basename_proc;

  uri = file_save_dialog_get_uri (dialog);

  if (! uri)
    return FALSE;

  basename      = file_utils_uri_display_basename (uri);

  save_proc     = dialog->file_proc;
  uri_proc      = file_procedure_find (file_save_dialog_get_procs (dialog, picman),
                                       uri, NULL);
  basename_proc = file_procedure_find (file_save_dialog_get_procs (dialog, picman),
                                       basename, NULL);

  PICMAN_LOG (SAVE_DIALOG, "URI = %s", uri);
  PICMAN_LOG (SAVE_DIALOG, "basename = %s", basename);
  PICMAN_LOG (SAVE_DIALOG, "selected save_proc: %s",
            save_proc && save_proc->menu_label ?
            save_proc->menu_label : "NULL");
  PICMAN_LOG (SAVE_DIALOG, "URI save_proc: %s",
            uri_proc ? uri_proc->menu_label : "NULL");
  PICMAN_LOG (SAVE_DIALOG, "basename save_proc: %s",
            basename_proc && basename_proc->menu_label ?
            basename_proc->menu_label : "NULL");

  /*  first check if the user entered an extension at all  */
  if (! basename_proc)
    {
      PICMAN_LOG (SAVE_DIALOG, "basename has no valid extension");

      if (! strchr (basename, '.'))
        {
          const gchar *ext = NULL;

          PICMAN_LOG (SAVE_DIALOG, "basename has no '.', trying to add extension");

          if (! save_proc && ! dialog->export)
            {
              ext = "xcf";
            }
          else if (save_proc && save_proc->extensions_list)
            {
              ext = save_proc->extensions_list->data;
            }

          if (ext)
            {
              gchar *ext_basename;
              gchar *utf8;

              PICMAN_LOG (SAVE_DIALOG, "appending .%s to basename", ext);

              ext_basename = g_strconcat (basename, ".", ext, NULL);

              g_free (uri);
              g_free (basename);

              basename = ext_basename;

              utf8 = g_filename_to_utf8 (basename, -1, NULL, NULL, NULL);
              gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_dialog),
                                                 utf8);
              g_free (utf8);

              PICMAN_LOG (SAVE_DIALOG,
                        "set basename to %s, rerunning response and bailing out",
                        basename);

              /*  call the response callback again, so the
               *  overwrite-confirm logic can check the changed uri
               */
              gtk_dialog_response (GTK_DIALOG (save_dialog), GTK_RESPONSE_OK);

              g_free (basename);

              return FALSE;
            }
          else
            {
              PICMAN_LOG (SAVE_DIALOG,
                        "save_proc has no extensions, continuing without");

              /*  there may be file formats with no extension at all, use
               *  the selected proc in this case.
               */
              basename_proc = save_proc;

              if (! uri_proc)
                uri_proc = basename_proc;
            }

          if (! basename_proc)
            {
              PICMAN_LOG (SAVE_DIALOG,
                        "unable to figure save_proc, bailing out");

              file_save_dialog_unknown_ext_msg (dialog, picman, basename);

              g_free (uri);
              g_free (basename);
              return FALSE;
            }
        }
      else if (save_proc && ! save_proc->extensions_list)
        {
          PICMAN_LOG (SAVE_DIALOG,
                    "basename has '.', but save_proc has no extensions, "
                    "accepting random extension");

          /*  accept any random extension if the file format has
           *  no extensions at all
           */
          basename_proc = save_proc;

          if (! uri_proc)
            uri_proc = basename_proc;
        }
    }

  /*  then check if the selected format matches the entered extension  */
  if (! save_proc)
    {
      PICMAN_LOG (SAVE_DIALOG, "no save_proc was selected from the list");

      if (! basename_proc)
        {
          PICMAN_LOG (SAVE_DIALOG,
                    "basename has no useful extension, bailing out");

          file_save_dialog_unknown_ext_msg (dialog, picman, basename);

          g_free (uri);
          g_free (basename);
          return FALSE;
        }

      PICMAN_LOG (SAVE_DIALOG, "use URI's proc '%s' so indirect saving works",
                uri_proc->menu_label ? uri_proc->menu_label : "<unnamed>");

      /*  use the URI's proc if no save proc was selected  */
      save_proc = uri_proc;
    }
  else
    {
      PICMAN_LOG (SAVE_DIALOG, "save_proc '%s' was selected from the list",
                save_proc->menu_label ? save_proc->menu_label : "<unnamed>");

      if (save_proc != basename_proc)
        {
          PICMAN_LOG (SAVE_DIALOG, "however the basename's proc is '%s'",
                    basename_proc ? basename_proc->menu_label : "NULL");

          if (uri_proc != basename_proc)
            {
              PICMAN_LOG (SAVE_DIALOG,
                        "that's impossible for remote URIs, bailing out");

              /*  remote URI  */

              picman_message (picman, G_OBJECT (save_dialog), PICMAN_MESSAGE_WARNING,
                            _("Saving remote files needs to determine the "
                              "file format from the file extension. "
                              "Please enter a file extension that matches "
                              "the selected file format or enter no file "
                              "extension at all."));
              g_free (uri);
              g_free (basename);
              return FALSE;
            }
          else
            {
              PICMAN_LOG (SAVE_DIALOG,
                        "ask the user if she really wants that filename");

              /*  local URI  */

              if (! file_save_dialog_use_extension (save_dialog, uri))
                {
                  g_free (uri);
                  g_free (basename);
                  return FALSE;
                }
            }
        }
      else if (save_proc != uri_proc)
        {
          PICMAN_LOG (SAVE_DIALOG,
                    "use URI's proc '%s' so indirect saving works",
                    uri_proc->menu_label ? uri_proc->menu_label : "<unnamed>");

          /*  need to use the URI's proc for saving because e.g.
           *  the GIF plug-in can't save a GIF to sftp://
           */
          save_proc = uri_proc;
        }
    }

  if (! save_proc)
    {
      g_warning ("%s: EEEEEEK", G_STRFUNC);
      return FALSE;
    }

  *ret_uri       = uri;
  *ret_basename  = basename;
  *ret_save_proc = save_proc;

  return TRUE;
}

/*
 * IMPORTANT: Keep this up to date with file_save_dialog_check_uri().
 */
static gboolean
file_save_dialog_no_overwrite_confirmation (PicmanFileDialog *dialog,
                                            Picman           *picman)
{
  gboolean             uri_will_change = FALSE;
  gboolean             unknown_ext     = FALSE;
  gchar               *uri             = NULL;
  gchar               *basename        = NULL;
  PicmanPlugInProcedure *basename_proc   = NULL;
  PicmanPlugInProcedure *save_proc       = NULL;

  uri = file_save_dialog_get_uri (dialog);

  if (! uri)
    return FALSE;

  basename      = file_utils_uri_display_basename (uri);
  save_proc     = dialog->file_proc;
  basename_proc = file_procedure_find (file_save_dialog_get_procs (dialog, picman),
                                       basename, NULL);

  uri_will_change = (! basename_proc &&
                     ! strchr (basename, '.') &&
                     (! save_proc || save_proc->extensions_list));

  unknown_ext     = (! save_proc &&
                     ! basename_proc);

  g_free (basename);
  g_free (uri);

  return uri_will_change || unknown_ext;
}

static gchar *
file_save_dialog_get_uri (PicmanFileDialog *dialog)
{
  gchar *uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));

  if (uri && ! strlen (uri))
    {
      g_free (uri);
      uri = NULL;
    }

  return uri;
}

static GSList *
file_save_dialog_get_procs (PicmanFileDialog *dialog,
                            Picman           *picman)
{
  return (! dialog->export ?
          picman->plug_in_manager->save_procs :
          picman->plug_in_manager->export_procs);
}

static void
file_save_dialog_unknown_ext_msg (PicmanFileDialog *dialog,
                                  Picman           *picman,
                                  const gchar    *basename)
{
  PicmanPlugInProcedure *proc_in_other_group;

  proc_in_other_group =
    file_procedure_find ((dialog->export ?
                          picman->plug_in_manager->save_procs :
                          picman->plug_in_manager->export_procs),
                         basename,
                         NULL);

  if (dialog->export && proc_in_other_group)
    {
      picman_message (picman, G_OBJECT (dialog), PICMAN_MESSAGE_WARNING,
                    _("You can use this dialog to export to various file formats. "
                      "If you want to save the image to the PICMAN XCF format, use "
                      "File→Save instead."));
    }
  else if (! dialog->export && proc_in_other_group)
    {
      picman_message (picman, G_OBJECT (dialog), PICMAN_MESSAGE_WARNING,
                    _("You can use this dialog to save to the PICMAN XCF "
                      "format. Use File→Export to export to other file formats."));
    }
  else
    {
      picman_message (picman, G_OBJECT (dialog), PICMAN_MESSAGE_WARNING,
                    _("The given filename does not have any known "
                      "file extension. Please enter a known file "
                      "extension or select a file format from the "
                      "file format list."));
    }
}

static gboolean
file_save_dialog_use_extension (GtkWidget   *save_dialog,
                                const gchar *uri)
{
  GtkWidget *dialog;
  gboolean   use_name = FALSE;

  dialog = picman_message_dialog_new (_("Extension Mismatch"),
                                    PICMAN_STOCK_QUESTION,
                                    save_dialog, GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_SAVE,   GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("The given file extension does "
                                       "not match the chosen file type."));

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("Do you want to save the image using this "
                               "name anyway?"));

  gtk_dialog_set_response_sensitive (GTK_DIALOG (save_dialog),
                                     GTK_RESPONSE_CANCEL, FALSE);

  g_object_ref (dialog);

  use_name = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);
  g_object_unref (dialog);

  gtk_dialog_set_response_sensitive (GTK_DIALOG (save_dialog),
                                     GTK_RESPONSE_CANCEL, TRUE);

  return use_name;
}

gboolean
file_save_dialog_save_image (PicmanProgress        *progress,
                             Picman                *picman,
                             PicmanImage           *image,
                             const gchar         *uri,
                             PicmanPlugInProcedure *save_proc,
                             PicmanRunMode          run_mode,
                             gboolean             change_saved_state,
                             gboolean             export_backward,
                             gboolean             export_forward,
                             gboolean             verbose_cancel)
{
  PicmanPDBStatusType  status;
  GError            *error   = NULL;
  GList             *list;
  gboolean           success = FALSE;

  for (list = picman_action_groups_from_name ("file");
       list;
       list = g_list_next (list))
    {
      picman_action_group_set_action_sensitive (list->data, "file-quit", FALSE);
    }

  status = file_save (picman, image, progress, uri,
                      save_proc, run_mode,
                      change_saved_state, export_backward, export_forward,
                      &error);

  switch (status)
    {
    case PICMAN_PDB_SUCCESS:
      success = TRUE;
      break;

    case PICMAN_PDB_CANCEL:
      if (verbose_cancel)
        picman_message_literal (picman,
                              G_OBJECT (progress), PICMAN_MESSAGE_INFO,
                              _("Saving canceled"));
      break;

    default:
      {
        gchar *filename = file_utils_uri_display_name (uri);

        picman_message (picman, G_OBJECT (progress), PICMAN_MESSAGE_ERROR,
                      _("Saving '%s' failed:\n\n%s"), filename, error->message);
        g_clear_error (&error);
        g_free (filename);
      }
      break;
    }

  for (list = picman_action_groups_from_name ("file");
       list;
       list = g_list_next (list))
    {
      picman_action_group_set_action_sensitive (list->data, "file-quit", TRUE);
    }

  return success;
}
