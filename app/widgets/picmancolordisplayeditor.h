/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancolordisplayeditor.h
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

#ifndef __PICMAN_COLOR_DISPLAY_EDITOR_H__
#define __PICMAN_COLOR_DISPLAY_EDITOR_H__


#define PICMAN_TYPE_COLOR_DISPLAY_EDITOR            (picman_color_display_editor_get_type ())
#define PICMAN_COLOR_DISPLAY_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_DISPLAY_EDITOR, PicmanColorDisplayEditor))
#define PICMAN_COLOR_DISPLAY_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_DISPLAY_EDITOR, PicmanColorDisplayEditorClass))
#define PICMAN_IS_COLOR_DISPLAY_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_DISPLAY_EDITOR))
#define PICMAN_IS_COLOR_DISPLAY_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_DISPLAY_EDITOR))
#define PICMAN_COLOR_DISPLAY_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_DISPLAY_EDITOR, PicmanColorDisplayEditorClass))


typedef struct _PicmanColorDisplayEditorClass  PicmanColorDisplayEditorClass;

struct _PicmanColorDisplayEditor
{
  GtkBox                 parent_instance;

  PicmanColorDisplayStack *stack;
  PicmanColorConfig       *config;
  PicmanColorManaged      *managed;

  GtkListStore          *src;
  GtkListStore          *dest;

  GtkTreeSelection      *src_sel;
  GtkTreeSelection      *dest_sel;

  PicmanColorDisplay      *selected;

  GtkWidget             *add_button;

  GtkWidget             *remove_button;
  GtkWidget             *up_button;
  GtkWidget             *down_button;

  GtkWidget             *config_frame;
  GtkWidget             *config_box;
  GtkWidget             *config_widget;

  GtkWidget             *reset_button;
};

struct _PicmanColorDisplayEditorClass
{
  GtkBoxClass parent_class;
};


GType       picman_color_display_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_color_display_editor_new      (PicmanColorDisplayStack *stack,
                                                PicmanColorConfig       *config,
                                                PicmanColorManaged      *managed);


#endif  /*  __PICMAN_COLOR_DISPLAY_EDITOR_H__  */
