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

#ifndef __PICMAN_HUE_SATURATION_TOOL_H__
#define __PICMAN_HUE_SATURATION_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_HUE_SATURATION_TOOL            (picman_hue_saturation_tool_get_type ())
#define PICMAN_HUE_SATURATION_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HUE_SATURATION_TOOL, PicmanHueSaturationTool))
#define PICMAN_HUE_SATURATION_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_HUE_SATURATION_TOOL, PicmanHueSaturationToolClass))
#define PICMAN_IS_HUE_SATURATION_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HUE_SATURATION_TOOL))
#define PICMAN_IS_HUE_SATURATION_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_HUE_SATURATION_TOOL))
#define PICMAN_HUE_SATURATION_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_HUE_SATURATION_TOOL, PicmanHueSaturationToolClass))


typedef struct _PicmanHueSaturationTool      PicmanHueSaturationTool;
typedef struct _PicmanHueSaturationToolClass PicmanHueSaturationToolClass;

struct _PicmanHueSaturationTool
{
  PicmanImageMapTool         parent_instance;

  PicmanHueSaturationConfig *config;

  /*  dialog  */
  GtkWidget               *range_radio;
  GtkWidget               *hue_range_color_area[6];
  GtkAdjustment           *overlap_data;
  GtkAdjustment           *hue_data;
  GtkAdjustment           *lightness_data;
  GtkAdjustment           *saturation_data;
};

struct _PicmanHueSaturationToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_hue_saturation_tool_register (PicmanToolRegisterCallback  callback,
                                           gpointer                  data);

GType   picman_hue_saturation_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_HUE_SATURATION_TOOL_H__  */
