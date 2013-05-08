/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlayertreeview.c
 * Copyright (C) 2001-2009 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmanimage-undo.h"
#include "core/picmanimage.h"
#include "core/picmanitemundo.h"
#include "core/picmanlayer.h"
#include "core/picmanlayermask.h"
#include "core/picmantreehandler.h"

#include "text/picmantextlayer.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "picmanactiongroup.h"
#include "picmancellrendererviewable.h"
#include "picmancontainertreestore.h"
#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmanhelp-ids.h"
#include "picmanlayertreeview.h"
#include "picmanspinscale.h"
#include "picmanuimanager.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-constructors.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


struct _PicmanLayerTreeViewPriv
{
  GtkWidget       *paint_mode_menu;
  GtkAdjustment   *opacity_adjustment;
  GtkWidget       *lock_alpha_toggle;

  gint             model_column_mask;
  gint             model_column_mask_visible;

  GtkCellRenderer *mask_cell;

  PangoAttrList   *italic_attrs;
  PangoAttrList   *bold_attrs;

  PicmanTreeHandler *mode_changed_handler;
  PicmanTreeHandler *opacity_changed_handler;
  PicmanTreeHandler *lock_alpha_changed_handler;
  PicmanTreeHandler *mask_changed_handler;
  PicmanTreeHandler *alpha_changed_handler;
};


static void       picman_layer_tree_view_view_iface_init            (PicmanContainerViewInterface *iface);

static void       picman_layer_tree_view_constructed                (GObject                    *object);
static void       picman_layer_tree_view_finalize                   (GObject                    *object);

static void       picman_layer_tree_view_set_container              (PicmanContainerView          *view,
                                                                   PicmanContainer              *container);
static void       picman_layer_tree_view_set_context                (PicmanContainerView          *view,
                                                                   PicmanContext                *context);
static gpointer   picman_layer_tree_view_insert_item                (PicmanContainerView          *view,
                                                                   PicmanViewable               *viewable,
                                                                   gpointer                    parent_insert_data,
                                                                   gint                        index);
static gboolean   picman_layer_tree_view_select_item                (PicmanContainerView          *view,
                                                                   PicmanViewable               *item,
                                                                   gpointer                    insert_data);
static void       picman_layer_tree_view_set_view_size              (PicmanContainerView          *view);
static gboolean   picman_layer_tree_view_drop_possible              (PicmanContainerTreeView      *view,
                                                                   PicmanDndType                 src_type,
                                                                   PicmanViewable               *src_viewable,
                                                                   PicmanViewable               *dest_viewable,
                                                                   GtkTreePath                *drop_path,
                                                                   GtkTreeViewDropPosition     drop_pos,
                                                                   GtkTreeViewDropPosition    *return_drop_pos,
                                                                   GdkDragAction              *return_drag_action);
static void       picman_layer_tree_view_drop_color                 (PicmanContainerTreeView      *view,
                                                                   const PicmanRGB              *color,
                                                                   PicmanViewable               *dest_viewable,
                                                                   GtkTreeViewDropPosition     drop_pos);
static void       picman_layer_tree_view_drop_uri_list              (PicmanContainerTreeView      *view,
                                                                   GList                      *uri_list,
                                                                   PicmanViewable               *dest_viewable,
                                                                   GtkTreeViewDropPosition     drop_pos);
static void       picman_layer_tree_view_drop_component             (PicmanContainerTreeView      *tree_view,
                                                                   PicmanImage                  *image,
                                                                   PicmanChannelType             component,
                                                                   PicmanViewable               *dest_viewable,
                                                                   GtkTreeViewDropPosition     drop_pos);
static void       picman_layer_tree_view_drop_pixbuf                (PicmanContainerTreeView      *tree_view,
                                                                   GdkPixbuf                  *pixbuf,
                                                                   PicmanViewable               *dest_viewable,
                                                                   GtkTreeViewDropPosition     drop_pos);
static void       picman_layer_tree_view_set_image                  (PicmanItemTreeView           *view,
                                                                   PicmanImage                  *image);
