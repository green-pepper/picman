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

#ifndef __PICMAN_BRUSH_H__
#define __PICMAN_BRUSH_H__


#include "picmandata.h"


#define PICMAN_TYPE_BRUSH            (picman_brush_get_type ())
#define PICMAN_BRUSH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH, PicmanBrush))
#define PICMAN_BRUSH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH, PicmanBrushClass))
#define PICMAN_IS_BRUSH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH))
#define PICMAN_IS_BRUSH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH))
#define PICMAN_BRUSH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH, PicmanBrushClass))


typedef struct _PicmanBrushClass PicmanBrushClass;

struct _PicmanBrush
{
  PicmanData        parent_instance;

  PicmanTempBuf    *mask;       /*  the actual mask                */
  PicmanTempBuf    *pixmap;     /*  optional pixmap data           */

  gint            spacing;    /*  brush's spacing                */
  PicmanVector2     x_axis;     /*  for calculating brush spacing  */
  PicmanVector2     y_axis;     /*  for calculating brush spacing  */

  gint            use_count;  /*  for keeping the caches alive   */
  PicmanBrushCache *mask_cache;
  PicmanBrushCache *pixmap_cache;
  PicmanBrushCache *boundary_cache;
};

struct _PicmanBrushClass
{
  PicmanDataClass  parent_class;

  /*  virtual functions  */
  void             (* begin_use)          (PicmanBrush        *brush);
  void             (* end_use)            (PicmanBrush        *brush);
  PicmanBrush      * (* select_brush)       (PicmanBrush        *brush,
                                           const PicmanCoords *last_coords,
                                           const PicmanCoords *current_coords);
  gboolean         (* want_null_motion)   (PicmanBrush        *brush,
                                           const PicmanCoords *last_coords,
                                           const PicmanCoords *current_coords);
  void             (* transform_size)     (PicmanBrush        *brush,
                                           gdouble           scale,
                                           gdouble           aspect_ratio,
                                           gdouble           angle,
                                           gint             *width,
                                           gint             *height);
  PicmanTempBuf    * (* transform_mask)     (PicmanBrush        *brush,
                                           gdouble           scale,
                                           gdouble           aspect_ratio,
                                           gdouble           angle,
                                           gdouble           hardness);
  PicmanTempBuf    * (* transform_pixmap)   (PicmanBrush        *brush,
                                           gdouble           scale,
                                           gdouble           aspect_ratio,
                                           gdouble           angle,
                                           gdouble           hardness);
  PicmanBezierDesc * (* transform_boundary) (PicmanBrush        *brush,
                                           gdouble           scale,
                                           gdouble           aspect_ratio,
                                           gdouble           angle,
                                           gdouble           hardness,
                                           gint             *width,
                                           gint             *height);

  /*  signals  */
  void             (* spacing_changed)    (PicmanBrush        *brush);
};


GType                  picman_brush_get_type           (void) G_GNUC_CONST;

PicmanData             * picman_brush_new                (PicmanContext      *context,
                                                      const gchar      *name);
PicmanData             * picman_brush_get_standard       (PicmanContext      *context);

void                   picman_brush_begin_use          (PicmanBrush        *brush);
void                   picman_brush_end_use            (PicmanBrush        *brush);

PicmanBrush            * picman_brush_select_brush       (PicmanBrush        *brush,
                                                      const PicmanCoords *last_coords,
                                                      const PicmanCoords *current_coords);
gboolean               picman_brush_want_null_motion   (PicmanBrush        *brush,
                                                      const PicmanCoords *last_coords,
                                                      const PicmanCoords *current_coords);

/* Gets width and height of a transformed mask of the brush, for
 * provided parameters.
 */
void                   picman_brush_transform_size     (PicmanBrush        *brush,
                                                      gdouble           scale,
                                                      gdouble           aspect_ratio,
                                                      gdouble           angle,
                                                      gint             *width,
                                                      gint             *height);
const PicmanTempBuf    * picman_brush_transform_mask     (PicmanBrush        *brush,
                                                      gdouble           scale,
                                                      gdouble           aspect_ratio,
                                                      gdouble           angle,
                                                      gdouble           hardness);
const PicmanTempBuf    * picman_brush_transform_pixmap   (PicmanBrush        *brush,
                                                      gdouble           scale,
                                                      gdouble           aspect_ratio,
                                                      gdouble           angle,
                                                      gdouble           hardness);
const PicmanBezierDesc * picman_brush_transform_boundary (PicmanBrush        *brush,
                                                      gdouble           scale,
                                                      gdouble           aspect_ratio,
                                                      gdouble           angle,
                                                      gdouble           hardness,
                                                      gint             *width,
                                                      gint             *height);

PicmanTempBuf          * picman_brush_get_mask           (const PicmanBrush  *brush);
PicmanTempBuf          * picman_brush_get_pixmap         (const PicmanBrush  *brush);

gint                   picman_brush_get_spacing        (const PicmanBrush  *brush);
void                   picman_brush_set_spacing        (PicmanBrush        *brush,
                                                      gint              spacing);


#endif /* __PICMAN_BRUSH_H__ */
