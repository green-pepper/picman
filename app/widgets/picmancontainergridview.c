/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainergridview.c
 * Copyright (C) 2001-2004 Michael Natterer <mitch@picman.org>
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmancontainergridview.h"
#include "picmancontainerview.h"
#include "picmanview.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"
#include "gtkhwrapbox.h"

#include "picman-intl.h"


enum
{
  MOVE_CURSOR,
  LAST_SIGNAL
};


static void  picman_container_grid_view_view_iface_init (PicmanContainerViewInterface *iface);

static gboolean picman_container_grid_view_move_cursor  (PicmanContainerGridView  *view,
                                                       GtkMovementStep         step,
                                                       gint                    count);
static gboolean picman_container_grid_view_focus        (GtkWidget              *widget,
                                                       GtkDirectionType        direction);
static gboolean  picman_container_grid_view_popup_menu  (GtkWidget              *widget);

static void     picman_container_grid_view_set_context  (PicmanContainerView      *view,
                                                       PicmanContext            *context);
static gpointer picman_container_grid_view_insert_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                parent_insert_data,
                                                       gint                    index);
static void     picman_container_grid_view_remove_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static void     picman_container_grid_view_reorder_item (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gint                    new_index,
                                                       gpointer                insert_data);
static void     picman_container_grid_view_rename_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static gboolean  picman_container_grid_view_select_item (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static void     picman_container_grid_view_clear_items  (PicmanContainerView      *view);
static void    picman_container_grid_view_set_view_size (PicmanContainerView      *view);

static gboolean picman_container_grid_view_item_selected(GtkWidget              *widget,
                                                       GdkEventButton         *bevent,
                                                       gpointer                data);
static void   picman_container_grid_view_item_activated (GtkWidget              *widget,
                                                       gpointer                data);
static void   picman_container_grid_view_item_context   (GtkWidget              *widget,
                                                       gpointer                data);
static void   picman_container_grid_view_highlight_item (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);

static void picman_container_grid_view_viewport_resized (GtkWidget              *widget,
                                                       GtkAllocation          *allocation,
                                                       PicmanContainerGridView  *view);
static gboolean picman_container_grid_view_button_press (GtkWidget              *widget,
                                                       GdkEventButton         *bevent,
                                                       PicmanContainerGridView  *view);


G_DEFINE_TYPE_WITH_CODE (PicmanContainerGridView, picman_container_grid_view,
                         PICMAN_TYPE_CONTAINER_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_container_grid_view_view_iface_init))

#define parent_class picman_container_grid_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;

static guint grid_view_signals[LAST_SIGNAL] = { 0 };

static PicmanRGB  white_color;
static PicmanRGB  black_color;


static void
picman_container_grid_view_class_init (PicmanContainerGridViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkBindingSet  *binding_set;

  binding_set  = gtk_binding_set_by_class (klass);

  widget_class->focus      = picman_container_grid_view_focus;
  widget_class->popup_menu = picman_container_grid_view_popup_menu;

  klass->move_cursor       = picman_container_grid_view_move_cursor;

  grid_view_signals[MOVE_CURSOR] =
    g_signal_new ("move-cursor",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanContainerGridViewClass, move_cursor),
                  NULL, NULL,
                  picman_marshal_BOOLEAN__ENUM_INT,
                  G_TYPE_BOOLEAN, 2,
                  GTK_TYPE_MOVEMENT_STEP,
                  G_TYPE_INT);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Home, 0,
                                "move-cursor", 2,
                                G_TYPE_ENUM, GTK_MOVEMENT_BUFFER_ENDS,
                                G_TYPE_INT, -1);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_End, 0,
                                "move-cursor", 2,
                                G_TYPE_ENUM, GTK_MOVEMENT_BUFFER_ENDS,
                                G_TYPE_INT, 1);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Page_Up, 0,
                                "move-cursor", 2,
                                G_TYPE_ENUM, GTK_MOVEMENT_PAGES,
                                G_TYPE_INT, -1);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Page_Down, 0,
                                "move-cursor", 2,
                                G_TYPE_ENUM, GTK_MOVEMENT_PAGES,
                                G_TYPE_INT, 1);

  picman_rgba_set (&white_color, 1.0, 1.0, 1.0, 1.0);
  picman_rgba_set (&black_color, 0.0, 0.0, 0.0, 1.0);
}

