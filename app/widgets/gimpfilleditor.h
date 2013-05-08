/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfilleditor.h
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_FILL_EDITOR_H__
#define __PICMAN_FILL_EDITOR_H__


#define PICMAN_TYPE_FILL_EDITOR            (picman_fill_editor_get_type ())
#define PICMAN_FILL_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILL_EDITOR, PicmanFillEditor))
#define PICMAN_FILL_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILL_EDITOR, PicmanFillEditorClass))
#define PICMAN_IS_FILL_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILL_EDITOR))
#define PICMAN_IS_FILL_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILL_EDITOR))
#define PICMAN_FILL_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILL_EDITOR, PicmanFillEditorClass))


typedef struct _PicmanFillEditorClass PicmanFillEditorClass;

struct _PicmanFillEditor
{
  GtkBox           parent_instance;

  PicmanFillOptions *options;
  gboolean         edit_context;
};

struct _PicmanFillEditorClass
{
  GtkBoxClass      parent_class;
};


GType       picman_fill_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_fill_editor_new      (PicmanFillOptions *options,
                                       gboolean         edit_context);


#endif /* __PICMAN_FILL_EDITOR_H__ */
