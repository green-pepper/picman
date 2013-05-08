/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancurvesconfig.c
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

#include "config.h"

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "operations-types.h"

#include "core/picmancurve.h"
#include "core/picmanhistogram.h"

#include "picmancurvesconfig.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CHANNEL,
  PROP_CURVE
};


static void     picman_curves_config_iface_init   (PicmanConfigInterface *iface);

static void     picman_curves_config_finalize     (GObject          *object);
static void     picman_curves_config_get_property (GObject          *object,
                                                 guint             property_id,
                                                 GValue           *value,
                                                 GParamSpec       *pspec);
static void     picman_curves_config_set_property (GObject          *object,
                                                 guint             property_id,
                                                 const GValue     *value,
                                                 GParamSpec       *pspec);

static gboolean picman_curves_config_serialize    (PicmanConfig       *config,
                                                 PicmanConfigWriter *writer,
                                                 gpointer          data);
static gboolean picman_curves_config_deserialize  (PicmanConfig       *config,
                                                 GScanner         *scanner,
                                                 gint              nest_level,
                                                 gpointer          data);
static gboolean picman_curves_config_equal        (PicmanConfig       *a,
                                                 PicmanConfig       *b);
static void     picman_curves_config_reset        (PicmanConfig       *config);
static gboolean picman_curves_config_copy         (PicmanConfig       *src,
                                                 PicmanConfig       *dest,
                                                 GParamFlags       flags);

static void     picman_curves_config_curve_dirty  (PicmanCurve        *curve,
                                                 PicmanCurvesConfig *config);


G_DEFINE_TYPE_WITH_CODE (PicmanCurvesConfig, picman_curves_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_curves_config_iface_init))

#define parent_class picman_curves_config_parent_class


static void
picman_curves_config_class_init (PicmanCurvesConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->finalize           = picman_curves_config_finalize;
  object_class->set_property       = picman_curves_config_set_property;
  object_class->get_property       = picman_curves_config_get_property;

  viewable_class->default_stock_id = "picman-tool-curves";

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CHANNEL,
                                 "channel",
                                 "The affected channel",
                                 PICMAN_TYPE_HISTOGRAM_CHANNEL,
                                 PICMAN_HISTOGRAM_VALUE, 0);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_CURVE,
                                   "curve",
                                   "Curve",
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);
}

static void
picman_curves_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_curves_config_serialize;
  iface->deserialize = picman_curves_config_deserialize;
  iface->equal       = picman_curves_config_equal;
  iface->reset       = picman_curves_config_reset;
  iface->copy        = picman_curves_config_copy;
}

static void
picman_curves_config_init (PicmanCurvesConfig *self)
{
  PicmanHistogramChannel channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      self->curve[channel] = PICMAN_CURVE (picman_curve_new ("curves config"));

      g_signal_connect_object (self->curve[channel], "dirty",
                               G_CALLBACK (picman_curves_config_curve_dirty),
                               self, 0);
    }

  picman_config_reset (PICMAN_CONFIG (self));
}

