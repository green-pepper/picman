/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreeview.c
 * Copyright (C) 2003-2010 Michael Natterer <mitch@picman.org>
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

#include <string.h>

#include <gegl.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmanviewable.h"

#include "picmancellrendererviewable.h"
#include "picmancontainertreestore.h"
#include "picmancontainertreeview.h"
#include "picmancontainertreeview-dnd.h"
#include "picmancontainertreeview.h"
#include "picmancontainertreeview-private.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"


enum
{
  EDIT_NAME,
  LAST_SIGNAL
};


static void          picman_container_tree_view_view_iface_init   (PicmanContainerViewInterface  *iface);

static void          picman_container_tree_view_constructed       (GObject                     *object);
static void          picman_container_tree_view_finalize          (GObject                     *object);

static void          picman_container_tree_view_unmap             (GtkWidget                   *widget);
static gboolean      picman_container_tree_view_popup_menu        (GtkWidget                   *widget);

static void          picman_container_tree_view_set_container     (PicmanContainerView           *view,
                                                                 PicmanContainer               *container);
static void          picman_container_tree_view_set_context       (PicmanContainerView           *view,
                                                                 PicmanContext                 *context);
static void          picman_container_tree_view_set_selection_mode(PicmanContainerView           *view,
                                                                 GtkSelectionMode             mode);

static gpointer      picman_container_tree_view_insert_item       (PicmanContainerView           *view,
                                                                 PicmanViewable                *viewable,
                                                                 gpointer                     parent_insert_data,
                                                                 gint                         index);
static void          picman_container_tree_view_remove_item       (PicmanContainerView           *view,
                                                                 PicmanViewable                *viewable,
                                                                 gpointer                     insert_data);
static void          picman_container_tree_view_reorder_item      (PicmanContainerView           *view,
                                                                 PicmanViewable                *viewable,
                                                                 gint                         new_index,
                                                                 gpointer                     insert_data);
static void          picman_container_tree_view_rename_item       (PicmanContainerView           *view,
                                                                 PicmanViewable                *viewable,
                                                                 gpointer                     insert_data);
static gboolean      picman_container_tree_view_select_item       (PicmanContainerView           *view,
                                                                 PicmanViewable                *viewable,
                                                                 gpointer                     insert_data);
static void          picman_container_tree_view_clear_items       (PicmanContainerView           *view);
static void          picman_container_tree_view_set_view_size     (PicmanContainerView           *view);

static void          picman_container_tree_view_real_edit_name    (PicmanContainerTreeView       *tree_view);

static void          picman_container_tree_view_name_started      (GtkCellRendererText         *cell,
                                                                 GtkCellEditable             *editable,
                                                                 const gchar                 *path_str,
                                                                 PicmanContainerTreeView       *tree_view);
static void          picman_container_tree_view_name_canceled     (GtkCellRendererText         *cell,
                                                                 PicmanContainerTreeView       *tree_view);

static void          picman_container_tree_view_selection_changed (GtkTreeSelection            *sel,
                                                                 PicmanContainerTreeView       *tree_view);
static gboolean      picman_container_tree_view_button_press      (GtkWidget                   *widget,
                                                                 GdkEventButton              *bevent,
                                                                 PicmanContainerTreeView       *tree_view);
static gboolean      picman_container_tree_view_tooltip           (GtkWidget                   *widget,
                                                                 gint                         x,
                                                                 gint                         y,
                                                                 gboolean                     keyboard_tip,
                                                                 GtkTooltip                  *tooltip,
                                                                 PicmanContainerTreeView       *tree_view);
static PicmanViewable *picman_container_tree_view_drag_viewable     (GtkWidget                   *widget,
                                                                 PicmanContext                **context,
                                                                 gpointer                     data);
static GdkPixbuf    *picman_container_tree_view_drag_pixbuf       (GtkWidget                   *widget,
                                                                 gpointer                     data);

static gboolean      picman_container_tree_view_get_selected_single (PicmanContainerTreeView  *tree_view,
                                                                   GtkTreeIter            *iter);
static gint          picman_container_tree_view_get_selected        (PicmanContainerView    *view,
                                                                   GList               **items);
static void          picman_container_tree_view_row_expanded        (GtkTreeView               *tree_view,
                                                                   GtkTreeIter               *iter,
                                                                   GtkTreePath               *path,
                                                                   PicmanContainerTreeView     *view);
static void          picman_container_tree_view_expand_rows         (GtkTreeModel             *model,
                                                                   GtkTreeView              *view,
                                                                   GtkTreeIter              *parent);


G_DEFINE_TYPE_WITH_CODE (PicmanContainerTreeView, picman_container_tree_view,
                         PICMAN_TYPE_CONTAINER_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_container_tree_view_view_iface_init))

#define parent_class picman_container_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;

static guint tree_view_signals[LAST_SIGNAL] = { 0 };


