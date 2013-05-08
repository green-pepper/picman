/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-grid.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmangrid.h"

#include "widgets/picmangrideditor.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "grid-dialog.h"

#include "picman-intl.h"


#define GRID_RESPONSE_RESET 1


/*  local functions  */

static void  grid_dialog_response (GtkWidget *widget,
                                   gint       response_id,
                                   GtkWidget *dialog);


/*  public function  */


GtkWidget *
grid_dialog_new (PicmanImage   *image,
                 PicmanContext *context,
                 GtkWidget   *parent)
{
  PicmanGrid  *grid;
  PicmanGrid  *grid_backup;
  GtkWidget *dialog;
  GtkWidget *editor;
  gdouble    xres;
  gdouble    yres;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (parent == NULL || GTK_IS_WIDGET (parent), NULL);

  picman_image_get_resolution (image, &xres, &yres);

  grid = picman_image_get_grid (PICMAN_IMAGE (image));
  grid_backup = picman_config_duplicate (PICMAN_CONFIG (grid));

  dialog = picman_viewable_dialog_new (PICMAN_VIEWABLE (image), context,
                                     _("Configure Grid"), "picman-grid-configure",
                                     PICMAN_STOCK_GRID, _("Configure Image Grid"),
                                     parent,
                                     picman_standard_help_func,
                                     PICMAN_HELP_IMAGE_GRID,

                                     PICMAN_STOCK_RESET, GRID_RESPONSE_RESET,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_OK,     GTK_RESPONSE_OK,

                                     NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GRID_RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (grid_dialog_response),
                    dialog);

  editor = picman_grid_editor_new (grid, context, xres, yres);
  gtk_container_set_border_width (GTK_CONTAINER (editor), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      editor, TRUE, TRUE, 0);

  gtk_widget_show (editor);

  g_object_set_data (G_OBJECT (dialog), "image", image);
  g_object_set_data (G_OBJECT (dialog), "grid", grid);

  g_object_set_data_full (G_OBJECT (dialog), "grid-backup", grid_backup,
                          (GDestroyNotify) g_object_unref);

  return dialog;
}


/*  local functions  */

static void
grid_dialog_response (GtkWidget  *widget,
                      gint        response_id,
                      GtkWidget  *dialog)
{
  PicmanImage *image;
  PicmanImage *grid;
  PicmanGrid  *grid_backup;

  image       = g_object_get_data (G_OBJECT (dialog), "image");
  grid        = g_object_get_data (G_OBJECT (dialog), "grid");
  grid_backup = g_object_get_data (G_OBJECT (dialog), "grid-backup");

  switch (response_id)
    {
    case GRID_RESPONSE_RESET:
      picman_config_sync (G_OBJECT (image->picman->config->default_grid),
                        G_OBJECT (grid), 0);
      break;

    case GTK_RESPONSE_OK:
      if (! picman_config_is_equal_to (PICMAN_CONFIG (grid_backup),
                                     PICMAN_CONFIG (grid)))
        {
          picman_image_undo_push_image_grid (image, _("Grid"), grid_backup);
        }

      gtk_widget_destroy (dialog);
      break;

    default:
      picman_image_set_grid (PICMAN_IMAGE (image), grid_backup, FALSE);
      gtk_widget_destroy (dialog);
    }
}
