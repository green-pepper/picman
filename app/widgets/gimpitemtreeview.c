/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanitemtreeview.c
 * Copyright (C) 2001-2011 Michael Natterer <mitch@picman.org>
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
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanchannel.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanitem-exclusive.h"
#include "core/picmanitemundo.h"
#include "core/picmanmarshal.h"
#include "core/picmantreehandler.h"
#include "core/picmanundostack.h"

#include "vectors/picmanvectors.h"

#include "picmancontainertreestore.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanitemtreeview.h"
#include "picmanmenufactory.h"
#include "picmanviewrenderer.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  SET_IMAGE,
  LAST_SIGNAL
};


struct _PicmanItemTreeViewPriv
{
  PicmanImage       *image;

  GtkWidget       *options_box;
  GtkSizeGroup    *options_group;
  GtkWidget       *lock_box;

  GtkWidget       *lock_content_toggle;
  GtkWidget       *lock_position_toggle;

  GtkWidget       *edit_button;
  GtkWidget       *new_button;
  GtkWidget       *raise_button;
  GtkWidget       *lower_button;
  GtkWidget       *duplicate_button;
  GtkWidget       *delete_button;

  gint             model_column_visible;
  gint             model_column_viewable;
  gint             model_column_linked;
  GtkCellRenderer *eye_cell;
  GtkCellRenderer *chain_cell;

  PicmanTreeHandler *visible_changed_handler;
  PicmanTreeHandler *linked_changed_handler;
  PicmanTreeHandler *lock_content_changed_handler;
  PicmanTreeHandler *lock_position_changed_handler;

  gboolean         inserting_item; /* EEK */
};


static void   picman_item_tree_view_view_iface_init   (PicmanContainerViewInterface *view_iface);
static void   picman_item_tree_view_docked_iface_init (PicmanDockedInterface *docked_iface);

static void   picman_item_tree_view_constructed       (GObject           *object);
static void   picman_item_tree_view_dispose           (GObject           *object);

static void   picman_item_tree_view_style_set         (GtkWidget         *widget,
                                                     GtkStyle          *prev_style);

static void   picman_item_tree_view_real_set_image    (PicmanItemTreeView  *view,
                                                     PicmanImage         *image);

static void   picman_item_tree_view_image_flush       (PicmanImage         *image,
                                                     gboolean           invalidate_preview,
                                                     PicmanItemTreeView  *view);

static void   picman_item_tree_view_set_container     (PicmanContainerView *view,
                                                     PicmanContainer     *container);
static void   picman_item_tree_view_set_context       (PicmanContainerView *view,
                                                     PicmanContext       *context);

static gpointer picman_item_tree_view_insert_item     (PicmanContainerView *view,
                                                     PicmanViewable      *viewable,
                                                     gpointer           parent_insert_data,
                                                     gint               index);
static void   picman_item_tree_view_insert_item_after (PicmanContainerView *view,
                                                     PicmanViewable      *viewable,
                                                     gpointer           insert_data);
static gboolean picman_item_tree_view_select_item     (PicmanContainerView *view,
                                                     PicmanViewable      *item,
                                                     gpointer           insert_data);
static void   picman_item_tree_view_activate_item     (PicmanContainerView *view,
                                                     PicmanViewable      *item,
                                                     gpointer           insert_data);
static void   picman_item_tree_view_context_item      (PicmanContainerView *view,
                                                     PicmanViewable      *item,
                                                     gpointer           insert_data);

static gboolean picman_item_tree_view_drop_possible   (PicmanContainerTreeView *view,
                                                     PicmanDndType        src_type,
                                                     PicmanViewable      *src_viewable,
                                                     PicmanViewable      *dest_viewable,
                                                     GtkTreePath       *drop_path,
                                                     GtkTreeViewDropPosition  drop_pos,
                                                     GtkTreeViewDropPosition *return_drop_pos,
                                                     GdkDragAction     *return_drag_action);
static void     picman_item_tree_view_drop_viewable   (PicmanContainerTreeView *view,
                                                     PicmanViewable      *src_viewable,
                                                     PicmanViewable      *dest_viewable,
                                                     GtkTreeViewDropPosition  drop_pos);

static void   picman_item_tree_view_new_dropped       (GtkWidget         *widget,
                                                     gint               x,
                                                     gint               y,
                                                     PicmanViewable      *viewable,
                                                     gpointer           data);

static void   picman_item_tree_view_item_changed      (PicmanImage         *image,
                                                     PicmanItemTreeView  *view);
static void   picman_item_tree_view_size_changed      (PicmanImage         *image,
                                                     PicmanItemTreeView  *view);

static void   picman_item_tree_view_name_edited       (GtkCellRendererText *cell,
                                                     const gchar       *path,
                                                     const gchar       *new_name,
                                                     PicmanItemTreeView  *view);

static void   picman_item_tree_view_visible_changed      (PicmanItem          *item,
                                                        PicmanItemTreeView  *view);
static void   picman_item_tree_view_linked_changed       (PicmanItem          *item,
                                                        PicmanItemTreeView  *view);
static void   picman_item_tree_view_lock_content_changed (PicmanItem          *item,
                                                        PicmanItemTreeView  *view);
static void   picman_item_tree_view_lock_position_changed(PicmanItem          *item,
                                                        PicmanItemTreeView  *view);

static void   picman_item_tree_view_eye_clicked       (GtkCellRendererToggle *toggle,
                                                     gchar             *path,
                                                     GdkModifierType    state,
                                                     PicmanItemTreeView  *view);
static void   picman_item_tree_view_chain_clicked     (GtkCellRendererToggle *toggle,
                                                     gchar             *path,
                                                     GdkModifierType    state,
                                                     PicmanItemTreeView  *view);
static void   picman_item_tree_view_lock_content_toggled
                                                    (GtkWidget         *widget,
                                                     PicmanItemTreeView  *view);
static void   picman_item_tree_view_lock_position_toggled
                                                    (GtkWidget         *widget,
                                                     PicmanItemTreeView  *view);
static void   picman_item_tree_view_update_options    (PicmanItemTreeView  *view,
                                                     PicmanItem          *item);