static void
picman_container_grid_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  iface->set_context   = picman_container_grid_view_set_context;
  iface->insert_item   = picman_container_grid_view_insert_item;
  iface->remove_item   = picman_container_grid_view_remove_item;
  iface->reorder_item  = picman_container_grid_view_reorder_item;
  iface->rename_item   = picman_container_grid_view_rename_item;
  iface->select_item   = picman_container_grid_view_select_item;
  iface->clear_items   = picman_container_grid_view_clear_items;
  iface->set_view_size = picman_container_grid_view_set_view_size;
}

static void
picman_container_grid_view_init (PicmanContainerGridView *grid_view)
{
  PicmanContainerBox *box = PICMAN_CONTAINER_BOX (grid_view);
  GtkWidget        *viewport;

  grid_view->rows          = 1;
  grid_view->columns       = 1;
  grid_view->visible_rows  = 0;
  grid_view->selected_item = NULL;

  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (box->scrolled_win),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (box->scrolled_win),
                                  GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  picman_editor_set_show_name (PICMAN_EDITOR (grid_view), TRUE);

  grid_view->wrap_box = gtk_hwrap_box_new (FALSE);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (box->scrolled_win),
                                         grid_view->wrap_box);
  viewport = gtk_widget_get_parent (grid_view->wrap_box);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_NONE);
  gtk_widget_show (grid_view->wrap_box);

  g_signal_connect (viewport, "size-allocate",
                    G_CALLBACK (picman_container_grid_view_viewport_resized),
                    grid_view);
  g_signal_connect (viewport, "button-press-event",
                    G_CALLBACK (picman_container_grid_view_button_press),
                    grid_view);

  gtk_widget_set_can_focus (GTK_WIDGET (grid_view), TRUE);
}

GtkWidget *
picman_container_grid_view_new (PicmanContainer *container,
                              PicmanContext   *context,
                              gint           view_size,
                              gint           view_border_width)
{
  PicmanContainerGridView *grid_view;
  PicmanContainerView     *view;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size  > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);

  grid_view = g_object_new (PICMAN_TYPE_CONTAINER_GRID_VIEW, NULL);

  view = PICMAN_CONTAINER_VIEW (grid_view);

  picman_container_view_set_view_size (view, view_size, view_border_width);

  if (container)
    picman_container_view_set_container (view, container);

  if (context)
    picman_container_view_set_context (view, context);

  return GTK_WIDGET (grid_view);
}

static gboolean
picman_container_grid_view_move_by (PicmanContainerGridView *grid_view,
                                  gint                   x,
                                  gint                   y)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (grid_view);
  PicmanContainer     *container;
  PicmanViewable      *item;
  gint               index;

  if (! grid_view->selected_item)
    return FALSE;

  container = picman_container_view_get_container (view);

  item = grid_view->selected_item->viewable;

  index = picman_container_get_child_index (container, PICMAN_OBJECT (item));

  index += x;
  index = CLAMP (index, 0, picman_container_get_n_children (container) - 1);

  index += y * grid_view->columns;
  while (index < 0)
    index += grid_view->columns;
  while (index >= picman_container_get_n_children (container))
    index -= grid_view->columns;

  item = (PicmanViewable *) picman_container_get_child_by_index (container, index);
  if (item)
    picman_container_view_item_selected (PICMAN_CONTAINER_VIEW (view), item);

  return TRUE;
}

static gboolean
picman_container_grid_view_move_cursor (PicmanContainerGridView *grid_view,
                                      GtkMovementStep        step,
                                      gint                   count)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (grid_view);
  PicmanContainer     *container;
  PicmanViewable      *item;

  if (! gtk_widget_has_focus (GTK_WIDGET (grid_view)) || count == 0)
    return FALSE;

  container = picman_container_view_get_container (view);

  switch (step)
    {
    case GTK_MOVEMENT_PAGES:
      return picman_container_grid_view_move_by (grid_view, 0,
                                               count * grid_view->visible_rows);

    case GTK_MOVEMENT_BUFFER_ENDS:
      count = count < 0 ? 0 : picman_container_get_n_children (container) - 1;

      item = (PicmanViewable *) picman_container_get_child_by_index (container,
                                                                 count);
      if (item)
        picman_container_view_item_selected (view, item);

      return TRUE;

    default:
      break;
    }

  return FALSE;
}

