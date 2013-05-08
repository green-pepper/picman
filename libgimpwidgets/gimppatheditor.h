/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanpatheditor.h
 * Copyright (C) 1999-2004 Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_WIDGETS_H_INSIDE__) && !defined (PICMAN_WIDGETS_COMPILATION)
#error "Only <libpicmanwidgets/picmanwidgets.h> can be included directly."
#endif

#ifndef __PICMAN_PATH_EDITOR_H__
#define __PICMAN_PATH_EDITOR_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_PATH_EDITOR            (picman_path_editor_get_type ())
#define PICMAN_PATH_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PATH_EDITOR, PicmanPathEditor))
#define PICMAN_PATH_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PATH_EDITOR, PicmanPathEditorClass))
#define PICMAN_IS_PATH_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, PICMAN_TYPE_PATH_EDITOR))
#define PICMAN_IS_PATH_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PATH_EDITOR))
#define PICMAN_PATH_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PATH_EDITOR, PicmanPathEditorClass))


typedef struct _PicmanPathEditorClass  PicmanPathEditorClass;

struct _PicmanPathEditor
{
  GtkBox             parent_instance;

  GtkWidget         *upper_hbox;

  GtkWidget         *new_button;
  GtkWidget         *up_button;
  GtkWidget         *down_button;
  GtkWidget         *delete_button;

  GtkWidget         *file_entry;

  GtkListStore      *dir_list;

  GtkTreeSelection  *sel;
  GtkTreePath       *sel_path;

  GtkTreeViewColumn *writable_column;

  gint               num_items;
};

struct _PicmanPathEditorClass
{
  GtkBoxClass  parent_class;

  void (* path_changed)     (PicmanPathEditor *editor);
  void (* writable_changed) (PicmanPathEditor *editor);

  /* Padding for future expansion */
  void (* _picman_reserved1) (void);
  void (* _picman_reserved2) (void);
  void (* _picman_reserved3) (void);
  void (* _picman_reserved4) (void);
};


/* For information look into the C source or the html documentation */

GType       picman_path_editor_get_type          (void) G_GNUC_CONST;

GtkWidget * picman_path_editor_new               (const gchar    *title,
                                                const gchar    *path);

gchar     * picman_path_editor_get_path          (PicmanPathEditor *editor);
void        picman_path_editor_set_path          (PicmanPathEditor *editor,
                                                const gchar    *path);

gchar     * picman_path_editor_get_writable_path (PicmanPathEditor *editor);
void        picman_path_editor_set_writable_path (PicmanPathEditor *editor,
                                                const gchar    *path);

gboolean    picman_path_editor_get_dir_writable  (PicmanPathEditor *editor,
                                                const gchar    *directory);
void        picman_path_editor_set_dir_writable  (PicmanPathEditor *editor,
                                                const gchar    *directory,
                                                gboolean        writable);

G_END_DECLS

#endif /* __PICMAN_PATH_EDITOR_H__ */
