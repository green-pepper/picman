/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanlevelsconfig.c
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

#include <errno.h>
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
#include "picmanlevelsconfig.h"
#include "picmanoperationlevels.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CHANNEL,
  PROP_GAMMA,
  PROP_LOW_INPUT,
  PROP_HIGH_INPUT,
  PROP_LOW_OUTPUT,
  PROP_HIGH_OUTPUT
};


static void     picman_levels_config_iface_init   (PicmanConfigInterface *iface);

static void     picman_levels_config_get_property (GObject          *object,
                                                 guint             property_id,
                                                 GValue           *value,
                                                 GParamSpec       *pspec);
static void     picman_levels_config_set_property (GObject          *object,
                                                 guint             property_id,
                                                 const GValue     *value,
                                                 GParamSpec       *pspec);

static gboolean picman_levels_config_serialize    (PicmanConfig       *config,
                                                 PicmanConfigWriter *writer,
                                                 gpointer          data);
static gboolean picman_levels_config_deserialize  (PicmanConfig       *config,
                                                 GScanner         *scanner,
                                                 gint              nest_level,
                                                 gpointer          data);
static gboolean picman_levels_config_equal        (PicmanConfig       *a,
                                                 PicmanConfig       *b);
static void     picman_levels_config_reset        (PicmanConfig       *config);
static gboolean picman_levels_config_copy         (PicmanConfig       *src,
                                                 PicmanConfig       *dest,
                                                 GParamFlags       flags);


G_DEFINE_TYPE_WITH_CODE (PicmanLevelsConfig, picman_levels_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_levels_config_iface_init))

#define parent_class picman_levels_config_parent_class


static void
picman_levels_config_class_init (PicmanLevelsConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->set_property       = picman_levels_config_set_property;
  object_class->get_property       = picman_levels_config_get_property;

  viewable_class->default_stock_id = "picman-tool-levels";

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CHANNEL,
                                 "channel",
                                 "The affected channel",
                                 PICMAN_TYPE_HISTOGRAM_CHANNEL,
                                 PICMAN_HISTOGRAM_VALUE, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_GAMMA,
                                   "gamma",
                                   "Gamma",
                                   0.1, 10.0, 1.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LOW_INPUT,
                                   "low-input",
                                   "Low Input",
                                   0.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_HIGH_INPUT,
                                   "high-input",
                                   "High Input",
                                   0.0, 1.0, 1.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LOW_OUTPUT,
                                   "low-output",
                                   "Low Output",
                                   0.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_HIGH_OUTPUT,
                                   "high-output",
                                   "High Output",
                                   0.0, 1.0, 1.0, 0);
}

static void
picman_levels_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_levels_config_serialize;
  iface->deserialize = picman_levels_config_deserialize;
  iface->equal       = picman_levels_config_equal;
  iface->reset       = picman_levels_config_reset;
  iface->copy        = picman_levels_config_copy;
}

static void
picman_levels_config_init (PicmanLevelsConfig *self)
{
  picman_config_reset (PICMAN_CONFIG (self));
}