static gboolean
picman_container_grid_view_focus (GtkWidget        *widget,
                                GtkDirectionType  direction)
{
  PicmanContainerGridView *view = PICMAN_CONTAINER_GRID_VIEW (widget);

  if (gtk_widget_get_can_focus (widget) && ! gtk_widget_has_focus (widget))
    {
      gtk_widget_grab_focus (GTK_WIDGET (widget));
      return TRUE;
    }

  switch (direction)
    {
    case GTK_DIR_UP:
      return picman_container_grid_view_move_by (view,  0, -1);
    case GTK_DIR_DOWN:
      return picman_container_grid_view_move_by (view,  0,  1);
    case GTK_DIR_LEFT:
      return picman_container_grid_view_move_by (view, -1,  0);
    case GTK_DIR_RIGHT:
      return picman_container_grid_view_move_by (view,  1,  0);

    case GTK_DIR_TAB_FORWARD:
    case GTK_DIR_TAB_BACKWARD:
      break;
    }

  return FALSE;
}

static void
picman_container_grid_view_menu_position (GtkMenu  *menu,
                                        gint     *x,
                                        gint     *y,
                                        gpointer  data)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (data);
  GtkWidget             *widget;
  GtkAllocation          allocation;

  if (grid_view->selected_item)
    widget = GTK_WIDGET (grid_view->selected_item);
  else
    widget = GTK_WIDGET (grid_view->wrap_box);

  gtk_widget_get_allocation (widget, &allocation);

  gdk_window_get_origin (gtk_widget_get_window (widget), x, y);

  if (! gtk_widget_get_has_window (widget))
    {
      *x += allocation.x;
      *y += allocation.y;
    }

  if (grid_view->selected_item)
    {
      *x += allocation.width  / 2;
      *y += allocation.height / 2;
    }
  else
    {
      GtkStyle *style = gtk_widget_get_style (widget);

      *x += style->xthickness;
      *y += style->ythickness;
    }

  picman_menu_position (menu, x, y);
}

static gboolean
picman_container_grid_view_popup_menu (GtkWidget *widget)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (widget);

  return picman_editor_popup_menu (PICMAN_EDITOR (widget),
                                 picman_container_grid_view_menu_position,
                                 grid_view);
}

static void
picman_container_grid_view_set_context (PicmanContainerView *view,
                                      PicmanContext       *context)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (view);
  GtkWrapBoxChild       *child;

  parent_view_iface->set_context (view, context);

  for (child = GTK_WRAP_BOX (grid_view->wrap_box)->children;
       child;
       child = child->next)
    {
      PicmanView *view = PICMAN_VIEW (child->widget);

      picman_view_renderer_set_context (view->renderer, context);
    }
}

static gpointer
picman_container_grid_view_insert_item (PicmanContainerView *container_view,
                                      PicmanViewable      *viewable,
                                      gpointer           parent_insert_data,
                                      gint               index)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (container_view);
  GtkWidget             *view;
  gint                   view_size;

  view_size = picman_container_view_get_view_size (container_view, NULL);

  view = picman_view_new_full (picman_container_view_get_context (container_view),
                             viewable,
                             view_size, view_size, 1,
                             FALSE, TRUE, TRUE);
  picman_view_renderer_set_border_type (PICMAN_VIEW (view)->renderer,
                                      PICMAN_VIEW_BORDER_WHITE);
  picman_view_renderer_remove_idle (PICMAN_VIEW (view)->renderer);

  gtk_wrap_box_pack (GTK_WRAP_BOX (grid_view->wrap_box), view,
                     FALSE, FALSE, FALSE, FALSE);

  if (index != -1)
    gtk_wrap_box_reorder_child (GTK_WRAP_BOX (grid_view->wrap_box),
                                view, index);

  gtk_widget_show (view);

  g_signal_connect (view, "button-press-event",
                    G_CALLBACK (picman_container_grid_view_item_selected),
                    container_view);
  g_signal_connect (view, "double-clicked",
                    G_CALLBACK (picman_container_grid_view_item_activated),
                    container_view);
  g_signal_connect (view, "context",
                    G_CALLBACK (picman_container_grid_view_item_context),
                    container_view);

  return (gpointer) view;
}

