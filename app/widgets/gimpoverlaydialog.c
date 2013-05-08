/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoverlaydialog.c
 * Copyright (C) 2009-2010  Michael Natterer <mitch@picman.org>
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
#include <gdk/gdkkeysyms.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanmarshal.h"
#include "core/picmantoolinfo.h"

#include "picmanoverlaydialog.h"


enum
{
  RESPONSE,
  CLOSE,
  LAST_SIGNAL
};


typedef struct _ResponseData ResponseData;

struct _ResponseData
{
  gint response_id;
};


static void       picman_overlay_dialog_dispose       (GObject           *object);

static void       picman_overlay_dialog_size_request  (GtkWidget         *widget,
                                                     GtkRequisition    *requisition);
static void       picman_overlay_dialog_size_allocate (GtkWidget         *widget,
                                                     GtkAllocation     *allocation);

static void       picman_overlay_dialog_forall        (GtkContainer      *container,
                                                     gboolean           include_internals,
                                                     GtkCallback        callback,
                                                     gpointer           callback_data);

static void       picman_overlay_dialog_close         (PicmanOverlayDialog *dialog);

static ResponseData * get_response_data             (GtkWidget         *widget,
                                                     gboolean          create);


G_DEFINE_TYPE (PicmanOverlayDialog, picman_overlay_dialog,
               PICMAN_TYPE_OVERLAY_FRAME)

static guint signals[LAST_SIGNAL] = { 0, };

#define parent_class picman_overlay_dialog_parent_class


static void
picman_overlay_dialog_class_init (PicmanOverlayDialogClass *klass)
{
  GObjectClass      *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass    *widget_class    = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->dispose       = picman_overlay_dialog_dispose;

  widget_class->size_request  = picman_overlay_dialog_size_request;
  widget_class->size_allocate = picman_overlay_dialog_size_allocate;

  container_class->forall     = picman_overlay_dialog_forall;

  klass->close                = picman_overlay_dialog_close;

  signals[RESPONSE] =
    g_signal_new ("response",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanOverlayDialogClass, response),
                  NULL, NULL,
                  picman_marshal_VOID__INT,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);

  signals[CLOSE] =
    g_signal_new ("close",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanOverlayDialogClass, close),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  gtk_binding_entry_add_signal (gtk_binding_set_by_class (klass),
                                GDK_KEY_Escape, 0, "close", 0);
}

static void
picman_overlay_dialog_init (PicmanOverlayDialog *dialog)
{
  dialog->action_area = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->action_area),
                             GTK_BUTTONBOX_END);
  gtk_widget_set_parent (dialog->action_area, GTK_WIDGET (dialog));
  gtk_widget_show (dialog->action_area);
}

static void
picman_overlay_dialog_dispose (GObject *object)
{
  PicmanOverlayDialog *dialog = PICMAN_OVERLAY_DIALOG (object);

  if (dialog->action_area)
    {
      gtk_widget_unparent (dialog->action_area);
      dialog->action_area = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_overlay_dialog_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition)
{
  GtkContainer      *container = GTK_CONTAINER (widget);
  PicmanOverlayDialog *dialog    = PICMAN_OVERLAY_DIALOG (widget);
  GtkWidget         *child     = gtk_bin_get_child (GTK_BIN (widget));
  GtkRequisition     child_requisition;
  GtkRequisition     action_requisition;
  gint               border_width;

  border_width = gtk_container_get_border_width (container);

  requisition->width  = border_width * 2;
  requisition->height = border_width * 2;

  if (child && gtk_widget_get_visible (child))
    {
      gtk_widget_size_request (child, &child_requisition);
    }
  else
    {
      child_requisition.width  = 0;
      child_requisition.height = 0;
    }

  gtk_widget_size_request (dialog->action_area, &action_requisition);

  requisition->width  += MAX (child_requisition.width,
                              action_requisition.width);
  requisition->height += (child_requisition.height +
                          border_width +
                          action_requisition.height);
}

static void
picman_overlay_dialog_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
  GtkContainer      *container = GTK_CONTAINER (widget);
  PicmanOverlayDialog *dialog    = PICMAN_OVERLAY_DIALOG (widget);
  GtkWidget         *child     = gtk_bin_get_child (GTK_BIN (widget));
  GtkRequisition     action_requisition;
  GtkAllocation      child_allocation = { 0, };
  GtkAllocation      action_allocation;
  gint               border_width;

  gtk_widget_set_allocation (widget, allocation);

  border_width = gtk_container_get_border_width (container);

  gtk_widget_size_request (dialog->action_area, &action_requisition);

  if (child && gtk_widget_get_visible (child))
    {
      child_allocation.x      = allocation->x + border_width;
      child_allocation.y      = allocation->y + border_width;
      child_allocation.width  = MAX (allocation->width  - 2 * border_width, 0);
      child_allocation.height = MAX (allocation->height -
                                     3 * border_width -
                                     action_requisition.height, 0);

      gtk_widget_size_allocate (child, &child_allocation);
    }

  action_allocation.x = allocation->x + border_width;
  action_allocation.y = (child_allocation.y + child_allocation.height +
                         border_width);
  action_allocation.width  = MAX (allocation->width  - 2 * border_width, 0);
  action_allocation.height = MAX (action_requisition.height, 0);

  gtk_widget_size_allocate (dialog->action_area, &action_allocation);
}

