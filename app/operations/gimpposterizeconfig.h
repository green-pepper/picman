/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanposterizeconfig.h
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_POSTERIZE_CONFIG_H__
#define __PICMAN_POSTERIZE_CONFIG_H__


#include "core/picmanimagemapconfig.h"


#define PICMAN_TYPE_POSTERIZE_CONFIG            (picman_posterize_config_get_type ())
#define PICMAN_POSTERIZE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_POSTERIZE_CONFIG, PicmanPosterizeConfig))
#define PICMAN_POSTERIZE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_POSTERIZE_CONFIG, PicmanPosterizeConfigClass))
#define PICMAN_IS_POSTERIZE_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_POSTERIZE_CONFIG))
#define PICMAN_IS_POSTERIZE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_POSTERIZE_CONFIG))
#define PICMAN_POSTERIZE_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_POSTERIZE_CONFIG, PicmanPosterizeConfigClass))


typedef struct _PicmanPosterizeConfigClass PicmanPosterizeConfigClass;

struct _PicmanPosterizeConfig
{
  PicmanImageMapConfig  parent_instance;

  gint                levels;
};

struct _PicmanPosterizeConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType   picman_posterize_config_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_POSTERIZE_CONFIG_H__ */