static void
picman_container_tree_view_class_init (PicmanContainerTreeViewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkBindingSet  *binding_set;

  object_class->constructed = picman_container_tree_view_constructed;
  object_class->finalize    = picman_container_tree_view_finalize;

  widget_class->unmap       = picman_container_tree_view_unmap;
  widget_class->popup_menu  = picman_container_tree_view_popup_menu;

  klass->edit_name          = picman_container_tree_view_real_edit_name;
  klass->drop_possible      = picman_container_tree_view_real_drop_possible;
  klass->drop_viewable      = picman_container_tree_view_real_drop_viewable;
  klass->drop_color         = NULL;
  klass->drop_uri_list      = NULL;
  klass->drop_svg           = NULL;
  klass->drop_component     = NULL;
  klass->drop_pixbuf        = NULL;

  tree_view_signals[EDIT_NAME] =
    g_signal_new ("edit-name",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanContainerTreeViewClass, edit_name),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_F2, 0,
                                "edit-name", 0);

  g_type_class_add_private (klass, sizeof (PicmanContainerTreeViewPriv));
}

static void
picman_container_tree_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  if (! parent_view_iface)
    parent_view_iface = g_type_default_interface_peek (PICMAN_TYPE_CONTAINER_VIEW);

  iface->set_container      = picman_container_tree_view_set_container;
  iface->set_context        = picman_container_tree_view_set_context;
  iface->set_selection_mode = picman_container_tree_view_set_selection_mode;
  iface->insert_item        = picman_container_tree_view_insert_item;
  iface->remove_item        = picman_container_tree_view_remove_item;
  iface->reorder_item       = picman_container_tree_view_reorder_item;
  iface->rename_item        = picman_container_tree_view_rename_item;
  iface->select_item        = picman_container_tree_view_select_item;
  iface->clear_items        = picman_container_tree_view_clear_items;
  iface->set_view_size      = picman_container_tree_view_set_view_size;
  iface->get_selected       = picman_container_tree_view_get_selected;

  iface->insert_data_free = (GDestroyNotify) gtk_tree_iter_free;
}

static void
picman_container_tree_view_init (PicmanContainerTreeView *tree_view)
{
  PicmanContainerBox *box = PICMAN_CONTAINER_BOX (tree_view);

  tree_view->priv = G_TYPE_INSTANCE_GET_PRIVATE (tree_view,
                                                 PICMAN_TYPE_CONTAINER_TREE_VIEW,
                                                 PicmanContainerTreeViewPriv);

  picman_container_tree_store_columns_init (tree_view->model_columns,
                                          &tree_view->n_model_columns);

  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (box->scrolled_win),
                                       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (box->scrolled_win),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
}

static void
picman_container_tree_view_constructed (GObject *object)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (object);
  PicmanContainerView     *view      = PICMAN_CONTAINER_VIEW (object);
  PicmanContainerBox      *box       = PICMAN_CONTAINER_BOX (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  tree_view->model = picman_container_tree_store_new (view,
                                                    tree_view->n_model_columns,
                                                    tree_view->model_columns);

  tree_view->view = g_object_new (GTK_TYPE_TREE_VIEW,
                                  "model",           tree_view->model,
                                  "search-column",   PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME,
                                  "enable-search",   FALSE,
                                  "headers-visible", FALSE,
                                  "has-tooltip",     TRUE,
                                  "show-expanders",  PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->model_is_tree,
                                  NULL);
  g_object_unref (tree_view->model);

  gtk_container_add (GTK_CONTAINER (box->scrolled_win),
                     GTK_WIDGET (tree_view->view));
  gtk_widget_show (GTK_WIDGET (tree_view->view));

  picman_container_view_set_dnd_widget (view, GTK_WIDGET (tree_view->view));

  tree_view->main_column = gtk_tree_view_column_new ();
  gtk_tree_view_insert_column (tree_view->view, tree_view->main_column, 0);

  gtk_tree_view_set_expander_column (tree_view->view, tree_view->main_column);
  gtk_tree_view_set_enable_tree_lines (tree_view->view, TRUE);

  tree_view->renderer_cell = picman_cell_renderer_viewable_new ();
  gtk_tree_view_column_pack_start (tree_view->main_column,
                                   tree_view->renderer_cell,
                                   FALSE);

  gtk_tree_view_column_set_attributes (tree_view->main_column,
                                       tree_view->renderer_cell,
                                       "renderer", PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER,
                                       NULL);

  tree_view->priv->name_cell = gtk_cell_renderer_text_new ();
  g_object_set (tree_view->priv->name_cell, "xalign", 0.0, NULL);
  gtk_tree_view_column_pack_end (tree_view->main_column,
                                 tree_view->priv->name_cell,
                                 FALSE);

  gtk_tree_view_column_set_attributes (tree_view->main_column,
                                       tree_view->priv->name_cell,
                                       "text",       PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME,
                                       "attributes", PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES,
                                       "sensitive",  PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE,
                                       NULL);

  g_signal_connect (tree_view->priv->name_cell, "editing-started",
                    G_CALLBACK (picman_container_tree_view_name_started),
                    tree_view);
  g_signal_connect (tree_view->priv->name_cell, "editing-canceled",
                    G_CALLBACK (picman_container_tree_view_name_canceled),
                    tree_view);

  picman_container_tree_view_add_renderer_cell (tree_view,
                                              tree_view->renderer_cell);

  tree_view->priv->selection = gtk_tree_view_get_selection (tree_view->view);

  g_signal_connect (tree_view->priv->selection, "changed",
                    G_CALLBACK (picman_container_tree_view_selection_changed),
                    tree_view);

  g_signal_connect (tree_view->view, "drag-leave",
                    G_CALLBACK (picman_container_tree_view_drag_leave),
                    tree_view);
  g_signal_connect (tree_view->view, "drag-motion",
                    G_CALLBACK (picman_container_tree_view_drag_motion),
                    tree_view);
  g_signal_connect (tree_view->view, "drag-drop",
                    G_CALLBACK (picman_container_tree_view_drag_drop),
                    tree_view);
  g_signal_connect (tree_view->view, "drag-data-received",
                    G_CALLBACK (picman_container_tree_view_drag_data_received),
                    tree_view);

  g_signal_connect (tree_view->view, "query-tooltip",
                    G_CALLBACK (picman_container_tree_view_tooltip),
                    tree_view);
}

