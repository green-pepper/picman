/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanregionselecttool.h
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

#ifndef __PICMAN_REGION_SELECT_TOOL_H__
#define __PICMAN_REGION_SELECT_TOOL_H__


#include "picmanselectiontool.h"


#define PICMAN_TYPE_REGION_SELECT_TOOL            (picman_region_select_tool_get_type ())
#define PICMAN_REGION_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_REGION_SELECT_TOOL, PicmanRegionSelectTool))
#define PICMAN_REGION_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_REGION_SELECT_TOOL, PicmanRegionSelectToolClass))
#define PICMAN_IS_REGION_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_REGION_SELECT_TOOL))
#define PICMAN_IS_REGION_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_REGION_SELECT_TOOL))
#define PICMAN_REGION_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_REGION_SELECT_TOOL, PicmanRegionSelectToolClass))

#define PICMAN_REGION_SELECT_TOOL_GET_OPTIONS(t)  (PICMAN_REGION_SELECT_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanRegionSelectTool      PicmanRegionSelectTool;
typedef struct _PicmanRegionSelectToolClass PicmanRegionSelectToolClass;

struct _PicmanRegionSelectTool
{
  PicmanSelectionTool  parent_instance;

  gint               x, y;
  gdouble            saved_threshold;

  GeglBuffer        *region_mask;
  PicmanBoundSeg      *segs;
  gint               n_segs;
};

struct _PicmanRegionSelectToolClass
{
  PicmanSelectionToolClass parent_class;

  const gchar * undo_desc;

  GeglBuffer * (* get_mask) (PicmanRegionSelectTool *region_tool,
                             PicmanDisplay          *display);
};


GType   picman_region_select_tool_get_type (void) G_GNUC_CONST;


#endif  /* __PICMAN_REGION_SELECT_TOOL_H__ */
