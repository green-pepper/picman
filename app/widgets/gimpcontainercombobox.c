/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainercombobox.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanviewable.h"

#include "picmancellrendererviewable.h"
#include "picmancontainercombobox.h"
#include "picmancontainertreestore.h"
#include "picmancontainerview.h"
#include "picmanviewrenderer.h"


enum
{
  PROP_0,
  PROP_ELLIPSIZE = PICMAN_CONTAINER_VIEW_PROP_LAST + 1
};


static void     picman_container_combo_box_view_iface_init (PicmanContainerViewInterface *iface);

static void     picman_container_combo_box_set_property (GObject                *object,
                                                       guint                   property_id,
                                                       const GValue           *value,
                                                       GParamSpec             *pspec);
static void     picman_container_combo_box_get_property (GObject                *object,
                                                       guint                   property_id,
                                                       GValue                 *value,
                                                       GParamSpec             *pspec);

static void     picman_container_combo_box_set_context  (PicmanContainerView      *view,
                                                       PicmanContext            *context);
static gpointer picman_container_combo_box_insert_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                parent_insert_data,
                                                       gint                    index);
static void     picman_container_combo_box_remove_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static void     picman_container_combo_box_reorder_item (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gint                    new_index,
                                                       gpointer                insert_data);
static void     picman_container_combo_box_rename_item  (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static gboolean  picman_container_combo_box_select_item (PicmanContainerView      *view,
                                                       PicmanViewable           *viewable,
                                                       gpointer                insert_data);
static void     picman_container_combo_box_clear_items  (PicmanContainerView      *view);
static void    picman_container_combo_box_set_view_size (PicmanContainerView      *view);

static void     picman_container_combo_box_changed      (GtkComboBox            *combo_box,
                                                       PicmanContainerView      *view);


G_DEFINE_TYPE_WITH_CODE (PicmanContainerComboBox, picman_container_combo_box,
                         GTK_TYPE_COMBO_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONTAINER_VIEW,
                                                picman_container_combo_box_view_iface_init))

#define parent_class picman_container_combo_box_parent_class

static PicmanContainerViewInterface *parent_view_iface = NULL;


static void
picman_container_combo_box_class_init (PicmanContainerComboBoxClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_container_combo_box_set_property;
  object_class->get_property = picman_container_combo_box_get_property;

  picman_container_view_install_properties (object_class);

  g_object_class_install_property (object_class,
                                   PROP_ELLIPSIZE,
                                   g_param_spec_enum ("ellipsize", NULL, NULL,
						      PANGO_TYPE_ELLIPSIZE_MODE,
						      PANGO_ELLIPSIZE_MIDDLE,
						      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
}

static void
picman_container_combo_box_view_iface_init (PicmanContainerViewInterface *iface)
{
  parent_view_iface = g_type_interface_peek_parent (iface);

  if (! parent_view_iface)
    parent_view_iface = g_type_default_interface_peek (PICMAN_TYPE_CONTAINER_VIEW);

  iface->set_context   = picman_container_combo_box_set_context;
  iface->insert_item   = picman_container_combo_box_insert_item;
  iface->remove_item   = picman_container_combo_box_remove_item;
  iface->reorder_item  = picman_container_combo_box_reorder_item;
  iface->rename_item   = picman_container_combo_box_rename_item;
  iface->select_item   = picman_container_combo_box_select_item;
  iface->clear_items   = picman_container_combo_box_clear_items;
  iface->set_view_size = picman_container_combo_box_set_view_size;

  iface->insert_data_free = (GDestroyNotify) gtk_tree_iter_free;
}

static void
picman_container_combo_box_init (PicmanContainerComboBox *combo)
{
  GtkTreeModel    *model;
  GtkCellLayout   *layout;
  GtkCellRenderer *cell;
  GType            types[PICMAN_CONTAINER_TREE_STORE_N_COLUMNS];
  gint             n_types = 0;

  picman_container_tree_store_columns_init (types, &n_types);

  model = picman_container_tree_store_new (PICMAN_CONTAINER_VIEW (combo),
                                         n_types, types);

  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);

  g_object_unref (model);

  layout = GTK_CELL_LAYOUT (combo);

  cell = picman_cell_renderer_viewable_new ();
  gtk_cell_layout_pack_start (layout, cell, FALSE);
  gtk_cell_layout_set_attributes (layout, cell,
                                  "renderer",
                                  PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER,
                                  NULL);

  picman_container_tree_store_add_renderer_cell (PICMAN_CONTAINER_TREE_STORE (model),
                                               cell);

  combo->viewable_renderer = cell;

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (layout, cell, TRUE);
  gtk_cell_layout_set_attributes (layout, cell,
                                  "text",
                                  PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME,
                                  NULL);

  combo->text_renderer = cell;

  g_signal_connect (combo, "changed",
                    G_CALLBACK (picman_container_combo_box_changed),
                    combo);

  gtk_widget_set_sensitive (GTK_WIDGET (combo), FALSE);
}

static void
picman_container_combo_box_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  PicmanContainerComboBox *combo = PICMAN_CONTAINER_COMBO_BOX (object);

  switch (property_id)
    {
    case PROP_ELLIPSIZE:
      g_object_set_property (G_OBJECT (combo->text_renderer),
                             pspec->name, value);
      break;

    default:
      picman_container_view_set_property (object, property_id, value, pspec);
      break;
    }
}

