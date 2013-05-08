/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#include "config/picmancoreconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"
#include "display/picmandisplayshell.h"

#include "widgets/picmancontainertreeview.h"
#include "widgets/picmancontainerview.h"
#include "widgets/picmandnd.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmanmessagedialog.h"

#include "quit-dialog.h"

#include "picman-intl.h"


static GtkWidget * quit_close_all_dialog_new               (Picman              *picman,
                                                            gboolean           do_quit);
static void        quit_close_all_dialog_response          (GtkWidget         *dialog,
                                                            gint               response_id,
                                                            Picman              *picman);
static void        quit_close_all_dialog_container_changed (PicmanContainer     *images,
                                                            PicmanObject        *image,
                                                            PicmanMessageBox    *box);
static void        quit_close_all_dialog_image_activated   (PicmanContainerView *view,
                                                            PicmanImage         *image,
                                                            gpointer           insert_data,
                                                            Picman              *picman);


/*  public functions  */

GtkWidget *
quit_dialog_new (Picman *picman)
{
  return quit_close_all_dialog_new (picman, TRUE);
}

GtkWidget *
close_all_dialog_new (Picman *picman)
{
  return quit_close_all_dialog_new (picman, FALSE);
}

static GtkWidget *
quit_close_all_dialog_new (Picman     *picman,
                           gboolean  do_quit)
{
  PicmanContainer  *images;
  PicmanContext    *context;
  PicmanMessageBox *box;
  GtkWidget      *dialog;
  GtkWidget      *label;
  GtkWidget      *button;
  GtkWidget      *view;
  GtkWidget      *dnd_widget;
  gint            rows;
  gint            view_size;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  /* FIXME: need container of dirty images */

  images  = picman_displays_get_dirty_images (picman);
  context = picman_context_new (picman, "close-all-dialog",
                              picman_get_user_context (picman));

  g_return_val_if_fail (images != NULL, NULL);

  dialog =
    picman_message_dialog_new (do_quit ? _("Quit PICMAN") : _("Close All Images"),
                             PICMAN_STOCK_WARNING,
                             NULL, 0,
                             picman_standard_help_func,
                             do_quit ?
                             PICMAN_HELP_FILE_QUIT : PICMAN_HELP_FILE_CLOSE_ALL,

                             GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,

                             NULL);

  g_object_set_data_full (G_OBJECT (dialog), "dirty-images",
                          images, (GDestroyNotify) g_object_unref);
  g_object_set_data_full (G_OBJECT (dialog), "dirty-images-context",
                          context, (GDestroyNotify) g_object_unref);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (quit_close_all_dialog_response),
                    picman);

  box = PICMAN_MESSAGE_DIALOG (dialog)->box;

  button = gtk_dialog_add_button (GTK_DIALOG (dialog), "", GTK_RESPONSE_OK);

  g_object_set_data (G_OBJECT (box), "ok-button", button);
  g_object_set_data (G_OBJECT (box), "do-quit", GINT_TO_POINTER (do_quit));

  g_signal_connect_object (images, "add",
                           G_CALLBACK (quit_close_all_dialog_container_changed),
                           box, 0);
  g_signal_connect_object (images, "remove",
                           G_CALLBACK (quit_close_all_dialog_container_changed),
                           box, 0);

  gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                           GTK_RESPONSE_OK,
                                           GTK_RESPONSE_CANCEL,
                                           -1);

  view_size = picman->config->layer_preview_size;
  rows      = CLAMP (picman_container_get_n_children (images), 3, 6);

  view = picman_container_tree_view_new (images, context, view_size, 1);
  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (view),
                                       -1,
                                       rows * (view_size + 2));
  gtk_box_pack_start (GTK_BOX (box), view, TRUE, TRUE, 0);
  gtk_widget_show (view);

  g_signal_connect (view, "activate-item",
                    G_CALLBACK (quit_close_all_dialog_image_activated),
                    picman);

  dnd_widget = picman_container_view_get_dnd_widget (PICMAN_CONTAINER_VIEW (view));
  picman_dnd_xds_source_add (dnd_widget,
                           (PicmanDndDragViewableFunc) picman_dnd_get_drag_data,
                           NULL);

  if (do_quit)
    label = gtk_label_new (_("If you quit PICMAN now, "
                             "these changes will be lost."));
  else
    label = gtk_label_new (_("If you close these images now, "
                             "changes will be lost."));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  g_object_set_data (G_OBJECT (box), "lost-label", label);

  quit_close_all_dialog_container_changed (images, NULL,
                                           PICMAN_MESSAGE_DIALOG (dialog)->box);

  return dialog;
}

static void
quit_close_all_dialog_response (GtkWidget *dialog,
                                gint       response_id,
                                Picman      *picman)
{
  PicmanMessageBox *box     = PICMAN_MESSAGE_DIALOG (dialog)->box;
  gboolean        do_quit = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (box),
                                                                "do-quit"));

  gtk_widget_destroy (dialog);

  if (response_id == GTK_RESPONSE_OK)
    {
      if (do_quit)
        picman_exit (picman, TRUE);
      else
        picman_displays_close (picman);
    }
}

static void
quit_close_all_dialog_container_changed (PicmanContainer  *images,
                                         PicmanObject     *image,
                                         PicmanMessageBox *box)
{
  gint       num_images = picman_container_get_n_children (images);
  GtkWidget *label      = g_object_get_data (G_OBJECT (box), "lost-label");
  GtkWidget *button     = g_object_get_data (G_OBJECT (box), "ok-button");
  GtkWidget *dialog     = gtk_widget_get_toplevel (button);
  gboolean   do_quit    = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (box),
                                                              "do-quit"));
  picman_message_box_set_primary_text (box,
				     ngettext ("There is one image with "
					       "unsaved changes:",
					       "There are %d images with "
					       "unsaved changes:",
					       num_images), num_images);

  if (num_images == 0)
    {
      gtk_widget_hide (label);
      g_object_set (button,
                    "label",     do_quit ? GTK_STOCK_QUIT : GTK_STOCK_CLOSE,
                    "use-stock", TRUE,
                    "image",     NULL,
                    NULL);
      gtk_widget_grab_default (button);
    }
  else
    {
      GtkWidget *icon = gtk_image_new_from_stock (GTK_STOCK_DELETE,
                                                  GTK_ICON_SIZE_BUTTON);
      gtk_widget_show (label);
      g_object_set (button,
                    "label",     _("_Discard Changes"),
                    "use-stock", FALSE,
                    "image",     icon,
                    NULL);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                       GTK_RESPONSE_CANCEL);
    }
}

static void
quit_close_all_dialog_image_activated (PicmanContainerView *view,
                                       PicmanImage         *image,
                                       gpointer           insert_data,
                                       Picman              *picman)
{
  GList *list;

  for (list = picman_get_display_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanDisplay *display = list->data;

      if (picman_display_get_image (display) == image)
        picman_display_shell_present (picman_display_get_shell (display));
    }
}
