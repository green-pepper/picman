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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage-new.h"
#include "core/picmantemplate.h"

#include "widgets/picmancontainerview.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmantemplateeditor.h"
#include "widgets/picmantemplateview.h"

#include "dialogs/template-options-dialog.h"

#include "actions.h"
#include "templates-commands.h"

#include "picman-intl.h"


typedef struct
{
  PicmanContext   *context;
  PicmanContainer *container;
  PicmanTemplate  *template;
} TemplateDeleteData;


/*  local function prototypes  */

static void   templates_new_response     (GtkWidget             *dialog,
                                          gint                   response_id,
                                          TemplateOptionsDialog *options);
static void   templates_edit_response    (GtkWidget             *widget,
                                          gint                   response_id,
                                          TemplateOptionsDialog *options);
static void   templates_delete_response  (GtkWidget             *dialog,
                                          gint                   response_id,
                                          TemplateDeleteData    *delete_data);
static void   templates_delete_data_free (TemplateDeleteData    *delete_data);


/*  public functions */

void
templates_create_image_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  Picman                *picman;
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanTemplate        *template;
  return_if_no_picman(picman,data);

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  template = picman_context_get_template (context);

  if (template && picman_container_have (container, PICMAN_OBJECT (template)))
    {
      picman_image_new_from_template (picman, template, context);
      picman_image_new_set_last_template (picman, template);
    }
}

void
templates_new_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanContainerEditor   *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext           *context;
  TemplateOptionsDialog *options;

  context = picman_container_view_get_context (editor->view);

  options = template_options_dialog_new (NULL, context,
                                         GTK_WIDGET (editor),
                                         _("New Template"),
                                         "picman-template-new",
                                         PICMAN_STOCK_TEMPLATE,
                                         _("Create a New Template"),
                                         PICMAN_HELP_TEMPLATE_NEW);

  g_signal_connect (options->dialog, "response",
                    G_CALLBACK (templates_new_response),
                    options);

  gtk_widget_show (options->dialog);
}

void
templates_duplicate_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanTemplate        *template;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  template = picman_context_get_template (context);

  if (template && picman_container_have (container, PICMAN_OBJECT (template)))
    {
      PicmanTemplate *new_template;

      new_template = picman_config_duplicate (PICMAN_CONFIG (template));

      picman_container_add (container, PICMAN_OBJECT (new_template));
      picman_context_set_by_type (context,
                                picman_container_get_children_type (container),
                                PICMAN_OBJECT (new_template));
      g_object_unref (new_template);

      templates_edit_cmd_callback (action, data);
    }
}

void
templates_edit_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanTemplate        *template;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  template = picman_context_get_template (context);

  if (template && picman_container_have (container, PICMAN_OBJECT (template)))
    {
      TemplateOptionsDialog *options;

      options = template_options_dialog_new (template, context,
                                             GTK_WIDGET (editor),
                                             _("Edit Template"),
                                             "picman-template-edit",
                                             GTK_STOCK_EDIT,
                                             _("Edit Template"),
                                             PICMAN_HELP_TEMPLATE_EDIT);

      g_signal_connect (options->dialog, "response",
                        G_CALLBACK (templates_edit_response),
                        options);

      gtk_widget_show (options->dialog);
    }
}

void
templates_delete_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;
  PicmanContext         *context;
  PicmanTemplate        *template;

  container = picman_container_view_get_container (editor->view);
  context   = picman_container_view_get_context (editor->view);

  template = picman_context_get_template (context);

  if (template && picman_container_have (container, PICMAN_OBJECT (template)))
    {
      TemplateDeleteData *delete_data = g_slice_new (TemplateDeleteData);
      GtkWidget          *dialog;

      delete_data->context   = context;
      delete_data->container = container;
      delete_data->template  = template;

      dialog =
        picman_message_dialog_new (_("Delete Template"), GTK_STOCK_DELETE,
                                 GTK_WIDGET (editor), 0,
                                 picman_standard_help_func, NULL,

                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_DELETE, GTK_RESPONSE_OK,

                                 NULL);

      gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                               GTK_RESPONSE_OK,
                                               GTK_RESPONSE_CANCEL,
                                               -1);

      g_object_weak_ref (G_OBJECT (dialog),
                         (GWeakNotify) templates_delete_data_free, delete_data);

      g_signal_connect_object (template, "disconnect",
                               G_CALLBACK (gtk_widget_destroy),
                               dialog, G_CONNECT_SWAPPED);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (templates_delete_response),
                        delete_data);

      picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                         _("Are you sure you want to delete "
                                           "template '%s' from the list and "
                                           "from disk?"),
                                         picman_object_get_name (template));
      gtk_widget_show (dialog);
    }
}


/*  private functions  */

static void
templates_new_response (GtkWidget             *dialog,
                        gint                   response_id,
                        TemplateOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanTemplateEditor *editor = PICMAN_TEMPLATE_EDITOR (options->editor);
      PicmanTemplate       *template;

      template = picman_template_editor_get_template (editor);

      picman_container_add (options->picman->templates, PICMAN_OBJECT (template));
      picman_context_set_template (picman_get_user_context (options->picman),
                                 template);
    }

  gtk_widget_destroy (dialog);
}

static void
templates_edit_response (GtkWidget             *dialog,
                         gint                   response_id,
                         TemplateOptionsDialog *options)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanTemplateEditor *editor = PICMAN_TEMPLATE_EDITOR (options->editor);
      PicmanTemplate       *template;

      template = picman_template_editor_get_template (editor);

      picman_config_sync (G_OBJECT (template),
                        G_OBJECT (options->template), 0);
    }

  gtk_widget_destroy (dialog);
}

static void
templates_delete_response (GtkWidget          *dialog,
                           gint                response_id,
                           TemplateDeleteData *delete_data)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanObject *new_active = NULL;

      if (delete_data->template ==
          picman_context_get_template (delete_data->context))
        {
          new_active = picman_container_get_neighbor_of (delete_data->container,
                                                       PICMAN_OBJECT (delete_data->template));
        }

      if (picman_container_have (delete_data->container,
                               PICMAN_OBJECT (delete_data->template)))
        {
          if (new_active)
            picman_context_set_by_type (delete_data->context,
                                      picman_container_get_children_type (delete_data->container),
                                      new_active);

          picman_container_remove (delete_data->container,
                                 PICMAN_OBJECT (delete_data->template));
        }
    }

  gtk_widget_destroy (dialog);
}

static void
templates_delete_data_free (TemplateDeleteData *delete_data)
{
  g_slice_free (TemplateDeleteData, delete_data);
}