static void
picman_container_combo_box_get_property (GObject    *object,
                                       guint       property_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  PicmanContainerComboBox *combo = PICMAN_CONTAINER_COMBO_BOX (object);

  switch (property_id)
    {
    case PROP_ELLIPSIZE:
      g_object_get_property (G_OBJECT (combo->text_renderer),
                             pspec->name, value);
      break;

    default:
      picman_container_view_get_property (object, property_id, value, pspec);
      break;
    }
}

GtkWidget *
picman_container_combo_box_new (PicmanContainer *container,
                              PicmanContext   *context,
                              gint           view_size,
                              gint           view_border_width)
{
  GtkWidget         *combo_box;
  PicmanContainerView *view;

  g_return_val_if_fail (container == NULL || PICMAN_IS_CONTAINER (container),
                        NULL);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);

  combo_box = g_object_new (PICMAN_TYPE_CONTAINER_COMBO_BOX, NULL);

  view = PICMAN_CONTAINER_VIEW (combo_box);

  picman_container_view_set_view_size (view, view_size, view_border_width);

  if (container)
    picman_container_view_set_container (view, container);

  if (context)
    picman_container_view_set_context (view, context);

  return combo_box;
}


/*  PicmanContainerView methods  */

static void
picman_container_combo_box_set_context (PicmanContainerView *view,
                                      PicmanContext       *context)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  parent_view_iface->set_context (view, context);

  if (model)
    picman_container_tree_store_set_context (PICMAN_CONTAINER_TREE_STORE (model),
                                           context);
}

static gpointer
picman_container_combo_box_insert_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           parent_insert_data,
                                      gint               index)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    {
      GtkTreeIter *iter;

      iter = picman_container_tree_store_insert_item (PICMAN_CONTAINER_TREE_STORE (model),
                                                    viewable,
                                                    parent_insert_data,
                                                    index);

      if (gtk_tree_model_iter_n_children (model, NULL) == 1)
        {
          /*  PicmanContainerViews don't select items by default  */
          gtk_combo_box_set_active (GTK_COMBO_BOX (view), -1);

          gtk_widget_set_sensitive (GTK_WIDGET (view), TRUE);
        }

      return iter;
    }

  return NULL;
}

static void
picman_container_combo_box_remove_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    {
      GtkTreeIter *iter = insert_data;

      picman_container_tree_store_remove_item (PICMAN_CONTAINER_TREE_STORE (model),
                                             viewable,
                                             iter);

      if (iter && gtk_tree_model_iter_n_children (model, NULL) == 0)
        {
          gtk_widget_set_sensitive (GTK_WIDGET (view), FALSE);
        }
    }
}

static void
picman_container_combo_box_reorder_item (PicmanContainerView *view,
                                       PicmanViewable      *viewable,
                                       gint               new_index,
                                       gpointer           insert_data)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    picman_container_tree_store_reorder_item (PICMAN_CONTAINER_TREE_STORE (model),
                                            viewable,
                                            new_index,
                                            insert_data);
}

static void
picman_container_combo_box_rename_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    picman_container_tree_store_rename_item (PICMAN_CONTAINER_TREE_STORE (model),
                                           viewable,
                                           insert_data);
}

static gboolean
picman_container_combo_box_select_item (PicmanContainerView *view,
                                      PicmanViewable      *viewable,
                                      gpointer           insert_data)
{
  GtkComboBox *combo_box = GTK_COMBO_BOX (view);

  if (gtk_combo_box_get_model (GTK_COMBO_BOX (view)))
    {
      GtkTreeIter *iter = insert_data;

     g_signal_handlers_block_by_func (combo_box,
                                       picman_container_combo_box_changed,
                                       view);

      if (iter)
        {
          gtk_combo_box_set_active_iter (combo_box, iter);
        }
      else
        {
          gtk_combo_box_set_active (combo_box, -1);
        }

      g_signal_handlers_unblock_by_func (combo_box,
                                         picman_container_combo_box_changed,
                                         view);
    }

  return TRUE;
}

static void
picman_container_combo_box_clear_items (PicmanContainerView *view)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    picman_container_tree_store_clear_items (PICMAN_CONTAINER_TREE_STORE (model));

  gtk_widget_set_sensitive (GTK_WIDGET (view), FALSE);

  parent_view_iface->clear_items (view);
}

static void
picman_container_combo_box_set_view_size (PicmanContainerView *view)
{
  GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (view));

  if (model)
    picman_container_tree_store_set_view_size (PICMAN_CONTAINER_TREE_STORE (model));
}

static void
picman_container_combo_box_changed (GtkComboBox       *combo,
                                  PicmanContainerView *view)
{
  GtkTreeIter iter;

  if (gtk_combo_box_get_active_iter (combo, &iter))
    {
      PicmanViewRenderer *renderer;

      gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      picman_container_view_item_selected (view, renderer->viewable);
      g_object_unref (renderer);
    }
}
