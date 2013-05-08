/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandevicestatus.h
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DEVICE_STATUS_H__
#define __PICMAN_DEVICE_STATUS_H__


#include "picmaneditor.h"


#define PICMAN_TYPE_DEVICE_STATUS            (picman_device_status_get_type ())
#define PICMAN_DEVICE_STATUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DEVICE_STATUS, PicmanDeviceStatus))
#define PICMAN_DEVICE_STATUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DEVICE_STATUS, PicmanDeviceStatusClass))
#define PICMAN_IS_DEVICE_STATUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DEVICE_STATUS))
#define PICMAN_IS_DEVICE_STATUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DEVICE_STATUS))
#define PICMAN_DEVICE_STATUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DEVICE_STATUS, PicmanDeviceStatusClass))


typedef struct _PicmanDeviceStatusEntry PicmanDeviceStatusEntry;
typedef struct _PicmanDeviceStatusClass PicmanDeviceStatusClass;

struct _PicmanDeviceStatus
{
  PicmanEditor      parent_instance;

  Picman           *picman;
  PicmanDeviceInfo *current_device;

  GList          *devices;

  GtkWidget      *vbox;

  GtkWidget      *save_button;
  GtkWidget      *edit_button;
};

struct _PicmanDeviceStatusClass
{
  PicmanEditorClass  parent_class;
};


GType       picman_device_status_get_type (void) G_GNUC_CONST;

GtkWidget * picman_device_status_new      (Picman *picman);


#endif  /*  __PICMAN_DEVICE_STATUS_H__  */
