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

#include "dialogs-types.h"

#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanitemstack.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "image-merge-layers-dialog.h"

#include "picman-intl.h"


static void  image_merge_layers_dialog_free (ImageMergeLayersDialog *dialog);


ImageMergeLayersDialog *
image_merge_layers_dialog_new (PicmanImage     *image,
                               PicmanContext   *context,
                               GtkWidget     *parent,
                               PicmanMergeType  merge_type,
                               gboolean       merge_active_group,
                               gboolean       discard_invisible)
{
  ImageMergeLayersDialog *dialog;
  GtkWidget              *vbox;
  GtkWidget              *frame;
  GtkWidget              *button;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  dialog = g_slice_new0 (ImageMergeLayersDialog);

  dialog->image              = image;
  dialog->context            = context;
  dialog->merge_type         = PICMAN_EXPAND_AS_NECESSARY;
  dialog->merge_active_group = merge_active_group;
  dialog->discard_invisible  = discard_invisible;

  dialog->dialog =
    picman_viewable_dialog_new (PICMAN_VIEWABLE (image), context,
                              _("Merge Layers"), "picman-image-merge-layers",
                              PICMAN_STOCK_MERGE_DOWN,
                              _("Layers Merge Options"),
                              parent,
                              picman_standard_help_func,
                              PICMAN_HELP_IMAGE_MERGE_LAYERS,

                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              _("_Merge"),      GTK_RESPONSE_OK,

                              NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) image_merge_layers_dialog_free, dialog);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame = picman_int_radio_group_new (TRUE, _("Final, Merged Layer should be:"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &dialog->merge_type, dialog->merge_type,

                                    _("Expanded as necessary"),
                                    PICMAN_EXPAND_AS_NECESSARY, NULL,

                                    _("Clipped to image"),
                                    PICMAN_CLIP_TO_IMAGE, NULL,

                                    _("Clipped to bottom layer"),
                                    PICMAN_CLIP_TO_BOTTOM_LAYER, NULL,

                                    NULL);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  button = gtk_check_button_new_with_mnemonic (_("Merge within active _group only"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                dialog->merge_active_group);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->merge_active_group);

  if (picman_item_stack_is_flat (PICMAN_ITEM_STACK (picman_image_get_layers (image))))
    gtk_widget_set_sensitive (button, FALSE);

  button = gtk_check_button_new_with_mnemonic (_("_Discard invisible layers"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                dialog->discard_invisible);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->discard_invisible);


  return dialog;
}

static void
image_merge_layers_dialog_free (ImageMergeLayersDialog *dialog)
{
  g_slice_free (ImageMergeLayersDialog, dialog);
}
