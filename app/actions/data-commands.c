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

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandata.h"
#include "core/picmandatafactory.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "widgets/picmanclipboard.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmandataeditor.h"
#include "widgets/picmandatafactoryview.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"
#include "widgets/picmanwindowstrategy.h"

#include "dialogs/data-delete-dialog.h"

#include "actions.h"
#include "data-commands.h"

#include "picman-intl.h"


/*  public functions  */

void
data_open_as_image_cmd_callback (GtkAction *action,
                                 gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  PicmanContext         *context;
  PicmanData            *data;

  context =
    picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

  data = (PicmanData *)
    picman_context_get_by_type (context,
                              picman_data_factory_view_get_children_type (view));

  if (data && picman_data_get_filename (data))
    {
      gchar *uri = g_filename_to_uri (picman_data_get_filename (data), NULL, NULL);

      if (uri)
        {
          PicmanImage         *image;
          PicmanPDBStatusType  status;
          GError            *error = NULL;

          image = file_open_with_display (context->picman, context, NULL,
                                          uri, FALSE,
                                          &status, &error);

          if (! image && status != PICMAN_PDB_CANCEL)
            {
              gchar *filename = file_utils_uri_display_name (uri);

              picman_message (context->picman, G_OBJECT (view),
                            PICMAN_MESSAGE_ERROR,
                            _("Opening '%s' failed:\n\n%s"),
                            filename, error->message);
              g_clear_error (&error);

              g_free (filename);
            }

          g_free (uri);
        }
    }
}

void
data_new_cmd_callback (GtkAction *action,
                       gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);

  if (picman_data_factory_view_has_data_new_func (view))
    {
      PicmanDataFactory *factory;
      PicmanContext     *context;
      PicmanData        *data;

      factory = picman_data_factory_view_get_data_factory (view);

      context =
        picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

      data = picman_data_factory_data_new (factory, context, _("Untitled"));

      if (data)
        {
          picman_context_set_by_type (context,
                                    picman_data_factory_view_get_children_type (view),
                                    PICMAN_OBJECT (data));

          gtk_button_clicked (GTK_BUTTON (picman_data_factory_view_get_edit_button (view)));
        }
    }
}

void
data_duplicate_cmd_callback (GtkAction *action,
                             gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  PicmanContext         *context;
  PicmanData            *data;

  context = picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

  data = (PicmanData *)
    picman_context_get_by_type (context,
                              picman_data_factory_view_get_children_type (view));

  if (data && picman_data_factory_view_have (view,
                                           PICMAN_OBJECT (data)))
    {
      PicmanData *new_data;

      new_data = picman_data_factory_data_duplicate (picman_data_factory_view_get_data_factory (view), data);

      if (new_data)
        {
          picman_context_set_by_type (context,
                                    picman_data_factory_view_get_children_type (view),
                                    PICMAN_OBJECT (new_data));

          gtk_button_clicked (GTK_BUTTON (picman_data_factory_view_get_edit_button (view)));
        }
    }
}

void
data_copy_location_cmd_callback (GtkAction *action,
                                 gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  PicmanContext         *context;
  PicmanData            *data;

  context = picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

  data = (PicmanData *)
    picman_context_get_by_type (context,
                              picman_data_factory_view_get_children_type (view));

  if (data)
    {
      const gchar *filename = picman_data_get_filename (data);

      if (filename && *filename)
        {
          gchar *uri = g_filename_to_uri (filename, NULL, NULL);

          if (uri)
            {
              picman_clipboard_set_text (context->picman, uri);
              g_free (uri);
            }
        }
    }
}

void
data_delete_cmd_callback (GtkAction *action,
                          gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  PicmanContext         *context;
  PicmanData            *data;

  context =
    picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

  data = (PicmanData *)
    picman_context_get_by_type (context,
                              picman_data_factory_view_get_children_type (view));

  if (data                          &&
      picman_data_is_deletable (data) &&
      picman_data_factory_view_have (view,
                                   PICMAN_OBJECT (data)))
    {
      PicmanDataFactory *factory;
      GtkWidget       *dialog;

      factory = picman_data_factory_view_get_data_factory (view);

      dialog = data_delete_dialog_new (factory, data, context,
                                       GTK_WIDGET (view));
      gtk_widget_show (dialog);
    }
}

void
data_refresh_cmd_callback (GtkAction *action,
                           gpointer   user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  Picman                *picman;
  return_if_no_picman (picman, user_data);

  picman_set_busy (picman);
  picman_data_factory_data_refresh (picman_data_factory_view_get_data_factory (view),
                                  action_data_get_context (user_data));
  picman_unset_busy (picman);
}

void
data_edit_cmd_callback (GtkAction   *action,
                        const gchar *value,
                        gpointer     user_data)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (user_data);
  PicmanContext         *context;
  PicmanData            *data;

  context = picman_container_view_get_context (PICMAN_CONTAINER_EDITOR (view)->view);

  data = (PicmanData *)
    picman_context_get_by_type (context,
                              picman_data_factory_view_get_children_type (view));

  if (data && picman_data_factory_view_have (view,
                                           PICMAN_OBJECT (data)))
    {
      GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (view));
      GtkWidget *dockable;

      dockable =
        picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                                   context->picman,
                                                   picman_dialog_factory_get_singleton (),
                                                   screen,
                                                   value);

      picman_data_editor_set_data (PICMAN_DATA_EDITOR (gtk_bin_get_child (GTK_BIN (dockable))),
                                 data);
    }
}