static void
picman_container_tree_view_finalize (GObject *object)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (object);

  if (tree_view->priv->toggle_cells)
    {
      g_list_free (tree_view->priv->toggle_cells);
      tree_view->priv->toggle_cells = NULL;
    }

  if (tree_view->priv->renderer_cells)
    {
      g_list_free (tree_view->priv->renderer_cells);
      tree_view->priv->renderer_cells = NULL;
    }

  if (tree_view->priv->editable_cells)
    {
      g_list_free (tree_view->priv->editable_cells);
      tree_view->priv->editable_cells = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_container_tree_view_unmap (GtkWidget *widget)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (widget);

  if (tree_view->priv->scroll_timeout_id)
    {
      g_source_remove (tree_view->priv->scroll_timeout_id);
      tree_view->priv->scroll_timeout_id = 0;
    }

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_container_tree_view_menu_position (GtkMenu  *menu,
                                        gint     *x,
                                        gint     *y,
                                        gpointer  data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (data);
  GtkWidget             *widget    = GTK_WIDGET (tree_view->view);
  GtkAllocation          allocation;
  GtkTreeIter            selected_iter;

  gtk_widget_get_allocation (widget, &allocation);

  gdk_window_get_origin (gtk_widget_get_window (widget), x, y);

  if (! gtk_widget_get_has_window (widget))
    {
      *x += allocation.x;
      *y += allocation.y;
    }

  if (picman_container_tree_view_get_selected_single (tree_view, &selected_iter))
    {
      GtkTreePath  *path;
      GdkRectangle  cell_rect;
      gint          center;

      path = gtk_tree_model_get_path (tree_view->model, &selected_iter);
      gtk_tree_view_get_cell_area (tree_view->view, path,
                                   tree_view->main_column, &cell_rect);
      gtk_tree_path_free (path);

      center = cell_rect.y + cell_rect.height / 2;
      center = CLAMP (center, 0, allocation.height);

      *x += allocation.width / 2;
      *y += center;
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
picman_container_tree_view_popup_menu (GtkWidget *widget)
{
  return picman_editor_popup_menu (PICMAN_EDITOR (widget),
                                 picman_container_tree_view_menu_position,
                                 widget);
}

GtkWidget *
picman_container_tree_view_new (PicmanContainer *container,
                              PicmanContext   *context,
                              gint           view_size,
                              gint           view_border_width)
{
  PicmanContainerTreeView *tree_view;
  PicmanContainerView     *view;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);

  tree_view = g_object_new (PICMAN_TYPE_CONTAINER_TREE_VIEW, NULL);

  view = PICMAN_CONTAINER_VIEW (tree_view);

  picman_container_view_set_view_size (view, view_size, view_border_width);

  if (container)
    picman_container_view_set_container (view, container);

  if (context)
    picman_container_view_set_context (view, context);

  return GTK_WIDGET (tree_view);
}

void
picman_container_tree_view_set_main_column_title (PicmanContainerTreeView *tree_view,
                                                const gchar           *title)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_VIEW (tree_view));

  gtk_tree_view_column_set_title (tree_view->main_column,
                                  title);
}

void
picman_container_tree_view_add_toggle_cell (PicmanContainerTreeView *tree_view,
                                          GtkCellRenderer       *cell)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_VIEW (tree_view));
  g_return_if_fail (PICMAN_IS_CELL_RENDERER_TOGGLE (cell));

  tree_view->priv->toggle_cells = g_list_prepend (tree_view->priv->toggle_cells,
                                                  cell);
}

