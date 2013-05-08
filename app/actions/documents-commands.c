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

#include "libpicmanthumb/picmanthumb.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimagefile.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "widgets/picmanclipboard.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmancontainerview-utils.h"
#include "widgets/picmandocumentview.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"
#include "display/picmandisplayshell.h"

#include "documents-commands.h"
#include "file-commands.h"

#include "picman-intl.h"


typedef struct
{
  const gchar *name;
  gboolean     found;
} RaiseClosure;


/*  local function prototypes  */

static void   documents_open_image    (GtkWidget     *editor,
                                       PicmanContext   *context,
                                       PicmanImagefile *imagefile);
static void   documents_raise_display (PicmanDisplay   *display,
                                       RaiseClosure  *closure);



/*  public functions */

void
documents_open_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanContainer       *container;
  PicmanImagefile       *imagefile;

  context   = picman_container_view_get_context (editor->view);
  container = picman_container_view_get_container (editor->view);

  imagefile = picman_context_get_imagefile (context);

  if (imagefile && picman_container_have (container, PICMAN_OBJECT (imagefile)))
    {
      documents_open_image (GTK_WIDGET (editor), context, imagefile);
    }
  else
    {
      file_file_open_dialog (context->picman, NULL, GTK_WIDGET (editor));
    }
}

void
documents_raise_or_open_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanContainer       *container;
  PicmanImagefile       *imagefile;

  context   = picman_container_view_get_context (editor->view);
  container = picman_container_view_get_container (editor->view);

  imagefile = picman_context_get_imagefile (context);

  if (imagefile && picman_container_have (container, PICMAN_OBJECT (imagefile)))
    {
      RaiseClosure closure;

      closure.name  = picman_object_get_name (imagefile);
      closure.found = FALSE;

      picman_container_foreach (context->picman->displays,
                              (GFunc) documents_raise_display,
                              &closure);

      if (! closure.found)
        documents_open_image (GTK_WIDGET (editor), context, imagefile);
    }
}

void
documents_file_open_dialog_cmd_callback (GtkAction *action,
                                         gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanContainer       *container;
  PicmanImagefile       *imagefile;

  context   = picman_container_view_get_context (editor->view);
  container = picman_container_view_get_container (editor->view);

  imagefile = picman_context_get_imagefile (context);

  if (imagefile && picman_container_have (container, PICMAN_OBJECT (imagefile)))
    {
      file_file_open_dialog (context->picman,
                             picman_object_get_name (imagefile),
                             GTK_WIDGET (editor));
    }
}

void
documents_copy_location_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanImagefile       *imagefile;

  context   = picman_container_view_get_context (editor->view);
  imagefile = picman_context_get_imagefile (context);

  if (imagefile)
    picman_clipboard_set_text (context->picman,
                             picman_object_get_name (imagefile));
}

void
documents_remove_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanContainerEditor *editor  = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context = picman_container_view_get_context (editor->view);
  PicmanImagefile       *imagefile = picman_context_get_imagefile (context);
  const gchar         *uri;

  uri = picman_object_get_name (imagefile);

  gtk_recent_manager_remove_item (gtk_recent_manager_get_default (), uri, NULL);

  picman_container_view_remove_active (editor->view);
}

void
documents_clear_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanContainerEditor *editor  = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context = picman_container_view_get_context (editor->view);
  Picman                *picman    = context->picman;
  GtkWidget           *dialog;

  dialog = picman_message_dialog_new (_("Clear Document History"),
                                    GTK_STOCK_CLEAR,
                                    GTK_WIDGET (editor),
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_CLEAR,  GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect_object (gtk_widget_get_toplevel (GTK_WIDGET (editor)),
                           "unmap",
                           G_CALLBACK (gtk_widget_destroy),
                           dialog, G_CONNECT_SWAPPED);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("Clear the Recent Documents list?"));

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("Clearing the document history will "
                               "permanently remove all images from "
                               "the recent documents list."));

  if (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      GtkRecentManager *manager = gtk_recent_manager_get_default ();
      GList            *items;
      GList            *list;

      items = gtk_recent_manager_get_items (manager);

      for (list = items; list; list = list->next)
        {
          GtkRecentInfo *info = list->data;

          if (gtk_recent_info_has_application (info,
                                               "GNU Image Manipulation Program"))
            {
              gtk_recent_manager_remove_item (manager,
                                              gtk_recent_info_get_uri (info),
                                              NULL);
            }

          gtk_recent_info_unref (info);
        }

      g_list_free (items);

      picman_container_clear (picman->documents);
    }

  gtk_widget_destroy (dialog);
}

void
documents_recreate_preview_cmd_callback (GtkAction *action,
                                         gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanContainer       *container;
  PicmanImagefile       *imagefile;

  context   = picman_container_view_get_context (editor->view);
  container = picman_container_view_get_container (editor->view);

  imagefile = picman_context_get_imagefile (context);

  if (imagefile && picman_container_have (container, PICMAN_OBJECT (imagefile)))
    {
      picman_imagefile_create_thumbnail (imagefile,
                                       context, NULL,
                                       context->picman->config->thumbnail_size,
                                       FALSE);
    }
}

void
documents_reload_previews_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;

  container = picman_container_view_get_container (editor->view);

  picman_container_foreach (container,
                          (GFunc) picman_imagefile_update,
                          editor->view);
}

static void
documents_remove_dangling_foreach (PicmanImagefile *imagefile,
                                   PicmanContainer *container)
{
  PicmanThumbnail *thumbnail = picman_imagefile_get_thumbnail (imagefile);

  if (picman_thumbnail_peek_image (thumbnail) == PICMAN_THUMB_STATE_NOT_FOUND)
    {
      const gchar *uri = picman_object_get_name (imagefile);

      gtk_recent_manager_remove_item (gtk_recent_manager_get_default (), uri,
                                      NULL);

      picman_container_remove (container, PICMAN_OBJECT (imagefile));
    }
}

void
documents_remove_dangling_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContainer       *container;

  container = picman_container_view_get_container (editor->view);

  picman_container_foreach (container,
                          (GFunc) documents_remove_dangling_foreach,
                          container);
}


/*  private functions  */

static void
documents_open_image (GtkWidget     *editor,
                      PicmanContext   *context,
                      PicmanImagefile *imagefile)
{
  const gchar        *uri;
  PicmanImage          *image;
  PicmanPDBStatusType   status;
  GError             *error = NULL;

  uri = picman_object_get_name (imagefile);

  image = file_open_with_display (context->picman, context, NULL, uri, FALSE,
                                  &status, &error);

  if (! image && status != PICMAN_PDB_CANCEL)
    {
      gchar *filename = file_utils_uri_display_name (uri);

      picman_message (context->picman, G_OBJECT (editor), PICMAN_MESSAGE_ERROR,
                    _("Opening '%s' failed:\n\n%s"),
                    filename, error->message);
      g_clear_error (&error);

      g_free (filename);
    }
}

static void
documents_raise_display (PicmanDisplay  *display,
                         RaiseClosure *closure)
{
  const gchar *uri = picman_object_get_name (picman_display_get_image (display));

  if (! g_strcmp0 (closure->name, uri))
    {
      closure->found = TRUE;
      picman_display_shell_present (picman_display_get_shell (display));
    }
}
