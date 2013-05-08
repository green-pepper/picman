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
#include "core/picmanitem.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmansizebox.h"
#include "widgets/picmanviewabledialog.h"

#include "scale-dialog.h"

#include "picman-intl.h"


#define RESPONSE_RESET 1


typedef struct
{
  PicmanViewable          *viewable;
  PicmanUnit               unit;
  PicmanInterpolationType  interpolation;
  GtkWidget             *box;
  GtkWidget             *combo;
  PicmanScaleCallback      callback;
  gpointer               user_data;
} ScaleDialog;


static void   scale_dialog_response (GtkWidget   *dialog,
                                     gint         response_id,
                                     ScaleDialog *private);
static void   scale_dialog_reset    (ScaleDialog *private);
static void   scale_dialog_free     (ScaleDialog *private);


GtkWidget *
scale_dialog_new (PicmanViewable          *viewable,
                  PicmanContext           *context,
                  const gchar           *title,
                  const gchar           *role,
                  GtkWidget             *parent,
                  PicmanHelpFunc           help_func,
                  const gchar           *help_id,
                  PicmanUnit               unit,
                  PicmanInterpolationType  interpolation,
                  PicmanScaleCallback      callback,
                  gpointer               user_data)
{
  GtkWidget   *dialog;
  GtkWidget   *vbox;
  GtkWidget   *hbox;
  GtkWidget   *frame;
  GtkWidget   *label;
  ScaleDialog *private;
  PicmanImage   *image = NULL;
  const gchar *text  = NULL;
  gint         width, height;
  gdouble      xres, yres;

  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (callback != NULL, NULL);

  if (PICMAN_IS_IMAGE (viewable))
    {
      image = PICMAN_IMAGE (viewable);

      width  = picman_image_get_width (image);
      height = picman_image_get_height (image);

      text = _("Image Size");
    }
  else if (PICMAN_IS_ITEM (viewable))
    {
      PicmanItem *item = PICMAN_ITEM (viewable);

      image = picman_item_get_image (item);

      width  = picman_item_get_width  (item);
      height = picman_item_get_height (item);

      text = _("Layer Size");
    }
  else
    {
      g_return_val_if_reached (NULL);
    }

  dialog = picman_viewable_dialog_new (viewable, context,
                                     title, role, PICMAN_STOCK_SCALE, title,
                                     parent,
                                     help_func, help_id,

                                     PICMAN_STOCK_RESET, RESPONSE_RESET,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     PICMAN_STOCK_SCALE, GTK_RESPONSE_OK,

                                     NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  private = g_slice_new0 (ScaleDialog);

  g_object_weak_ref (G_OBJECT (dialog),
                     (GWeakNotify) scale_dialog_free, private);

  private->viewable      = viewable;
  private->interpolation = interpolation;
  private->unit          = unit;
  private->callback      = callback;
  private->user_data     = user_data;

  picman_image_get_resolution (image, &xres, &yres);

  private->box = g_object_new (PICMAN_TYPE_SIZE_BOX,
                               "width",           width,
                               "height",          height,
                               "unit",            unit,
                               "xresolution",     xres,
                               "yresolution",     yres,
                               "resolution-unit", picman_image_get_unit (image),
                               "keep-aspect",     TRUE,
                               "edit-resolution", PICMAN_IS_IMAGE (viewable),
                               NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (scale_dialog_response),
                    private);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  frame = picman_frame_new (text);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_container_add (GTK_CONTAINER (frame), private->box);
  gtk_widget_show (private->box);

  frame = picman_frame_new (_("Quality"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("I_nterpolation:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  gtk_size_group_add_widget (PICMAN_SIZE_BOX (private->box)->size_group, label);

  private->combo = picman_enum_combo_box_new (PICMAN_TYPE_INTERPOLATION_TYPE);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), private->combo);
  gtk_box_pack_start (GTK_BOX (hbox), private->combo, TRUE, TRUE, 0);
  gtk_widget_show (private->combo);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (private->combo),
                                 private->interpolation);

  if (picman_image_get_base_type (image) == PICMAN_INDEXED)
    {
      GtkWidget *box = picman_message_box_new (PICMAN_STOCK_INFO);

      picman_message_box_set_text (PICMAN_MESSAGE_BOX (box),
                                 _("Indexed color layers are always scaled "
                                   "without interpolation. The chosen "
                                   "interpolation type will affect channels "
                                   "and layer masks only."));

      gtk_container_set_border_width (GTK_CONTAINER (box), 0);
      gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);
      gtk_widget_show (box);
    }

  return dialog;
}

static void
scale_dialog_response (GtkWidget   *dialog,
                       gint         response_id,
                       ScaleDialog *private)
{
  PicmanUnit  unit          = private->unit;
  gint      interpolation = private->interpolation;
  PicmanUnit  resolution_unit;
  gint      width, height;
  gdouble   xres, yres;

  switch (response_id)
    {
    case RESPONSE_RESET:
      scale_dialog_reset (private);
      break;

    case GTK_RESPONSE_OK:
      g_object_get (private->box,
                    "width",           &width,
                    "height",          &height,
                    "unit",            &unit,
                    "xresolution",     &xres,
                    "yresolution",     &yres,
                    "resolution-unit", &resolution_unit,
                    NULL);

      picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (private->combo),
                                     &interpolation);

      private->callback (dialog,
                         private->viewable,
                         width, height, unit, interpolation,
                         xres, yres, resolution_unit,
                         private->user_data);
      break;

    default:
      gtk_widget_destroy (dialog);
      break;
    }
}

static void
scale_dialog_reset (ScaleDialog *private)
{
  PicmanImage *image;
  gint       width, height;
  gdouble    xres, yres;

  if (PICMAN_IS_IMAGE (private->viewable))
    {
      image = PICMAN_IMAGE (private->viewable);

      width  = picman_image_get_width (image);
      height = picman_image_get_height (image);
    }
  else if (PICMAN_IS_ITEM (private->viewable))
    {
      PicmanItem *item = PICMAN_ITEM (private->viewable);

      image = picman_item_get_image (item);

      width  = picman_item_get_width  (item);
      height = picman_item_get_height (item);
    }
  else
    {
      g_return_if_reached ();
    }

  picman_image_get_resolution (image, &xres, &yres);

  g_object_set (private->box,
                "keep-aspect",     FALSE,
                NULL);

  g_object_set (private->box,
                "width",           width,
                "height",          height,
                "unit",            private->unit,
                NULL);

  g_object_set (private->box,
                "keep-aspect",     TRUE,
                "xresolution",     xres,
                "yresolution",     yres,
                "resolution-unit", picman_image_get_unit (image),
                NULL);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (private->combo),
                                 private->interpolation);
}

static void
scale_dialog_free (ScaleDialog *private)
{
  g_slice_free (ScaleDialog, private);
}
