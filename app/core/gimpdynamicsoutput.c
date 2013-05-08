/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "paint/picmanpaintoptions.h"


#include "picmancurve.h"
#include "picmancurve-map.h"

#include "picmandynamicsoutput.h"

#include "picman-intl.h"


#define DEFAULT_USE_PRESSURE  FALSE
#define DEFAULT_USE_VELOCITY  FALSE
#define DEFAULT_USE_DIRECTION FALSE
#define DEFAULT_USE_TILT      FALSE
#define DEFAULT_USE_WHEEL     FALSE
#define DEFAULT_USE_RANDOM    FALSE
#define DEFAULT_USE_FADE      FALSE


enum
{
  PROP_0,

  PROP_TYPE,
  PROP_USE_PRESSURE,
  PROP_USE_VELOCITY,
  PROP_USE_DIRECTION,
  PROP_USE_TILT,
  PROP_USE_WHEEL,
  PROP_USE_RANDOM,
  PROP_USE_FADE,
  PROP_PRESSURE_CURVE,
  PROP_VELOCITY_CURVE,
  PROP_DIRECTION_CURVE,
  PROP_TILT_CURVE,
  PROP_WHEEL_CURVE,
  PROP_RANDOM_CURVE,
  PROP_FADE_CURVE
};


typedef struct _PicmanDynamicsOutputPrivate PicmanDynamicsOutputPrivate;

struct _PicmanDynamicsOutputPrivate
{
  PicmanDynamicsOutputType  type;

  gboolean                use_pressure;
  gboolean                use_velocity;
  gboolean                use_direction;
  gboolean                use_tilt;
  gboolean                use_wheel;
  gboolean                use_random;
  gboolean                use_fade;

  PicmanCurve              *pressure_curve;
  PicmanCurve              *velocity_curve;
  PicmanCurve              *direction_curve;
  PicmanCurve              *tilt_curve;
  PicmanCurve              *wheel_curve;
  PicmanCurve              *random_curve;
  PicmanCurve              *fade_curve;
};

#define GET_PRIVATE(output) \
        G_TYPE_INSTANCE_GET_PRIVATE (output, \
                                     PICMAN_TYPE_DYNAMICS_OUTPUT, \
                                     PicmanDynamicsOutputPrivate)


static void   picman_dynamics_output_finalize     (GObject           *object);
static void   picman_dynamics_output_set_property (GObject           *object,
                                                 guint              property_id,
                                                 const GValue      *value,
                                                 GParamSpec        *pspec);
static void   picman_dynamics_output_get_property (GObject           *object,
                                                 guint              property_id,
                                                 GValue            *value,
                                                 GParamSpec        *pspec);
static void   picman_dynamics_output_copy_curve   (PicmanCurve          *src,
                                                 PicmanCurve          *dest);

static PicmanCurve *
              picman_dynamics_output_create_curve (PicmanDynamicsOutput *output,
                                                 const gchar        *name);
static void   picman_dynamics_output_curve_dirty  (PicmanCurve          *curve,
                                                 PicmanDynamicsOutput *output);


