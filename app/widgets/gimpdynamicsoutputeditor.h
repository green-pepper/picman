/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandynamicsoutputeditor.h
 * Copyright (C) 2010 Alexia Death
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

#ifndef __PICMAN_DYNAMICS_OUTPUT_EDITOR_H__
#define __PICMAN_DYNAMICS_OUTPUT_EDITOR_H__


#define PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR            (picman_dynamics_output_editor_get_type ())
#define PICMAN_DYNAMICS_OUTPUT_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR, PicmanDynamicsOutputEditor))
#define PICMAN_DYNAMICS_OUTPUT_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR, PicmanDynamicsOutputEditorClass))
#define PICMAN_IS_DYNAMICS_OUTPUT_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR))
#define PICMAN_IS_DYNAMICS_OUTPUT_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR))
#define PICMAN_DYNAMICS_OUTPUT_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT_EDITOR, PicmanDynamicsOutputEditorClass))


typedef struct _PicmanDynamicsOutputEditorClass PicmanDynamicsOutputEditorClass;

struct _PicmanDynamicsOutputEditor
{
  GtkBox  parent_instance;
};

struct _PicmanDynamicsOutputEditorClass
{
  GtkBoxClass  parent_class;
};


GType       picman_dynamics_output_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_dynamics_output_editor_new      (PicmanDynamicsOutput *output);


#endif /* __PICMAN_DYNAMICS_OUTPUT_EDITOR_H__ */
