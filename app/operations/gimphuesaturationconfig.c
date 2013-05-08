/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanhuesaturationconfig.c
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

#include "libpicmanconfig/picmanconfig.h"

#include "operations-types.h"

#include "picmanhuesaturationconfig.h"


enum
{
  PROP_0,
  PROP_RANGE,
  PROP_HUE,
  PROP_SATURATION,
  PROP_LIGHTNESS,
  PROP_OVERLAP
};


static void     picman_hue_saturation_config_iface_init   (PicmanConfigInterface *iface);

static void     picman_hue_saturation_config_get_property (GObject          *object,
                                                         guint             property_id,
                                                         GValue           *value,
                                                         GParamSpec       *pspec);
static void     picman_hue_saturation_config_set_property (GObject          *object,
                                                         guint             property_id,
                                                         const GValue     *value,
                                                         GParamSpec       *pspec);

static gboolean picman_hue_saturation_config_serialize    (PicmanConfig       *config,
                                                         PicmanConfigWriter *writer,
                                                         gpointer          data);
static gboolean picman_hue_saturation_config_deserialize  (PicmanConfig       *config,
                                                         GScanner         *scanner,
                                                         gint              nest_level,
                                                         gpointer          data);
static gboolean picman_hue_saturation_config_equal        (PicmanConfig       *a,
                                                         PicmanConfig       *b);
static void     picman_hue_saturation_config_reset        (PicmanConfig       *config);
static gboolean picman_hue_saturation_config_copy         (PicmanConfig       *src,
                                                         PicmanConfig       *dest,
                                                         GParamFlags       flags);


G_DEFINE_TYPE_WITH_CODE (PicmanHueSaturationConfig, picman_hue_saturation_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_hue_saturation_config_iface_init))

#define parent_class picman_hue_saturation_config_parent_class


static void
picman_hue_saturation_config_class_init (PicmanHueSaturationConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->set_property       = picman_hue_saturation_config_set_property;
  object_class->get_property       = picman_hue_saturation_config_get_property;

  viewable_class->default_stock_id = "picman-tool-hue-saturation";

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_RANGE,
                                 "range",
                                 "The affected range",
                                 PICMAN_TYPE_HUE_RANGE,
                                 PICMAN_ALL_HUES, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_HUE,
                                   "hue",
                                   "Hue",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_SATURATION,
                                   "saturation",
                                   "Saturation",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_LIGHTNESS,
                                   "lightness",
                                   "Lightness",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_OVERLAP,
                                   "overlap",
                                   "Overlap",
                                   0.0, 1.0, 0.0, 0);
}

static void
picman_hue_saturation_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_hue_saturation_config_serialize;
  iface->deserialize = picman_hue_saturation_config_deserialize;
  iface->equal       = picman_hue_saturation_config_equal;
  iface->reset       = picman_hue_saturation_config_reset;
  iface->copy        = picman_hue_saturation_config_copy;
}

static void
picman_hue_saturation_config_init (PicmanHueSaturationConfig *self)
{
  picman_config_reset (PICMAN_CONFIG (self));
}

