/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreeview.h
 * Copyright (C) 2003-2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_TREE_VIEW_H__
#define __PICMAN_CONTAINER_TREE_VIEW_H__


#include "picmancontainerbox.h"


#define PICMAN_TYPE_CONTAINER_TREE_VIEW            (picman_container_tree_view_get_type ())
#define PICMAN_CONTAINER_TREE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_TREE_VIEW, PicmanContainerTreeView))
#define PICMAN_CONTAINER_TREE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_TREE_VIEW, PicmanContainerTreeViewClass))
#define PICMAN_IS_CONTAINER_TREE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_TREE_VIEW))
#define PICMAN_IS_CONTAINER_TREE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_TREE_VIEW))
#define PICMAN_CONTAINER_TREE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_TREE_VIEW, PicmanContainerTreeViewClass))


typedef struct _PicmanContainerTreeViewClass PicmanContainerTreeViewClass;
typedef struct _PicmanContainerTreeViewPriv  PicmanContainerTreeViewPriv;

struct _PicmanContainerTreeView
{
  PicmanContainerBox           parent_instance;

  GtkTreeModel              *model;
  gint                       n_model_columns;
  GType                      model_columns[16];

  GtkTreeView               *view;

  GtkTreeViewColumn         *main_column;
  GtkCellRenderer           *renderer_cell;

  Picman                      *dnd_picman; /* eek */

  PicmanContainerTreeViewPriv *priv;
};

struct _PicmanContainerTreeViewClass
{
  PicmanContainerBoxClass  parent_class;

  /* signals */

  void     (* edit_name)      (PicmanContainerTreeView   *tree_view);

  /* virtual functions */

  gboolean (* drop_possible)  (PicmanContainerTreeView   *tree_view,
                               PicmanDndType              src_type,
                               PicmanViewable            *src_viewable,
                               PicmanViewable            *dest_viewable,
                               GtkTreePath             *drop_path,
                               GtkTreeViewDropPosition  drop_pos,
                               GtkTreeViewDropPosition *return_drop_pos,
                               GdkDragAction           *return_drag_action);
  void     (* drop_viewable)  (PicmanContainerTreeView   *tree_view,
                               PicmanViewable            *src_viewable,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
  void     (* drop_color)     (PicmanContainerTreeView   *tree_view,
                               const PicmanRGB           *src_color,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
  void     (* drop_uri_list)  (PicmanContainerTreeView   *tree_view,
                               GList                   *uri_list,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
  void     (* drop_svg)       (PicmanContainerTreeView   *tree_view,
                               const gchar             *svg_data,
                               gsize                    svg_data_length,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
  void     (* drop_component) (PicmanContainerTreeView   *tree_view,
                               PicmanImage               *image,
                               PicmanChannelType          component,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
  void     (* drop_pixbuf)    (PicmanContainerTreeView   *tree_view,
                               GdkPixbuf               *pixbuf,
                               PicmanViewable            *dest_viewable,
                               GtkTreeViewDropPosition  drop_pos);
};


GType       picman_container_tree_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_container_tree_view_new      (PicmanContainer *container,
                                               PicmanContext   *context,
                                               gint           view_size,
                                               gint           view_border_width);

void        picman_container_tree_view_set_main_column_title
                                              (PicmanContainerTreeView *tree_view,
                                               const gchar           *title);

void        picman_container_tree_view_add_toggle_cell
                                              (PicmanContainerTreeView *tree_view,
                                               GtkCellRenderer       *cell);

void        picman_container_tree_view_add_renderer_cell
                                              (PicmanContainerTreeView *tree_view,
                                               GtkCellRenderer       *cell);

void        picman_container_tree_view_set_dnd_drop_to_empty
                                              (PicmanContainerTreeView *tree_view,
                                               gboolean               dnd_drop_to_emtpy);
void        picman_container_tree_view_connect_name_edited
                                              (PicmanContainerTreeView *tree_view,
                                               GCallback              callback,
                                               gpointer               data);


#endif  /*  __PICMAN_CONTAINER_TREE_VIEW_H__  */
