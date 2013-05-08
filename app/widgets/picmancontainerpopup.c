/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerpopup.c
 * Copyright (C) 2003-2005 Michael Natterer <mitch@picman.org>
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

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmancontainer.h"
#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmancontainerbox.h"
#include "picmancontainereditor.h"
#include "picmancontainerpopup.h"
#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmandialogfactory.h"
#include "picmanviewrenderer.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


enum
{
  CANCEL,
  CONFIRM,
  LAST_SIGNAL
};


static void     picman_container_popup_finalize     (GObject            *object);

static void     picman_container_popup_map          (GtkWidget          *widget);
static gboolean picman_container_popup_button_press (GtkWidget          *widget,
                                                   GdkEventButton     *bevent);
static gboolean picman_container_popup_key_press    (GtkWidget          *widget,
                                                   GdkEventKey        *kevent);

static void     picman_container_popup_real_cancel  (PicmanContainerPopup *popup);
static void     picman_container_popup_real_confirm (PicmanContainerPopup *popup);

static void     picman_container_popup_create_view  (PicmanContainerPopup *popup);

static void picman_container_popup_smaller_clicked  (GtkWidget          *button,
                                                   PicmanContainerPopup *popup);
static void picman_container_popup_larger_clicked   (GtkWidget          *button,
                                                   PicmanContainerPopup *popup);
static void picman_container_popup_view_type_toggled(GtkWidget          *button,
                                                   PicmanContainerPopup *popup);
static void picman_container_popup_dialog_clicked   (GtkWidget          *button,
                                                   PicmanContainerPopup *popup);


G_DEFINE_TYPE (PicmanContainerPopup, picman_container_popup, GTK_TYPE_WINDOW)

#define parent_class picman_container_popup_parent_class

static guint popup_signals[LAST_SIGNAL];


static void
picman_container_popup_class_init (PicmanContainerPopupClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkBindingSet  *binding_set;

  popup_signals[CANCEL] =
    g_signal_new ("cancel",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanContainerPopupClass, cancel),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  popup_signals[CONFIRM] =
    g_signal_new ("confirm",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanContainerPopupClass, confirm),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize           = picman_container_popup_finalize;

  widget_class->map                = picman_container_popup_map;
  widget_class->button_press_event = picman_container_popup_button_press;
  widget_class->key_press_event    = picman_container_popup_key_press;

  klass->cancel                    = picman_container_popup_real_cancel;
  klass->confirm                   = picman_container_popup_real_confirm;

  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Escape, 0,
                                "cancel", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Return, 0,
                                "confirm", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Enter, 0,
                                "confirm", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_ISO_Enter, 0,
                                "confirm", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_space, 0,
                                "confirm", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Space, 0,
                                "confirm", 0);
}

static void
picman_container_popup_init (PicmanContainerPopup *popup)
{
  popup->view_type         = PICMAN_VIEW_TYPE_LIST;
  popup->default_view_size = PICMAN_VIEW_SIZE_SMALL;
  popup->view_size         = PICMAN_VIEW_SIZE_SMALL;
  popup->view_border_width = 1;

  popup->frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (popup->frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (popup), popup->frame);
  gtk_widget_show (popup->frame);
}