static gboolean picman_item_tree_view_item_pre_clicked(PicmanCellRendererViewable *cell,
                                                     const gchar              *path_str,
                                                     GdkModifierType           state,
                                                     PicmanItemTreeView         *item_view);

/*  utility function to avoid code duplication  */
static void   picman_item_tree_view_toggle_clicked    (GtkCellRendererToggle *toggle,
                                                     gchar             *path_str,
                                                     GdkModifierType    state,
                                                     PicmanItemTreeView  *view,
                                                     PicmanUndoType       undo_type);

static void   picman_item_tree_view_row_expanded      (GtkTreeView       *tree_view,
                                                     GtkTreeIter       *iter,
                                                     GtkTreePath       *path,
                                                     PicmanItemTreeView  *item_view);


G_DEFINE_TYPE_WITH_CODE (PicmanItemTreeView, picman_item_tree_view,
                         PICMAN_TYPE_CONTAINER_TREE_VIEW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_item_tree_view_view_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_item_tree_view_docked_iface_init))

#define parent_class picman_item_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;

static guint view_signals[LAST_SIGNAL] = { 0 };


static void
picman_item_tree_view_class_init (PicmanItemTreeViewClass *klass)
{
  GObjectClass               *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass             *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanContainerTreeViewClass *tree_view_class;

  tree_view_class = PICMAN_CONTAINER_TREE_VIEW_CLASS (klass);

  view_signals[SET_IMAGE] =
    g_signal_new ("set-image",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanItemTreeViewClass, set_image),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_OBJECT);

  object_class->constructed      = picman_item_tree_view_constructed;
  object_class->dispose          = picman_item_tree_view_dispose;

  widget_class->style_set        = picman_item_tree_view_style_set;

  tree_view_class->drop_possible = picman_item_tree_view_drop_possible;
  tree_view_class->drop_viewable = picman_item_tree_view_drop_viewable;

  klass->set_image               = picman_item_tree_view_real_set_image;

  klass->item_type               = G_TYPE_NONE;
  klass->signal_name             = NULL;

  klass->get_container           = NULL;
  klass->get_active_item         = NULL;
  klass->set_active_item         = NULL;
  klass->add_item                = NULL;
  klass->remove_item             = NULL;
  klass->new_item                = NULL;

  klass->action_group            = NULL;
  klass->edit_action             = NULL;
  klass->new_action              = NULL;
  klass->new_default_action      = NULL;
  klass->raise_action            = NULL;
  klass->raise_top_action        = NULL;
  klass->lower_action            = NULL;
  klass->lower_bottom_action     = NULL;
  klass->duplicate_action        = NULL;
  klass->delete_action           = NULL;

  klass->lock_content_stock_id   = NULL;
  klass->lock_content_tooltip    = NULL;
  klass->lock_content_help_id    = NULL;

  klass->lock_position_stock_id   = NULL;
  klass->lock_position_tooltip    = NULL;
  klass->lock_position_help_id    = NULL;

  g_type_class_add_private (klass, sizeof (PicmanItemTreeViewPriv));
}

static void
picman_item_tree_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  iface->set_container     = picman_item_tree_view_set_container;
  iface->set_context       = picman_item_tree_view_set_context;
  iface->insert_item       = picman_item_tree_view_insert_item;
  iface->insert_item_after = picman_item_tree_view_insert_item_after;
  iface->select_item       = picman_item_tree_view_select_item;
  iface->activate_item     = picman_item_tree_view_activate_item;
  iface->context_item      = picman_item_tree_view_context_item;
}

static void
picman_item_tree_view_docked_iface_init (PicmanDockedInterface *iface)
{
  iface->get_preview = NULL;
}

static void
picman_item_tree_view_init (PicmanItemTreeView *view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PICMAN_TYPE_ITEM_TREE_VIEW,
                                            PicmanItemTreeViewPriv);

  view->priv->model_column_visible =
    picman_container_tree_store_columns_add (tree_view->model_columns,
                                           &tree_view->n_model_columns,
                                           G_TYPE_BOOLEAN);

  view->priv->model_column_viewable =
    picman_container_tree_store_columns_add (tree_view->model_columns,
                                           &tree_view->n_model_columns,
                                           G_TYPE_BOOLEAN);

  view->priv->model_column_linked =
    picman_container_tree_store_columns_add (tree_view->model_columns,
                                           &tree_view->n_model_columns,
                                           G_TYPE_BOOLEAN);

  picman_container_tree_view_set_dnd_drop_to_empty (tree_view, TRUE);

  view->priv->image  = NULL;
}

