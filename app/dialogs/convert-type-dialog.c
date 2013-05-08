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

#include "core/picman.h"
#include "core/picmancontainer-filter.h"
#include "core/picmancontext.h"
#include "core/picmandatafactory.h"
#include "core/picmanimage.h"
#include "core/picmanimage-convert-type.h"
#include "core/picmanlist.h"
#include "core/picmanpalette.h"
#include "core/picmanprogress.h"

#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewablebox.h"
#include "widgets/picmanviewabledialog.h"
#include "widgets/picmanwidgets-utils.h"

#include "convert-type-dialog.h"

#include "picman-intl.h"


typedef struct
{
  GtkWidget              *dialog;

  PicmanImage              *image;
  PicmanProgress           *progress;
  PicmanContext            *context;
  PicmanContainer          *container;
  PicmanPalette            *custom_palette;

  PicmanConvertDitherType   dither_type;
  gboolean                alpha_dither;
  gboolean                text_layer_dither;
  gboolean                remove_dups;
  gint                    num_colors;
  PicmanConvertPaletteType  palette_type;
} IndexedDialog;


static void        convert_dialog_response        (GtkWidget        *widget,
                                                   gint              response_id,
                                                   IndexedDialog    *dialog);
static GtkWidget * convert_dialog_palette_box     (IndexedDialog    *dialog);
static gboolean    convert_dialog_palette_filter  (const PicmanObject *object,
                                                   gpointer          user_data);
static void        convert_dialog_palette_changed (PicmanContext      *context,
                                                   PicmanPalette      *palette,
                                                   IndexedDialog    *dialog);
static void        convert_dialog_free            (IndexedDialog    *dialog);


/*  defaults  */

static PicmanConvertDitherType   saved_dither_type       = PICMAN_NO_DITHER;
static gboolean                saved_alpha_dither      = FALSE;
static gboolean                saved_text_layer_dither = FALSE;
static gboolean                saved_remove_dups       = TRUE;
static gint                    saved_num_colors        = 256;
static PicmanConvertPaletteType  saved_palette_type      = PICMAN_MAKE_PALETTE;
static PicmanPalette            *saved_palette           = NULL;


/*  public functions  */

GtkWidget *
convert_type_dialog_new (PicmanImage    *image,
                         PicmanContext  *context,
                         GtkWidget    *parent,
                         PicmanProgress *progress)
{
  IndexedDialog *dialog;
  GtkWidget     *button;
  GtkWidget     *main_vbox;
  GtkWidget     *vbox;
  GtkWidget     *hbox;
  GtkWidget     *label;
  GtkObject     *adjustment;
  GtkWidget     *spinbutton;
  GtkWidget     *frame;
  GtkWidget     *toggle;
  GtkWidget     *palette_box;
  GtkWidget     *combo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);

  dialog = g_slice_new0 (IndexedDialog);

  dialog->image             = image;
  dialog->progress          = progress;
  dialog->dither_type       = saved_dither_type;
  dialog->alpha_dither      = saved_alpha_dither;
  dialog->text_layer_dither = saved_text_layer_dither;
  dialog->remove_dups       = saved_remove_dups;
  dialog->num_colors        = saved_num_colors;
  dialog->palette_type      = saved_palette_type;

  dialog->dialog =
    picman_viewable_dialog_new (PICMAN_VIEWABLE (image), context,
                              _("Indexed Color Conversion"),
                              "picman-image-convert-indexed",
                              PICMAN_STOCK_CONVERT_INDEXED,
                              _("Convert Image to Indexed Colors"),
                              parent,
                              picman_standard_help_func,
                              PICMAN_HELP_IMAGE_CONVERT_INDEXED,

                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,

                              NULL);

  button = gtk_dialog_add_button (GTK_DIALOG (dialog->dialog),
                                  _("C_onvert"), GTK_RESPONSE_OK);
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_stock (PICMAN_STOCK_CONVERT_INDEXED,
                                                  GTK_ICON_SIZE_BUTTON));

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog->dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  gtk_window_set_resizable (GTK_WINDOW (dialog->dialog), FALSE);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) convert_dialog_free, dialog);

  g_signal_connect (dialog->dialog, "response",
                    G_CALLBACK (convert_dialog_response),
                    dialog);

  palette_box = convert_dialog_palette_box (dialog);

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog->dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);


  /*  palette  */

  frame =
    picman_enum_radio_frame_new_with_range (PICMAN_TYPE_CONVERT_PALETTE_TYPE,
                                          PICMAN_MAKE_PALETTE,
                                          (palette_box ?
                                           PICMAN_CUSTOM_PALETTE :
                                           PICMAN_MONO_PALETTE),
                                          gtk_label_new (_("Colormap")),
                                          G_CALLBACK (picman_radio_button_update),
                                          &dialog->palette_type,
                                          &button);

  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (button),
                                   dialog->palette_type);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  /*  max n_colors  */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  picman_enum_radio_frame_add (GTK_FRAME (frame), hbox,
                             PICMAN_MAKE_PALETTE, TRUE);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("_Maximum number of colors:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  if (dialog->num_colors == 256 && picman_image_has_alpha (image))
    dialog->num_colors = 255;

  spinbutton = picman_spin_button_new (&adjustment, dialog->num_colors,
                                     2, 256, 1, 8, 0, 1, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), spinbutton);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_int_adjustment_update),
                    &dialog->num_colors);

  /*  custom palette  */
  if (palette_box)
    {
      picman_enum_radio_frame_add (GTK_FRAME (frame), palette_box,
                                 PICMAN_CUSTOM_PALETTE, TRUE);
      gtk_widget_show (palette_box);
    }

  vbox = gtk_bin_get_child (GTK_BIN (frame));

  toggle = gtk_check_button_new_with_mnemonic (_("_Remove unused colors "
                                                 "from colormap"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                dialog->remove_dups);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 3);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->remove_dups);

  g_object_bind_property (button, "active",
                          toggle, "sensitive",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  /*  dithering  */

  frame = picman_frame_new (_("Dithering"));
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Color _dithering:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = picman_enum_combo_box_new (PICMAN_TYPE_CONVERT_DITHER_TYPE);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 0);
  gtk_widget_show (combo);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (combo),
                              dialog->dither_type,
                              G_CALLBACK (picman_int_combo_box_get_active),
                              &dialog->dither_type);

  toggle =
    gtk_check_button_new_with_mnemonic (_("Enable dithering of _transparency"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                dialog->alpha_dither);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->alpha_dither);


  toggle =
    gtk_check_button_new_with_mnemonic (_("Enable dithering of text layers"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                dialog->text_layer_dither);
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (picman_toggle_button_update),
                    &dialog->text_layer_dither);

  picman_help_set_help_data (toggle,
                           _("Dithering text layers will make them uneditable"),
                           NULL);

  return dialog->dialog;
}


