/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanitemtreeview.h
 * Copyright (C) 2001-2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_ITEM_TREE_VIEW_H__
#define __PICMAN_ITEM_TREE_VIEW_H__


#include "picmancontainertreeview.h"


typedef PicmanContainer * (* PicmanGetContainerFunc) (const PicmanImage *image);
typedef PicmanItem      * (* PicmanGetItemFunc)      (const PicmanImage *image);
typedef void            (* PicmanSetItemFunc)      (PicmanImage       *image,
                                                  PicmanItem        *item);
typedef void            (* PicmanAddItemFunc)      (PicmanImage       *image,
                                                  PicmanItem        *item,
                                                  PicmanItem        *parent,
                                                  gint             index,
                                                  gboolean         push_undo);
typedef void            (* PicmanRemoveItemFunc)   (PicmanImage       *image,
                                                  PicmanItem        *item,
                                                  gboolean         push_undo,
                                                  PicmanItem        *new_active);
typedef PicmanItem      * (* PicmanNewItemFunc)      (PicmanImage       *image);


#define PICMAN_TYPE_ITEM_TREE_VIEW            (picman_item_tree_view_get_type ())
#define PICMAN_ITEM_TREE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ITEM_TREE_VIEW, PicmanItemTreeView))
#define PICMAN_ITEM_TREE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ITEM_TREE_VIEW, PicmanItemTreeViewClass))
#define PICMAN_IS_ITEM_TREE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ITEM_TREE_VIEW))
#define PICMAN_IS_ITEM_TREE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ITEM_TREE_VIEW))
#define PICMAN_ITEM_TREE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ITEM_TREE_VIEW, PicmanItemTreeViewClass))


typedef struct _PicmanItemTreeViewClass  PicmanItemTreeViewClass;
typedef struct _PicmanItemTreeViewPriv   PicmanItemTreeViewPriv;

struct _PicmanItemTreeView
{
  PicmanContainerTreeView  parent_instance;

  PicmanItemTreeViewPriv  *priv;
};

struct _PicmanItemTreeViewClass
{
  PicmanContainerTreeViewClass  parent_class;

  /*  signals  */
  void (* set_image) (PicmanItemTreeView *view,
                      PicmanImage        *image);

  GType                 item_type;
  const gchar          *signal_name;

  /*  virtual functions for manipulating the image's item tree  */
  PicmanGetContainerFunc  get_container;
  PicmanGetItemFunc       get_active_item;
  PicmanSetItemFunc       set_active_item;
  PicmanAddItemFunc       add_item;
  PicmanRemoveItemFunc    remove_item;
  PicmanNewItemFunc       new_item;

  /*  action names  */
  const gchar          *action_group;
  const gchar          *activate_action;
  const gchar          *edit_action;
  const gchar          *new_action;
  const gchar          *new_default_action;
  const gchar          *raise_action;
  const gchar          *raise_top_action;
  const gchar          *lower_action;
  const gchar          *lower_bottom_action;
  const gchar          *duplicate_action;
  const gchar          *delete_action;

  /*  lock content button appearance  */
  const gchar          *lock_content_stock_id;
  const gchar          *lock_content_tooltip;
  const gchar          *lock_content_help_id;

  /* lock position (translation and transformation) button appearance */
  const gchar          *lock_position_stock_id;
  const gchar          *lock_position_tooltip;
  const gchar          *lock_position_help_id;
};


GType       picman_item_tree_view_get_type        (void) G_GNUC_CONST;

GtkWidget * picman_item_tree_view_new             (GType             view_type,
                                                 gint              view_size,
                                                 gint              view_border_width,
                                                 PicmanImage        *image,
                                                 PicmanMenuFactory  *menu_facotry,
                                                 const gchar      *menu_identifier,
                                                 const gchar      *ui_identifier);

void        picman_item_tree_view_set_image       (PicmanItemTreeView *view,
                                                 PicmanImage        *image);
PicmanImage * picman_item_tree_view_get_image       (PicmanItemTreeView *view);

void        picman_item_tree_view_add_options     (PicmanItemTreeView *view,
                                                 const gchar      *label,
                                                 GtkWidget        *options);
GtkWidget * picman_item_tree_view_get_lock_box    (PicmanItemTreeView *view);

GtkWidget * picman_item_tree_view_get_new_button  (PicmanItemTreeView *view);
GtkWidget * picman_item_tree_view_get_edit_button (PicmanItemTreeView *view);

gint        picman_item_tree_view_get_drop_index  (PicmanItemTreeView *view,
                                                 PicmanViewable     *dest_viewable,
                                                 GtkTreeViewDropPosition drop_pos,
                                                 PicmanViewable    **parent);


#endif  /*  __PICMAN_ITEM_TREE_VIEW_H__  */
