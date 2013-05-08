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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "core/picman.h"
#include "core/picmanviewable.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-rotate.h"
#include "picmandisplayshell-rotate-dialog.h"

#include "picman-intl.h"


#define RESPONSE_RESET 1


typedef struct
{
  PicmanDisplayShell *shell;
  GtkAdjustment    *rotate_adj;
} RotateDialogData;


/*  local function prototypes  */

static void  picman_display_shell_rotate_dialog_response (GtkWidget        *widget,
                                                        gint              response_id,
                                                        RotateDialogData  *dialog);
static void  picman_display_shell_rotate_dialog_free     (RotateDialogData  *dialog);

static void  rotate_adjustment_changed                (GtkAdjustment     *adj,
                                                       RotateDialogData  *dialog);
static void  display_shell_rotated                    (PicmanDisplayShell  *shell,
                                                       RotateDialogData  *dialog);



/*  public functions  */

/**
 * picman_display_shell_rotate_dialog:
 * @shell: the #PicmanDisplayShell
 *
 * Constructs and displays a dialog allowing the user to enter a
 * custom display rotate.
 **/
void
picman_display_shell_rotate_dialog (PicmanDisplayShell *shell)
{
  RotateDialogData *data;
  PicmanImage        *image;
  GtkWidget        *toplevel;
  GtkWidget        *hbox;
  GtkWidget        *spin;
  GtkWidget        *label;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->rotate_dialog)
    {
      gtk_window_present (GTK_WINDOW (shell->rotate_dialog));
      return;
    }

  image = picman_display_get_image (shell->display);

  data = g_slice_new (RotateDialogData);

  data->shell = shell;

  shell->rotate_dialog =
    picman_viewable_dialog_new (PICMAN_VIEWABLE (image),
                              picman_get_user_context (shell->display->picman),
                              _("Rotate View"), "display-rotate",
                              PICMAN_STOCK_ROTATE_180,
                              _("Select Rotation Angle"),
                              GTK_WIDGET (shell),
                              picman_standard_help_func,
                              PICMAN_HELP_VIEW_ROTATE_OTHER,

                              PICMAN_STOCK_RESET, RESPONSE_RESET,
                              GTK_STOCK_OK,     GTK_RESPONSE_OK,

                              NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (shell->rotate_dialog),
                                           GTK_RESPONSE_OK,
                                           RESPONSE_RESET,
                                           -1);

  g_object_weak_ref (G_OBJECT (shell->rotate_dialog),
                     (GWeakNotify) picman_display_shell_rotate_dialog_free, data);

  g_object_add_weak_pointer (G_OBJECT (shell->rotate_dialog),
                             (gpointer) &shell->rotate_dialog);

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

  gtk_window_set_transient_for (GTK_WINDOW (shell->rotate_dialog),
                                GTK_WINDOW (toplevel));
  gtk_window_set_destroy_with_parent (GTK_WINDOW (shell->rotate_dialog), TRUE);

  g_signal_connect (shell->rotate_dialog, "response",
                    G_CALLBACK (picman_display_shell_rotate_dialog_response),
                    data);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (shell->rotate_dialog))),
                      hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Angle:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  spin = picman_spin_button_new ((GtkObject **) &data->rotate_adj,
                               shell->rotate_angle,
                               0.0, 360.0,
                               1, 15, 0, 1, 2);
  gtk_entry_set_activates_default (GTK_ENTRY (spin), TRUE);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spin), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), spin, TRUE, TRUE, 0);
  gtk_widget_show (spin);

  label = gtk_label_new (_("degrees"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  g_signal_connect (data->rotate_adj, "value-changed",
                    G_CALLBACK (rotate_adjustment_changed),
                    data);
  g_signal_connect (shell, "rotated",
                    G_CALLBACK (display_shell_rotated),
                    data);

  gtk_widget_show (shell->rotate_dialog);
}

static void
picman_display_shell_rotate_dialog_response (GtkWidget        *widget,
                                           gint              response_id,
                                           RotateDialogData *dialog)
{
  if (response_id == RESPONSE_RESET)
    {
      gtk_adjustment_set_value (dialog->rotate_adj, 0.0);
    }

  gtk_widget_destroy (dialog->shell->rotate_dialog);
}

static void
picman_display_shell_rotate_dialog_free (RotateDialogData *dialog)
{
  g_signal_handlers_disconnect_by_func (dialog->shell,
                                        display_shell_rotated,
                                        dialog);

  g_slice_free (RotateDialogData, dialog);
}

static void
rotate_adjustment_changed (GtkAdjustment    *adj,
                           RotateDialogData *dialog)
{
  gdouble angle = gtk_adjustment_get_value (dialog->rotate_adj);

  g_signal_handlers_block_by_func (dialog->shell,
                                   display_shell_rotated,
                                   dialog);

  picman_display_shell_rotate_to (dialog->shell, angle);

  g_signal_handlers_unblock_by_func (dialog->shell,
                                     display_shell_rotated,
                                     dialog);
}

static void
display_shell_rotated (PicmanDisplayShell  *shell,
                       RotateDialogData  *dialog)
{
  g_signal_handlers_block_by_func (dialog->rotate_adj,
                                   rotate_adjustment_changed,
                                   dialog);

  gtk_adjustment_set_value (dialog->rotate_adj, shell->rotate_angle);

  g_signal_handlers_unblock_by_func (dialog->rotate_adj,
                                     rotate_adjustment_changed,
                                     dialog);
}