static void
picman_overlay_dialog_forall (GtkContainer *container,
                            gboolean      include_internals,
                            GtkCallback   callback,
                            gpointer      callback_data)
{
  GTK_CONTAINER_CLASS (parent_class)->forall (container, include_internals,
                                              callback, callback_data);

  if (include_internals)
    {
      PicmanOverlayDialog *dialog = PICMAN_OVERLAY_DIALOG (container);

      if (dialog->action_area)
        (* callback) (dialog->action_area, callback_data);
    }
}

static void
picman_overlay_dialog_close (PicmanOverlayDialog *dialog)
{
  GList        *children;
  GList        *list;
  ResponseData *ad = NULL;

  children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

  for (list = children; list; list = g_list_next (list))
    {
      GtkWidget *child = list->data;

      ad = get_response_data (child, FALSE);

      if (ad->response_id == GTK_RESPONSE_CLOSE ||
          ad->response_id == GTK_RESPONSE_CANCEL)
        {
          break;
        }

      ad = NULL;
    }

  g_list_free (children);

  if (ad)
    picman_overlay_dialog_response (dialog, ad->response_id);
}

GtkWidget *
picman_overlay_dialog_new (PicmanToolInfo *tool_info,
                         const gchar  *desc,
                         ...)
{
  GtkWidget   *dialog;
  /* const gchar *stock_id; */
  va_list      args;

  g_return_val_if_fail (PICMAN_IS_TOOL_INFO (tool_info), NULL);

  /* stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info)); */

  dialog = g_object_new (PICMAN_TYPE_OVERLAY_DIALOG, NULL);

  va_start (args, desc);
  picman_overlay_dialog_add_buttons_valist (PICMAN_OVERLAY_DIALOG (dialog), args);
  va_end (args);

  return dialog;
}

void
picman_overlay_dialog_response (PicmanOverlayDialog *dialog,
                              gint               response_id)
{
  g_return_if_fail (PICMAN_IS_OVERLAY_DIALOG (dialog));

  g_signal_emit (dialog, signals[RESPONSE], 0,
		 response_id);
}

void
picman_overlay_dialog_add_buttons_valist (PicmanOverlayDialog *dialog,
                                        va_list            args)
{
  const gchar *button_text;
  gint         response_id;

  g_return_if_fail (PICMAN_IS_OVERLAY_DIALOG (dialog));

  while ((button_text = va_arg (args, const gchar *)))
    {
      response_id = va_arg (args, gint);

      picman_overlay_dialog_add_button (dialog, button_text, response_id);
    }
}

static void
action_widget_activated (GtkWidget         *widget,
                         PicmanOverlayDialog *dialog)
{
  ResponseData *ad = get_response_data (widget, FALSE);

  picman_overlay_dialog_response (dialog, ad->response_id);
}

GtkWidget *
picman_overlay_dialog_add_button (PicmanOverlayDialog *dialog,
                                const gchar       *button_text,
                                gint               response_id)
{
  GtkWidget    *button;
  ResponseData *ad;
  guint         signal_id;
  GClosure     *closure;

  g_return_val_if_fail (PICMAN_IS_OVERLAY_DIALOG (dialog), NULL);
  g_return_val_if_fail (button_text != NULL, NULL);

  button = gtk_button_new_from_stock (button_text);

  gtk_widget_set_can_default (button, TRUE);

  gtk_widget_show (button);

  ad = get_response_data (button, TRUE);

  ad->response_id = response_id;

  signal_id = g_signal_lookup ("clicked", GTK_TYPE_BUTTON);

  closure = g_cclosure_new_object (G_CALLBACK (action_widget_activated),
                                   G_OBJECT (dialog));
  g_signal_connect_closure_by_id (button, signal_id, 0,
                                  closure, FALSE);

  gtk_box_pack_end (GTK_BOX (dialog->action_area), button, FALSE, TRUE, 0);

  if (response_id == GTK_RESPONSE_HELP)
    gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (dialog->action_area),
                                        button, TRUE);

  return button;
}

static void
response_data_free (gpointer data)
{
  g_slice_free (ResponseData, data);
}

static ResponseData *
get_response_data (GtkWidget *widget,
		   gboolean   create)
{
  ResponseData *ad = g_object_get_data (G_OBJECT (widget),
                                        "picman-overlay-dialog-response-data");

  if (! ad && create)
    {
      ad = g_slice_new (ResponseData);

      g_object_set_data_full (G_OBJECT (widget),
                              "picman-overlay-dialog-response-data",
                              ad, response_data_free);
    }

  return ad;
}
