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

#ifndef __PICMAN_COLOR_BALANCE_TOOL_H__
#define __PICMAN_COLOR_BALANCE_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_COLOR_BALANCE_TOOL            (picman_color_balance_tool_get_type ())
#define PICMAN_COLOR_BALANCE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLOR_BALANCE_TOOL, PicmanColorBalanceTool))
#define PICMAN_COLOR_BALANCE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_BALANCE_TOOL, PicmanColorBalanceToolClass))
#define PICMAN_IS_COLOR_BALANCE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLOR_BALANCE_TOOL))
#define PICMAN_COLOR_BALANCE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLOR_BALANCE_TOOL, PicmanColorBalanceToolClass))
#define PICMAN_IS_COLOR_BALANCE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLOR_BALANCE_TOOL))
#define PICMAN_COLOR_BALANCE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLOR_BALANCE_TOOL, PicmanColorBalanceToolClass))


typedef struct _PicmanColorBalanceTool      PicmanColorBalanceTool;
typedef struct _PicmanColorBalanceToolClass PicmanColorBalanceToolClass;

struct _PicmanColorBalanceTool
{
  PicmanImageMapTool        parent_instance;

  PicmanColorBalanceConfig *config;

  /*  dialog  */
  GtkWidget              *range_radio;
  GtkAdjustment          *cyan_red_adj;
  GtkAdjustment          *magenta_green_adj;
  GtkAdjustment          *yellow_blue_adj;
  GtkWidget              *preserve_toggle;
};

struct _PicmanColorBalanceToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_color_balance_tool_register (PicmanToolRegisterCallback  callback,
                                          gpointer                  data);

GType   picman_color_balance_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_COLOR_BALANCE_TOOL_H__  */
