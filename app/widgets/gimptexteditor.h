/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextEditor
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_EDITOR_H__
#define __PICMAN_TEXT_EDITOR_H__


#define PICMAN_TYPE_TEXT_EDITOR    (picman_text_editor_get_type ())
#define PICMAN_TEXT_EDITOR(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_EDITOR, PicmanTextEditor))
#define PICMAN_IS_TEXT_EDITOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_EDITOR))


typedef struct _PicmanTextEditorClass  PicmanTextEditorClass;

struct _PicmanTextEditor
{
  PicmanDialog         parent_instance;

  /*<  private  >*/
  PicmanTextDirection  base_dir;
  gchar             *font_name;

  GtkWidget         *view;
  GtkWidget         *font_toggle;
  GtkWidget         *file_dialog;
  PicmanUIManager     *ui_manager;
};

struct _PicmanTextEditorClass
{
  PicmanDialogClass   parent_class;

  void (* text_changed) (PicmanTextEditor *editor);
  void (* dir_changed)  (PicmanTextEditor *editor);
};


GType               picman_text_editor_get_type      (void) G_GNUC_CONST;
GtkWidget         * picman_text_editor_new           (const gchar       *title,
                                                    GtkWindow         *parent,
                                                    Picman              *picman,
                                                    PicmanMenuFactory   *menu_factory,
                                                    PicmanText          *text,
                                                    PicmanTextBuffer    *text_buffer,
                                                    gdouble            xres,
                                                    gdouble            yres);

void                picman_text_editor_set_text      (PicmanTextEditor    *editor,
                                                    const gchar       *text,
                                                    gint               len);
gchar             * picman_text_editor_get_text      (PicmanTextEditor    *editor);

void                picman_text_editor_set_direction (PicmanTextEditor    *editor,
                                                    PicmanTextDirection  base_dir);
PicmanTextDirection   picman_text_editor_get_direction (PicmanTextEditor    *editor);

void                picman_text_editor_set_font_name (PicmanTextEditor    *editor,
                                                    const gchar       *font_name);
const gchar       * picman_text_editor_get_font_name (PicmanTextEditor    *editor);


#endif  /* __PICMAN_TEXT_EDITOR_H__ */
