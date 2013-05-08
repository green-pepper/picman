/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
 *
 * picmanerrorconsole.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
 *
 * partly based on errorconsole.c
 * Copyright (C) 1998 Nick Fetchak <nuke@bayside.net>
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

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"

#include "picmanerrorconsole.h"
#include "picmanmenufactory.h"
#include "picmantextbuffer.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


static void      picman_error_console_constructed  (GObject          *object);
static void      picman_error_console_dispose      (GObject          *object);

static void      picman_error_console_unmap        (GtkWidget        *widget);

static gboolean  picman_error_console_button_press (GtkWidget        *widget,
                                                  GdkEventButton   *event,
                                                  PicmanErrorConsole *console);


G_DEFINE_TYPE (PicmanErrorConsole, picman_error_console, PICMAN_TYPE_EDITOR)

#define parent_class picman_error_console_parent_class


static void
picman_error_console_class_init (PicmanErrorConsoleClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = picman_error_console_constructed;
  object_class->dispose     = picman_error_console_dispose;

  widget_class->unmap       = picman_error_console_unmap;
}

static void
picman_error_console_init (PicmanErrorConsole *console)
{
  GtkWidget *scrolled_window;

  console->text_buffer = GTK_TEXT_BUFFER (picman_text_buffer_new ());

  gtk_text_buffer_create_tag (console->text_buffer, "title",
                              "scale",  PANGO_SCALE_LARGE,
                              "weight", PANGO_WEIGHT_BOLD,
                              NULL);
  gtk_text_buffer_create_tag (console->text_buffer, "message",
                              NULL);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (console), scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show (scrolled_window);

  console->text_view = gtk_text_view_new_with_buffer (console->text_buffer);
  g_object_unref (console->text_buffer);

  gtk_text_view_set_editable (GTK_TEXT_VIEW (console->text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (console->text_view),
                               GTK_WRAP_WORD);
  gtk_container_add (GTK_CONTAINER (scrolled_window), console->text_view);
  gtk_widget_show (console->text_view);

  g_signal_connect (console->text_view, "button-press-event",
                    G_CALLBACK (picman_error_console_button_press),
                    console);

  console->file_dialog = NULL;
}

static void
picman_error_console_constructed (GObject *object)
{
  PicmanErrorConsole *console = PICMAN_ERROR_CONSOLE (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  console->clear_button =
    picman_editor_add_action_button (PICMAN_EDITOR (console), "error-console",
                                   "error-console-clear", NULL);

  console->save_button =
    picman_editor_add_action_button (PICMAN_EDITOR (console), "error-console",
                                   "error-console-save-all",
                                   "error-console-save-selection",
                                   GDK_SHIFT_MASK,
                                   NULL);
}

static void
picman_error_console_dispose (GObject *object)
{
  PicmanErrorConsole *console = PICMAN_ERROR_CONSOLE (object);

  if (console->file_dialog)
    gtk_widget_destroy (console->file_dialog);

  console->picman->message_handler = PICMAN_MESSAGE_BOX;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_error_console_unmap (GtkWidget *widget)
{
  PicmanErrorConsole *console = PICMAN_ERROR_CONSOLE (widget);

  if (console->file_dialog)
    gtk_widget_destroy (console->file_dialog);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

GtkWidget *
picman_error_console_new (Picman            *picman,
                        PicmanMenuFactory *menu_factory)
{
  PicmanErrorConsole *console;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  console = g_object_new (PICMAN_TYPE_ERROR_CONSOLE,
                          "menu-factory",   menu_factory,
                          "menu-identifier", "<ErrorConsole>",
                          "ui-path",        "/error-console-popup",
                          NULL);

  console->picman = picman;

  console->picman->message_handler = PICMAN_ERROR_CONSOLE;

  return GTK_WIDGET (console);
}

void
picman_error_console_add (PicmanErrorConsole    *console,
                        PicmanMessageSeverity  severity,
                        const gchar         *domain,
                        const gchar         *message)
{
  const gchar *desc;
  GtkTextIter  end;
  GtkTextMark *end_mark;
  GdkPixbuf   *pixbuf;
  gchar       *str;

  g_return_if_fail (PICMAN_IS_ERROR_CONSOLE (console));
  g_return_if_fail (domain != NULL);
  g_return_if_fail (message != NULL);

  picman_enum_get_value (PICMAN_TYPE_MESSAGE_SEVERITY, severity,
                       NULL, NULL, &desc, NULL);

  gtk_text_buffer_get_end_iter (console->text_buffer, &end);

  pixbuf = gtk_widget_render_icon (console->text_view,
                                   picman_get_message_stock_id (severity),
                                   GTK_ICON_SIZE_BUTTON, NULL);
  gtk_text_buffer_insert_pixbuf (console->text_buffer, &end, pixbuf);
  g_object_unref (pixbuf);

  gtk_text_buffer_insert (console->text_buffer, &end, "  ", -1);

  str = g_strdup_printf ("%s %s", domain, desc);
  gtk_text_buffer_insert_with_tags_by_name (console->text_buffer, &end,
                                            str, -1,
                                            "title",
                                            NULL);
  g_free (str);

  gtk_text_buffer_insert (console->text_buffer, &end, "\n", -1);

  gtk_text_buffer_insert_with_tags_by_name (console->text_buffer, &end,
                                            message, -1,
                                            "message",
                                            NULL);

  gtk_text_buffer_insert (console->text_buffer, &end, "\n\n", -1);

  end_mark = gtk_text_buffer_create_mark (console->text_buffer,
                                          NULL, &end, TRUE);
  gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (console->text_view), end_mark,
                                FALSE, TRUE, 1.0, 0.0);
  gtk_text_buffer_delete_mark (console->text_buffer, end_mark);
}


/*  private functions  */

static gboolean
picman_error_console_button_press (GtkWidget        *widget,
                                 GdkEventButton   *bevent,
                                 PicmanErrorConsole *console)
{
  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      return picman_editor_popup_menu (PICMAN_EDITOR (console), NULL, NULL);
    }

  return FALSE;
}
