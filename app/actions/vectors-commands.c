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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-merge.h"
#include "core/picmanimage-undo.h"
#include "core/picmanitemundo.h"
#include "core/picmanparamspecs.h"
#include "core/picmanprogress.h"
#include "core/picmanstrokeoptions.h"
#include "core/picmantoolinfo.h"

#include "pdb/picmanpdb.h"
#include "pdb/picmanprocedure.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanvectors-export.h"
#include "vectors/picmanvectors-import.h"

#include "widgets/picmanaction.h"
#include "widgets/picmanclipboard.h"
#include "widgets/picmanhelp-ids.h"

#include "display/picmandisplay.h"

#include "tools/picmanvectortool.h"
#include "tools/tool_manager.h"

#include "dialogs/stroke-dialog.h"
#include "dialogs/vectors-export-dialog.h"
#include "dialogs/vectors-import-dialog.h"
#include "dialogs/vectors-options-dialog.h"

#include "actions.h"
#include "vectors-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   vectors_new_vectors_response  (GtkWidget            *widget,
                                             gint                  response_id,
                                             VectorsOptionsDialog *options);
static void   vectors_edit_vectors_response (GtkWidget            *widget,
                                             gint                  response_id,
                                             VectorsOptionsDialog *options);
static void   vectors_import_response       (GtkWidget            *widget,
                                             gint                  response_id,
                                             VectorsImportDialog  *dialog);
static void   vectors_export_response       (GtkWidget            *widget,
                                             gint                  response_id,
                                             VectorsExportDialog  *dialog);


/*  private variables  */

static gchar    *vectors_name               = NULL;
static gboolean  vectors_import_merge       = FALSE;
static gboolean  vectors_import_scale       = FALSE;
static gboolean  vectors_export_active_only = TRUE;


/*  public functions  */

void
vectors_vectors_tool_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  PicmanTool    *active_tool;
  return_if_no_vectors (image, vectors, data);

  active_tool = tool_manager_get_active (image->picman);

  if (! PICMAN_IS_VECTOR_TOOL (active_tool))
    {
      PicmanToolInfo  *tool_info = picman_get_tool_info (image->picman,
                                                     "picman-vector-tool");

      if (PICMAN_IS_TOOL_INFO (tool_info))
        {
          picman_context_set_tool (action_data_get_context (data), tool_info);
          active_tool = tool_manager_get_active (image->picman);
        }
    }

  if (PICMAN_IS_VECTOR_TOOL (active_tool))
    picman_vector_tool_set_vectors (PICMAN_VECTOR_TOOL (active_tool), vectors);
}

void
vectors_edit_attributes_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  VectorsOptionsDialog *options;
  PicmanImage            *image;
  PicmanVectors          *vectors;
  GtkWidget            *widget;
  return_if_no_vectors (image, vectors, data);
  return_if_no_widget (widget, data);

  options = vectors_options_dialog_new (image, vectors,
                                        action_data_get_context (data),
                                        widget,
                                        picman_object_get_name (vectors),
                                        _("Path Attributes"),
                                        "picman-vectors-edit",
                                        GTK_STOCK_EDIT,
                                        _("Edit Path Attributes"),
                                        PICMAN_HELP_PATH_EDIT);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (vectors_edit_vectors_response),
                    options);

  gtk_widget_show (options->dialog);
}

void
vectors_new_cmd_callback (GtkAction *action,
                          gpointer   data)
{
  VectorsOptionsDialog *options;
  PicmanImage            *image;
  GtkWidget            *widget;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  options = vectors_options_dialog_new (image, NULL,
                                        action_data_get_context (data),
                                        widget,
                                        vectors_name ? vectors_name :
                                        _("Path"),
                                        _("New Path"),
                                        "picman-vectors-new",
                                        PICMAN_STOCK_PATH,
                                        _("New Path Options"),
                                        PICMAN_HELP_PATH_NEW);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (vectors_new_vectors_response),
                    options);

  gtk_widget_show (options->dialog);
}

void
vectors_new_last_vals_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *new_vectors;
  return_if_no_image (image, data);

  new_vectors = picman_vectors_new (image, vectors_name);

  picman_image_add_vectors (image, new_vectors,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_flush (image);
}

