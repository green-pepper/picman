/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmandrawablestack.h
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DRAWABLE_STACK_H__
#define __PICMAN_DRAWABLE_STACK_H__

#include "picmanitemstack.h"


#define PICMAN_TYPE_DRAWABLE_STACK            (picman_drawable_stack_get_type ())
#define PICMAN_DRAWABLE_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DRAWABLE_STACK, PicmanDrawableStack))
#define PICMAN_DRAWABLE_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DRAWABLE_STACK, PicmanDrawableStackClass))
#define PICMAN_IS_DRAWABLE_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DRAWABLE_STACK))
#define PICMAN_IS_DRAWABLE_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DRAWABLE_STACK))


typedef struct _PicmanDrawableStackClass PicmanDrawableStackClass;

struct _PicmanDrawableStack
{
  PicmanItemStack  parent_instance;
};

struct _PicmanDrawableStackClass
{
  PicmanItemStackClass  parent_class;

  void (* update) (PicmanDrawableStack *stack,
                   gint               x,
                   gint               y,
                   gint               width,
                   gint               height);
};


GType           picman_drawable_stack_get_type  (void) G_GNUC_CONST;
PicmanContainer * picman_drawable_stack_new       (GType drawable_type);


#endif  /*  __PICMAN_DRAWABLE_STACK_H__  */
