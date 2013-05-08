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

#ifndef __PICMAN_FREE_SELECT_TOOL_H__
#define __PICMAN_FREE_SELECT_TOOL_H__


#include "picmanselectiontool.h"


#define PICMAN_TYPE_FREE_SELECT_TOOL            (picman_free_select_tool_get_type ())
#define PICMAN_FREE_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FREE_SELECT_TOOL, PicmanFreeSelectTool))
#define PICMAN_FREE_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FREE_SELECT_TOOL, PicmanFreeSelectToolClass))
#define PICMAN_IS_FREE_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FREE_SELECT_TOOL))
#define PICMAN_IS_FREE_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FREE_SELECT_TOOL))
#define PICMAN_FREE_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FREE_SELECT_TOOL, PicmanFreeSelectToolClass))


typedef struct _PicmanFreeSelectTool      PicmanFreeSelectTool;
typedef struct _PicmanFreeSelectToolClass PicmanFreeSelectToolClass;

struct _PicmanFreeSelectTool
{
  PicmanSelectionTool  parent_instance;
};

struct _PicmanFreeSelectToolClass
{
  PicmanSelectionToolClass  parent_class;

  /*  virtual function  */

  void (* select) (PicmanFreeSelectTool *free_select_tool,
                   PicmanDisplay        *display);
};


void    picman_free_select_tool_register   (PicmanToolRegisterCallback  callback,
                                          gpointer                  data);

GType   picman_free_select_tool_get_type   (void) G_GNUC_CONST;

void    picman_free_select_tool_select     (PicmanFreeSelectTool       *free_sel,
                                          PicmanDisplay              *display);

void    picman_free_select_tool_get_points (PicmanFreeSelectTool       *free_sel,
                                          const PicmanVector2       **points,
                                          gint                     *n_points);


#endif  /*  __PICMAN_FREE_SELECT_TOOL_H__  */
