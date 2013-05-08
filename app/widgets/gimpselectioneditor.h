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

#ifndef __PICMAN_SELECTION_EDITOR_H__
#define __PICMAN_SELECTION_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_SELECTION_EDITOR            (picman_selection_editor_get_type ())
#define PICMAN_SELECTION_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SELECTION_EDITOR, PicmanSelectionEditor))
#define PICMAN_SELECTION_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SELECTION_EDITOR, PicmanSelectionEditorClass))
#define PICMAN_IS_SELECTION_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SELECTION_EDITOR))
#define PICMAN_IS_SELECTION_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SELECTION_EDITOR))
#define PICMAN_SELECTION_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SELECTION_EDITOR, PicmanSelectionEditorClass))


typedef struct _PicmanSelectionEditorClass PicmanSelectionEditorClass;

struct _PicmanSelectionEditor
{
  PicmanImageEditor  parent_instance;

  GtkWidget       *view;

  GtkWidget       *all_button;
  GtkWidget       *none_button;
  GtkWidget       *invert_button;
  GtkWidget       *save_button;
  GtkWidget       *path_button;
  GtkWidget       *stroke_button;
};

struct _PicmanSelectionEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_selection_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_selection_editor_new      (PicmanMenuFactory *menu_factory);


#endif /* __PICMAN_SELECTION_EDITOR_H__ */
