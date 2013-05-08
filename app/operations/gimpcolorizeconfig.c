/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancolorizeconfig.c
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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "operations-types.h"

#include "picmancolorizeconfig.h"


enum
{
  PROP_0,
  PROP_HUE,
  PROP_SATURATION,
  PROP_LIGHTNESS
};


static void   picman_colorize_config_get_property (GObject      *object,
                                                 guint         property_id,
                                                 GValue       *value,
                                                 GParamSpec   *pspec);
static void   picman_colorize_config_set_property (GObject      *object,
                                                 guint         property_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec);


G_DEFINE_TYPE_WITH_CODE (PicmanColorizeConfig, picman_colorize_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

#define parent_class picman_colorize_config_parent_class


static void
picman_colorize_config_class_init (PicmanColorizeConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->set_property       = picman_colorize_config_set_property;
  object_class->get_property       = picman_colorize_config_get_property;

  viewable_class->default_stock_id = "picman-tool-colorize";

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_HUE,
                                   "hue",
                                   "Hue",
                                   0.0, 1.0, 0.5, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SATURATION,
                                   "saturation",
                                   "Saturation",
                                   0.0, 1.0, 0.5, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LIGHTNESS,
                                   "lightness",
                                   "Lightness",
                                   -1.0, 1.0, 0.0, 0);
}

static void
picman_colorize_config_init (PicmanColorizeConfig *self)
{
}

static void
picman_colorize_config_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanColorizeConfig *self = PICMAN_COLORIZE_CONFIG (object);

  switch (property_id)
    {
    case PROP_HUE:
      g_value_set_double (value, self->hue);
      break;

    case PROP_SATURATION:
      g_value_set_double (value, self->saturation);
      break;

    case PROP_LIGHTNESS:
      g_value_set_double (value, self->lightness);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_colorize_config_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanColorizeConfig *self = PICMAN_COLORIZE_CONFIG (object);

  switch (property_id)
    {
    case PROP_HUE:
      self->hue = g_value_get_double (value);
      break;

    case PROP_SATURATION:
      self->saturation = g_value_get_double (value);
      break;

    case PROP_LIGHTNESS:
      self->lightness = g_value_get_double (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

void
picman_colorize_config_get_color (PicmanColorizeConfig *config,
                                PicmanRGB            *color)
{
  PicmanHSL hsl;

  g_return_if_fail (PICMAN_IS_COLORIZE_CONFIG (config));
  g_return_if_fail (color != NULL);

  picman_hsl_set (&hsl,
                config->hue,
                config->saturation,
                (config->lightness + 1.0) / 2.0);
  picman_hsl_to_rgb (&hsl, color);
  picman_rgb_set_alpha (color, 1.0);
}

void
picman_colorize_config_set_color (PicmanColorizeConfig *config,
                                const PicmanRGB      *color)
{
  PicmanHSL hsl;

  g_return_if_fail (PICMAN_IS_COLORIZE_CONFIG (config));
  g_return_if_fail (color != NULL);

  picman_rgb_to_hsl (color, &hsl);

  if (hsl.h == -1)
    hsl.h = config->hue;

  if (hsl.l == 0.0 || hsl.l == 1.0)
    hsl.s = config->saturation;

  g_object_set (config,
                "hue",        hsl.h,
                "saturation", hsl.s,
                "lightness",  hsl.l * 2.0 - 1.0,
                NULL);
}