static void
picman_levels_config_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanLevelsConfig *self = PICMAN_LEVELS_CONFIG (object);

  switch (property_id)
    {
    case PROP_CHANNEL:
      g_value_set_enum (value, self->channel);
      break;

    case PROP_GAMMA:
      g_value_set_double (value, self->gamma[self->channel]);
      break;

    case PROP_LOW_INPUT:
      g_value_set_double (value, self->low_input[self->channel]);
      break;

    case PROP_HIGH_INPUT:
      g_value_set_double (value, self->high_input[self->channel]);
      break;

    case PROP_LOW_OUTPUT:
      g_value_set_double (value, self->low_output[self->channel]);
      break;

    case PROP_HIGH_OUTPUT:
      g_value_set_double (value, self->high_output[self->channel]);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_levels_config_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanLevelsConfig *self = PICMAN_LEVELS_CONFIG (object);

  switch (property_id)
    {
    case PROP_CHANNEL:
      self->channel = g_value_get_enum (value);
      g_object_notify (object, "gamma");
      g_object_notify (object, "low-input");
      g_object_notify (object, "high-input");
      g_object_notify (object, "low-output");
      g_object_notify (object, "high-output");
      break;

    case PROP_GAMMA:
      self->gamma[self->channel] = g_value_get_double (value);
      break;

    case PROP_LOW_INPUT:
      self->low_input[self->channel] = g_value_get_double (value);
      break;

    case PROP_HIGH_INPUT:
      self->high_input[self->channel] = g_value_get_double (value);
      break;

    case PROP_LOW_OUTPUT:
      self->low_output[self->channel] = g_value_get_double (value);
      break;

    case PROP_HIGH_OUTPUT:
      self->high_output[self->channel] = g_value_get_double (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_levels_config_serialize (PicmanConfig       *config,
                              PicmanConfigWriter *writer,
                              gpointer          data)
{
  PicmanLevelsConfig     *l_config = PICMAN_LEVELS_CONFIG (config);
  PicmanHistogramChannel  channel;
  PicmanHistogramChannel  old_channel;
  gboolean              success = TRUE;

  if (! picman_config_serialize_property_by_name (config, "time", writer))
    return FALSE;

  old_channel = l_config->channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      l_config->channel = channel;

      success = picman_config_serialize_properties (config, writer);

      if (! success)
        break;
    }

  l_config->channel = old_channel;

  return success;
}

static gboolean
picman_levels_config_deserialize (PicmanConfig *config,
                                GScanner   *scanner,
                                gint        nest_level,
                                gpointer    data)
{
  PicmanLevelsConfig     *l_config = PICMAN_LEVELS_CONFIG (config);
  PicmanHistogramChannel  old_channel;
  gboolean              success = TRUE;

  old_channel = l_config->channel;

  success = picman_config_deserialize_properties (config, scanner, nest_level);

  g_object_set (config, "channel", old_channel, NULL);

  return success;
}

static gboolean
picman_levels_config_equal (PicmanConfig *a,
                          PicmanConfig *b)
{
  PicmanLevelsConfig     *config_a = PICMAN_LEVELS_CONFIG (a);
  PicmanLevelsConfig     *config_b = PICMAN_LEVELS_CONFIG (b);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      if (config_a->gamma[channel]       != config_b->gamma[channel]      ||
          config_a->low_input[channel]   != config_b->low_input[channel]  ||
          config_a->high_input[channel]  != config_b->high_input[channel] ||
          config_a->low_output[channel]  != config_b->low_output[channel] ||
          config_a->high_output[channel] != config_b->high_output[channel])
        return FALSE;
    }

  /* don't compare "channel" */

  return TRUE;
}

static void
picman_levels_config_reset (PicmanConfig *config)
{
  PicmanLevelsConfig     *l_config = PICMAN_LEVELS_CONFIG (config);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      l_config->channel = channel;
      picman_levels_config_reset_channel (l_config);
    }

  picman_config_reset_property (G_OBJECT (config), "channel");
}

static gboolean
picman_levels_config_copy (PicmanConfig  *src,
                         PicmanConfig  *dest,
                         GParamFlags  flags)
{
  PicmanLevelsConfig     *src_config  = PICMAN_LEVELS_CONFIG (src);
  PicmanLevelsConfig     *dest_config = PICMAN_LEVELS_CONFIG (dest);
  PicmanHistogramChannel  channel;

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      dest_config->gamma[channel]       = src_config->gamma[channel];
      dest_config->low_input[channel]   = src_config->low_input[channel];
      dest_config->high_input[channel]  = src_config->high_input[channel];
      dest_config->low_output[channel]  = src_config->low_output[channel];
      dest_config->high_output[channel] = src_config->high_output[channel];
    }

  g_object_notify (G_OBJECT (dest), "gamma");
  g_object_notify (G_OBJECT (dest), "low-input");
  g_object_notify (G_OBJECT (dest), "high-input");
  g_object_notify (G_OBJECT (dest), "low-output");
  g_object_notify (G_OBJECT (dest), "high-output");

  dest_config->channel = src_config->channel;

  g_object_notify (G_OBJECT (dest), "channel");

  return TRUE;
}


/*  public functions  */

void
picman_levels_config_reset_channel (PicmanLevelsConfig *config)
{
  g_return_if_fail (PICMAN_IS_LEVELS_CONFIG (config));

  g_object_freeze_notify (G_OBJECT (config));

  picman_config_reset_property (G_OBJECT (config), "gamma");
  picman_config_reset_property (G_OBJECT (config), "low-input");
  picman_config_reset_property (G_OBJECT (config), "high-input");
  picman_config_reset_property (G_OBJECT (config), "low-output");
  picman_config_reset_property (G_OBJECT (config), "high-output");

  g_object_thaw_notify (G_OBJECT (config));
}

