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

#ifndef __PICMAN_UNIFIED_TRANSFORM_TOOL_H__
#define __PICMAN_UNIFIED_TRANSFORM_TOOL_H__


#include "picmantransformtool.h"


#define PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL            (picman_unified_transform_tool_get_type ())
#define PICMAN_UNIFIED_TRANSFORM_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL, PicmanUnifiedTransformTool))
#define PICMAN_UNIFIED_TRANSFORM_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL, PicmanUnifiedTransformToolClass))
#define PICMAN_IS_UNIFIED_TRANSFORM_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL))
#define PICMAN_IS_UNIFIED_TRANSFORM_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL))
#define PICMAN_UNIFIED_TRANSFORM_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_UNIFIED_TRANSFORM_TOOL, PicmanUnifiedTransformToolClass))


typedef struct _PicmanUnifiedTransformTool      PicmanUnifiedTransformTool;
typedef struct _PicmanUnifiedTransformToolClass PicmanUnifiedTransformToolClass;

struct _PicmanUnifiedTransformTool
{
  PicmanTransformTool  parent_instance;

  GtkWidget         *label[3][3];
};

struct _PicmanUnifiedTransformToolClass
{
  PicmanTransformToolClass  parent_class;
};


void    picman_unified_transform_tool_register (PicmanToolRegisterCallback  callback,
                                              gpointer                  data);

GType   picman_unified_transform_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_UNIFIED_TRANSFORM_TOOL_H__  */
