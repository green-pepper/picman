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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmanimage.h"
#include "core/picmanpaintinfo.h"
#include "core/picmanstrokeoptions.h"
#include "core/picmantoolinfo.h"

#include "widgets/picmancontainercombobox.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmanviewabledialog.h"
#include "widgets/picmanstrokeeditor.h"

#include "stroke-dialog.h"

#include "picman-intl.h"


#define RESPONSE_RESET 1


/*  local functions  */

static void  stroke_dialog_response (GtkWidget *widget,
                                     gint       response_id,
                                     GtkWidget *dialog);


/*  public function  */

GtkWidget *
stroke_dialog_new (PicmanItem    *item,
                   PicmanContext *context,
                   const gchar *title,
                   const gchar *stock_id,
                   const gchar *help_id,
                   GtkWidget   *parent)
{
  PicmanStrokeOptions *options;
  PicmanStrokeOptions *saved_options;
  PicmanImage         *image;
  GtkWidget         *dialog;
  GtkWidget         *main_vbox;
  GtkWidget         *radio_box;
  GtkWidget         *cairo_radio;
  GtkWidget         *paint_radio;
  GSList            *group;
  GtkWidget         *frame;

  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);
  g_return_val_if_fail (help_id != NULL, NULL);
  g_return_val_if_fail (parent == NULL || GTK_IS_WIDGET (parent), NULL);

  image = picman_item_get_image (item);

  options = picman_stroke_options_new (context->picman, context, TRUE);

  saved_options = g_object_get_data (G_OBJECT (context->picman),
                                     "saved-stroke-options");

  if (saved_options)
    picman_config_sync (G_OBJECT (saved_options), G_OBJECT (options), 0);

  dialog = picman_viewable_dialog_new (PICMAN_VIEWABLE (item), context,
                                     title, "picman-stroke-options",
                                     stock_id,
                                     _("Choose Stroke Style"),
                                     parent,
                                     picman_standard_help_func,
                                     help_id,

                                     PICMAN_STOCK_RESET, RESPONSE_RESET,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     stock_id,         GTK_RESPONSE_OK,

                                     NULL);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           RESPONSE_RESET,
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (stroke_dialog_response),
                    dialog);

  g_object_set_data (G_OBJECT (dialog), "picman-item", item);
  g_object_set_data_full (G_OBJECT (dialog), "picman-stroke-options", options,
                          (GDestroyNotify) g_object_unref);

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  radio_box = picman_prop_enum_radio_box_new (G_OBJECT (options), "method",
                                            -1, -1);

  group = gtk_radio_button_get_group (g_object_get_data (G_OBJECT (radio_box),
                                                         "radio-button"));

  cairo_radio = g_object_ref (group->next->data);
  gtk_container_remove (GTK_CONTAINER (radio_box), cairo_radio);

  paint_radio = g_object_ref (group->data);
  gtk_container_remove (GTK_CONTAINER (radio_box), paint_radio);

  g_object_ref_sink (radio_box);
  g_object_unref (radio_box);

  {
    PangoFontDescription *font_desc;

    font_desc = pango_font_description_new ();
    pango_font_description_set_weight (font_desc, PANGO_WEIGHT_BOLD);

    gtk_widget_modify_font (gtk_bin_get_child (GTK_BIN (cairo_radio)),
                            font_desc);
    gtk_widget_modify_font (gtk_bin_get_child (GTK_BIN (paint_radio)),
                            font_desc);

    pango_font_description_free (font_desc);
  }


  /*  the stroke frame  */

  frame = picman_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_frame_set_label_widget (GTK_FRAME (frame), cairo_radio);
  g_object_unref (cairo_radio);

  {
    GtkWidget *stroke_editor;
    gdouble    xres;
    gdouble    yres;

    picman_image_get_resolution (image, &xres, &yres);

    stroke_editor = picman_stroke_editor_new (options, yres, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), stroke_editor);
    gtk_widget_show (stroke_editor);

    g_object_bind_property (cairo_radio,   "active",
                            stroke_editor, "sensitive",
                            G_BINDING_SYNC_CREATE);
  }


  /*  the paint tool frame  */

  frame = picman_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  gtk_frame_set_label_widget (GTK_FRAME (frame), paint_radio);
  g_object_unref (paint_radio);

  {
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *button;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_widget_show (vbox);

    g_object_bind_property (paint_radio, "active",
                            vbox,        "sensitive",
                            G_BINDING_SYNC_CREATE);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    label = gtk_label_new (_("Paint tool:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    combo = picman_container_combo_box_new (image->picman->paint_info_list,
                                          PICMAN_CONTEXT (options),
                                          16, 0);
    gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
    gtk_widget_show (combo);

    g_object_set_data (G_OBJECT (dialog), "picman-tool-menu", combo);

    button = picman_prop_check_button_new (G_OBJECT (options),
                                         "emulate-brush-dynamics",
                                         _("_Emulate brush dynamics"));
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);
  }

  return dialog;
}


/*  private functions  */

static void
stroke_dialog_response (GtkWidget  *widget,
                        gint        response_id,
                        GtkWidget  *dialog)
{
  PicmanStrokeOptions *options;
  PicmanItem          *item;
  PicmanImage         *image;
  PicmanContext       *context;
  GtkWidget         *combo;

  item    = g_object_get_data (G_OBJECT (dialog), "picman-item");
  options = g_object_get_data (G_OBJECT (dialog), "picman-stroke-options");
  combo   = g_object_get_data (G_OBJECT (dialog), "picman-tool-menu");

  image   = picman_item_get_image (item);
  context = PICMAN_VIEWABLE_DIALOG (dialog)->context;

  switch (response_id)
    {
    case RESPONSE_RESET:
      {
        PicmanToolInfo *tool_info = picman_context_get_tool (context);

        picman_config_reset (PICMAN_CONFIG (options));

        picman_container_view_select_item (PICMAN_CONTAINER_VIEW (combo),
                                         PICMAN_VIEWABLE (tool_info->paint_info));

      }
      break;

    case GTK_RESPONSE_OK:
      {
        PicmanDrawable      *drawable = picman_image_get_active_drawable (image);
        PicmanStrokeOptions *saved_options;
        GError            *error    = NULL;

        if (! drawable)
          {
            picman_message_literal (context->picman, G_OBJECT (widget),
				  PICMAN_MESSAGE_WARNING,
				  _("There is no active layer or channel "
				    "to stroke to."));
            return;
          }

        saved_options = g_object_get_data (G_OBJECT (context->picman),
                                           "saved-stroke-options");

        if (saved_options)
          g_object_ref (saved_options);
        else
          saved_options = picman_stroke_options_new (context->picman, context, TRUE);

        picman_config_sync (G_OBJECT (options), G_OBJECT (saved_options), 0);

        g_object_set_data_full (G_OBJECT (context->picman), "saved-stroke-options",
                                saved_options,
                                (GDestroyNotify) g_object_unref);

        if (! picman_item_stroke (item, drawable, context, options, FALSE, TRUE,
                                NULL, &error))
          {
            picman_message_literal (context->picman,
                                  G_OBJECT (widget),
                                  PICMAN_MESSAGE_WARNING,
                                  error ? error->message : "NULL");

            g_clear_error (&error);
            return;
          }

        picman_image_flush (image);
      }
      /* fallthrough */

    default:
      gtk_widget_destroy (dialog);
      break;
    }
}