void
picman_levels_config_stretch (PicmanLevelsConfig *config,
                            PicmanHistogram    *histogram,
                            gboolean          is_color)
{
  g_return_if_fail (PICMAN_IS_LEVELS_CONFIG (config));
  g_return_if_fail (histogram != NULL);

  g_object_freeze_notify (G_OBJECT (config));

  if (is_color)
    {
      PicmanHistogramChannel channel;

      /*  Set the overall value to defaults  */
      channel = config->channel;
      config->channel = PICMAN_HISTOGRAM_VALUE;
      picman_levels_config_reset_channel (config);
      config->channel = channel;

      for (channel = PICMAN_HISTOGRAM_RED;
           channel <= PICMAN_HISTOGRAM_BLUE;
           channel++)
        {
          picman_levels_config_stretch_channel (config, histogram, channel);
        }
    }
  else
    {
      picman_levels_config_stretch_channel (config, histogram,
                                          PICMAN_HISTOGRAM_VALUE);
    }

  g_object_thaw_notify (G_OBJECT (config));
}

void
picman_levels_config_stretch_channel (PicmanLevelsConfig     *config,
                                    PicmanHistogram        *histogram,
                                    PicmanHistogramChannel  channel)
{
  gdouble count;
  gint    i;

  g_return_if_fail (PICMAN_IS_LEVELS_CONFIG (config));
  g_return_if_fail (histogram != NULL);

  g_object_freeze_notify (G_OBJECT (config));

  config->gamma[channel]       = 1.0;
  config->low_output[channel]  = 0.0;
  config->high_output[channel] = 1.0;

  count = picman_histogram_get_count (histogram, channel, 0, 255);

  if (count == 0.0)
    {
      config->low_input[channel]  = 0.0;
      config->high_input[channel] = 0.0;
    }
  else
    {
      gdouble new_count;
      gdouble percentage;
      gdouble next_percentage;

      /*  Set the low input  */
      new_count = 0.0;

      for (i = 0; i < 255; i++)
        {
          new_count += picman_histogram_get_value (histogram, channel, i);
          percentage = new_count / count;
          next_percentage = (new_count +
                             picman_histogram_get_value (histogram,
                                                       channel,
                                                       i + 1)) / count;

          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
            {
              config->low_input[channel] = (gdouble) (i + 1) / 255.0;
              break;
            }
        }

      /*  Set the high input  */
      new_count = 0.0;

      for (i = 255; i > 0; i--)
        {
          new_count += picman_histogram_get_value (histogram, channel, i);
          percentage = new_count / count;
          next_percentage = (new_count +
                             picman_histogram_get_value (histogram,
                                                       channel,
                                                       i - 1)) / count;

          if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006))
            {
              config->high_input[channel] = (gdouble) (i - 1) / 255.0;
              break;
            }
        }
    }

  g_object_notify (G_OBJECT (config), "gamma");
  g_object_notify (G_OBJECT (config), "low-input");
  g_object_notify (G_OBJECT (config), "high-input");
  g_object_notify (G_OBJECT (config), "low-output");
  g_object_notify (G_OBJECT (config), "high-output");

  g_object_thaw_notify (G_OBJECT (config));
}

static gdouble
picman_levels_config_input_from_color (PicmanHistogramChannel  channel,
                                     const PicmanRGB        *color)
{
  switch (channel)
    {
    case PICMAN_HISTOGRAM_VALUE:
      return MAX (MAX (color->r, color->g), color->b);

    case PICMAN_HISTOGRAM_RED:
      return color->r;

    case PICMAN_HISTOGRAM_GREEN:
      return color->g;

    case PICMAN_HISTOGRAM_BLUE:
      return color->b;

    case PICMAN_HISTOGRAM_ALPHA:
      return color->a;

    case PICMAN_HISTOGRAM_RGB:
      return MIN (MIN (color->r, color->g), color->b);
    }

  return 0.0;
}

