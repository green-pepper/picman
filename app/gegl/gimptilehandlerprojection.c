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

#include <cairo.h>
#include <gegl.h>

#include "picman-gegl-types.h"

#include "picmantilehandlerprojection.h"


enum
{
  PROP_0,
  PROP_FORMAT,
  PROP_TILE_WIDTH,
  PROP_TILE_HEIGHT
};


static void     picman_tile_handler_projection_finalize     (GObject         *object);
static void     picman_tile_handler_projection_set_property (GObject         *object,
                                                           guint            property_id,
                                                           const GValue    *value,
                                                           GParamSpec      *pspec);
static void     picman_tile_handler_projection_get_property (GObject         *object,
                                                           guint            property_id,
                                                           GValue          *value,
                                                           GParamSpec      *pspec);

static gpointer picman_tile_handler_projection_command      (GeglTileSource  *source,
                                                           GeglTileCommand  command,
                                                           gint             x,
                                                           gint             y,
                                                           gint             z,
                                                           gpointer         data);

static void     picman_tile_handler_projection_update_max_z (PicmanTileHandlerProjection *projection);


G_DEFINE_TYPE (PicmanTileHandlerProjection, picman_tile_handler_projection,
               GEGL_TYPE_TILE_HANDLER)

#define parent_class picman_tile_handler_projection_parent_class