static void
picman_container_popup_finalize (GObject *object)
{
  PicmanContainerPopup *popup = PICMAN_CONTAINER_POPUP (object);

  if (popup->context)
    {
      g_object_unref (popup->context);
      popup->context = NULL;
    }

  if (popup->dialog_identifier)
    {
      g_free (popup->dialog_identifier);
      popup->dialog_identifier = NULL;
    }

  if (popup->dialog_stock_id)
    {
      g_free (popup->dialog_stock_id);
      popup->dialog_stock_id = NULL;
    }

  if (popup->dialog_tooltip)
    {
      g_free (popup->dialog_tooltip);
      popup->dialog_tooltip = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_container_popup_grab_notify (GtkWidget *widget,
                                  gboolean   was_grabbed)
{
  if (was_grabbed)
    return;

  /* ignore grabs on one of our children, like the scrollbar */
  if (gtk_widget_is_ancestor (gtk_grab_get_current (), widget))
    return;

  g_signal_emit (widget, popup_signals[CANCEL], 0);
}

static gboolean
picman_container_popup_grab_broken_event (GtkWidget          *widget,
                                        GdkEventGrabBroken *event)
{
  picman_container_popup_grab_notify (widget, FALSE);

  return FALSE;
}

static void
picman_container_popup_map (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (parent_class)->map (widget);

  /*  grab with owner_events == TRUE so the popup's widgets can
   *  receive events. we filter away events outside this toplevel
   *  away in button_press()
   */
  if (gdk_pointer_grab (gtk_widget_get_window (widget), TRUE,
                        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                        GDK_POINTER_MOTION_MASK,
                        NULL, NULL, GDK_CURRENT_TIME) == 0)
    {
      if (gdk_keyboard_grab (gtk_widget_get_window (widget), TRUE,
                             GDK_CURRENT_TIME) == 0)
        {
          gtk_grab_add (widget);

          g_signal_connect (widget, "grab-notify",
                            G_CALLBACK (picman_container_popup_grab_notify),
                            widget);
          g_signal_connect (widget, "grab-broken-event",
                            G_CALLBACK (picman_container_popup_grab_broken_event),
                            widget);

          return;
        }
      else
        {
          gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                      GDK_CURRENT_TIME);
        }
    }

  /*  if we could not grab, destroy the popup instead of leaving it
   *  around uncloseable.
   */
  g_signal_emit (widget, popup_signals[CANCEL], 0);
}

static gboolean
picman_container_popup_button_press (GtkWidget      *widget,
                                   GdkEventButton *bevent)
{
  GtkWidget *event_widget;
  gboolean   cancel = FALSE;

  event_widget = gtk_get_event_widget ((GdkEvent *) bevent);

  if (event_widget == widget)
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation (widget, &allocation);

      /*  the event was on the popup, which can either be really on the
       *  popup or outside picman (owner_events == TRUE, see map())
       */
      if (bevent->x < 0                ||
          bevent->y < 0                ||
          bevent->x > allocation.width ||
          bevent->y > allocation.height)
        {
          /*  the event was outsde picman  */

          cancel = TRUE;
        }
    }
  else if (gtk_widget_get_toplevel (event_widget) != widget)
    {
      /*  the event was on a picman widget, but not inside the popup  */

      cancel = TRUE;
    }

  if (cancel)
    g_signal_emit (widget, popup_signals[CANCEL], 0);

  return cancel;
}

static gboolean
picman_container_popup_key_press (GtkWidget   *widget,
                                GdkEventKey *kevent)
{
  GtkBindingSet *binding_set;

  binding_set =
    gtk_binding_set_by_class (PICMAN_CONTAINER_POPUP_GET_CLASS (widget));

  /*  invoke the popup's binding entries manually, because otherwise
   *  the focus widget (GtkTreeView e.g.) would consume it
   */
  if (gtk_binding_set_activate (binding_set,
                                kevent->keyval,
                                kevent->state,
                                GTK_OBJECT (widget)))
    {
      return TRUE;
    }

  return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, kevent);
}

static void
picman_container_popup_real_cancel (PicmanContainerPopup *popup)
{
  GtkWidget *widget = GTK_WIDGET (popup);

  if (gtk_grab_get_current () == widget)
    gtk_grab_remove (widget);

  gtk_widget_destroy (widget);
}

static void
picman_container_popup_real_confirm (PicmanContainerPopup *popup)
{
  GtkWidget  *widget = GTK_WIDGET (popup);
  PicmanObject *object;

  object = picman_context_get_by_type (popup->context,
                                     picman_container_get_children_type (popup->container));
  picman_context_set_by_type (popup->orig_context,
                            picman_container_get_children_type (popup->container),
                            object);

  if (gtk_grab_get_current () == widget)
    gtk_grab_remove (widget);

  gtk_widget_destroy (widget);
}

static void
picman_container_popup_context_changed (PicmanContext        *context,
                                      PicmanViewable       *viewable,
                                      PicmanContainerPopup *popup)
{
  GdkEvent *current_event;
  gboolean  confirm = FALSE;

  current_event = gtk_get_current_event ();

  if (current_event)
    {
      if (((GdkEventAny *) current_event)->type == GDK_BUTTON_PRESS ||
          ((GdkEventAny *) current_event)->type == GDK_BUTTON_RELEASE)
        confirm = TRUE;

      gdk_event_free (current_event);
    }

  if (confirm)
    g_signal_emit (popup, popup_signals[CONFIRM], 0);
}

