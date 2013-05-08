/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationsemiflatten.h
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

#ifndef __PICMAN_OPERATION_SEMI_FLATTEN_H__
#define __PICMAN_OPERATION_SEMI_FLATTEN_H__


#include <gegl-plugin.h>


#define PICMAN_TYPE_OPERATION_SEMI_FLATTEN            (picman_operation_semi_flatten_get_type ())
#define PICMAN_OPERATION_SEMI_FLATTEN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_SEMI_FLATTEN, PicmanOperationSemiFlatten))
#define PICMAN_OPERATION_SEMI_FLATTEN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_SEMI_FLATTEN, PicmanOperationSemiFlattenClass))
#define PICMAN_IS_OPERATION_SEMI_FLATTEN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_SEMI_FLATTEN))
#define PICMAN_IS_OPERATION_SEMI_FLATTEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_SEMI_FLATTEN))
#define PICMAN_OPERATION_SEMI_FLATTEN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_SEMI_FLATTEN, PicmanOperationSemiFlattenClass))


typedef struct _PicmanOperationSemiFlatten      PicmanOperationSemiFlatten;
typedef struct _PicmanOperationSemiFlattenClass PicmanOperationSemiFlattenClass;

struct _PicmanOperationSemiFlatten
{
  GeglOperationPointFilter  parent_instance;

  PicmanRGB                   color;
};

struct _PicmanOperationSemiFlattenClass
{
  GeglOperationPointFilterClass  parent_class;
};


GType   picman_operation_semi_flatten_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_SEMI_FLATTEN_H__ */
