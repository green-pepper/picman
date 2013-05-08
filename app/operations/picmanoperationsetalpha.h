/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationsetalpha.h
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_OPERATION_SET_ALPHA_H__
#define __PICMAN_OPERATION_SET_ALPHA_H__


#include <gegl-plugin.h>


#define PICMAN_TYPE_OPERATION_SET_ALPHA            (picman_operation_set_alpha_get_type ())
#define PICMAN_OPERATION_SET_ALPHA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_OPERATION_SET_ALPHA, PicmanOperationSetAlpha))
#define PICMAN_OPERATION_SET_ALPHA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_OPERATION_SET_ALPHA, PicmanOperationSetAlphaClass))
#define PICMAN_IS_OPERATION_SET_ALPHA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_OPERATION_SET_ALPHA))
#define PICMAN_IS_OPERATION_SET_ALPHA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_OPERATION_SET_ALPHA))
#define PICMAN_OPERATION_SET_ALPHA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_OPERATION_SET_ALPHA, PicmanOperationSetAlphaClass))


typedef struct _PicmanOperationSetAlpha      PicmanOperationSetAlpha;
typedef struct _PicmanOperationSetAlphaClass PicmanOperationSetAlphaClass;

struct _PicmanOperationSetAlpha
{
  GeglOperationPointComposer  parent_instance;

  gdouble                     value;
};

struct _PicmanOperationSetAlphaClass
{
  GeglOperationPointComposerClass  parent_class;
};


GType   picman_operation_set_alpha_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_OPERATION_SET_ALPHA_H__ */
