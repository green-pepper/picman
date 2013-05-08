/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmaninputdevicestore.h
 * Copyright (C) 2007  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_INPUT_DEVICE_STORE_H__
#define __PICMAN_INPUT_DEVICE_STORE_H__


#define PICMAN_TYPE_INPUT_DEVICE_STORE    (picman_input_device_store_get_type ())
#define PICMAN_INPUT_DEVICE_STORE(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_INPUT_DEVICE_STORE, PicmanInputDeviceStore))
#define PICMAN_IS_INPUT_DEVICE_STORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_INPUT_DEVICE_STORE))

typedef struct _PicmanInputDeviceStore PicmanInputDeviceStore;


void                   picman_input_device_store_register_types  (GTypeModule           *module);

GType                  picman_input_device_store_get_type        (void);

PicmanInputDeviceStore * picman_input_device_store_new             (void);
gchar                * picman_input_device_store_get_device_file (PicmanInputDeviceStore  *store,
                                                                const gchar           *udi);
GError               * picman_input_device_store_get_error       (PicmanInputDeviceStore  *store);


#endif  /* __PICMAN_INPUT_DEVICE_STORE_H__ */