static PicmanItem * picman_layer_tree_view_item_new                   (PicmanImage                  *image);
static void       picman_layer_tree_view_floating_selection_changed (PicmanImage                  *image,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_paint_mode_menu_callback   (GtkWidget                  *widget,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_opacity_scale_changed      (GtkAdjustment              *adj,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_lock_alpha_button_toggled  (GtkWidget                  *widget,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_layer_signal_handler       (PicmanLayer                  *layer,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_update_options             (PicmanLayerTreeView          *view,
                                                                   PicmanLayer                  *layer);
static void       picman_layer_tree_view_update_menu                (PicmanLayerTreeView          *view,
                                                                   PicmanLayer                  *layer);
static void       picman_layer_tree_view_mask_update                (PicmanLayerTreeView          *view,
                                                                   GtkTreeIter                *iter,
                                                                   PicmanLayer                  *layer);
static void       picman_layer_tree_view_mask_changed               (PicmanLayer                  *layer,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_renderer_update            (PicmanViewRenderer           *renderer,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_update_borders             (PicmanLayerTreeView          *view,
                                                                   GtkTreeIter                *iter);
static void       picman_layer_tree_view_mask_callback              (PicmanLayer                  *mask,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_layer_clicked              (PicmanCellRendererViewable   *cell,
                                                                   const gchar                *path,
                                                                   GdkModifierType             state,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_mask_clicked               (PicmanCellRendererViewable   *cell,
                                                                   const gchar                *path,
                                                                   GdkModifierType             state,
                                                                   PicmanLayerTreeView          *view);
static void       picman_layer_tree_view_alpha_update               (PicmanLayerTreeView          *view,
                                                                   GtkTreeIter                *iter,
                                                                   PicmanLayer                  *layer);
static void       picman_layer_tree_view_alpha_changed              (PicmanLayer                  *layer,
                                                                   PicmanLayerTreeView          *view);


G_DEFINE_TYPE_WITH_CODE (PicmanLayerTreeView, picman_layer_tree_view,
                         PICMAN_TYPE_DRAWABLE_TREE_VIEW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_layer_tree_view_view_iface_init))

#define parent_class picman_layer_tree_view_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;


static void
picman_layer_tree_view_class_init (PicmanLayerTreeViewClass *klass)
{
  GObjectClass               *object_class = G_OBJECT_CLASS (klass);
  PicmanContainerTreeViewClass *tree_view_class;
  PicmanItemTreeViewClass      *item_view_class;

  tree_view_class = PICMAN_CONTAINER_TREE_VIEW_CLASS (klass);
  item_view_class = PICMAN_ITEM_TREE_VIEW_CLASS (klass);

  object_class->constructed = picman_layer_tree_view_constructed;
  object_class->finalize    = picman_layer_tree_view_finalize;

  tree_view_class->drop_possible   = picman_layer_tree_view_drop_possible;
  tree_view_class->drop_color      = picman_layer_tree_view_drop_color;
  tree_view_class->drop_uri_list   = picman_layer_tree_view_drop_uri_list;
  tree_view_class->drop_component  = picman_layer_tree_view_drop_component;
  tree_view_class->drop_pixbuf     = picman_layer_tree_view_drop_pixbuf;

  item_view_class->item_type       = PICMAN_TYPE_LAYER;
  item_view_class->signal_name     = "active-layer-changed";

  item_view_class->set_image       = picman_layer_tree_view_set_image;
  item_view_class->get_container   = picman_image_get_layers;
  item_view_class->get_active_item = (PicmanGetItemFunc) picman_image_get_active_layer;
  item_view_class->set_active_item = (PicmanSetItemFunc) picman_image_set_active_layer;
  item_view_class->add_item        = (PicmanAddItemFunc) picman_image_add_layer;
  item_view_class->remove_item     = (PicmanRemoveItemFunc) picman_image_remove_layer;
  item_view_class->new_item        = picman_layer_tree_view_item_new;

  item_view_class->action_group          = "layers";
  item_view_class->activate_action       = "layers-text-tool";
  item_view_class->edit_action           = "layers-edit-attributes";
  item_view_class->new_action            = "layers-new";
  item_view_class->new_default_action    = "layers-new-last-values";
  item_view_class->raise_action          = "layers-raise";
  item_view_class->raise_top_action      = "layers-raise-to-top";
  item_view_class->lower_action          = "layers-lower";
  item_view_class->lower_bottom_action   = "layers-lower-to-bottom";
  item_view_class->duplicate_action      = "layers-duplicate";
  item_view_class->delete_action         = "layers-delete";
  item_view_class->lock_content_help_id  = PICMAN_HELP_LAYER_LOCK_PIXELS;
  item_view_class->lock_position_help_id = PICMAN_HELP_LAYER_LOCK_POSITION;

  g_type_class_add_private (klass, sizeof (PicmanLayerTreeViewPriv));
}

static void
picman_layer_tree_view_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  iface->set_container = picman_layer_tree_view_set_container;
  iface->set_context   = picman_layer_tree_view_set_context;
  iface->insert_item   = picman_layer_tree_view_insert_item;
  iface->select_item   = picman_layer_tree_view_select_item;
  iface->set_view_size = picman_layer_tree_view_set_view_size;

  iface->model_is_tree = TRUE;
}

static void
picman_layer_tree_view_init (PicmanLayerTreeView *view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);
  GtkWidget             *scale;
  GtkWidget             *hbox;
  GtkWidget             *image;
  GtkIconSize            icon_size;
  PangoAttribute        *attr;

  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PICMAN_TYPE_LAYER_TREE_VIEW,
                                            PicmanLayerTreeViewPriv);

  view->priv->model_column_mask =
    picman_container_tree_store_columns_add (tree_view->model_columns,
                                           &tree_view->n_model_columns,
                                           PICMAN_TYPE_VIEW_RENDERER);

  view->priv->model_column_mask_visible =
    picman_container_tree_store_columns_add (tree_view->model_columns,
                                           &tree_view->n_model_columns,
                                           G_TYPE_BOOLEAN);

  /*  Paint mode menu  */

  view->priv->paint_mode_menu = picman_paint_mode_menu_new (FALSE, FALSE);
  picman_item_tree_view_add_options (PICMAN_ITEM_TREE_VIEW (view),
                                   _("Mode:"),
                                   view->priv->paint_mode_menu);

  picman_int_combo_box_connect (PICMAN_INT_COMBO_BOX (view->priv->paint_mode_menu),
                              PICMAN_NORMAL_MODE,
                              G_CALLBACK (picman_layer_tree_view_paint_mode_menu_callback),
                              view);

  picman_help_set_help_data (view->priv->paint_mode_menu, NULL,
                           PICMAN_HELP_LAYER_DIALOG_PAINT_MODE_MENU);

  /*  Opacity scale  */

  view->priv->opacity_adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (100.0, 0.0, 100.0,
                                        1.0, 10.0, 0.0));
  scale = picman_spin_scale_new (view->priv->opacity_adjustment, _("Opacity"), 1);
  picman_help_set_help_data (scale, NULL,
                           PICMAN_HELP_LAYER_DIALOG_OPACITY_SCALE);
  picman_item_tree_view_add_options (PICMAN_ITEM_TREE_VIEW (view),
                                   NULL, scale);

  g_signal_connect (view->priv->opacity_adjustment, "value-changed",
                    G_CALLBACK (picman_layer_tree_view_opacity_scale_changed),
                    view);

  /*  Lock alpha toggle  */

  hbox = picman_item_tree_view_get_lock_box (PICMAN_ITEM_TREE_VIEW (view));

  view->priv->lock_alpha_toggle = gtk_toggle_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox), view->priv->lock_alpha_toggle,
                      FALSE, FALSE, 0);
  gtk_widget_show (view->priv->lock_alpha_toggle);

  g_signal_connect (view->priv->lock_alpha_toggle, "toggled",
                    G_CALLBACK (picman_layer_tree_view_lock_alpha_button_toggled),
                    view);

  picman_help_set_help_data (view->priv->lock_alpha_toggle,
                           _("Lock alpha channel"),
                           PICMAN_HELP_LAYER_LOCK_ALPHA);

  gtk_widget_style_get (GTK_WIDGET (view),
                        "button-icon-size", &icon_size,
                        NULL);

  image = gtk_image_new_from_stock (PICMAN_STOCK_TRANSPARENCY, icon_size);
  gtk_container_add (GTK_CONTAINER (view->priv->lock_alpha_toggle), image);
  gtk_widget_show (image);

  view->priv->italic_attrs = pango_attr_list_new ();
  attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
  attr->start_index = 0;
  attr->end_index   = -1;
  pango_attr_list_insert (view->priv->italic_attrs, attr);

  view->priv->bold_attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index   = -1;
  pango_attr_list_insert (view->priv->bold_attrs, attr);
}

static void
picman_layer_tree_view_constructed (GObject *object)
{
  PicmanContainerTreeView *tree_view  = PICMAN_CONTAINER_TREE_VIEW (object);
  PicmanLayerTreeView     *layer_view = PICMAN_LAYER_TREE_VIEW (object);
  GtkWidget             *button;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  layer_view->priv->mask_cell = picman_cell_renderer_viewable_new ();
  gtk_tree_view_column_pack_start (tree_view->main_column,
                                   layer_view->priv->mask_cell,
                                   FALSE);
  gtk_tree_view_column_set_attributes (tree_view->main_column,
                                       layer_view->priv->mask_cell,
                                       "renderer",
                                       layer_view->priv->model_column_mask,
                                       "visible",
                                       layer_view->priv->model_column_mask_visible,
                                       NULL);

  picman_container_tree_view_add_renderer_cell (tree_view,
                                              layer_view->priv->mask_cell);

  g_signal_connect (tree_view->renderer_cell, "clicked",
                    G_CALLBACK (picman_layer_tree_view_layer_clicked),
                    layer_view);
  g_signal_connect (layer_view->priv->mask_cell, "clicked",
                    G_CALLBACK (picman_layer_tree_view_mask_clicked),
                    layer_view);

  picman_dnd_component_dest_add (GTK_WIDGET (tree_view->view),
                               NULL, tree_view);
  picman_dnd_viewable_dest_add  (GTK_WIDGET (tree_view->view), PICMAN_TYPE_CHANNEL,
                               NULL, tree_view);
  picman_dnd_viewable_dest_add  (GTK_WIDGET (tree_view->view), PICMAN_TYPE_LAYER_MASK,
                               NULL, tree_view);
  picman_dnd_uri_list_dest_add  (GTK_WIDGET (tree_view->view),
                               NULL, tree_view);
  picman_dnd_pixbuf_dest_add    (GTK_WIDGET (tree_view->view),
                               NULL, tree_view);

  /*  hide basically useless edit button  */
  gtk_widget_hide (picman_item_tree_view_get_edit_button (PICMAN_ITEM_TREE_VIEW (layer_view)));

  button = picman_editor_add_action_button (PICMAN_EDITOR (layer_view), "layers",
                                          "layers-new-group", NULL);
  gtk_box_reorder_child (picman_editor_get_button_box (PICMAN_EDITOR (layer_view)),
                         button, 2);

  button = picman_editor_add_action_button (PICMAN_EDITOR (layer_view), "layers",
                                          "layers-anchor", NULL);
  picman_container_view_enable_dnd (PICMAN_CONTAINER_VIEW (layer_view),
                                  GTK_BUTTON (button),
                                  PICMAN_TYPE_LAYER);
  gtk_box_reorder_child (picman_editor_get_button_box (PICMAN_EDITOR (layer_view)),
                         button, 6);
}

static void
picman_layer_tree_view_finalize (GObject *object)
{
  PicmanLayerTreeView *layer_view = PICMAN_LAYER_TREE_VIEW (object);

  if (layer_view->priv->italic_attrs)
    {
      pango_attr_list_unref (layer_view->priv->italic_attrs);
      layer_view->priv->italic_attrs = NULL;
    }

  if (layer_view->priv->bold_attrs)
    {
      pango_attr_list_unref (layer_view->priv->bold_attrs);
      layer_view->priv->bold_attrs = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/*  PicmanContainerView methods  */

static void
picman_layer_tree_view_set_container (PicmanContainerView *view,
                                    PicmanContainer     *container)
{
  PicmanLayerTreeView *layer_view = PICMAN_LAYER_TREE_VIEW (view);
  PicmanContainer     *old_container;

  old_container = picman_container_view_get_container (view);

  if (old_container)
    {
      picman_tree_handler_disconnect (layer_view->priv->mode_changed_handler);
      layer_view->priv->mode_changed_handler = NULL;

      picman_tree_handler_disconnect (layer_view->priv->opacity_changed_handler);
      layer_view->priv->opacity_changed_handler = NULL;

      picman_tree_handler_disconnect (layer_view->priv->lock_alpha_changed_handler);
      layer_view->priv->lock_alpha_changed_handler = NULL;

      picman_tree_handler_disconnect (layer_view->priv->mask_changed_handler);
      layer_view->priv->mask_changed_handler = NULL;

      picman_tree_handler_disconnect (layer_view->priv->alpha_changed_handler);
      layer_view->priv->alpha_changed_handler = NULL;
    }

  parent_view_iface->set_container (view, container);

  if (container)
    {
      layer_view->priv->mode_changed_handler =
        picman_tree_handler_connect (container, "mode-changed",
                                   G_CALLBACK (picman_layer_tree_view_layer_signal_handler),
                                   view);

      layer_view->priv->opacity_changed_handler =
        picman_tree_handler_connect (container, "opacity-changed",
                                   G_CALLBACK (picman_layer_tree_view_layer_signal_handler),
                                   view);

      layer_view->priv->lock_alpha_changed_handler =
        picman_tree_handler_connect (container, "lock-alpha-changed",
                                   G_CALLBACK (picman_layer_tree_view_layer_signal_handler),
                                   view);

      layer_view->priv->mask_changed_handler =
        picman_tree_handler_connect (container, "mask-changed",
                                   G_CALLBACK (picman_layer_tree_view_mask_changed),
                                   view);

      layer_view->priv->alpha_changed_handler =
        picman_tree_handler_connect (container, "alpha-changed",
                                   G_CALLBACK (picman_layer_tree_view_alpha_changed),
                                   view);
    }
}

typedef struct
{
  gint         mask_column;
  PicmanContext *context;
} SetContextForeachData;

static gboolean
picman_layer_tree_view_set_context_foreach (GtkTreeModel *model,
                                          GtkTreePath  *path,
                                          GtkTreeIter  *iter,
                                          gpointer      data)
{
  SetContextForeachData *context_data = data;
  PicmanViewRenderer      *renderer;

  gtk_tree_model_get (model, iter,
                      context_data->mask_column, &renderer,
                      -1);

  if (renderer)
    {
      picman_view_renderer_set_context (renderer, context_data->context);

      g_object_unref (renderer);
    }

  return FALSE;
}

static void
picman_layer_tree_view_set_context (PicmanContainerView *view,
                                  PicmanContext       *context)
{
  PicmanContainerTreeView *tree_view  = PICMAN_CONTAINER_TREE_VIEW (view);
  PicmanLayerTreeView     *layer_view = PICMAN_LAYER_TREE_VIEW (view);

  parent_view_iface->set_context (view, context);

  if (tree_view->model)
    {
      SetContextForeachData context_data = { layer_view->priv->model_column_mask,
                                             context };

      gtk_tree_model_foreach (tree_view->model,
                              picman_layer_tree_view_set_context_foreach,
                              &context_data);
    }
}

static gpointer
picman_layer_tree_view_insert_item (PicmanContainerView *view,
                                  PicmanViewable      *viewable,
                                  gpointer           parent_insert_data,
                                  gint               index)
{
  PicmanLayerTreeView *layer_view = PICMAN_LAYER_TREE_VIEW (view);
  PicmanLayer         *layer;
  GtkTreeIter       *iter;

  iter = parent_view_iface->insert_item (view, viewable,
                                         parent_insert_data, index);

  layer = PICMAN_LAYER (viewable);

  if (! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    picman_layer_tree_view_alpha_update (layer_view, iter, layer);

  picman_layer_tree_view_mask_update (layer_view, iter, layer);

  return iter;
}

static gboolean
picman_layer_tree_view_select_item (PicmanContainerView *view,
                                  PicmanViewable      *item,
                                  gpointer           insert_data)
{
  PicmanItemTreeView  *item_view  = PICMAN_ITEM_TREE_VIEW (view);
  PicmanLayerTreeView *layer_view = PICMAN_LAYER_TREE_VIEW (view);
  gboolean           success;

  success = parent_view_iface->select_item (view, item, insert_data);

  if (item)
    {
      if (success)
        {
          picman_layer_tree_view_update_borders (layer_view,
                                               (GtkTreeIter *) insert_data);
          picman_layer_tree_view_update_options (layer_view, PICMAN_LAYER (item));
          picman_layer_tree_view_update_menu (layer_view, PICMAN_LAYER (item));
        }

      if (! success || picman_layer_is_floating_sel (PICMAN_LAYER (item)))
        {
          gtk_widget_set_sensitive (picman_item_tree_view_get_edit_button (item_view), FALSE);
        }
    }

  return success;
}

typedef struct
{
  gint mask_column;
  gint view_size;
  gint border_width;
} SetSizeForeachData;

static gboolean
picman_layer_tree_view_set_view_size_foreach (GtkTreeModel *model,
                                            GtkTreePath  *path,
                                            GtkTreeIter  *iter,
                                            gpointer      data)
{
  SetSizeForeachData *size_data = data;
  PicmanViewRenderer   *renderer;

  gtk_tree_model_get (model, iter,
                      size_data->mask_column, &renderer,
                      -1);

  if (renderer)
    {
      picman_view_renderer_set_size (renderer,
                                   size_data->view_size,
                                   size_data->border_width);

      g_object_unref (renderer);
    }

  return FALSE;
}

static void
picman_layer_tree_view_set_view_size (PicmanContainerView *view)
{
  PicmanContainerTreeView *tree_view  = PICMAN_CONTAINER_TREE_VIEW (view);

  if (tree_view->model)
    {
      PicmanLayerTreeView *layer_view = PICMAN_LAYER_TREE_VIEW (view);
      SetSizeForeachData size_data;

      size_data.mask_column = layer_view->priv->model_column_mask;

      size_data.view_size =
        picman_container_view_get_view_size (view, &size_data.border_width);

      gtk_tree_model_foreach (tree_view->model,
                              picman_layer_tree_view_set_view_size_foreach,
                              &size_data);
    }

  parent_view_iface->set_view_size (view);
}


/*  PicmanContainerTreeView methods  */

static gboolean
picman_layer_tree_view_drop_possible (PicmanContainerTreeView   *tree_view,
                                    PicmanDndType              src_type,
                                    PicmanViewable            *src_viewable,
                                    PicmanViewable            *dest_viewable,
                                    GtkTreePath             *drop_path,
                                    GtkTreeViewDropPosition  drop_pos,
                                    GtkTreeViewDropPosition *return_drop_pos,
                                    GdkDragAction           *return_drag_action)
{
  /* If we are dropping a new layer, check if the destionation image
   * has a floating selection.
   */
  if  (src_type == PICMAN_DND_TYPE_URI_LIST     ||
       src_type == PICMAN_DND_TYPE_TEXT_PLAIN   ||
       src_type == PICMAN_DND_TYPE_NETSCAPE_URL ||
       src_type == PICMAN_DND_TYPE_COMPONENT    ||
       src_type == PICMAN_DND_TYPE_PIXBUF       ||
       PICMAN_IS_DRAWABLE (src_viewable))
    {
      PicmanImage *dest_image = picman_item_tree_view_get_image (PICMAN_ITEM_TREE_VIEW (tree_view));

      if (picman_image_get_floating_selection (dest_image))
        return FALSE;
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
picman_layer_tree_view_drop_color (PicmanContainerTreeView   *view,
                                 const PicmanRGB           *color,
                                 PicmanViewable            *dest_viewable,
                                 GtkTreeViewDropPosition  drop_pos)
{
  if (picman_item_is_text_layer (PICMAN_ITEM (dest_viewable)))
    {
      picman_text_layer_set (PICMAN_TEXT_LAYER (dest_viewable), NULL,
                           "color", color,
                           NULL);
      picman_image_flush (picman_item_tree_view_get_image (PICMAN_ITEM_TREE_VIEW (view)));
      return;
    }

  PICMAN_CONTAINER_TREE_VIEW_CLASS (parent_class)->drop_color (view, color,
                                                             dest_viewable,
                                                             drop_pos);
}

static void
picman_layer_tree_view_drop_uri_list (PicmanContainerTreeView   *view,
                                    GList                   *uri_list,
                                    PicmanViewable            *dest_viewable,
                                    GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView  *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanContainerView *cont_view = PICMAN_CONTAINER_VIEW (view);
  PicmanImage         *image     = picman_item_tree_view_get_image (item_view);
  PicmanLayer         *parent;
  gint               index;
  GList             *list;

  index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                              drop_pos,
                                              (PicmanViewable **) &parent);

  for (list = uri_list; list; list = g_list_next (list))
    {
      const gchar       *uri   = list->data;
      GList             *new_layers;
      PicmanPDBStatusType  status;
      GError            *error = NULL;

      new_layers = file_open_layers (image->picman,
                                     picman_container_view_get_context (cont_view),
                                     NULL,
                                     image, FALSE,
                                     uri, PICMAN_RUN_INTERACTIVE, NULL,
                                     &status, &error);

      if (new_layers)
        {
          picman_image_add_layers (image, new_layers, parent, index,
                                 0, 0,
                                 picman_image_get_width (image),
                                 picman_image_get_height (image),
                                 _("Drop layers"));

          index += g_list_length (new_layers);

          g_list_free (new_layers);
        }
      else if (status != PICMAN_PDB_CANCEL)
        {
          gchar *filename = file_utils_uri_display_name (uri);

          picman_message (image->picman, G_OBJECT (view), PICMAN_MESSAGE_ERROR,
                        _("Opening '%s' failed:\n\n%s"),
                        filename, error->message);

          g_clear_error (&error);
          g_free (filename);
        }
    }

  picman_image_flush (image);
}

static void
picman_layer_tree_view_drop_component (PicmanContainerTreeView   *tree_view,
                                     PicmanImage               *src_image,
                                     PicmanChannelType          component,
                                     PicmanViewable            *dest_viewable,
                                     GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (tree_view);
  PicmanImage        *image     = picman_item_tree_view_get_image (item_view);
  PicmanChannel      *channel;
  PicmanItem         *new_item;
  PicmanLayer        *parent;
  gint              index;
  const gchar      *desc;

  index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                              drop_pos,
                                              (PicmanViewable **) &parent);

  channel = picman_channel_new_from_component (src_image, component, NULL, NULL);

  new_item = picman_item_convert (PICMAN_ITEM (channel), image,
                                PICMAN_TYPE_LAYER);

  g_object_unref (channel);

  picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                       NULL, NULL, &desc, NULL);
  picman_object_take_name (PICMAN_OBJECT (new_item),
                         g_strdup_printf (_("%s Channel Copy"), desc));

  picman_image_add_layer (image, PICMAN_LAYER (new_item), parent, index, TRUE);

  picman_image_flush (image);
}

static void
picman_layer_tree_view_drop_pixbuf (PicmanContainerTreeView   *tree_view,
                                  GdkPixbuf               *pixbuf,
                                  PicmanViewable            *dest_viewable,
                                  GtkTreeViewDropPosition  drop_pos)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (tree_view);
  PicmanImage        *image     = picman_item_tree_view_get_image (item_view);
  PicmanLayer        *new_layer;
  PicmanLayer        *parent;
  gint              index;

  index = picman_item_tree_view_get_drop_index (item_view, dest_viewable,
                                              drop_pos,
                                              (PicmanViewable **) &parent);

  new_layer =
    picman_layer_new_from_pixbuf (pixbuf, image,
                                picman_image_get_layer_format (image, TRUE),
                                _("Dropped Buffer"),
                                PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  picman_image_add_layer (image, new_layer, parent, index, TRUE);

  picman_image_flush (image);
}


/*  PicmanItemTreeView methods  */

static void
picman_layer_tree_view_set_image (PicmanItemTreeView *view,
                                PicmanImage        *image)
{
  if (picman_item_tree_view_get_image (view))
    g_signal_handlers_disconnect_by_func (picman_item_tree_view_get_image (view),
                                          picman_layer_tree_view_floating_selection_changed,
                                          view);

  PICMAN_ITEM_TREE_VIEW_CLASS (parent_class)->set_image (view, image);

  if (picman_item_tree_view_get_image (view))
    g_signal_connect (picman_item_tree_view_get_image (view),
                      "floating-selection-changed",
                      G_CALLBACK (picman_layer_tree_view_floating_selection_changed),
                      view);
}

static PicmanItem *
picman_layer_tree_view_item_new (PicmanImage *image)
{
  PicmanLayer *new_layer;

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE,
                               _("New Layer"));

  new_layer = picman_layer_new (image,
                              picman_image_get_width (image),
                              picman_image_get_height (image),
                              picman_image_get_layer_format (image, TRUE),
                              NULL, 1.0, PICMAN_NORMAL_MODE);

  picman_image_add_layer (image, new_layer,
                        PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  picman_image_undo_group_end (image);

  return PICMAN_ITEM (new_layer);
}


/*  callbacks  */

static void
picman_layer_tree_view_floating_selection_changed (PicmanImage         *image,
                                                 PicmanLayerTreeView *layer_view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  PicmanContainerView     *view      = PICMAN_CONTAINER_VIEW (layer_view);
  PicmanLayer             *floating_sel;
  GtkTreeIter           *iter;

  floating_sel = picman_image_get_floating_selection (image);

  if (floating_sel)
    {
      iter = picman_container_view_lookup (view, (PicmanViewable *) floating_sel);

      if (iter)
        gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                            PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES,
                            layer_view->priv->italic_attrs,
                            -1);
    }
  else
    {
      GList *all_layers;
      GList *list;

      all_layers = picman_image_get_layer_list (image);

      for (list = all_layers; list; list = g_list_next (list))
        {
          PicmanDrawable *drawable = list->data;

          iter = picman_container_view_lookup (view, (PicmanViewable *) drawable);

          if (iter)
            gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                                PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES,
                                picman_drawable_has_alpha (drawable) ?
                                NULL : layer_view->priv->bold_attrs,
                                -1);
        }

      g_list_free (all_layers);
    }
}


/*  Paint Mode, Opacity and Lock alpha callbacks  */

#define BLOCK() \
        g_signal_handlers_block_by_func (layer, \
        picman_layer_tree_view_layer_signal_handler, view)

#define UNBLOCK() \
        g_signal_handlers_unblock_by_func (layer, \
        picman_layer_tree_view_layer_signal_handler, view)


static void
picman_layer_tree_view_paint_mode_menu_callback (GtkWidget         *widget,
                                               PicmanLayerTreeView *view)
{
  PicmanImage *image;
  PicmanLayer *layer = NULL;

  image = picman_item_tree_view_get_image (PICMAN_ITEM_TREE_VIEW (view));

  if (image)
    layer = (PicmanLayer *)
      PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (layer)
    {
      gint mode;

      if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget),
                                         &mode) &&
          picman_layer_get_mode (layer) != (PicmanLayerModeEffects) mode)
        {
          PicmanUndo *undo;
          gboolean  push_undo = TRUE;

          /*  compress layer mode undos  */
          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               PICMAN_UNDO_LAYER_MODE);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
            push_undo = FALSE;

          BLOCK();
          picman_layer_set_mode (layer, (PicmanLayerModeEffects) mode, push_undo);
          UNBLOCK();

          picman_image_flush (image);

          if (! push_undo)
            picman_undo_refresh_preview (undo, picman_container_view_get_context (PICMAN_CONTAINER_VIEW (view)));
        }
    }
}

