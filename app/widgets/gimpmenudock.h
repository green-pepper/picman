/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmenudock.h
 * Copyright (C) 2001-2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_MENU_DOCK_H__
#define __PICMAN_MENU_DOCK_H__


#include "picmandock.h"


#define PICMAN_TYPE_MENU_DOCK            (picman_menu_dock_get_type ())
#define PICMAN_MENU_DOCK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_MENU_DOCK, PicmanMenuDock))
#define PICMAN_MENU_DOCK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_MENU_DOCK, PicmanMenuDockClass))
#define PICMAN_IS_MENU_DOCK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_MENU_DOCK))
#define PICMAN_IS_MENU_DOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_MENU_DOCK))
#define PICMAN_MENU_DOCK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_MENU_DOCK, PicmanMenuDockClass))

typedef struct _PicmanMenuDockPrivate PicmanMenuDockPrivate;
typedef struct _PicmanMenuDockClass   PicmanMenuDockClass;

struct _PicmanMenuDock
{
  PicmanDock             parent_instance;

  PicmanMenuDockPrivate *p;
};

struct _PicmanMenuDockClass
{
  PicmanDockClass  parent_class;
};


GType       picman_menu_dock_get_type (void) G_GNUC_CONST;

GtkWidget * picman_menu_dock_new      (void);



#endif /* __PICMAN_MENU_DOCK_H__ */
