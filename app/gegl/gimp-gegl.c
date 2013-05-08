/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-gegl.c
 * Copyright (C) 2007 Øyvind Kolås <pippin@picman.org>
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

#include "picman-gegl-types.h"

#include "config/picmangeglconfig.h"

#include "operations/picman-operations.h"

#include "core/picman.h"

#include "picman-babl.h"
#include "picman-gegl.h"


static void  picman_gegl_notify_tile_cache_size (PicmanGeglConfig *config);
static void  picman_gegl_notify_num_processors  (PicmanGeglConfig *config);


void
picman_gegl_init (Picman *picman)
{
  PicmanGeglConfig *config;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  config = PICMAN_GEGL_CONFIG (picman->config);

#ifdef __GNUC__
#warning not setting GeglConfig:threads
#endif

  if (g_object_class_find_property (G_OBJECT_GET_CLASS (gegl_config ()),
                                    "tile-cache-size"))
    {
      g_object_set (gegl_config (),
                    "tile-cache-size", (guint64) config->tile_cache_size,
#if 0
                    "threads",    config->num_processors,
#endif
                    NULL);
    }
  else
    {
#ifdef __GNUC__
#warning limiting tile cache size to G_MAXINT
#endif

      g_object_set (gegl_config (),
                    "cache-size", (gint) MIN (config->tile_cache_size, G_MAXINT),
#if 0
                    "threads",    config->num_processors,
#endif
                    NULL);
    }

  /* turn down the precision of babl - permitting use of lookup tables for
   * gamma conversions, this precision is anyways high enough for both 8bit
   * and 16bit operation
   */
  g_object_set (gegl_config (),
                "babl-tolerance", 0.00015,
                NULL);

  g_signal_connect (config, "notify::tile-cache-size",
                    G_CALLBACK (picman_gegl_notify_tile_cache_size),
                    NULL);
  g_signal_connect (config, "notify::num-processors",
                    G_CALLBACK (picman_gegl_notify_num_processors),
                    NULL);

  picman_babl_init ();

  picman_operations_init ();
}

static void
picman_gegl_notify_tile_cache_size (PicmanGeglConfig *config)
{
  if (g_object_class_find_property (G_OBJECT_GET_CLASS (gegl_config ()),
                                    "tile-cache-size"))
    {
      g_object_set (gegl_config (),
                    "tile-cache-size", (guint64) config->tile_cache_size,
                    NULL);
    }
  else
    {
      g_object_set (gegl_config (),
                    "cache-size", (gint) MIN (config->tile_cache_size, G_MAXINT),
                    NULL);
    }
}

static void
picman_gegl_notify_num_processors (PicmanGeglConfig *config)
{
#if 0
  g_object_set (gegl_config (),
                "threads", config->num_processors,
                NULL);
#endif
}
