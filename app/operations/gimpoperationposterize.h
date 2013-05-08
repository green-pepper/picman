/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationposterize.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_POSTERIZE_H__
#define __PICMAN_OPERATION_POSTERIZE_H__


#include "picmanoperationpointfilter.h"


#define PICMAN_TYPE_OPERATION_POSTERIZE            (picman_operation_posterize_get_type ())
#define PICMAN_OPERATION_POSTERIZE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_POSTERIZE, PicmanOperationPosterize))
#define PICMAN_OPERATION_POSTERIZE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_POSTERIZE, PicmanOperationPosterizeClass))
#define PICMAN_IS_OPERATION_POSTERIZE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_POSTERIZE))
#define PICMAN_IS_OPERATION_POSTERIZE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_POSTERIZE))
#define PICMAN_OPERATION_POSTERIZE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_POSTERIZE, PicmanOperationPosterizeClass))


typedef struct _PicmanOperationPosterize      PicmanOperationPosterize;
typedef struct _PicmanOperationPosterizeClass PicmanOperationPosterizeClass;

struct _PicmanOperationPosterize
{
  PicmanOperationPointFilter  parent_instance;
};

struct _PicmanOperationPosterizeClass
{
  PicmanOperationPointFilterClass  parent_class;
};


GType   picman_operation_posterize_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_POSTERIZE_H__ */
