/* picmantilebackendtilemanager.c
 * Copyright (C) 2012 Øyvind Kolås <pippin@picman.org>
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"
#include "picmantilebackendplugin.h"


#define TILE_WIDTH  picman_tile_width()
#define TILE_HEIGHT picman_tile_width()


struct _PicmanTileBackendPluginPrivate
{
  PicmanDrawable *drawable;
  gboolean      shadow;
  gint          mul;
};


static gint
picman_gegl_tile_mul (void)
{
  static gint     mul    = 2;
  static gboolean inited = FALSE;

  if (G_LIKELY (inited))
    return mul;

  inited = TRUE;

  if (g_getenv ("PICMAN_GEGL_TILE_MUL"))
    mul = atoi (g_getenv ("PICMAN_GEGL_TILE_MUL"));

  if (mul < 1)
    mul = 1;

  return mul;
}

static void     picman_tile_backend_plugin_finalize (GObject         *object);
static gpointer picman_tile_backend_plugin_command  (GeglTileSource  *tile_store,
                                                   GeglTileCommand  command,
                                                   gint             x,
                                                   gint             y,
                                                   gint             z,
                                                   gpointer         data);

static void       picman_tile_write_mul (PicmanTileBackendPlugin *backend_plugin,
                                       gint                   x,
                                       gint                   y,
                                       guchar                *source);

static GeglTile * picman_tile_read_mul (PicmanTileBackendPlugin *backend_plugin,
                                      gint                   x,
                                      gint                   y);


G_DEFINE_TYPE (PicmanTileBackendPlugin, _picman_tile_backend_plugin,
               GEGL_TYPE_TILE_BACKEND)

#define parent_class _picman_tile_backend_plugin_parent_class


static void
_picman_tile_backend_plugin_class_init (PicmanTileBackendPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = picman_tile_backend_plugin_finalize;

  g_type_class_add_private (klass, sizeof (PicmanTileBackendPluginPrivate));

  picman_tile_cache_ntiles (64);
}

static void
_picman_tile_backend_plugin_init (PicmanTileBackendPlugin *backend)
{
  GeglTileSource *source = GEGL_TILE_SOURCE (backend);

  backend->priv = G_TYPE_INSTANCE_GET_PRIVATE (backend,
                                               PICMAN_TYPE_TILE_BACKEND_PLUGIN,
                                               PicmanTileBackendPluginPrivate);

  source->command = picman_tile_backend_plugin_command;
}

static void
picman_tile_backend_plugin_finalize (GObject *object)
{
  PicmanTileBackendPlugin *backend = PICMAN_TILE_BACKEND_PLUGIN (object);

  if (backend->priv->drawable) /* This also causes a flush */
    picman_drawable_detach (backend->priv->drawable);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gpointer
picman_tile_backend_plugin_command (GeglTileSource  *tile_store,
                                  GeglTileCommand  command,
                                  gint             x,
                                  gint             y,
                                  gint             z,
                                  gpointer         data)
{
  PicmanTileBackendPlugin *backend_plugin = PICMAN_TILE_BACKEND_PLUGIN (tile_store);

  switch (command)
    {
    case GEGL_TILE_GET:
      return picman_tile_read_mul (backend_plugin, x, y);

    case GEGL_TILE_SET:
      picman_tile_write_mul (backend_plugin, x, y, gegl_tile_get_data (data));
      gegl_tile_mark_as_stored (data);
      break;

    case GEGL_TILE_FLUSH:
      picman_drawable_flush (backend_plugin->priv->drawable);
      break;

    default:
      g_assert (command < GEGL_TILE_LAST_COMMAND && command >= 0);
    }

  return NULL;
}