/*  private functions  */

static void
convert_dialog_response (GtkWidget     *widget,
                         gint           response_id,
                         IndexedDialog *dialog)
{
  if (response_id == GTK_RESPONSE_OK)
    {
      PicmanProgress *progress;
      GError       *error = NULL;

      progress = picman_progress_start (dialog->progress,
                                      _("Converting to indexed colors"), FALSE);

      /*  Convert the image to indexed color  */
      if (! picman_image_convert_type (dialog->image,
                                     PICMAN_INDEXED,
                                     dialog->num_colors,
                                     dialog->dither_type,
                                     dialog->alpha_dither,
                                     dialog->text_layer_dither,
                                     dialog->remove_dups,
                                     dialog->palette_type,
                                     dialog->custom_palette,
                                     progress, &error))
        {
          picman_message_literal (dialog->image->picman, G_OBJECT (dialog->dialog),
				PICMAN_MESSAGE_WARNING, error->message);
          g_clear_error (&error);

          if (progress)
            picman_progress_end (progress);

          return;
        }

      if (progress)
        picman_progress_end (progress);

      picman_image_flush (dialog->image);

      /* Save defaults for next time */
      saved_dither_type       = dialog->dither_type;
      saved_alpha_dither      = dialog->alpha_dither;
      saved_text_layer_dither = dialog->text_layer_dither;
      saved_remove_dups       = dialog->remove_dups;
      saved_num_colors        = dialog->num_colors;
      saved_palette_type      = dialog->palette_type;
      saved_palette           = dialog->custom_palette;
    }

  gtk_widget_destroy (dialog->dialog);
}

static GtkWidget *
convert_dialog_palette_box (IndexedDialog *dialog)
{
  Picman        *picman = dialog->image->picman;
  GList       *list;
  PicmanPalette *palette;
  PicmanPalette *web_palette   = NULL;
  gboolean     default_found = FALSE;

  /* We can't dither to > 256 colors */
  dialog->container = picman_container_filter (picman_data_factory_get_container (picman->palette_factory),
                                             convert_dialog_palette_filter,
                                             NULL);

  if (picman_container_is_empty (dialog->container))
    {
      g_object_unref (dialog->container);
      dialog->container = NULL;
      return NULL;
    }

  dialog->context = picman_context_new (picman, "convert-dialog", NULL);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) g_object_unref, dialog->context);

  g_object_weak_ref (G_OBJECT (dialog->dialog),
                     (GWeakNotify) g_object_unref, dialog->container);

  for (list = PICMAN_LIST (dialog->container)->list;
       list;
       list = g_list_next (list))
    {
      palette = list->data;

      /* Preferentially, the initial default is 'Web' if available */
      if (web_palette == NULL &&
          g_ascii_strcasecmp (picman_object_get_name (palette), "Web") == 0)
        {
          web_palette = palette;
        }

      if (saved_palette == palette)
        {
          dialog->custom_palette = saved_palette;
          default_found = TRUE;
        }
    }

  if (! default_found)
    {
      if (web_palette)
        dialog->custom_palette = web_palette;
      else
        dialog->custom_palette = PICMAN_LIST (dialog->container)->list->data;
    }

  picman_context_set_palette (dialog->context, dialog->custom_palette);

  g_signal_connect (dialog->context, "palette-changed",
                    G_CALLBACK (convert_dialog_palette_changed),
                    dialog);

  return picman_palette_box_new (dialog->container, dialog->context, NULL, 4);
}

static gboolean
convert_dialog_palette_filter (const PicmanObject *object,
                               gpointer          user_data)
{
  PicmanPalette *palette = PICMAN_PALETTE (object);

  return (picman_palette_get_n_colors (palette) > 0 &&
          picman_palette_get_n_colors (palette) <= 256);
}

static void
convert_dialog_palette_changed (PicmanContext   *context,
                                PicmanPalette   *palette,
                                IndexedDialog *dialog)
{
  if (! palette)
    return;

  if (picman_palette_get_n_colors (palette) > 256)
    {
      picman_message (dialog->image->picman, G_OBJECT (dialog->dialog),
                    PICMAN_MESSAGE_WARNING,
                    _("Cannot convert to a palette "
                      "with more than 256 colors."));
    }
  else
    {
      dialog->custom_palette = palette;
    }
}

static void
convert_dialog_free (IndexedDialog *dialog)
{
  g_slice_free (IndexedDialog, dialog);
}
