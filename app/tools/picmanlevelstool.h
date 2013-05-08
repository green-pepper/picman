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

#ifndef __PICMAN_LEVELS_TOOL_H__
#define __PICMAN_LEVELS_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_LEVELS_TOOL            (picman_levels_tool_get_type ())
#define PICMAN_LEVELS_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LEVELS_TOOL, PicmanLevelsTool))
#define PICMAN_LEVELS_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LEVELS_TOOL, PicmanLevelsToolClass))
#define PICMAN_IS_LEVELS_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LEVELS_TOOL))
#define PICMAN_IS_LEVELS_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LEVELS_TOOL))
#define PICMAN_LEVELS_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LEVELS_TOOL, PicmanLevelsToolClass))


typedef struct _PicmanLevelsTool      PicmanLevelsTool;
typedef struct _PicmanLevelsToolClass PicmanLevelsToolClass;

struct _PicmanLevelsTool
{
  PicmanImageMapTool      parent_instance;

  PicmanLevelsConfig     *config;

  /* dialog */
  PicmanHistogram        *histogram;

  GtkWidget            *channel_menu;

  GtkWidget            *histogram_view;

  gdouble               ui_scale_factor;
  GtkWidget            *input_bar;
  GtkWidget            *input_sliders;
  GtkWidget            *low_input_spinbutton;
  GtkWidget            *high_input_spinbutton;
  GtkWidget            *low_output_spinbutton;
  GtkWidget            *high_output_spinbutton;
  GtkAdjustment        *low_input;
  GtkAdjustment        *gamma;
  GtkAdjustment        *gamma_linear;
  GtkAdjustment        *high_input;

  GtkWidget            *output_bar;
  GtkWidget            *output_sliders;
  GtkAdjustment        *low_output;
  GtkAdjustment        *high_output;

  /* export dialog */
  gboolean              export_old_format;
};

struct _PicmanLevelsToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_levels_tool_register (PicmanToolRegisterCallback  callback,
                                   gpointer                  data);

GType   picman_levels_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_LEVELS_TOOL_H__  */