static void
picman_curves_config_finalize (GObject *object)
{
  PicmanCurvesConfig     *self = PICMAN_CURVES_CONFIG (object);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      g_object_unref (self->curve[channel]);
      self->curve[channel] = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_curves_config_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanCurvesConfig *self = PICMAN_CURVES_CONFIG (object);

  switch (property_id)
    {
    case PROP_CHANNEL:
      g_value_set_enum (value, self->channel);
      break;

    case PROP_CURVE:
      g_value_set_object (value, self->curve[self->channel]);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_curves_config_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanCurvesConfig *self = PICMAN_CURVES_CONFIG (object);

  switch (property_id)
    {
    case PROP_CHANNEL:
      self->channel = g_value_get_enum (value);
      g_object_notify (object, "curve");
      break;

    case PROP_CURVE:
      {
        PicmanCurve *src_curve  = g_value_get_object (value);
        PicmanCurve *dest_curve = self->curve[self->channel];

        if (src_curve && dest_curve)
          {
            picman_config_copy (PICMAN_CONFIG (src_curve),
                              PICMAN_CONFIG (dest_curve), 0);
          }
      }
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_curves_config_serialize (PicmanConfig       *config,
                              PicmanConfigWriter *writer,
                              gpointer          data)
{
  PicmanCurvesConfig     *c_config = PICMAN_CURVES_CONFIG (config);
  PicmanHistogramChannel  channel;
  PicmanHistogramChannel  old_channel;
  gboolean              success = TRUE;

  if (! picman_config_serialize_property_by_name (config, "time", writer))
    return FALSE;

  old_channel = c_config->channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      c_config->channel = channel;

      success = picman_config_serialize_properties (config, writer);

      if (! success)
        break;
    }

  c_config->channel = old_channel;

  return success;
}

static gboolean
picman_curves_config_deserialize (PicmanConfig *config,
                                GScanner   *scanner,
                                gint        nest_level,
                                gpointer    data)
{
  PicmanCurvesConfig     *c_config = PICMAN_CURVES_CONFIG (config);
  PicmanHistogramChannel  old_channel;
  gboolean              success = TRUE;

  old_channel = c_config->channel;

  success = picman_config_deserialize_properties (config, scanner, nest_level);

  g_object_set (config, "channel", old_channel, NULL);

  return success;
}

static gboolean
picman_curves_config_equal (PicmanConfig *a,
                          PicmanConfig *b)
{
  PicmanCurvesConfig     *config_a = PICMAN_CURVES_CONFIG (a);
  PicmanCurvesConfig     *config_b = PICMAN_CURVES_CONFIG (b);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      PicmanCurve *curve_a = config_a->curve[channel];
      PicmanCurve *curve_b = config_b->curve[channel];

      if (curve_a && curve_b)
        {
          if (! picman_config_is_equal_to (PICMAN_CONFIG (curve_a),
                                         PICMAN_CONFIG (curve_b)))
            return FALSE;
        }
      else if (curve_a || curve_b)
        {
          return FALSE;
        }
    }

  /* don't compare "channel" */

  return TRUE;
}

static void
picman_curves_config_reset (PicmanConfig *config)
{
  PicmanCurvesConfig     *c_config = PICMAN_CURVES_CONFIG (config);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      c_config->channel = channel;
      picman_curves_config_reset_channel (c_config);
    }

  picman_config_reset_property (G_OBJECT (config), "channel");
}

static gboolean
picman_curves_config_copy (PicmanConfig  *src,
                         PicmanConfig  *dest,
                         GParamFlags  flags)
{
  PicmanCurvesConfig     *src_config  = PICMAN_CURVES_CONFIG (src);
  PicmanCurvesConfig     *dest_config = PICMAN_CURVES_CONFIG (dest);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      picman_config_copy (PICMAN_CONFIG (src_config->curve[channel]),
                        PICMAN_CONFIG (dest_config->curve[channel]),
                        flags);
    }

  dest_config->channel = src_config->channel;

  g_object_notify (G_OBJECT (dest), "channel");

  return TRUE;
}

static void
picman_curves_config_curve_dirty (PicmanCurve        *curve,
                                PicmanCurvesConfig *config)
{
  g_object_notify (G_OBJECT (config), "curve");
}


/*  public functions  */

GObject *
picman_curves_config_new_spline (gint32        channel,
                               const guint8 *points,
                               gint          n_points)
{
  PicmanCurvesConfig *config;
  PicmanCurve        *curve;
  gint              i;

  g_return_val_if_fail (channel >= PICMAN_HISTOGRAM_VALUE &&
                        channel <= PICMAN_HISTOGRAM_ALPHA, NULL);

  config = g_object_new (PICMAN_TYPE_CURVES_CONFIG, NULL);

  curve = config->curve[channel];

  picman_data_freeze (PICMAN_DATA (curve));

  /* FIXME: create a curves object with the right number of points */
  /*  unset the last point  */
  picman_curve_set_point (curve, curve->n_points - 1, -1, -1);

  n_points = MIN (n_points / 2, curve->n_points);

  for (i = 0; i < n_points; i++)
    picman_curve_set_point (curve, i,
                          (gdouble) points[i * 2]     / 255.0,
                          (gdouble) points[i * 2 + 1] / 255.0);

  picman_data_thaw (PICMAN_DATA (curve));

  return G_OBJECT (config);
}

GObject *
picman_curves_config_new_explicit (gint32        channel,
                                 const guint8 *points,
                                 gint          n_points)
{
  PicmanCurvesConfig *config;
  PicmanCurve        *curve;
  gint              i;

  g_return_val_if_fail (channel >= PICMAN_HISTOGRAM_VALUE &&
                        channel <= PICMAN_HISTOGRAM_ALPHA, NULL);

  config = g_object_new (PICMAN_TYPE_CURVES_CONFIG, NULL);

  curve = config->curve[channel];

  picman_data_freeze (PICMAN_DATA (curve));

  picman_curve_set_curve_type (curve, PICMAN_CURVE_FREE);

  for (i = 0; i < 256; i++)
    picman_curve_set_curve (curve,
                          (gdouble) i         / 255.0,
                          (gdouble) points[i] / 255.0);

  picman_data_thaw (PICMAN_DATA (curve));

  return G_OBJECT (config);
}

