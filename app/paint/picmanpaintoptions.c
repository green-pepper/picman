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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "paint-types.h"

#include "core/picman.h"
#include "core/picmanimage.h"
#include "core/picmandynamics.h"
#include "core/picmandynamicsoutput.h"
#include "core/picmangradient.h"
#include "core/picmanpaintinfo.h"

#include "picmanpaintoptions.h"

#include "picman-intl.h"


#define DEFAULT_BRUSH_SIZE             20.0
#define DEFAULT_BRUSH_ASPECT_RATIO     0.0
#define DEFAULT_BRUSH_ANGLE            0.0

#define DEFAULT_APPLICATION_MODE       PICMAN_PAINT_CONSTANT
#define DEFAULT_HARD                   FALSE

#define DEFAULT_USE_JITTER             FALSE
#define DEFAULT_JITTER_AMOUNT          0.2

#define DEFAULT_DYNAMICS_EXPANDED      FALSE

#define DEFAULT_FADE_LENGTH            100.0
#define DEFAULT_FADE_REVERSE           FALSE
#define DEFAULT_FADE_REPEAT            PICMAN_REPEAT_NONE
#define DEFAULT_FADE_UNIT              PICMAN_UNIT_PIXEL

#define DEFAULT_GRADIENT_REVERSE       FALSE
#define DEFAULT_GRADIENT_REPEAT        PICMAN_REPEAT_TRIANGULAR
#define DEFAULT_GRADIENT_LENGTH        100.0
#define DEFAULT_GRADIENT_UNIT          PICMAN_UNIT_PIXEL

#define DYNAMIC_MAX_VALUE              1.0
#define DYNAMIC_MIN_VALUE              0.0

#define DEFAULT_SMOOTHING_QUALITY      20
#define DEFAULT_SMOOTHING_FACTOR       50


enum
{
  PROP_0,

  PROP_PAINT_INFO,

  PROP_BRUSH_SIZE,
  PROP_BRUSH_ASPECT_RATIO,
  PROP_BRUSH_ANGLE,

  PROP_APPLICATION_MODE,
  PROP_HARD,

  PROP_USE_JITTER,
  PROP_JITTER_AMOUNT,

  PROP_DYNAMICS_EXPANDED,

  PROP_FADE_LENGTH,
  PROP_FADE_REVERSE,
  PROP_FADE_REPEAT,
  PROP_FADE_UNIT,

  PROP_GRADIENT_REVERSE,

  PROP_BRUSH_VIEW_TYPE,
  PROP_BRUSH_VIEW_SIZE,
  PROP_DYNAMICS_VIEW_TYPE,
  PROP_DYNAMICS_VIEW_SIZE,
  PROP_PATTERN_VIEW_TYPE,
  PROP_PATTERN_VIEW_SIZE,
  PROP_GRADIENT_VIEW_TYPE,
  PROP_GRADIENT_VIEW_SIZE,

  PROP_USE_SMOOTHING,
  PROP_SMOOTHING_QUALITY,
  PROP_SMOOTHING_FACTOR
};


