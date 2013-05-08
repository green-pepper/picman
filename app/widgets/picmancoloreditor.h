/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancoloreditor.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_COLOR_EDITOR_H__
#define __PICMAN_COLOR_EDITOR_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_COLOR_EDITOR            (picman_color_editor_get_type ())
#define PICMAN_COLOR_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_EDITOR, PicmanColorEditor))
#define PICMAN_COLOR_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_EDITOR, PicmanColorEditorClass))
#define PICMAN_IS_COLOR_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_EDITOR))
#define PICMAN_IS_COLOR_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_EDITOR))
#define PICMAN_COLOR_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_EDITOR, PicmanColorEditorClass))


typedef struct _PicmanColorEditorClass PicmanColorEditorClass;

struct _PicmanColorEditor
{
  PicmanEditor   parent_instance;

  PicmanContext *context;
  gboolean     edit_bg;

  GtkWidget   *hbox;
  GtkWidget   *notebook;
  GtkWidget   *fg_bg;
  GtkWidget   *hex_entry;
};

struct _PicmanColorEditorClass
{
  PicmanEditorClass  parent_class;
};


GType       picman_color_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_color_editor_new      (PicmanContext *context);


#endif /* __PICMAN_COLOR_EDITOR_H__ */
