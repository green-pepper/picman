/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontrollereditor.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTROLLER_EDITOR_H__
#define __PICMAN_CONTROLLER_EDITOR_H__


#define PICMAN_TYPE_CONTROLLER_EDITOR            (picman_controller_editor_get_type ())
#define PICMAN_CONTROLLER_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTROLLER_EDITOR, PicmanControllerEditor))
#define PICMAN_CONTROLLER_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTROLLER_EDITOR, PicmanControllerEditorClass))
#define PICMAN_IS_CONTROLLER_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTROLLER_EDITOR))
#define PICMAN_IS_CONTROLLER_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTROLLER_EDITOR))
#define PICMAN_CONTROLLER_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTROLLER_EDITOR, PicmanControllerEditorClass))


typedef struct _PicmanControllerEditorClass PicmanControllerEditorClass;

struct _PicmanControllerEditor
{
  GtkBox              parent_instance;

  PicmanControllerInfo *info;
  PicmanContext        *context;

  GtkTreeSelection   *sel;

  GtkWidget          *grab_button;
  GtkWidget          *edit_button;
  GtkWidget          *delete_button;

  GtkWidget          *edit_dialog;
  GtkTreeSelection   *edit_sel;
};

struct _PicmanControllerEditorClass
{
  GtkBoxClass   parent_class;
};


GType       picman_controller_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_controller_editor_new      (PicmanControllerInfo *info,
                                             PicmanContext        *context);


#endif  /*  __PICMAN_CONTROLLER_EDITOR_H__  */
