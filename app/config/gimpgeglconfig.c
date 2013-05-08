/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGeglConfig class
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"

#include "core/core-types.h" /* eek */

#include "picmanrc-blurbs.h"
#include "picmangeglconfig.h"

#include "core/picman-utils.h"

#include "picman-debug.h"

#include "picman-intl.h"


#define PICMAN_MAX_NUM_THREADS 16
#define PICMAN_MAX_MEM_PROCESS (MIN (G_MAXSIZE, PICMAN_MAX_MEMSIZE))

enum
{
  PROP_0,
  PROP_TEMP_PATH,
  PROP_SWAP_PATH,
  PROP_NUM_PROCESSORS,
  PROP_TILE_CACHE_SIZE,

  /* ignored, only for backward compatibility: */
  PROP_STINGY_MEMORY_USE
};


static void   picman_gegl_config_class_init   (PicmanGeglConfigClass *klass);
static void   picman_gegl_config_init         (PicmanGeglConfig      *config,
                                             PicmanGeglConfigClass *klass);
static void   picman_gegl_config_finalize     (GObject             *object);
static void   picman_gegl_config_set_property (GObject             *object,
                                             guint                property_id,
                                             const GValue        *value,
                                             GParamSpec          *pspec);
static void   picman_gegl_config_get_property (GObject             *object,
                                             guint                property_id,
                                             GValue              *value,
                                             GParamSpec          *pspec);


static GObjectClass *parent_class = NULL;


GType
picman_gegl_config_get_type (void)
{
  static GType config_type = 0;

  if (! config_type)
    {
      const GTypeInfo config_info =
      {
        sizeof (PicmanGeglConfigClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) picman_gegl_config_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (PicmanGeglConfig),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) picman_gegl_config_init,
      };

      config_type = g_type_register_static (G_TYPE_OBJECT,
                                            "PicmanGeglConfig",
                                            &config_info, 0);
    }

  return config_type;
}

static void
picman_gegl_config_class_init (PicmanGeglConfigClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  gint          num_processors;
  guint64       memory_size;

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = picman_gegl_config_finalize;
  object_class->set_property = picman_gegl_config_set_property;
  object_class->get_property = picman_gegl_config_get_property;

  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class, PROP_TEMP_PATH,
                                 "temp-path", TEMP_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR,
                                 "${picman_dir}" G_DIR_SEPARATOR_S "tmp",
                                 PICMAN_PARAM_STATIC_STRINGS |
                                 PICMAN_CONFIG_PARAM_RESTART);
  PICMAN_CONFIG_INSTALL_PROP_PATH (object_class, PROP_SWAP_PATH,
                                 "swap-path", SWAP_PATH_BLURB,
                                 PICMAN_CONFIG_PATH_DIR,
                                 "${picman_dir}",
                                 PICMAN_PARAM_STATIC_STRINGS |
                                 PICMAN_CONFIG_PARAM_RESTART);

  num_processors = picman_get_number_of_processors ();

#ifdef PICMAN_UNSTABLE
  num_processors = num_processors * 2;
#endif

  num_processors = MIN (num_processors, PICMAN_MAX_NUM_THREADS);

  PICMAN_CONFIG_INSTALL_PROP_UINT (object_class, PROP_NUM_PROCESSORS,
                                 "num-processors", NUM_PROCESSORS_BLURB,
                                 1, PICMAN_MAX_NUM_THREADS, num_processors,
                                 PICMAN_PARAM_STATIC_STRINGS);

  memory_size = picman_get_physical_memory_size ();

  /* limit to the amount one process can handle */
  memory_size = MIN (PICMAN_MAX_MEM_PROCESS, memory_size);

  if (memory_size > 0)
    memory_size = memory_size / 2; /* half the memory */
  else
    memory_size = 1 << 30; /* 1GB */

  PICMAN_CONFIG_INSTALL_PROP_MEMSIZE (object_class, PROP_TILE_CACHE_SIZE,
                                    "tile-cache-size", TILE_CACHE_SIZE_BLURB,
                                    0, PICMAN_MAX_MEM_PROCESS,
                                    memory_size,
                                    PICMAN_PARAM_STATIC_STRINGS |
                                    PICMAN_CONFIG_PARAM_CONFIRM);

  /*  only for backward compatibility:  */
  PICMAN_CONFIG_INSTALL_PROP_BOOLEAN (object_class, PROP_STINGY_MEMORY_USE,
                                    "stingy-memory-use", NULL,
                                    FALSE,
                                    PICMAN_CONFIG_PARAM_IGNORE);
}

static void
picman_gegl_config_init (PicmanGeglConfig      *config,
                       PicmanGeglConfigClass *klass)
{
  picman_debug_add_instance (G_OBJECT (config), G_OBJECT_CLASS (klass));
}

static void
picman_gegl_config_finalize (GObject *object)
{
  PicmanGeglConfig *gegl_config = PICMAN_GEGL_CONFIG (object);

  g_free (gegl_config->temp_path);
  g_free (gegl_config->swap_path);

  picman_debug_remove_instance (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_gegl_config_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanGeglConfig *gegl_config = PICMAN_GEGL_CONFIG (object);

  switch (property_id)
    {
    case PROP_TEMP_PATH:
      g_free (gegl_config->temp_path);
      gegl_config->temp_path = g_value_dup_string (value);
      break;
    case PROP_SWAP_PATH:
      g_free (gegl_config->swap_path);
      gegl_config->swap_path = g_value_dup_string (value);
      break;
    case PROP_NUM_PROCESSORS:
      gegl_config->num_processors = g_value_get_uint (value);
      break;
    case PROP_TILE_CACHE_SIZE:
      gegl_config->tile_cache_size = g_value_get_uint64 (value);
      break;

    case PROP_STINGY_MEMORY_USE:
      /* ignored */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_gegl_config_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanGeglConfig *gegl_config = PICMAN_GEGL_CONFIG (object);

  switch (property_id)
    {
    case PROP_TEMP_PATH:
      g_value_set_string (value, gegl_config->temp_path);
      break;
    case PROP_SWAP_PATH:
      g_value_set_string (value, gegl_config->swap_path);
      break;
    case PROP_NUM_PROCESSORS:
      g_value_set_uint (value, gegl_config->num_processors);
      break;
    case PROP_TILE_CACHE_SIZE:
      g_value_set_uint64 (value, gegl_config->tile_cache_size);
      break;

    case PROP_STINGY_MEMORY_USE:
      /* ignored */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