void
picman_container_tree_view_add_renderer_cell (PicmanContainerTreeView *tree_view,
                                            GtkCellRenderer       *cell)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_VIEW (tree_view));
  g_return_if_fail (PICMAN_IS_CELL_RENDERER_VIEWABLE (cell));

  tree_view->priv->renderer_cells = g_list_prepend (tree_view->priv->renderer_cells,
                                                    cell);

  picman_container_tree_store_add_renderer_cell (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                               cell);
}

void
picman_container_tree_view_set_dnd_drop_to_empty (PicmanContainerTreeView *tree_view,
                                                gboolean               dnd_drop_to_empty)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_VIEW (tree_view));

  tree_view->priv->dnd_drop_to_empty = dnd_drop_to_empty;
}

void
picman_container_tree_view_connect_name_edited (PicmanContainerTreeView *tree_view,
                                              GCallback              callback,
                                              gpointer               data)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_VIEW (tree_view));
  g_return_if_fail (callback != NULL);

  g_object_set (tree_view->priv->name_cell,
                "mode",     GTK_CELL_RENDERER_MODE_EDITABLE,
                "editable", TRUE,
                NULL);

  if (! g_list_find (tree_view->priv->editable_cells, tree_view->priv->name_cell))
    tree_view->priv->editable_cells = g_list_prepend (tree_view->priv->editable_cells,
                                                      tree_view->priv->name_cell);

  g_signal_connect (tree_view->priv->name_cell, "edited",
                    callback,
                    data);
}


/*  PicmanContainerView methods  */

static void
picman_container_tree_view_set_container (PicmanContainerView *view,
                                        PicmanContainer     *container)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  PicmanContainer         *old_container;

  old_container = picman_container_view_get_container (view);

  if (old_container)
    {
      tree_view->priv->dnd_renderer = NULL;

      g_signal_handlers_disconnect_by_func (tree_view->view,
                                            picman_container_tree_view_row_expanded,
                                            tree_view);
      if (! container)
        {
          if (picman_dnd_viewable_source_remove (GTK_WIDGET (tree_view->view),
                                               picman_container_get_children_type (old_container)))
            {
              if (PICMAN_VIEWABLE_CLASS (g_type_class_peek (picman_container_get_children_type (old_container)))->get_size)
                picman_dnd_pixbuf_source_remove (GTK_WIDGET (tree_view->view));

              gtk_drag_source_unset (GTK_WIDGET (tree_view->view));
            }

          g_signal_handlers_disconnect_by_func (tree_view->view,
                                                picman_container_tree_view_button_press,
                                                tree_view);
        }
    }
  else if (container)
    {
      if (picman_dnd_drag_source_set_by_type (GTK_WIDGET (tree_view->view),
                                            GDK_BUTTON1_MASK | GDK_BUTTON2_MASK,
                                            picman_container_get_children_type (container),
                                            GDK_ACTION_COPY))
        {
          picman_dnd_viewable_source_add (GTK_WIDGET (tree_view->view),
                                        picman_container_get_children_type (container),
                                        picman_container_tree_view_drag_viewable,
                                        tree_view);

          if (PICMAN_VIEWABLE_CLASS (g_type_class_peek (picman_container_get_children_type (container)))->get_size)
            picman_dnd_pixbuf_source_add (GTK_WIDGET (tree_view->view),
                                        picman_container_tree_view_drag_pixbuf,
                                        tree_view);
        }

      /*  connect button_press_event after DND so we can keep the list from
       *  selecting the item on button2
       */
      g_signal_connect (tree_view->view, "button-press-event",
                        G_CALLBACK (picman_container_tree_view_button_press),
                        tree_view);
    }

  parent_view_iface->set_container (view, container);

  if (container)
    {
      picman_container_tree_view_expand_rows (tree_view->model,
                                            tree_view->view,
                                            NULL);

      g_signal_connect (tree_view->view,
                        "row-collapsed",
                        G_CALLBACK (picman_container_tree_view_row_expanded),
                        tree_view);
      g_signal_connect (tree_view->view,
                        "row-expanded",
                        G_CALLBACK (picman_container_tree_view_row_expanded),
                        tree_view);
    }

  gtk_tree_view_columns_autosize (tree_view->view);
}

static void
picman_container_tree_view_set_context (PicmanContainerView *view,
                                      PicmanContext       *context)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  if (tree_view->model)
    picman_container_tree_store_set_context (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                           context);

  parent_view_iface->set_context (view, context);
}

static void
picman_container_tree_view_set_selection_mode (PicmanContainerView *view,
                                             GtkSelectionMode   mode)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  gtk_tree_selection_set_mode (tree_view->priv->selection, mode);

  parent_view_iface->set_selection_mode (view, mode);
}

