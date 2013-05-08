/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmantaggedcontainer.h
 * Copyright (C) 2008 Aurimas Juška <aurisj@svn.gnome.org>
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

#ifndef __PICMAN_TAGGED_CONTAINER_H__
#define __PICMAN_TAGGED_CONTAINER_H__


#include "picmanfilteredcontainer.h"


#define PICMAN_TYPE_TAGGED_CONTAINER            (picman_tagged_container_get_type ())
#define PICMAN_TAGGED_CONTAINER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TAGGED_CONTAINER, PicmanTaggedContainer))
#define PICMAN_TAGGED_CONTAINER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TAGGED_CONTAINER, PicmanTaggedContainerClass))
#define PICMAN_IS_TAGGED_CONTAINER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TAGGED_CONTAINER))
#define PICMAN_IS_TAGGED_CONTAINER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), PICMAN_TYPE_TAGGED_CONTAINER))
#define PICMAN_TAGGED_CONTAINER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TAGGED_CONTAINER, PicmanTaggedContainerClass))


typedef struct _PicmanTaggedContainerClass PicmanTaggedContainerClass;

struct _PicmanTaggedContainer
{
  PicmanFilteredContainer  parent_instance;

  GList                 *filter;
  GHashTable            *tag_ref_counts;
  gint                   tag_count;
};

struct _PicmanTaggedContainerClass
{
  PicmanFilteredContainerClass  parent_class;

  void (* tag_count_changed) (PicmanTaggedContainer *container,
                              gint                 count);
};


GType           picman_tagged_container_get_type      (void) G_GNUC_CONST;

PicmanContainer * picman_tagged_container_new           (PicmanContainer       *src_container);

void            picman_tagged_container_set_filter    (PicmanTaggedContainer *tagged_container,
                                                     GList               *tags);
const GList   * picman_tagged_container_get_filter    (PicmanTaggedContainer *tagged_container);

gint            picman_tagged_container_get_tag_count (PicmanTaggedContainer *container);


#endif  /* __PICMAN_TAGGED_CONTAINER_H__ */
