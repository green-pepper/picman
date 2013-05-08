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

#ifndef __PICMAN_MOVE_TOOL_H__
#define __PICMAN_MOVE_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_MOVE_TOOL            (picman_move_tool_get_type ())
#define PICMAN_MOVE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MOVE_TOOL, PicmanMoveTool))
#define PICMAN_MOVE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MOVE_TOOL, PicmanMoveToolClass))
#define PICMAN_IS_MOVE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MOVE_TOOL))
#define PICMAN_IS_MOVE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MOVE_TOOL))
#define PICMAN_MOVE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MOVE_TOOL, PicmanMoveToolClass))

#define PICMAN_MOVE_TOOL_GET_OPTIONS(t)  (PICMAN_MOVE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanMoveTool      PicmanMoveTool;
typedef struct _PicmanMoveToolClass PicmanMoveToolClass;

struct _PicmanMoveTool
{
  PicmanDrawTool         parent_instance;

  PicmanLayer           *floating_layer;
  PicmanGuide           *guide;

  gboolean             moving_guide;
  gint                 guide_position;
  PicmanOrientationType  guide_orientation;

  PicmanTransformType    saved_type;

  PicmanLayer           *old_active_layer;
  PicmanVectors         *old_active_vectors;
};

struct _PicmanMoveToolClass
{
  PicmanDrawToolClass  parent_class;
};


void    picman_move_tool_register (PicmanToolRegisterCallback  callback,
                                 gpointer                  data);

GType   picman_move_tool_get_type (void) G_GNUC_CONST;


void    picman_move_tool_start_hguide (PicmanTool    *tool,
                                     PicmanDisplay *display);
void    picman_move_tool_start_vguide (PicmanTool    *tool,
                                     PicmanDisplay *display);


#endif  /*  __PICMAN_MOVE_TOOL_H__  */
