/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationhistogramsink.h
 * Copyright (C) 2012 Øyvind Kolås
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

#ifndef __PICMAN_OPERATION_HISTOGRAM_SINK_H__
#define __PICMAN_OPERATION_HISTOGRAM_SINK_H__


#include <gegl-plugin.h>
#include <operation/gegl-operation-sink.h>


#define PICMAN_TYPE_OPERATION_HISTOGRAM_SINK            (picman_operation_histogram_sink_get_type ())
#define PICMAN_OPERATION_HISTOGRAM_SINK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_HISTOGRAM_SINK, PicmanOperationHistogramSink))
#define PICMAN_OPERATION_HISTOGRAM_SINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_HISTOGRAM_SINK, PicmanOperationHistogramSinkClass))
#define GEGL_IS_OPERATION_HISTOGRAM_SINK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_HISTOGRAM_SINK))
#define GEGL_IS_OPERATION_HISTOGRAM_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_HISTOGRAM_SINK))
#define PICMAN_OPERATION_HISTOGRAM_SINK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_HISTOGRAM_SINK, PicmanOperationHistogramSinkClass))


typedef struct _PicmanOperationHistogramSink      PicmanOperationHistogramSink;
typedef struct _PicmanOperationHistogramSinkClass PicmanOperationHistogramSinkClass;

struct _PicmanOperationHistogramSink
{
  GeglOperation  parent_instance;

  PicmanHistogram *histogram;
};

struct _PicmanOperationHistogramSinkClass
{
  GeglOperationSinkClass  parent_class;
};


GType   picman_operation_histogram_sink_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_HISTOGRAM_SINK_C__ */
