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

#ifndef __PICMAN_AIRBRUSH_H__
#define __PICMAN_AIRBRUSH_H__


#include "picmanpaintbrush.h"


#define PICMAN_TYPE_AIRBRUSH            (picman_airbrush_get_type ())
#define PICMAN_AIRBRUSH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_AIRBRUSH, PicmanAirbrush))
#define PICMAN_AIRBRUSH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_AIRBRUSH, PicmanAirbrushClass))
#define PICMAN_IS_AIRBRUSH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_AIRBRUSH))
#define PICMAN_IS_AIRBRUSH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_AIRBRUSH))
#define PICMAN_AIRBRUSH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_AIRBRUSH, PicmanAirbrushClass))


typedef struct _PicmanAirbrushClass PicmanAirbrushClass;

struct _PicmanAirbrush
{
  PicmanPaintbrush    parent_instance;

  guint             timeout_id;
  PicmanDrawable     *drawable;
  PicmanPaintOptions *paint_options;
};

struct _PicmanAirbrushClass
{
  PicmanPaintbrushClass  parent_class;
};


void    picman_airbrush_register (Picman                      *picman,
                                PicmanPaintRegisterCallback  callback);

GType   picman_airbrush_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_AIRBRUSH_H__  */
