/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanlist.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_LIST_H__
#define __PICMAN_LIST_H__


#include "picmancontainer.h"


#define PICMAN_TYPE_LIST            (picman_list_get_type ())
#define PICMAN_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LIST, PicmanList))
#define PICMAN_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LIST, PicmanListClass))
#define PICMAN_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LIST))
#define PICMAN_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LIST))
#define PICMAN_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LIST, PicmanListClass))


typedef struct _PicmanListClass PicmanListClass;

struct _PicmanList
{
  PicmanContainer  parent_instance;

  GList         *list;
  gboolean       unique_names;
  GCompareFunc   sort_func;
  gboolean       append;
};

struct _PicmanListClass
{
  PicmanContainerClass  parent_class;
};


GType           picman_list_get_type      (void) G_GNUC_CONST;

PicmanContainer * picman_list_new           (GType         children_type,
                                         gboolean      unique_names);
PicmanContainer * picman_list_new_weak      (GType         children_type,
                                         gboolean      unique_names);

void            picman_list_reverse       (PicmanList     *list);
void            picman_list_set_sort_func (PicmanList     *list,
                                         GCompareFunc  sort_func);
GCompareFunc    picman_list_get_sort_func (PicmanList     *list);
void            picman_list_sort          (PicmanList     *list,
                                         GCompareFunc  sort_func);
void            picman_list_sort_by_name  (PicmanList     *list);


#endif  /* __PICMAN_LIST_H__ */
