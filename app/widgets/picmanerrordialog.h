/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanerrordialog.c
 * Copyright (C) 2004  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_ERROR_DIALOG_H__
#define __PICMAN_ERROR_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_ERROR_DIALOG            (picman_error_dialog_get_type ())
#define PICMAN_ERROR_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ERROR_DIALOG, PicmanErrorDialog))
#define PICMAN_ERROR_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ERROR_DIALOG, PicmanErrorDialogClass))
#define PICMAN_IS_ERROR_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ERROR_DIALOG))
#define PICMAN_IS_ERROR_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ERROR_DIALOG))
#define PICMAN_ERROR_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ERROR_DIALOG, PicmanErrorDialogClass))


typedef struct _PicmanErrorDialogClass  PicmanErrorDialogClass;

struct _PicmanErrorDialog
{
  PicmanDialog       parent_instance;

  GtkWidget       *vbox;

  GtkWidget       *last_box;
  gchar           *last_domain;
  gchar           *last_message;
  gint             num_messages;
};

struct _PicmanErrorDialogClass
{
  PicmanDialogClass  parent_class;
};


GType       picman_error_dialog_get_type (void) G_GNUC_CONST;

GtkWidget * picman_error_dialog_new      (const gchar     *title);
void        picman_error_dialog_add      (PicmanErrorDialog *dialog,
                                        const gchar     *stock_id,
                                        const gchar     *domain,
                                        const gchar     *message);



G_END_DECLS

#endif /* __PICMAN_ERROR_DIALOG_H__ */
