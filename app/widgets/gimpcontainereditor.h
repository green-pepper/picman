/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainereditor.h
 * Copyright (C) 2001-2011 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_EDITOR_H__
#define __PICMAN_CONTAINER_EDITOR_H__


#define PICMAN_TYPE_CONTAINER_EDITOR            (picman_container_editor_get_type ())
#define PICMAN_CONTAINER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_EDITOR, PicmanContainerEditor))
#define PICMAN_CONTAINER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_EDITOR, PicmanContainerEditorClass))
#define PICMAN_IS_CONTAINER_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_EDITOR))
#define PICMAN_IS_CONTAINER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_EDITOR))
#define PICMAN_CONTAINER_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_EDITOR, PicmanContainerEditorClass))


typedef struct _PicmanContainerEditorPrivate PicmanContainerEditorPrivate;
typedef struct _PicmanContainerEditorClass   PicmanContainerEditorClass;

struct _PicmanContainerEditor
{
  GtkBox             parent_instance;

  PicmanContainerView *view;

  PicmanContainerEditorPrivate *priv;
};

struct _PicmanContainerEditorClass
{
  GtkBoxClass  parent_class;

  void (* select_item)   (PicmanContainerEditor *editor,
                          PicmanViewable        *object);
  void (* activate_item) (PicmanContainerEditor *editor,
                          PicmanViewable        *object);
  void (* context_item)  (PicmanContainerEditor *editor,
                          PicmanViewable        *object);
};


GType            picman_container_editor_get_type           (void) G_GNUC_CONST;

GtkSelectionMode picman_container_editor_get_selection_mode (PicmanContainerEditor *editor);
void             picman_container_editor_set_selection_mode (PicmanContainerEditor *editor,
                                                           GtkSelectionMode     mode);


#endif  /*  __PICMAN_CONTAINER_EDITOR_H__  */