static void
picman_item_tree_view_constructed (GObject *object)
{
  PicmanItemTreeViewClass *item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (object);
  PicmanEditor            *editor          = PICMAN_EDITOR (object);
  PicmanContainerTreeView *tree_view       = PICMAN_CONTAINER_TREE_VIEW (object);
  PicmanItemTreeView      *item_view       = PICMAN_ITEM_TREE_VIEW (object);
  GtkTreeViewColumn     *column;
  GtkWidget             *hbox;
  GtkWidget             *image;
  GtkIconSize            icon_size;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_container_tree_view_connect_name_edited (tree_view,
                                                G_CALLBACK (picman_item_tree_view_name_edited),
                                                item_view);

  g_signal_connect (tree_view->view, "row-expanded",
                    G_CALLBACK (picman_item_tree_view_row_expanded),
                    tree_view);

  g_signal_connect (tree_view->renderer_cell, "pre-clicked",
                    G_CALLBACK (picman_item_tree_view_item_pre_clicked),
                    item_view);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_insert_column (tree_view->view, column, 0);

  item_view->priv->eye_cell = picman_cell_renderer_toggle_new (PICMAN_STOCK_VISIBLE);
  g_object_set (item_view->priv->eye_cell,
                "xpad", 0,
                "ypad", 0,
                NULL);
  gtk_tree_view_column_pack_start (column, item_view->priv->eye_cell, FALSE);
  gtk_tree_view_column_set_attributes (column, item_view->priv->eye_cell,
                                       "active",
                                       item_view->priv->model_column_visible,
                                       "inconsistent",
                                       item_view->priv->model_column_viewable,
                                       NULL);

  picman_container_tree_view_add_toggle_cell (tree_view,
                                            item_view->priv->eye_cell);

  g_signal_connect (item_view->priv->eye_cell, "clicked",
                    G_CALLBACK (picman_item_tree_view_eye_clicked),
                    item_view);

  column = gtk_tree_view_column_new ();
  gtk_tree_view_insert_column (tree_view->view, column, 1);

  item_view->priv->chain_cell = picman_cell_renderer_toggle_new (PICMAN_STOCK_LINKED);
  g_object_set (item_view->priv->chain_cell,
                "xpad", 0,
                "ypad", 0,
                NULL);
  gtk_tree_view_column_pack_start (column, item_view->priv->chain_cell, FALSE);
  gtk_tree_view_column_set_attributes (column, item_view->priv->chain_cell,
                                       "active",
                                       item_view->priv->model_column_linked,
                                       NULL);

  picman_container_tree_view_add_toggle_cell (tree_view,
                                            item_view->priv->chain_cell);

  g_signal_connect (item_view->priv->chain_cell, "clicked",
                    G_CALLBACK (picman_item_tree_view_chain_clicked),
                    item_view);

  /*  disable the default PicmanContainerView drop handler  */
  picman_container_view_set_dnd_widget (PICMAN_CONTAINER_VIEW (item_view), NULL);

  picman_dnd_drag_dest_set_by_type (GTK_WIDGET (tree_view->view),
                                  GTK_DEST_DEFAULT_HIGHLIGHT,
                                  item_view_class->item_type,
                                  GDK_ACTION_MOVE | GDK_ACTION_COPY);

  item_view->priv->edit_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->edit_action, NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (item_view),
                                  GTK_BUTTON (item_view->priv->edit_button),
                                  item_view_class->item_type);

  item_view->priv->new_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->new_action,
                                   item_view_class->new_default_action,
                                   GDK_SHIFT_MASK,
                                   NULL);
  /*  connect "drop to new" manually as it makes a difference whether
   *  it was clicked or dropped
   */
  picman_dnd_viewable_dest_add (item_view->priv->new_button,
                              item_view_class->item_type,
                              picman_item_tree_view_new_dropped,
                              item_view);

  item_view->priv->raise_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->raise_action,
                                   item_view_class->raise_top_action,
                                   GDK_SHIFT_MASK,
                                   NULL);

  item_view->priv->lower_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->lower_action,
                                   item_view_class->lower_bottom_action,
                                   GDK_SHIFT_MASK,
                                   NULL);

  item_view->priv->duplicate_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->duplicate_action, NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (item_view),
                                  GTK_BUTTON (item_view->priv->duplicate_button),
                                  item_view_class->item_type);

  item_view->priv->delete_button =
    picman_editor_add_action_button (editor, item_view_class->action_group,
                                   item_view_class->delete_action, NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (item_view),
                                  GTK_BUTTON (item_view->priv->delete_button),
                                  item_view_class->item_type);

  hbox = picman_item_tree_view_get_lock_box (item_view);

  /*  Lock content toggle  */
  item_view->priv->lock_content_toggle = gtk_toggle_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox), item_view->priv->lock_content_toggle,
                      FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (hbox),
                         item_view->priv->lock_content_toggle, 0);
  gtk_widget_show (item_view->priv->lock_content_toggle);

  g_signal_connect (item_view->priv->lock_content_toggle, "toggled",
                    G_CALLBACK (picman_item_tree_view_lock_content_toggled),
                    item_view);

  picman_help_set_help_data (item_view->priv->lock_content_toggle,
                           item_view_class->lock_content_tooltip,
                           item_view_class->lock_content_help_id);

  gtk_widget_style_get (GTK_WIDGET (item_view),
                        "button-icon-size", &icon_size,
                        NULL);

  image = gtk_image_new_from_stock (item_view_class->lock_content_stock_id,
                                    icon_size);
  gtk_container_add (GTK_CONTAINER (item_view->priv->lock_content_toggle),
                     image);
  gtk_widget_show (image);

  /*  Lock position toggle  */
  item_view->priv->lock_position_toggle = gtk_toggle_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox), item_view->priv->lock_position_toggle,
                      FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (hbox),
                         item_view->priv->lock_position_toggle, 1);
  gtk_widget_show (item_view->priv->lock_position_toggle);

  g_signal_connect (item_view->priv->lock_position_toggle, "toggled",
                    G_CALLBACK (picman_item_tree_view_lock_position_toggled),
                    item_view);

  picman_help_set_help_data (item_view->priv->lock_position_toggle,
                           item_view_class->lock_position_tooltip,
                           item_view_class->lock_position_help_id);

  image = gtk_image_new_from_stock (item_view_class->lock_position_stock_id,
                                    icon_size);
  gtk_container_add (GTK_CONTAINER (item_view->priv->lock_position_toggle),
                     image);
  gtk_widget_show (image);
}

