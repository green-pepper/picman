/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandesaturateconfig.h
 * Copyright (C) 2008 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_DESATURATE_CONFIG_H__
#define __PICMAN_DESATURATE_CONFIG_H__


#include "core/picmanimagemapconfig.h"


#define PICMAN_TYPE_DESATURATE_CONFIG            (picman_desaturate_config_get_type ())
#define PICMAN_DESATURATE_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DESATURATE_CONFIG, PicmanDesaturateConfig))
#define PICMAN_DESATURATE_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_DESATURATE_CONFIG, PicmanDesaturateConfigClass))
#define PICMAN_IS_DESATURATE_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DESATURATE_CONFIG))
#define PICMAN_IS_DESATURATE_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_DESATURATE_CONFIG))
#define PICMAN_DESATURATE_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_DESATURATE_CONFIG, PicmanDesaturateConfigClass))


typedef struct _PicmanDesaturateConfigClass PicmanDesaturateConfigClass;

struct _PicmanDesaturateConfig
{
  PicmanImageMapConfig  parent_instance;

  PicmanDesaturateMode  mode;
};

struct _PicmanDesaturateConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType   picman_desaturate_config_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_DESATURATE_CONFIG_H__ */
