/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanfilteredcontainer.h
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
 *               2011 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_FILTERED_CONTAINER_H__
#define __PICMAN_FILTERED_CONTAINER_H__


#include "picmanlist.h"


#define PICMAN_TYPE_FILTERED_CONTAINER            (picman_filtered_container_get_type ())
#define PICMAN_FILTERED_CONTAINER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILTERED_CONTAINER, PicmanFilteredContainer))
#define PICMAN_FILTERED_CONTAINER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILTERED_CONTAINER, PicmanFilteredContainerClass))
#define PICMAN_IS_FILTERED_CONTAINER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILTERED_CONTAINER))
#define PICMAN_IS_FILTERED_CONTAINER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), PICMAN_TYPE_FILTERED_CONTAINER))
#define PICMAN_FILTERED_CONTAINER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FILTERED_CONTAINER, PicmanFilteredContainerClass))


typedef struct _PicmanFilteredContainerClass PicmanFilteredContainerClass;

struct _PicmanFilteredContainer
{
  PicmanList              parent_instance;

  PicmanContainer        *src_container;
  PicmanObjectFilterFunc  filter_func;
  gpointer              filter_data;
};

struct _PicmanFilteredContainerClass
{
  PicmanContainerClass  parent_class;

  void (* src_add)    (PicmanFilteredContainer *filtered_container,
                       PicmanObject            *object);
  void (* src_remove) (PicmanFilteredContainer *filtered_container,
                       PicmanObject            *object);
  void (* src_freeze) (PicmanFilteredContainer *filtered_container);
  void (* src_thaw)   (PicmanFilteredContainer *filtered_container);
};


GType           picman_filtered_container_get_type (void) G_GNUC_CONST;

PicmanContainer * picman_filtered_container_new      (PicmanContainer        *src_container,
                                                  PicmanObjectFilterFunc  filter_func,
                                                  gpointer              filter_data);


#endif  /* __PICMAN_FILTERED_CONTAINER_H__ */