static void
picman_tile_handler_projection_class_init (PicmanTileHandlerProjectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_tile_handler_projection_finalize;
  object_class->set_property = picman_tile_handler_projection_set_property;
  object_class->get_property = picman_tile_handler_projection_get_property;

  g_object_class_install_property (object_class, PROP_FORMAT,
                                   g_param_spec_pointer ("format", NULL, NULL,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_TILE_WIDTH,
                                   g_param_spec_int ("tile-width", NULL, NULL,
                                                     1, G_MAXINT, 1,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_TILE_HEIGHT,
                                   g_param_spec_int ("tile-height", NULL, NULL,
                                                     1, G_MAXINT, 1,
                                                     PICMAN_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT));
}

static void
picman_tile_handler_projection_init (PicmanTileHandlerProjection *projection)
{
  GeglTileSource *source = GEGL_TILE_SOURCE (projection);

  source->command = picman_tile_handler_projection_command;

  projection->dirty_region = cairo_region_create ();
}

static void
picman_tile_handler_projection_finalize (GObject *object)
{
  PicmanTileHandlerProjection *projection = PICMAN_TILE_HANDLER_PROJECTION (object);

  if (projection->graph)
    {
      g_object_unref (projection->graph);
      projection->graph = NULL;
    }

  cairo_region_destroy (projection->dirty_region);
  projection->dirty_region = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_tile_handler_projection_set_property (GObject      *object,
                                           guint         property_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  PicmanTileHandlerProjection *projection = PICMAN_TILE_HANDLER_PROJECTION (object);

  switch (property_id)
    {
    case PROP_FORMAT:
      projection->format = g_value_get_pointer (value);
      break;
    case PROP_TILE_WIDTH:
      projection->tile_width = g_value_get_int (value);
      picman_tile_handler_projection_update_max_z (projection);
      break;
    case PROP_TILE_HEIGHT:
      projection->tile_height = g_value_get_int (value);
      picman_tile_handler_projection_update_max_z (projection);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_tile_handler_projection_get_property (GObject    *object,
                                           guint       property_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  PicmanTileHandlerProjection *projection = PICMAN_TILE_HANDLER_PROJECTION (object);

  switch (property_id)
    {
    case PROP_FORMAT:
      g_value_set_pointer (value, (gpointer) projection->format);
      break;
    case PROP_TILE_WIDTH:
      g_value_set_int (value, projection->tile_width);
      break;
    case PROP_TILE_HEIGHT:
      g_value_set_int (value, projection->tile_height);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static GeglTile *
picman_tile_handler_projection_validate (GeglTileSource *source,
                                       GeglTile       *tile,
                                       gint            x,
                                       gint            y)
{
  PicmanTileHandlerProjection *projection;
  cairo_region_t            *tile_region;
  cairo_rectangle_int_t      tile_rect;

  projection = PICMAN_TILE_HANDLER_PROJECTION (source);

  if (cairo_region_is_empty (projection->dirty_region))
    return tile;

  tile_region = cairo_region_copy (projection->dirty_region);

  tile_rect.x      = x * projection->tile_width;
  tile_rect.y      = y * projection->tile_height;
  tile_rect.width  = projection->tile_width;
  tile_rect.height = projection->tile_height;

  cairo_region_intersect_rectangle (tile_region, &tile_rect);

  if (! cairo_region_is_empty (tile_region))
    {
      gint tile_bpp;
      gint tile_stride;
      gint n_rects;
      gint i;

      if (! tile)
        tile = gegl_tile_handler_create_tile (GEGL_TILE_HANDLER (source),
                                              x, y, 0);

      cairo_region_subtract_rectangle (projection->dirty_region, &tile_rect);

      tile_bpp    = babl_format_get_bytes_per_pixel (projection->format);
      tile_stride = tile_bpp * projection->tile_width;

      gegl_tile_lock (tile);

      n_rects = cairo_region_num_rectangles (tile_region);

#if 0
      g_printerr ("%d ", n_rects);
#endif

      for (i = 0; i < n_rects; i++)
        {
          cairo_rectangle_int_t blit_rect;

          cairo_region_get_rectangle (tile_region, i, &blit_rect);

#if 0
          g_printerr ("constructing projection at %d %d %d %d\n",
                      blit_rect.x,
                      blit_rect.y,
                      blit_rect.width,
                      blit_rect.height);
#endif

          gegl_node_blit (projection->graph, 1.0,
                          GEGL_RECTANGLE (blit_rect.x,
                                          blit_rect.y,
                                          blit_rect.width,
                                          blit_rect.height),
                          projection->format,
                          gegl_tile_get_data (tile) +
                          (blit_rect.y % projection->tile_height) * tile_stride +
                          (blit_rect.x % projection->tile_width)  * tile_bpp,
                          tile_stride,
                          GEGL_BLIT_DEFAULT);
        }

      gegl_tile_unlock (tile);
    }

  cairo_region_destroy (tile_region);

  return tile;
}

static gpointer
picman_tile_handler_projection_command (GeglTileSource  *source,
                                      GeglTileCommand  command,
                                      gint             x,
                                      gint             y,
                                      gint             z,
                                      gpointer         data)
{
  gpointer retval;

  retval = gegl_tile_handler_source_command (source, command, x, y, z, data);

  if (command == GEGL_TILE_GET && z == 0)
    retval = picman_tile_handler_projection_validate (source, retval, x, y);

  return retval;
}

static void
picman_tile_handler_projection_update_max_z (PicmanTileHandlerProjection *projection)
{
  projection->max_z = 0;

  if (projection->proj_width > 0 && projection->proj_height > 0 &&
      projection->tile_width > 0 && projection->tile_height > 0)
    {
      gint n_tiles;

      n_tiles = MAX (projection->proj_width  / projection->tile_width,
                     projection->proj_height / projection->tile_height) + 1;

      while (n_tiles >>= 1)
        projection->max_z++;
    }
}

GeglTileHandler *
picman_tile_handler_projection_new (GeglNode *graph,
                                  gint      proj_width,
                                  gint      proj_height)
{
  PicmanTileHandlerProjection *projection;

  g_return_val_if_fail (GEGL_IS_NODE (graph), NULL);

  projection = g_object_new (PICMAN_TYPE_TILE_HANDLER_PROJECTION, NULL);

  projection->graph       = g_object_ref (graph);
  projection->proj_width  = proj_width;
  projection->proj_height = proj_height;

  return GEGL_TILE_HANDLER (projection);
}

static void
picman_tile_handler_projection_void_pyramid (GeglTileSource *source,
                                           gint            x,
                                           gint            y,
                                           gint            z,
                                           gint            max_z)
{
  gegl_tile_source_void (source, x, y, z);

  if (z < max_z)
    picman_tile_handler_projection_void_pyramid (source, x / 2, y / 2, z + 1,
                                               max_z);
}

void
picman_tile_handler_projection_invalidate (PicmanTileHandlerProjection *projection,
                                         gint                       x,
                                         gint                       y,
                                         gint                       width,
                                         gint                       height)
{
  cairo_rectangle_int_t rect = { x, y, width, height };

  g_return_if_fail (PICMAN_IS_TILE_HANDLER_PROJECTION (projection));

  cairo_region_union_rectangle (projection->dirty_region, &rect);

  if (projection->max_z > 0)
    {
      gint tile_x1 = x / projection->tile_width;
      gint tile_y1 = y / projection->tile_height;
      gint tile_x2 = (x + width  - 1) / projection->tile_width;
      gint tile_y2 = (y + height - 1) / projection->tile_height;
      gint tile_x;
      gint tile_y;

      for (tile_y = tile_y1; tile_y <= tile_y2; tile_y++)
        {
          for (tile_x = tile_x1; tile_x <= tile_x2; tile_x++)
            {
              picman_tile_handler_projection_void_pyramid (GEGL_TILE_SOURCE (projection),
                                                         tile_x / 2,
                                                         tile_y / 2,
                                                         1,
                                                         projection->max_z);
            }
        }
    }
}
