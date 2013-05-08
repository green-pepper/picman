/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanviewablebutton.h
 * Copyright (C) 2003-2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_VIEWABLE_BUTTON_H__
#define __PICMAN_VIEWABLE_BUTTON_H__


#define PICMAN_TYPE_VIEWABLE_BUTTON            (picman_viewable_button_get_type ())
#define PICMAN_VIEWABLE_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VIEWABLE_BUTTON, PicmanViewableButton))
#define PICMAN_VIEWABLE_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_VIEWABLE_BUTTON, PicmanViewableButtonClass))
#define PICMAN_IS_VIEWABLE_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VIEWABLE_BUTTON))
#define PICMAN_IS_VIEWABLE_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_VIEWABLE_BUTTON))
#define PICMAN_VIEWABLE_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_VIEWABLE_BUTTON, PicmanViewableButtonClass))


typedef struct _PicmanViewableButtonClass PicmanViewableButtonClass;

struct _PicmanViewableButton
{
  PicmanButton         parent_instance;

  PicmanContainer     *container;
  PicmanContext       *context;

  PicmanViewType       popup_view_type;
  gint               popup_view_size;

  gint               button_view_size;
  gint               view_border_width;

  PicmanDialogFactory *dialog_factory;
  gchar             *dialog_identifier;
  gchar             *dialog_stock_id;
  gchar             *dialog_tooltip;

  GtkWidget         *view;
};

struct _PicmanViewableButtonClass
{
  PicmanButtonClass  parent_class;
};


GType       picman_viewable_button_get_type (void) G_GNUC_CONST;

GtkWidget * picman_viewable_button_new      (PicmanContainer      *container,
                                           PicmanContext        *context,
                                           PicmanViewType        view_type,
                                           gint                button_view_size,
                                           gint                view_size,
                                           gint                view_border_width,
                                           PicmanDialogFactory  *dialog_factory,
                                           const gchar        *dialog_identifier,
                                           const gchar        *dialog_stock_id,
                                           const gchar        *dialog_tooltip);

PicmanViewType picman_viewable_button_get_view_type (PicmanViewableButton *button);
void         picman_viewable_button_set_view_type (PicmanViewableButton *button,
                                                 PicmanViewType        view_type);

gint         picman_viewable_button_get_view_size (PicmanViewableButton *button);
void         picman_viewable_button_set_view_size (PicmanViewableButton *button,
                                                 gint                view_size);


#endif /* __PICMAN_VIEWABLE_BUTTON_H__ */
