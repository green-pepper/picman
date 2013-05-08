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

#ifndef __PICMAN_CURVES_TOOL_H__
#define __PICMAN_CURVES_TOOL_H__


#include "picmanimagemaptool.h"


#define PICMAN_TYPE_CURVES_TOOL            (picman_curves_tool_get_type ())
#define PICMAN_CURVES_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CURVES_TOOL, PicmanCurvesTool))
#define PICMAN_IS_CURVES_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CURVES_TOOL))
#define PICMAN_CURVES_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CURVES_TOOL, PicmanCurvesToolClass))
#define PICMAN_IS_CURVES_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CURVES_TOOL))


typedef struct _PicmanCurvesTool      PicmanCurvesTool;
typedef struct _PicmanCurvesToolClass PicmanCurvesToolClass;

struct _PicmanCurvesTool
{
  PicmanImageMapTool      parent_instance;

  PicmanCurvesConfig     *config;

  /* dialog */
  gdouble               picked_color[5];

  GtkWidget            *channel_menu;
  GtkWidget            *xrange;
  GtkWidget            *yrange;
  GtkWidget            *graph;
  GtkWidget            *curve_type;

  /* export dialog */
  gboolean              export_old_format;
};

struct _PicmanCurvesToolClass
{
  PicmanImageMapToolClass  parent_class;
};


void    picman_curves_tool_register (PicmanToolRegisterCallback  callback,
                                   gpointer                  data);

GType   picman_curves_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_CURVES_TOOL_H__  */
