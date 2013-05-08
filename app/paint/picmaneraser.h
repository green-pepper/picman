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

#ifndef __PICMAN_ERASER_H__
#define __PICMAN_ERASER_H__


#include "picmanbrushcore.h"


#define PICMAN_TYPE_ERASER            (picman_eraser_get_type ())
#define PICMAN_ERASER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_ERASER, PicmanEraser))
#define PICMAN_ERASER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_ERASER, PicmanEraserClass))
#define PICMAN_IS_ERASER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_ERASER))
#define PICMAN_IS_ERASER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_ERASER))
#define PICMAN_ERASER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_ERASER, PicmanEraserClass))


typedef struct _PicmanEraserClass PicmanEraserClass;

struct _PicmanEraser
{
  PicmanBrushCore  parent_instance;
};

struct _PicmanEraserClass
{
  PicmanBrushCoreClass  parent_class;
};


void    picman_eraser_register (Picman                      *picman,
                              PicmanPaintRegisterCallback  callback);

GType   picman_eraser_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_ERASER_H__  */
