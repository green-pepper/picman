/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextStyleEditor
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEXT_STYLE_EDITOR_H__
#define __PICMAN_TEXT_STYLE_EDITOR_H__


#define PICMAN_TYPE_TEXT_STYLE_EDITOR            (picman_text_style_editor_get_type ())
#define PICMAN_TEXT_STYLE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_STYLE_EDITOR, PicmanTextStyleEditor))
#define PICMAN_TEXT_STYLE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_STYLE_EDITOR, PicmanTextStyleEditorClass))
#define PICMAN_IS_TEXT_STYLE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_STYLE_EDITOR))
#define PICMAN_IS_TEXT_STYLE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_STYLE_EDITOR))
#define PICMAN_TEXT_STYLE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEXT_STYLE_EDITOR, PicmanTextStyleEditorClass))


typedef struct _PicmanTextStyleEditorClass PicmanTextStyleEditorClass;

struct _PicmanTextStyleEditor
{
  GtkBox          parent_instance;

  Picman           *picman;
  PicmanContext    *context;

  PicmanText       *text; /* read-only for default values */
  PicmanTextBuffer *buffer;

  PicmanContainer  *fonts;
  gdouble         resolution_x;
  gdouble         resolution_y;

  GtkWidget      *upper_hbox;
  GtkWidget      *lower_hbox;

  GtkWidget      *font_entry;
  GtkWidget      *size_entry;

  GtkWidget      *color_button;

  GtkWidget      *clear_button;

  GtkWidget      *baseline_spinbutton;
  GtkAdjustment  *baseline_adjustment;

  GtkWidget      *kerning_spinbutton;
  GtkAdjustment  *kerning_adjustment;

  GList          *toggles;

  guint           update_idle_id;
};

struct _PicmanTextStyleEditorClass
{
  GtkBoxClass  parent_class;
};


GType       picman_text_style_editor_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_text_style_editor_new       (Picman                 *picman,
                                              PicmanText             *text,
                                              PicmanTextBuffer       *buffer,
                                              PicmanContainer        *fonts,
                                              gdouble               resolution_x,
                                              gdouble               resolution_y);

GList     * picman_text_style_editor_list_tags (PicmanTextStyleEditor  *editor,
                                              GList               **remove_tags);


#endif /*  __PICMAN_TEXT_STYLE_EDITOR_H__  */
