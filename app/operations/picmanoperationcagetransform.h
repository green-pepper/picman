/* PICMAN - The GNU Image Manipulation Program
 *
 * picmanoperationcagetransform.h
 * Copyright (C) 2010 Michael Muré <batolettre@gmail.com>
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

#ifndef __PICMAN_OPERATION_CAGE_TRANSFORM_H__
#define __PICMAN_OPERATION_CAGE_TRANSFORM_H__


#include <gegl-plugin.h>
#include <operation/gegl-operation-composer.h>


#define PICMAN_TYPE_OPERATION_CAGE_TRANSFORM            (picman_operation_cage_transform_get_type ())
#define PICMAN_OPERATION_CAGE_TRANSFORM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_CAGE_TRANSFORM, PicmanOperationCageTransform))
#define PICMAN_OPERATION_CAGE_TRANSFORM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_CAGE_TRANSFORM, PicmanOperationCageTransformClass))
#define PICMAN_IS_OPERATION_CAGE_TRANSFORM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_CAGE_TRANSFORM))
#define PICMAN_IS_OPERATION_CAGE_TRANSFORM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_CAGE_TRANSFORM))
#define PICMAN_OPERATION_CAGE_TRANSFORM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_CAGE_TRANSFORM, PicmanOperationCageTransformClass))


typedef struct _PicmanOperationCageTransform      PicmanOperationCageTransform;
typedef struct _PicmanOperationCageTransformClass PicmanOperationCageTransformClass;

struct _PicmanOperationCageTransform
{
  GeglOperationComposer  parent_instance;

  PicmanCageConfig        *config;
  gboolean               fill_plain_color;

  const Babl            *format_coords;

  gdouble                progress; /* bad hack */
};

struct _PicmanOperationCageTransformClass
{
  GeglOperationComposerClass  parent_class;
};


GType   picman_operation_cage_transform_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_CAGE_TRANSFORM_H__ */