static void
picman_container_grid_view_remove_item (PicmanContainerView *container_view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (container_view);
  GtkWidget             *view      = GTK_WIDGET (insert_data);

  if (view == (GtkWidget *) grid_view->selected_item)
    grid_view->selected_item = NULL;

  gtk_widget_destroy (view);
}

static void
picman_container_grid_view_reorder_item (PicmanContainerView *container_view,
                                       PicmanViewable      *viewable,
                                       gint               new_index,
                                       gpointer           insert_data)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (container_view);
  GtkWidget             *view      = GTK_WIDGET (insert_data);

  gtk_wrap_box_reorder_child (GTK_WRAP_BOX (grid_view->wrap_box),
                              view, new_index);
}

static void
picman_container_grid_view_rename_item (PicmanContainerView *container_view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (container_view);
  GtkWidget             *view      = GTK_WIDGET (insert_data);

  if (view == (GtkWidget *) grid_view->selected_item)
    {
      gchar *name = picman_viewable_get_description (viewable, NULL);

      picman_editor_set_name (PICMAN_EDITOR (container_view), name);
      g_free (name);
    }
}

static gboolean
picman_container_grid_view_select_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  picman_container_grid_view_highlight_item (view, viewable, insert_data);

  return TRUE;
}

static void
picman_container_grid_view_clear_items (PicmanContainerView *view)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (view);

  grid_view->selected_item = NULL;

  while (GTK_WRAP_BOX (grid_view->wrap_box)->children)
    gtk_widget_destroy (GTK_WRAP_BOX (grid_view->wrap_box)->children->widget);

  parent_view_iface->clear_items (view);
}

static void
picman_container_grid_view_set_view_size (PicmanContainerView *view)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (view);
  GtkWrapBoxChild       *child;
  gint                   view_size;

  view_size = picman_container_view_get_view_size (view, NULL);

  for (child = GTK_WRAP_BOX (grid_view->wrap_box)->children;
       child;
       child = child->next)
    {
      PicmanView *view = PICMAN_VIEW (child->widget);

      picman_view_renderer_set_size (view->renderer,
                                   view_size,
                                   view->renderer->border_width);
    }

  gtk_widget_queue_resize (grid_view->wrap_box);
}

static gboolean
picman_container_grid_view_item_selected (GtkWidget      *widget,
                                        GdkEventButton *bevent,
                                        gpointer        data)
{
  if (bevent->type == GDK_BUTTON_PRESS && bevent->button == 1)
    {
      if (gtk_widget_get_can_focus (data) && ! gtk_widget_has_focus (data))
        gtk_widget_grab_focus (GTK_WIDGET (data));

      picman_container_view_item_selected (PICMAN_CONTAINER_VIEW (data),
                                         PICMAN_VIEW (widget)->viewable);
    }

  return FALSE;
}

static void
picman_container_grid_view_item_activated (GtkWidget *widget,
                                         gpointer   data)
{
  picman_container_view_item_activated (PICMAN_CONTAINER_VIEW (data),
                                      PICMAN_VIEW (widget)->viewable);
}

static void
picman_container_grid_view_item_context (GtkWidget *widget,
                                       gpointer   data)
{
  /*  ref the view because calling picman_container_view_item_selected()
   *  may destroy the widget
   */
  g_object_ref (data);

  if (picman_container_view_item_selected (PICMAN_CONTAINER_VIEW (data),
                                         PICMAN_VIEW (widget)->viewable))
    {
      picman_container_view_item_context (PICMAN_CONTAINER_VIEW (data),
                                        PICMAN_VIEW (widget)->viewable);
    }

  g_object_unref (data);
}

