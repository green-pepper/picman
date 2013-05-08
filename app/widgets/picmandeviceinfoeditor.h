/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandeviceinfoeditor.h
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DEVICE_INFO_EDITOR_H__
#define __PICMAN_DEVICE_INFO_EDITOR_H__


#define PICMAN_TYPE_DEVICE_INFO_EDITOR            (picman_device_info_editor_get_type ())
#define PICMAN_DEVICE_INFO_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DEVICE_INFO_EDITOR, PicmanDeviceInfoEditor))
#define PICMAN_DEVICE_INFO_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DEVICE_INFO_EDITOR, PicmanDeviceInfoEditorClass))
#define PICMAN_IS_DEVICE_INFO_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DEVICE_INFO_EDITOR))
#define PICMAN_IS_DEVICE_INFO_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DEVICE_INFO_EDITOR))
#define PICMAN_DEVICE_INFO_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DEVICE_INFO_EDITOR, PicmanDeviceInfoEditorClass))


typedef struct _PicmanDeviceInfoEditorClass PicmanDeviceInfoEditorClass;

struct _PicmanDeviceInfoEditor
{
  GtkBox  parent_instance;
};

struct _PicmanDeviceInfoEditorClass
{
  GtkBoxClass  parent_class;
};


GType       picman_device_info_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_device_info_editor_new      (PicmanDeviceInfo *info);


#endif /* __PICMAN_DEVICE_INFO_EDITOR_H__ */