void
picman_levels_config_adjust_by_colors (PicmanLevelsConfig     *config,
                                     PicmanHistogramChannel  channel,
                                     const PicmanRGB        *black,
                                     const PicmanRGB        *gray,
                                     const PicmanRGB        *white)
{
  g_return_if_fail (PICMAN_IS_LEVELS_CONFIG (config));

  g_object_freeze_notify (G_OBJECT (config));

  if (black)
    {
      config->low_input[channel] = picman_levels_config_input_from_color (channel,
                                                                        black);
      g_object_notify (G_OBJECT (config), "low-input");
    }


  if (white)
    {
      config->high_input[channel] = picman_levels_config_input_from_color (channel,
                                                                         white);
      g_object_notify (G_OBJECT (config), "high-input");
    }

  if (gray)
    {
      gdouble input;
      gdouble range;
      gdouble inten;
      gdouble out_light;
      gdouble lightness;

      /* Calculate lightness value */
      lightness = PICMAN_RGB_LUMINANCE (gray->r, gray->g, gray->b);

      input = picman_levels_config_input_from_color (channel, gray);

      range = config->high_input[channel] - config->low_input[channel];
      if (range <= 0)
        goto out;

      input -= config->low_input[channel];
      if (input < 0)
        goto out;

      /* Normalize input and lightness */
      inten = input / range;
      out_light = lightness / range;

      /* See bug 622054: picking pure black or white as gamma doesn't
       * work. But we cannot compare to 0.0 or 1.0 because cpus and
       * compilers are shit. If you try to check out_light using
       * printf() it will give exact 0.0 or 1.0 anyway, probably
       * because the generated code is different and out_light doesn't
       * live in a register. That must be why the cpu/compiler mafia
       * invented epsilon and defined this shit to be the programmer's
       * responsibility.
       */
      if (out_light <= 0.0001 || out_light >= 0.9999)
        goto out;

      /* Map selected color to corresponding lightness */
      config->gamma[channel] = log (inten) / log (out_light);
      config->gamma[channel] = CLAMP (config->gamma[channel], 0.1, 10.0);
      g_object_notify (G_OBJECT (config), "gamma");
    }

 out:
  g_object_thaw_notify (G_OBJECT (config));
}

PicmanCurvesConfig *
picman_levels_config_to_curves_config (PicmanLevelsConfig *config)
{
  PicmanCurvesConfig     *curves;
  PicmanHistogramChannel  channel;

  g_return_val_if_fail (PICMAN_IS_LEVELS_CONFIG (config), NULL);

  curves = g_object_new (PICMAN_TYPE_CURVES_CONFIG, NULL);

  for (channel = PICMAN_HISTOGRAM_VALUE;
       channel <= PICMAN_HISTOGRAM_ALPHA;
       channel++)
    {
      PicmanCurve  *curve    = curves->curve[channel];
      const gint  n_points = picman_curve_get_n_points (curve);
      static const gint n  = 4;
      gint        point    = -1;
      gdouble     gamma    = config->gamma[channel];
      gdouble     delta_in;
      gdouble     delta_out;
      gdouble     x, y;

      /* clear the points set by default */
      picman_curve_set_point (curve, 0, -1, -1);
      picman_curve_set_point (curve, n_points - 1, -1, -1);

      delta_in  = config->high_input[channel] - config->low_input[channel];
      delta_out = config->high_output[channel] - config->low_output[channel];

      x = config->low_input[channel];
      y = config->low_output[channel];

      point = CLAMP (n_points * x, point + 1, n_points - 1 - n);
      picman_curve_set_point (curve, point, x, y);

      if (delta_out != 0 && gamma != 1.0)
        {
          /* The Levels tool performs gamma correction, which is a
           * power law, while the Curves tool uses cubic Bézier
           * curves. Here we try to approximate this gamma correction
           * with a Bézier curve with 5 control points. Two of them
           * must be (low_input, low_output) and (high_input,
           * high_output), so we need to add 3 more control points in
           * the middle.
           */
          gint i;

          if (gamma > 1)
            {
              /* Case no. 1: γ > 1
               *
               * The curve should look like a horizontal
               * parabola. Since its curvature is greatest when x is
               * small, we add more control points there, so the
               * approximation is more accurate. I decided to set the
               * length of the consecutive segments to x₀, γ⋅x₀, γ²⋅x₀
               * and γ³⋅x₀ and I saw that the curves looked
               * good. Still, this is completely arbitrary.
               */
              gdouble dx = 0;
              gdouble x0;

              for (i = 0; i < n; ++i)
                dx = dx * gamma + 1;
              x0 = delta_in / dx;

              dx = 0;
              for (i = 1; i < n; ++i)
                {
                  dx = dx * gamma + x0;
                  x = config->low_input[channel] + dx;
                  y = config->low_output[channel] + delta_out *
                      picman_operation_levels_map_input (config, channel, x);
                  point = CLAMP (n_points * x, point + 1, n_points - 1 - n + i);
                  picman_curve_set_point (curve, point, x, y);
                }
            }
          else
            {
              /* Case no. 2: γ < 1
               *
               * The curve is the same as the one in case no. 1,
               * observed through a reflexion along the y = x axis. So
               * if we invert γ and swap the x and y axes we can use
               * the same method as in case no. 1.
               */
              PicmanLevelsConfig *config_inv;
              gdouble           dy = 0;
              gdouble           y0;
              const gdouble     gamma_inv = 1 / gamma;

              config_inv = picman_config_duplicate (PICMAN_CONFIG (config));

              config_inv->gamma[channel]       = gamma_inv;
              config_inv->low_input[channel]   = config->low_output[channel];
              config_inv->low_output[channel]  = config->low_input[channel];
              config_inv->high_input[channel]  = config->high_output[channel];
              config_inv->high_output[channel] = config->high_input[channel];

              for (i = 0; i < n; ++i)
                dy = dy * gamma_inv + 1;
              y0 = delta_out / dy;

              dy = 0;
              for (i = 1; i < n; ++i)
                {
                  dy = dy * gamma_inv + y0;
                  y = config->low_output[channel] + dy;
                  x = config->low_input[channel] + delta_in *
                      picman_operation_levels_map_input (config_inv, channel, y);
                  point = CLAMP (n_points * x, point + 1, n_points - 1 - n + i);
                  picman_curve_set_point (curve, point, x, y);
                }

              g_object_unref (config_inv);
            }
        }

      x = config->high_input[channel];
      y = config->high_output[channel];

      point = CLAMP (n_points * x, point + 1, n_points - 1);
      picman_curve_set_point (curve, point, x, y);
    }

  return curves;
}

