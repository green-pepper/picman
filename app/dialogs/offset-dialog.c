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

#include "dialogs-types.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmandrawable.h"
#include "core/picmandrawable-offset.h"
#include "core/picmanitem.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmanimage.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "offset-dialog.h"

#include "picman-intl.h"


#define WRAP_AROUND  (1 << 3)
#define FILL_MASK    (PICMAN_OFFSET_BACKGROUND | PICMAN_OFFSET_TRANSPARENT)


typedef struct
{
  PicmanContext    *context;

  GtkWidget      *dialog;
  GtkWidget      *off_se;

  PicmanOffsetType  fill_type;

  PicmanImage      *image;
} OffsetDialog;


/*  local function prototypes  */

static void  offset_response         (GtkWidget    *widget,
                                      gint          response_id,
                                      OffsetDialog *dialog);
static void  offset_half_xy_callback (GtkWidget    *widget,
                                      OffsetDialog *dialog);
static void  offset_half_x_callback  (GtkWidget    *widget,
                                      OffsetDialog *dialog);
static void  offset_half_y_callback  (GtkWidget    *widget,
                                      OffsetDialog *dialog);
static void  offset_dialog_free      (OffsetDialog *dialog);


/*  public functions  */

GtkWidget *
offset_dialog_new (PicmanDrawable *drawable,
                   PicmanContext  *context,
                   GtkWidget    *parent)
{
  PicmanItem     *item;
  OffsetDialog *dialog;
  GtkWidget    *main_vbox;
  GtkWidget    *vbox;
  GtkWidget    *hbox;
  GtkWidget    *button;
  GtkWidget    *spinbutton;
  GtkWidget    *frame;
  GtkWidget    *radio_button;
  GtkObject    *adjustment;
  gdouble       xres;
  gdouble       yres;
  const gchar  *title = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);

  dialog = g_slice_new0 (OffsetDialog);

  dialog->context   = context;
  dialog->fill_type = picman_drawable_has_alpha (drawable) | WRAP_AROUND;
  item = PICMAN_ITEM (drawable);
  dialog->image     = picman_item_get_image (item);

  picman_image_get_resolution (dialog->image, &xres, &yres);

  if (PICMAN_IS_LAYER (drawable))
    title = _("Offset Layer");
  else if (PICMAN_IS_LAYER_MASK (drawable))
    title = _("Offset Layer Mask");
  else if (PICMAN_IS_CHANNEL (drawable))
    title = _("Offset Channel");
  else
    g_warning ("%s: unexpected drawable type", G_STRFUNC);

  dialog->dialog =
    picman_viewable_dialog_new (PICMAN_VIEWABLE (drawable), context,
                              _("Offset"), "picman-drawable-offset",
                              PICMAN_STOCK_TOOL_MOVE,
                              title,
                              parent,
                              picman_standard_help_func,
                              PICMAN_HELP_LAYER_OFFSET,

                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                              /*  offset, used as a verb  */
                              _("_Offset"),     GTK_RESPONSE_OK,

                              NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) offset_dialog_free, dialog);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (offset_response),
                    dialog);

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  /*  The offset frame  */
  frame = picman_frame_new (_("Offset"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  spinbutton = picman_spin_button_new (&adjustment,
                                     1, 1, 1, 1, 10, 0,
                                     1, 2);
  gtk_entry_set_width_chars (GTK_ENTRY (spinbutton), 10);

  dialog->off_se = picman_size_entry_new (1, PICMAN_UNIT_PIXEL, "%a",
                                        TRUE, TRUE, FALSE, 10,
                                        PICMAN_SIZE_ENTRY_UPDATE_SIZE);

  gtk_table_set_col_spacing (GTK_TABLE (dialog->off_se), 0, 4);
  gtk_table_set_col_spacing (GTK_TABLE (dialog->off_se), 1, 4);
  gtk_table_set_row_spacing (GTK_TABLE (dialog->off_se), 0, 2);

  picman_size_entry_add_field (PICMAN_SIZE_ENTRY (dialog->off_se),
                             GTK_SPIN_BUTTON (spinbutton), NULL);
  gtk_table_attach_defaults (GTK_TABLE (dialog->off_se), spinbutton,
                             1, 2, 0, 1);
  gtk_widget_show (spinbutton);

  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (dialog->off_se),
                                _("_X:"), 0, 0, 0.0);
  picman_size_entry_attach_label (PICMAN_SIZE_ENTRY (dialog->off_se),
                                _("_Y:"), 1, 0, 0.0);

  gtk_box_pack_start (GTK_BOX (vbox), dialog->off_se, FALSE, FALSE, 0);
  gtk_widget_show (dialog->off_se);

  picman_size_entry_set_unit (PICMAN_SIZE_ENTRY (dialog->off_se), PICMAN_UNIT_PIXEL);

  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (dialog->off_se), 0,
                                  xres, FALSE);
  picman_size_entry_set_resolution (PICMAN_SIZE_ENTRY (dialog->off_se), 1,
                                  yres, FALSE);

  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (dialog->off_se), 0,
                                         - picman_item_get_width (item),
                                         picman_item_get_width (item));
  picman_size_entry_set_refval_boundaries (PICMAN_SIZE_ENTRY (dialog->off_se), 1,
                                         - picman_item_get_height (item),
                                         picman_item_get_height (item));

  picman_size_entry_set_size (PICMAN_SIZE_ENTRY (dialog->off_se), 0,
                            0, picman_item_get_width (item));
  picman_size_entry_set_size (PICMAN_SIZE_ENTRY (dialog->off_se), 1,
                            0, picman_item_get_height (item));

  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se), 0, 0);
  picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se), 1, 0);

  button = gtk_button_new_with_mnemonic (_("By width/_2, height/2"));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (offset_half_xy_callback),
                    dialog);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  button = gtk_button_new_with_mnemonic ("By _width/2");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (offset_half_x_callback),
                    dialog);

  button = gtk_button_new_with_mnemonic ("By _height/2");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  g_signal_connect (button, "clicked",
                    G_CALLBACK (offset_half_y_callback),
                    dialog);

  /*  The edge behavior frame  */
  frame = picman_int_radio_group_new (TRUE, _("Edge Behavior"),
                                    G_CALLBACK (picman_radio_button_update),
                                    &dialog->fill_type, dialog->fill_type,

                                    _("W_rap around"),
                                    WRAP_AROUND, NULL,

                                    _("Fill with _background color"),
                                    PICMAN_OFFSET_BACKGROUND, NULL,

                                    _("Make _transparent"),
                                    PICMAN_OFFSET_TRANSPARENT, &radio_button,
                                    NULL);

  if (! picman_drawable_has_alpha (drawable))
    gtk_widget_set_sensitive (radio_button, FALSE);

  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  return dialog->dialog;
}