static gpointer
picman_container_tree_view_insert_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           parent_insert_data,
                                      gint               index)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter;

  iter = picman_container_tree_store_insert_item (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                                viewable,
                                                parent_insert_data,
                                                index);
  return iter;
}

static void
picman_container_tree_view_remove_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter      = (GtkTreeIter *) insert_data;

  picman_container_tree_store_remove_item (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                         viewable,
                                         iter);

  if (iter)
    gtk_tree_view_columns_autosize (tree_view->view);
}

static void
picman_container_tree_view_reorder_item (PicmanContainerView *view,
                                       PicmanViewable      *viewable,
                                       gint               new_index,
                                       gpointer           insert_data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter      = (GtkTreeIter *) insert_data;
  gboolean               selected  = FALSE;

  if (iter)
    {
      GtkTreeIter selected_iter;

      selected = picman_container_tree_view_get_selected_single (tree_view,
                                                               &selected_iter);

      if (selected)
        {
          PicmanViewRenderer *renderer;

          gtk_tree_model_get (tree_view->model, &selected_iter,
                              PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                              -1);

          if (renderer->viewable != viewable)
            selected = FALSE;

          g_object_unref (renderer);
        }
    }

  picman_container_tree_store_reorder_item (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                          viewable,
                                          new_index,
                                          iter);

  if (selected)
    picman_container_view_select_item (view, viewable);
}

static void
picman_container_tree_view_rename_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter      = (GtkTreeIter *) insert_data;

  if (picman_container_tree_store_rename_item (PICMAN_CONTAINER_TREE_STORE (tree_view->model),
                                             viewable,
                                             iter))
    {
      gtk_tree_view_columns_autosize (tree_view->view);
    }
}

static gboolean
picman_container_tree_view_select_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  if (viewable && insert_data)
    {
      GtkTreePath *path;
      GtkTreePath *parent_path;
      GtkTreeIter *iter = (GtkTreeIter *) insert_data;

      path = gtk_tree_model_get_path (tree_view->model, iter);

      parent_path = gtk_tree_path_copy (path);

      if (gtk_tree_path_up (parent_path))
        gtk_tree_view_expand_to_path (tree_view->view, parent_path);

      gtk_tree_path_free (parent_path);

      g_signal_handlers_block_by_func (tree_view->priv->selection,
                                       picman_container_tree_view_selection_changed,
                                       tree_view);

      gtk_tree_view_set_cursor (tree_view->view, path, NULL, FALSE);

      g_signal_handlers_unblock_by_func (tree_view->priv->selection,
                                         picman_container_tree_view_selection_changed,
                                         tree_view);

      gtk_tree_view_scroll_to_cell (tree_view->view, path,
                                    NULL, FALSE, 0.0, 0.0);

      gtk_tree_path_free (path);
    }
  else if (insert_data == NULL)
    {
      /* viewable == NULL && insert_data != NULL means multiple selection.
       * viewable == NULL && insert_data == NULL means no selection. */
      gtk_tree_selection_unselect_all (tree_view->priv->selection);
    }

  return TRUE;
}

static void
picman_container_tree_view_clear_items (PicmanContainerView *view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  picman_container_tree_store_clear_items (PICMAN_CONTAINER_TREE_STORE (tree_view->model));

  parent_view_iface->clear_items (view);
}

static void
picman_container_tree_view_set_view_size (PicmanContainerView *view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkWidget             *tree_widget;
  GList                 *list;
  gint                   view_size;
  gint                   border_width;

  view_size = picman_container_view_get_view_size (view, &border_width);

  if (tree_view->model)
    picman_container_tree_store_set_view_size (PICMAN_CONTAINER_TREE_STORE (tree_view->model));

  tree_widget = GTK_WIDGET (tree_view->view);

  if (! tree_widget)
    return;

  for (list = tree_view->priv->toggle_cells; list; list = g_list_next (list))
    {
      gchar       *stock_id;
      GtkIconSize  icon_size;

      g_object_get (list->data, "stock-id", &stock_id, NULL);

      if (stock_id)
        {
          GtkStyle *style = gtk_widget_get_style (tree_widget);

          icon_size = picman_get_icon_size (tree_widget,
                                          stock_id,
                                          GTK_ICON_SIZE_BUTTON,
                                          view_size -
                                          2 * style->xthickness,
                                          view_size -
                                          2 * style->ythickness);

          g_object_set (list->data, "stock-size", icon_size, NULL);

          g_free (stock_id);
        }
    }

  gtk_tree_view_columns_autosize (tree_view->view);
}


/*  PicmanContainerTreeView methods  */

static void
picman_container_tree_view_real_edit_name (PicmanContainerTreeView *tree_view)
{
  GtkTreeIter selected_iter;

  if (g_list_find (tree_view->priv->editable_cells,
                   tree_view->priv->name_cell) &&
      picman_container_tree_view_get_selected_single (tree_view,
                                                    &selected_iter))
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (tree_view->model, &selected_iter);

      gtk_tree_view_set_cursor_on_cell (tree_view->view, path,
                                        tree_view->main_column,
                                        tree_view->priv->name_cell,
                                        TRUE);

      gtk_tree_path_free (path);
    }
}