static void
picman_layer_tree_view_lock_alpha_button_toggled (GtkWidget         *widget,
                                                PicmanLayerTreeView *view)
{
  PicmanImage *image;
  PicmanLayer *layer;

  image = picman_item_tree_view_get_image (PICMAN_ITEM_TREE_VIEW (view));

  layer = (PicmanLayer *)
    PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (layer)
    {
      gboolean lock_alpha;

      lock_alpha = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

      if (picman_layer_get_lock_alpha (layer) != lock_alpha)
        {
          PicmanUndo *undo;
          gboolean  push_undo = TRUE;

          /*  compress lock alpha undos  */
          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               PICMAN_UNDO_LAYER_LOCK_ALPHA);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
            push_undo = FALSE;

          BLOCK();
          picman_layer_set_lock_alpha (layer, lock_alpha, push_undo);
          UNBLOCK();

          picman_image_flush (image);
        }
    }
}

static void
picman_layer_tree_view_opacity_scale_changed (GtkAdjustment     *adjustment,
                                            PicmanLayerTreeView *view)
{
  PicmanImage *image;
  PicmanLayer *layer;

  image = picman_item_tree_view_get_image (PICMAN_ITEM_TREE_VIEW (view));

  layer = (PicmanLayer *)
    PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (image);

  if (layer)
    {
      gdouble opacity = gtk_adjustment_get_value (adjustment) / 100.0;

      if (picman_layer_get_opacity (layer) != opacity)
        {
          PicmanUndo *undo;
          gboolean  push_undo = TRUE;

          /*  compress opacity undos  */
          undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                               PICMAN_UNDO_LAYER_OPACITY);

          if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (layer))
            push_undo = FALSE;

          BLOCK();
          picman_layer_set_opacity (layer, opacity, push_undo);
          UNBLOCK();

          picman_image_flush (image);

          if (! push_undo)
            picman_undo_refresh_preview (undo, picman_container_view_get_context (PICMAN_CONTAINER_VIEW (view)));
        }
    }
}

