/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushcache.c
 * Copyright (C) 2011 Michael Natterer <mitch@picman.org>
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

#include "core-types.h"

#include "picmanbrushcache.h"

#include "picman-log.h"
#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_DATA_DESTROY
};


static void   picman_brush_cache_constructed  (GObject      *object);
static void   picman_brush_cache_finalize     (GObject      *object);
static void   picman_brush_cache_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void   picman_brush_cache_get_property (GObject      *object,
                                             guint         property_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);


G_DEFINE_TYPE (PicmanBrushCache, picman_brush_cache, PICMAN_TYPE_OBJECT)

#define parent_class picman_brush_cache_parent_class


static void
picman_brush_cache_class_init (PicmanBrushCacheClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_brush_cache_constructed;
  object_class->finalize     = picman_brush_cache_finalize;
  object_class->set_property = picman_brush_cache_set_property;
  object_class->get_property = picman_brush_cache_get_property;

  g_object_class_install_property (object_class, PROP_DATA_DESTROY,
                                   g_param_spec_pointer ("data-destroy",
                                                         NULL, NULL,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_brush_cache_init (PicmanBrushCache *brush)
{
}

static void
picman_brush_cache_constructed (GObject *object)
{
  PicmanBrushCache *cache = PICMAN_BRUSH_CACHE (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (cache->data_destroy != NULL);
}

static void
picman_brush_cache_finalize (GObject *object)
{
  PicmanBrushCache *cache = PICMAN_BRUSH_CACHE (object);

  if (cache->last_data)
    {
      cache->data_destroy (cache->last_data);
      cache->last_data = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_brush_cache_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanBrushCache *cache = PICMAN_BRUSH_CACHE (object);

  switch (property_id)
    {
    case PROP_DATA_DESTROY:
      cache->data_destroy = g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_brush_cache_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanBrushCache *cache = PICMAN_BRUSH_CACHE (object);

  switch (property_id)
    {
    case PROP_DATA_DESTROY:
      g_value_set_pointer (value, cache->data_destroy);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

PicmanBrushCache *
picman_brush_cache_new (GDestroyNotify  data_destroy,
                      gchar           debug_hit,
                      gchar           debug_miss)
{
  PicmanBrushCache *cache;

  g_return_val_if_fail (data_destroy != NULL, NULL);

  cache =  g_object_new (PICMAN_TYPE_BRUSH_CACHE,
                         "data-destroy", data_destroy,
                         NULL);

  cache->debug_hit  = debug_hit;
  cache->debug_miss = debug_miss;

  return cache;
}

void
picman_brush_cache_clear (PicmanBrushCache *cache)
{
  g_return_if_fail (PICMAN_IS_BRUSH_CACHE (cache));

  if (cache->last_data)
    {
      cache->data_destroy (cache->last_data);
      cache->last_data = NULL;
    }
}

gconstpointer
picman_brush_cache_get (PicmanBrushCache *cache,
                      gint            width,
                      gint            height,
                      gdouble         scale,
                      gdouble         aspect_ratio,
                      gdouble         angle,
                      gdouble         hardness)
{
  g_return_val_if_fail (PICMAN_IS_BRUSH_CACHE (cache), NULL);

  if (cache->last_data                         &&
      cache->last_width        == width        &&
      cache->last_height       == height       &&
      cache->last_scale        == scale        &&
      cache->last_aspect_ratio == aspect_ratio &&
      cache->last_angle        == angle        &&
      cache->last_hardness     == hardness)
    {
      if (picman_log_flags & PICMAN_LOG_BRUSH_CACHE)
        g_printerr ("%c", cache->debug_hit);

      return (gconstpointer) cache->last_data;
    }

  if (picman_log_flags & PICMAN_LOG_BRUSH_CACHE)
    g_printerr ("%c", cache->debug_miss);

  return NULL;
}

void
picman_brush_cache_add (PicmanBrushCache *cache,
                      gpointer        data,
                      gint            width,
                      gint            height,
                      gdouble         scale,
                      gdouble         aspect_ratio,
                      gdouble         angle,
                      gdouble         hardness)
{
  g_return_if_fail (PICMAN_IS_BRUSH_CACHE (cache));
  g_return_if_fail (data != NULL);

  if (data == cache->last_data)
    return;

  if (cache->last_data)
    cache->data_destroy (cache->last_data);

  cache->last_data         = data;
  cache->last_width        = width;
  cache->last_height       = height;
  cache->last_scale        = scale;
  cache->last_aspect_ratio = aspect_ratio;
  cache->last_angle        = angle;
  cache->last_hardness     = hardness;
}