G_DEFINE_TYPE_WITH_CODE (PicmanDynamicsOutput, picman_dynamics_output,
                         PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

#define parent_class picman_dynamics_output_parent_class


static void
picman_dynamics_output_class_init (PicmanDynamicsOutputClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_dynamics_output_finalize;
  object_class->set_property = picman_dynamics_output_set_property;
  object_class->get_property = picman_dynamics_output_get_property;

  g_object_class_install_property (object_class, PROP_TYPE,
                                   g_param_spec_enum ("type", NULL,
                                                      _("Output type"),
                                                      PICMAN_TYPE_DYNAMICS_OUTPUT_TYPE,
                                                      PICMAN_DYNAMICS_OUTPUT_OPACITY,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_PRESSURE,
                                    "use-pressure", NULL,
                                    DEFAULT_USE_PRESSURE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_VELOCITY,
                                    "use-velocity", NULL,
                                    DEFAULT_USE_VELOCITY,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_DIRECTION,
                                    "use-direction", NULL,
                                    DEFAULT_USE_DIRECTION,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_TILT,
                                    "use-tilt", NULL,
                                    DEFAULT_USE_TILT,
                                    PICMAN_PARAM_STATIC_STRINGS);


  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_WHEEL,
                                    "use-wheel", NULL,
                                    DEFAULT_USE_TILT,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_RANDOM,
                                    "use-random", NULL,
                                    DEFAULT_USE_RANDOM,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_FADE,
                                    "use-fade", NULL,
                                    DEFAULT_USE_FADE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_PRESSURE_CURVE,
                                   "pressure-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_VELOCITY_CURVE,
                                   "velocity-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_DIRECTION_CURVE,
                                   "direction-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_TILT_CURVE,
                                   "tilt-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_WHEEL_CURVE,
                                   "wheel-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_RANDOM_CURVE,
                                   "random-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  PICMAN_CONFIG_INSTALL_PROP_OBJECT (object_class, PROP_FADE_CURVE,
                                   "fade-curve", NULL,
                                   PICMAN_TYPE_CURVE,
                                   PICMAN_CONFIG_PARAM_AGGREGATE);

  g_type_class_add_private (klass, sizeof (PicmanDynamicsOutputPrivate));
}

static void
picman_dynamics_output_init (PicmanDynamicsOutput *output)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (output);

  private->pressure_curve  = picman_dynamics_output_create_curve (output,
                                                                "pressure-curve");
  private->velocity_curve  = picman_dynamics_output_create_curve (output,
                                                                "velocity-curve");
  private->direction_curve = picman_dynamics_output_create_curve (output,
                                                                "direction-curve");
  private->tilt_curve      = picman_dynamics_output_create_curve (output,
                                                                "tilt-curve");
  private->wheel_curve     = picman_dynamics_output_create_curve (output,
                                                                "wheel-curve");
  private->random_curve    = picman_dynamics_output_create_curve (output,
                                                                "random-curve");
  private->fade_curve      = picman_dynamics_output_create_curve (output,
                                                                "fade-curve");
}

static void
picman_dynamics_output_finalize (GObject *object)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (object);

  g_object_unref (private->pressure_curve);
  g_object_unref (private->velocity_curve);
  g_object_unref (private->direction_curve);
  g_object_unref (private->tilt_curve);
  g_object_unref (private->wheel_curve);
  g_object_unref (private->random_curve);
  g_object_unref (private->fade_curve);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_dynamics_output_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TYPE:
      private->type = g_value_get_enum (value);
      break;

    case PROP_USE_PRESSURE:
      private->use_pressure = g_value_get_boolean (value);
      break;

    case PROP_USE_VELOCITY:
      private->use_velocity = g_value_get_boolean (value);
      break;

    case PROP_USE_DIRECTION:
      private->use_direction = g_value_get_boolean (value);
      break;

    case PROP_USE_TILT:
      private->use_tilt = g_value_get_boolean (value);
      break;

    case PROP_USE_WHEEL:
      private->use_wheel = g_value_get_boolean (value);
      break;

    case PROP_USE_RANDOM:
      private->use_random = g_value_get_boolean (value);
      break;

    case PROP_USE_FADE:
      private->use_fade = g_value_get_boolean (value);
      break;

    case PROP_PRESSURE_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->pressure_curve);
      break;

    case PROP_VELOCITY_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->velocity_curve);
      break;

    case PROP_DIRECTION_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->direction_curve);
      break;

    case PROP_TILT_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->tilt_curve);
      break;

    case PROP_WHEEL_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->wheel_curve);
      break;

    case PROP_RANDOM_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->random_curve);
      break;

    case PROP_FADE_CURVE:
      picman_dynamics_output_copy_curve (g_value_get_object (value),
                                       private->fade_curve);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dynamics_output_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_TYPE:
      g_value_set_enum (value, private->type);
      break;

    case PROP_USE_PRESSURE:
      g_value_set_boolean (value, private->use_pressure);
      break;

    case PROP_USE_VELOCITY:
      g_value_set_boolean (value, private->use_velocity);
      break;

    case PROP_USE_DIRECTION:
      g_value_set_boolean (value, private->use_direction);
      break;

    case PROP_USE_TILT:
      g_value_set_boolean (value, private->use_tilt);
      break;

    case PROP_USE_WHEEL:
      g_value_set_boolean (value, private->use_wheel);
      break;

    case PROP_USE_RANDOM:
      g_value_set_boolean (value, private->use_random);
      break;

    case PROP_USE_FADE:
      g_value_set_boolean (value, private->use_fade);
      break;

    case PROP_PRESSURE_CURVE:
      g_value_set_object (value, private->pressure_curve);
      break;

    case PROP_VELOCITY_CURVE:
      g_value_set_object (value, private->velocity_curve);
      break;

    case PROP_DIRECTION_CURVE:
      g_value_set_object (value, private->direction_curve);
      break;

    case PROP_TILT_CURVE:
      g_value_set_object (value, private->tilt_curve);
      break;

    case PROP_WHEEL_CURVE:
      g_value_set_object (value, private->wheel_curve);
      break;

    case PROP_RANDOM_CURVE:
      g_value_set_object (value, private->random_curve);
      break;

    case PROP_FADE_CURVE:
      g_value_set_object (value, private->fade_curve);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

