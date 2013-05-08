/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantemplateview.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEMPLATE_VIEW_H__
#define __PICMAN_TEMPLATE_VIEW_H__


#include "picmancontainereditor.h"


#define PICMAN_TYPE_TEMPLATE_VIEW            (picman_template_view_get_type ())
#define PICMAN_TEMPLATE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEMPLATE_VIEW, PicmanTemplateView))
#define PICMAN_TEMPLATE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEMPLATE_VIEW, PicmanTemplateViewClass))
#define PICMAN_IS_TEMPLATE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEMPLATE_VIEW))
#define PICMAN_IS_TEMPLATE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEMPLATE_VIEW))
#define PICMAN_TEMPLATE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEMPLATE_VIEW, PicmanTemplateViewClass))


typedef struct _PicmanTemplateViewClass  PicmanTemplateViewClass;

struct _PicmanTemplateView
{
  PicmanContainerEditor  parent_instance;

  GtkWidget           *create_button;
  GtkWidget           *new_button;
  GtkWidget           *duplicate_button;
  GtkWidget           *edit_button;
  GtkWidget           *delete_button;
};

struct _PicmanTemplateViewClass
{
  PicmanContainerEditorClass  parent_class;
};


GType       picman_template_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_template_view_new      (PicmanViewType     view_type,
                                         PicmanContainer   *container,
                                         PicmanContext     *context,
                                         gint             view_size,
                                         gint             view_border_width,
                                         PicmanMenuFactory *menu_factory);


#endif  /*  __PICMAN_TEMPLATE_VIEW_H__  */