GtkWidget *
picman_container_popup_new (PicmanContainer     *container,
                          PicmanContext       *context,
                          PicmanViewType       view_type,
                          gint               default_view_size,
                          gint               view_size,
                          gint               view_border_width,
                          PicmanDialogFactory *dialog_factory,
                          const gchar       *dialog_identifier,
                          const gchar       *dialog_stock_id,
                          const gchar       *dialog_tooltip)
{
  PicmanContainerPopup *popup;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (default_view_size >  0 &&
                        default_view_size <= PICMAN_VIEWABLE_MAX_POPUP_SIZE,
                        NULL);
  g_return_val_if_fail (view_size >  0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_POPUP_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (dialog_factory == NULL ||
                        PICMAN_IS_DIALOG_FACTORY (dialog_factory), NULL);
  if (dialog_factory)
    {
      g_return_val_if_fail (dialog_identifier != NULL, NULL);
      g_return_val_if_fail (dialog_stock_id != NULL, NULL);
      g_return_val_if_fail (dialog_tooltip != NULL, NULL);
    }

  popup = g_object_new (PICMAN_TYPE_CONTAINER_POPUP,
                        "type", GTK_WINDOW_POPUP,
                        NULL);
  gtk_window_set_resizable (GTK_WINDOW (popup), FALSE);

  popup->container    = container;
  popup->orig_context = context;
  popup->context      = picman_context_new (context->picman, "popup", context);

  popup->view_type         = view_type;
  popup->default_view_size = default_view_size;
  popup->view_size         = view_size;
  popup->view_border_width = view_border_width;

  g_signal_connect (popup->context,
                    picman_context_type_to_signal_name (picman_container_get_children_type (container)),
                    G_CALLBACK (picman_container_popup_context_changed),
                    popup);

  if (dialog_factory)
    {
      popup->dialog_factory    = dialog_factory;
      popup->dialog_identifier = g_strdup (dialog_identifier);
      popup->dialog_stock_id   = g_strdup (dialog_stock_id);
      popup->dialog_tooltip    = g_strdup (dialog_tooltip);
    }

  picman_container_popup_create_view (popup);

  return GTK_WIDGET (popup);
}

void
picman_container_popup_show (PicmanContainerPopup *popup,
                           GtkWidget          *widget)
{
  GdkScreen      *screen;
  GtkRequisition  requisition;
  GtkAllocation   allocation;
  GdkRectangle    rect;
  gint            monitor;
  gint            orig_x;
  gint            orig_y;
  gint            x;
  gint            y;

  g_return_if_fail (PICMAN_IS_CONTAINER_POPUP (popup));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_widget_size_request (GTK_WIDGET (popup), &requisition);

  gtk_widget_get_allocation (widget, &allocation);
  gdk_window_get_origin (gtk_widget_get_window (widget), &orig_x, &orig_y);

  if (! gtk_widget_get_has_window (widget))
    {
      orig_x += allocation.x;
      orig_y += allocation.y;
    }

  screen = gtk_widget_get_screen (widget);

  monitor = gdk_screen_get_monitor_at_point (screen, orig_x, orig_y);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    {
      x = orig_x + allocation.width - requisition.width;

      if (x < rect.x)
        x -= allocation.width - requisition.width;
    }
  else
    {
      x = orig_x;

      if (x + requisition.width > rect.x + rect.width)
        x += allocation.width - requisition.width;
    }

  y = orig_y + allocation.height;

  if (y + requisition.height > rect.y + rect.height)
    y = orig_y - requisition.height;

  gtk_window_set_screen (GTK_WINDOW (popup), screen);
  gtk_window_set_transient_for (GTK_WINDOW (popup),
                                GTK_WINDOW (gtk_widget_get_toplevel (widget)));

  gtk_window_move (GTK_WINDOW (popup), x, y);
  gtk_widget_show (GTK_WIDGET (popup));
}

PicmanViewType
picman_container_popup_get_view_type (PicmanContainerPopup *popup)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER_POPUP (popup), PICMAN_VIEW_TYPE_LIST);

  return popup->view_type;
}

void
picman_container_popup_set_view_type (PicmanContainerPopup *popup,
                                    PicmanViewType        view_type)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_POPUP (popup));

  if (view_type != popup->view_type)
    {
      popup->view_type = view_type;

      gtk_widget_destroy (GTK_WIDGET (popup->editor));
      picman_container_popup_create_view (popup);
    }
}

gint
picman_container_popup_get_view_size (PicmanContainerPopup *popup)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER_POPUP (popup), PICMAN_VIEW_SIZE_SMALL);

  return popup->view_size;
}

void
picman_container_popup_set_view_size (PicmanContainerPopup *popup,
                                    gint                view_size)
{
  GtkWidget     *scrolled_win;
  GtkWidget     *viewport;
  GtkAllocation  allocation;

  g_return_if_fail (PICMAN_IS_CONTAINER_POPUP (popup));

  scrolled_win = PICMAN_CONTAINER_BOX (popup->editor->view)->scrolled_win;
  viewport     = gtk_bin_get_child (GTK_BIN (scrolled_win));

  gtk_widget_get_allocation (viewport, &allocation);

  view_size = CLAMP (view_size, PICMAN_VIEW_SIZE_TINY,
                     MIN (PICMAN_VIEW_SIZE_GIGANTIC,
                          allocation.width - 2 * popup->view_border_width));

  if (view_size != popup->view_size)
    {
      popup->view_size = view_size;

      picman_container_view_set_view_size (popup->editor->view,
                                         popup->view_size,
                                         popup->view_border_width);
    }
}


