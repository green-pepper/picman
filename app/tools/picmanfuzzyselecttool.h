/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfuzzyselecttool.h
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

#ifndef __PICMAN_FUZZY_SELECT_TOOL_H__
#define __PICMAN_FUZZY_SELECT_TOOL_H__


#include "picmanregionselecttool.h"


#define PICMAN_TYPE_FUZZY_SELECT_TOOL            (picman_fuzzy_select_tool_get_type ())
#define PICMAN_FUZZY_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FUZZY_SELECT_TOOL, PicmanFuzzySelectTool))
#define PICMAN_FUZZY_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FUZZY_SELECT_TOOL, PicmanFuzzySelectToolClass))
#define PICMAN_IS_FUZZY_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FUZZY_SELECT_TOOL))
#define PICMAN_IS_FUZZY_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FUZZY_SELECT_TOOL))
#define PICMAN_FUZZY_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FUZZY_SELECT_TOOL, PicmanFuzzySelectToolClass))


typedef struct _PicmanFuzzySelectTool      PicmanFuzzySelectTool;
typedef struct _PicmanFuzzySelectToolClass PicmanFuzzySelectToolClass;

struct _PicmanFuzzySelectTool
{
  PicmanRegionSelectTool  parent_instance;
};

struct _PicmanFuzzySelectToolClass
{
  PicmanRegionSelectToolClass  parent_class;
};


void    picman_fuzzy_select_tool_register (PicmanToolRegisterCallback  callback,
                                         gpointer                  data);

GType   picman_fuzzy_select_tool_get_type (void) G_GNUC_CONST;


#endif  /* __PICMAN_FUZZY_SELECT_TOOL_H__ */