static GeglTile *
picman_tile_read_mul (PicmanTileBackendPlugin *backend_plugin,
                    gint                   x,
                    gint                   y)
{
  PicmanTileBackendPluginPrivate *priv    = backend_plugin->priv;
  GeglTileBackend              *backend = GEGL_TILE_BACKEND (backend_plugin);
  GeglTile                     *tile;
  gint                          tile_size;
  gint                          u, v;
  gint                          mul = priv->mul;
  guchar                       *tile_data;

  x *= mul;
  y *= mul;

  tile_size  = gegl_tile_backend_get_tile_size (backend);
  tile       = gegl_tile_new (tile_size);
  tile_data  = gegl_tile_get_data (tile);

  for (u = 0; u < mul; u++)
    {
      for (v = 0; v < mul; v++)
        {
          PicmanTile *picman_tile;

          if (x + u >= priv->drawable->ntile_cols ||
              y + v >= priv->drawable->ntile_rows)
            continue;

          picman_tile = picman_drawable_get_tile (priv->drawable,
                                              priv->shadow,
                                              y + v, x + u);
          picman_tile_ref (picman_tile);

          {
            gint ewidth           = picman_tile->ewidth;
            gint eheight          = picman_tile->eheight;
            gint bpp              = picman_tile->bpp;
            gint tile_stride      = mul * TILE_WIDTH * bpp;
            gint picman_tile_stride = ewidth * bpp;
            gint row;

            for (row = 0; row < eheight; row++)
              {
                memcpy (tile_data + (row + TILE_HEIGHT * v) *
                        tile_stride + u * TILE_WIDTH * bpp,
                        ((gchar *) picman_tile->data) + row * picman_tile_stride,
                        picman_tile_stride);
              }
          }

          picman_tile_unref (picman_tile, FALSE);
        }
    }

  return tile;
}

static void
picman_tile_write_mul (PicmanTileBackendPlugin *backend_plugin,
                     gint                   x,
                     gint                   y,
                     guchar                *source)
{
  PicmanTileBackendPluginPrivate *priv = backend_plugin->priv;
  gint                          u, v;
  gint                          mul = priv->mul;

  x *= mul;
  y *= mul;

  for (v = 0; v < mul; v++)
    {
      for (u = 0; u < mul; u++)
        {
          PicmanTile *picman_tile;

          if (x + u >= priv->drawable->ntile_cols ||
              y + v >= priv->drawable->ntile_rows)
            continue;

          picman_tile = picman_drawable_get_tile (priv->drawable,
                                              priv->shadow,
                                              y+v, x+u);
          picman_tile_ref (picman_tile);

          {
            gint ewidth           = picman_tile->ewidth;
            gint eheight          = picman_tile->eheight;
            gint bpp              = picman_tile->bpp;
            gint tile_stride      = mul * TILE_WIDTH * bpp;
            gint picman_tile_stride = ewidth * bpp;
            gint row;

            for (row = 0; row < eheight; row++)
              memcpy (((gchar *)picman_tile->data) + row * picman_tile_stride,
                      source + (row + v * TILE_HEIGHT) *
                      tile_stride + u * TILE_WIDTH * bpp,
                      picman_tile_stride);
          }

          picman_tile_unref (picman_tile, TRUE);
        }
    }
}

GeglTileBackend *
_picman_tile_backend_plugin_new (PicmanDrawable *drawable,
                               gint          shadow)
{
  GeglTileBackend       *backend;
  PicmanTileBackendPlugin *backend_plugin;
  const Babl            *format;
  gint                   width  = picman_drawable_width (drawable->drawable_id);
  gint                   height = picman_drawable_height (drawable->drawable_id);
  gint                   mul    = picman_gegl_tile_mul ();

  format = picman_drawable_get_format (drawable->drawable_id);

  backend = g_object_new (PICMAN_TYPE_TILE_BACKEND_PLUGIN,
                          "tile-width",  TILE_WIDTH  * mul,
                          "tile-height", TILE_HEIGHT * mul,
                          "format",      format,
                          NULL);

  backend_plugin = PICMAN_TILE_BACKEND_PLUGIN (backend);

  backend_plugin->priv->drawable = drawable;
  backend_plugin->priv->mul      = mul;
  backend_plugin->priv->shadow   = shadow;

  gegl_tile_backend_set_extent (backend,
                                GEGL_RECTANGLE (0, 0, width, height));

  return backend;
}