static void
picman_item_tree_view_dispose (GObject *object)
{
  PicmanItemTreeView *view = PICMAN_ITEM_TREE_VIEW (object);

  if (view->priv->image)
    picman_item_tree_view_set_image (view, NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_item_tree_view_style_set (GtkWidget *widget,
                               GtkStyle  *prev_style)
{
  PicmanItemTreeView *view = PICMAN_ITEM_TREE_VIEW (widget);
  GList            *children;
  GList            *list;
  GtkReliefStyle    button_relief;
  GtkIconSize       button_icon_size;
  gint              content_spacing;
  gint              button_spacing;

  gtk_widget_style_get (widget,
                        "button-relief",    &button_relief,
                        "button-icon-size", &button_icon_size,
                        "content-spacing",  &content_spacing,
                        "button-spacing",   &button_spacing,
                        NULL);

  if (view->priv->options_box)
    {
      gtk_box_set_spacing (GTK_BOX (view->priv->options_box), content_spacing);

      children =
        gtk_container_get_children (GTK_CONTAINER (view->priv->options_box));

      for (list = children; list; list = g_list_next (list))
        {
          GtkWidget *child = list->data;

          if (GTK_IS_BOX (child))
            gtk_box_set_spacing (GTK_BOX (child), button_spacing);
        }

      g_list_free (children);
    }

  if (view->priv->lock_box)
    {
      gtk_box_set_spacing (GTK_BOX (view->priv->lock_box), button_spacing);

      children =
        gtk_container_get_children (GTK_CONTAINER (view->priv->lock_box));

      for (list = children; list; list = g_list_next (list))
        {
          GtkWidget *child = list->data;

          if (GTK_IS_BUTTON (child))
            {
              GtkWidget *image;

              gtk_button_set_relief (GTK_BUTTON (child), button_relief);

              image = gtk_bin_get_child (GTK_BIN (child));

              if (GTK_IS_IMAGE (image))
                {
                  GtkIconSize  old_size;
                  gchar       *stock_id;

                  gtk_image_get_stock (GTK_IMAGE (image), &stock_id, &old_size);

                  if (button_icon_size != old_size)
                    gtk_image_set_from_stock (GTK_IMAGE (image),
                                              stock_id, button_icon_size);
                }
            }
        }

      g_list_free (children);
    }

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);
}

GtkWidget *
picman_item_tree_view_new (GType            view_type,
                         gint             view_size,
                         gint             view_border_width,
                         PicmanImage       *image,
                         PicmanMenuFactory *menu_factory,
                         const gchar     *menu_identifier,
                         const gchar     *ui_path)
{
  PicmanItemTreeView *item_view;

  g_return_val_if_fail (g_type_is_a (view_type, PICMAN_TYPE_ITEM_TREE_VIEW), NULL);
  g_return_val_if_fail (view_size >  0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (image == NULL || PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (menu_identifier != NULL, NULL);
  g_return_val_if_fail (ui_path != NULL, NULL);

  item_view = g_object_new (view_type,
                            "reorderable",     TRUE,
                            "menu-factory",    menu_factory,
                            "menu-identifier", menu_identifier,
                            "ui-path",         ui_path,
                            NULL);

  picman_container_view_set_view_size (PICMAN_CONTAINER_VIEW (item_view),
                                     view_size, view_border_width);

  picman_item_tree_view_set_image (item_view, image);

  return GTK_WIDGET (item_view);
}

void
picman_item_tree_view_set_image (PicmanItemTreeView *view,
                               PicmanImage        *image)
{
  g_return_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view));
  g_return_if_fail (image == NULL || PICMAN_IS_IMAGE (image));

  g_signal_emit (view, view_signals[SET_IMAGE], 0, image);

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (view)), view);
}

PicmanImage *
picman_item_tree_view_get_image (PicmanItemTreeView *view)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view), NULL);

  return view->priv->image;
}

void
picman_item_tree_view_add_options (PicmanItemTreeView *view,
                                 const gchar      *label,
                                 GtkWidget        *options)
{
  gint content_spacing;
  gint button_spacing;

  g_return_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view));
  g_return_if_fail (GTK_IS_WIDGET (options));

  gtk_widget_style_get (GTK_WIDGET (view),
                        "content-spacing", &content_spacing,
                        "button-spacing",  &button_spacing,
                        NULL);

  if (! view->priv->options_box)
    {
      PicmanItemTreeViewClass *item_view_class;

      item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view);

      view->priv->options_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, content_spacing);
      gtk_box_pack_start (GTK_BOX (view), view->priv->options_box,
                          FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (view), view->priv->options_box, 0);
      gtk_widget_show (view->priv->options_box);

      if (! view->priv->image ||
          ! item_view_class->get_active_item (view->priv->image))
        {
          gtk_widget_set_sensitive (view->priv->options_box, FALSE);
        }
    }

  if (label)
    {
      GtkWidget *hbox;
      GtkWidget *label_widget;
      gboolean   group_created = FALSE;

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, button_spacing);
      gtk_box_pack_start (GTK_BOX (view->priv->options_box), hbox,
                          FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      if (! view->priv->options_group)
        {
          view->priv->options_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
          group_created = TRUE;
        }

      label_widget = gtk_label_new (label);
      gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);
      gtk_size_group_add_widget (view->priv->options_group, label_widget);
      gtk_box_pack_start (GTK_BOX (hbox), label_widget, FALSE, FALSE, 0);
      gtk_widget_show (label_widget);

      if (group_created)
        g_object_unref (view->priv->options_group);

      gtk_box_pack_start (GTK_BOX (hbox), options, TRUE, TRUE, 0);
      gtk_widget_show (options);
    }
  else
    {
      gtk_box_pack_start (GTK_BOX (view->priv->options_box), options,
                          FALSE, FALSE, 0);
      gtk_widget_show (options);
    }
}

GtkWidget *
picman_item_tree_view_get_lock_box (PicmanItemTreeView *view)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view), NULL);

  if (! view->priv->lock_box)
    {
      gint button_spacing;

      gtk_widget_style_get (GTK_WIDGET (view),
                            "button-spacing", &button_spacing,
                            NULL);

      view->priv->lock_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, button_spacing);

      picman_item_tree_view_add_options (view, _("Lock:"), view->priv->lock_box);

      gtk_box_set_child_packing (GTK_BOX (view->priv->options_box),
                                 gtk_widget_get_parent (view->priv->lock_box),
                                 FALSE, FALSE, 0, GTK_PACK_END);
    }

  return view->priv->lock_box;
}

GtkWidget *
picman_item_tree_view_get_new_button (PicmanItemTreeView *view)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view), NULL);

  return view->priv->new_button;
}

GtkWidget *
picman_item_tree_view_get_edit_button (PicmanItemTreeView *view)
{
  g_return_val_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view), NULL);

  return view->priv->edit_button;
}

