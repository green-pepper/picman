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

#ifndef __PICMAN_DEVICE_INFO_COORDS_H__
#define __PICMAN_DEVICE_INFO_COORDS_H__


gboolean picman_device_info_get_event_coords   (PicmanDeviceInfo  *info,
                                              GdkWindow       *window,
                                              const GdkEvent  *event,
                                              PicmanCoords      *coords);
void     picman_device_info_get_device_coords  (PicmanDeviceInfo  *info,
                                              GdkWindow       *window,
                                              PicmanCoords      *coords);

void     picman_device_info_get_time_coords    (PicmanDeviceInfo  *info,
                                              GdkTimeCoord    *event,
                                              PicmanCoords      *coords);

gboolean picman_device_info_get_event_state    (PicmanDeviceInfo  *info,
                                              GdkWindow       *window,
                                              const GdkEvent  *event,
                                              GdkModifierType *state);
void     picman_device_info_get_device_state   (PicmanDeviceInfo  *info,
                                              GdkWindow       *window,
                                              GdkModifierType *state);


#endif /* __PICMAN_DEVICE_INFO_COORDS_H__ */
