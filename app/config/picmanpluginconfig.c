/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanPluginConfig class
 * Copyright (C) 2001  Sven Neumann <sven@picman.org>
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

#include <glib-object.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "config-types.h"

#include "picmanrc-blurbs.h"
#include "picmanpluginconfig.h"


enum
{
  PROP_0,
  PROP_FRACTALEXPLORER_PATH,
  PROP_GFIG_PATH,
  PROP_GFLARE_PATH,
  PROP_PICMANRESSIONIST_PATH,
  PROP_SCRIPT_FU_PATH
};


static void  picman_plugin_config_finalize     (GObject      *object);
static void  picman_plugin_config_set_property (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec);
static void  picman_plugin_config_get_property (GObject      *object,
                                              guint         property_id,
                                              GValue       *value,
                                              GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanPluginConfig, picman_plugin_config, PICMAN_TYPE_GUI_CONFIG)

#define parent_class picman_plugin_config_parent_class


static void
picman_plugin_config_class_init (PicmanPluginConfigClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  gchar        *path;

  object_class->finalize     = picman_plugin_config_finalize;
  object_class->set_property = picman_plugin_config_set_property;
  object_class->get_property = picman_plugin_config_get_property;

  path = picman_config_build_data_path ("fractalexplorer");
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class,
                                 PROP_FRACTALEXPLORER_PATH,
                                 "fractalexplorer-path",
                                 FRACTALEXPLORER_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR_LIST, path,
                                 PICMAN_PARAM_STATIC_STRINGS);
  g_free (path);

  path = picman_config_build_data_path ("gfig");
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class,
                                 PROP_GFIG_PATH,
                                 "gfig-path", GFIG_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR_LIST, path,
                                 PICMAN_PARAM_STATIC_STRINGS);
  g_free (path);

  path = picman_config_build_data_path ("gflare");
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class,
                                 PROP_GFLARE_PATH,
                                 "gflare-path", GFLARE_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR_LIST, path,
                                 PICMAN_PARAM_STATIC_STRINGS);
  g_free (path);

  path = picman_config_build_data_path ("picmanressionist");
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class,
                                 PROP_PICMANRESSIONIST_PATH,
                                 "picmanressionist-path",
                                 PICMANRESSIONIST_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR_LIST, path,
                                 PICMAN_PARAM_STATIC_STRINGS);
  g_free (path);

  path = picman_config_build_data_path ("scripts");
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class,
                                 PROP_SCRIPT_FU_PATH,
                                 "script-fu-path",
                                 SCRIPT_FU_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR_LIST, path,
                                 PICMAN_PARAM_STATIC_STRINGS);
  g_free (path);
}

static void
picman_plugin_config_init (PicmanPluginConfig *config)
{
}

static void
picman_plugin_config_finalize (GObject *object)
{
  PicmanPluginConfig *plugin_config = PICMAN_PLUGIN_CONFIG (object);

  g_free (plugin_config->fractalexplorer_path);
  g_free (plugin_config->gfig_path);
  g_free (plugin_config->gflare_path);
  g_free (plugin_config->picmanressionist_path);
  g_free (plugin_config->script_fu_path);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_plugin_config_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanPluginConfig *plugin_config = PICMAN_PLUGIN_CONFIG (object);

  switch (property_id)
    {
    case PROP_FRACTALEXPLORER_PATH:
      g_free (plugin_config->fractalexplorer_path);
      plugin_config->fractalexplorer_path = g_value_dup_string (value);
      break;

    case PROP_GFIG_PATH:
      g_free (plugin_config->gfig_path);
      plugin_config->gfig_path = g_value_dup_string (value);
      break;

    case PROP_GFLARE_PATH:
      g_free (plugin_config->gflare_path);
      plugin_config->gflare_path = g_value_dup_string (value);
      break;

    case PROP_PICMANRESSIONIST_PATH:
      g_free (plugin_config->picmanressionist_path);
      plugin_config->picmanressionist_path = g_value_dup_string (value);
      break;

    case PROP_SCRIPT_FU_PATH:
      g_free (plugin_config->script_fu_path);
      plugin_config->script_fu_path = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_plugin_config_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanPluginConfig *plugin_config = PICMAN_PLUGIN_CONFIG (object);

  switch (property_id)
    {
    case PROP_FRACTALEXPLORER_PATH:
      g_value_set_string (value, plugin_config->fractalexplorer_path);
      break;

    case PROP_GFIG_PATH:
      g_value_set_string (value, plugin_config->gfig_path);
      break;

    case PROP_GFLARE_PATH:
      g_value_set_string (value, plugin_config->gflare_path);
      break;

    case PROP_PICMANRESSIONIST_PATH:
      g_value_set_string (value, plugin_config->picmanressionist_path);
      break;

    case PROP_SCRIPT_FU_PATH:
      g_value_set_string (value, plugin_config->script_fu_path);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
