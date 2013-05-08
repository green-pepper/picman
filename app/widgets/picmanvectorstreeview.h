/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectorstreeview.h
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

#ifndef __PICMAN_VECTORS_TREE_VIEW_H__
#define __PICMAN_VECTORS_TREE_VIEW_H__


#include "picmanitemtreeview.h"


#define PICMAN_TYPE_VECTORS_TREE_VIEW            (picman_vectors_tree_view_get_type ())
#define PICMAN_VECTORS_TREE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VECTORS_TREE_VIEW, PicmanVectorsTreeView))
#define PICMAN_VECTORS_TREE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VECTORS_TREE_VIEW, PicmanVectorsTreeViewClass))
#define PICMAN_IS_VECTORS_TREE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VECTORS_TREE_VIEW))
#define PICMAN_IS_VECTORS_TREE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VECTORS_TREE_VIEW))
#define PICMAN_VECTORS_TREE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VECTORS_TREE_VIEW, PicmanVectorsTreeViewClass))


typedef struct _PicmanVectorsTreeViewClass  PicmanVectorsTreeViewClass;

struct _PicmanVectorsTreeView
{
  PicmanItemTreeView  parent_instance;

  GtkWidget        *toselection_button;
  GtkWidget        *tovectors_button;
  GtkWidget        *stroke_button;
};

struct _PicmanVectorsTreeViewClass
{
  PicmanItemTreeViewClass  parent_class;
};


GType   picman_vectors_tree_view_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_VECTORS_TREE_VIEW_H__  */
