/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantooloptionseditor.h
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

#ifndef __PICMAN_TOOL_OPTIONS_EDITOR_H__
#define __PICMAN_TOOL_OPTIONS_EDITOR_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_TOOL_OPTIONS_EDITOR            (picman_tool_options_editor_get_type ())
#define PICMAN_TOOL_OPTIONS_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TOOL_OPTIONS_EDITOR, PicmanToolOptionsEditor))
#define PICMAN_TOOL_OPTIONS_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TOOL_OPTIONS_EDITOR, PicmanToolOptionsEditorClass))
#define PICMAN_IS_TOOL_OPTIONS_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TOOL_OPTIONS_EDITOR))
#define PICMAN_IS_TOOL_OPTIONS_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TOOL_OPTIONS_EDITOR))
#define PICMAN_TOOL_OPTIONS_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TOOL_OPTIONS_EDITOR, PicmanToolOptionsEditorClass))


typedef struct _PicmanToolOptionsEditorPrivate  PicmanToolOptionsEditorPrivate;
typedef struct _PicmanToolOptionsEditorClass    PicmanToolOptionsEditorClass;

struct _PicmanToolOptionsEditor
{
  PicmanEditor                    parent_instance;

  PicmanToolOptionsEditorPrivate *p;
};

struct _PicmanToolOptionsEditorClass
{
  PicmanEditorClass  parent_class;
};


GType             picman_tool_options_editor_get_type         (void) G_GNUC_CONST;
GtkWidget       * picman_tool_options_editor_new              (Picman                  *picman,
                                                             PicmanMenuFactory       *menu_factory);
PicmanToolOptions * picman_tool_options_editor_get_tool_options (PicmanToolOptionsEditor *editor);


#endif  /*  __PICMAN_TOOL_OPTIONS_EDITOR_H__  */
