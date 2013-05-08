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

#ifndef __PICMAN_DYNAMICS_H__
#define __PICMAN_DYNAMICS_H__


#include "picmandata.h"


#define PICMAN_TYPE_DYNAMICS            (picman_dynamics_get_type ())
#define PICMAN_DYNAMICS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DYNAMICS, PicmanDynamics))
#define PICMAN_DYNAMICS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DYNAMICS, PicmanDynamicsClass))
#define PICMAN_IS_DYNAMICS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DYNAMICS))
#define PICMAN_IS_DYNAMICS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DYNAMICS))
#define PICMAN_DYNAMICS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DYNAMICS, PicmanDynamicsClass))


typedef struct _PicmanDynamicsClass PicmanDynamicsClass;

struct _PicmanDynamics
{
  PicmanData  parent_instance;
};

struct _PicmanDynamicsClass
{
  PicmanDataClass  parent_class;
};


GType                picman_dynamics_get_type     (void) G_GNUC_CONST;

PicmanData           * picman_dynamics_new          (PicmanContext            *context,
                                                 const gchar            *name);
PicmanData           * picman_dynamics_get_standard (PicmanContext            *context);

PicmanDynamicsOutput * picman_dynamics_get_output   (PicmanDynamics           *dynamics,
                                                 PicmanDynamicsOutputType  type);

gdouble         picman_dynamics_get_linear_value  (PicmanDynamics           *dynamics,
                                                 PicmanDynamicsOutputType  type,
                                                 const PicmanCoords       *coords,
                                                 PicmanPaintOptions       *options,
                                                 gdouble                 fade_point);

gdouble         picman_dynamics_get_angular_value (PicmanDynamics           *dynamics,
                                                 PicmanDynamicsOutputType  type,
                                                 const PicmanCoords       *coords,
                                                 PicmanPaintOptions       *options,
                                                 gdouble                 fade_point);

gdouble         picman_dynamics_get_aspect_value  (PicmanDynamics           *dynamics,
                                                 PicmanDynamicsOutputType  type,
                                                 const PicmanCoords       *coords,
                                                 PicmanPaintOptions       *options,
                                                 gdouble                 fade_point);


#endif  /*  __PICMAN_DYNAMICS_H__  */
