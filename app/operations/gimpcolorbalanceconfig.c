/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancolorbalanceconfig.c
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
#include "libpicmanmath/picmanmath.h"
#include "libpicmanconfig/picmanconfig.h"

#include "operations-types.h"

#include "picmancolorbalanceconfig.h"


enum
{
  PROP_0,
  PROP_RANGE,
  PROP_CYAN_RED,
  PROP_MAGENTA_GREEN,
  PROP_YELLOW_BLUE,
  PROP_PRESERVE_LUMINOSITY
};


static void     picman_color_balance_config_iface_init   (PicmanConfigInterface *iface);

static void     picman_color_balance_config_get_property (GObject          *object,
                                                        guint             property_id,
                                                        GValue           *value,
                                                        GParamSpec       *pspec);
static void     picman_color_balance_config_set_property (GObject          *object,
                                                        guint             property_id,
                                                        const GValue     *value,
                                                        GParamSpec       *pspec);

static gboolean picman_color_balance_config_serialize    (PicmanConfig       *config,
                                                        PicmanConfigWriter *writer,
                                                        gpointer          data);
static gboolean picman_color_balance_config_deserialize  (PicmanConfig       *config,
                                                        GScanner         *scanner,
                                                        gint              nest_level,
                                                        gpointer          data);
static gboolean picman_color_balance_config_equal        (PicmanConfig       *a,
                                                        PicmanConfig       *b);
static void     picman_color_balance_config_reset        (PicmanConfig       *config);
static gboolean picman_color_balance_config_copy         (PicmanConfig       *src,
                                                        PicmanConfig       *dest,
                                                        GParamFlags       flags);


G_DEFINE_TYPE_WITH_CODE (PicmanColorBalanceConfig, picman_color_balance_config,
                         PICMAN_TYPE_IMAGE_MAP_CONFIG,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG,
                                                picman_color_balance_config_iface_init))

#define parent_class picman_color_balance_config_parent_class


static void
picman_color_balance_config_class_init (PicmanColorBalanceConfigClass *klass)
{
  GObjectClass      *object_class   = G_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);

  object_class->set_property       = picman_color_balance_config_set_property;
  object_class->get_property       = picman_color_balance_config_get_property;

  viewable_class->default_stock_id = "picman-tool-color-balance";

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_RANGE,
                                 "range",
                                 "The affected range",
                                 PICMAN_TYPE_TRANSFER_MODE,
                                 PICMAN_MIDTONES, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_CYAN_RED,
                                   "cyan-red",
                                   "Cyan-Red",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_MAGENTA_GREEN,
                                   "magenta-green",
                                   "Magenta-Green",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_YELLOW_BLUE,
                                   "yellow-blue",
                                   "Yellow-Blue",
                                   -1.0, 1.0, 0.0, 0);

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_PRESERVE_LUMINOSITY,
                                    "preserve-luminosity",
                                    "Preserve Luminosity",
                                    TRUE, 0);
}

static void
picman_color_balance_config_iface_init (PicmanConfigInterface *iface)
{
  iface->serialize   = picman_color_balance_config_serialize;
  iface->deserialize = picman_color_balance_config_deserialize;
  iface->equal       = picman_color_balance_config_equal;
  iface->reset       = picman_color_balance_config_reset;
  iface->copy        = picman_color_balance_config_copy;
}

static void
picman_color_balance_config_init (PicmanColorBalanceConfig *self)
{
  picman_config_reset (PICMAN_CONFIG (self));
}

