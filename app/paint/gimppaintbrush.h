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

#ifndef __PICMAN_PAINTBRUSH_H__
#define __PICMAN_PAINTBRUSH_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_PAINTBRUSH            (picman_paintbrush_get_type ())
#define PICMAN_PAINTBRUSH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PAINTBRUSH, PicmanPaintbrush))
#define PICMAN_PAINTBRUSH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PAINTBRUSH, PicmanPaintbrushClass))
#define PICMAN_IS_PAINTBRUSH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PAINTBRUSH))
#define PICMAN_IS_PAINTBRUSH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PAINTBRUSH))
#define PICMAN_PAINTBRUSH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PAINTBRUSH, PicmanPaintbrushClass))


typedef struct _PicmanPaintbrushClass PicmanPaintbrushClass;

struct _PicmanPaintbrush
{
  PicmanBrushCore  parent_instance;
};

struct _PicmanPaintbrushClass
{
  PicmanBrushCoreClass  parent_class;
};


void    picman_paintbrush_register (Picman                      *picman,
                                  PicmanPaintRegisterCallback  callback);

GType   picman_paintbrush_get_type (void) G_GNUC_CONST;


/*  protected  */

void    _picman_paintbrush_motion  (PicmanPaintCore             *paint_core,
                                  PicmanDrawable              *drawable,
                                  PicmanPaintOptions          *paint_options,
                                  const PicmanCoords          *coords,
                                  gdouble                    opacity);


#endif  /*  __PICMAN_PAINTBRUSH_H__  */
