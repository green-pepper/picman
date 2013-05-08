/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationshrink.h
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

#ifndef __PICMAN_OPERATION_SHRINK_H__
#define __PICMAN_OPERATION_SHRINK_H__


#include <gegl-plugin.h>


#define PICMAN_TYPE_OPERATION_SHRINK            (picman_operation_shrink_get_type ())
#define PICMAN_OPERATION_SHRINK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_SHRINK, PicmanOperationShrink))
#define PICMAN_OPERATION_SHRINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_SHRINK, PicmanOperationShrinkClass))
#define PICMAN_IS_OPERATION_SHRINK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_SHRINK))
#define PICMAN_IS_OPERATION_SHRINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_SHRINK))
#define PICMAN_OPERATION_SHRINK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_SHRINK, PicmanOperationShrinkClass))


typedef struct _PicmanOperationShrink      PicmanOperationShrink;
typedef struct _PicmanOperationShrinkClass PicmanOperationShrinkClass;

struct _PicmanOperationShrink
{
  GeglOperationFilter  parent_instance;

  gint                 radius_x;
  gint                 radius_y;
  gboolean             edge_lock;
};

struct _PicmanOperationShrinkClass
{
  GeglOperationFilterClass  parent_class;
};


GType   picman_operation_shrink_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_SHRINK_H__ */