#undef BLOCK
#undef UNBLOCK


static void
picman_layer_tree_view_layer_signal_handler (PicmanLayer         *layer,
                                           PicmanLayerTreeView *view)
{
  PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (view);
  PicmanLayer        *active_layer;

  active_layer = (PicmanLayer *)
    PICMAN_ITEM_TREE_VIEW_GET_CLASS (view)->get_active_item (picman_item_tree_view_get_image (item_view));

  if (active_layer == layer)
    picman_layer_tree_view_update_options (view, layer);
}


#define BLOCK(object,function) \
        g_signal_handlers_block_by_func ((object), (function), view)

#define UNBLOCK(object,function) \
        g_signal_handlers_unblock_by_func ((object), (function), view)

static void
picman_layer_tree_view_update_options (PicmanLayerTreeView *view,
                                     PicmanLayer         *layer)
{
  BLOCK (view->priv->paint_mode_menu,
         picman_layer_tree_view_paint_mode_menu_callback);

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (view->priv->paint_mode_menu),
                                 picman_layer_get_mode (layer));

  UNBLOCK (view->priv->paint_mode_menu,
           picman_layer_tree_view_paint_mode_menu_callback);

  if (picman_layer_get_lock_alpha (layer) !=
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->priv->lock_alpha_toggle)))
    {
      BLOCK (view->priv->lock_alpha_toggle,
             picman_layer_tree_view_lock_alpha_button_toggled);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->priv->lock_alpha_toggle),
                                    picman_layer_get_lock_alpha (layer));

      UNBLOCK (view->priv->lock_alpha_toggle,
               picman_layer_tree_view_lock_alpha_button_toggled);
    }

  gtk_widget_set_sensitive (view->priv->lock_alpha_toggle,
                            picman_layer_can_lock_alpha (layer));

  if (picman_layer_get_opacity (layer) * 100.0 !=
      gtk_adjustment_get_value (view->priv->opacity_adjustment))
    {
      BLOCK (view->priv->opacity_adjustment,
             picman_layer_tree_view_opacity_scale_changed);

      gtk_adjustment_set_value (view->priv->opacity_adjustment,
                                picman_layer_get_opacity (layer) * 100.0);

      UNBLOCK (view->priv->opacity_adjustment,
               picman_layer_tree_view_opacity_scale_changed);
    }
}

