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

#ifndef __PICMAN_BRUSH_CORE_H__
#define __PICMAN_BRUSH_CORE_H__


#include "picmanpaintcore.h"


#define BRUSH_CORE_SUBSAMPLE        4
#define BRUSH_CORE_SOLID_SUBSAMPLE  2
#define BRUSH_CORE_JITTER_LUTSIZE   360


#define PICMAN_TYPE_BRUSH_CORE            (picman_brush_core_get_type ())
#define PICMAN_BRUSH_CORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_CORE, PicmanBrushCore))
#define PICMAN_BRUSH_CORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_CORE, PicmanBrushCoreClass))
#define PICMAN_IS_BRUSH_CORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_CORE))
#define PICMAN_IS_BRUSH_CORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_CORE))
#define PICMAN_BRUSH_CORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_CORE, PicmanBrushCoreClass))


typedef struct _PicmanBrushCoreClass PicmanBrushCoreClass;

struct _PicmanBrushCore
{
  PicmanPaintCore      parent_instance;

  PicmanBrush         *main_brush;
  PicmanBrush         *brush;
  PicmanDynamics      *dynamics;
  gdouble            spacing;
  gdouble            scale;
  gdouble            angle;
  gdouble            hardness;
  gdouble            aspect_ratio;

  /*  brush buffers  */
  PicmanTempBuf       *pressure_brush;

  PicmanTempBuf       *solid_brushes[BRUSH_CORE_SOLID_SUBSAMPLE][BRUSH_CORE_SOLID_SUBSAMPLE];
  const PicmanTempBuf *last_solid_brush_mask;
  gboolean           solid_cache_invalid;

  const PicmanTempBuf *transform_brush;
  const PicmanTempBuf *transform_pixmap;

  PicmanTempBuf       *subsample_brushes[BRUSH_CORE_SUBSAMPLE + 1][BRUSH_CORE_SUBSAMPLE + 1];
  const PicmanTempBuf *last_subsample_brush_mask;
  gboolean           subsample_cache_invalid;

  gdouble            jitter;
  gdouble            jitter_lut_x[BRUSH_CORE_JITTER_LUTSIZE];
  gdouble            jitter_lut_y[BRUSH_CORE_JITTER_LUTSIZE];

  GRand             *rand;
};

struct _PicmanBrushCoreClass
{
  PicmanPaintCoreClass  parent_class;

  /*  Set for tools that don't mind if the brush changes while painting  */
  gboolean            handles_changing_brush;

  /*  Set for tools that don't mind if the brush scales while painting  */
  gboolean            handles_transforming_brush;

  /*  Set for tools that don't mind if the brush scales mid stroke  */
  gboolean            handles_dynamic_transforming_brush;

  void (* set_brush)    (PicmanBrushCore *core,
                         PicmanBrush     *brush);
  void (* set_dynamics) (PicmanBrushCore *core,
                         PicmanDynamics  *brush);
};


GType  picman_brush_core_get_type       (void) G_GNUC_CONST;

void   picman_brush_core_set_brush      (PicmanBrushCore            *core,
                                       PicmanBrush                *brush);

void   picman_brush_core_set_dynamics   (PicmanBrushCore            *core,
                                       PicmanDynamics             *dynamics);

void   picman_brush_core_paste_canvas   (PicmanBrushCore            *core,
                                       PicmanDrawable             *drawable,
                                       const PicmanCoords         *coords,
                                       gdouble                   brush_opacity,
                                       gdouble                   image_opacity,
                                       PicmanLayerModeEffects      paint_mode,
                                       PicmanBrushApplicationMode  brush_hardness,
                                       gdouble                   dynamic_hardness,
                                       PicmanPaintApplicationMode  mode);
void   picman_brush_core_replace_canvas (PicmanBrushCore            *core,
                                       PicmanDrawable             *drawable,
                                       const PicmanCoords         *coords,
                                       gdouble                   brush_opacity,
                                       gdouble                   image_opacity,
                                       PicmanBrushApplicationMode  brush_hardness,
                                       gdouble                   dynamic_hardness,
                                       PicmanPaintApplicationMode  mode);

void   picman_brush_core_color_area_with_pixmap
                                      (PicmanBrushCore            *core,
                                       PicmanDrawable             *drawable,
                                       const PicmanCoords         *coords,
                                       GeglBuffer               *area,
                                       gint                      area_x,
                                       gint                      area_y,
                                       PicmanBrushApplicationMode  mode);

const PicmanTempBuf * picman_brush_core_get_brush_mask
                                      (PicmanBrushCore            *core,
                                       const PicmanCoords         *coords,
                                       PicmanBrushApplicationMode  brush_hardness,
                                       gdouble                   dynamic_hardness);

void   picman_brush_core_eval_transform_dynamics
                                      (PicmanBrushCore            *paint_core,
                                       PicmanDrawable             *drawable,
                                       PicmanPaintOptions         *paint_options,
                                       const PicmanCoords         *coords);


#endif  /*  __PICMAN_BRUSH_CORE_H__  */
