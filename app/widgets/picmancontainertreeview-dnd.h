/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreeview-dnd.h
 * Copyright (C) 2003-2009 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_TREE_VIEW_DND_H__
#define __PICMAN_CONTAINER_TREE_VIEW_DND_H__


void     picman_container_tree_view_drag_leave  (GtkWidget             *widget,
                                               GdkDragContext        *context,
                                               guint                  time,
                                               PicmanContainerTreeView *view);
gboolean picman_container_tree_view_drag_motion (GtkWidget             *widget,
                                               GdkDragContext        *context,
                                               gint                   x,
                                               gint                   y,
                                               guint                  time,
                                               PicmanContainerTreeView *view);
gboolean picman_container_tree_view_drag_drop   (GtkWidget             *widget,
                                               GdkDragContext        *context,
                                               gint                   x,
                                               gint                   y,
                                               guint                  time,
                                               PicmanContainerTreeView *view);
void     picman_container_tree_view_drag_data_received
                                              (GtkWidget             *widget,
                                               GdkDragContext        *context,
                                               gint                   x,
                                               gint                   y,
                                               GtkSelectionData      *selection_data,
                                               guint                  info,
                                               guint                  time,
                                               PicmanContainerTreeView *view);

gboolean
picman_container_tree_view_real_drop_possible (PicmanContainerTreeView   *tree_view,
                                             PicmanDndType              src_type,
                                             PicmanViewable            *src_viewable,
                                             PicmanViewable            *dest_viewable,
                                             GtkTreePath             *drop_path,
                                             GtkTreeViewDropPosition  drop_pos,
                                             GtkTreeViewDropPosition *return_drop_pos,
                                             GdkDragAction           *return_drag_action);
void
picman_container_tree_view_real_drop_viewable (PicmanContainerTreeView   *tree_view,
                                             PicmanViewable            *src_viewable,
                                             PicmanViewable            *dest_viewable,
                                             GtkTreeViewDropPosition  drop_pos);


#endif  /*  __PICMAN_CONTAINER_TREE_VIEW_DND_H__  */