gint
picman_item_tree_view_get_drop_index (PicmanItemTreeView         *view,
                                    PicmanViewable             *dest_viewable,
                                    GtkTreeViewDropPosition   drop_pos,
                                    PicmanViewable            **parent)
{
  gint index = -1;

  g_return_val_if_fail (PICMAN_IS_ITEM_TREE_VIEW (view), -1);
  g_return_val_if_fail (dest_viewable == NULL ||
                        PICMAN_IS_VIEWABLE (dest_viewable), -1);
  g_return_val_if_fail (parent != NULL, -1);

  *parent = NULL;

  if (dest_viewable)
    {
      *parent = picman_viewable_get_parent (dest_viewable);
      index   = picman_item_get_index (PICMAN_ITEM (dest_viewable));

      if (drop_pos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER)
        {
          PicmanContainer *children = picman_viewable_get_children (dest_viewable);

          if (children)
            {
              *parent = dest_viewable;
              index   = 0;
            }
          else
            {
              index++;
            }
        }
      else if (drop_pos == GTK_TREE_VIEW_DROP_AFTER)
        {
          index++;
        }
    }

  return index;
}

static void
picman_item_tree_view_real_set_image (PicmanItemTreeView *view,
                                    PicmanImage        *image)
{
  if (view->priv->image == image)
    return;

  if (view->priv->image)
    {
      g_signal_handlers_disconnect_by_func (view->priv->image,
                                            picman_item_tree_view_item_changed,
                                            view);
      g_signal_handlers_disconnect_by_func (view->priv->image,
                                            picman_item_tree_view_size_changed,
                                            view);

      picman_container_view_set_container (PICMAN_CONTAINER_VIEW (view), NULL);

      g_signal_handlers_disconnect_by_func (view->priv->image,
                                            picman_item_tree_view_image_flush,
                                            view);
    }

  view->priv->image = image;

  if (view->priv->image)
    {
      PicmanContainer *container;

      container =
        PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_container (view->priv->image);

      picman_container_view_set_container (PICMAN_CONTAINER_VIEW (view), container);

      g_signal_connect (view->priv->image,
                        PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->signal_name,
                        G_CALLBACK (picman_item_tree_view_item_changed),
                        view);
      g_signal_connect (view->priv->image, "size-changed",
                        G_CALLBACK (picman_item_tree_view_size_changed),
                        view);

      g_signal_connect (view->priv->image, "flush",
                        G_CALLBACK (picman_item_tree_view_image_flush),
                        view);

      picman_item_tree_view_item_changed (view->priv->image, view);
    }
}

static void
picman_item_tree_view_image_flush (PicmanImage        *image,
                                 gboolean          invalidate_preview,
                                 PicmanItemTreeView *view)
{
  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (view)), view);
}


/*  PicmanContainerView methods  */

static void
picman_item_tree_view_set_container (PicmanContainerView *view,
                                   PicmanContainer     *container)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanContainer    *old_container;

  old_container = picman_container_view_get_container (view);

  if (old_container)
    {
      picman_tree_handler_disconnect (item_view->priv->visible_changed_handler);
      item_view->priv->visible_changed_handler = NULL;

      picman_tree_handler_disconnect (item_view->priv->linked_changed_handler);
      item_view->priv->linked_changed_handler = NULL;

      picman_tree_handler_disconnect (item_view->priv->lock_content_changed_handler);
      item_view->priv->lock_content_changed_handler = NULL;

      picman_tree_handler_disconnect (item_view->priv->lock_position_changed_handler);
      item_view->priv->lock_position_changed_handler = NULL;
    }

  parent_view_iface->set_container (view, container);

  if (container)
    {
      item_view->priv->visible_changed_handler =
        picman_tree_handler_connect (container, "visibility-changed",
                                   G_CALLBACK (picman_item_tree_view_visible_changed),
                                   view);

      item_view->priv->linked_changed_handler =
        picman_tree_handler_connect (container, "linked-changed",
                                   G_CALLBACK (picman_item_tree_view_linked_changed),
                                   view);

      item_view->priv->lock_content_changed_handler =
        picman_tree_handler_connect (container, "lock-content-changed",
                                   G_CALLBACK (picman_item_tree_view_lock_content_changed),
                                   view);

      item_view->priv->lock_position_changed_handler =
        picman_tree_handler_connect (container, "lock-position-changed",
                                   G_CALLBACK (picman_item_tree_view_lock_position_changed),
                                   view);
    }
}

static void
picman_item_tree_view_set_context (PicmanContainerView *view,
                                 PicmanContext       *context)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  PicmanItemTreeView      *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanImage             *image     = NULL;
  PicmanContext           *old_context;

  old_context = picman_container_view_get_context (view);

  if (old_context)
    {
      g_signal_handlers_disconnect_by_func (old_context,
                                            picman_item_tree_view_set_image,
                                            item_view);
    }

  parent_view_iface->set_context (view, context);

  if (context)
    {
      if (! tree_view->dnd_picman)
        tree_view->dnd_picman = context->picman;

      g_signal_connect_swapped (context, "image-changed",
                                G_CALLBACK (picman_item_tree_view_set_image),
                                item_view);

      image = picman_context_get_image (context);
    }

  picman_item_tree_view_set_image (item_view, image);
}

static gpointer
picman_item_tree_view_insert_item (PicmanContainerView *view,
                                 PicmanViewable      *viewable,
                                 gpointer           parent_insert_data,
                                 gint               index)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  PicmanItemTreeView      *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanItem              *item      = PICMAN_ITEM (viewable);
  GtkTreeIter           *iter;

  item_view->priv->inserting_item = TRUE;

  iter = parent_view_iface->insert_item (view, viewable,
                                         parent_insert_data, index);

  item_view->priv->inserting_item = FALSE;

  gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                      item_view->priv->model_column_visible,
                      picman_item_get_visible (item),
                      item_view->priv->model_column_viewable,
                      picman_item_get_visible (item) &&
                      ! picman_item_is_visible (item),
                      item_view->priv->model_column_linked,
                      picman_item_get_linked (item),
                      -1);

  return iter;
}

static void
picman_item_tree_view_insert_item_after (PicmanContainerView *view,
                                       PicmanViewable      *viewable,
                                       gpointer           insert_data)
{
  PicmanItemTreeView      *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanItemTreeViewClass *item_view_class;
  PicmanItem              *active_item;

  item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (item_view);

  active_item = item_view_class->get_active_item (item_view->priv->image);

  if (active_item == (PicmanItem *) viewable)
    picman_container_view_select_item (view, viewable);
}

