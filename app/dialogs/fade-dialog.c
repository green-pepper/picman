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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "dialogs-types.h"

#include "core/picman-edit.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmandrawable.h"
#include "core/picmandrawableundo.h"
#include "core/picmanundostack.h"

#include "widgets/picmanpropwidgets.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanviewabledialog.h"

#include "fade-dialog.h"

#include "picman-intl.h"


typedef struct
{
  PicmanImage            *image;
  PicmanDrawable         *drawable;
  PicmanContext          *context;

  gboolean              applied;
  PicmanLayerModeEffects  orig_paint_mode;
  gdouble               orig_opacity;
} FadeDialog;


static void   fade_dialog_response        (GtkWidget  *dialog,
                                           gint        response_id,
                                           FadeDialog *private);

static void   fade_dialog_context_changed (FadeDialog *private);
static void   fade_dialog_free            (FadeDialog *private);


/*  public functions  */

GtkWidget *
fade_dialog_new (PicmanImage *image,
                 GtkWidget *parent)
{
  FadeDialog       *private;
  PicmanDrawableUndo *undo;
  PicmanDrawable     *drawable;
  PicmanItem         *item;

  GtkWidget        *dialog;
  GtkWidget        *main_vbox;
  GtkWidget        *table;
  GtkWidget        *menu;
  gchar            *title;
  gint              table_row = 0;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (parent), NULL);

  undo = PICMAN_DRAWABLE_UNDO (picman_image_undo_get_fadeable (image));

  if (! (undo && undo->applied_buffer))
    return NULL;

  item      = PICMAN_ITEM_UNDO (undo)->item;
  drawable  = PICMAN_DRAWABLE (item);

  private = g_slice_new0 (FadeDialog);

  private->image           = image;
  private->drawable        = drawable;
  private->context         = picman_context_new (image->picman,
                                               "fade-dialog", NULL);
  private->applied         = FALSE;
  private->orig_paint_mode = undo->paint_mode;
  private->orig_opacity    = undo->opacity;

  g_object_set (private->context,
                "paint-mode", undo->paint_mode,
                "opacity",    undo->opacity,
                NULL);

  title = g_strdup_printf (_("Fade %s"), picman_object_get_name (undo));


  dialog = picman_viewable_dialog_new (PICMAN_VIEWABLE (drawable),
                                     private->context,
                                     title, "picman-edit-fade",
                                     GTK_STOCK_UNDO, title,
                                     parent,
                                     picman_standard_help_func,
                                     PICMAN_HELP_EDIT_FADE,

                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     _("_Fade"),       GTK_RESPONSE_OK,

                                     NULL);

  g_free (title);

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  g_object_weak_ref (G_OBJECT (dialog),
                     (GWeakNotify) fade_dialog_free, private);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (fade_dialog_response),
                    private);

  main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (main_vbox), 12);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      main_vbox, TRUE, TRUE, 0);
  gtk_widget_show (main_vbox);

  table = gtk_table_new (3, 3, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, FALSE, FALSE, 0);
  gtk_widget_show (table);

  /*  the paint mode menu  */
  menu = picman_prop_paint_mode_menu_new (G_OBJECT (private->context),
                                        "paint-mode", TRUE, TRUE);
  picman_table_attach_aligned (GTK_TABLE (table), 0, table_row++,
                             _("_Mode:"), 0.0, 0.5,
                             menu, 2, FALSE);

  /*  the opacity scale  */
  picman_prop_opacity_entry_new (G_OBJECT (private->context), "opacity",
                               GTK_TABLE (table), 0, table_row++,
                               _("_Opacity:"));

  g_signal_connect_swapped (private->context, "paint-mode-changed",
                            G_CALLBACK (fade_dialog_context_changed),
                            private);
  g_signal_connect_swapped (private->context, "opacity-changed",
                            G_CALLBACK (fade_dialog_context_changed),
                            private);

  return dialog;
}


/*  private functions  */

static void
fade_dialog_response (GtkWidget  *dialog,
                      gint        response_id,
                      FadeDialog *private)
{
  g_signal_handlers_disconnect_by_func (private->context,
                                        fade_dialog_context_changed,
                                        private);

  if (response_id != GTK_RESPONSE_OK && private->applied)
    {
      g_object_set (private->context,
                    "paint-mode", private->orig_paint_mode,
                    "opacity",    private->orig_opacity,
                    NULL);

      fade_dialog_context_changed (private);
    }

  g_object_unref (private->context);
  gtk_widget_destroy (dialog);
}

static void
fade_dialog_context_changed (FadeDialog *private)
{
  if (picman_edit_fade (private->image, private->context))
    {
      private->applied = TRUE;
      picman_image_flush (private->image);
    }
}

static void
fade_dialog_free (FadeDialog *private)
{
  g_slice_free (FadeDialog, private);
}