static void    picman_paint_options_dispose          (GObject      *object);
static void    picman_paint_options_finalize         (GObject      *object);
static void    picman_paint_options_set_property     (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void    picman_paint_options_get_property     (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);



G_DEFINE_TYPE (PicmanPaintOptions, picman_paint_options, PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_paint_options_parent_class


static void
picman_paint_options_class_init (PicmanPaintOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = picman_paint_options_dispose;
  object_class->finalize     = picman_paint_options_finalize;
  object_class->set_property = picman_paint_options_set_property;
  object_class->get_property = picman_paint_options_get_property;

  g_object_class_install_property (object_class, PROP_PAINT_INFO,
                                   g_param_spec_object ("paint-info",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PAINT_INFO,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_BRUSH_SIZE,
                                   "brush-size", _("Brush Size"),
                                   1.0, 10000.0, DEFAULT_BRUSH_SIZE,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_BRUSH_ASPECT_RATIO,
                                   "brush-aspect-ratio", _("Brush Aspect Ratio"),
                                   -20.0, 20.0, DEFAULT_BRUSH_ASPECT_RATIO,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_BRUSH_ANGLE,
                                   "brush-angle", _("Brush Angle"),
                                   -180.0, 180.0, DEFAULT_BRUSH_ANGLE,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_APPLICATION_MODE,
                                 "application-mode", _("Every stamp has its own opacity"),
                                 PICMAN_TYPE_PAINT_APPLICATION_MODE,
                                 DEFAULT_APPLICATION_MODE,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_HARD,
                                    "hard", _("Ignore fuzziness of the current brush"),
                                    DEFAULT_HARD,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_JITTER,
                                    "use-jitter", _("Scatter brush as you paint"),
                                    DEFAULT_USE_JITTER,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_JITTER_AMOUNT,
                                   "jitter-amount", _("Distance of scattering"),
                                   0.0, 50.0, DEFAULT_JITTER_AMOUNT,
                                   PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_DYNAMICS_EXPANDED,
                                     "dynamics-expanded", NULL,
                                    DEFAULT_DYNAMICS_EXPANDED,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_FADE_LENGTH,
                                   "fade-length", _("Distance over which strokes fade out"),
                                   0.0, 32767.0, DEFAULT_FADE_LENGTH,
                                   PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_UNIT (object_class, PROP_FADE_UNIT,
                                 "fade-unit", NULL,
                                 TRUE, TRUE, DEFAULT_FADE_UNIT,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_FADE_REVERSE,
                                    "fade-reverse", _("Reverse direction of fading"),
                                    DEFAULT_FADE_REVERSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_FADE_REPEAT,
                                 "fade-repeat", _("How fade is repeated as you paint"),
                                 PICMAN_TYPE_REPEAT_MODE,
                                 DEFAULT_FADE_REPEAT,
                                 PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_GRADIENT_REVERSE,
                                    "gradient-reverse", NULL,
                                    DEFAULT_GRADIENT_REVERSE,
                                    PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_BRUSH_VIEW_TYPE,
                                 "brush-view-type", NULL,
                                 PICMAN_TYPE_VIEW_TYPE,
                                 PICMAN_VIEW_TYPE_GRID,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_BRUSH_VIEW_SIZE,
                                "brush-view-size", NULL,
                                PICMAN_VIEW_SIZE_TINY,
                                PICMAN_VIEWABLE_MAX_BUTTON_SIZE,
                                PICMAN_VIEW_SIZE_SMALL,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_DYNAMICS_VIEW_TYPE,
                                  "dynamics-view-type", NULL,
                                 PICMAN_TYPE_VIEW_TYPE,
                                 PICMAN_VIEW_TYPE_LIST,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_DYNAMICS_VIEW_SIZE,
                                "dynamics-view-size", NULL,
                                PICMAN_VIEW_SIZE_TINY,
                                PICMAN_VIEWABLE_MAX_BUTTON_SIZE,
                                PICMAN_VIEW_SIZE_SMALL,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_PATTERN_VIEW_TYPE,
                                 "pattern-view-type", NULL,
                                 PICMAN_TYPE_VIEW_TYPE,
                                 PICMAN_VIEW_TYPE_GRID,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_PATTERN_VIEW_SIZE,
                                "pattern-view-size", NULL,
                                PICMAN_VIEW_SIZE_TINY,
                                PICMAN_VIEWABLE_MAX_BUTTON_SIZE,
                                PICMAN_VIEW_SIZE_SMALL,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_GRADIENT_VIEW_TYPE,
                                 "gradient-view-type", NULL,
                                 PICMAN_TYPE_VIEW_TYPE,
                                 PICMAN_VIEW_TYPE_LIST,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_GRADIENT_VIEW_SIZE,
                                "gradient-view-size", NULL,
                                PICMAN_VIEW_SIZE_TINY,
                                PICMAN_VIEWABLE_MAX_BUTTON_SIZE,
                                PICMAN_VIEW_SIZE_LARGE,
                                PICMAN_PARAM_STATIC_STRINGS);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_USE_SMOOTHING,
                                    "use-smoothing", _("Paint smoother strokes"),
                                    FALSE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_INT (object_class, PROP_SMOOTHING_QUALITY,
                                "smoothing-quality", _("Depth of smoothing"),
                                1, 100, DEFAULT_SMOOTHING_QUALITY,
                                PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SMOOTHING_FACTOR,
                                   "smoothing-factor", _("Gravity of the pen"),
                                   3.0, 1000.0, DEFAULT_SMOOTHING_FACTOR,
                                   /* Max velocity is set at 3.
                                    * Allowing for smoothing factor to be
                                    * less than velcoty results in numeric
                                    * instablility */
                                   PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_paint_options_init (PicmanPaintOptions *options)
{
  options->application_mode_save = DEFAULT_APPLICATION_MODE;

  options->jitter_options    = g_slice_new0 (PicmanJitterOptions);
  options->fade_options      = g_slice_new0 (PicmanFadeOptions);
  options->gradient_options  = g_slice_new0 (PicmanGradientOptions);
  options->smoothing_options = g_slice_new0 (PicmanSmoothingOptions);
}

static void
picman_paint_options_dispose (GObject *object)
{
  PicmanPaintOptions *options = PICMAN_PAINT_OPTIONS (object);

  if (options->paint_info)
    {
      g_object_unref (options->paint_info);
      options->paint_info = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_paint_options_finalize (GObject *object)
{
  PicmanPaintOptions *options = PICMAN_PAINT_OPTIONS (object);

  g_slice_free (PicmanJitterOptions,    options->jitter_options);
  g_slice_free (PicmanFadeOptions,      options->fade_options);
  g_slice_free (PicmanGradientOptions,  options->gradient_options);
  g_slice_free (PicmanSmoothingOptions, options->smoothing_options);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_paint_options_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanPaintOptions    *options            = PICMAN_PAINT_OPTIONS (object);
  PicmanFadeOptions     *fade_options       = options->fade_options;
  PicmanJitterOptions   *jitter_options     = options->jitter_options;
  PicmanGradientOptions *gradient_options   = options->gradient_options;
  PicmanSmoothingOptions *smoothing_options = options->smoothing_options;

  switch (property_id)
    {
    case PROP_PAINT_INFO:
      options->paint_info = g_value_dup_object (value);
      break;

    case PROP_BRUSH_SIZE:
      options->brush_size = g_value_get_double (value);
      break;

    case PROP_BRUSH_ASPECT_RATIO:
      options->brush_aspect_ratio = g_value_get_double (value);
      break;

    case PROP_BRUSH_ANGLE:
      options->brush_angle = - 1.0 * g_value_get_double (value) / 360.0; /* let's make the angle mathematically correct */
      break;

    case PROP_APPLICATION_MODE:
      options->application_mode = g_value_get_enum (value);
      break;

    case PROP_HARD:
      options->hard = g_value_get_boolean (value);
      break;

    case PROP_USE_JITTER:
      jitter_options->use_jitter = g_value_get_boolean (value);
      break;

    case PROP_JITTER_AMOUNT:
      jitter_options->jitter_amount = g_value_get_double (value);
      break;

    case PROP_DYNAMICS_EXPANDED:
      options->dynamics_expanded = g_value_get_boolean (value);
      break;

    case PROP_FADE_LENGTH:
      fade_options->fade_length = g_value_get_double (value);
      break;

    case PROP_FADE_REVERSE:
      fade_options->fade_reverse = g_value_get_boolean (value);
      break;

    case PROP_FADE_REPEAT:
      fade_options->fade_repeat = g_value_get_enum (value);
      break;

    case PROP_FADE_UNIT:
      fade_options->fade_unit = g_value_get_int (value);
      break;

    case PROP_GRADIENT_REVERSE:
      gradient_options->gradient_reverse = g_value_get_boolean (value);
      break;

    case PROP_BRUSH_VIEW_TYPE:
      options->brush_view_type = g_value_get_enum (value);
      break;

    case PROP_BRUSH_VIEW_SIZE:
      options->brush_view_size = g_value_get_int (value);
      break;

    case PROP_DYNAMICS_VIEW_TYPE:
      options->dynamics_view_type = g_value_get_enum (value);
      break;

    case PROP_DYNAMICS_VIEW_SIZE:
      options->dynamics_view_size = g_value_get_int (value);
      break;

    case PROP_PATTERN_VIEW_TYPE:
      options->pattern_view_type = g_value_get_enum (value);
      break;

    case PROP_PATTERN_VIEW_SIZE:
      options->pattern_view_size = g_value_get_int (value);
      break;

    case PROP_GRADIENT_VIEW_TYPE:
      options->gradient_view_type = g_value_get_enum (value);
      break;

    case PROP_GRADIENT_VIEW_SIZE:
      options->gradient_view_size = g_value_get_int (value);
      break;

    case PROP_USE_SMOOTHING:
      smoothing_options->use_smoothing = g_value_get_boolean (value);
      break;

    case PROP_SMOOTHING_QUALITY:
      smoothing_options->smoothing_quality = g_value_get_int (value);
      break;

    case PROP_SMOOTHING_FACTOR:
      smoothing_options->smoothing_factor = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_paint_options_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanPaintOptions     *options           = PICMAN_PAINT_OPTIONS (object);
  PicmanFadeOptions      *fade_options      = options->fade_options;
  PicmanJitterOptions    *jitter_options    = options->jitter_options;
  PicmanGradientOptions  *gradient_options  = options->gradient_options;
  PicmanSmoothingOptions *smoothing_options = options->smoothing_options;

  switch (property_id)
    {
    case PROP_PAINT_INFO:
      g_value_set_object (value, options->paint_info);
      break;

    case PROP_BRUSH_SIZE:
      g_value_set_double (value, options->brush_size);
      break;

    case PROP_BRUSH_ASPECT_RATIO:
      g_value_set_double (value, options->brush_aspect_ratio);
      break;

    case PROP_BRUSH_ANGLE:
      g_value_set_double (value, - 1.0 * options->brush_angle * 360.0); /* mathematically correct -> intuitively correct */
      break;

    case PROP_APPLICATION_MODE:
      g_value_set_enum (value, options->application_mode);
      break;

    case PROP_HARD:
      g_value_set_boolean (value, options->hard);
      break;

    case PROP_USE_JITTER:
      g_value_set_boolean (value, jitter_options->use_jitter);
      break;

    case PROP_JITTER_AMOUNT:
      g_value_set_double (value, jitter_options->jitter_amount);
      break;

    case PROP_DYNAMICS_EXPANDED:
      g_value_set_boolean (value, options->dynamics_expanded);
      break;

    case PROP_FADE_LENGTH:
      g_value_set_double (value, fade_options->fade_length);
      break;

    case PROP_FADE_REVERSE:
      g_value_set_boolean (value, fade_options->fade_reverse);
      break;

    case PROP_FADE_REPEAT:
      g_value_set_enum (value, fade_options->fade_repeat);
      break;

    case PROP_FADE_UNIT:
      g_value_set_int (value, fade_options->fade_unit);
      break;

    case PROP_GRADIENT_REVERSE:
      g_value_set_boolean (value, gradient_options->gradient_reverse);
      break;

    case PROP_BRUSH_VIEW_TYPE:
      g_value_set_enum (value, options->brush_view_type);
      break;

    case PROP_BRUSH_VIEW_SIZE:
      g_value_set_int (value, options->brush_view_size);
      break;

    case PROP_DYNAMICS_VIEW_TYPE:
      g_value_set_enum (value, options->dynamics_view_type);
      break;

    case PROP_DYNAMICS_VIEW_SIZE:
      g_value_set_int (value, options->dynamics_view_size);
      break;

    case PROP_PATTERN_VIEW_TYPE:
      g_value_set_enum (value, options->pattern_view_type);
      break;

    case PROP_PATTERN_VIEW_SIZE:
      g_value_set_int (value, options->pattern_view_size);
      break;

    case PROP_GRADIENT_VIEW_TYPE:
      g_value_set_enum (value, options->gradient_view_type);
      break;

    case PROP_GRADIENT_VIEW_SIZE:
      g_value_set_int (value, options->gradient_view_size);
      break;

    case PROP_USE_SMOOTHING:
      g_value_set_boolean (value, smoothing_options->use_smoothing);
      break;

    case PROP_SMOOTHING_QUALITY:
      g_value_set_int (value, smoothing_options->smoothing_quality);
      break;

    case PROP_SMOOTHING_FACTOR:
      g_value_set_double (value, smoothing_options->smoothing_factor);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


PicmanPaintOptions *
picman_paint_options_new (PicmanPaintInfo *paint_info)
{
  PicmanPaintOptions *options;

  g_return_val_if_fail (PICMAN_IS_PAINT_INFO (paint_info), NULL);

  options = g_object_new (paint_info->paint_options_type,
                          "picman",       paint_info->picman,
                          "name",       picman_object_get_name (paint_info),
                          "paint-info", paint_info,
                          NULL);

  return options;
}

gdouble
picman_paint_options_get_fade (PicmanPaintOptions *paint_options,
                             PicmanImage        *image,
                             gdouble           pixel_dist)
{
  PicmanFadeOptions *fade_options;
  gdouble          z        = -1.0;
  gdouble          fade_out =  0.0;
  gdouble          unit_factor;
  gdouble          pos;

  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options),
                        DYNAMIC_MAX_VALUE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), DYNAMIC_MAX_VALUE);

  fade_options = paint_options->fade_options;

  switch (fade_options->fade_unit)
    {
    case PICMAN_UNIT_PIXEL:
      fade_out = fade_options->fade_length;
      break;

    case PICMAN_UNIT_PERCENT:
      fade_out = (MAX (picman_image_get_width  (image),
                       picman_image_get_height (image)) *
                  fade_options->fade_length / 100);
      break;

    default:
      {
        gdouble xres;
        gdouble yres;

        picman_image_get_resolution (image, &xres, &yres);

        unit_factor = picman_unit_get_factor (fade_options->fade_unit);
        fade_out    = (fade_options->fade_length *
                       MAX (xres, yres) / unit_factor);
      }
      break;
    }

  /*  factor in the fade out value  */
  if (fade_out > 0.0)
    {
      pos = pixel_dist / fade_out;
    }
  else
    pos = DYNAMIC_MAX_VALUE;

  /*  for no repeat, set pos close to 1.0 after the first chunk  */
  if (fade_options->fade_repeat == PICMAN_REPEAT_NONE && pos >= DYNAMIC_MAX_VALUE)
    pos = DYNAMIC_MAX_VALUE - 0.0000001;

  if (((gint) pos & 1) &&
      fade_options->fade_repeat != PICMAN_REPEAT_SAWTOOTH)
    pos = DYNAMIC_MAX_VALUE - (pos - (gint) pos);
  else
    pos = pos - (gint) pos;

  z = pos;

  if (fade_options->fade_reverse)
    z = 1.0 - z;

  return z;    /*  ln (1/255)  */
}

gdouble
picman_paint_options_get_jitter (PicmanPaintOptions *paint_options,
                               PicmanImage        *image)
{
  PicmanJitterOptions *jitter_options;

  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), 0.0);

  jitter_options = paint_options->jitter_options;

  if (jitter_options->use_jitter)
    {
      return jitter_options->jitter_amount;
    }

  return 0.0;
}

gboolean
picman_paint_options_get_gradient_color (PicmanPaintOptions *paint_options,
                                       PicmanImage        *image,
                                       gdouble           grad_point,
                                       gdouble           pixel_dist,
                                       PicmanRGB          *color)
{
  PicmanGradientOptions *gradient_options;
  PicmanGradient        *gradient;
  PicmanDynamics        *dynamics;
  PicmanDynamicsOutput  *color_output;

  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), FALSE);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  gradient_options = paint_options->gradient_options;

  gradient = picman_context_get_gradient (PICMAN_CONTEXT (paint_options));

  dynamics = picman_context_get_dynamics (PICMAN_CONTEXT (paint_options));

  color_output = picman_dynamics_get_output (dynamics,
                                           PICMAN_DYNAMICS_OUTPUT_COLOR);

  if (picman_dynamics_output_is_enabled (color_output))
    {
      picman_gradient_get_color_at (gradient, PICMAN_CONTEXT (paint_options),
                                  NULL, grad_point,
                                  gradient_options->gradient_reverse,
                                  color);

      return TRUE;
    }

  return FALSE;
}

PicmanBrushApplicationMode
picman_paint_options_get_brush_mode (PicmanPaintOptions *paint_options)
{
  PicmanDynamics       *dynamics;
  PicmanDynamicsOutput *force_output;

  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), PICMAN_BRUSH_SOFT);

  if (paint_options->hard)
    return PICMAN_BRUSH_HARD;

  dynamics = picman_context_get_dynamics (PICMAN_CONTEXT (paint_options));

  force_output = picman_dynamics_get_output (dynamics,
                                           PICMAN_DYNAMICS_OUTPUT_FORCE);

  if (!force_output)
    return PICMAN_BRUSH_SOFT;

  if (picman_dynamics_output_is_enabled (force_output))
    return PICMAN_BRUSH_PRESSURE;

  return PICMAN_BRUSH_SOFT;
}

void
picman_paint_options_copy_brush_props (PicmanPaintOptions *src,
                                     PicmanPaintOptions *dest)
{
  gdouble  brush_size;
  gdouble  brush_angle;
  gdouble  brush_aspect_ratio;

  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (src));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (dest));

  g_object_get (src,
                "brush-size", &brush_size,
                "brush-angle", &brush_angle,
                "brush-aspect-ratio", &brush_aspect_ratio,
                NULL);

  g_object_set (dest,
                "brush-size", brush_size,
                "brush-angle", brush_angle,
                "brush-aspect-ratio", brush_aspect_ratio,
                NULL);
}

void
picman_paint_options_copy_dynamics_props (PicmanPaintOptions *src,
                                        PicmanPaintOptions *dest)
{
  gboolean        dynamics_expanded;
  gboolean        fade_reverse;
  gdouble         fade_length;
  PicmanUnit        fade_unit;
  PicmanRepeatMode  fade_repeat;

  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (src));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (dest));

  g_object_get (src,
                "dynamics-expanded", &dynamics_expanded,
                "fade-reverse", &fade_reverse,
                "fade-length", &fade_length,
                "fade-unit", &fade_unit,
                "fade-repeat", &fade_repeat,
                NULL);

  g_object_set (dest,
                "dynamics-expanded", dynamics_expanded,
                "fade-reverse", fade_reverse,
                "fade-length", fade_length,
                "fade-unit", fade_unit,
                "fade-repeat", fade_repeat,
                NULL);
}

void
picman_paint_options_copy_gradient_props (PicmanPaintOptions *src,
                                        PicmanPaintOptions *dest)
{
  gboolean  gradient_reverse;

  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (src));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (dest));

  g_object_get (src,
                "gradient-reverse", &gradient_reverse,
                NULL);

  g_object_set (dest,
                "gradient-reverse", gradient_reverse,
                NULL);
}
