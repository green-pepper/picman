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

#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandata.h"
#include "core/picmandatafactory.h"

#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "data-delete-dialog.h"

#include "picman-intl.h"


typedef struct _DataDeleteDialog DataDeleteDialog;

struct _DataDeleteDialog
{
  PicmanDataFactory *factory;
  PicmanData        *data;
  PicmanContext     *context;
  GtkWidget       *parent;
};


/*  local function prototypes  */

static void  data_delete_dialog_response (GtkWidget        *dialog,
                                          gint              response_id,
                                          DataDeleteDialog *delete_data);


/*  public functions  */

GtkWidget *
data_delete_dialog_new (PicmanDataFactory *factory,
                        PicmanData        *data,
                        PicmanContext     *context,
                        GtkWidget       *parent)
{
  DataDeleteDialog *delete_data;
  GtkWidget        *dialog;

  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_DATA (data), NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);

  delete_data = g_slice_new0 (DataDeleteDialog);

  delete_data->factory = factory;
  delete_data->data    = data;
  delete_data->context = context;
  delete_data->parent  = parent;

  dialog = picman_message_dialog_new (_("Delete Object"), GTK_STOCK_DELETE,
                                    gtk_widget_get_toplevel (parent), 0,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_DELETE, GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect_object (data, "disconnect",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (data_delete_dialog_response),
                    delete_data);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Delete '%s'?"),
                                     picman_object_get_name (data));
  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("Are you sure you want to remove '%s' "
                               "from the list and delete it on disk?"),
                             picman_object_get_name (data));

  return dialog;
}


/*  private functions  */

static void
data_delete_dialog_response (GtkWidget        *dialog,
                             gint              response_id,
                             DataDeleteDialog *delete_data)
{
  gtk_widget_destroy (dialog);

  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanDataFactory *factory    = delete_data->factory;
      PicmanData        *data       = delete_data->data;
      PicmanContainer   *container;
      PicmanObject      *new_active = NULL;
      GError          *error      = NULL;

      container = picman_data_factory_get_container (factory);

      if (delete_data->context &&
          PICMAN_OBJECT (data) ==
          picman_context_get_by_type (delete_data->context,
                                    picman_container_get_children_type (container)))
        {
          new_active = picman_container_get_neighbor_of (container,
                                                       PICMAN_OBJECT (data));
        }

      if (! picman_data_factory_data_delete (factory, data, TRUE, &error))
        {
          picman_message (picman_data_factory_get_picman (factory),
                        G_OBJECT (delete_data->parent), PICMAN_MESSAGE_ERROR,
                        "%s", error->message);
          g_clear_error (&error);
        }

      if (new_active)
        picman_context_set_by_type (delete_data->context,
                                  picman_container_get_children_type (container),
                                  new_active);
    }

  g_slice_free (DataDeleteDialog, delete_data);
}
