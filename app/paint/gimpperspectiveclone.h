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

#ifndef __PICMAN_PERSPECTIVE_CLONE_H__
#define __PICMAN_PERSPECTIVE_CLONE_H__


#include "picmanclone.h"


#define PICMAN_TYPE_PERSPECTIVE_CLONE            (picman_perspective_clone_get_type ())
#define PICMAN_PERSPECTIVE_CLONE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE, PicmanPerspectiveClone))
#define PICMAN_PERSPECTIVE_CLONE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE, PicmanPerspectiveCloneClass))
#define PICMAN_IS_PERSPECTIVE_CLONE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE))
#define PICMAN_IS_PERSPECTIVE_CLONE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE))
#define PICMAN_PERSPECTIVE_CLONE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE, PicmanPerspectiveCloneClass))


typedef struct _PicmanPerspectiveCloneClass PicmanPerspectiveCloneClass;

struct _PicmanPerspectiveClone
{
  PicmanClone      parent_instance;

  gdouble        src_x_fv;     /* source coords in front_view perspective */
  gdouble        src_y_fv;

  gdouble        dest_x_fv;    /* destination coords in front_view perspective */
  gdouble        dest_y_fv;

  PicmanMatrix3    transform;
  PicmanMatrix3    transform_inv;

  GeglNode      *node;
  GeglNode      *crop;
  GeglNode      *transform_node;
  GeglNode      *dest_node;
};

struct _PicmanPerspectiveCloneClass
{
  PicmanCloneClass  parent_class;
};


void    picman_perspective_clone_register      (Picman                      *picman,
                                              PicmanPaintRegisterCallback  callback);

GType   picman_perspective_clone_get_type         (void) G_GNUC_CONST;

void    picman_perspective_clone_set_transform    (PicmanPerspectiveClone   *clone,
                                                 PicmanMatrix3            *transform);
void    picman_perspective_clone_get_source_point (PicmanPerspectiveClone   *clone,
                                                 gdouble                 x,
                                                 gdouble                 y,
                                                 gdouble                *newx,
                                                 gdouble                *newy);


#endif  /*  __PICMAN_PERSPECTIVE_CLONE_H__  */