static gboolean
picman_item_tree_view_select_item (PicmanContainerView *view,
                                 PicmanViewable      *item,
                                 gpointer           insert_data)
{
  PicmanItemTreeView *tree_view         = PICMAN_ITEM_TREE_VIEW (view);
  gboolean          options_sensitive = FALSE;
  gboolean          success;

  success = parent_view_iface->select_item (view, item, insert_data);

  if (item)
    {
      PicmanItemTreeViewClass *item_view_class;
      PicmanItem              *active_item;

      item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (tree_view);

      active_item = item_view_class->get_active_item (tree_view->priv->image);

      if (active_item != (PicmanItem *) item)
        {
          item_view_class->set_active_item (tree_view->priv->image,
                                            PICMAN_ITEM (item));

          picman_image_flush (tree_view->priv->image);
        }

      options_sensitive = TRUE;

      picman_item_tree_view_update_options (tree_view, PICMAN_ITEM (item));
    }

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (tree_view)), tree_view);

  if (tree_view->priv->options_box)
    gtk_widget_set_sensitive (tree_view->priv->options_box, options_sensitive);

  return success;
}

static void
picman_item_tree_view_activate_item (PicmanContainerView *view,
                                   PicmanViewable      *item,
                                   gpointer           insert_data)
{
  PicmanItemTreeViewClass *item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view);

  if (parent_view_iface->activate_item)
    parent_view_iface->activate_item (view, item, insert_data);

  if (item_view_class->activate_action)
    {
      picman_ui_manager_activate_action (picman_editor_get_ui_manager (PICMAN_EDITOR (view)),
                                       item_view_class->action_group,
                                       item_view_class->activate_action);
    }
}

static void
picman_item_tree_view_context_item (PicmanContainerView *view,
                                  PicmanViewable      *item,
                                  gpointer           insert_data)
{
  if (parent_view_iface->context_item)
    parent_view_iface->context_item (view, item, insert_data);

  picman_editor_popup_menu (PICMAN_EDITOR (view), NULL, NULL);
}

static gboolean
picman_item_tree_view_drop_possible (PicmanContainerTreeView   *tree_view,
                                   PicmanDndType              src_type,
                                   PicmanViewable            *src_viewable,
                                   PicmanViewable            *dest_viewable,
                                   GtkTreePath             *drop_path,
                                   GtkTreeViewDropPosition  drop_pos,
                                   GtkTreeViewDropPosition *return_drop_pos,
                                   GdkDragAction           *return_drag_action)
{
  if (PICMAN_IS_ITEM (src_viewable) &&
      (dest_viewable == NULL ||
       picman_item_get_image (PICMAN_ITEM (src_viewable)) !=
       picman_item_get_image (PICMAN_ITEM (dest_viewable))))
    {
      if (return_drop_pos)
        *return_drop_pos = drop_pos;

      if (return_drag_action)
        *return_drag_action = GDK_ACTION_COPY;

      return TRUE;
    }

  return PICMAN_CONTAINER_TREE_VIEW_CLASS (parent_class)->drop_possible (tree_view,
                                                                       src_type,
                                                                       src_viewable,
                                                                       dest_viewable,
                                                                       drop_path,
                                                                       drop_pos,
                                                                       return_drop_pos,
                                                                       return_drag_action);
}

static void
picman_item_tree_view_drop_viewable (PicmanContainerTreeView   *tree_view,
                                   PicmanViewable            *src_viewable,
                                   PicmanViewable            *dest_viewable,
                                   GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeViewClass *item_view_class;
  PicmanItemTreeView      *item_view  = PICMAN_ITEM_TREE_VIEW (tree_view);
  gint                   dest_index = -1;

  item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (item_view);

  if (item_view->priv->image != picman_item_get_image (PICMAN_ITEM (src_viewable)) ||
      ! g_type_is_a (G_TYPE_FROM_INSTANCE (src_viewable),
                     item_view_class->item_type))
    {
      GType     item_type = item_view_class->item_type;
      PicmanItem *new_item;
      PicmanItem *parent;

      if (g_type_is_a (G_TYPE_FROM_INSTANCE (src_viewable), item_type))
        item_type = G_TYPE_FROM_INSTANCE (src_viewable);

      dest_index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                                       drop_pos,
                                                       (PicmanViewable **) &parent);

      new_item = picman_item_convert (PICMAN_ITEM (src_viewable),
                                    item_view->priv->image, item_type);

      picman_item_set_linked (new_item, FALSE, FALSE);

      item_view_class->add_item (item_view->priv->image, new_item,
                                 parent, dest_index, TRUE);
    }
  else if (dest_viewable)
    {
      PicmanItem *src_parent;
      PicmanItem *dest_parent;
      gint      src_index;
      gint      dest_index;

      src_parent = PICMAN_ITEM (picman_viewable_get_parent (src_viewable));
      src_index  = picman_item_get_index (PICMAN_ITEM (src_viewable));

      dest_index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                                       drop_pos,
                                                       (PicmanViewable **) &dest_parent);

      if (src_parent == dest_parent)
        {
          if (src_index < dest_index)
            dest_index--;
        }

      picman_image_reorder_item (item_view->priv->image,
                               PICMAN_ITEM (src_viewable),
                               dest_parent,
                               dest_index,
                               TRUE, NULL);
    }

  picman_image_flush (item_view->priv->image);
}


/*  "New" functions  */

static void
picman_item_tree_view_new_dropped (GtkWidget    *widget,
                                 gint          x,
                                 gint          y,
                                 PicmanViewable *viewable,
                                 gpointer      data)
{
  PicmanItemTreeViewClass *item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (data);
  PicmanContainerView     *view            = PICMAN_CONTAINER_VIEW (data);

  if (item_view_class->new_default_action &&
      viewable && picman_container_view_lookup (view, viewable))
    {
      GtkAction *action;

      action = picman_ui_manager_find_action (picman_editor_get_ui_manager (PICMAN_EDITOR (view)),
                                            item_view_class->action_group,
                                            item_view_class->new_default_action);

      if (action)
        {
          g_object_set (action, "viewable", viewable, NULL);
          gtk_action_activate (action);
          g_object_set (action, "viewable", NULL, NULL);
        }
    }
}


