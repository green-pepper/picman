/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerpopup.h
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

#ifndef __PICMAN_CONTAINER_POPUP_H__
#define __PICMAN_CONTAINER_POPUP_H__


#define PICMAN_TYPE_CONTAINER_POPUP            (picman_container_popup_get_type ())
#define PICMAN_CONTAINER_POPUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_POPUP, PicmanContainerPopup))
#define PICMAN_CONTAINER_POPUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_POPUP, PicmanContainerPopupClass))
#define PICMAN_IS_CONTAINER_POPUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_POPUP))
#define PICMAN_IS_CONTAINER_POPUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_POPUP))
#define PICMAN_CONTAINER_POPUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_POPUP, PicmanContainerPopupClass))


typedef struct _PicmanContainerPopupClass  PicmanContainerPopupClass;

struct _PicmanContainerPopup
{
  GtkWindow            parent_instance;

  PicmanContainer       *container;
  PicmanContext         *orig_context;
  PicmanContext         *context;

  PicmanViewType         view_type;
  gint                 default_view_size;
  gint                 view_size;
  gint                 view_border_width;

  GtkWidget           *frame;
  PicmanContainerEditor *editor;

  PicmanDialogFactory   *dialog_factory;
  gchar               *dialog_identifier;
  gchar               *dialog_stock_id;
  gchar               *dialog_tooltip;
};

struct _PicmanContainerPopupClass
{
  GtkWindowClass  parent_instance;

  void (* cancel)  (PicmanContainerPopup *popup);
  void (* confirm) (PicmanContainerPopup *popup);
};


GType       picman_container_popup_get_type (void) G_GNUC_CONST;

GtkWidget * picman_container_popup_new      (PicmanContainer      *container,
                                           PicmanContext        *context,
                                           PicmanViewType        view_type,
                                           gint                default_view_size,
                                           gint                view_size,
                                           gint                view_border_width,
                                           PicmanDialogFactory  *dialog_factory,
                                           const gchar        *dialog_identifier,
                                           const gchar        *dialog_stock_id,
                                           const gchar        *dialog_tooltip);
void        picman_container_popup_show     (PicmanContainerPopup *popup,
                                           GtkWidget          *widget);

PicmanViewType picman_container_popup_get_view_type (PicmanContainerPopup *popup);
void         picman_container_popup_set_view_type (PicmanContainerPopup *popup,
                                                 PicmanViewType        view_type);

gint         picman_container_popup_get_view_size (PicmanContainerPopup *popup);
void         picman_container_popup_set_view_size (PicmanContainerPopup *popup,
                                                 gint                view_size);


#endif  /*  __PICMAN_CONTAINER_POPUP_H__  */
