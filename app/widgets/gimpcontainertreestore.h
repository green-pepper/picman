/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreestore.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_TREE_STORE_H__
#define __PICMAN_CONTAINER_TREE_STORE_H__


enum
{
  PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER,
  PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME,
  PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES,
  PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE,
  PICMAN_CONTAINER_TREE_STORE_COLUMN_USER_DATA,
  PICMAN_CONTAINER_TREE_STORE_N_COLUMNS
};


#define PICMAN_TYPE_CONTAINER_TREE_STORE            (picman_container_tree_store_get_type ())
#define PICMAN_CONTAINER_TREE_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_TREE_STORE, PicmanContainerTreeStore))
#define PICMAN_CONTAINER_TREE_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_TREE_STORE, PicmanContainerTreeStoreClass))
#define PICMAN_IS_CONTAINER_TREE_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_TREE_STORE))
#define PICMAN_IS_CONTAINER_TREE_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_TREE_STORE))
#define PICMAN_CONTAINER_TREE_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_TREE_STORE, PicmanContainerTreeStoreClass))


typedef struct _PicmanContainerTreeStoreClass PicmanContainerTreeStoreClass;

struct _PicmanContainerTreeStore
{
  GtkTreeStore  parent_instance;
};

struct _PicmanContainerTreeStoreClass
{
  GtkTreeStoreClass  parent_class;
};


GType          picman_container_tree_store_get_type      (void) G_GNUC_CONST;

void           picman_container_tree_store_columns_init  (GType                  *types,
                                                        gint                   *n_types);
gint           picman_container_tree_store_columns_add   (GType                  *types,
                                                        gint                   *n_types,
                                                        GType                   type);

GtkTreeModel * picman_container_tree_store_new           (PicmanContainerView      *container_view,
                                                        gint                    n_columns,
                                                        GType                  *types);

void       picman_container_tree_store_add_renderer_cell (PicmanContainerTreeStore *store,
                                                        GtkCellRenderer        *cell);
void           picman_container_tree_store_set_use_name  (PicmanContainerTreeStore *store,
                                                        gboolean                use_name);
gboolean       picman_container_tree_store_get_use_name  (PicmanContainerTreeStore *store);

void           picman_container_tree_store_set_context   (PicmanContainerTreeStore *store,
                                                        PicmanContext            *context);
GtkTreeIter *  picman_container_tree_store_insert_item   (PicmanContainerTreeStore *store,
                                                        PicmanViewable           *viewable,
                                                        GtkTreeIter            *parent,
                                                        gint                    index);
void           picman_container_tree_store_remove_item   (PicmanContainerTreeStore *store,
                                                        PicmanViewable           *viewable,
                                                        GtkTreeIter            *iter);
void           picman_container_tree_store_reorder_item  (PicmanContainerTreeStore *store,
                                                        PicmanViewable           *viewable,
                                                        gint                    new_index,
                                                        GtkTreeIter            *iter);
gboolean       picman_container_tree_store_rename_item   (PicmanContainerTreeStore *store,
                                                        PicmanViewable           *viewable,
                                                        GtkTreeIter            *iter);
void           picman_container_tree_store_clear_items   (PicmanContainerTreeStore *store);
void           picman_container_tree_store_set_view_size (PicmanContainerTreeStore *store);


#endif  /*  __PICMAN_CONTAINER_TREE_STORE_H__  */
