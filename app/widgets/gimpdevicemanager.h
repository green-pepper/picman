/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandevicemanager.h
 * Copyright (C) 2011 Michael Natterer
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

#ifndef __PICMAN_DEVICE_MANAGER_H__
#define __PICMAN_DEVICE_MANAGER_H__


#include "core/picmanlist.h"


G_BEGIN_DECLS


#define PICMAN_TYPE_DEVICE_MANAGER            (picman_device_manager_get_type ())
#define PICMAN_DEVICE_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DEVICE_MANAGER, PicmanDeviceManager))
#define PICMAN_DEVICE_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DEVICE_MANAGER, PicmanDeviceManagerClass))
#define PICMAN_IS_DEVICE_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DEVICE_MANAGER))
#define PICMAN_IS_DEVICE_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DEVICE_MANAGER))
#define PICMAN_DEVICE_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DEVICE_MANAGER, PicmanDeviceManagerClass))


typedef struct _PicmanDeviceManagerClass PicmanDeviceManagerClass;

struct _PicmanDeviceManager
{
  PicmanList  parent_instance;
};

struct _PicmanDeviceManagerClass
{
  PicmanListClass  parent_class;
};


GType               picman_device_manager_get_type           (void) G_GNUC_CONST;

PicmanDeviceManager * picman_device_manager_new                (Picman *picman);

PicmanDeviceInfo    * picman_device_manager_get_current_device (PicmanDeviceManager *manager);
void                picman_device_manager_set_current_device (PicmanDeviceManager *manager,
                                                            PicmanDeviceInfo    *info);


G_END_DECLS

#endif /* __PICMAN_DEVICE_MANAGER_H__ */
