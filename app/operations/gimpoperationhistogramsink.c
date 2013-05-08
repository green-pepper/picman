/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationhistogramsink.c
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

#include "config.h"

#include <gegl.h>

#include "operations-types.h"

#include "core/picmanhistogram.h"

#include "picmanoperationhistogramsink.h"


enum
{
  PROP_0,
  PROP_AUX,
  PROP_HISTOGRAM
};


static void     picman_operation_histogram_sink_finalize     (GObject             *object);
static void     picman_operation_histogram_sink_get_property (GObject             *object,
                                                            guint                prop_id,
                                                            GValue              *value,
                                                            GParamSpec          *pspec);
static void     picman_operation_histogram_sink_set_property (GObject             *object,
                                                            guint                prop_id,
                                                            const GValue        *value,
                                                            GParamSpec          *pspec);

static void     picman_operation_histogram_sink_attach       (GeglOperation       *operation);
static void     picman_operation_histogram_sink_prepare      (GeglOperation       *operation);
static GeglRectangle
     picman_operation_histogram_sink_get_required_for_output (GeglOperation        *self,
                                                            const gchar         *input_pad,
                                                            const GeglRectangle *roi);
static gboolean picman_operation_histogram_sink_process      (GeglOperation       *operation,
                                                            GeglOperationContext     *context,
                                                            const gchar         *output_prop,
                                                            const GeglRectangle *result,
                                                            gint                 level);


G_DEFINE_TYPE (PicmanOperationHistogramSink, picman_operation_histogram_sink,
               GEGL_TYPE_OPERATION_SINK)

#define parent_class picman_operation_histogram_sink_parent_class


static void
picman_operation_histogram_sink_class_init (PicmanOperationHistogramSinkClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);

  object_class->finalize     = picman_operation_histogram_sink_finalize;
  object_class->set_property = picman_operation_histogram_sink_set_property;
  object_class->get_property = picman_operation_histogram_sink_get_property;

  gegl_operation_class_set_keys (operation_class,
                                 "name"       , "picman:histogram-sink",
                                 "categories" , "color",
                                 "description", "PICMAN Histogram sink operation",
                                 NULL);

  operation_class->attach                  = picman_operation_histogram_sink_attach;
  operation_class->prepare                 = picman_operation_histogram_sink_prepare;
  operation_class->get_required_for_output = picman_operation_histogram_sink_get_required_for_output;
  operation_class->process                 = picman_operation_histogram_sink_process;

  g_object_class_install_property (object_class, PROP_AUX,
                                   g_param_spec_object ("aux",
                                                        "Aux",
                                                        "Auxiliary image buffer input pad.",
                                                        GEGL_TYPE_BUFFER,
                                                        G_PARAM_READWRITE |
                                                        GEGL_PARAM_PAD_INPUT));

  g_object_class_install_property (object_class, PROP_HISTOGRAM,
                                   g_param_spec_pointer ("histogram",
                                                         "Histogram",
                                                         "The result histogram",
                                                         G_PARAM_READWRITE));
}

static void
picman_operation_histogram_sink_init (PicmanOperationHistogramSink *self)
{
}

static void
picman_operation_histogram_sink_finalize (GObject *object)
{
  PicmanOperationHistogramSink *sink = PICMAN_OPERATION_HISTOGRAM_SINK (object);

  if (sink->histogram)
    {
      picman_histogram_unref (sink->histogram);
      sink->histogram = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_operation_histogram_sink_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  PicmanOperationHistogramSink *sink = PICMAN_OPERATION_HISTOGRAM_SINK (object);

  switch (prop_id)
    {
    case PROP_AUX:
      break;

    case PROP_HISTOGRAM:
      g_value_set_pointer (value, sink->histogram);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_operation_histogram_sink_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  PicmanOperationHistogramSink *sink = PICMAN_OPERATION_HISTOGRAM_SINK (object);

  switch (prop_id)
    {
    case PROP_AUX:
      break;

    case PROP_HISTOGRAM:
      if (sink->histogram)
        picman_histogram_unref (sink->histogram);
      sink->histogram = g_value_get_pointer (value);
      if (sink->histogram)
        picman_histogram_ref (sink->histogram);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_operation_histogram_sink_attach (GeglOperation *self)
{
  GeglOperation *operation    = GEGL_OPERATION (self);
  GObjectClass  *object_class = G_OBJECT_GET_CLASS (self);

  GEGL_OPERATION_CLASS (parent_class)->attach (self);

  gegl_operation_create_pad (operation,
                             g_object_class_find_property (object_class,
                                                           "aux"));
}

static void
picman_operation_histogram_sink_prepare (GeglOperation *operation)
{
  /* XXX gegl_operation_set_format (operation, "input", babl_format ("Y u8")); */
  gegl_operation_set_format (operation, "aux",   babl_format ("Y float"));
}

static GeglRectangle
picman_operation_histogram_sink_get_required_for_output (GeglOperation       *self,
                                                       const gchar         *input_pad,
                                                       const GeglRectangle *roi)
{
  /* dunno what to do here, make a wild guess */
  return *roi;
}

static gboolean
picman_operation_histogram_sink_process (GeglOperation        *operation,
                                       GeglOperationContext *context,
                                       const gchar          *output_prop,
                                       const GeglRectangle  *result,
                                       gint                  level)
{
  GeglBuffer *input;
  GeglBuffer *aux;

  if (strcmp (output_prop, "output"))
    {
      g_warning ("requested processing of %s pad on a sink", output_prop);
      return FALSE;
    }

  input = gegl_operation_context_get_source (context, "input");
  aux   = gegl_operation_context_get_source (context, "aux");

  if (! input)
    {
      g_warning ("received NULL input");

      return FALSE;
    }

  if (aux)
    {
      /* do hist with mask */

      g_printerr ("aux format: %s\n",
                  babl_get_name (gegl_buffer_get_format (aux)));

      g_object_unref (aux);
    }
  else
    {
      /* without */
    }

  g_printerr ("input format: %s\n",
              babl_get_name (gegl_buffer_get_format (input)));

  g_object_unref (input);

  return TRUE;
}
