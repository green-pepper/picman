/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfiledialog.h
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

#ifndef __PICMAN_FILE_DIALOG_H__
#define __PICMAN_FILE_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_FILE_DIALOG            (picman_file_dialog_get_type ())
#define PICMAN_FILE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILE_DIALOG, PicmanFileDialog))
#define PICMAN_FILE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILE_DIALOG, PicmanFileDialogClass))
#define PICMAN_IS_FILE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILE_DIALOG))
#define PICMAN_IS_FILE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILE_DIALOG))
#define PICMAN_FILE_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILE_DIALOG, PicmanFileDialogClass))


typedef struct _PicmanFileDialogClass  PicmanFileDialogClass;

struct _PicmanFileDialog
{
  GtkFileChooserDialog  parent_instance;

  PicmanPlugInProcedure  *file_proc;

  PicmanImage            *image;
  gboolean              open_as_layers;
  gboolean              save_a_copy;
  gboolean              export;
  gboolean              close_after_saving;
  PicmanObject           *display_to_close;

  GtkWidget            *thumb_box;
  GtkWidget            *proc_expander;
  GtkWidget            *proc_view;
  GtkWidget            *progress;

  gboolean              busy;
  gboolean              canceled;
};

struct _PicmanFileDialogClass
{
  GtkFileChooserDialogClass  parent_class;
};


typedef struct _PicmanFileDialogState PicmanFileDialogState;


GType       picman_file_dialog_get_type       (void) G_GNUC_CONST;

GtkWidget * picman_file_dialog_new            (Picman                 *picman,
                                             PicmanFileChooserAction action,
                                             const gchar          *title,
                                             const gchar          *role,
                                             const gchar          *stock_id,
                                             const gchar          *help_id);

void        picman_file_dialog_set_sensitive  (PicmanFileDialog       *dialog,
                                             gboolean              sensitive);

void        picman_file_dialog_set_file_proc  (PicmanFileDialog       *dialog,
                                             PicmanPlugInProcedure  *file_proc);

void        picman_file_dialog_set_open_image (PicmanFileDialog       *dialog,
                                             PicmanImage            *image,
                                             gboolean              open_as_layers);
void        picman_file_dialog_set_save_image (PicmanFileDialog       *dialog,
                                             Picman                 *picman,
                                             PicmanImage            *image,
                                             gboolean              save_a_copy,
                                             gboolean              export,
                                             gboolean              close_after_saving,
                                             PicmanObject           *display);

PicmanFileDialogState * picman_file_dialog_get_state     (PicmanFileDialog      *dialog);
void                  picman_file_dialog_set_state     (PicmanFileDialog      *dialog,
                                                      PicmanFileDialogState *state);
void                  picman_file_dialog_state_destroy (PicmanFileDialogState *state);


G_END_DECLS

#endif /* __PICMAN_FILE_DIALOG_H__ */
