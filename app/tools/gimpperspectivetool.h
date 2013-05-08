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

#ifndef __PICMAN_PERSPECTIVE_TOOL_H__
#define __PICMAN_PERSPECTIVE_TOOL_H__


#include "picmantransformtool.h"


#define PICMAN_TYPE_PERSPECTIVE_TOOL            (picman_perspective_tool_get_type ())
#define PICMAN_PERSPECTIVE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PERSPECTIVE_TOOL, PicmanPerspectiveTool))
#define PICMAN_PERSPECTIVE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PERSPECTIVE_TOOL, PicmanPerspectiveToolClass))
#define PICMAN_IS_PERSPECTIVE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PERSPECTIVE_TOOL))
#define PICMAN_IS_PERSPECTIVE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PERSPECTIVE_TOOL))
#define PICMAN_PERSPECTIVE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PERSPECTIVE_TOOL, PicmanPerspectiveToolClass))


typedef struct _PicmanPerspectiveTool      PicmanPerspectiveTool;
typedef struct _PicmanPerspectiveToolClass PicmanPerspectiveToolClass;

struct _PicmanPerspectiveTool
{
  PicmanTransformTool  parent_instance;

  GtkWidget         *label[3][3];
};

struct _PicmanPerspectiveToolClass
{
  PicmanTransformToolClass  parent_class;
};


void    picman_perspective_tool_register (PicmanToolRegisterCallback  callback,
                                        gpointer                  data);

GType   picman_perspective_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_PERSPECTIVE_TOOL_H__  */
