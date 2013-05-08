/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmandnd-xds.c
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
 *
 * Saving Files via Drag-and-Drop:
 * The Direct Save Protocol for the X Window System
 *
 *   http://www.newplanetsoftware.com/xds/
 *   http://rox.sourceforge.net/xds.html
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

#undef GSEAL_ENABLE

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"

#include "plug-in/picmanpluginmanager.h"

#include "file/file-procedure.h"
#include "file/file-save.h"
#include "file/file-utils.h"

#include "picmandnd-xds.h"
#include "picmanfiledialog.h"
#include "picmanmessagebox.h"
#include "picmanmessagedialog.h"

#include "picman-log.h"
#include "picman-intl.h"


#define MAX_URI_LEN 4096


/*  local function prototypes  */

static gboolean   picman_file_overwrite_dialog (GtkWidget   *parent,
                                              const gchar *uri);


/*  public functions  */

void
picman_dnd_xds_source_set (GdkDragContext *context,
                         PicmanImage      *image)
{
  GdkAtom  property;

  g_return_if_fail (GDK_IS_DRAG_CONTEXT (context));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));

  PICMAN_LOG (DND, NULL);

  property = gdk_atom_intern_static_string ("XdndDirectSave0");

  if (image)
    {
      GdkAtom  type     = gdk_atom_intern_static_string ("text/plain");
      gchar   *filename = picman_image_get_filename (image);
      gchar   *basename;

      if (filename)
        {
          basename = g_path_get_basename (filename);
        }
      else
        {
          gchar *tmp = g_strconcat (_("Untitled"), ".xcf", NULL);
          basename = g_filename_from_utf8 (tmp, -1, NULL, NULL, NULL);
          g_free (tmp);
        }


      gdk_property_change (context->source_window,
                           property, type, 8, GDK_PROP_MODE_REPLACE,
                           (const guchar *) basename,
                           basename ? strlen (basename) : 0);

      g_free (basename);
      g_free (filename);
    }
  else
    {
      gdk_property_delete (context->source_window, property);
    }
}

void
picman_dnd_xds_save_image (GdkDragContext   *context,
                         PicmanImage        *image,
                         GtkSelectionData *selection)
{
  PicmanPlugInProcedure *proc;
  GdkAtom              property;
  GdkAtom              type;
  gint                 length;
  guchar              *data;
  gchar               *uri;
  gboolean             export = FALSE;
  GError              *error  = NULL;

  g_return_if_fail (GDK_IS_DRAG_CONTEXT (context));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  PICMAN_LOG (DND, NULL);

  property = gdk_atom_intern_static_string ("XdndDirectSave0");
  type     = gdk_atom_intern_static_string ("text/plain");

  if (! gdk_property_get (context->source_window, property, type,
                          0, MAX_URI_LEN, FALSE,
                          NULL, NULL, &length, &data))
    return;


  uri = g_strndup ((const gchar *) data, length);
  g_free (data);

  proc = file_procedure_find (image->picman->plug_in_manager->save_procs, uri,
                              NULL);
  if (! proc)
    {
      proc = file_procedure_find (image->picman->plug_in_manager->export_procs, uri,
                                  NULL);

      export = TRUE;
    }

  if (proc)
    {
      gchar *filename = file_utils_filename_from_uri (uri);

      /*  FIXME: shouldn't overwrite non-local files w/o confirmation  */

      if (! filename ||
          ! g_file_test (filename, G_FILE_TEST_EXISTS) ||
          picman_file_overwrite_dialog (NULL, uri))
        {
          if (file_save (image->picman,
                         image, NULL,
                         uri, proc, PICMAN_RUN_INTERACTIVE,
                         ! export, FALSE, export,
                         &error) == PICMAN_PDB_SUCCESS)
            {
              gtk_selection_data_set (selection,
                                      gtk_selection_data_get_target (selection),
                                      8, (const guchar *) "S", 1);
            }
          else
            {
              gtk_selection_data_set (selection,
                                      gtk_selection_data_get_target (selection),
                                      8, (const guchar *) "E", 1);

              if (error)
                {
                  gchar *filename = file_utils_uri_display_name (uri);

                  picman_message (image->picman, NULL, PICMAN_MESSAGE_ERROR,
                                _("Saving '%s' failed:\n\n%s"),
                                filename, error->message);

                  g_free (filename);
                  g_error_free (error);
                }
            }
        }

      g_free (filename);
    }
  else
    {
      gtk_selection_data_set (selection,
                              gtk_selection_data_get_target (selection),
                              8, (const guchar *) "E", 1);

      picman_message_literal (image->picman, NULL, PICMAN_MESSAGE_ERROR,
			    _("The given filename does not have any known "
			      "file extension."));
    }

  g_free (uri);
}


/*  private functions  */

static gboolean
picman_file_overwrite_dialog (GtkWidget   *parent,
                            const gchar *uri)
{
  GtkWidget *dialog;
  gchar     *filename;
  gboolean   overwrite = FALSE;

  dialog = picman_message_dialog_new (_("File Exists"), PICMAN_STOCK_WARNING,
                                    parent, GTK_DIALOG_DESTROY_WITH_PARENT,
                                    picman_standard_help_func, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    _("_Replace"),    GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  filename = file_utils_uri_display_name (uri);
  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     _("A file named '%s' already exists."),
                                     filename);
  g_free (filename);

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             _("Do you want to replace it with the image "
                               "you are saving?"));

  if (GTK_IS_DIALOG (parent))
    gtk_dialog_set_response_sensitive (GTK_DIALOG (parent),
                                       GTK_RESPONSE_CANCEL, FALSE);

  g_object_ref (dialog);

  overwrite = (picman_dialog_run (PICMAN_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);
  g_object_unref (dialog);

  if (GTK_IS_DIALOG (parent))
    gtk_dialog_set_response_sensitive (GTK_DIALOG (parent),
                                       GTK_RESPONSE_CANCEL, TRUE);

  return overwrite;
}
