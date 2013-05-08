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

#include "core/picmanchannel.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanlayer.h"

#include "widgets/picmancontainercombobox.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"
#include "widgets/picmanwidgets-utils.h"

#include "layer-add-mask-dialog.h"

#include "picman-intl.h"


/*  local function prototypes  */

static gboolean   layer_add_mask_dialog_channel_selected (PicmanContainerView  *view,
                                                          PicmanViewable       *viewable,
                                                          gpointer            insert_data,
                                                          LayerAddMaskDialog *dialog);
static void       layer_add_mask_dialog_free             (LayerAddMaskDialog *dialog);


/*  public functions  */

LayerAddMaskDialog *
layer_add_mask_dialog_new (PicmanLayer       *layer,
                           PicmanContext     *context,
                           GtkWidget       *parent,
                           PicmanAddMaskType  add_mask_type,
                           gboolean         invert)
{
  LayerAddMaskDialog *dialog;
  GtkWidget          *vbox;
  GtkWidget          *frame;
  GtkWidget          *combo;
  GtkWidget          *button;
  PicmanImage          *image;
  PicmanChannel        *channel;

  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  dialog = g_slice_new0 (LayerAddMaskDialog);

  dialog->layer         = layer;
  dialog->add_mask_type = add_mask_type;
  dialog->invert        = invert;

  dialog->dialog =
    picman_viewable_dialog_new (PICMAN_VIEWABLE (layer), context,
                              _("Add Layer Mask"), "picman-layer-add-mask",
                              PICMAN_STOCK_LAYER_MASK,
                              _("Add a Mask to the Layer"),
                              parent,
                              picman_standard_help_func,
                              PICMAN_HELP_LAYER_MASK_ADD,

                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              GTK_STOCK_ADD,    GTK_RESPONSE_OK,

                              NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) layer_add_mask_dialog_free, dialog);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame =
    picman_enum_radio_frame_new (PICMAN_TYPE_ADD_MASK_TYPE,
                               gtk_label_new (_("Initialize Layer Mask to:")),
                               G_CALLBACK (picman_radio_button_update),
                               &dialog->add_mask_type,
                               &button);
  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (button),
                                   dialog->add_mask_type);

  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  image = picman_item_get_image (PICMAN_ITEM (layer));

  combo = picman_container_combo_box_new (picman_image_get_channels (image),
                                        context,
                                        PICMAN_VIEW_SIZE_SMALL, 1);
  picman_enum_radio_frame_add (GTK_FRAME (frame), combo,
                             PICMAN_ADD_CHANNEL_MASK, TRUE);
  gtk_widget_show (combo);

  g_signal_connect (combo, "select-item",
                    G_CALLBACK (layer_add_mask_dialog_channel_selected),
                    dialog);

  channel = picman_image_get_active_channel (image);

  if (! channel)
    channel = PICMAN_CHANNEL (picman_container_get_first_child (picman_image_get_channels (image)));

  picman_container_view_select_item (PICMAN_CONTAINER_VIEW (combo),
                                   PICMAN_VIEWABLE (channel));

  button = gtk_check_button_new_with_mnemonic (_("In_vert mask"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), dialog->invert);
  gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->invert);

  return dialog;
}


/*  private functions  */

static gboolean
layer_add_mask_dialog_channel_selected (PicmanContainerView  *view,
                                        PicmanViewable       *viewable,
                                        gpointer            insert_data,
                                        LayerAddMaskDialog *dialog)
{
  dialog->channel = PICMAN_CHANNEL (viewable);

  return TRUE;
}

static void
layer_add_mask_dialog_free (LayerAddMaskDialog *dialog)
{
  g_slice_free (LayerAddMaskDialog, dialog);
}
