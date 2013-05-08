/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanitemtree.h
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

#ifndef __PICMAN_ITEM_TREE_H__
#define __PICMAN_ITEM_TREE_H__


#include "picmanobject.h"


#define PICMAN_TYPE_ITEM_TREE            (picman_item_tree_get_type ())
#define PICMAN_ITEM_TREE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ITEM_TREE, PicmanItemTree))
#define PICMAN_ITEM_TREE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ITEM_TREE, PicmanItemTreeClass))
#define PICMAN_IS_ITEM_TREE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ITEM_TREE))
#define PICMAN_IS_ITEM_TREE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ITEM_TREE))


typedef struct _PicmanItemTreeClass PicmanItemTreeClass;

struct _PicmanItemTree
{
  PicmanObject     parent_instance;

  PicmanContainer *container;
};

struct _PicmanItemTreeClass
{
  PicmanObjectClass  parent_class;
};


GType          picman_item_tree_get_type         (void) G_GNUC_CONST;
PicmanItemTree * picman_item_tree_new              (PicmanImage     *image,
                                                GType          container_type,
                                                GType          item_type);

PicmanItem     * picman_item_tree_get_active_item  (PicmanItemTree  *tree);
void           picman_item_tree_set_active_item  (PicmanItemTree  *tree,
                                                PicmanItem      *item);

PicmanItem     * picman_item_tree_get_item_by_name (PicmanItemTree  *tree,
                                                const gchar   *name);

gboolean       picman_item_tree_get_insert_pos   (PicmanItemTree  *tree,
                                                PicmanItem      *item,
                                                PicmanItem     **parent,
                                                gint          *position);

void           picman_item_tree_add_item         (PicmanItemTree  *tree,
                                                PicmanItem      *item,
                                                PicmanItem      *parent,
                                                gint           position);
PicmanItem     * picman_item_tree_remove_item      (PicmanItemTree  *tree,
                                                PicmanItem      *item,
                                                PicmanItem      *new_active);

gboolean       picman_item_tree_reorder_item     (PicmanItemTree  *tree,
                                                PicmanItem      *item,
                                                PicmanItem      *new_parent,
                                                gint           new_index,
                                                gboolean       push_undo,
                                                const gchar   *undo_desc);

void           picman_item_tree_rename_item      (PicmanItemTree  *tree,
                                                PicmanItem      *item,
                                                const gchar   *new_name,
                                                gboolean       push_undo,
                                                const gchar   *undo_desc);


#endif  /*  __PICMAN_ITEM_TREE_H__  */
