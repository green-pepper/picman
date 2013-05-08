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

#ifndef  __PICMAN_INK_TOOL_H__
#define  __PICMAN_INK_TOOL_H__


#include "picmanpainttool.h"


#define PICMAN_TYPE_INK_TOOL            (picman_ink_tool_get_type ())
#define PICMAN_INK_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INK_TOOL, PicmanInkTool))
#define PICMAN_INK_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_INK_TOOL, PicmanInkToolClass))
#define PICMAN_IS_INK_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INK_TOOL))
#define PICMAN_IS_INK_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_INK_TOOL))
#define PICMAN_INK_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_INK_TOOL, PicmanInkToolClass))


typedef struct _PicmanInkTool      PicmanInkTool;
typedef struct _PicmanInkToolClass PicmanInkToolClass;

struct _PicmanInkTool
{
  PicmanPaintTool parent_instance;
};

struct _PicmanInkToolClass
{
  PicmanPaintToolClass parent_class;
};


void    picman_ink_tool_register (PicmanToolRegisterCallback  callback,
                                gpointer                  data);

GType   picman_ink_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_INK_TOOL_H__  */
