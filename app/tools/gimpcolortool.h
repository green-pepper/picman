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

#ifndef  __PICMAN_COLOR_TOOL_H__
#define  __PICMAN_COLOR_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_COLOR_TOOL            (picman_color_tool_get_type ())
#define PICMAN_COLOR_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_TOOL, PicmanColorTool))
#define PICMAN_COLOR_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_TOOL, PicmanColorToolClass))
#define PICMAN_IS_COLOR_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_TOOL))
#define PICMAN_IS_COLOR_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_TOOL))
#define PICMAN_COLOR_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_TOOL, PicmanColorToolClass))

#define PICMAN_COLOR_TOOL_GET_OPTIONS(t)  (PICMAN_COLOR_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanColorToolClass PicmanColorToolClass;

struct _PicmanColorTool
{
  PicmanDrawTool       parent_instance;

  gboolean           enabled;
  gint               center_x;
  gint               center_y;
  PicmanColorPickMode  pick_mode;

  PicmanColorOptions  *options;

  PicmanSamplePoint   *sample_point;
  gboolean           moving_sample_point;
  gint               sample_point_x;
  gint               sample_point_y;
};

struct _PicmanColorToolClass
{
  PicmanDrawToolClass  parent_class;

  /*  virtual functions  */
  gboolean (* pick)   (PicmanColorTool      *tool,
                       gint                x,
                       gint                y,
                       const Babl        **sample_format,
                       PicmanRGB            *color,
                       gint               *color_index);

  /*  signals  */
  void     (* picked) (PicmanColorTool      *tool,
                       PicmanColorPickState  pick_state,
                       const Babl         *sample_format,
                       const PicmanRGB      *color,
                       gint                color_index);
};


GType      picman_color_tool_get_type           (void) G_GNUC_CONST;

void       picman_color_tool_enable             (PicmanColorTool    *color_tool,
                                               PicmanColorOptions *options);
void       picman_color_tool_disable            (PicmanColorTool    *color_tool);
gboolean   picman_color_tool_is_enabled         (PicmanColorTool    *color_tool);

void       picman_color_tool_start_sample_point (PicmanTool         *tool,
                                               PicmanDisplay      *display);


#endif  /*  __PICMAN_COLOR_TOOL_H__  */