/*  callbacks  */

static void
picman_container_tree_view_name_started (GtkCellRendererText   *cell,
                                       GtkCellEditable       *editable,
                                       const gchar           *path_str,
                                       PicmanContainerTreeView *tree_view)
{
  GtkTreePath *path;
  GtkTreeIter  iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      const gchar      *real_name;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      real_name = picman_object_get_name (renderer->viewable);

      g_object_unref (renderer);

      gtk_entry_set_text (GTK_ENTRY (editable), real_name);
    }

  gtk_tree_path_free (path);
}

static void
picman_container_tree_view_name_canceled (GtkCellRendererText   *cell,
                                        PicmanContainerTreeView *tree_view)
{
  GtkTreeIter iter;

  if (picman_container_tree_view_get_selected_single (tree_view, &iter))
    {
      PicmanViewRenderer *renderer;
      gchar            *name;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      name = picman_viewable_get_description (renderer->viewable, NULL);

      gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, name,
                          -1);

      g_free (name);
      g_object_unref (renderer);
    }
}

static void
picman_container_tree_view_selection_changed (GtkTreeSelection      *selection,
                                            PicmanContainerTreeView *tree_view)
{
  PicmanContainerView    *view = PICMAN_CONTAINER_VIEW (tree_view);
  GList                *items;

  picman_container_tree_view_get_selected (view, &items);
  picman_container_view_multi_selected (view, items);
  g_list_free (items);
}

static GtkCellRenderer *
picman_container_tree_view_find_click_cell (GtkWidget         *widget,
                                          GList             *cells,
                                          GtkTreeViewColumn *column,
                                          GdkRectangle      *column_area,
                                          gint               tree_x,
                                          gint               tree_y)
{
  GList    *list;
  gboolean  rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  for (list = cells; list; list = g_list_next (list))
    {
      GtkCellRenderer *renderer = list->data;
      gint             start;
      gint             width;

      if (gtk_cell_renderer_get_visible (renderer) &&
          gtk_tree_view_column_cell_get_position (column, renderer,
                                                  &start, &width))
        {
          gint xpad, ypad;
          gint x;

          gtk_cell_renderer_get_padding (renderer, &xpad, &ypad);

          if (rtl)
            x = column_area->x + column_area->width - start - width;
          else
            x = start + column_area->x;

          if (tree_x >= x + xpad &&
              tree_x <  x + width - xpad)
            {
              return renderer;
            }
        }
    }

  return NULL;
}

