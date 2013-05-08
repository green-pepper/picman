/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandasheditor.h
 * Copyright (C) 2003 Simon Budig  <simon@picman.org>
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

#ifndef __PICMAN_DASH_EDITOR_H__
#define __PICMAN_DASH_EDITOR_H__


#define PICMAN_TYPE_DASH_EDITOR            (picman_dash_editor_get_type ())
#define PICMAN_DASH_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DASH_EDITOR, PicmanDashEditor))
#define PICMAN_DASH_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DASH_EDITOR, PicmanDashEditorClass))
#define PICMAN_IS_DASH_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DASH_EDITOR))
#define PICMAN_IS_DASH_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DASH_EDITOR))
#define PICMAN_DASH_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DASH_EDITOR, PicmanDashEditorClass))


typedef struct _PicmanDashEditorClass  PicmanDashEditorClass;

struct _PicmanDashEditor
{
  GtkDrawingArea     parent_instance;

  PicmanStrokeOptions *stroke_options;
  gdouble            dash_length;

  /* GUI stuff */
  gint               n_segments;
  gboolean          *segments;

  /* coordinates of the first block main dash pattern */
  gint               x0;
  gint               y0;
  gint               block_width;
  gint               block_height;

  gboolean           edit_mode;
  gint               edit_button_x0;
};

struct _PicmanDashEditorClass
{
  GtkDrawingAreaClass  parent_class;
};


GType       picman_dash_editor_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_dash_editor_new         (PicmanStrokeOptions *stroke_options);

void        picman_dash_editor_shift_left  (PicmanDashEditor    *editor);
void        picman_dash_editor_shift_right (PicmanDashEditor    *editor);


#endif /* __PICMAN_DASH_EDITOR_H__ */
