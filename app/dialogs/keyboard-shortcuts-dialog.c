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

#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman.h"

#include "widgets/picmanactioneditor.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanuimanager.h"

#include "keyboard-shortcuts-dialog.h"

#include "picman-intl.h"


GtkWidget *
keyboard_shortcuts_dialog_new (Picman *picman)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *editor;
  GtkWidget *box;
  GtkWidget *button;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  dialog = picman_dialog_new (_("Configure Keyboard Shortcuts"),
                            "picman-keyboard-shortcuts-dialog",
                            NULL, 0,
                            picman_standard_help_func,
                            PICMAN_HELP_KEYBOARD_SHORTCUTS,

                            GTK_STOCK_CLOSE, GTK_RESPONSE_OK,

                            NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  editor = picman_action_editor_new (picman_ui_managers_from_name ("<Image>")->data,
                                   NULL, TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), editor, TRUE, TRUE, 0);
  gtk_widget_show (editor);

  box = picman_hint_box_new (_("To edit a shortcut key, click on the "
                             "corresponding row and type a new "
                             "accelerator, or press backspace to "
                             "clear."));
  gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  button = picman_prop_check_button_new (G_OBJECT (picman->config), "save-accels",
                                       _("S_ave keyboard shortcuts on exit"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  return dialog;
}
