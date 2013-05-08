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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "actions.h"
#include "window-commands.h"


/*  public functions  */

void
window_close_cmd_callback (GtkAction *action,
                           gpointer   data)
{
  GtkWidget *widget;
  return_if_no_widget (widget, data);

  if (! gtk_widget_is_toplevel (widget))
    widget = gtk_widget_get_toplevel (widget);

  if (widget && gtk_widget_get_window (widget))
    {
      GdkEvent *event = gdk_event_new (GDK_DELETE);

      event->any.window     = g_object_ref (gtk_widget_get_window (widget));
      event->any.send_event = TRUE;

      gtk_main_do_event (event);
      gdk_event_free (event);
    }
}

void
window_open_display_cmd_callback (GtkAction *action,
                                  gpointer   data)
{
  GtkWidget *widget;
  GtkWidget *dialog;
  GtkWidget *entry;
  return_if_no_widget (widget, data);

  dialog = picman_message_dialog_new ("Open Display", PICMAN_STOCK_WILBER_EEK,
                                    widget, GTK_DIALOG_MODAL,
                                    NULL, NULL,

                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                    NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  picman_message_box_set_primary_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                     "Experimental multi-display stuff!\n"
                                     "Click OK and have fun crashing PICMAN...");

  picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                             "Please enter the name of the new display:");

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  gtk_box_pack_start (GTK_BOX (PICMAN_MESSAGE_DIALOG (dialog)->box), entry,
                      TRUE, TRUE, 0);

  gtk_widget_grab_focus (entry);
  gtk_widget_show_all (dialog);

  while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    {
      gchar *screen_name;

      screen_name = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

      if (strcmp (screen_name, ""))
        {
          GdkDisplay *display;

          gtk_widget_set_sensitive (dialog, FALSE);

          display = gdk_display_open (screen_name);

          if (! display)
            picman_message_box_set_text (PICMAN_MESSAGE_DIALOG (dialog)->box,
                                       "Can't open display '%s'. "
                                       "Please try another one:",
                                       screen_name);

          g_free (screen_name);

          gtk_widget_set_sensitive (dialog, TRUE);

          if (display)
            break;
        }

      gtk_widget_grab_focus (entry);
    }

  gtk_widget_destroy (dialog);
}

void
window_move_to_screen_cmd_callback (GtkAction *action,
                                    GtkAction *current,
                                    gpointer   data)
{
  GtkWidget *widget;
  GdkScreen *screen;
  return_if_no_widget (widget, data);

  if (! gtk_widget_is_toplevel (widget))
    widget = gtk_widget_get_toplevel (widget);

  screen = g_object_get_data (G_OBJECT (current), "screen");

  if (GDK_IS_SCREEN (screen) && screen != gtk_widget_get_screen (widget))
    {
      gtk_window_set_screen (GTK_WINDOW (widget), screen);
    }
}
