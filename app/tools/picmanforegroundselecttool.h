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

#ifndef __PICMAN_FOREGROUND_SELECT_TOOL_H__
#define __PICMAN_FOREGROUND_SELECT_TOOL_H__


#include "picmanfreeselecttool.h"


typedef enum  /*< pdb-skip, skip >*/
{
  SIOX_REFINEMENT_NO_CHANGE          = 0,
  SIOX_REFINEMENT_ADD_FOREGROUND     = (1 << 0),
  SIOX_REFINEMENT_ADD_BACKGROUND     = (1 << 1),
  SIOX_REFINEMENT_RECALCULATE        = 0xFF
} SioxRefinementType;


#define PICMAN_TYPE_FOREGROUND_SELECT_TOOL            (picman_foreground_select_tool_get_type ())
#define PICMAN_FOREGROUND_SELECT_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_FOREGROUND_SELECT_TOOL, PicmanForegroundSelectTool))
#define PICMAN_FOREGROUND_SELECT_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_FOREGROUND_SELECT_TOOL, PicmanForegroundSelectToolClass))
#define PICMAN_IS_FOREGROUND_SELECT_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_FOREGROUND_SELECT_TOOL))
#define PICMAN_IS_FOREGROUND_SELECT_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_FOREGROUND_SELECT_TOOL))
#define PICMAN_FOREGROUND_SELECT_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_FOREGROUND_SELECT_TOOL, PicmanForegroundSelectToolClass))

#define PICMAN_FOREGROUND_SELECT_TOOL_GET_OPTIONS(t)  (PICMAN_FOREGROUND_SELECT_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanForegroundSelectTool      PicmanForegroundSelectTool;
typedef struct _PicmanForegroundSelectToolClass PicmanForegroundSelectToolClass;

struct _PicmanForegroundSelectTool
{
  PicmanFreeSelectTool  parent_instance;

  PicmanCoords          last_coords;
  guint               idle_id;
  GArray             *stroke;
  GList              *strokes;
  PicmanChannel        *mask;
#if 0
  SioxState          *state;
#endif
  SioxRefinementType  refinement;
};

struct _PicmanForegroundSelectToolClass
{
  PicmanFreeSelectToolClass  parent_class;
};


void    picman_foreground_select_tool_register (PicmanToolRegisterCallback  callback,
                                              gpointer                  data);

GType   picman_foreground_select_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_FOREGROUND_SELECT_TOOL_H__  */
