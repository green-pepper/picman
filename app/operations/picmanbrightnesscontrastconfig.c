/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrightnesscontrastconfig.c
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

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "operations-types.h"

#include "picmanbrightnesscontrastconfig.h"
#include "picmanlevelsconfig.h"


enum
{
  PROP_0,
  PROP_BRIGHTNESS,
  PROP_CONTRAST
};


static void   picman_brightness_contrast_config_get_property (GObject      *object,
                                                            guint         property_id,
                                                            GValue       *value,
                                                            GParamSpec   *pspec);
static void   picman_brightness_contrast_config_set_property (GObject      *object,
                                                            guint         property_id,
                                                            const GValue *value,
                                                            GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanBrightnessContrastConfig,
                         picman_brightness_contrast_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

#define parent_class picman_brightness_contrast_config_parent_class


static void
picman_brightness_contrast_config_class_init (PicmanBrightnessContrastConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->set_property       = picman_brightness_contrast_config_set_property;
  object_class->get_property       = picman_brightness_contrast_config_get_property;

  viewable_class->default_stock_id = "picman-tool-brightness-contrast";

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_BRIGHTNESS,
                                   "brightness",
                                   "Brightness",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_CONTRAST,
                                   "contrast",
                                   "Contrast",
                                   -1.0, 1.0, 0.0, 0);
}

static void
picman_brightness_contrast_config_init (PicmanBrightnessContrastConfig *self)
{
}

static void
picman_brightness_contrast_config_get_property (GObject    *object,
                                              guint       property_id,
                                              GValue     *value,
                                              GParamSpec *pspec)
{
  PicmanBrightnessContrastConfig *self = PICMAN_BRIGHTNESS_CONTRAST_CONFIG (object);

  switch (property_id)
    {
    case PROP_BRIGHTNESS:
      g_value_set_double (value, self->brightness);
      break;

    case PROP_CONTRAST:
      g_value_set_double (value, self->contrast);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_brightness_contrast_config_set_property (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
  PicmanBrightnessContrastConfig *self = PICMAN_BRIGHTNESS_CONTRAST_CONFIG (object);

  switch (property_id)
    {
    case PROP_BRIGHTNESS:
      self->brightness = g_value_get_double (value);
      break;

    case PROP_CONTRAST:
      self->contrast = g_value_get_double (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

PicmanLevelsConfig *
picman_brightness_contrast_config_to_levels_config (PicmanBrightnessContrastConfig *config)
{
  PicmanLevelsConfig *levels;
  gdouble           brightness;
  gdouble           slant;
  gdouble           value;

  g_return_val_if_fail (PICMAN_IS_BRIGHTNESS_CONTRAST_CONFIG (config), NULL);

  levels = g_object_new (PICMAN_TYPE_LEVELS_CONFIG, NULL);

  brightness = config->brightness / 2.0;
  slant = tan ((config->contrast + 1) * G_PI_4);

  if (config->brightness >= 0)
    {
      value = -0.5 * slant + brightness * slant + 0.5;

      if (value < 0.0)
        {
          value = 0.0;

          /* this slightly convoluted math follows by inverting the
           * calculation of the brightness/contrast LUT in base/lut-funcs.h */

          levels->low_input[PICMAN_HISTOGRAM_VALUE] =
            (- brightness * slant + 0.5 * slant - 0.5) / (slant - brightness * slant);
        }

      levels->low_output[PICMAN_HISTOGRAM_VALUE] = value;

      value = 0.5 * slant + 0.5;

      if (value > 1.0)
        {
          value = 1.0;

          levels->high_input[PICMAN_HISTOGRAM_VALUE] =
            (- brightness * slant + 0.5 * slant + 0.5) / (slant - brightness * slant);
        }

      levels->high_output[PICMAN_HISTOGRAM_VALUE] = value;
    }
  else
    {
      value = 0.5 - 0.5 * slant;

      if (value < 0.0)
        {
          value = 0.0;

          levels->low_input[PICMAN_HISTOGRAM_VALUE] =
            (0.5 * slant - 0.5) / (slant + brightness * slant);
        }

      levels->low_output[PICMAN_HISTOGRAM_VALUE] = value;

      value = slant * brightness + slant * 0.5 + 0.5;

      if (value > 1.0)
        {
          value = 1.0;

          levels->high_input[PICMAN_HISTOGRAM_VALUE] =
            (0.5 * slant + 0.5) / (slant + brightness * slant);
        }

      levels->high_output[PICMAN_HISTOGRAM_VALUE] = value;
    }

  return levels;
}