static gboolean
picman_container_tree_view_button_press (GtkWidget             *widget,
                                       GdkEventButton        *bevent,
                                       PicmanContainerTreeView *tree_view)
{
  PicmanContainerView *container_view = PICMAN_CONTAINER_VIEW (tree_view);
  GtkTreeViewColumn *column;
  GtkTreePath       *path;

  tree_view->priv->dnd_renderer = NULL;

  if (! gtk_widget_has_focus (widget))
    gtk_widget_grab_focus (widget);

  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
                                     bevent->x, bevent->y,
                                     &path, &column, NULL, NULL))
    {
      PicmanViewRenderer         *renderer;
      PicmanCellRendererToggle   *toggled_cell = NULL;
      PicmanCellRendererViewable *clicked_cell = NULL;
      GtkCellRenderer          *edit_cell    = NULL;
      GdkRectangle              column_area;
      GtkTreeIter               iter;
      gboolean                  handled = TRUE;
      gboolean                  multisel_mode;

      multisel_mode = (gtk_tree_selection_get_mode (tree_view->priv->selection)
                       == GTK_SELECTION_MULTIPLE);

      gtk_tree_model_get_iter (tree_view->model, &iter, path);

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      tree_view->priv->dnd_renderer = renderer;

      gtk_tree_view_get_cell_area (tree_view->view, path,
                                   column, &column_area);

      gtk_tree_view_column_cell_set_cell_data (column,
                                               tree_view->model,
                                               &iter,
                                               FALSE, FALSE);

      if (bevent->button == 1                                     &&
          gtk_tree_model_iter_has_child (tree_view->model, &iter) &&
          column == gtk_tree_view_get_expander_column (tree_view->view))
        {
          GList *cells;

          cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (column));

          if (! picman_container_tree_view_find_click_cell (widget,
                                                          cells,
                                                          column, &column_area,
                                                          bevent->x, bevent->y))
            {
              /*  we didn't click on any cell, but we clicked on empty
               *  space in the expander column of a row that has
               *  children; let GtkTreeView process the button press
               *  to maybe handle a click on an expander.
               */
              g_list_free (cells);
              gtk_tree_path_free (path);
              g_object_unref (renderer);

              return FALSE;
            }

          g_list_free (cells);
        }

      toggled_cell = (PicmanCellRendererToggle *)
        picman_container_tree_view_find_click_cell (widget,
                                                  tree_view->priv->toggle_cells,
                                                  column, &column_area,
                                                  bevent->x, bevent->y);

      if (! toggled_cell)
        clicked_cell = (PicmanCellRendererViewable *)
          picman_container_tree_view_find_click_cell (widget,
                                                    tree_view->priv->renderer_cells,
                                                    column, &column_area,
                                                    bevent->x, bevent->y);

      if (! toggled_cell && ! clicked_cell)
        edit_cell =
          picman_container_tree_view_find_click_cell (widget,
                                                    tree_view->priv->editable_cells,
                                                    column, &column_area,
                                                    bevent->x, bevent->y);

      g_object_ref (tree_view);

      if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
        {
          if (picman_container_view_item_selected (container_view,
                                                 renderer->viewable))
            {
              if (picman_container_view_get_container (container_view))
                picman_container_view_item_context (container_view,
                                                  renderer->viewable);
            }
        }
      else if (bevent->button == 1)
        {
          if (bevent->type == GDK_BUTTON_PRESS)
            {
              /*  don't select item if a toggle was clicked */
              if (! toggled_cell)
                {
                  gchar *path_str = gtk_tree_path_to_string (path);

                  handled = FALSE;

                  if (clicked_cell)
                    handled =
                      picman_cell_renderer_viewable_pre_clicked (clicked_cell,
                                                               path_str,
                                                               bevent->state);

                  if (! handled)
                    {
                      if (multisel_mode)
                        {
                          /* let parent do the work */
                        }
                      else
                        {
                          handled =
                            picman_container_view_item_selected (container_view,
                                                               renderer->viewable);
                        }
                    }

                  g_free (path_str);
                }

              /*  a callback invoked by selecting the item may have
               *  destroyed us, so check if the container is still there
               */
              if (picman_container_view_get_container (container_view))
                {
                  /*  another row may have been set by selecting  */
                  gtk_tree_view_column_cell_set_cell_data (column,
                                                           tree_view->model,
                                                           &iter,
                                                           FALSE, FALSE);

                  if (toggled_cell || clicked_cell)
                    {
                      gchar *path_str = gtk_tree_path_to_string (path);

                      if (toggled_cell)
                        {
                          picman_cell_renderer_toggle_clicked (toggled_cell,
                                                             path_str,
                                                             bevent->state);
                        }
                      else if (clicked_cell)
                        {
                          picman_cell_renderer_viewable_clicked (clicked_cell,
                                                               path_str,
                                                               bevent->state);
                        }

                      g_free (path_str);
                    }
                }
            }
          else if (bevent->type == GDK_2BUTTON_PRESS)
            {
              gboolean success = TRUE;

              /*  don't select item if a toggle was clicked */
              if (! toggled_cell)
                success = picman_container_view_item_selected (container_view,
                                                             renderer->viewable);

              if (success)
                {
                  if (edit_cell)
                    {
                      gtk_tree_view_set_cursor_on_cell (tree_view->view, path,
                                                        column, edit_cell, TRUE);
                    }
                  else if (! toggled_cell &&
                           ! (bevent->state & picman_get_all_modifiers_mask ()))
                    {
                      /* Only activate if we're not in a toggled cell
                       * and no modifier keys are pressed
                       */
                      picman_container_view_item_activated (container_view,
                                                          renderer->viewable);
                    }
                }
            }
        }
      else if (bevent->button == 2)
        {
          if (bevent->type == GDK_BUTTON_PRESS)
            {
              if (clicked_cell)
                {
                  gchar *path_str = gtk_tree_path_to_string (path);

                  picman_cell_renderer_viewable_clicked (clicked_cell,
                                                       path_str,
                                                       bevent->state);

                  g_free (path_str);
                }
            }
        }

      g_object_unref (tree_view);

      gtk_tree_path_free (path);
      g_object_unref (renderer);

      return multisel_mode ? handled : TRUE;
    }
  else
    {
      if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
        {
          picman_editor_popup_menu (PICMAN_EDITOR (tree_view), NULL, NULL);
        }

      return TRUE;
    }
}

static gboolean
picman_container_tree_view_tooltip (GtkWidget             *widget,
                                  gint                   x,
                                  gint                   y,
                                  gboolean               keyboard_tip,
                                  GtkTooltip            *tooltip,
                                  PicmanContainerTreeView *tree_view)
{
  PicmanViewRenderer *renderer;
  GtkTreeIter       iter;
  GtkTreePath      *path;
  gboolean          show_tip = FALSE;

  if (! gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget), &x, &y,
                                           keyboard_tip,
                                           NULL, &path, &iter))
    return FALSE;

  gtk_tree_model_get (tree_view->model, &iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                      -1);

  if (renderer)
    {
      gchar *desc;
      gchar *tip;

      desc = picman_viewable_get_description (renderer->viewable, &tip);

      if (tip)
        {
          gtk_tooltip_set_text (tooltip, tip);
          gtk_tree_view_set_tooltip_row (GTK_TREE_VIEW (widget), tooltip, path);

          show_tip = TRUE;

          g_free (tip);
        }

      g_free (desc);
      g_object_unref (renderer);
    }

  gtk_tree_path_free (path);

  return show_tip;
}