void
vectors_raise_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_image_raise_item (image, PICMAN_ITEM (vectors), NULL);
  picman_image_flush (image);
}

void
vectors_raise_to_top_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_image_raise_item_to_top (image, PICMAN_ITEM (vectors));
  picman_image_flush (image);
}

void
vectors_lower_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_image_lower_item (image, PICMAN_ITEM (vectors), NULL);
  picman_image_flush (image);
}

void
vectors_lower_to_bottom_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_image_lower_item_to_bottom (image, PICMAN_ITEM (vectors));
  picman_image_flush (image);
}

void
vectors_duplicate_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  PicmanVectors *new_vectors;
  return_if_no_vectors (image, vectors, data);

  new_vectors = PICMAN_VECTORS (picman_item_duplicate (PICMAN_ITEM (vectors),
                                                   G_TYPE_FROM_INSTANCE (vectors)));

  /*  use the actual parent here, not PICMAN_IMAGE_ACTIVE_PARENT because
   *  the latter would add a duplicated group inside itself instead of
   *  above it
   */
  picman_image_add_vectors (image, new_vectors,
                          picman_vectors_get_parent (vectors), -1,
                          TRUE);

  picman_image_flush (image);
}

void
vectors_delete_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_image_remove_vectors (image, vectors, TRUE, NULL);
  picman_image_flush (image);
}

void
vectors_merge_visible_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  GtkWidget   *widget;
  GError      *error = NULL;
  return_if_no_vectors (image, vectors, data);
  return_if_no_widget (widget, data);

  if (! picman_image_merge_visible_vectors (image, &error))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    error->message);
      g_clear_error (&error);
      return;
    }

  picman_image_flush (image);
}

void
vectors_to_selection_cmd_callback (GtkAction *action,
                                   gint       value,
                                   gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  return_if_no_vectors (image, vectors, data);

  picman_item_to_selection (PICMAN_ITEM (vectors),
                          (PicmanChannelOps) value,
                          TRUE, FALSE, 0, 0);
  picman_image_flush (image);
}

void
vectors_selection_to_vectors_cmd_callback (GtkAction *action,
                                           gint       value,
                                           gpointer   data)
{
  PicmanImage      *image;
  GtkWidget      *widget;
  PicmanProcedure  *procedure;
  PicmanValueArray *args;
  PicmanDisplay    *display;
  GError         *error = NULL;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  if (value)
    procedure = picman_pdb_lookup_procedure (image->picman->pdb,
                                           "plug-in-sel2path-advanced");
  else
    procedure = picman_pdb_lookup_procedure (image->picman->pdb,
                                           "plug-in-sel2path");

  if (! procedure)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_ERROR,
			    "Selection to path procedure lookup failed.");
      return;
    }

  display = picman_context_get_display (action_data_get_context (data));

  args = picman_procedure_get_arguments (procedure);
  picman_value_array_truncate (args, 2);

  g_value_set_int      (picman_value_array_index (args, 0),
                        PICMAN_RUN_INTERACTIVE);
  picman_value_set_image (picman_value_array_index (args, 1),
                        image);

  picman_procedure_execute_async (procedure, image->picman,
                                action_data_get_context (data),
                                PICMAN_PROGRESS (display), args,
                                PICMAN_OBJECT (display), &error);

  picman_value_array_unref (args);

  if (error)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_ERROR,
			    error->message);
      g_error_free (error);
    }
}

void
vectors_stroke_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage    *image;
  PicmanVectors  *vectors;
  PicmanDrawable *drawable;
  GtkWidget    *widget;
  GtkWidget    *dialog;
  return_if_no_vectors (image, vectors, data);
  return_if_no_widget (widget, data);

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to stroke to."));
      return;
    }

  dialog = stroke_dialog_new (PICMAN_ITEM (vectors),
                              action_data_get_context (data),
                              _("Stroke Path"),
                              PICMAN_STOCK_PATH_STROKE,
                              PICMAN_HELP_PATH_STROKE,
                              widget);
  gtk_widget_show (dialog);
}

