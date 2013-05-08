/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainergridview.h
 * Copyright (C) 2001-2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTAINER_GRID_VIEW_H__
#define __PICMAN_CONTAINER_GRID_VIEW_H__


#include "picmancontainerbox.h"


#define PICMAN_TYPE_CONTAINER_GRID_VIEW            (picman_container_grid_view_get_type ())
#define PICMAN_CONTAINER_GRID_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_GRID_VIEW, PicmanContainerGridView))
#define PICMAN_CONTAINER_GRID_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_GRID_VIEW, PicmanContainerGridViewClass))
#define PICMAN_IS_CONTAINER_GRID_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_GRID_VIEW))
#define PICMAN_IS_CONTAINER_GRID_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_GRID_VIEW))
#define PICMAN_CONTAINER_GRID_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_GRID_VIEW, PicmanContainerGridViewClass))


typedef struct _PicmanContainerGridViewClass PicmanContainerGridViewClass;

struct _PicmanContainerGridView
{
  PicmanContainerBox  parent_instance;

  GtkWidget        *wrap_box;

  gint              rows;
  gint              columns;
  gint              visible_rows;

  PicmanView         *selected_item;
};

struct _PicmanContainerGridViewClass
{
  PicmanContainerBoxClass  parent_class;

  gboolean (* move_cursor) (PicmanContainerGridView *grid_view,
                            GtkMovementStep        step,
                            gint                   count);
};


GType       picman_container_grid_view_get_type (void) G_GNUC_CONST;

GtkWidget * picman_container_grid_view_new      (PicmanContainer *container,
                                               PicmanContext   *context,
                                               gint           view_size,
                                               gint           view_border_width);


#endif  /*  __PICMAN_CONTAINER_GRID_VIEW_H__  */
