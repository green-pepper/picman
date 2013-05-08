/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1999 Manish Singh
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmanimage.h"

#include "widgets/picmancolordisplayeditor.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-filter.h"
#include "picmandisplayshell-filter-dialog.h"

#include "picman-intl.h"


typedef struct
{
  PicmanDisplayShell      *shell;
  GtkWidget             *dialog;

  PicmanColorDisplayStack *old_stack;
} ColorDisplayDialog;


/*  local function prototypes  */

static void picman_display_shell_filter_dialog_response (GtkWidget          *widget,
                                                       gint                response_id,
                                                       ColorDisplayDialog *cdd);

static void picman_display_shell_filter_dialog_free     (ColorDisplayDialog *cdd);


/*  public functions  */

GtkWidget *
picman_display_shell_filter_dialog_new (PicmanDisplayShell *shell)
{
  PicmanDisplayConfig  *config;
  PicmanImage          *image;
  ColorDisplayDialog *cdd;
  GtkWidget          *editor;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  config = shell->display->config;
  image  = picman_display_get_image (shell->display);

  cdd = g_slice_new0 (ColorDisplayDialog);

  cdd->shell  = shell;
  cdd->dialog = picman_viewable_dialog_new (PICMAN_VIEWABLE (image),
                                          picman_get_user_context (shell->display->picman),
                                          _("Color Display Filters"),
                                          "picman-display-filters",
                                          PICMAN_STOCK_DISPLAY_FILTER,
                                          _("Configure Color Display Filters"),
                                          GTK_WIDGET (cdd->shell),
                                          picman_standard_help_func,
                                          PICMAN_HELP_DISPLAY_FILTER_DIALOG,

                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                          NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (cdd->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_destroy_with_parent (GTK_WINDOW (cdd->dialog), TRUE);

  g_object_weak_ref (G_OBJECT (cdd->dialog),
                     (GWeakNotify) picman_display_shell_filter_dialog_free, cdd);

  g_signal_connect (cdd->dialog, "response",
                    G_CALLBACK (picman_display_shell_filter_dialog_response),
                    cdd);

  if (shell->filter_stack)
    {
      cdd->old_stack = picman_color_display_stack_clone (shell->filter_stack);

      g_object_weak_ref (G_OBJECT (cdd->dialog),
                         (GWeakNotify) g_object_unref, cdd->old_stack);
    }
  else
    {
      PicmanColorDisplayStack *stack = picman_color_display_stack_new ();

      picman_display_shell_filter_set (shell, stack);
      g_object_unref (stack);
    }

  editor = picman_color_display_editor_new (shell->filter_stack,
                                          PICMAN_CORE_CONFIG (config)->color_management,
                                          PICMAN_COLOR_MANAGED (shell));
  gtk_container_set_border_width (GTK_CONTAINER (editor), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (cdd->dialog))),
                      editor, TRUE, TRUE, 0);
  gtk_widget_show (editor);

  return cdd->dialog;
}


/*  private functions  */

static void
picman_display_shell_filter_dialog_response (GtkWidget          *widget,
                                           gint                response_id,
                                           ColorDisplayDialog *cdd)
{
  if (response_id != GTK_RESPONSE_OK)
    picman_display_shell_filter_set (cdd->shell, cdd->old_stack);

  gtk_widget_destroy (GTK_WIDGET (cdd->dialog));
}

static void
picman_display_shell_filter_dialog_free (ColorDisplayDialog *cdd)
{
  g_slice_free (ColorDisplayDialog, cdd);
}
