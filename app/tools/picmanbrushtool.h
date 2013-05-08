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

#ifndef __PICMAN_BRUSH_TOOL_H__
#define __PICMAN_BRUSH_TOOL_H__


#include "picmanpainttool.h"


#define PICMAN_TYPE_BRUSH_TOOL            (picman_brush_tool_get_type ())
#define PICMAN_BRUSH_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_TOOL, PicmanBrushTool))
#define PICMAN_BRUSH_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_TOOL, PicmanBrushToolClass))
#define PICMAN_IS_BRUSH_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_TOOL))
#define PICMAN_IS_BRUSH_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_TOOL))
#define PICMAN_BRUSH_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_TOOL, PicmanBrushToolClass))


typedef struct _PicmanBrushToolClass PicmanBrushToolClass;

struct _PicmanBrushTool
{
  PicmanPaintTool  parent_instance;

  gboolean       show_cursor;
  gboolean       draw_brush;
  gdouble        brush_x;
  gdouble        brush_y;
};

struct _PicmanBrushToolClass
{
  PicmanPaintToolClass  parent_class;
};


GType            picman_brush_tool_get_type       (void) G_GNUC_CONST;

PicmanCanvasItem * picman_brush_tool_create_outline (PicmanBrushTool *brush_tool,
                                                 PicmanDisplay   *display,
                                                 gdouble        x,
                                                 gdouble        y,
                                                 gboolean       draw_fallback);


#endif  /*  __PICMAN_BRUSH_TOOL_H__  */
