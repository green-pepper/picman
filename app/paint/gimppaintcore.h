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

#ifndef __PICMAN_PAINT_CORE_H__
#define __PICMAN_PAINT_CORE_H__


#include "core/picmanobject.h"


#define PICMAN_TYPE_PAINT_CORE            (picman_paint_core_get_type ())
#define PICMAN_PAINT_CORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINT_CORE, PicmanPaintCore))
#define PICMAN_PAINT_CORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINT_CORE, PicmanPaintCoreClass))
#define PICMAN_IS_PAINT_CORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINT_CORE))
#define PICMAN_IS_PAINT_CORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINT_CORE))
#define PICMAN_PAINT_CORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINT_CORE, PicmanPaintCoreClass))


typedef struct _PicmanPaintCoreClass PicmanPaintCoreClass;

struct _PicmanPaintCore
{
  PicmanObject   parent_instance;

  gint         ID;                /*  unique instance ID                  */

  gchar       *undo_desc;         /*  undo description                    */

  PicmanCoords   start_coords;      /*  starting coords (for undo only)     */

  PicmanCoords   cur_coords;        /*  current coords                      */
  PicmanCoords   last_coords;       /*  last coords                         */

  PicmanVector2  last_paint;        /*  last point that was painted         */

  gdouble      distance;          /*  distance traveled by brush          */
  gdouble      pixel_dist;        /*  distance in pixels                  */

  gint         x1, y1;            /*  undo extents in image coords        */
  gint         x2, y2;            /*  undo extents in image coords        */

  gboolean     use_saved_proj;    /*  keep the unmodified proj around     */

  GeglBuffer  *undo_buffer;       /*  pixels which have been modified     */
  GeglBuffer  *saved_proj_buffer; /*  proj tiles which have been modified */
  GeglBuffer  *canvas_buffer;     /*  the buffer to paint the mask to     */

  GeglBuffer  *paint_buffer;      /*  the buffer to paint pixels to       */
  gint         paint_buffer_x;
  gint         paint_buffer_y;

  GArray      *stroke_buffer;

  PicmanApplicator *applicator;
};

struct _PicmanPaintCoreClass
{
  PicmanObjectClass  parent_class;

  /*  virtual functions  */
  gboolean     (* start)            (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     const PicmanCoords *coords,
                                     GError          **error);

  gboolean     (* pre_paint)        (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     PicmanPaintState    paint_state,
                                     guint32           time);
  void         (* paint)            (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     const PicmanCoords *coords,
                                     PicmanPaintState    paint_state,
                                     guint32           time);
  void         (* post_paint)       (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     PicmanPaintState    paint_state,
                                     guint32           time);

  void         (* interpolate)      (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     guint32           time);

  GeglBuffer * (* get_paint_buffer) (PicmanPaintCore    *core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     const PicmanCoords *coords,
                                     gint             *paint_buffer_x,
                                     gint             *paint_buffer_y);

  PicmanUndo   * (* push_undo)        (PicmanPaintCore    *core,
                                     PicmanImage        *image,
                                     const gchar      *undo_desc);
};


GType     picman_paint_core_get_type                  (void) G_GNUC_CONST;

void      picman_paint_core_paint                     (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     PicmanPaintState    state,
                                                     guint32           time);

gboolean  picman_paint_core_start                     (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     const PicmanCoords *coords,
                                                     GError          **error);
void      picman_paint_core_finish                    (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     gboolean          push_undo);
void      picman_paint_core_cancel                    (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable);
void      picman_paint_core_cleanup                   (PicmanPaintCore    *core);

void      picman_paint_core_interpolate               (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     const PicmanCoords *coords,
                                                     guint32           time);

void      picman_paint_core_set_current_coords        (PicmanPaintCore    *core,
                                                     const PicmanCoords *coords);
void      picman_paint_core_get_current_coords        (PicmanPaintCore    *core,
                                                     PicmanCoords       *coords);

void      picman_paint_core_set_last_coords           (PicmanPaintCore    *core,
                                                     const PicmanCoords *coords);
void      picman_paint_core_get_last_coords           (PicmanPaintCore    *core,
                                                     PicmanCoords       *coords);

void      picman_paint_core_round_line                (PicmanPaintCore    *core,
                                                     PicmanPaintOptions *options,
                                                     gboolean          constrain_15_degrees);


/*  protected functions  */

GeglBuffer * picman_paint_core_get_paint_buffer       (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *options,
                                                     const PicmanCoords *coords,
                                                     gint             *paint_buffer_x,
                                                     gint             *paint_buffer_y);

GeglBuffer * picman_paint_core_get_orig_image         (PicmanPaintCore    *core);
GeglBuffer * picman_paint_core_get_orig_proj          (PicmanPaintCore    *core);

void      picman_paint_core_paste             (PicmanPaintCore            *core,
                                             GeglBuffer               *paint_mask,
                                             const GeglRectangle      *paint_mask_rect,
                                             PicmanDrawable             *drawable,
                                             gdouble                   paint_opacity,
                                             gdouble                   image_opacity,
                                             PicmanLayerModeEffects      paint_mode,
                                             PicmanPaintApplicationMode  mode);
void      picman_paint_core_replace           (PicmanPaintCore            *core,
                                             GeglBuffer               *paint_mask,
                                             const GeglRectangle      *paint_mask_rect,
                                             PicmanDrawable             *drawable,
                                             gdouble                   paint_opacity,
                                             gdouble                   image_opacity,
                                             PicmanPaintApplicationMode  mode);

void      picman_paint_core_smooth_coords             (PicmanPaintCore    *core,
                                                     PicmanPaintOptions *paint_options,
                                                     PicmanCoords       *coords);


#endif  /*  __PICMAN_PAINT_CORE_H__  */
