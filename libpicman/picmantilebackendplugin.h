/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantilebackendtilemanager.h
 * Copyright (C) 2011 Øyvind Kolås <pippin@picman.org>
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

#ifndef __PICMAN_TILE_BACKEND_PLUGIN_H__
#define __PICMAN_TILE_BACKEND_PLUGIN_H__

#include <gegl-buffer-backend.h>

G_BEGIN_DECLS

#define PICMAN_TYPE_TILE_BACKEND_PLUGIN            (_picman_tile_backend_plugin_get_type ())
#define PICMAN_TILE_BACKEND_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TILE_BACKEND_PLUGIN, PicmanTileBackendPlugin))
#define PICMAN_TILE_BACKEND_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_TILE_BACKEND_PLUGIN, PicmanTileBackendPluginClass))
#define PICMAN_IS_TILE_BACKEND_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TILE_BACKEND_PLUGIN))
#define PICMAN_IS_TILE_BACKEND_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_TILE_BACKEND_PLUGIN))
#define PICMAN_TILE_BACKEND_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_TILE_BACKEND_PLUGIN, PicmanTileBackendPluginClass))


typedef struct _PicmanTileBackendPlugin        PicmanTileBackendPlugin;
typedef struct _PicmanTileBackendPluginClass   PicmanTileBackendPluginClass;
typedef struct _PicmanTileBackendPluginPrivate PicmanTileBackendPluginPrivate;

struct _PicmanTileBackendPlugin
{
  GeglTileBackend  parent_instance;

  PicmanTileBackendPluginPrivate *priv;
};

struct _PicmanTileBackendPluginClass
{
  GeglTileBackendClass parent_class;
};

GType             _picman_tile_backend_plugin_get_type (void) G_GNUC_CONST;

GeglTileBackend * _picman_tile_backend_plugin_new      (PicmanDrawable *drawable,
                                                      gint          shadow);

G_END_DECLS

#endif /* __PICMAN_TILE_BACKEND_plugin_H__ */
