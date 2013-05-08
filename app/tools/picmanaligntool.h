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

#ifndef __PICMAN_ALIGN_TOOL_H__
#define __PICMAN_ALIGN_TOOL_H__


#include "picmandrawtool.h"


/*  tool function/operation/state/mode  */
typedef enum
{
  ALIGN_TOOL_IDLE,
  ALIGN_TOOL_PICK_LAYER,
  ALIGN_TOOL_ADD_LAYER,
  ALIGN_TOOL_PICK_GUIDE,
  ALIGN_TOOL_ADD_GUIDE,
  ALIGN_TOOL_PICK_PATH,
  ALIGN_TOOL_ADD_PATH,
  ALIGN_TOOL_DRAG_BOX
} PicmanAlignToolFunction;


#define PICMAN_TYPE_ALIGN_TOOL            (picman_align_tool_get_type ())
#define PICMAN_ALIGN_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ALIGN_TOOL, PicmanAlignTool))
#define PICMAN_ALIGN_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ALIGN_TOOL, PicmanAlignToolClass))
#define PICMAN_IS_ALIGN_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ALIGN_TOOL))
#define PICMAN_IS_ALIGN_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ALIGN_TOOL))
#define PICMAN_ALIGN_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ALIGN_TOOL, PicmanAlignToolClass))

#define PICMAN_ALIGN_TOOL_GET_OPTIONS(t)  (PICMAN_ALIGN_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanAlignTool      PicmanAlignTool;
typedef struct _PicmanAlignToolClass PicmanAlignToolClass;

struct _PicmanAlignTool
{
  PicmanDrawTool           parent_instance;

  PicmanAlignToolFunction  function;
  GList                 *selected_objects;

  gint                   x1, y1, x2, y2;   /* rubber-band rectangle */

  gboolean               set_reference;
};

struct _PicmanAlignToolClass
{
  PicmanDrawToolClass parent_class;
};


void    picman_align_tool_register (PicmanToolRegisterCallback  callback,
                                 gpointer                  data);

GType   picman_align_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_ALIGN_TOOL_H__  */
