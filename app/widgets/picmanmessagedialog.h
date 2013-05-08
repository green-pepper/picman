/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmessagedialog.h
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_MESSAGE_DIALOG_H__
#define __PICMAN_MESSAGE_DIALOG_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_MESSAGE_DIALOG            (picman_message_dialog_get_type ())
#define PICMAN_MESSAGE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MESSAGE_DIALOG, PicmanMessageDialog))
#define PICMAN_MESSAGE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MESSAGE_DIALOG, PicmanMessageDialogClass))
#define PICMAN_IS_MESSAGE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MESSAGE_DIALOG))
#define PICMAN_IS_MESSAGE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MESSAGE_DIALOG))
#define PICMAN_MESSAGE_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MESSAGE_DIALOG, PicmanMessageDialogClass))


typedef struct _PicmanMessageDialogClass  PicmanMessageDialogClass;

struct _PicmanMessageDialog
{
  PicmanDialog       parent_instance;

  PicmanMessageBox  *box;
};

struct _PicmanMessageDialogClass
{
  PicmanDialogClass  parent_class;
};


GType       picman_message_dialog_get_type (void) G_GNUC_CONST;

GtkWidget * picman_message_dialog_new      (const gchar       *title,
                                          const gchar       *stock_id,
                                          GtkWidget         *parent,
                                          GtkDialogFlags     flags,
                                          PicmanHelpFunc       help_func,
                                          const gchar       *help_id,
                                          ...) G_GNUC_NULL_TERMINATED;


G_END_DECLS

#endif /* __PICMAN_MESSAGE_DIALOG_H__ */
