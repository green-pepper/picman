/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainericonview.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_ICON_VIEW_H__
#define __PICMAN_CONTAINER_ICON_VIEW_H__


#include "picmancontainerbox.h"


#define PICMAN_TYPE_CONTAINER_ICON_VIEW            (picman_container_icon_view_get_type ())
#define PICMAN_CONTAINER_ICON_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_ICON_VIEW, PicmanContainerIconView))
#define PICMAN_CONTAINER_ICON_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_ICON_VIEW, PicmanContainerIconViewClass))
#define PICMAN_IS_CONTAINER_ICON_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_ICON_VIEW))
#define PICMAN_IS_CONTAINER_ICON_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_ICON_VIEW))
#define PICMAN_CONTAINER_ICON_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_ICON_VIEW, PicmanContainerIconViewClass))


typedef struct _PicmanContainerIconViewClass PicmanContainerIconViewClass;
typedef struct _PicmanContainerIconViewPriv  PicmanContainerIconViewPriv;

struct _PicmanContainerIconView
{
  PicmanContainerBox           parent_instance;

  GtkTreeModel              *model;
  gint                       n_model_columns;
  GType                      model_columns[16];

  GtkIconView               *view;

  GtkCellRenderer           *renderer_cell;

  Picman                      *dnd_picman; /* eek */

  PicmanContainerIconViewPriv *priv;
};

struct _PicmanContainerIconViewClass
{
  PicmanContainerBoxClass  parent_class;
};


GType       picman_container_icon_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_container_icon_view_new      (PicmanContainer *container,
                                               PicmanContext   *context,
                                               gint           view_size,
                                               gint           view_border_width);


#endif  /*  __PICMAN_CONTAINER_ICON_VIEW_H__  */
