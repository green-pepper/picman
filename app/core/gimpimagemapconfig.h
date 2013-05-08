/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanimagemapconfig.h
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_IMAGE_MAP_CONFIG_H__
#define __PICMAN_IMAGE_MAP_CONFIG_H__


#include "picmanviewable.h"


#define PICMAN_TYPE_IMAGE_MAP_CONFIG            (picman_image_map_config_get_type ())
#define PICMAN_IMAGE_MAP_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_MAP_CONFIG, PicmanImageMapConfig))
#define PICMAN_IMAGE_MAP_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_IMAGE_MAP_CONFIG, PicmanImageMapConfigClass))
#define PICMAN_IS_IMAGE_MAP_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_MAP_CONFIG))
#define PICMAN_IS_IMAGE_MAP_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_IMAGE_MAP_CONFIG))
#define PICMAN_IMAGE_MAP_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_IMAGE_MAP_CONFIG, PicmanImageMapConfigClass))


typedef struct _PicmanImageMapConfigClass PicmanImageMapConfigClass;

struct _PicmanImageMapConfig
{
  PicmanViewable      parent_instance;

  guint             time;
};

struct _PicmanImageMapConfigClass
{
  PicmanViewableClass  parent_class;
};


GType   picman_image_map_config_get_type (void) G_GNUC_CONST;

gint    picman_image_map_config_compare  (PicmanImageMapConfig *a,
                                        PicmanImageMapConfig *b);


#endif /* __PICMAN_IMAGE_MAP_CONFIG_H__ */
