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

#include "core-types.h"

#include "gegl/picman-babl.h"
#include "gegl/picman-gegl-utils.h"
#include "gegl/picmantilehandlerprojection.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmanarea.h"
#include "picmanimage.h"
#include "picmanmarshal.h"
#include "picmanpickable.h"
#include "picmanprojectable.h"
#include "picmanprojection.h"


/*  halfway between G_PRIORITY_HIGH_IDLE and G_PRIORITY_DEFAULT_IDLE  */
#define PICMAN_PROJECTION_IDLE_PRIORITY \
        ((G_PRIORITY_HIGH_IDLE + G_PRIORITY_DEFAULT_IDLE) / 2)


enum
{
  UPDATE,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void   picman_projection_pickable_iface_init (PicmanPickableInterface  *iface);

static void        picman_projection_finalize              (GObject         *object);

static gint64      picman_projection_get_memsize           (PicmanObject      *object,
                                                          gint64          *gui_size);

static void        picman_projection_pickable_flush        (PicmanPickable    *pickable);
static PicmanImage * picman_projection_get_image             (PicmanPickable    *pickable);
static const Babl * picman_projection_get_format           (PicmanPickable    *pickable);
static GeglBuffer * picman_projection_get_buffer           (PicmanPickable    *pickable);
static gboolean    picman_projection_get_pixel_at          (PicmanPickable    *pickable,
                                                          gint             x,
                                                          gint             y,
                                                          const Babl      *format,
                                                          gpointer         pixel);
static gdouble     picman_projection_get_opacity_at        (PicmanPickable    *pickable,
                                                          gint             x,
                                                          gint             y);

static void        picman_projection_free_buffer           (PicmanProjection  *proj);
static void        picman_projection_add_update_area       (PicmanProjection  *proj,
                                                          gint             x,
                                                          gint             y,
                                                          gint             w,
                                                          gint             h);
static void        picman_projection_flush_whenever        (PicmanProjection  *proj,
                                                          gboolean         now);
static void        picman_projection_idle_render_init      (PicmanProjection  *proj);
static gboolean    picman_projection_idle_render_callback  (gpointer         data);
static gboolean    picman_projection_idle_render_next_area (PicmanProjection  *proj);
static void        picman_projection_paint_area            (PicmanProjection  *proj,
                                                          gboolean         now,
                                                          gint             x,
                                                          gint             y,
                                                          gint             w,
                                                          gint             h);
static void        picman_projection_invalidate            (PicmanProjection  *proj,
                                                          guint            x,
                                                          guint            y,
                                                          guint            w,
                                                          guint            h);

static void        picman_projection_projectable_invalidate(PicmanProjectable *projectable,
                                                          gint             x,
                                                          gint             y,
                                                          gint             w,
                                                          gint             h,
                                                          PicmanProjection  *proj);
static void        picman_projection_projectable_flush     (PicmanProjectable *projectable,
                                                          gboolean         invalidate_preview,
                                                          PicmanProjection  *proj);
static void        picman_projection_projectable_changed   (PicmanProjectable *projectable,
                                                          PicmanProjection  *proj);


G_DEFINE_TYPE_WITH_CODE (PicmanProjection, picman_projection, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_projection_pickable_iface_init))

#define parent_class picman_projection_parent_class

static guint projection_signals[LAST_SIGNAL] = { 0 };


static void
picman_projection_class_init (PicmanProjectionClass *klass)
{
  GObjectClass    *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass *picman_object_class = PICMAN_OBJECT_CLASS (klass);

  projection_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanProjectionClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__BOOLEAN_INT_INT_INT_INT,
                  G_TYPE_NONE, 5,
                  G_TYPE_BOOLEAN,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT);

  object_class->finalize         = picman_projection_finalize;

  picman_object_class->get_memsize = picman_projection_get_memsize;
}

static void
picman_projection_init (PicmanProjection *proj)
{
}

