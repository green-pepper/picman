/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmessagebox.h
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

#ifndef __PICMAN_MESSAGE_BOX_H__
#define __PICMAN_MESSAGE_BOX_H__

G_BEGIN_DECLS


#define PICMAN_TYPE_MESSAGE_BOX            (picman_message_box_get_type ())
#define PICMAN_MESSAGE_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MESSAGE_BOX, PicmanMessageBox))
#define PICMAN_MESSAGE_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MESSAGE_BOX, PicmanMessageBoxClass))
#define PICMAN_IS_MESSAGE_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MESSAGE_BOX))
#define PICMAN_IS_MESSAGE_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MESSAGE_BOX))
#define PICMAN_MESSAGE_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MESSAGE_BOX, PicmanMessageBoxClass))


typedef struct _PicmanMessageBoxClass  PicmanMessageBoxClass;

struct _PicmanMessageBox
{
  GtkBox     parent_instance;

  gchar     *stock_id;
  gint       repeat;
  GtkWidget *label[3];
  GtkWidget *image;
};

struct _PicmanMessageBoxClass
{
  GtkBoxClass  parent_class;
};


GType       picman_message_box_get_type         (void) G_GNUC_CONST;

GtkWidget * picman_message_box_new              (const gchar    *stock_id);
void        picman_message_box_set_primary_text (PicmanMessageBox *box,
                                               const gchar    *format,
                                               ...) G_GNUC_PRINTF (2, 3);
void        picman_message_box_set_text         (PicmanMessageBox *box,
                                               const gchar    *format,
                                               ...) G_GNUC_PRINTF (2, 3);
void        picman_message_box_set_markup       (PicmanMessageBox *box,
                                               const gchar    *format,
                                               ...) G_GNUC_PRINTF (2, 3);
gint        picman_message_box_repeat           (PicmanMessageBox *box);


G_END_DECLS

#endif /* __PICMAN_MESSAGE_BOX_H__ */