#undef BLOCK
#undef UNBLOCK


static void
picman_layer_tree_view_update_menu (PicmanLayerTreeView *layer_view,
                                  PicmanLayer         *layer)
{
  PicmanUIManager   *ui_manager = picman_editor_get_ui_manager (PICMAN_EDITOR (layer_view));
  PicmanActionGroup *group;
  PicmanLayerMask   *mask;

  group = picman_ui_manager_get_action_group (ui_manager, "layers");

  mask = picman_layer_get_mask (layer);

  picman_action_group_set_action_active (group, "layers-mask-show",
                                       mask &&
                                       picman_layer_get_show_mask (layer));
  picman_action_group_set_action_active (group, "layers-mask-disable",
                                       mask &&
                                       ! picman_layer_get_apply_mask (layer));
  picman_action_group_set_action_active (group, "layers-mask-edit",
                                       mask &&
                                       picman_layer_get_edit_mask (layer));
}


/*  Layer Mask callbacks  */

static void
picman_layer_tree_view_mask_update (PicmanLayerTreeView *layer_view,
                                  GtkTreeIter       *iter,
                                  PicmanLayer         *layer)
{
  PicmanContainerView     *view         = PICMAN_CONTAINER_VIEW (layer_view);
  PicmanContainerTreeView *tree_view    = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  PicmanLayerMask         *mask;
  PicmanViewRenderer      *renderer     = NULL;
  gboolean               mask_visible = FALSE;

  mask = picman_layer_get_mask (layer);

  if (mask)
    {
      GClosure *closure;
      gint      view_size;
      gint      border_width;

      view_size = picman_container_view_get_view_size (view, &border_width);

      mask_visible = TRUE;

      renderer = picman_view_renderer_new (picman_container_view_get_context (view),
                                         G_TYPE_FROM_INSTANCE (mask),
                                         view_size, border_width,
                                         FALSE);
      picman_view_renderer_set_viewable (renderer, PICMAN_VIEWABLE (mask));

      g_signal_connect (renderer, "update",
                        G_CALLBACK (picman_layer_tree_view_renderer_update),
                        layer_view);

      closure = g_cclosure_new (G_CALLBACK (picman_layer_tree_view_mask_callback),
                                layer_view, NULL);
      g_object_watch_closure (G_OBJECT (renderer), closure);
      g_signal_connect_closure (layer, "apply-mask-changed", closure, FALSE);
      g_signal_connect_closure (layer, "edit-mask-changed",  closure, FALSE);
      g_signal_connect_closure (layer, "show-mask-changed",  closure, FALSE);
    }

  gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                      layer_view->priv->model_column_mask,         renderer,
                      layer_view->priv->model_column_mask_visible, mask_visible,
                      -1);

  picman_layer_tree_view_update_borders (layer_view, iter);

  if (renderer)
    {
      picman_view_renderer_remove_idle (renderer);
      g_object_unref (renderer);
    }
}

