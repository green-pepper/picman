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

#ifndef __PICMAN_CONVOLVE_TOOL_H__
#define __PICMAN_CONVOLVE_TOOL_H__


#include "picmanbrushtool.h"


#define PICMAN_TYPE_CONVOLVE_TOOL            (picman_convolve_tool_get_type ())
#define PICMAN_CONVOLVE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONVOLVE_TOOL, PicmanConvolveTool))
#define PICMAN_CONVOLVE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONVOLVE_TOOL, PicmanConvolveToolClass))
#define PICMAN_IS_CONVOLVE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONVOLVE_TOOL))
#define PICMAN_IS_CONVOLVE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONVOLVE_TOOL))
#define PICMAN_CONVOLVE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONVOLVE_TOOL, PicmanConvolveToolClass))

#define PICMAN_CONVOLVE_TOOL_GET_OPTIONS(t)  (PICMAN_CONVOLVE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanConvolveTool      PicmanConvolveTool;
typedef struct _PicmanConvolveToolClass PicmanConvolveToolClass;

struct _PicmanConvolveTool
{
  PicmanBrushTool parent_instance;

  gboolean      toggled;
};

struct _PicmanConvolveToolClass
{
  PicmanBrushToolClass parent_class;
};


void    picman_convolve_tool_register (PicmanToolRegisterCallback  callback,
                                     gpointer                  data);

GType   picman_convolve_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_CONVOLVE_TOOL_H__  */
