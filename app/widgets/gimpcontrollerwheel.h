/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancontrollerwheel.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_CONTROLLER_WHELL_H__
#define __PICMAN_CONTROLLER_WHEEL_H__


#define PICMAN_ENABLE_CONTROLLER_UNDER_CONSTRUCTION
#include "libpicmanwidgets/picmancontroller.h"


#define PICMAN_TYPE_CONTROLLER_WHEEL            (picman_controller_wheel_get_type ())
#define PICMAN_CONTROLLER_WHEEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CONTROLLER_WHEEL, PicmanControllerWheel))
#define PICMAN_CONTROLLER_WHEEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_CONTROLLER_WHEEL, PicmanControllerWheelClass))
#define PICMAN_IS_CONTROLLER_WHEEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CONTROLLER_WHEEL))
#define PICMAN_IS_CONTROLLER_WHEEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_CONTROLLER_WHEEL))
#define PICMAN_CONTROLLER_WHEEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_CONTROLLER_WHEEL, PicmanControllerWheelClass))


typedef struct _PicmanControllerWheelClass PicmanControllerWheelClass;

struct _PicmanControllerWheel
{
  PicmanController parent_instance;
};

struct _PicmanControllerWheelClass
{
  PicmanControllerClass parent_class;
};


GType      picman_controller_wheel_get_type (void) G_GNUC_CONST;

gboolean   picman_controller_wheel_scroll   (PicmanControllerWheel  *wheel,
                                           const GdkEventScroll *sevent);


#endif /* __PICMAN_CONTROLLER_WHEEL_H__ */
