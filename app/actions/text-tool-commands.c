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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmantextbuffer.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "display/picmandisplay.h"

#include "tools/picmantexttool.h"

#include "text-tool-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   text_tool_load_dialog_destroyed (GtkWidget    *dialog,
					       GObject      *tool);
static void   text_tool_load_dialog_response  (GtkWidget    *dialog,
					       gint          response_id,
					       PicmanTextTool *tool);


/*  public functions  */

void
text_tool_cut_cmd_callback (GtkAction *action,
                            gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_cut_clipboard (text_tool);
}

void
text_tool_copy_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_copy_clipboard (text_tool);
}

void
text_tool_paste_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_paste_clipboard (text_tool);
}

void
text_tool_delete_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_delete_selection (text_tool);
}

void
text_tool_load_cmd_callback (GtkAction *action,
                             gpointer   data)
{
  PicmanTextTool   *text_tool = PICMAN_TEXT_TOOL (data);
  GtkWidget      *dialog;
  GtkWidget      *parent    = NULL;
  GtkFileChooser *chooser;

  dialog = g_object_get_data (G_OBJECT (text_tool), "picman-text-file-dialog");

  if (dialog)
    {
      gtk_window_present (GTK_WINDOW (dialog));
      return;
    }

  if (PICMAN_TOOL (text_tool)->display)
    {
      PicmanDisplayShell *shell;

      shell = picman_display_get_shell (PICMAN_TOOL (text_tool)->display);

      parent = gtk_widget_get_toplevel (GTK_WIDGET (shell));
    }

  dialog = gtk_file_chooser_dialog_new (_("Open Text File (UTF-8)"),
					parent ? GTK_WINDOW (parent) : NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,

					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN,   GTK_RESPONSE_OK,

					NULL);

  chooser = GTK_FILE_CHOOSER (dialog);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_object_set_data (G_OBJECT (text_tool), "picman-text-file-dialog", dialog);

  g_signal_connect (dialog, "destroy",
		    G_CALLBACK (text_tool_load_dialog_destroyed),
		    text_tool);

  gtk_window_set_role (GTK_WINDOW (chooser), "picman-text-load-file");
  gtk_window_set_position (GTK_WINDOW (chooser), GTK_WIN_POS_MOUSE);

  gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_OK);

  g_signal_connect (chooser, "response",
                    G_CALLBACK (text_tool_load_dialog_response),
                    text_tool);
  g_signal_connect (chooser, "delete-event",
                    G_CALLBACK (gtk_true),
                    NULL);

  gtk_widget_show (GTK_WIDGET (chooser));
}

void
text_tool_clear_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanTextTool  *text_tool = PICMAN_TEXT_TOOL (data);
  GtkTextBuffer *buffer    = GTK_TEXT_BUFFER (text_tool->buffer);
  GtkTextIter    start, end;

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  gtk_text_buffer_select_range (buffer, &start, &end);
  picman_text_tool_delete_selection (text_tool);
}

void
text_tool_text_to_path_cmd_callback (GtkAction *action,
                                     gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_create_vectors (text_tool);
}

void
text_tool_text_along_path_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);

  picman_text_tool_create_vectors_warped (text_tool);
}

void
text_tool_direction_cmd_callback (GtkAction *action,
                                  GtkAction *current,
                                  gpointer   data)
{
  PicmanTextTool *text_tool = PICMAN_TEXT_TOOL (data);
  gint          value;

  value = gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action));

  g_object_set (text_tool->proxy,
		"base-direction", (PicmanTextDirection) value,
		NULL);
}


/*  private functions  */

static void
text_tool_load_dialog_destroyed (GtkWidget *dialog,
				 GObject   *tool)
{
  g_object_set_data (tool, "picman-text-file-dialog", NULL);
}

static void
text_tool_load_dialog_response (GtkWidget    *dialog,
				gint          response_id,
				PicmanTextTool *tool)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      gchar  *filename;
      GError *error = NULL;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      if (! picman_text_buffer_load (tool->buffer, filename, &error))
        {
          picman_message (PICMAN_TOOL (tool)->tool_info->picman, G_OBJECT (dialog),
                        PICMAN_MESSAGE_ERROR,
                        _("Could not open '%s' for reading: %s"),
                        picman_filename_to_utf8 (filename),
                        error->message);
          g_clear_error (&error);
          g_free (filename);
          return;
        }

      g_free (filename);
    }

  gtk_widget_hide (dialog);
}
