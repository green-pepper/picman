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

#ifndef __PICMAN_DESATURATE_TOOL_H__
#define __PICMAN_DESATURATE_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_DESATURATE_TOOL            (picman_desaturate_tool_get_type ())
#define PICMAN_DESATURATE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DESATURATE_TOOL, PicmanDesaturateTool))
#define PICMAN_DESATURATE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DESATURATE_TOOL, PicmanDesaturateToolClass))
#define PICMAN_IS_DESATURATE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DESATURATE_TOOL))
#define PICMAN_IS_DESATURATE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DESATURATE_TOOL))
#define PICMAN_DESATURATE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DESATURATE_TOOL, PicmanDesaturateToolClass))


typedef struct _PicmanDesaturateTool      PicmanDesaturateTool;
typedef struct _PicmanDesaturateToolClass PicmanDesaturateToolClass;

struct _PicmanDesaturateTool
{
  PicmanImageMapTool       parent_instance;

  PicmanDesaturateConfig  *config;

  /*  dialog  */
  GtkWidget             *button;
};

struct _PicmanDesaturateToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_desaturate_tool_register (PicmanToolRegisterCallback  callback,
                                       gpointer                  data);

GType   picman_desaturate_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_DESATURATE_TOOL_H__  */
