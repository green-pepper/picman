/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "libpicmanconfig/picmanconfig.h"

#include "paint-types.h"

#include "picmandodgeburnoptions.h"


#define DODGE_BURN_DEFAULT_TYPE     PICMAN_DODGE
#define DODGE_BURN_DEFAULT_MODE     PICMAN_MIDTONES
#define DODGE_BURN_DEFAULT_EXPOSURE 50.0


enum
{
  PROP_0,
  PROP_TYPE,
  PROP_MODE,
  PROP_EXPOSURE
};


static void   picman_dodge_burn_options_set_property (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void   picman_dodge_burn_options_get_property (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanDodgeBurnOptions, picman_dodge_burn_options,
               PICMAN_TYPE_PAINT_OPTIONS)


static void
picman_dodge_burn_options_class_init (PicmanDodgeBurnOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_dodge_burn_options_set_property;
  object_class->get_property = picman_dodge_burn_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_TYPE,
                                 "type", NULL,
                                 PICMAN_TYPE_DODGE_BURN_TYPE,
                                 DODGE_BURN_DEFAULT_TYPE,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_MODE,
                                 "mode", NULL,
                                 PICMAN_TYPE_TRANSFER_MODE,
                                 DODGE_BURN_DEFAULT_MODE,
                                 PICMAN_PARAM_STATIC_STRINGS);
  PICMAN_CONFIG_INSTALL_PROP_DOUBLE (object_class, PROP_EXPOSURE,
                                   "exposure", NULL,
                                   0.0, 100.0, DODGE_BURN_DEFAULT_EXPOSURE,
                                   PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_dodge_burn_options_init (PicmanDodgeBurnOptions *options)
{
}

static void
picman_dodge_burn_options_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  PicmanDodgeBurnOptions *options = PICMAN_DODGE_BURN_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TYPE:
      options->type = g_value_get_enum (value);
      break;
    case PROP_MODE:
      options->mode = g_value_get_enum (value);
      break;
    case PROP_EXPOSURE:
      options->exposure = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dodge_burn_options_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  PicmanDodgeBurnOptions *options = PICMAN_DODGE_BURN_OPTIONS (object);

  switch (property_id)
    {
    case PROP_TYPE:
      g_value_set_enum (value, options->type);
      break;
    case PROP_MODE:
      g_value_set_enum (value, options->mode);
      break;
    case PROP_EXPOSURE:
      g_value_set_double (value, options->exposure);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
