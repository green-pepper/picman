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

#ifndef  __PICMAN_BLEND_TOOL_H__
#define  __PICMAN_BLEND_TOOL_H__


#include "picmandrawtool.h"


#define PICMAN_TYPE_BLEND_TOOL            (picman_blend_tool_get_type ())
#define PICMAN_BLEND_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BLEND_TOOL, PicmanBlendTool))
#define PICMAN_BLEND_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BLEND_TOOL, PicmanBlendToolClass))
#define PICMAN_IS_BLEND_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BLEND_TOOL))
#define PICMAN_IS_BLEND_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BLEND_TOOL))
#define PICMAN_BLEND_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BLEND_TOOL, PicmanBlendToolClass))

#define PICMAN_BLEND_TOOL_GET_OPTIONS(t)  (PICMAN_BLEND_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanBlendTool      PicmanBlendTool;
typedef struct _PicmanBlendToolClass PicmanBlendToolClass;

struct _PicmanBlendTool
{
  PicmanDrawTool    parent_instance;

  gdouble         start_x;    /*  starting x coord  */
  gdouble         start_y;    /*  starting y coord  */
  gdouble         end_x;      /*  ending x coord    */
  gdouble         end_y;      /*  ending y coord    */

  gdouble         last_x;     /*  last x coord      */
  gdouble         last_y;     /*  last y coord      */
  gdouble         mouse_x;    /*  pointer x coord   */
  gdouble         mouse_y;    /*  pointer y coord   */

  PicmanCanvasItem *start_handle;
  PicmanCanvasItem *line;
  PicmanCanvasItem *end_handle;
};

struct _PicmanBlendToolClass
{
  PicmanDrawToolClass  parent_class;
};


void    picman_blend_tool_register (PicmanToolRegisterCallback  callback,
                                  gpointer                  data);

GType   picman_blend_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_BLEND_TOOL_H__  */
