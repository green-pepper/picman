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

#ifndef __PICMAN_SCALE_TOOL_H__
#define __PICMAN_SCALE_TOOL_H__


#include "picmantransformtool.h"


#define PICMAN_TYPE_SCALE_TOOL            (picman_scale_tool_get_type ())
#define PICMAN_SCALE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_SCALE_TOOL, PicmanScaleTool))
#define PICMAN_SCALE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_SCALE_TOOL, PicmanScaleToolClass))
#define PICMAN_IS_SCALE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_SCALE_TOOL))
#define PICMAN_SCALE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_SCALE_TOOL, PicmanScaleToolClass))


typedef struct _PicmanScaleTool      PicmanScaleTool;
typedef struct _PicmanScaleToolClass PicmanScaleToolClass;

struct _PicmanScaleTool
{
  PicmanTransformTool  parent_instance;

  GtkWidget         *box;
};

struct _PicmanScaleToolClass
{
  PicmanTransformToolClass parent_class;
};


void    picman_scale_tool_register (PicmanToolRegisterCallback  callback,
                                  gpointer                  data);

GType   picman_scale_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_SCALE_TOOL_H__  */
