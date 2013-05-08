/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerbox.h
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

#ifndef __PICMAN_CONTAINER_BOX_H__
#define __PICMAN_CONTAINER_BOX_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_CONTAINER_BOX            (picman_container_box_get_type ())
#define PICMAN_CONTAINER_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTAINER_BOX, PicmanContainerBox))
#define PICMAN_CONTAINER_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTAINER_BOX, PicmanContainerBoxClass))
#define PICMAN_IS_CONTAINER_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTAINER_BOX))
#define PICMAN_IS_CONTAINER_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTAINER_BOX))
#define PICMAN_CONTAINER_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTAINER_BOX, PicmanContainerBoxClass))


typedef struct _PicmanContainerBoxClass  PicmanContainerBoxClass;

struct _PicmanContainerBox
{
  PicmanEditor  parent_instance;

  GtkWidget  *scrolled_win;
};

struct _PicmanContainerBoxClass
{
  PicmanEditorClass  parent_class;
};


GType     picman_container_box_get_type         (void) G_GNUC_CONST;

void      picman_container_box_set_size_request (PicmanContainerBox *box,
                                               gint              width,
                                               gint              height);


#endif  /*  __PICMAN_CONTAINER_BOX_H__  */
