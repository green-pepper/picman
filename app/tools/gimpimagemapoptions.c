/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others
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
#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"

#include "tools-types.h"

#include "picmanimagemapoptions.h"


enum
{
  PROP_0,
  PROP_PREVIEW,
  PROP_SETTINGS
};


static void   picman_image_map_options_finalize     (GObject      *object);
static void   picman_image_map_options_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);
static void   picman_image_map_options_get_property (GObject      *object,
                                                   guint         property_id,
                                                   GValue       *value,
                                                   GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanImageMapOptions, picman_image_map_options,
               PICMAN_TYPE_TOOL_OPTIONS)

#define parent_class picman_image_map_options_parent_class


static void
picman_image_map_options_class_init (PicmanImageMapOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_image_map_options_finalize;
  object_class->set_property = picman_image_map_options_set_property;
  object_class->get_property = picman_image_map_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_PREVIEW,
                                    "preview", NULL,
                                    TRUE,
                                    PICMAN_PARAM_STATIC_STRINGS);
  g_object_class_install_property (object_class, PROP_SETTINGS,
                                   g_param_spec_string ("settings",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_image_map_options_init (PicmanImageMapOptions *options)
{
}

static void
picman_image_map_options_finalize (GObject *object)
{
  PicmanImageMapOptions *options = PICMAN_IMAGE_MAP_OPTIONS (object);

  if (options->settings)
    {
      g_free (options->settings);
      options->settings = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
picman_image_map_options_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanImageMapOptions *options = PICMAN_IMAGE_MAP_OPTIONS (object);

  switch (property_id)
    {
    case PROP_PREVIEW:
      options->preview = g_value_get_boolean (value);
      break;
    case PROP_SETTINGS:
      g_free (options->settings);
      options->settings = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_map_options_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanImageMapOptions *options = PICMAN_IMAGE_MAP_OPTIONS (object);

  switch (property_id)
    {
    case PROP_PREVIEW:
      g_value_set_boolean (value, options->preview);
      break;
    case PROP_SETTINGS:
      g_value_set_string (value, options->settings);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