PicmanDynamicsOutput *
picman_dynamics_output_new (const gchar            *name,
                          PicmanDynamicsOutputType  type)
{
  g_return_val_if_fail (name != NULL, NULL);

  return g_object_new (PICMAN_TYPE_DYNAMICS_OUTPUT,
                       "name", name,
                       "type", type,
                       NULL);
}

gboolean
picman_dynamics_output_is_enabled (PicmanDynamicsOutput *output)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (output);

  return (private->use_pressure  ||
          private->use_velocity  ||
          private->use_direction ||
          private->use_tilt      ||
          private->use_wheel     ||
          private->use_random    ||
          private->use_fade);
}

gdouble
picman_dynamics_output_get_linear_value (PicmanDynamicsOutput *output,
                                       const PicmanCoords   *coords,
                                       PicmanPaintOptions   *options,
                                       gdouble             fade_point)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (output);
  gdouble                    total   = 0.0;
  gdouble                    result  = 1.0;
  gint                       factors = 0;

  if (private->use_pressure)
    {
      total += picman_curve_map_value (private->pressure_curve,
                                     coords->pressure);
      factors++;
    }

  if (private->use_velocity)
    {
      total += picman_curve_map_value (private->velocity_curve,
                                    (1.0 - coords->velocity));
      factors++;
    }

  if (private->use_direction)
    {
      total += picman_curve_map_value (private->direction_curve,
                                     fmod (coords->direction + 0.5, 1));
      factors++;
    }

  if (private->use_tilt)
    {
      total += picman_curve_map_value (private->tilt_curve,
                                     (1.0 - sqrt (SQR (coords->xtilt) +
                                      SQR (coords->ytilt))));
      factors++;
    }

  if (private->use_wheel)
    {
      gdouble wheel;

      wheel = coords->wheel;

      total += picman_curve_map_value (private->wheel_curve, wheel);
      factors++;
    }

  if (private->use_random)
    {
      total += picman_curve_map_value (private->random_curve,
                                     g_random_double_range (0.0, 1.0));
      factors++;
    }

  if (private->use_fade)
    {
      total += picman_curve_map_value (private->fade_curve, fade_point);

      factors++;
    }

  if (factors > 0)
    result = total / factors;

#if 0
  g_printerr ("Dynamics queried(linear). Result: %f, factors: %d, total: %f\n",
              result, factors, total);
#endif

  return result;
}

gdouble
picman_dynamics_output_get_angular_value (PicmanDynamicsOutput *output,
                                        const PicmanCoords   *coords,
                                        PicmanPaintOptions   *options,
                                        gdouble             fade_point)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (output);
  gdouble                    total   = 0.0;
  gdouble                    result  = 0.0; /* angles are additive, so we retun zero for no change. */
  gint                       factors = 0;

  if (private->use_pressure)
    {
      total += picman_curve_map_value (private->pressure_curve,
                                     coords->pressure);
      factors++;
    }

  if (private->use_velocity)
    {
      total += picman_curve_map_value (private->velocity_curve,
                                    (1.0 - coords->velocity));
      factors++;
    }

  if (private->use_direction)
    {
      total += picman_curve_map_value (private->direction_curve,
                                     coords->direction);
      factors++;
    }

  /* For tilt to make sense, it needs to be converted to an angle, not
   * just a vector
   */
  if (private->use_tilt)
    {
      gdouble tilt_x = coords->xtilt;
      gdouble tilt_y = coords->ytilt;
      gdouble tilt   = 0.0;

      if (tilt_x == 0.0)
        {
          if (tilt_y >= 0.0)
            tilt = 0.5;
          else if (tilt_y < 0.0)
            tilt = 0.0;
          else
            tilt = -1.0;
        }
      else
        {
          tilt = atan ((- 1.0 * tilt_y) /
                                tilt_x) / (2 * G_PI);

          if (tilt_x > 0.0)
            tilt = tilt + 0.5;
        }

      tilt = tilt + 0.5; /* correct the angle, its wrong by 180 degrees */

      while (tilt > 1.0)
        tilt -= 1.0;

      while (tilt < 0.0)
        tilt += 1.0;

      total += picman_curve_map_value (private->tilt_curve, tilt);
      factors++;
    }

  if (private->use_wheel)
    {
      gdouble angle = 1.0 - fmod(0.5 + coords->wheel, 1);

      total += picman_curve_map_value (private->wheel_curve, angle);
      factors++;
    }

  if (private->use_random)
    {
      total += picman_curve_map_value (private->random_curve,
                                     g_random_double_range (0.0, 1.0));
      factors++;
    }

  if (private->use_fade)
    {
      total += picman_curve_map_value (private->fade_curve, fade_point);

      factors++;
    }

  if (factors > 0)
    result = total / factors;

