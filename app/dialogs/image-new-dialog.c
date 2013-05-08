/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-new.h"
#include "core/picmantemplate.h"

#include "widgets/picmancontainercombobox.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmantemplateeditor.h"

#include "image-new-dialog.h"

#include "picman-intl.h"


#define RESPONSE_RESET 1

typedef struct
{
  GtkWidget    *dialog;
  GtkWidget    *confirm_dialog;

  GtkWidget    *combo;
  GtkWidget    *editor;

  PicmanContext  *context;
  PicmanTemplate *template;
} ImageNewDialog;


/*  local function prototypes  */

static void   image_new_dialog_free      (ImageNewDialog *dialog);
static void   image_new_dialog_response  (GtkWidget      *widget,
                                          gint            response_id,
                                          ImageNewDialog *dialog);
static void   image_new_template_changed (PicmanContext    *context,
                                          PicmanTemplate   *template,
                                          ImageNewDialog *dialog);
static void   image_new_confirm_dialog   (ImageNewDialog *dialog);
static void   image_new_create_image     (ImageNewDialog *dialog);


/*  public functions  */

GtkWidget *
image_new_dialog_new (PicmanContext *context)
{
  ImageNewDialog *dialog;
  GtkWidget      *main_vbox;
  GtkWidget      *hbox;
  GtkWidget      *label;
  PicmanSizeEntry  *entry;

  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  dialog = g_slice_new0 (ImageNewDialog);

  dialog->context  = picman_context_new (context->picman, "image-new-dialog",
                                       context);
  dialog->template = g_object_new (PICMAN_TYPE_TEMPLATE, NULL);

  dialog->dialog = picman_dialog_new (_("Create a New Image"),
                                    "picman-image-new",
                                    NULL, 0,
                                    picman_standard_help_func, PICMAN_HELP_FILE_NEW,

                                    PICMAN_STOCK_RESET, RESPONSE_RESET,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                    NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

  g_object_set_data_full (G_OBJECT (dialog->dialog),
                          "picman-image-new-dialog", dialog,
                          (GDestroyNotify) image_new_dialog_free);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (image_new_dialog_response),
                    dialog);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog->dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  /*  The template combo  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Template:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  dialog->combo = g_object_new (PICMAN_TYPE_CONTAINER_COMBO_BOX,
                                "container",         context->picman->templates,
                                "context",           dialog->context,
                                "view-size",         16,
                                "view-border-width", 0,
                                "ellipsize",         PANGO_ELLIPSIZE_NONE,
                                "focus-on-click",    FALSE,
                                NULL);
  gtk_box_pack_start (GTK_BOX (hbox), dialog->combo, TRUE, TRUE, 0);
  gtk_widget_show (dialog->combo);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->combo);

  g_signal_connect (dialog->context, "template-changed",
                    G_CALLBACK (image_new_template_changed),
                    dialog);

  /*  Template editor  */
  dialog->editor = picman_template_editor_new (dialog->template, context->picman,
                                             FALSE);
  gtk_box_pack_start (GTK_BOX (main_vbox), dialog->editor, FALSE, FALSE, 0);
  gtk_widget_show (dialog->editor);

  entry = PICMAN_SIZE_ENTRY (picman_template_editor_get_size_se (PICMAN_TEMPLATE_EDITOR (dialog->editor)));
  picman_size_entry_set_activates_default (entry, TRUE);
  picman_size_entry_grab_focus (entry);

  image_new_template_changed (dialog->context,
                              picman_context_get_template (dialog->context),
                              dialog);

  return dialog->dialog;
}

void
image_new_dialog_set (GtkWidget    *widget,
                      PicmanImage    *image,
                      PicmanTemplate *template)
{
  ImageNewDialog *dialog;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));
  g_return_if_fail (template == NULL || PICMAN_IS_TEMPLATE (template));

  dialog = g_object_get_data (G_OBJECT (widget), "picman-image-new-dialog");

  g_return_if_fail (dialog != NULL);

  picman_context_set_template (dialog->context, template);

  if (! template)
    {
      template = picman_image_new_get_last_template (dialog->context->picman,
                                                   image);

      /*  make sure the resolution values are copied first (see bug #546924)  */
      picman_config_sync (G_OBJECT (template), G_OBJECT (dialog->template),
                        PICMAN_TEMPLATE_PARAM_COPY_FIRST);
      picman_config_sync (G_OBJECT (template), G_OBJECT (dialog->template),
                        0);

      g_object_unref (template);
    }
}


