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

#ifndef  __PICMAN_COLOR_PICKER_TOOL_H__
#define  __PICMAN_COLOR_PICKER_TOOL_H__


#include "picmancolortool.h"


#define PICMAN_TYPE_COLOR_PICKER_TOOL            (picman_color_picker_tool_get_type ())
#define PICMAN_COLOR_PICKER_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_PICKER_TOOL, PicmanColorPickerTool))
#define PICMAN_COLOR_PICKER_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_PICKER_TOOL, PicmanColorPickerToolClass))
#define PICMAN_IS_COLOR_PICKER_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_PICKER_TOOL))
#define PICMAN_IS_COLOR_PICKER_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_PICKER_TOOL))
#define PICMAN_COLOR_PICKER_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_PICKER_TOOL, PicmanColorPickerToolClass))

#define PICMAN_COLOR_PICKER_TOOL_GET_OPTIONS(t)  (PICMAN_COLOR_PICKER_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanColorPickerTool      PicmanColorPickerTool;
typedef struct _PicmanColorPickerToolClass PicmanColorPickerToolClass;

struct _PicmanColorPickerTool
{
  PicmanColorTool  parent_instance;

  GtkWidget     *dialog;
  GtkWidget     *color_area;
  GtkWidget     *color_frame1;
  GtkWidget     *color_frame2;
};

struct _PicmanColorPickerToolClass
{
  PicmanColorToolClass  parent_class;
};


void    picman_color_picker_tool_register (PicmanToolRegisterCallback  callback,
                                         gpointer                  data);

GType   picman_color_picker_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_COLOR_PICKER_TOOL_H__  */