/*  PicmanImage callbacks  */

static void
picman_item_tree_view_item_changed (PicmanImage        *image,
                                  PicmanItemTreeView *view)
{
  PicmanItem *item;

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (view->priv->image);

  picman_container_view_select_item (PICMAN_CONTAINER_VIEW (view),
                                   (PicmanViewable *) item);
}

static void
picman_item_tree_view_size_changed (PicmanImage        *image,
                                  PicmanItemTreeView *tree_view)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (tree_view);
  gint               view_size;
  gint               border_width;

  view_size = picman_container_view_get_view_size (view, &border_width);

  picman_container_view_set_view_size (view, view_size, border_width);
}

static void
picman_item_tree_view_name_edited (GtkCellRendererText *cell,
                                 const gchar         *path_str,
                                 const gchar         *new_name,
                                 PicmanItemTreeView    *view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreePath           *path;
  GtkTreeIter            iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      PicmanItem         *item;
      const gchar      *old_name;
      GError           *error = NULL;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      item = PICMAN_ITEM (renderer->viewable);

      old_name = picman_object_get_name (item);

      if (! old_name) old_name = "";
      if (! new_name) new_name = "";

      if (strcmp (old_name, new_name) &&
          picman_item_rename (item, new_name, &error))
        {
          picman_image_flush (picman_item_get_image (item));
        }
      else
        {
          gchar *name = picman_viewable_get_description (renderer->viewable, NULL);

          gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), &iter,
                              PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, name,
                              -1);
          g_free (name);

          if (error)
            {
              picman_message_literal (view->priv->image->picman, G_OBJECT (view),
				    PICMAN_MESSAGE_WARNING,
				    error->message);
              g_clear_error (&error);
            }
        }

      g_object_unref (renderer);
    }

  gtk_tree_path_free (path);
}


/*  "Visible" callbacks  */

static void
picman_item_tree_view_visible_changed (PicmanItem         *item,
                                     PicmanItemTreeView *view)
{
  PicmanContainerView     *container_view = PICMAN_CONTAINER_VIEW (view);
  PicmanContainerTreeView *tree_view      = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter;

  iter = picman_container_view_lookup (container_view,
                                     (PicmanViewable *) item);

  if (iter)
    {
      PicmanContainer *children;

      gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                          view->priv->model_column_visible,
                          picman_item_get_visible (item),
                          view->priv->model_column_viewable,
                          picman_item_get_visible (item) &&
                          ! picman_item_is_visible (item),
                          -1);

      children = picman_viewable_get_children (PICMAN_VIEWABLE (item));

      if (children)
        picman_container_foreach (children,
                                (GFunc) picman_item_tree_view_visible_changed,
                                view);
    }
}

static void
picman_item_tree_view_eye_clicked (GtkCellRendererToggle *toggle,
                                 gchar                 *path_str,
                                 GdkModifierType        state,
                                 PicmanItemTreeView      *view)
{
  picman_item_tree_view_toggle_clicked (toggle, path_str, state, view,
                                      PICMAN_UNDO_ITEM_VISIBILITY);
}


/*  "Linked" callbacks  */

static void
picman_item_tree_view_linked_changed (PicmanItem         *item,
                                    PicmanItemTreeView *view)
{
  PicmanContainerView     *container_view = PICMAN_CONTAINER_VIEW (view);
  PicmanContainerTreeView *tree_view      = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreeIter           *iter;

  iter = picman_container_view_lookup (container_view,
                                     (PicmanViewable *) item);

  if (iter)
    gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                        view->priv->model_column_linked,
                        picman_item_get_linked (item),
                        -1);
}

static void
picman_item_tree_view_chain_clicked (GtkCellRendererToggle *toggle,
                                   gchar                 *path_str,
                                   GdkModifierType        state,
                                   PicmanItemTreeView      *view)
{
  picman_item_tree_view_toggle_clicked (toggle, path_str, state, view,
                                      PICMAN_UNDO_ITEM_LINKED);
}


/*  "Lock Content" callbacks  */

static void
picman_item_tree_view_lock_content_changed (PicmanItem         *item,
                                          PicmanItemTreeView *view)
{
  PicmanImage *image = view->priv->image;
  PicmanItem  *active_item;

  active_item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (active_item == item)
    picman_item_tree_view_update_options (view, item);
}

static void
picman_item_tree_view_lock_content_toggled (GtkWidget         *widget,
                                          PicmanItemTreeView  *view)
{
  PicmanImage *image = view->priv->image;
  PicmanItem  *item;

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (item)
    {
      gboolean lock_content;

      lock_content = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

      if (picman_item_get_lock_content (item) != lock_content)
        {
#if 0
          PicmanUndo *undo;
#endif
          gboolean  push_undo = TRUE;

#if 0
          /*  compress lock content undos  */
          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               PICMAN_UNDO_ITEM_LOCK_CONTENT);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == item)
            push_undo = FALSE;
#endif

          g_signal_handlers_block_by_func (item,
                                           picman_item_tree_view_lock_content_changed,
                                           view);

          picman_item_set_lock_content (item, lock_content, push_undo);

          g_signal_handlers_unblock_by_func (item,
                                             picman_item_tree_view_lock_content_changed,
                                             view);

          picman_image_flush (image);
        }
    }
}

static void
picman_item_tree_view_lock_position_changed (PicmanItem         *item,
                                           PicmanItemTreeView *view)
{
  PicmanImage *image = view->priv->image;
  PicmanItem  *active_item;

  active_item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (active_item == item)
    picman_item_tree_view_update_options (view, item);
}

