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

#ifndef __PICMAN_MEASURE_TOOL_H__
#define __PICMAN_MEASURE_TOOL_H__


#include "picmandrawtool.h"


/*  possible measure functions  */
typedef enum
{
  CREATING,
  ADDING,
  MOVING,
  MOVING_ALL,
  GUIDING,
  FINISHED
} MeasureFunction;


#define PICMAN_TYPE_MEASURE_TOOL            (picman_measure_tool_get_type ())
#define PICMAN_MEASURE_TOOL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MEASURE_TOOL, PicmanMeasureTool))
#define PICMAN_MEASURE_TOOL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MEASURE_TOOL, PicmanMeasureToolClass))
#define PICMAN_IS_MEASURE_TOOL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MEASURE_TOOL))
#define PICMAN_IS_MEASURE_TOOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MEASURE_TOOL))
#define PICMAN_MEASURE_TOOL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MEASURE_TOOL, PicmanMeasureToolClass))

#define PICMAN_MEASURE_TOOL_GET_OPTIONS(t)  (PICMAN_MEASURE_OPTIONS (picman_tool_get_options (PICMAN_TOOL (t))))


typedef struct _PicmanMeasureTool      PicmanMeasureTool;
typedef struct _PicmanMeasureToolClass PicmanMeasureToolClass;

struct _PicmanMeasureTool
{
  PicmanDrawTool     parent_instance;

  MeasureFunction  function;    /*  function we're performing  */
  gdouble          mouse_x;     /*  pointer x coordinate       */
  gdouble          mouse_y;     /*  pointer y coordinate       */
  gint             last_x;      /*  last x coordinate          */
  gint             last_y;      /*  last y coordinate          */
  gint             point;       /*  what are we manipulating?  */
  gint             num_points;  /*  how many points?           */
  gint             x[3];        /*  three x coordinates        */
  gint             y[3];        /*  three y coordinates        */
  gdouble          angle1;      /*  first angle                */
  gdouble          angle2;      /*  second angle               */
  gboolean         status_help; /*  help is currently in s.bar */

  PicmanCanvasItem  *handles[3];

  GtkWidget       *dialog;
  GtkWidget       *distance_label[2];
  GtkWidget       *angle_label[2];
  GtkWidget       *width_label[2];
  GtkWidget       *height_label[2];
  GtkWidget       *unit_label[4];
};

struct _PicmanMeasureToolClass
{
  PicmanDrawToolClass parent_class;
};


void    picman_measure_tool_register (PicmanToolRegisterCallback  callback,
                                    gpointer                  data);

GType   picman_measure_tool_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_MEASURE_TOOL_H__  */