static void
picman_layer_tree_view_mask_changed (PicmanLayer         *layer,
                                   PicmanLayerTreeView *layer_view)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (layer_view);
  GtkTreeIter       *iter;

  iter = picman_container_view_lookup (view, PICMAN_VIEWABLE (layer));

  if (iter)
    picman_layer_tree_view_mask_update (layer_view, iter, layer);
}

static void
picman_layer_tree_view_renderer_update (PicmanViewRenderer  *renderer,
                                      PicmanLayerTreeView *layer_view)
{
  PicmanContainerView     *view      = PICMAN_CONTAINER_VIEW (layer_view);
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  PicmanLayerMask         *mask;
  GtkTreeIter           *iter;

  mask = PICMAN_LAYER_MASK (renderer->viewable);

  iter = picman_container_view_lookup (view, (PicmanViewable *)
                                     picman_layer_mask_get_layer (mask));

  if (iter)
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (tree_view->model, iter);

      gtk_tree_model_row_changed (tree_view->model, path, iter);

      gtk_tree_path_free (path);
    }
}

static void
picman_layer_tree_view_update_borders (PicmanLayerTreeView *layer_view,
                                     GtkTreeIter       *iter)
{
  PicmanContainerTreeView *tree_view  = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  PicmanViewRenderer      *layer_renderer;
  PicmanViewRenderer      *mask_renderer;
  PicmanLayer             *layer;
  PicmanLayerMask         *mask       = NULL;
  PicmanViewBorderType     layer_type = PICMAN_VIEW_BORDER_BLACK;

  gtk_tree_model_get (tree_view->model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &layer_renderer,
                      layer_view->priv->model_column_mask,       &mask_renderer,
                      -1);

  layer = PICMAN_LAYER (layer_renderer->viewable);

  if (mask_renderer)
    mask = PICMAN_LAYER_MASK (mask_renderer->viewable);

  if (! mask || (mask && ! picman_layer_get_edit_mask (layer)))
    layer_type = PICMAN_VIEW_BORDER_WHITE;

  picman_view_renderer_set_border_type (layer_renderer, layer_type);

  if (mask)
    {
      PicmanViewBorderType mask_color = PICMAN_VIEW_BORDER_BLACK;

      if (picman_layer_get_show_mask (layer))
        {
          mask_color = PICMAN_VIEW_BORDER_GREEN;
        }
      else if (! picman_layer_get_apply_mask (layer))
        {
          mask_color = PICMAN_VIEW_BORDER_RED;
        }
      else if (picman_layer_get_edit_mask (layer))
        {
          mask_color = PICMAN_VIEW_BORDER_WHITE;
        }

      picman_view_renderer_set_border_type (mask_renderer, mask_color);
    }

  if (layer_renderer)
    g_object_unref (layer_renderer);

  if (mask_renderer)
    g_object_unref (mask_renderer);
}

