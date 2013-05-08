/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimageview.h
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

#ifndef __PICMAN_IMAGE_VIEW_H__
#define __PICMAN_IMAGE_VIEW_H__


#include "picmancontainereditor.h"


#define PICMAN_TYPE_IMAGE_VIEW            (picman_image_view_get_type ())
#define PICMAN_IMAGE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_VIEW, PicmanImageView))
#define PICMAN_IMAGE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_VIEW, PicmanImageViewClass))
#define PICMAN_IS_IMAGE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_VIEW))
#define PICMAN_IS_IMAGE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_VIEW))
#define PICMAN_IMAGE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_VIEW, PicmanImageViewClass))


typedef struct _PicmanImageViewClass PicmanImageViewClass;

struct _PicmanImageView
{
  PicmanContainerEditor  parent_instance;

  GtkWidget           *raise_button;
  GtkWidget           *new_button;
  GtkWidget           *delete_button;
};

struct _PicmanImageViewClass
{
  PicmanContainerEditorClass  parent_class;
};


GType       picman_image_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_image_view_new      (PicmanViewType     view_type,
                                      PicmanContainer   *container,
                                      PicmanContext     *context,
                                      gint             view_size,
                                      gint             view_border_width,
                                      PicmanMenuFactory *menu_factory);


#endif  /*  __PICMAN_IMAGE_VIEW_H__  */