void
vectors_stroke_last_vals_cmd_callback (GtkAction *action,
                                       gpointer   data)
{
  PicmanImage         *image;
  PicmanVectors       *vectors;
  PicmanDrawable      *drawable;
  PicmanContext       *context;
  GtkWidget         *widget;
  PicmanStrokeOptions *options;
  GError            *error = NULL;
  return_if_no_vectors (image, vectors, data);
  return_if_no_context (context, data);
  return_if_no_widget (widget, data);

  drawable = picman_image_get_active_drawable (image);

  if (! drawable)
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("There is no active layer or channel to stroke to."));
      return;
    }


  options = g_object_get_data (G_OBJECT (image->picman), "saved-stroke-options");

  if (options)
    g_object_ref (options);
  else
    options = picman_stroke_options_new (image->picman, context, TRUE);

  if (! picman_item_stroke (PICMAN_ITEM (vectors), drawable, context, options, FALSE,
                          TRUE, NULL, &error))
    {
      picman_message_literal (image->picman, G_OBJECT (widget),
			    PICMAN_MESSAGE_WARNING, error->message);
      g_clear_error (&error);
    }
  else
    {
      picman_image_flush (image);
    }

  g_object_unref (options);
}

void
vectors_copy_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  gchar       *svg;
  return_if_no_vectors (image, vectors, data);

  svg = picman_vectors_export_string (image, vectors);

  if (svg)
    {
      picman_clipboard_set_svg (image->picman, svg);
      g_free (svg);
    }
}

void
vectors_paste_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanImage *image;
  GtkWidget *widget;
  gchar     *svg;
  gsize      svg_size;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  svg = picman_clipboard_get_svg (image->picman, &svg_size);

  if (svg)
    {
      GError *error = NULL;

      if (! picman_vectors_import_buffer (image, svg, svg_size,
                                        TRUE, FALSE,
                                        PICMAN_IMAGE_ACTIVE_PARENT, -1,
                                        NULL, &error))
        {
          picman_message (image->picman, G_OBJECT (widget), PICMAN_MESSAGE_ERROR,
                        "%s", error->message);
          g_clear_error (&error);
        }
      else
        {
          picman_image_flush (image);
        }

      g_free (svg);
    }
}

void
vectors_export_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  VectorsExportDialog *dialog;
  PicmanImage           *image;
  PicmanVectors         *vectors;
  GtkWidget           *widget;
  const gchar         *folder;
  return_if_no_vectors (image, vectors, data);
  return_if_no_widget (widget, data);

  dialog = vectors_export_dialog_new (image, widget,
                                      vectors_export_active_only);

  folder = g_object_get_data (G_OBJECT (image->picman),
                              "picman-vectors-export-folder");
  if (folder)
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog->dialog),
                                         folder);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (vectors_export_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
vectors_import_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  VectorsImportDialog *dialog;
  PicmanImage           *image;
  GtkWidget           *widget;
  const gchar         *folder;
  return_if_no_image (image, data);
  return_if_no_widget (widget, data);

  dialog = vectors_import_dialog_new (image, widget,
                                      vectors_import_merge,
                                      vectors_import_scale);

  folder = g_object_get_data (G_OBJECT (image->picman),
                              "picman-vectors-import-folder");
  if (folder)
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog->dialog),
                                         folder);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (vectors_import_response),
                    dialog);

  gtk_widget_show (dialog->dialog);
}

void
vectors_visible_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  gboolean     visible;
  return_if_no_vectors (image, vectors, data);

  visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (visible != picman_item_get_visible (PICMAN_ITEM (vectors)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_VISIBILITY);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (vectors))
        push_undo = FALSE;

      picman_item_set_visible (PICMAN_ITEM (vectors), visible, push_undo);
      picman_image_flush (image);
    }
}

void
vectors_linked_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  gboolean     linked;
  return_if_no_vectors (image, vectors, data);

  linked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (linked != picman_item_get_linked (PICMAN_ITEM (vectors)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_LINKED);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (vectors))
        push_undo = FALSE;

      picman_item_set_linked (PICMAN_ITEM (vectors), linked, push_undo);
      picman_image_flush (image);
    }
}

