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

#ifndef __PICMAN_PAINT_TOOL_H__
#define __PICMAN_PAINT_TOOL_H__


#include "picmancolortool.h"


#define PICMAN_TYPE_PAINT_TOOL            (picman_paint_tool_get_type ())
#define PICMAN_PAINT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINT_TOOL, PicmanPaintTool))
#define PICMAN_PAINT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINT_TOOL, PicmanPaintToolClass))
#define PICMAN_IS_PAINT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINT_TOOL))
#define PICMAN_IS_PAINT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINT_TOOL))
#define PICMAN_PAINT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINT_TOOL, PicmanPaintToolClass))

#define PICMAN_PAINT_TOOL_GET_OPTIONS(t)  (PICMAN_PAINT_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanPaintToolClass PicmanPaintToolClass;

struct _PicmanPaintTool
{
  PicmanColorTool  parent_instance;

  gboolean       pick_colors;  /*  pick color if ctrl is pressed   */
  gboolean       draw_line;

  const gchar   *status;       /* status message */
  const gchar   *status_line;  /* status message when drawing a line */
  const gchar   *status_ctrl;  /* additional message for the ctrl modifier */

  PicmanPaintCore *core;
};

struct _PicmanPaintToolClass
{
  PicmanColorToolClass  parent_class;
};


GType   picman_paint_tool_get_type            (void) G_GNUC_CONST;

void    picman_paint_tool_enable_color_picker (PicmanPaintTool     *tool,
                                             PicmanColorPickMode  mode);


#endif  /*  __PICMAN_PAINT_TOOL_H__  */
