/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreeview-private.h
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

#ifndef __PICMAN_CONTAINER_TREE_VIEW_PRIVATE_H__
#define __PICMAN_CONTAINER_TREE_VIEW_PRIVATE_H__


struct _PicmanContainerTreeViewPriv
{
  GtkTreeSelection   *selection;

  GtkCellRenderer    *name_cell;

  GList              *editable_cells;

  PicmanViewRenderer   *dnd_renderer;

  GList              *toggle_cells;
  GList              *renderer_cells;

  guint               scroll_timeout_id;
  guint               scroll_timeout_interval;
  GdkScrollDirection  scroll_dir;

  gboolean            dnd_drop_to_empty;
};


#endif  /*  __PICMAN_CONTAINER_TREE_VIEW_PRIVATE_H__  */
