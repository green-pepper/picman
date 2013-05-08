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

#ifndef __PICMAN_CONVOLVE_H__
#define __PICMAN_CONVOLVE_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_CONVOLVE            (picman_convolve_get_type ())
#define PICMAN_CONVOLVE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONVOLVE, PicmanConvolve))
#define PICMAN_CONVOLVE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONVOLVE, PicmanConvolveClass))
#define PICMAN_IS_CONVOLVE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONVOLVE))
#define PICMAN_IS_CONVOLVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONVOLVE))
#define PICMAN_CONVOLVE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONVOLVE, PicmanConvolveClass))


typedef struct _PicmanConvolveClass PicmanConvolveClass;

struct _PicmanConvolve
{
  PicmanBrushCore  parent_instance;
  gfloat         matrix[9];
  gfloat         matrix_divisor;
};

struct _PicmanConvolveClass
{
  PicmanBrushCoreClass parent_class;
};


void    picman_convolve_register (Picman                      *picman,
                                PicmanPaintRegisterCallback  callback);

GType   picman_convolve_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_CONVOLVE_H__  */