static PicmanViewable *
picman_container_tree_view_drag_viewable (GtkWidget    *widget,
                                        PicmanContext **context,
                                        gpointer      data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (data);

  if (context)
    *context = picman_container_view_get_context (PICMAN_CONTAINER_VIEW (data));

  if (tree_view->priv->dnd_renderer)
    return tree_view->priv->dnd_renderer->viewable;

  return NULL;
}

static GdkPixbuf *
picman_container_tree_view_drag_pixbuf (GtkWidget *widget,
                                      gpointer   data)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (data);
  PicmanViewRenderer      *renderer  = tree_view->priv->dnd_renderer;
  gint                   width;
  gint                   height;

  if (renderer && picman_viewable_get_size (renderer->viewable, &width, &height))
    return picman_viewable_get_new_pixbuf (renderer->viewable,
                                         renderer->context,
                                         width, height);

  return NULL;
}

static gboolean
picman_container_tree_view_get_selected_single (PicmanContainerTreeView  *tree_view,
                                               GtkTreeIter            *iter)
{
  GtkTreeSelection         *selection;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view->view));

  if (gtk_tree_selection_count_selected_rows (selection) == 1)
    {
      GList    *selected_rows;

      selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);

      gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_view->model), iter,
                               (GtkTreePath *) selected_rows->data);

      g_list_free_full (selected_rows, (GDestroyNotify) gtk_tree_path_free);

      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

static gint
picman_container_tree_view_get_selected (PicmanContainerView    *view,
                                       GList               **items)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeSelection      *selection;
  gint                   selected_count;
  GList                 *selected_rows;
  GList                 *current_row;
  GtkTreeIter            iter;
  PicmanViewRenderer      *renderer;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view->view));
  selected_count = gtk_tree_selection_count_selected_rows (selection);

  if (items == NULL)
    {
      /* just provide selected count */
      return selected_count;
    }

  selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);

  *items = NULL;
  for (current_row = selected_rows;
       current_row;
       current_row = g_list_next (current_row))
    {
      gtk_tree_model_get_iter (GTK_TREE_MODEL (tree_view->model), &iter,
                               (GtkTreePath *) current_row->data);

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      if (renderer->viewable)
        *items = g_list_prepend (*items, renderer->viewable);

      g_object_unref (renderer);
    }

  g_list_free_full (selected_rows, (GDestroyNotify) gtk_tree_path_free);

  *items = g_list_reverse (*items);

  return selected_count;
}

static void
picman_container_tree_view_row_expanded (GtkTreeView           *tree_view,
                                       GtkTreeIter           *iter,
                                       GtkTreePath           *path,
                                       PicmanContainerTreeView *view)
{
  PicmanViewRenderer *renderer;

  gtk_tree_model_get (view->model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                      -1);
  if (renderer)
    {
      gboolean expanded = gtk_tree_view_row_expanded (tree_view, path);

      picman_viewable_set_expanded (renderer->viewable,
                                  expanded);
      if (expanded)
        {
          g_signal_handlers_block_by_func (tree_view,
                                           picman_container_tree_view_row_expanded,
                                           view);

          picman_container_tree_view_expand_rows (view->model, tree_view, iter);

          g_signal_handlers_unblock_by_func (tree_view,
                                             picman_container_tree_view_row_expanded,
                                             view);
        }

      g_object_unref (renderer);
    }
}

static void
picman_container_tree_view_expand_rows (GtkTreeModel *model,
                                      GtkTreeView  *view,
                                      GtkTreeIter  *parent)
{
  GtkTreeIter iter;

  if (gtk_tree_model_iter_children (model, &iter, parent))
    do
      if (gtk_tree_model_iter_has_child (model, &iter))
        {
          PicmanViewRenderer *renderer;

          gtk_tree_model_get (model, &iter,
                              PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                              -1);
          if (renderer)
            {
              GtkTreePath *path = gtk_tree_model_get_path (model, &iter);

              if (picman_viewable_get_expanded (renderer->viewable))
                gtk_tree_view_expand_row (view, path, FALSE);
              else
                gtk_tree_view_collapse_row (view, path);

              gtk_tree_path_free (path);
              g_object_unref (renderer);
            }

          picman_container_tree_view_expand_rows (model, view, &iter);
        }
    while (gtk_tree_model_iter_next (model, &iter));
}