/*  private functions  */

static void
picman_container_popup_create_view (PicmanContainerPopup *popup)
{
  PicmanEditor *editor;
  GtkWidget  *button;

  popup->editor = g_object_new (PICMAN_TYPE_CONTAINER_EDITOR,
                                "view-type",         popup->view_type,
                                "container",         popup->container,
                                "context",           popup->context,
                                "view-size",         popup->view_size,
                                "view-border-width", popup->view_border_width,
                                NULL);

  picman_container_view_set_reorderable (PICMAN_CONTAINER_VIEW (popup->editor->view),
                                       FALSE);

  if (popup->view_type == PICMAN_VIEW_TYPE_LIST)
    {
      GtkWidget *search_entry;

      search_entry = gtk_entry_new ();
      gtk_box_pack_end (GTK_BOX (popup->editor->view), search_entry,
                        FALSE, FALSE, 0);
      gtk_tree_view_set_search_entry (GTK_TREE_VIEW (PICMAN_CONTAINER_TREE_VIEW (PICMAN_CONTAINER_VIEW (popup->editor->view))->view),
                                      GTK_ENTRY (search_entry));
      gtk_widget_show (search_entry);
    }

  picman_container_box_set_size_request (PICMAN_CONTAINER_BOX (popup->editor->view),
                                       6  * (popup->default_view_size +
                                             2 * popup->view_border_width),
                                       10 * (popup->default_view_size +
                                             2 * popup->view_border_width));

  if (PICMAN_IS_EDITOR (popup->editor->view))
    picman_editor_set_show_name (PICMAN_EDITOR (popup->editor->view), FALSE);

  gtk_container_add (GTK_CONTAINER (popup->frame), GTK_WIDGET (popup->editor));
  gtk_widget_show (GTK_WIDGET (popup->editor));

  editor = PICMAN_EDITOR (popup->editor->view);

  picman_editor_add_button (editor, GTK_STOCK_ZOOM_OUT,
                          _("Smaller Previews"), NULL,
                          G_CALLBACK (picman_container_popup_smaller_clicked),
                          NULL,
                          popup);
  picman_editor_add_button (editor, GTK_STOCK_ZOOM_IN,
                          _("Larger Previews"), NULL,
                          G_CALLBACK (picman_container_popup_larger_clicked),
                          NULL,
                          popup);

  button = picman_editor_add_stock_box (editor, PICMAN_TYPE_VIEW_TYPE, "picman",
                                      G_CALLBACK (picman_container_popup_view_type_toggled),
                                      popup);
  picman_int_radio_group_set_active (GTK_RADIO_BUTTON (button), popup->view_type);

  if (popup->dialog_factory)
    picman_editor_add_button (editor, popup->dialog_stock_id,
                            popup->dialog_tooltip, NULL,
                            G_CALLBACK (picman_container_popup_dialog_clicked),
                            NULL,
                            popup);

  gtk_widget_grab_focus (GTK_WIDGET (popup->editor));
}

static void
picman_container_popup_smaller_clicked (GtkWidget          *button,
                                      PicmanContainerPopup *popup)
{
  gint view_size;

  view_size = picman_container_view_get_view_size (popup->editor->view, NULL);

  picman_container_popup_set_view_size (popup, view_size * 0.8);
}

static void
picman_container_popup_larger_clicked (GtkWidget          *button,
                                     PicmanContainerPopup *popup)
{
  gint view_size;

  view_size = picman_container_view_get_view_size (popup->editor->view, NULL);

  picman_container_popup_set_view_size (popup, view_size * 1.2);
}

static void
picman_container_popup_view_type_toggled (GtkWidget          *button,
                                        PicmanContainerPopup *popup)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    {
      PicmanViewType view_type;

      view_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button),
                                                      "picman-item-data"));

      picman_container_popup_set_view_type (popup, view_type);
    }
}

static void
picman_container_popup_dialog_clicked (GtkWidget          *button,
                                     PicmanContainerPopup *popup)
{
  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (popup->context->picman)),
                                             popup->context->picman,
                                             popup->dialog_factory,
                                             gtk_widget_get_screen (button),
                                             popup->dialog_identifier);
  g_signal_emit (popup, popup_signals[CONFIRM], 0);
}
