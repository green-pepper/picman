/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfgbgeditor.h
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

#ifndef __PICMAN_FG_BG_EDITOR_H__
#define __PICMAN_FG_BG_EDITOR_H__


#define PICMAN_TYPE_FG_BG_EDITOR            (picman_fg_bg_editor_get_type ())
#define PICMAN_FG_BG_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FG_BG_EDITOR, PicmanFgBgEditor))
#define PICMAN_FG_BG_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FG_BG_EDITOR, PicmanFgBgEditorClass))
#define PICMAN_IS_FG_BG_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FG_BG_EDITOR))
#define PICMAN_IS_FG_BG_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FG_BG_EDITOR))
#define PICMAN_FG_BG_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FG_BG_EDITOR, PicmanFgBgEditorClass))


typedef struct _PicmanFgBgEditorClass PicmanFgBgEditorClass;

struct _PicmanFgBgEditor
{
  GtkDrawingArea   parent_instance;

  PicmanContext     *context;
  PicmanActiveColor  active_color;

  GdkPixbuf       *default_icon;
  GdkPixbuf       *swap_icon;

  gint             rect_width;
  gint             rect_height;
  gint             click_target;
};

struct _PicmanFgBgEditorClass
{
  GtkDrawingAreaClass  parent_class;

  /*  signals  */

  void (* color_clicked) (PicmanFgBgEditor  *editor,
                          PicmanActiveColor  color);
};


GType       picman_fg_bg_editor_get_type    (void) G_GNUC_CONST;

GtkWidget * picman_fg_bg_editor_new         (PicmanContext     *context);

void        picman_fg_bg_editor_set_context (PicmanFgBgEditor  *editor,
                                           PicmanContext     *context);
void        picman_fg_bg_editor_set_active  (PicmanFgBgEditor  *editor,
                                           PicmanActiveColor  active);


#endif  /*  __PICMAN_FG_BG_EDITOR_H__  */
