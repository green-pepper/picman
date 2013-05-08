/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationgrow.h
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_GROW_H__
#define __PICMAN_OPERATION_GROW_H__


#include <gegl-plugin.h>


#define PICMAN_TYPE_OPERATION_GROW            (picman_operation_grow_get_type ())
#define PICMAN_OPERATION_GROW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_GROW, PicmanOperationGrow))
#define PICMAN_OPERATION_GROW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_GROW, PicmanOperationGrowClass))
#define PICMAN_IS_OPERATION_GROW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_GROW))
#define PICMAN_IS_OPERATION_GROW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_GROW))
#define PICMAN_OPERATION_GROW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_GROW, PicmanOperationGrowClass))


typedef struct _PicmanOperationGrow      PicmanOperationGrow;
typedef struct _PicmanOperationGrowClass PicmanOperationGrowClass;

struct _PicmanOperationGrow
{
  GeglOperationFilter  parent_instance;

  gint                 radius_x;
  gint                 radius_y;
};

struct _PicmanOperationGrowClass
{
  GeglOperationFilterClass  parent_class;
};


GType   picman_operation_grow_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_GROW_H__ */
