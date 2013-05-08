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

#ifndef __PICMAN_BRIGHTNESS_CONTRAST_TOOL_H__
#define __PICMAN_BRIGHTNESS_CONTRAST_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL            (picman_brightness_contrast_tool_get_type ())
#define PICMAN_BRIGHTNESS_CONTRAST_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL, PicmanBrightnessContrastTool))
#define PICMAN_BRIGHTNESS_CONTRAST_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL, PicmanBrightnessContrastToolClass))
#define PICMAN_IS_BRIGHTNESS_CONTRAST_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL))
#define PICMAN_IS_BRIGHTNESS_CONTRAST_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL))
#define PICMAN_BRIGHTNESS_CONTRAST_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRIGHTNESS_CONTRAST_TOOL, PicmanBrightnessContrastToolClass))


typedef struct _PicmanBrightnessContrastTool      PicmanBrightnessContrastTool;
typedef struct _PicmanBrightnessContrastToolClass PicmanBrightnessContrastToolClass;

struct _PicmanBrightnessContrastTool
{
  PicmanImageMapTool              parent_instance;

  PicmanBrightnessContrastConfig *config;

  gdouble                       x, y;
  gdouble                       dx, dy;

  /*  dialog  */
  GtkAdjustment                *brightness_data;
  GtkAdjustment                *contrast_data;
};

struct _PicmanBrightnessContrastToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_brightness_contrast_tool_register (PicmanToolRegisterCallback  callback,
                                                gpointer                  data);

GType   picman_brightness_contrast_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_BRIGHTNESS_CONTRAST_TOOL_H__  */
