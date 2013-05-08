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

#include <string.h>

#include <gegl.h>

#include "libpicmanconfig/picmanconfig.h"

#include "paint-types.h"

#include "picmancloneoptions.h"


enum
{
  PROP_0,
  PROP_CLONE_TYPE
};


static void   picman_clone_options_set_property (GObject      *object,
                                               guint         property_id,
                                               const GValue *value,
                                               GParamSpec   *pspec);
static void   picman_clone_options_get_property (GObject      *object,
                                               guint         property_id,
                                               GValue       *value,
                                               GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanCloneOptions, picman_clone_options, PICMAN_TYPE_SOURCE_OPTIONS)

#define parent_class picman_clone_options_parent_class


static void
picman_clone_options_class_init (PicmanCloneOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = picman_clone_options_set_property;
  object_class->get_property = picman_clone_options_get_property;

  PICMAN_CONFIG_INSTALL_PROP_ENUM (object_class, PROP_CLONE_TYPE,
                                 "clone-type", NULL,
                                 PICMAN_TYPE_CLONE_TYPE,
                                 PICMAN_IMAGE_CLONE,
                                 PICMAN_PARAM_STATIC_STRINGS);
}

static void
picman_clone_options_init (PicmanCloneOptions *options)
{
}

static void
picman_clone_options_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanCloneOptions *options = PICMAN_CLONE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CLONE_TYPE:
      options->clone_type = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_clone_options_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanCloneOptions *options = PICMAN_CLONE_OPTIONS (object);

  switch (property_id)
    {
    case PROP_CLONE_TYPE:
      g_value_set_enum (value, options->clone_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
