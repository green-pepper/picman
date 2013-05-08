/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_UNDO_EDITOR_H__
#define __PICMAN_UNDO_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_UNDO_EDITOR            (picman_undo_editor_get_type ())
#define PICMAN_UNDO_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNDO_EDITOR, PicmanUndoEditor))
#define PICMAN_UNDO_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNDO_EDITOR, PicmanUndoEditorClass))
#define PICMAN_IS_UNDO_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UNDO_EDITOR))
#define PICMAN_IS_UNDO_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNDO_EDITOR))
#define PICMAN_UNDO_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNDO_EDITOR, PicmanUndoEditorClass))


typedef struct _PicmanUndoEditorClass PicmanUndoEditorClass;

struct _PicmanUndoEditor
{
  PicmanImageEditor  parent_instance;

  PicmanContext     *context;
  PicmanContainer   *container;
  GtkWidget       *view;
  PicmanViewSize     view_size;

  PicmanUndo        *base_item;

  GtkWidget       *undo_button;
  GtkWidget       *redo_button;
  GtkWidget       *clear_button;
};

struct _PicmanUndoEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_undo_editor_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_undo_editor_new       (PicmanCoreConfig  *config,
                                        PicmanMenuFactory *menu_factory);


#endif /* __PICMAN_UNDO_EDITOR_H__ */