#if 0
  g_printerr ("Dynamics queried(angle). Result: %f, factors: %d, total: %f\n",
              result, factors, total);
#endif

   return result;
}

gdouble
picman_dynamics_output_get_aspect_value (PicmanDynamicsOutput *output,
                                       const PicmanCoords   *coords,
                                       PicmanPaintOptions   *options,
                                       gdouble             fade_point)
{
  PicmanDynamicsOutputPrivate *private = GET_PRIVATE (output);
  gdouble                    total   = 0.0;
  gint                       factors = 0;
  gdouble                    sign    = 1.0;
  gdouble                    result  = 1.0;

  if (private->use_pressure)
    {
      total += picman_curve_map_value (private->pressure_curve,
                                     coords->pressure);
      factors++;
    }

  if (private->use_velocity)
    {
      total += picman_curve_map_value (private->velocity_curve,
                                     coords->velocity);
      factors++;
    }

  if (private->use_direction)
    {
      gdouble direction = picman_curve_map_value (private->direction_curve,
                                                coords->direction);

      if (((direction > 0.875) && (direction <= 1.0)) ||
          ((direction > 0.0) && (direction < 0.125))  ||
          ((direction > 0.375) && (direction < 0.625)))
        sign = -1.0;

      total += 1.0;
      factors++;
    }

  if (private->use_tilt)
    {
      gdouble tilt_value =  MAX (fabs (coords->xtilt), fabs (coords->ytilt));

      tilt_value = picman_curve_map_value (private->tilt_curve,
                                         tilt_value);

      total += tilt_value;

      factors++;
    }

  if (private->use_wheel)
    {
      gdouble wheel = picman_curve_map_value (private->wheel_curve,
                                            coords->wheel);

      if (((wheel > 0.875) && (wheel <= 1.0)) ||
          ((wheel > 0.0) && (wheel < 0.125))  ||
          ((wheel > 0.375) && (wheel < 0.625)))
        sign = -1.0;

      total += 1.0;
      factors++;

    }

  if (private->use_random)
    {
      gdouble random = picman_curve_map_value (private->random_curve,
                                             g_random_double_range (0.0, 1.0));

      total += random;
      factors++;
    }

  if (private->use_fade)
    {
      total += picman_curve_map_value (private->fade_curve, fade_point);

      factors++;
    }

  if (factors > 0)
    result = total / factors;


#if 0
  g_printerr ("Dynamics queried(aspect). Result: %f, factors: %d, total: %f sign: %f\n",
              result, factors, total, sign);
#endif
  result = CLAMP (result * sign, -1.0, 1.0);

  return result;
}

static void
picman_dynamics_output_copy_curve (PicmanCurve *src,
                                 PicmanCurve *dest)
{
  if (src && dest)
    {
      picman_config_copy (PICMAN_CONFIG (src),
                        PICMAN_CONFIG (dest),
                        PICMAN_CONFIG_PARAM_SERIALIZE);
    }
}

static PicmanCurve *
picman_dynamics_output_create_curve (PicmanDynamicsOutput *output,
                                   const gchar        *name)
{
  PicmanCurve *curve = PICMAN_CURVE (picman_curve_new (name));

  g_signal_connect_object (curve, "dirty",
                           G_CALLBACK (picman_dynamics_output_curve_dirty),
                           output, 0);

  return curve;
}

static void
picman_dynamics_output_curve_dirty (PicmanCurve          *curve,
                                  PicmanDynamicsOutput *output)
{
  g_object_notify (G_OBJECT (output), picman_object_get_name (curve));
}
