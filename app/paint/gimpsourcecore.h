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

#ifndef __PICMAN_SOURCE_CORE_H__
#define __PICMAN_SOURCE_CORE_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_SOURCE_CORE            (picman_source_core_get_type ())
#define PICMAN_SOURCE_CORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SOURCE_CORE, PicmanSourceCore))
#define PICMAN_SOURCE_CORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SOURCE_CORE, PicmanSourceCoreClass))
#define PICMAN_IS_SOURCE_CORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SOURCE_CORE))
#define PICMAN_IS_SOURCE_CORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SOURCE_CORE))
#define PICMAN_SOURCE_CORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SOURCE_CORE, PicmanSourceCoreClass))


typedef struct _PicmanSourceCoreClass PicmanSourceCoreClass;

struct _PicmanSourceCore
{
  PicmanBrushCore  parent_instance;

  gboolean       set_source;

  PicmanDrawable  *src_drawable;
  gdouble        src_x;
  gdouble        src_y;

  gdouble        orig_src_x;
  gdouble        orig_src_y;

  gdouble        offset_x;
  gdouble        offset_y;
  gboolean       first_stroke;
};

struct _PicmanSourceCoreClass
{
  PicmanBrushCoreClass  parent_class;

  gboolean     (* use_source) (PicmanSourceCore    *source_core,
                               PicmanSourceOptions *options);

  GeglBuffer * (* get_source) (PicmanSourceCore    *source_core,
                               PicmanDrawable      *drawable,
                               PicmanPaintOptions  *paint_options,
                               PicmanPickable      *src_pickable,
                               gint               src_offset_x,
                               gint               src_offset_y,
                               GeglBuffer        *paint_buffer,
                               gint               paint_buffer_x,
                               gint               paint_buffer_y,
                               /* offsets *into* the paint_buffer: */
                               gint              *paint_area_offset_x,
                               gint              *paint_area_offset_y,
                               gint              *paint_area_width,
                               gint              *paint_area_height,
                               GeglRectangle     *src_rect);

  void         (*  motion)    (PicmanSourceCore    *source_core,
                               PicmanDrawable      *drawable,
                               PicmanPaintOptions  *paint_options,
                               const PicmanCoords  *coords,
                               gdouble            opacity,
                               PicmanPickable      *src_pickable,
                               GeglBuffer        *src_buffer,
                               GeglRectangle     *src_rect,
                               gint               src_offset_x,
                               gint               src_offset_y,
                               GeglBuffer        *paint_buffer,
                               gint               paint_buffer_x,
                               gint               paint_buffer_y,
                               /* offsets *into* the paint_buffer: */
                               gint               paint_area_offset_x,
                               gint               paint_area_offset_y,
                               gint               paint_area_width,
                               gint               paint_area_height);
};


GType    picman_source_core_get_type   (void) G_GNUC_CONST;

gboolean picman_source_core_use_source (PicmanSourceCore    *source_core,
                                      PicmanSourceOptions *options);

/* TEMP HACK */
void     picman_source_core_motion     (PicmanSourceCore    *source_core,
                                      PicmanDrawable      *drawable,
                                      PicmanPaintOptions  *paint_options,
                                      const PicmanCoords  *coords);


#endif  /*  __PICMAN_SOURCE_CORE_H__  */
