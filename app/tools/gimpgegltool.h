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

#ifndef __PICMAN_GEGL_TOOL_H__
#define __PICMAN_GEGL_TOOL_H__


#include "picmanoperationtool.h"


#define PICMAN_TYPE_GEGL_TOOL            (picman_gegl_tool_get_type ())
#define PICMAN_GEGL_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GEGL_TOOL, PicmanGeglTool))
#define PICMAN_GEGL_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GEGL_TOOL, PicmanGeglToolClass))
#define PICMAN_IS_GEGL_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GEGL_TOOL))
#define PICMAN_IS_GEGL_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GEGL_TOOL))
#define PICMAN_GEGL_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GEGL_TOOL, PicmanGeglToolClass))


typedef struct _PicmanGeglTool      PicmanGeglTool;
typedef struct _PicmanGeglToolClass PicmanGeglToolClass;

struct _PicmanGeglTool
{
  PicmanOperationTool  parent_instance;

  /* dialog */
  GtkWidget         *operation_combo;
};

struct _PicmanGeglToolClass
{
  PicmanOperationToolClass  parent_class;
};


void    picman_gegl_tool_register (PicmanToolRegisterCallback  callback,
                                 gpointer                  data);

GType   picman_gegl_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_GEGL_TOOL_H__  */
