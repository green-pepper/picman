/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmanitemstack.h
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

#ifndef __PICMAN_ITEM_STACK_H__
#define __PICMAN_ITEM_STACK_H__

#include "picmanfilterstack.h"


#define PICMAN_TYPE_ITEM_STACK            (picman_item_stack_get_type ())
#define PICMAN_ITEM_STACK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ITEM_STACK, PicmanItemStack))
#define PICMAN_ITEM_STACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ITEM_STACK, PicmanItemStackClass))
#define PICMAN_IS_ITEM_STACK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ITEM_STACK))
#define PICMAN_IS_ITEM_STACK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ITEM_STACK))


typedef struct _PicmanItemStackClass PicmanItemStackClass;

struct _PicmanItemStack
{
  PicmanFilterStack  parent_instance;
};

struct _PicmanItemStackClass
{
  PicmanFilterStackClass  parent_class;
};


GType           picman_item_stack_get_type            (void) G_GNUC_CONST;
PicmanContainer * picman_item_stack_new                 (GType          item_type);

gint            picman_item_stack_get_n_items         (PicmanItemStack *stack);
gboolean        picman_item_stack_is_flat             (PicmanItemStack *stack);
GList         * picman_item_stack_get_item_iter       (PicmanItemStack *stack);
GList         * picman_item_stack_get_item_list       (PicmanItemStack *stack);
PicmanItem      * picman_item_stack_get_item_by_tattoo  (PicmanItemStack *stack,
                                                     PicmanTattoo     tattoo);
PicmanItem      * picman_item_stack_get_item_by_path    (PicmanItemStack *stack,
                                                     GList         *path);
PicmanItem      * picman_item_stack_get_parent_by_path  (PicmanItemStack *stack,
                                                     GList         *path,
                                                     gint          *index);

void            picman_item_stack_invalidate_previews (PicmanItemStack *stack);


#endif  /*  __PICMAN_ITEM_STACK_H__  */
