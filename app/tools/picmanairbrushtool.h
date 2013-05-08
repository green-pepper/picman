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

#ifndef __PICMAN_AIRBRUSH_TOOL_H__
#define __PICMAN_AIRBRUSH_TOOL_H__


#include "picmanpaintbrushtool.h"


#define PICMAN_TYPE_AIRBRUSH_TOOL            (picman_airbrush_tool_get_type ())
#define PICMAN_AIRBRUSH_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_AIRBRUSH_TOOL, PicmanAirbrushTool))
#define PICMAN_AIRBRUSH_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_AIRBRUSH_TOOL, PicmanAirbrushToolClass))
#define PICMAN_IS_AIRBRUSH_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_AIRBRUSH_TOOL))
#define PICMAN_IS_AIRBRUSH_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_AIRBRUSH_TOOL))
#define PICMAN_AIRBRUSH_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_AIRBRUSH_TOOL, PicmanAirbrushToolClass))


typedef struct _PicmanAirbrushTool      PicmanAirbrushTool;
typedef struct _PicmanAirbrushToolClass PicmanAirbrushToolClass;

struct _PicmanAirbrushTool
{
  PicmanPaintbrushTool parent_instance;
};

struct _PicmanAirbrushToolClass
{
  PicmanPaintbrushToolClass parent_class;
};


void       picman_airbrush_tool_register (PicmanToolRegisterCallback  callback,
                                        gpointer                  data);

GType      picman_airbrush_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_AIRBRUSH_TOOL_H__  */
