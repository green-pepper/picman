/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushcache.h
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

#ifndef __PICMAN_BRUSH_CACHE_H__
#define __PICMAN_BRUSH_CACHE_H__


#include "picmanobject.h"


#define PICMAN_TYPE_BRUSH_CACHE            (picman_brush_cache_get_type ())
#define PICMAN_BRUSH_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_CACHE, PicmanBrushCache))
#define PICMAN_BRUSH_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_CACHE, PicmanBrushCacheClass))
#define PICMAN_IS_BRUSH_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_CACHE))
#define PICMAN_IS_BRUSH_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_CACHE))
#define PICMAN_BRUSH_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_CACHE, PicmanBrushCacheClass))


typedef struct _PicmanBrushCacheClass PicmanBrushCacheClass;

struct _PicmanBrushCache
{
  PicmanObject      parent_instance;

  GDestroyNotify  data_destroy;

  gpointer        last_data;
  gint            last_width;
  gint            last_height;
  gdouble         last_scale;
  gdouble         last_aspect_ratio;
  gdouble         last_angle;
  gdouble         last_hardness;

  gchar           debug_hit;
  gchar           debug_miss;
};

struct _PicmanBrushCacheClass
{
  PicmanObjectClass  parent_class;
};


GType            picman_brush_cache_get_type (void) G_GNUC_CONST;

PicmanBrushCache * picman_brush_cache_new      (GDestroyNotify  data_destory,
                                            gchar           debug_hit,
                                            gchar           debug_miss);

void             picman_brush_cache_clear    (PicmanBrushCache *cache);

gconstpointer    picman_brush_cache_get      (PicmanBrushCache *cache,
                                            gint            width,
                                            gint            height,
                                            gdouble         scale,
                                            gdouble         aspect_ratio,
                                            gdouble         angle,
                                            gdouble         hardness);
void             picman_brush_cache_add      (PicmanBrushCache *cache,
                                            gpointer        data,
                                            gint            width,
                                            gint            height,
                                            gdouble         scale,
                                            gdouble         aspect_ratio,
                                            gdouble         angle,
                                            gdouble         hardness);


#endif  /*  __PICMAN_BRUSH_CACHE_H__  */