static void
picman_color_balance_config_get_property (GObject    *object,
                                        guint       property_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  PicmanColorBalanceConfig *self = PICMAN_COLOR_BALANCE_CONFIG (object);

  switch (property_id)
    {
    case PROP_RANGE:
      g_value_set_enum (value, self->range);
      break;

    case PROP_CYAN_RED:
      g_value_set_double (value, self->cyan_red[self->range]);
      break;

    case PROP_MAGENTA_GREEN:
      g_value_set_double (value, self->magenta_green[self->range]);
      break;

    case PROP_YELLOW_BLUE:
      g_value_set_double (value, self->yellow_blue[self->range]);
      break;

    case PROP_PRESERVE_LUMINOSITY:
      g_value_set_boolean (value, self->preserve_luminosity);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_balance_config_set_property (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  PicmanColorBalanceConfig *self = PICMAN_COLOR_BALANCE_CONFIG (object);

  switch (property_id)
    {
    case PROP_RANGE:
      self->range = g_value_get_enum (value);
      g_object_notify (object, "cyan-red");
      g_object_notify (object, "magenta-green");
      g_object_notify (object, "yellow-blue");
      break;

    case PROP_CYAN_RED:
      self->cyan_red[self->range] = g_value_get_double (value);
      break;

    case PROP_MAGENTA_GREEN:
      self->magenta_green[self->range] = g_value_get_double (value);
      break;

    case PROP_YELLOW_BLUE:
      self->yellow_blue[self->range] = g_value_get_double (value);
      break;

    case PROP_PRESERVE_LUMINOSITY:
      self->preserve_luminosity = g_value_get_boolean (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_color_balance_config_serialize (PicmanConfig       *config,
                                     PicmanConfigWriter *writer,
                                     gpointer          data)
{
  PicmanColorBalanceConfig *bc_config = PICMAN_COLOR_BALANCE_CONFIG (config);
  PicmanTransferMode        range;
  PicmanTransferMode        old_range;
  gboolean                success = TRUE;

  if (! picman_config_serialize_property_by_name (config, "time", writer))
    return FALSE;

  old_range = bc_config->range;

  for (range = PICMAN_SHADOWS; range <= PICMAN_HIGHLIGHTS; range++)
    {
      bc_config->range = range;

      success = (picman_config_serialize_property_by_name (config,
                                                         "range",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config,
                                                         "cyan-red",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config,
                                                         "magenta-green",
                                                         writer) &&
                 picman_config_serialize_property_by_name (config,
                                                         "yellow-blue",
                                                         writer));

      if (! success)
        break;
    }

  if (success)
    success = picman_config_serialize_property_by_name (config,
                                                      "preserve-luminosity",
                                                      writer);

  bc_config->range = old_range;

  return success;
}

static gboolean
picman_color_balance_config_deserialize (PicmanConfig *config,
                                       GScanner   *scanner,
                                       gint        nest_level,
                                       gpointer    data)
{
  PicmanColorBalanceConfig *cb_config = PICMAN_COLOR_BALANCE_CONFIG (config);
  PicmanTransferMode        old_range;
  gboolean                success = TRUE;

  old_range = cb_config->range;

  success = picman_config_deserialize_properties (config, scanner, nest_level);

  g_object_set (config, "range", old_range, NULL);

  return success;
}

static gboolean
picman_color_balance_config_equal (PicmanConfig *a,
                                 PicmanConfig *b)
{
  PicmanColorBalanceConfig *config_a = PICMAN_COLOR_BALANCE_CONFIG (a);
  PicmanColorBalanceConfig *config_b = PICMAN_COLOR_BALANCE_CONFIG (b);
  PicmanTransferMode        range;

  for (range = PICMAN_SHADOWS; range <= PICMAN_HIGHLIGHTS; range++)
    {
      if (config_a->cyan_red[range]      != config_b->cyan_red[range]      ||
          config_a->magenta_green[range] != config_b->magenta_green[range] ||
          config_a->yellow_blue[range]   != config_b->yellow_blue[range])
        return FALSE;
    }

  /* don't compare "range" */

  if (config_a->preserve_luminosity != config_b->preserve_luminosity)
    return FALSE;

  return TRUE;
}

static void
picman_color_balance_config_reset (PicmanConfig *config)
{
  PicmanColorBalanceConfig *cb_config = PICMAN_COLOR_BALANCE_CONFIG (config);
  PicmanTransferMode        range;

  for (range = PICMAN_SHADOWS; range <= PICMAN_HIGHLIGHTS; range++)
    {
      cb_config->range = range;
      picman_color_balance_config_reset_range (cb_config);
    }

  picman_config_reset_property (G_OBJECT (config), "range");
  picman_config_reset_property (G_OBJECT (config), "preserve-luminosity");
}

static gboolean
picman_color_balance_config_copy (PicmanConfig  *src,
                                PicmanConfig  *dest,
                                GParamFlags  flags)
{
  PicmanColorBalanceConfig *src_config  = PICMAN_COLOR_BALANCE_CONFIG (src);
  PicmanColorBalanceConfig *dest_config = PICMAN_COLOR_BALANCE_CONFIG (dest);
  PicmanTransferMode        range;

  for (range = PICMAN_SHADOWS; range <= PICMAN_HIGHLIGHTS; range++)
    {
      dest_config->cyan_red[range]      = src_config->cyan_red[range];
      dest_config->magenta_green[range] = src_config->magenta_green[range];
      dest_config->yellow_blue[range]   = src_config->yellow_blue[range];
    }

  g_object_notify (G_OBJECT (dest), "cyan-red");
  g_object_notify (G_OBJECT (dest), "magenta-green");
  g_object_notify (G_OBJECT (dest), "yellow-blue");

  dest_config->range               = src_config->range;
  dest_config->preserve_luminosity = src_config->preserve_luminosity;

  g_object_notify (G_OBJECT (dest), "range");
  g_object_notify (G_OBJECT (dest), "preserve-luminosity");

  return TRUE;
}


/*  public functions  */

void
picman_color_balance_config_reset_range (PicmanColorBalanceConfig *config)
{
  g_return_if_fail (PICMAN_IS_COLOR_BALANCE_CONFIG (config));

  g_object_freeze_notify (G_OBJECT (config));

  picman_config_reset_property (G_OBJECT (config), "cyan-red");
  picman_config_reset_property (G_OBJECT (config), "magenta-green");
  picman_config_reset_property (G_OBJECT (config), "yellow-blue");

  g_object_thaw_notify (G_OBJECT (config));
}
