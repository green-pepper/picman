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

#ifndef __PICMAN_SOURCE_TOOL_H__
#define __PICMAN_SOURCE_TOOL_H__


#include "picmanbrushtool.h"


#define PICMAN_TYPE_SOURCE_TOOL            (picman_source_tool_get_type ())
#define PICMAN_SOURCE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SOURCE_TOOL, PicmanSourceTool))
#define PICMAN_SOURCE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SOURCE_TOOL, PicmanSourceToolClass))
#define PICMAN_IS_SOURCE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SOURCE_TOOL))
#define PICMAN_IS_SOURCE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SOURCE_TOOL))
#define PICMAN_SOURCE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SOURCE_TOOL, PicmanSourceToolClass))

#define PICMAN_SOURCE_TOOL_GET_OPTIONS(t)  (PICMAN_SOURCE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanSourceTool      PicmanSourceTool;
typedef struct _PicmanSourceToolClass PicmanSourceToolClass;

struct _PicmanSourceTool
{
  PicmanBrushTool   parent_instance;

  PicmanDisplay    *src_display;
  gint            src_x;
  gint            src_y;

  gboolean        show_source_outline;

  PicmanCanvasItem *src_handle;
  PicmanCanvasItem *src_outline;

  const gchar    *status_paint;
  const gchar    *status_set_source;
  const gchar    *status_set_source_ctrl;
};

struct _PicmanSourceToolClass
{
  PicmanBrushToolClass  parent_class;
};


GType   picman_source_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_SOURCE_TOOL_H__  */
