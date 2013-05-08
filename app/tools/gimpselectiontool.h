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

#ifndef __PICMAN_SELECTION_TOOL_H__
#define __PICMAN_SELECTION_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_SELECTION_TOOL            (picman_selection_tool_get_type ())
#define PICMAN_SELECTION_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SELECTION_TOOL, PicmanSelectionTool))
#define PICMAN_SELECTION_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SELECTION_TOOL, PicmanSelectionToolClass))
#define PICMAN_IS_SELECTION_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SELECTION_TOOL))
#define PICMAN_IS_SELECTION_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_SELECTION_TOOL))
#define PICMAN_SELECTION_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SELECTION_TOOL, PicmanSelectionToolClass))

#define PICMAN_SELECTION_TOOL_GET_OPTIONS(t)  (PICMAN_SELECTION_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanSelectionTool      PicmanSelectionTool;
typedef struct _PicmanSelectionToolClass PicmanSelectionToolClass;

struct _PicmanSelectionTool
{
  PicmanDrawTool   parent_instance;

  SelectFunction function;         /*  selection function        */
  PicmanChannelOps saved_operation;  /*  saved tool options state  */

  gboolean       allow_move;
};

struct _PicmanSelectionToolClass
{
  PicmanDrawToolClass  parent_class;
};


GType      picman_selection_tool_get_type   (void) G_GNUC_CONST;

/*  protected function  */
gboolean   picman_selection_tool_start_edit (PicmanSelectionTool *sel_tool,
                                           PicmanDisplay       *display,
                                           const PicmanCoords  *coords);


#endif  /* __PICMAN_SELECTION_TOOL_H__ */
