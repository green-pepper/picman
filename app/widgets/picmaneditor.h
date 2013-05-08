/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaneditor.h
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

#ifndef __PICMAN_EDITOR_H__
#define __PICMAN_EDITOR_H__


#define PICMAN_TYPE_EDITOR            (picman_editor_get_type ())
#define PICMAN_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_EDITOR, PicmanEditor))
#define PICMAN_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_EDITOR, PicmanEditorClass))
#define PICMAN_IS_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_EDITOR))
#define PICMAN_IS_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_EDITOR))
#define PICMAN_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_EDITOR, PicmanEditorClass))


typedef struct _PicmanEditorClass    PicmanEditorClass;
typedef struct _PicmanEditorPrivate  PicmanEditorPrivate;

struct _PicmanEditor
{
  GtkBox            parent_instance;

  PicmanEditorPrivate *priv;
};

struct _PicmanEditorClass
{
  GtkBoxClass  parent_class;
};


GType       picman_editor_get_type          (void) G_GNUC_CONST;

GtkWidget * picman_editor_new               (void);

void        picman_editor_create_menu       (PicmanEditor           *editor,
                                           PicmanMenuFactory      *menu_factory,
                                           const gchar          *menu_identifier,
                                           const gchar          *ui_path,
                                           gpointer              popup_data);
gboolean    picman_editor_popup_menu        (PicmanEditor           *editor,
                                           PicmanMenuPositionFunc  position_func,
                                           gpointer              position_data);

GtkWidget * picman_editor_add_button        (PicmanEditor           *editor,
                                           const gchar          *stock_id,
                                           const gchar          *tooltip,
                                           const gchar          *help_id,
                                           GCallback             callback,
                                           GCallback             extended_callback,
                                           gpointer              callback_data);
GtkWidget * picman_editor_add_stock_box     (PicmanEditor           *editor,
                                           GType                 enum_type,
                                           const gchar          *stock_prefix,
                                           GCallback             callback,
                                           gpointer              callback_data);

GtkWidget * picman_editor_add_action_button (PicmanEditor           *editor,
                                           const gchar          *group_name,
                                           const gchar          *action_name,
                                           ...) G_GNUC_NULL_TERMINATED;

void        picman_editor_set_show_name       (PicmanEditor         *editor,
                                             gboolean            show);
void        picman_editor_set_name            (PicmanEditor         *editor,
                                             const gchar        *name);

void        picman_editor_set_box_style       (PicmanEditor         *editor,
                                             GtkBox             *box);
PicmanUIManager *
            picman_editor_get_ui_manager      (PicmanEditor         *editor);
GtkBox    * picman_editor_get_button_box      (PicmanEditor         *editor);
PicmanMenuFactory *
            picman_editor_get_menu_factory    (PicmanEditor         *editor);
gpointer *  picman_editor_get_popup_data      (PicmanEditor         *editor);
gchar *     picman_editor_get_ui_path         (PicmanEditor         *editor);

#endif  /*  __PICMAN_EDITOR_H__  */
