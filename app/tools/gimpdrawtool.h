/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis, and others.
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

#ifndef __PICMAN_DRAW_TOOL_H__
#define __PICMAN_DRAW_TOOL_H__


#include "picmantool.h"


#define PICMAN_TOOL_HANDLE_SIZE_CIRCLE 13
#define PICMAN_TOOL_HANDLE_SIZE_CROSS  15
#define PICMAN_TOOL_HANDLE_SIZE_LARGE  25
#define PICMAN_TOOL_HANDLE_SIZE_SMALL   7


#define PICMAN_TYPE_DRAW_TOOL            (picman_draw_tool_get_type ())
#define PICMAN_DRAW_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DRAW_TOOL, PicmanDrawTool))
#define PICMAN_DRAW_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DRAW_TOOL, PicmanDrawToolClass))
#define PICMAN_IS_DRAW_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DRAW_TOOL))
#define PICMAN_IS_DRAW_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DRAW_TOOL))
#define PICMAN_DRAW_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DRAW_TOOL, PicmanDrawToolClass))


typedef struct _PicmanDrawToolClass PicmanDrawToolClass;

struct _PicmanDrawTool
{
  PicmanTool        parent_instance;

  PicmanDisplay    *display;        /*  The display we are drawing to (may be
                                   *  a different one than tool->display)
                                   */

  gint            paused_count;   /*  count to keep track of multiple pauses  */
  guint           draw_timeout;   /*  draw delay timeout ID                   */
  guint64         last_draw_time; /*  time of last draw(), monotonically      */

  PicmanCanvasItem *preview;
  PicmanCanvasItem *item;
  GList          *group_stack;
};

struct _PicmanDrawToolClass
{
  PicmanToolClass   parent_class;

  /*  virtual function  */

  void (* draw) (PicmanDrawTool *draw_tool);
};


GType            picman_draw_tool_get_type             (void) G_GNUC_CONST;

void             picman_draw_tool_start                (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display);
void             picman_draw_tool_stop                 (PicmanDrawTool     *draw_tool);

gboolean         picman_draw_tool_is_active            (PicmanDrawTool     *draw_tool);

void             picman_draw_tool_pause                (PicmanDrawTool     *draw_tool);
void             picman_draw_tool_resume               (PicmanDrawTool     *draw_tool);

gdouble          picman_draw_tool_calc_distance        (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2);
gdouble          picman_draw_tool_calc_distance_square (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2);

void             picman_draw_tool_add_preview          (PicmanDrawTool     *draw_tool,
                                                      PicmanCanvasItem   *item);
void             picman_draw_tool_remove_preview       (PicmanDrawTool     *draw_tool,
                                                      PicmanCanvasItem   *item);

void             picman_draw_tool_add_item             (PicmanDrawTool     *draw_tool,
                                                      PicmanCanvasItem   *item);
void             picman_draw_tool_remove_item          (PicmanDrawTool     *draw_tool,
                                                      PicmanCanvasItem   *item);

PicmanCanvasGroup* picman_draw_tool_add_stroke_group     (PicmanDrawTool     *draw_tool);
PicmanCanvasGroup* picman_draw_tool_add_fill_group       (PicmanDrawTool     *draw_tool);

void             picman_draw_tool_push_group           (PicmanDrawTool     *draw_tool,
                                                      PicmanCanvasGroup  *group);
void             picman_draw_tool_pop_group            (PicmanDrawTool     *draw_tool);

PicmanCanvasItem * picman_draw_tool_add_line             (PicmanDrawTool     *draw_tool,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2);
PicmanCanvasItem * picman_draw_tool_add_guide            (PicmanDrawTool     *draw_tool,
                                                      PicmanOrientationType  orientation,
                                                      gint              position,
                                                      gboolean          guide_style);
PicmanCanvasItem * picman_draw_tool_add_crosshair        (PicmanDrawTool     *draw_tool,
                                                      gint              position_x,
                                                      gint              position_y);
PicmanCanvasItem * picman_draw_tool_add_sample_point     (PicmanDrawTool     *draw_tool,
                                                      gint              x,
                                                      gint              y,
                                                      gint              index);
PicmanCanvasItem * picman_draw_tool_add_rectangle        (PicmanDrawTool     *draw_tool,
                                                      gboolean          filled,
                                                      gdouble           x,
                                                      gdouble           y,
                                                      gdouble           width,
                                                      gdouble           height);
PicmanCanvasItem * picman_draw_tool_add_rectangle_guides (PicmanDrawTool     *draw_tool,
                                                      PicmanGuidesType    type,
                                                      gdouble           x,
                                                      gdouble           y,
                                                      gdouble           width,
                                                      gdouble           height);