void
vectors_lock_content_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  gboolean     locked;
  return_if_no_vectors (image, vectors, data);

  locked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (locked != picman_item_get_lock_content (PICMAN_ITEM (vectors)))
    {
#if 0
      PicmanUndo *undo;
#endif
      gboolean  push_undo = TRUE;

#if 0
      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_LINKED);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (vectors))
        push_undo = FALSE;
#endif

      picman_item_set_lock_content (PICMAN_ITEM (vectors), locked, push_undo);
      picman_image_flush (image);
    }
}

void
vectors_lock_position_cmd_callback (GtkAction *action,
                                   gpointer   data)
{
  PicmanImage   *image;
  PicmanVectors *vectors;
  gboolean     locked;
  return_if_no_vectors (image, vectors, data);

  locked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (locked != picman_item_get_lock_position (PICMAN_ITEM (vectors)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_LOCK_POSITION);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (vectors))
        push_undo = FALSE;


      picman_item_set_lock_position (PICMAN_ITEM (vectors), locked, push_undo);
      picman_image_flush (image);
    }
}

/*  private functions  */

static void
vectors_new_vectors_response (GtkWidget            *widget,
                              gint                  response_id,
                              VectorsOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanVectors *new_vectors;

      if (vectors_name)
        g_free (vectors_name);

      vectors_name =
        g_strdup (gtk_entry_get_text (GTK_ENTRY (options->name_entry)));

      new_vectors = picman_vectors_new (options->image, vectors_name);

      picman_image_add_vectors (options->image, new_vectors,
                              PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

      picman_image_flush (options->image);
    }

  gtk_widget_destroy (options->dialog);
}

static void
vectors_edit_vectors_response (GtkWidget            *widget,
                               gint                  response_id,
                               VectorsOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanVectors *vectors = options->vectors;
      const gchar *new_name;

      new_name = gtk_entry_get_text (GTK_ENTRY (options->name_entry));

      if (strcmp (new_name, picman_object_get_name (vectors)))
        {
          picman_item_rename (PICMAN_ITEM (vectors), new_name, NULL);
          picman_image_flush (options->image);
        }
    }

  gtk_widget_destroy (options->dialog);
}

static void
vectors_import_response (GtkWidget           *widget,
                         gint                 response_id,
                         VectorsImportDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (widget);
      gchar          *filename;
      GError         *error = NULL;

      vectors_import_merge = dialog->merge_vectors;
      vectors_import_scale = dialog->scale_vectors;

      filename = gtk_file_chooser_get_filename (chooser);

      if (picman_vectors_import_file (dialog->image, filename,
                                    vectors_import_merge, vectors_import_scale,
                                    PICMAN_IMAGE_ACTIVE_PARENT, -1,
                                    NULL, &error))
        {
          picman_image_flush (dialog->image);
        }
      else
        {
          picman_message (dialog->image->picman, G_OBJECT (widget),
                        PICMAN_MESSAGE_ERROR,
                        "%s", error->message);
          g_error_free (error);
          return;
        }

      g_free (filename);

      g_object_set_data_full (G_OBJECT (dialog->image->picman),
                              "picman-vectors-import-folder",
                              gtk_file_chooser_get_current_folder (chooser),
                              (GDestroyNotify) g_free);
    }

  gtk_widget_destroy (widget);
}

static void
vectors_export_response (GtkWidget           *widget,
                         gint                 response_id,
                         VectorsExportDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (widget);
      PicmanVectors    *vectors = NULL;
      gchar          *filename;
      GError         *error   = NULL;

      vectors_export_active_only = dialog->active_only;

      filename = gtk_file_chooser_get_filename (chooser);

      if (vectors_export_active_only)
        vectors = picman_image_get_active_vectors (dialog->image);

      if (! picman_vectors_export_file (dialog->image, vectors, filename, &error))
        {
          picman_message (dialog->image->picman, G_OBJECT (widget),
                        PICMAN_MESSAGE_ERROR,
                        "%s", error->message);
          g_error_free (error);
          return;
        }

      g_free (filename);

      g_object_set_data_full (G_OBJECT (dialog->image->picman),
                              "picman-vectors-export-folder",
                              gtk_file_chooser_get_current_folder (chooser),
                              (GDestroyNotify) g_free);
    }

  gtk_widget_destroy (widget);
}
