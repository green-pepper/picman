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
#include "core/picmangradient-save.h"
#include "core/picmancontext.h"

#include "widgets/picmancontainereditor.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmanhelp-ids.h"

#include "gradients-commands.h"

#include "picman-intl.h"


/*  local function prototypes  */

static void   gradients_save_as_pov_ray_response (GtkWidget    *dialog,
                                                  gint          response_id,
                                                  PicmanGradient *gradient);


/*  public functions  */

void
gradients_save_as_pov_ray_cmd_callback (GtkAction *action,
                                        gpointer   data)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (data);
  PicmanContext         *context;
  PicmanGradient        *gradient;
  GtkFileChooser      *chooser;
  gchar               *title;

  context = picman_container_view_get_context (editor->view);

  gradient = picman_context_get_gradient (context);

  if (! gradient)
    return;

  title = g_strdup_printf (_("Save '%s' as POV-Ray"),
                           picman_object_get_name (gradient));

  chooser = GTK_FILE_CHOOSER
    (gtk_file_chooser_dialog_new (title, NULL,
                                  GTK_FILE_CHOOSER_ACTION_SAVE,

                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_SAVE,   GTK_RESPONSE_OK,

                                  NULL));

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (chooser),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_object_set_data (G_OBJECT (chooser), "picman", context->picman);

  g_free (title);

  gtk_window_set_screen (GTK_WINDOW (chooser),
                         gtk_widget_get_screen (GTK_WIDGET (editor)));

  gtk_window_set_role (GTK_WINDOW (chooser), "picman-gradient-save-pov");
  gtk_window_set_position (GTK_WINDOW (chooser), GTK_WIN_POS_MOUSE);

  gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_OK);
  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

  g_signal_connect (chooser, "response",
                    G_CALLBACK (gradients_save_as_pov_ray_response),
                    gradient);
  g_signal_connect (chooser, "delete-event",
                    G_CALLBACK (gtk_true),
                    NULL);

  g_object_ref (gradient);

  g_signal_connect_object (chooser, "destroy",
                           G_CALLBACK (g_object_unref),
                           gradient,
                           G_CONNECT_SWAPPED);

  picman_help_connect (GTK_WIDGET (chooser), picman_standard_help_func,
                     PICMAN_HELP_GRADIENT_SAVE_AS_POV, NULL);

  gtk_widget_show (GTK_WIDGET (chooser));
}


/*  private functions  */

static void
gradients_save_as_pov_ray_response (GtkWidget    *dialog,
                                    gint          response_id,
                                    PicmanGradient *gradient)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      const gchar *filename;
      GError      *error = NULL;

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

      if (! picman_gradient_save_pov (gradient, filename, &error))
        {
          picman_message_literal (g_object_get_data (G_OBJECT (dialog), "picman"),
				G_OBJECT (dialog), PICMAN_MESSAGE_ERROR,
				error->message);
          g_clear_error (&error);
          return;
        }
    }

  gtk_widget_destroy (dialog);
}
