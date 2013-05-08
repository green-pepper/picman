/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_DYNAMICS_OUTPUT_H__
#define __PICMAN_DYNAMICS_OUTPUT_H__


#include "picmanobject.h"


#define PICMAN_TYPE_DYNAMICS_OUTPUT            (picman_dynamics_output_get_type ())
#define PICMAN_DYNAMICS_OUTPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT, PicmanDynamicsOutput))
#define PICMAN_DYNAMICS_OUTPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DYNAMICS_OUTPUT, PicmanDynamicsOutputClass))
#define PICMAN_IS_DYNAMICS_OUTPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT))
#define PICMAN_IS_DYNAMICS_OUTPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DYNAMICS_OUTPUT))
#define PICMAN_DYNAMICS_OUTPUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DYNAMICS_OUTPUT, PicmanDynamicsOutputClass))


typedef struct _PicmanDynamicsOutputClass PicmanDynamicsOutputClass;

struct _PicmanDynamicsOutput
{
  PicmanObject  parent_instance;
};

struct _PicmanDynamicsOutputClass
{
  PicmanObjectClass  parent_class;
};


GType                picman_dynamics_output_get_type (void) G_GNUC_CONST;

PicmanDynamicsOutput * picman_dynamics_output_new      (const gchar            *name,
                                                    PicmanDynamicsOutputType  type);

gboolean   picman_dynamics_output_is_enabled         (PicmanDynamicsOutput *output);

gdouble    picman_dynamics_output_get_linear_value   (PicmanDynamicsOutput *output,
                                                    const PicmanCoords   *coords,
                                                    PicmanPaintOptions   *options,
                                                    gdouble             fade_point);

gdouble    picman_dynamics_output_get_angular_value  (PicmanDynamicsOutput *output,
                                                    const PicmanCoords   *coords,
                                                    PicmanPaintOptions   *options,
                                                    gdouble             fade_point);
gdouble    picman_dynamics_output_get_aspect_value   (PicmanDynamicsOutput *output,
                                                    const PicmanCoords   *coords,
                                                    PicmanPaintOptions   *options,
                                                    gdouble             fade_point);


#endif  /*  __PICMAN_DYNAMICS_OUTPUT_H__  */