static void
picman_container_grid_view_highlight_item (PicmanContainerView *container_view,
                                         PicmanViewable      *viewable,
                                         gpointer           insert_data)
{
  PicmanContainerGridView *grid_view = PICMAN_CONTAINER_GRID_VIEW (container_view);
  PicmanContainerBox      *box       = PICMAN_CONTAINER_BOX (container_view);
  PicmanContainer         *container;
  PicmanView              *view      = NULL;

  container = picman_container_view_get_container (container_view);

  if (insert_data)
    view = PICMAN_VIEW (insert_data);

  if (grid_view->selected_item && grid_view->selected_item != view)
    {
      picman_view_renderer_set_border_type (grid_view->selected_item->renderer,
                                          PICMAN_VIEW_BORDER_WHITE);
      picman_view_renderer_update (grid_view->selected_item->renderer);
    }

  if (view)
    {
      GtkRequisition  view_requisition;
      GtkAdjustment  *adj;
      gint            item_height;
      gint            index;
      gint            row;
      gchar          *name;

      adj = gtk_scrolled_window_get_vadjustment
        (GTK_SCROLLED_WINDOW (box->scrolled_win));

      gtk_widget_size_request (GTK_WIDGET (view), &view_requisition);

      item_height = view_requisition.height;

      index = picman_container_get_child_index (container,
                                              PICMAN_OBJECT (viewable));

      row = index / grid_view->columns;

      if (row * item_height < gtk_adjustment_get_value (adj))
        {
          gtk_adjustment_set_value (adj, row * item_height);
        }
      else if ((row + 1) * item_height > (gtk_adjustment_get_value (adj) +
                                          gtk_adjustment_get_page_size (adj)))
        {
          gtk_adjustment_set_value (adj,
                                    (row + 1) * item_height -
                                    gtk_adjustment_get_page_size (adj));
        }

      picman_view_renderer_set_border_type (view->renderer,
                                          PICMAN_VIEW_BORDER_BLACK);
      picman_view_renderer_update (view->renderer);

      name = picman_viewable_get_description (view->renderer->viewable, NULL);
      picman_editor_set_name (PICMAN_EDITOR (grid_view), name);
      g_free (name);
    }
  else
    {
      picman_editor_set_name (PICMAN_EDITOR (grid_view), NULL);
    }

  grid_view->selected_item = view;
}

static void
picman_container_grid_view_viewport_resized (GtkWidget             *widget,
                                           GtkAllocation         *allocation,
                                           PicmanContainerGridView *grid_view)
{
  PicmanContainerView *container_view = PICMAN_CONTAINER_VIEW (grid_view);

  if (picman_container_view_get_container (container_view))
    {
      GList *children;
      gint   n_children;

      children = gtk_container_get_children (GTK_CONTAINER (grid_view->wrap_box));
      n_children = g_list_length (children);

      if (children)
        {
          GtkRequisition view_requisition;
          gint           columns;
          gint           rows;

          gtk_widget_size_request (GTK_WIDGET (children->data),
                                   &view_requisition);

          g_list_free (children);

          columns = MAX (1, allocation->width / view_requisition.width);

          rows = n_children / columns;

          if (n_children % columns)
            rows++;

          if ((rows != grid_view->rows) || (columns != grid_view->columns))
            {
              grid_view->rows    = rows;
              grid_view->columns = columns;

              gtk_widget_set_size_request (grid_view->wrap_box,
                                           columns * view_requisition.width,
                                           rows    * view_requisition.height);

            }

          grid_view->visible_rows = (allocation->height /
                                     view_requisition.height);
        }

      if (grid_view->selected_item)
        {
          PicmanView *view = grid_view->selected_item;

          picman_container_grid_view_highlight_item (container_view,
                                                   view->viewable,
                                                   view);
        }
    }
}

static gboolean
picman_container_grid_view_button_press (GtkWidget             *widget,
                                       GdkEventButton        *bevent,
                                       PicmanContainerGridView *grid_view)
{
  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      picman_editor_popup_menu (PICMAN_EDITOR (grid_view), NULL, NULL);
    }

  return TRUE;
}