/*  private functions  */

static void
offset_response (GtkWidget    *widget,
                 gint          response_id,
                 OffsetDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanImage *image = dialog->image;

      if (image)
        {
          PicmanDrawable *drawable = picman_image_get_active_drawable (image);
          gint          offset_x;
          gint          offset_y;

          offset_x =
            RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                              0));
          offset_y =
            RINT (picman_size_entry_get_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                              1));

          picman_drawable_offset (drawable,
                                dialog->context,
                                dialog->fill_type & WRAP_AROUND ? TRUE : FALSE,
                                dialog->fill_type & FILL_MASK,
                                offset_x, offset_y);
          picman_image_flush (image);
        }
    }

  gtk_widget_destroy (dialog->dialog);
}

static void
offset_half_xy_callback (GtkWidget    *widget,
                         OffsetDialog *dialog)
{
  PicmanImage *image = dialog->image;

  if (image)
    {
      PicmanItem *item = PICMAN_ITEM (picman_image_get_active_drawable (image));

      picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                  0, picman_item_get_width (item) / 2);
      picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                  1, picman_item_get_height (item) / 2);
   }
}

static void
offset_half_x_callback (GtkWidget    *widget,
                        OffsetDialog *dialog)
{
  PicmanImage *image = dialog->image;

  if (image)
    {
      PicmanItem *item = PICMAN_ITEM (picman_image_get_active_drawable (image));

      picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                  0, picman_item_get_width (item) / 2);
   }
}

static void
offset_half_y_callback (GtkWidget    *widget,
                        OffsetDialog *dialog)
{
  PicmanImage *image = dialog->image;

  if (image)
    {
      PicmanItem *item = PICMAN_ITEM (picman_image_get_active_drawable (image));

      picman_size_entry_set_refval (PICMAN_SIZE_ENTRY (dialog->off_se),
                                  1, picman_item_get_height (item) / 2);
   }
}

static void
offset_dialog_free (OffsetDialog *dialog)
{
  g_slice_free (OffsetDialog, dialog);
}
