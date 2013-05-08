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

#ifndef __PICMAN_PERSPECTIVE_CLONE_OPTIONS_H__
#define __PICMAN_PERSPECTIVE_CLONE_OPTIONS_H__


#include "picmancloneoptions.h"


#define PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS            (picman_perspective_clone_options_get_type ())
#define PICMAN_PERSPECTIVE_CLONE_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS, PicmanPerspectiveCloneOptions))
#define PICMAN_PERSPECTIVE_CLONE_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS, PicmanPerspectiveCloneOptionsClass))
#define PICMAN_IS_PERSPECTIVE_CLONE_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS))
#define PICMAN_IS_PERSPECTIVE_CLONE_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS))
#define PICMAN_PERSPECTIVE_CLONE_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PERSPECTIVE_CLONE_OPTIONS, PicmanPerspectiveCloneOptionsClass))


typedef struct _PicmanPerspectiveCloneOptionsClass PicmanPerspectiveCloneOptionsClass;

struct _PicmanPerspectiveCloneOptions
{
  PicmanCloneOptions          paint_instance;

  PicmanPerspectiveCloneMode  clone_mode;
};

struct _PicmanPerspectiveCloneOptionsClass
{
  PicmanCloneOptionsClass  parent_class;
};


GType   picman_perspective_clone_options_get_type (void) G_GNUC_CONST;


#endif  /*  __PICMAN_PERSPECTIVE_CLONE_OPTIONS_H__  */