static void
picman_layer_tree_view_mask_callback (PicmanLayer         *layer,
                                    PicmanLayerTreeView *layer_view)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (layer_view);
  GtkTreeIter       *iter;

  iter = picman_container_view_lookup (view, (PicmanViewable *) layer);

  picman_layer_tree_view_update_borders (layer_view, iter);
}

static void
picman_layer_tree_view_layer_clicked (PicmanCellRendererViewable *cell,
                                    const gchar              *path_str,
                                    GdkModifierType           state,
                                    PicmanLayerTreeView        *layer_view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  GtkTreePath           *path;
  GtkTreeIter            iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanUIManager    *ui_manager = picman_editor_get_ui_manager (PICMAN_EDITOR (tree_view));
      PicmanActionGroup  *group;
      PicmanViewRenderer *renderer;

      group = picman_ui_manager_get_action_group (ui_manager, "layers");

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      if (renderer)
        {
          PicmanLayer *layer = PICMAN_LAYER (renderer->viewable);

          if (picman_layer_get_mask (layer) &&
              picman_layer_get_edit_mask (layer))
            {
              picman_action_group_set_action_active (group,
                                                   "layers-mask-edit", FALSE);
            }

          g_object_unref (renderer);
        }
    }

  gtk_tree_path_free (path);
}