void
picman_curves_config_reset_channel (PicmanCurvesConfig *config)
{
  g_return_if_fail (PICMAN_IS_CURVES_CONFIG (config));

  picman_config_reset (PICMAN_CONFIG (config->curve[config->channel]));
}

#define PICMAN_CURVE_N_CRUFT_POINTS 17

gboolean
picman_curves_config_load_cruft (PicmanCurvesConfig  *config,
                               gpointer           fp,
                               GError           **error)
{
  FILE  *file = fp;
  gint   i, j;
  gint   fields;
  gchar  buf[50];
  gint   index[5][PICMAN_CURVE_N_CRUFT_POINTS];
  gint   value[5][PICMAN_CURVE_N_CRUFT_POINTS];

  g_return_val_if_fail (PICMAN_IS_CURVES_CONFIG (config), FALSE);
  g_return_val_if_fail (file != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! fgets (buf, sizeof (buf), file) ||
      strcmp (buf, "# PICMAN Curves File\n") != 0)
    {
      g_set_error_literal (error,
			   PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_PARSE,
			   _("not a PICMAN Curves file"));
      return FALSE;
    }

  for (i = 0; i < 5; i++)
    {
      for (j = 0; j < PICMAN_CURVE_N_CRUFT_POINTS; j++)
        {
          fields = fscanf (file, "%d %d ", &index[i][j], &value[i][j]);
          if (fields != 2)
            {
              /*  FIXME: should have a helpful error message here  */
              g_printerr ("fields != 2");
              g_set_error_literal (error,
				   PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_PARSE,
				   _("parse error"));
              return FALSE;
            }
        }
    }

  g_object_freeze_notify (G_OBJECT (config));

  for (i = 0; i < 5; i++)
    {
      PicmanCurve *curve = config->curve[i];

      picman_data_freeze (PICMAN_DATA (curve));

      picman_curve_set_curve_type (curve, PICMAN_CURVE_SMOOTH);

      picman_curve_reset (curve, FALSE);

      for (j = 0; j < PICMAN_CURVE_N_CRUFT_POINTS; j++)
        {
          if (index[i][j] < 0 || value[i][j] < 0)
            picman_curve_set_point (curve, j, -1.0, -1.0);
          else
            picman_curve_set_point (curve, j,
                                  (gdouble) index[i][j] / 255.0,
                                  (gdouble) value[i][j] / 255.0);
        }

      picman_data_thaw (PICMAN_DATA (curve));
    }

  g_object_thaw_notify (G_OBJECT (config));

  return TRUE;
}

gboolean
picman_curves_config_save_cruft (PicmanCurvesConfig  *config,
                               gpointer           fp,
                               GError           **error)
{
  FILE *file = fp;
  gint  i;

  g_return_val_if_fail (PICMAN_IS_CURVES_CONFIG (config), FALSE);
  g_return_val_if_fail (file != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  fprintf (file, "# PICMAN Curves File\n");

  for (i = 0; i < 5; i++)
    {
      PicmanCurve *curve = config->curve[i];
      gint       j;

      if (curve->curve_type == PICMAN_CURVE_FREE)
        {
          gint n_points;

          for (j = 0; j < curve->n_points; j++)
            {
              curve->points[j].x = -1;
              curve->points[j].y = -1;
            }

          /* pick some points from the curve and make them control
           * points
           */
          n_points = CLAMP (9, curve->n_points / 2, curve->n_points);

          for (j = 0; j < n_points; j++)
            {
              gint sample = j * (curve->n_samples - 1) / (n_points - 1);
              gint point  = j * (curve->n_points  - 1) / (n_points - 1);

              curve->points[point].x = ((gdouble) sample /
                                        (gdouble) (curve->n_samples - 1));
              curve->points[point].y = curve->samples[sample];
            }
        }

      for (j = 0; j < curve->n_points; j++)
        {
          /* don't use picman_curve_get_point() becaue that doesn't
           * work when the curve type is PICMAN_CURVE_FREE
           */
          gdouble x = curve->points[j].x;
          gdouble y = curve->points[j].y;

          if (x < 0.0 || y < 0.0)
            {
              fprintf (file, "%d %d ", -1, -1);
            }
          else
            {
              fprintf (file, "%d %d ",
                       (gint) (x * 255.999),
                       (gint) (y * 255.999));
            }
        }

      fprintf (file, "\n");
    }

  return TRUE;
}