gboolean
picman_levels_config_load_cruft (PicmanLevelsConfig  *config,
                               gpointer           fp,
                               GError           **error)
{
  FILE    *file = fp;
  gint     low_input[5];
  gint     high_input[5];
  gint     low_output[5];
  gint     high_output[5];
  gdouble  gamma[5];
  gint     i;
  gint     fields;
  gchar    buf[50];
  gchar   *nptr;

  g_return_val_if_fail (PICMAN_IS_LEVELS_CONFIG (config), FALSE);
  g_return_val_if_fail (file != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! fgets (buf, sizeof (buf), file) ||
      strcmp (buf, "# PICMAN Levels File\n") != 0)
    {
      g_set_error_literal (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_PARSE,
			   _("not a PICMAN Levels file"));
      return FALSE;
    }

  for (i = 0; i < 5; i++)
    {
      fields = fscanf (file, "%d %d %d %d ",
                       &low_input[i],
                       &high_input[i],
                       &low_output[i],
                       &high_output[i]);

      if (fields != 4)
        goto error;

      if (! fgets (buf, 50, file))
        goto error;

      gamma[i] = g_ascii_strtod (buf, &nptr);

      if (buf == nptr || errno == ERANGE)
        goto error;
    }

  g_object_freeze_notify (G_OBJECT (config));

  for (i = 0; i < 5; i++)
    {
      config->low_input[i]   = low_input[i]   / 255.0;
      config->high_input[i]  = high_input[i]  / 255.0;
      config->low_output[i]  = low_output[i]  / 255.0;
      config->high_output[i] = high_output[i] / 255.0;
      config->gamma[i]       = gamma[i];
    }

  g_object_notify (G_OBJECT (config), "gamma");
  g_object_notify (G_OBJECT (config), "low-input");
  g_object_notify (G_OBJECT (config), "high-input");
  g_object_notify (G_OBJECT (config), "low-output");
  g_object_notify (G_OBJECT (config), "high-output");

  g_object_thaw_notify (G_OBJECT (config));

  return TRUE;

 error:
  g_set_error_literal (error, PICMAN_CONFIG_ERROR, PICMAN_CONFIG_ERROR_PARSE,
		       _("parse error"));
  return FALSE;
}

gboolean
picman_levels_config_save_cruft (PicmanLevelsConfig  *config,
                               gpointer           fp,
                               GError           **error)
{
  FILE *file = fp;
  gint  i;

  g_return_val_if_fail (PICMAN_IS_LEVELS_CONFIG (config), FALSE);
  g_return_val_if_fail (file != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  fprintf (file, "# PICMAN Levels File\n");

  for (i = 0; i < 5; i++)
    {
      gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

      fprintf (file, "%d %d %d %d %s\n",
               (gint) (config->low_input[i]   * 255.999),
               (gint) (config->high_input[i]  * 255.999),
               (gint) (config->low_output[i]  * 255.999),
               (gint) (config->high_output[i] * 255.999),
               g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f",
                                config->gamma[i]));
    }

  return TRUE;
}