PicmanCanvasItem * picman_draw_tool_add_arc              (PicmanDrawTool     *draw_tool,
                                                      gboolean          filled,
                                                      gdouble           x,
                                                      gdouble           y,
                                                      gdouble           width,
                                                      gdouble           height,
                                                      gdouble           start_angle,
                                                      gdouble           slice_angle);
PicmanCanvasItem * picman_draw_tool_add_transform_guides (PicmanDrawTool     *draw_tool,
                                                      const PicmanMatrix3 *transform,
                                                      PicmanGuidesType    type,
                                                      gint              n_guides,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2);
PicmanCanvasItem * picman_draw_tool_add_transform_preview(PicmanDrawTool     *draw_tool,
                                                      PicmanDrawable     *drawable,
                                                      const PicmanMatrix3 *transform,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2,
                                                      gboolean          perspective,
                                                      gdouble            opacity);

PicmanCanvasItem * picman_draw_tool_add_handle           (PicmanDrawTool     *draw_tool,
                                                      PicmanHandleType    type,
                                                      gdouble           x,
                                                      gdouble           y,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanHandleAnchor  anchor);
PicmanCanvasItem * picman_draw_tool_add_corner           (PicmanDrawTool     *draw_tool,
                                                      gboolean          highlight,
                                                      gboolean          put_outside,
                                                      gdouble           x1,
                                                      gdouble           y1,
                                                      gdouble           x2,
                                                      gdouble           y2,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanHandleAnchor  anchor);

PicmanCanvasItem * picman_draw_tool_add_lines            (PicmanDrawTool     *draw_tool,
                                                      const PicmanVector2 *points,
                                                      gint              n_points,
                                                      gboolean          filled);

PicmanCanvasItem * picman_draw_tool_add_strokes          (PicmanDrawTool     *draw_tool,
                                                      const PicmanCoords *points,
                                                      gint              n_points,
                                                      gboolean          filled);
PicmanCanvasItem * picman_draw_tool_add_path             (PicmanDrawTool     *draw_tool,
                                                      const PicmanBezierDesc *desc,
                                                      gdouble           x,
                                                      gdouble           y);

PicmanCanvasItem * picman_draw_tool_add_pen              (PicmanDrawTool     *draw_tool,
                                                      const PicmanVector2 *points,
                                                      gint              n_points,
                                                      PicmanContext      *context,
                                                      PicmanActiveColor   color,
                                                      gint              width);

PicmanCanvasItem * picman_draw_tool_add_boundary         (PicmanDrawTool     *draw_tool,
                                                      const PicmanBoundSeg *bound_segs,
                                                      gint              n_bound_segs,
                                                      PicmanMatrix3      *transform,
                                                      gdouble           offset_x,
                                                      gdouble           offset_y);

PicmanCanvasItem * picman_draw_tool_add_text_cursor      (PicmanDrawTool     *draw_tool,
                                                      PangoRectangle   *cursor,
                                                      gboolean          overwrite);

gboolean         picman_draw_tool_on_handle            (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      gdouble           x,
                                                      gdouble           y,
                                                      PicmanHandleType    type,
                                                      gdouble           handle_x,
                                                      gdouble           handle_y,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanHandleAnchor  anchor);
gboolean         picman_draw_tool_on_vectors_handle    (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      PicmanVectors      *vectors,
                                                      const PicmanCoords *coord,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanAnchorType    preferred,
                                                      gboolean          exclusive,
                                                      PicmanAnchor      **ret_anchor,
                                                      PicmanStroke      **ret_stroke);
gboolean         picman_draw_tool_on_vectors_curve     (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      PicmanVectors      *vectors,
                                                      const PicmanCoords *coord,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanCoords       *ret_coords,
                                                      gdouble          *ret_pos,
                                                      PicmanAnchor      **ret_segment_start,
                                                      PicmanAnchor      **ret_segment_end,
                                                      PicmanStroke      **ret_stroke);

gboolean         picman_draw_tool_on_vectors           (PicmanDrawTool     *draw_tool,
                                                      PicmanDisplay      *display,
                                                      const PicmanCoords *coord,
                                                      gint              width,
                                                      gint              height,
                                                      PicmanCoords       *ret_coords,
                                                      gdouble          *ret_pos,
                                                      PicmanAnchor      **ret_segment_start,
                                                      PicmanAnchor      **ret_segment_end,
                                                      PicmanStroke      **ret_stroke,
                                                      PicmanVectors     **ret_vectors);


#endif  /*  __PICMAN_DRAW_TOOL_H__  */