/*  private functions  */

static void
image_new_dialog_free (ImageNewDialog *dialog)
{
  g_object_unref (dialog->context);
  g_object_unref (dialog->template);

  g_slice_free (ImageNewDialog, dialog);
}

static void
image_new_dialog_response (GtkWidget      *widget,
                           gint            response_id,
                           ImageNewDialog *dialog)
{
  switch (response_id)
    {
    case RESPONSE_RESET:
      picman_config_sync (G_OBJECT (dialog->context->picman->config->default_image),
                        G_OBJECT (dialog->template), 0);
      picman_context_set_template (dialog->context, NULL);
      break;

    case GTK_RESPONSE_OK:
      if (picman_template_get_initial_size (dialog->template) >
          PICMAN_GUI_CONFIG (dialog->context->picman->config)->max_new_image_size)
        image_new_confirm_dialog (dialog);
      else
        image_new_create_image (dialog);
      break;

    default:
      gtk_widget_destroy (dialog->dialog);
      break;
    }
}

static void
image_new_template_changed (PicmanContext    *context,
                            PicmanTemplate   *template,
                            ImageNewDialog *dialog)
{
  gchar *comment;

  if (!template)
    return;

  comment = (gchar *) picman_template_get_comment (template);

  if (! comment || ! strlen (comment))
    comment = g_strdup (picman_template_get_comment (dialog->template));
  else
    comment = NULL;

  /*  make sure the resolution values are copied first (see bug #546924)  */
  picman_config_sync (G_OBJECT (template), G_OBJECT (dialog->template),
                    PICMAN_TEMPLATE_PARAM_COPY_FIRST);
  picman_config_sync (G_OBJECT (template), G_OBJECT (dialog->template), 0);

  if (comment)
    {
      g_object_set (dialog->template,
                    "comment", comment,
                    NULL);

      g_free (comment);
    }
}


/*  the confirm dialog  */

static void
image_new_confirm_response (GtkWidget      *dialog,
                            gint            response_id,
                            ImageNewDialog *data)
{
  gtk_widget_destroy (dialog);

  data->confirm_dialog = NULL;

  if (response_id == GTK_RESPONSE_OK)
    image_new_create_image (data);
  else
    gtk_widget_set_sensitive (data->dialog, TRUE);
}

static void
image_new_confirm_dialog (ImageNewDialog *data)
{
  PicmanGuiConfig *config;
  GtkWidget     *dialog;
  gchar         *size;

  if (data->confirm_dialog)
    {
      gtk_window_present (GTK_WINDOW (data->confirm_dialog));
      return;
    }

  data->confirm_dialog =
    dialog = picman_message_dialog_new (_("Confirm Image Size"),
                                      PICMAN_STOCK_WARNING,
                                      data->dialog,
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      picman_standard_help_func, NULL,

                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                      NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (data->confirm_dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (image_new_confirm_response),
                    data);

  size = g_format_size (picman_template_get_initial_size (data->template));
  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("You are trying to create an image "
                                       "with a size of %s."), size);
  g_free (size);

  config = PICMAN_GUI_CONFIG (data->context->picman->config);
  size = g_format_size (config->max_new_image_size);
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                              _("An image of the chosen size will use more "
                                "memory than what is configured as "
                                "\"Maximum Image Size\" in the Preferences "
                                "dialog (currently %s)."), size);
  g_free (size);

  gtk_widget_set_sensitive (data->dialog, FALSE);

  gtk_widget_show (dialog);
}

static void
image_new_create_image (ImageNewDialog *dialog)
{
  PicmanTemplate *template = g_object_ref (dialog->template);
  Picman         *picman     = dialog->context->picman;

  gtk_widget_destroy (dialog->dialog);

  picman_image_new_from_template (picman, template, picman_get_user_context (picman));
  picman_image_new_set_last_template (picman, template);

  g_object_unref (template);
}