static void
picman_item_tree_view_lock_position_toggled (GtkWidget         *widget,
                                           PicmanItemTreeView  *view)
{
  PicmanImage *image = view->priv->image;
  PicmanItem  *item;

  item = PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (item)
    {
      gboolean lock_position;

      lock_position = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

      if (picman_item_get_lock_position (item) != lock_position)
        {
          PicmanUndo *undo;
          gboolean  push_undo = TRUE;

          /*  compress lock position undos  */
          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               PICMAN_UNDO_ITEM_LOCK_POSITION);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == item)
            push_undo = FALSE;

          g_signal_handlers_block_by_func (item,
                                           picman_item_tree_view_lock_position_changed,
                                           view);

          picman_item_set_lock_position (item, lock_position, push_undo);

          g_signal_handlers_unblock_by_func (item,
                                             picman_item_tree_view_lock_position_changed,
                                             view);

          picman_image_flush (image);
        }
    }
}

static gboolean
picman_item_tree_view_item_pre_clicked (PicmanCellRendererViewable *cell,
                                      const gchar              *path_str,
                                      GdkModifierType           state,
                                      PicmanItemTreeView         *item_view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (item_view);
  GtkTreePath           *path;
  GtkTreeIter            iter;
  gboolean               handled = FALSE;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path) &&
      (state & GDK_MOD1_MASK))
    {
      PicmanImage        *image    = picman_item_tree_view_get_image (item_view);
      PicmanViewRenderer *renderer = NULL;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      if (renderer)
        {
          PicmanItem       *item = PICMAN_ITEM (renderer->viewable);
          PicmanChannelOps  op   = picman_modifiers_to_channel_op (state);

          picman_item_to_selection (item, op,
                                  TRUE, FALSE, 0.0, 0.0);
          picman_image_flush (image);

          g_object_unref (renderer);

          /* Don't select the clicked layer */
          handled = TRUE;
        }
    }

  gtk_tree_path_free (path);

  return handled;
}

static void
picman_item_tree_view_update_options (PicmanItemTreeView *view,
                                    PicmanItem         *item)
{
  if (picman_item_get_lock_content (item) !=
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->priv->lock_content_toggle)))
    {
      g_signal_handlers_block_by_func (view->priv->lock_content_toggle,
                                       picman_item_tree_view_lock_content_toggled,
                                       view);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->priv->lock_content_toggle),
                                    picman_item_get_lock_content (item));

      g_signal_handlers_unblock_by_func (view->priv->lock_content_toggle,
                                         picman_item_tree_view_lock_content_toggled,
                                         view);
    }

  if (picman_item_get_lock_position (item) !=
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->priv->lock_position_toggle)))
    {
      g_signal_handlers_block_by_func (view->priv->lock_position_toggle,
                                       picman_item_tree_view_lock_position_toggled,
                                       view);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->priv->lock_position_toggle),
                                    picman_item_get_lock_position (item));

      g_signal_handlers_unblock_by_func (view->priv->lock_position_toggle,
                                         picman_item_tree_view_lock_position_toggled,
                                         view);
    }

  gtk_widget_set_sensitive (view->priv->lock_content_toggle,
                            picman_item_can_lock_content (item));

  gtk_widget_set_sensitive (view->priv->lock_position_toggle,
                            picman_item_can_lock_position (item));
}


/*  Utility functions used from eye_clicked and chain_clicked.
 *  Would make sense to do this in a generic fashion using
 *  properties, but for now it's better than duplicating the code.
 */
static void
picman_item_tree_view_toggle_clicked (GtkCellRendererToggle *toggle,
                                    gchar                 *path_str,
                                    GdkModifierType        state,
                                    PicmanItemTreeView      *view,
                                    PicmanUndoType           undo_type)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkTreePath           *path;
  GtkTreeIter            iter;

  void (* setter)    (PicmanItem    *item,
                      gboolean     value,
                      gboolean     push_undo);
  void (* exclusive) (PicmanItem    *item,
                      PicmanContext *context);

  switch (undo_type)
    {
    case PICMAN_UNDO_ITEM_VISIBILITY:
      setter     = picman_item_set_visible;
      exclusive  = picman_item_toggle_exclusive_visible;
      break;

    case PICMAN_UNDO_ITEM_LINKED:
      setter     = picman_item_set_linked;
      exclusive  = picman_item_toggle_exclusive_linked;
      break;

    default:
      return;
    }

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanContext      *context;
      PicmanViewRenderer *renderer;
      PicmanItem         *item;
      PicmanImage        *image;
      gboolean          active;

      context = picman_container_view_get_context (PICMAN_CONTAINER_VIEW (view));

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);
      g_object_get (toggle,
                    "active", &active,
                    NULL);

      item = PICMAN_ITEM (renderer->viewable);
      g_object_unref (renderer);

      image = picman_item_get_image (item);

      if ((state & GDK_SHIFT_MASK) && exclusive)
        {
          exclusive (item, context);
        }
      else
        {
          PicmanUndo *undo;
          gboolean  push_undo = TRUE;

          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               undo_type);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == item)
            push_undo = FALSE;

          setter (item, ! active, push_undo);

          if (!push_undo)
            picman_undo_refresh_preview (undo, context);
        }

      picman_image_flush (image);
    }

  gtk_tree_path_free (path);
}


/*  GtkTreeView callbacks  */

static void
picman_item_tree_view_row_expanded (GtkTreeView      *tree_view,
                                  GtkTreeIter      *iter,
                                  GtkTreePath      *path,
                                  PicmanItemTreeView *item_view)
{
  /*  don't select the item while it is being inserted  */
  if (! item_view->priv->inserting_item)
    {
      PicmanItemTreeViewClass *item_view_class;
      PicmanViewRenderer      *renderer;
      PicmanItem              *expanded_item;
      PicmanItem              *active_item;

      gtk_tree_model_get (PICMAN_CONTAINER_TREE_VIEW (item_view)->model, iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);
      expanded_item = PICMAN_ITEM (renderer->viewable);
      g_object_unref (renderer);

      item_view_class = PICMAN_ITEM_TREE_VIEW_GET_CLASS (item_view);

      active_item = item_view_class->get_active_item (item_view->priv->image);

      /*  select the active item only if it was made visible by expanding
       *  its immediate parent. See bug #666561.
       */
      if (active_item &&
          picman_item_get_parent (active_item) == expanded_item)
        {
          picman_container_view_select_item (PICMAN_CONTAINER_VIEW (item_view),
                                           PICMAN_VIEWABLE (active_item));
        }
    }
}
