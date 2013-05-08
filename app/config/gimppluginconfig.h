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

#ifndef __PICMAN_PLUGIN_CONFIG_H__
#define __PICMAN_PLUGIN_CONFIG_H__

#include "config/picmanguiconfig.h"


#define PICMAN_TYPE_PLUGIN_CONFIG            (picman_plugin_config_get_type ())
#define PICMAN_PLUGIN_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PLUGIN_CONFIG, PicmanPluginConfig))
#define PICMAN_PLUGIN_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PLUGIN_CONFIG, PicmanPluginConfigClass))
#define PICMAN_IS_PLUGIN_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PLUGIN_CONFIG))
#define PICMAN_IS_PLUGIN_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PLUGIN_CONFIG))


typedef struct _PicmanPluginConfigClass PicmanPluginConfigClass;

struct _PicmanPluginConfig
{
  PicmanGuiConfig       parent_instance;

  gchar              *fractalexplorer_path;
  gchar              *gfig_path;
  gchar              *gflare_path;
  gchar              *picmanressionist_path;
  gchar              *script_fu_path;
};

struct _PicmanPluginConfigClass
{
  PicmanGuiConfigClass  parent_class;
};


GType  picman_plugin_config_get_type (void) G_GNUC_CONST;


#endif /* PICMAN_PLUGIN_CONFIG_H__ */
