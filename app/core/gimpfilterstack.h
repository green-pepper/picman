/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanfilterstack.h
 * Copyright (C) 2008-2013 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_FILTER_STACK_H__
#define __PICMAN_FILTER_STACK_H__

#include "picmanlist.h"


#define PICMAN_TYPE_FILTER_STACK            (picman_filter_stack_get_type ())
#define PICMAN_FILTER_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FILTER_STACK, PicmanFilterStack))
#define PICMAN_FILTER_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FILTER_STACK, PicmanFilterStackClass))
#define PICMAN_IS_FILTER_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FILTER_STACK))
#define PICMAN_IS_FILTER_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FILTER_STACK))


typedef struct _PicmanFilterStackClass PicmanFilterStackClass;

struct _PicmanFilterStack
{
  PicmanList  parent_instance;

  GeglNode *graph;
};

struct _PicmanFilterStackClass
{
  PicmanListClass  parent_class;
};


GType           picman_filter_stack_get_type  (void) G_GNUC_CONST;
PicmanContainer * picman_filter_stack_new       (GType            filter_type);

GeglNode *      picman_filter_stack_get_graph (PicmanFilterStack *stack);


#endif  /*  __PICMAN_FILTER_STACK_H__  */
