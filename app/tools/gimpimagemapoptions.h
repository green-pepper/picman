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

#ifndef __PICMAN_IMAGE_MAP_OPTIONS_H__
#define __PICMAN_IMAGE_MAP_OPTIONS_H__


#include "core/picmantooloptions.h"


#define PICMAN_TYPE_IMAGE_MAP_OPTIONS            (picman_image_map_options_get_type ())
#define PICMAN_IMAGE_MAP_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_MAP_OPTIONS, PicmanImageMapOptions))
#define PICMAN_IMAGE_MAP_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_MAP_OPTIONS, PicmanImageMapOptionsClass))
#define PICMAN_IS_IMAGE_MAP_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_MAP_OPTIONS))
#define PICMAN_IS_IMAGE_MAP_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_MAP_OPTIONS))
#define PICMAN_IMAGE_MAP_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_MAP_OPTIONS, PicmanImageMapOptionsClass))


typedef struct _PicmanToolOptionsClass  PicmanImageMapOptionsClass;

struct _PicmanImageMapOptions
{
  PicmanToolOptions  parent_instance;

  gboolean         preview;
  gchar           *settings;
};


GType  picman_image_map_options_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_IMAGE_MAP_OPTIONS_H__ */
