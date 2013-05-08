/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancomponenteditor.h
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

#ifndef __PICMAN_COMPONENT_EDITOR_H__
#define __PICMAN_COMPONENT_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_COMPONENT_EDITOR            (picman_component_editor_get_type ())
#define PICMAN_COMPONENT_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COMPONENT_EDITOR, PicmanComponentEditor))
#define PICMAN_COMPONENT_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COMPONENT_EDITOR, PicmanComponentEditorClass))
#define PICMAN_IS_COMPONENT_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COMPONENT_EDITOR))
#define PICMAN_IS_COMPONENT_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COMPONENT_EDITOR))
#define PICMAN_COMPONENT_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COMPONENT_EDITOR, PicmanComponentEditorClass))


typedef struct _PicmanComponentEditorClass  PicmanComponentEditorClass;

struct _PicmanComponentEditor
{
  PicmanImageEditor    parent_instance;

  gint               view_size;

  GtkTreeModel      *model;
  GtkTreeView       *view;
  GtkTreeSelection  *selection;

  GtkTreeViewColumn *eye_column;
  GtkCellRenderer   *eye_cell;
  GtkCellRenderer   *renderer_cell;

  PicmanChannelType    clicked_component;
};

struct _PicmanComponentEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_component_editor_get_type      (void) G_GNUC_CONST;

GtkWidget * picman_component_editor_new           (gint                 view_size,
                                                 PicmanMenuFactory     *menu_factory);
void        picman_component_editor_set_view_size (PicmanComponentEditor *editor,
                                                 gint                 view_size);


#endif  /*  __PICMAN_COMPONENT_EDITOR_H__  */