static void
picman_projection_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->flush                 = picman_projection_pickable_flush;
  iface->get_image             = picman_projection_get_image;
  iface->get_format            = picman_projection_get_format;
  iface->get_format_with_alpha = picman_projection_get_format; /* sic */
  iface->get_buffer            = picman_projection_get_buffer;
  iface->get_pixel_at          = picman_projection_get_pixel_at;
  iface->get_opacity_at        = picman_projection_get_opacity_at;
}

static void
picman_projection_finalize (GObject *object)
{
  PicmanProjection *proj = PICMAN_PROJECTION (object);

  if (proj->idle_render.idle_id)
    {
      g_source_remove (proj->idle_render.idle_id);
      proj->idle_render.idle_id = 0;
    }

  picman_area_list_free (proj->update_areas);
  proj->update_areas = NULL;

  picman_area_list_free (proj->idle_render.update_areas);
  proj->idle_render.update_areas = NULL;

  picman_projection_free_buffer (proj);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_projection_get_memsize (PicmanObject *object,
                             gint64     *gui_size)
{
  PicmanProjection *projection = PICMAN_PROJECTION (object);
  gint64          memsize    = 0;

  memsize += picman_gegl_buffer_get_memsize (projection->buffer);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

/**
 * picman_projection_estimate_memsize:
 * @type:      the projectable's base type
 * @precision: the projectable's precision
 * @width:     projection width
 * @height:    projection height
 *
 * Calculates a rough estimate of the memory that is required for the
 * projection of an image with the given @width and @height.
 *
 * Return value: a rough estimate of the memory requirements.
 **/
gint64
picman_projection_estimate_memsize (PicmanImageBaseType type,
                                  PicmanPrecision     precision,
                                  gint              width,
                                  gint              height)
{
  const Babl *format;
  gint64      bytes;

  if (type == PICMAN_INDEXED)
    type = PICMAN_RGB;

  format = picman_babl_format (type, precision, TRUE);
  bytes  = babl_format_get_bytes_per_pixel (format);

  /* The pyramid levels constitute a geometric sum with a ratio of 1/4. */
  return bytes * (gint64) width * (gint64) height * 1.33;
}


static void
picman_projection_pickable_flush (PicmanPickable *pickable)
{
  PicmanProjection *proj = PICMAN_PROJECTION (pickable);

  /* create the buffer if it doesn't exist */
  picman_projection_get_buffer (pickable);

  picman_projection_finish_draw (proj);
  picman_projection_flush_now (proj);

  if (proj->invalidate_preview)
    {
      /* invalidate the preview here since it is constructed from
       * the projection
       */
      proj->invalidate_preview = FALSE;

      picman_projectable_invalidate_preview (proj->projectable);
    }
}

static PicmanImage *
picman_projection_get_image (PicmanPickable *pickable)
{
  PicmanProjection *proj = PICMAN_PROJECTION (pickable);

  return picman_projectable_get_image (proj->projectable);
}

static const Babl *
picman_projection_get_format (PicmanPickable *pickable)
{
  PicmanProjection *proj = PICMAN_PROJECTION (pickable);

  return picman_projectable_get_format (proj->projectable);
}

static GeglBuffer *
picman_projection_get_buffer (PicmanPickable *pickable)
{
  PicmanProjection *proj = PICMAN_PROJECTION (pickable);

  if (! proj->buffer)
    {
      GeglNode   *graph;
      const Babl *format;
      gint        width;
      gint        height;

      graph = picman_projectable_get_graph (proj->projectable);
      format = picman_projection_get_format (PICMAN_PICKABLE (proj));
      picman_projectable_get_size (proj->projectable, &width, &height);

      proj->buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                      format);

      proj->validate_handler = picman_tile_handler_projection_new (graph,
                                                                 width, height);
      gegl_buffer_add_handler (proj->buffer, proj->validate_handler);

      /*  This used to call picman_tile_handler_projection_invalidate()
       *  which forced the entire projection to be constructed in one
       *  go for new images, causing a potentially huge delay. Now we
       *  initially validate stuff the normal way, which makes the
       *  image appear incrementally, but it keeps everything
       *  responsive.
       */
      picman_projection_add_update_area (proj, 0, 0, width, height);
      proj->invalidate_preview = TRUE;
      picman_projection_flush (proj);
    }

  return proj->buffer;
}

static gboolean
picman_projection_get_pixel_at (PicmanPickable *pickable,
                              gint          x,
                              gint          y,
                              const Babl   *format,
                              gpointer      pixel)
{
  GeglBuffer *buffer = picman_projection_get_buffer (pickable);

  if (x <  0                               ||
      y <  0                               ||
      x >= gegl_buffer_get_width  (buffer) ||
      y >= gegl_buffer_get_height (buffer))
    return FALSE;

  gegl_buffer_sample (buffer, x, y, NULL, pixel, format,
                      GEGL_SAMPLER_NEAREST, GEGL_ABYSS_NONE);

  return TRUE;
}

static gdouble
picman_projection_get_opacity_at (PicmanPickable *pickable,
                                gint          x,
                                gint          y)
{
  return PICMAN_OPACITY_OPAQUE;
}

PicmanProjection *
picman_projection_new (PicmanProjectable *projectable)
{
  PicmanProjection *proj;

  g_return_val_if_fail (PICMAN_IS_PROJECTABLE (projectable), NULL);

  proj = g_object_new (PICMAN_TYPE_PROJECTION, NULL);

  proj->projectable = projectable;

  g_signal_connect_object (projectable, "invalidate",
                           G_CALLBACK (picman_projection_projectable_invalidate),
                           proj, 0);
  g_signal_connect_object (projectable, "flush",
                           G_CALLBACK (picman_projection_projectable_flush),
                           proj, 0);
  g_signal_connect_object (projectable, "structure-changed",
                           G_CALLBACK (picman_projection_projectable_changed),
                           proj, 0);

  return proj;
}

void
picman_projection_flush (PicmanProjection *proj)
{
  g_return_if_fail (PICMAN_IS_PROJECTION (proj));

  /* Construct on idle time */
  picman_projection_flush_whenever (proj, FALSE);
}

void
picman_projection_flush_now (PicmanProjection *proj)
{
  g_return_if_fail (PICMAN_IS_PROJECTION (proj));

  /* Construct NOW */
  picman_projection_flush_whenever (proj, TRUE);
}

void
picman_projection_finish_draw (PicmanProjection *proj)
{
  g_return_if_fail (PICMAN_IS_PROJECTION (proj));

  if (proj->idle_render.idle_id)
    {
      g_source_remove (proj->idle_render.idle_id);
      proj->idle_render.idle_id = 0;

      while (picman_projection_idle_render_callback (proj));
    }
}


/*  private functions  */

static void
picman_projection_free_buffer (PicmanProjection  *proj)
{
  if (proj->buffer)
    {
      if (proj->validate_handler)
        gegl_buffer_remove_handler (proj->buffer, proj->validate_handler);

      g_object_unref (proj->buffer);
      proj->buffer = NULL;
    }

  if (proj->validate_handler)
    {
      g_object_unref (proj->validate_handler);
      proj->validate_handler = NULL;
    }
}

static void
picman_projection_add_update_area (PicmanProjection *proj,
                                 gint            x,
                                 gint            y,
                                 gint            w,
                                 gint            h)
{
  PicmanArea *area;
  gint      off_x, off_y;
  gint      width, height;

  picman_projectable_get_offset (proj->projectable, &off_x, &off_y);
  picman_projectable_get_size   (proj->projectable, &width, &height);

  /*  subtract the projectable's offsets because the list of update
   *  areas is in tile-pyramid coordinates, but our external API is
   *  always in terms of image coordinates.
   */
  x -= off_x;
  y -= off_y;

  area = picman_area_new (CLAMP (x,     0, width),
                        CLAMP (y,     0, height),
                        CLAMP (x + w, 0, width),
                        CLAMP (y + h, 0, height));

  proj->update_areas = picman_area_list_process (proj->update_areas, area);
}

static void
picman_projection_flush_whenever (PicmanProjection *proj,
                                gboolean        now)
{
  /*  First the updates...  */
  if (proj->update_areas)
    {
      if (now)  /* Synchronous */
        {
          GSList *list;

          for (list = proj->update_areas; list; list = g_slist_next (list))
            {
              PicmanArea *area = list->data;

              if ((area->x1 != area->x2) && (area->y1 != area->y2))
                {
                  picman_projection_paint_area (proj,
                                              FALSE, /* sic! */
                                              area->x1,
                                              area->y1,
                                              (area->x2 - area->x1),
                                              (area->y2 - area->y1));
                }
            }
        }
      else  /* Asynchronous */
        {
          picman_projection_idle_render_init (proj);
        }

      /*  Free the update lists  */
      picman_area_list_free (proj->update_areas);
      proj->update_areas = NULL;
    }
  else if (! now && proj->invalidate_preview)
    {
      /* invalidate the preview here since it is constructed from
       * the projection
       */
      proj->invalidate_preview = FALSE;

      picman_projectable_invalidate_preview (proj->projectable);
    }
}

static void
picman_projection_idle_render_init (PicmanProjection *proj)
{
  GSList *list;

  /* We need to merge the IdleRender's and the PicmanProjection's update_areas
   * list to keep track of which of the updates have been flushed and hence
   * need to be drawn.
   */
  for (list = proj->update_areas; list; list = g_slist_next (list))
    {
      PicmanArea *area = list->data;

      proj->idle_render.update_areas =
        picman_area_list_process (proj->idle_render.update_areas,
                                picman_area_new (area->x1, area->y1,
                                               area->x2, area->y2));
    }

  /* If an idlerender was already running, merge the remainder of its
   * unrendered area with the update_areas list, and make it start work
   * on the next unrendered area in the list.
   */
  if (proj->idle_render.idle_id)
    {
      PicmanArea *area =
        picman_area_new (proj->idle_render.base_x,
                       proj->idle_render.y,
                       proj->idle_render.base_x + proj->idle_render.width,
                       proj->idle_render.y + (proj->idle_render.height -
                                               (proj->idle_render.y -
                                                proj->idle_render.base_y)));

      proj->idle_render.update_areas =
        picman_area_list_process (proj->idle_render.update_areas, area);

      picman_projection_idle_render_next_area (proj);
    }
  else
    {
      if (proj->idle_render.update_areas == NULL)
        {
          g_warning ("%s: wanted to start idle render with no update_areas",
                     G_STRFUNC);
          return;
        }

      picman_projection_idle_render_next_area (proj);

      proj->idle_render.idle_id =
        g_idle_add_full (PICMAN_PROJECTION_IDLE_PRIORITY,
                         picman_projection_idle_render_callback, proj,
                         NULL);
    }
}

/* Unless specified otherwise, projection re-rendering is organised by
 * IdleRender, which amalgamates areas to be re-rendered and breaks
 * them into bite-sized chunks which are chewed on in a low- priority
 * idle thread.  This greatly improves responsiveness for many PICMAN
 * operations.  -- Adam
 */
static gboolean
picman_projection_idle_render_callback (gpointer data)
{
  PicmanProjection *proj = data;
  gint            workx, worky;
  gint            workw, workh;

#define CHUNK_WIDTH  256
#define CHUNK_HEIGHT 128

  workw = CHUNK_WIDTH;
  workh = CHUNK_HEIGHT;
  workx = proj->idle_render.x;
  worky = proj->idle_render.y;

  if (workx + workw > proj->idle_render.base_x + proj->idle_render.width)
    {
      workw = proj->idle_render.base_x + proj->idle_render.width - workx;
    }

  if (worky + workh > proj->idle_render.base_y + proj->idle_render.height)
    {
      workh = proj->idle_render.base_y + proj->idle_render.height - worky;
    }

  picman_projection_paint_area (proj, TRUE /* sic! */,
                              workx, worky, workw, workh);

  proj->idle_render.x += CHUNK_WIDTH;

  if (proj->idle_render.x >=
      proj->idle_render.base_x + proj->idle_render.width)
    {
      proj->idle_render.x = proj->idle_render.base_x;
      proj->idle_render.y += CHUNK_HEIGHT;

      if (proj->idle_render.y >=
          proj->idle_render.base_y + proj->idle_render.height)
        {
          if (! picman_projection_idle_render_next_area (proj))
            {
              /* FINISHED */
              proj->idle_render.idle_id = 0;

              if (proj->invalidate_preview)
                {
                  /* invalidate the preview here since it is constructed from
                   * the projection
                   */
                  proj->invalidate_preview = FALSE;

                  picman_projectable_invalidate_preview (proj->projectable);
                }

              return FALSE;
            }
        }
    }

  /* Still work to do. */
  return TRUE;
}

static gboolean
picman_projection_idle_render_next_area (PicmanProjection *proj)
{
  PicmanArea *area;

  if (! proj->idle_render.update_areas)
    return FALSE;

  area = proj->idle_render.update_areas->data;

  proj->idle_render.update_areas =
    g_slist_remove (proj->idle_render.update_areas, area);

  proj->idle_render.x      = proj->idle_render.base_x = area->x1;
  proj->idle_render.y      = proj->idle_render.base_y = area->y1;
  proj->idle_render.width  = area->x2 - area->x1;
  proj->idle_render.height = area->y2 - area->y1;

  picman_area_free (area);

  return TRUE;
}

static void
picman_projection_paint_area (PicmanProjection *proj,
                            gboolean        now,
                            gint            x,
                            gint            y,
                            gint            w,
                            gint            h)
{
  gint off_x, off_y;
  gint width, height;
  gint x1, y1, x2, y2;

  picman_projectable_get_offset (proj->projectable, &off_x, &off_y);
  picman_projectable_get_size   (proj->projectable, &width, &height);

  /*  Bounds check  */
  x1 = CLAMP (x,     0, width);
  y1 = CLAMP (y,     0, height);
  x2 = CLAMP (x + w, 0, width);
  y2 = CLAMP (y + h, 0, height);

  picman_projection_invalidate (proj, x1, y1, x2 - x1, y2 - y1);

  /*  add the projectable's offsets because the list of update areas
   *  is in tile-pyramid coordinates, but our external API is always
   *  in terms of image coordinates.
   */
  g_signal_emit (proj, projection_signals[UPDATE], 0,
                 now,
                 x1 + off_x,
                 y1 + off_y,
                 x2 - x1,
                 y2 - y1);
}

static void
picman_projection_invalidate (PicmanProjection *proj,
                            guint           x,
                            guint           y,
                            guint           w,
                            guint           h)
{
  if (proj->validate_handler)
    picman_tile_handler_projection_invalidate (proj->validate_handler,
                                             x, y, w, h);
}


/*  image callbacks  */

static void
picman_projection_projectable_invalidate (PicmanProjectable *projectable,
                                        gint             x,
                                        gint             y,
                                        gint             w,
                                        gint             h,
                                        PicmanProjection  *proj)
{
  picman_projection_add_update_area (proj, x, y, w, h);
}

static void
picman_projection_projectable_flush (PicmanProjectable *projectable,
                                   gboolean         invalidate_preview,
                                   PicmanProjection  *proj)
{
  if (invalidate_preview)
    proj->invalidate_preview = TRUE;

  picman_projection_flush (proj);
}

static void
picman_projection_projectable_changed (PicmanProjectable *projectable,
                                     PicmanProjection  *proj)
{
  gint off_x, off_y;
  gint width, height;

  if (proj->idle_render.idle_id)
    {
      g_source_remove (proj->idle_render.idle_id);
      proj->idle_render.idle_id = 0;
    }

  picman_area_list_free (proj->update_areas);
  proj->update_areas = NULL;

  picman_projection_free_buffer (proj);

  picman_projectable_get_offset (proj->projectable, &off_x, &off_y);
  picman_projectable_get_size (projectable, &width, &height);

  picman_projection_add_update_area (proj, off_x, off_y, width, height);
}
