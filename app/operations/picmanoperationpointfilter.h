/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationpointfilter.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_POINT_FILTER_H__
#define __PICMAN_OPERATION_POINT_FILTER_H__


#include <gegl-plugin.h>
#include <operation/gegl-operation-point-filter.h>


enum
{
  PICMAN_OPERATION_POINT_FILTER_PROP_0,
  PICMAN_OPERATION_POINT_FILTER_PROP_CONFIG
};


#define PICMAN_TYPE_OPERATION_POINT_FILTER            (picman_operation_point_filter_get_type ())
#define PICMAN_OPERATION_POINT_FILTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_POINT_FILTER, PicmanOperationPointFilter))
#define PICMAN_OPERATION_POINT_FILTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_POINT_FILTER, PicmanOperationPointFilterClass))
#define PICMAN_IS_OPERATION_POINT_FILTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_POINT_FILTER))
#define PICMAN_IS_OPERATION_POINT_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_POINT_FILTER))
#define PICMAN_OPERATION_POINT_FILTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_POINT_FILTER, PicmanOperationPointFilterClass))


typedef struct _PicmanOperationPointFilterClass PicmanOperationPointFilterClass;

struct _PicmanOperationPointFilter
{
  GeglOperationPointFilter  parent_instance;

  GObject                  *config;
};

struct _PicmanOperationPointFilterClass
{
  GeglOperationPointFilterClass  parent_class;
};


GType   picman_operation_point_filter_get_type     (void) G_GNUC_CONST;

void    picman_operation_point_filter_get_property (GObject      *object,
                                                  guint         property_id,
                                                  GValue       *value,
                                                  GParamSpec   *pspec);
void    picman_operation_point_filter_set_property (GObject      *object,
                                                  guint         property_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec);


#endif /* __PICMAN_OPERATION_POINT_FILTER_H__ */
