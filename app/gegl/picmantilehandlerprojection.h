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

#ifndef __PICMAN_TILE_HANDLER_PROJECTION_H__
#define __PICMAN_TILE_HANDLER_PROJECTION_H__

#include <gegl-buffer-backend.h>

/***
 * PicmanTileHandlerProjection is a GeglTileHandler that renders the
 * projection.
 */

G_BEGIN_DECLS

#define PICMAN_TYPE_TILE_HANDLER_PROJECTION            (picman_tile_handler_projection_get_type ())
#define PICMAN_TILE_HANDLER_PROJECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TILE_HANDLER_PROJECTION, PicmanTileHandlerProjection))
#define PICMAN_TILE_HANDLER_PROJECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_TILE_HANDLER_PROJECTION, PicmanTileHandlerProjectionClass))
#define PICMAN_IS_TILE_HANDLER_PROJECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TILE_HANDLER_PROJECTION))
#define PICMAN_IS_TILE_HANDLER_PROJECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_TILE_HANDLER_PROJECTION))
#define PICMAN_TILE_HANDLER_PROJECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_TILE_HANDLER_PROJECTION, PicmanTileHandlerProjectionClass))


typedef struct _PicmanTileHandlerProjection      PicmanTileHandlerProjection;
typedef struct _PicmanTileHandlerProjectionClass PicmanTileHandlerProjectionClass;

struct _PicmanTileHandlerProjection
{
  GeglTileHandler  parent_instance;

  GeglNode        *graph;
  cairo_region_t  *dirty_region;
  const Babl      *format;
  gint             tile_width;
  gint             tile_height;
  gint             proj_width;
  gint             proj_height;
  gint             max_z;
};

struct _PicmanTileHandlerProjectionClass
{
  GeglTileHandlerClass  parent_class;
};


GType             picman_tile_handler_projection_get_type   (void) G_GNUC_CONST;
GeglTileHandler * picman_tile_handler_projection_new        (GeglNode                  *graph,
                                                           gint                       proj_width,
                                                           gint                       proj_height);

void              picman_tile_handler_projection_invalidate (PicmanTileHandlerProjection *projection,
                                                           gint                       x,
                                                           gint                       y,
                                                           gint                       width,
                                                           gint                       height);


G_END_DECLS

#endif /* __PICMAN_TILE_HANDLER_PROJECTION_H__ */
