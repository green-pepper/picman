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

#ifndef __PICMAN_CLONE_H__
#define __PICMAN_CLONE_H__


#include "picmansourcecore.h"


#define PICMAN_TYPE_CLONE            (picman_clone_get_type ())
#define PICMAN_CLONE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CLONE, PicmanClone))
#define PICMAN_CLONE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CLONE, PicmanCloneClass))
#define PICMAN_IS_CLONE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CLONE))
#define PICMAN_IS_CLONE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CLONE))
#define PICMAN_CLONE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CLONE, PicmanCloneClass))


typedef struct _PicmanCloneClass PicmanCloneClass;

struct _PicmanClone
{
  PicmanSourceCore  parent_instance;
};

struct _PicmanCloneClass
{
  PicmanSourceCoreClass  parent_class;
};


void    picman_clone_register (Picman                      *picman,
                             PicmanPaintRegisterCallback  callback);

GType   picman_clone_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_CLONE_H__  */
