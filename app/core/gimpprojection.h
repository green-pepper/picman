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

#ifndef __PICMAN_PROJECTION_H__
#define __PICMAN_PROJECTION_H__


#include "picmanobject.h"


typedef struct _PicmanProjectionIdleRender PicmanProjectionIdleRender;

struct _PicmanProjectionIdleRender
{
  gint    width;
  gint    height;
  gint    x;
  gint    y;
  gint    base_x;
  gint    base_y;
  guint   idle_id;
  GSList *update_areas;   /*  flushed update areas */
};


#define PICMAN_TYPE_PROJECTION            (picman_projection_get_type ())
#define PICMAN_PROJECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PROJECTION, PicmanProjection))
#define PICMAN_PROJECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PROJECTION, PicmanProjectionClass))
#define PICMAN_IS_PROJECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PROJECTION))
#define PICMAN_IS_PROJECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PROJECTION))
#define PICMAN_PROJECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PROJECTION, PicmanProjectionClass))



typedef struct _PicmanProjectionClass PicmanProjectionClass;

struct _PicmanProjection
{
  PicmanObject                parent_instance;

  PicmanProjectable          *projectable;

  GeglBuffer               *buffer;
  gpointer                  validate_handler;

  GSList                   *update_areas;
  PicmanProjectionIdleRender  idle_render;

  gboolean                  invalidate_preview;
};

struct _PicmanProjectionClass
{
  PicmanObjectClass  parent_class;

  void (* update) (PicmanProjection *proj,
                   gboolean        now,
                   gint            x,
                   gint            y,
                   gint            width,
                   gint            height);
};


GType            picman_projection_get_type         (void) G_GNUC_CONST;

PicmanProjection * picman_projection_new              (PicmanProjectable   *projectable);

void             picman_projection_flush            (PicmanProjection    *proj);
void             picman_projection_flush_now        (PicmanProjection    *proj);
void             picman_projection_finish_draw      (PicmanProjection    *proj);

gint64           picman_projection_estimate_memsize (PicmanImageBaseType  type,
                                                   PicmanPrecision      precision,
                                                   gint               width,
                                                   gint               height);


#endif /*  __PICMAN_PROJECTION_H__  */