static void
picman_layer_tree_view_mask_clicked (PicmanCellRendererViewable *cell,
                                   const gchar              *path_str,
                                   GdkModifierType           state,
                                   PicmanLayerTreeView        *layer_view)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (layer_view);
  GtkTreePath           *path;
  GtkTreeIter            iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      PicmanUIManager    *ui_manager;
      PicmanActionGroup  *group;

      ui_manager = picman_editor_get_ui_manager (PICMAN_EDITOR (tree_view));
      group      = picman_ui_manager_get_action_group (ui_manager, "layers");

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      if (renderer)
        {
          PicmanLayer *layer = PICMAN_LAYER (renderer->viewable);

          if (state & GDK_MOD1_MASK)
            picman_action_group_set_action_active (group, "layers-mask-show",
                                                 ! picman_layer_get_show_mask (layer));
          else if (state & picman_get_toggle_behavior_mask ())
            picman_action_group_set_action_active (group, "layers-mask-disable",
                                                 picman_layer_get_apply_mask (layer));
          else if (! picman_layer_get_edit_mask (layer))
            picman_action_group_set_action_active (group,
                                                 "layers-mask-edit", TRUE);

          g_object_unref (renderer);
        }
    }

  gtk_tree_path_free (path);
}


/*  PicmanDrawable alpha callbacks  */

static void
picman_layer_tree_view_alpha_update (PicmanLayerTreeView *view,
                                   GtkTreeIter       *iter,
                                   PicmanLayer         *layer)
{
  PicmanContainerTreeView *tree_view = PICMAN_CONTAINER_TREE_VIEW (view);

  gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES,
                      picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)) ?
                      NULL : view->priv->bold_attrs,
                      -1);
}

static void
picman_layer_tree_view_alpha_changed (PicmanLayer         *layer,
                                    PicmanLayerTreeView *layer_view)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (layer_view);
  GtkTreeIter       *iter;

  iter = picman_container_view_lookup (view, (PicmanViewable *) layer);

  if (iter)
    {
      PicmanItemTreeView *item_view = PICMAN_ITEM_TREE_VIEW (view);

      picman_layer_tree_view_alpha_update (layer_view, iter, layer);

      /*  update button states  */
      if (picman_image_get_active_layer (picman_item_tree_view_get_image (item_view)) == layer)
        picman_container_view_select_item (PICMAN_CONTAINER_VIEW (view),
                                         PICMAN_VIEWABLE (layer));
    }
}