static void
picman_hue_saturation_config_get_property (GObject    *object,
                                         guint       property_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  PicmanHueSaturationConfig *self = PICMAN_HUE_SATURATION_CONFIG (object);

  switch (property_id)
    {
    case PROP_RANGE:
      g_value_set_enum (value, self->range);
      break;

    case PROP_HUE:
      g_value_set_double (value, self->hue[self->range]);
      break;

    case PROP_SATURATION:
      g_value_set_double (value, self->saturation[self->range]);
      break;

    case PROP_LIGHTNESS:
      g_value_set_double (value, self->lightness[self->range]);
      break;

    case PROP_OVERLAP:
      g_value_set_double (value, self->overlap);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_hue_saturation_config_set_property (GObject      *object,
                                         guint         property_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  PicmanHueSaturationConfig *self = PICMAN_HUE_SATURATION_CONFIG (object);

  switch (property_id)
    {
    case PROP_RANGE:
      self->range = g_value_get_enum (value);
      g_object_notify (object, "hue");
      g_object_notify (object, "saturation");
      g_object_notify (object, "lightness");
      break;

    case PROP_HUE:
      self->hue[self->range] = g_value_get_double (value);
      break;

    case PROP_SATURATION:
      self->saturation[self->range] = g_value_get_double (value);
      break;

    case PROP_LIGHTNESS:
      self->lightness[self->range] = g_value_get_double (value);
      break;

    case PROP_OVERLAP:
      self->overlap = g_value_get_double (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_hue_saturation_config_serialize (PicmanConfig       *config,
                                      PicmanConfigWriter *writer,
                                      gpointer          data)
{
  PicmanHueSaturationConfig *hs_config = PICMAN_HUE_SATURATION_CONFIG (config);
  PicmanHueRange             range;
  PicmanHueRange             old_range;
  gboolean                 success = TRUE;

  if (! picman_config_serialize_property_by_name (config, "time", writer))
    return FALSE;

  old_range = hs_config->range;

  for (range = PICMAN_ALL_HUES; range <= PICMAN_MAGENTA_HUES; range++)
    {
      hs_config->range = range;

      success = (picman_config_serialize_property_by_name (config, "range",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config, "hue",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config, "saturation",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config, "lightness",
                                                         writer));

      if (! success)
        break;
    }

  if (success)
    success = picman_config_serialize_property_by_name (config, "overlap",
                                                      writer);

  hs_config->range = old_range;

  return success;
}

static gboolean
picman_hue_saturation_config_deserialize (PicmanConfig *config,
                                        GScanner   *scanner,
                                        gint        nest_level,
                                        gpointer    data)
{
  PicmanHueSaturationConfig *hs_config = PICMAN_HUE_SATURATION_CONFIG (config);
  PicmanHueRange             old_range;
  gboolean                 success = TRUE;

  old_range = hs_config->range;

  success = picman_config_deserialize_properties (config, scanner, nest_level);

  g_object_set (config, "range", old_range, NULL);

  return success;
}

static gboolean
picman_hue_saturation_config_equal (PicmanConfig *a,
                                  PicmanConfig *b)
{
  PicmanHueSaturationConfig *config_a = PICMAN_HUE_SATURATION_CONFIG (a);
  PicmanHueSaturationConfig *config_b = PICMAN_HUE_SATURATION_CONFIG (b);
  PicmanHueRange             range;

  for (range = PICMAN_ALL_HUES; range <= PICMAN_MAGENTA_HUES; range++)
    {
      if (config_a->hue[range]        != config_b->hue[range]        ||
          config_a->saturation[range] != config_b->saturation[range] ||
          config_a->lightness[range]  != config_b->lightness[range])
        return FALSE;
    }

  /* don't compare "range" */

  if (config_a->overlap != config_b->overlap)
    return FALSE;

  return TRUE;
}

static void
picman_hue_saturation_config_reset (PicmanConfig *config)
{
  PicmanHueSaturationConfig *hs_config = PICMAN_HUE_SATURATION_CONFIG (config);
  PicmanHueRange             range;

  for (range = PICMAN_ALL_HUES; range <= PICMAN_MAGENTA_HUES; range++)
    {
      hs_config->range = range;
      picman_hue_saturation_config_reset_range (hs_config);
    }

  picman_config_reset_property (G_OBJECT (config), "range");
  picman_config_reset_property (G_OBJECT (config), "overlap");
}

static gboolean
picman_hue_saturation_config_copy (PicmanConfig   *src,
                                 PicmanConfig   *dest,
                                 GParamFlags   flags)
{
  PicmanHueSaturationConfig *src_config  = PICMAN_HUE_SATURATION_CONFIG (src);
  PicmanHueSaturationConfig *dest_config = PICMAN_HUE_SATURATION_CONFIG (dest);
  PicmanHueRange             range;

  for (range = PICMAN_ALL_HUES; range <= PICMAN_MAGENTA_HUES; range++)
    {
      dest_config->hue[range]        = src_config->hue[range];
      dest_config->saturation[range] = src_config->saturation[range];
      dest_config->lightness[range]  = src_config->lightness[range];
    }

  g_object_notify (G_OBJECT (dest), "hue");
  g_object_notify (G_OBJECT (dest), "saturation");
  g_object_notify (G_OBJECT (dest), "lightness");

  dest_config->range   = src_config->range;
  dest_config->overlap = src_config->overlap;

  g_object_notify (G_OBJECT (dest), "range");
  g_object_notify (G_OBJECT (dest), "overlap");

  return TRUE;
}


/*  public functions  */

void
picman_hue_saturation_config_reset_range (PicmanHueSaturationConfig *config)
{
  g_return_if_fail (PICMAN_IS_HUE_SATURATION_CONFIG (config));

  g_object_freeze_notify (G_OBJECT (config));

  picman_config_reset_property (G_OBJECT (config), "hue");
  picman_config_reset_property (G_OBJECT (config), "saturation");
  picman_config_reset_property (G_OBJECT (config), "lightness");

  g_object_thaw_notify (G_OBJECT (config));
}
