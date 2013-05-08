/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanhuesaturationconfig.h
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

#ifndef __PICMAN_HUE_SATURATION_CONFIG_H__
#define __PICMAN_HUE_SATURATION_CONFIG_H__


#include "core/picmanimagemapconfig.h"


#define PICMAN_TYPE_HUE_SATURATION_CONFIG            (picman_hue_saturation_config_get_type ())
#define PICMAN_HUE_SATURATION_CONFIG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_HUE_SATURATION_CONFIG, PicmanHueSaturationConfig))
#define PICMAN_HUE_SATURATION_CONFIG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PICMAN_TYPE_HUE_SATURATION_CONFIG, PicmanHueSaturationConfigClass))
#define PICMAN_IS_HUE_SATURATION_CONFIG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_HUE_SATURATION_CONFIG))
#define PICMAN_IS_HUE_SATURATION_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PICMAN_TYPE_HUE_SATURATION_CONFIG))
#define PICMAN_HUE_SATURATION_CONFIG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PICMAN_TYPE_HUE_SATURATION_CONFIG, PicmanHueSaturationConfigClass))


typedef struct _PicmanHueSaturationConfigClass PicmanHueSaturationConfigClass;

struct _PicmanHueSaturationConfig
{
  PicmanImageMapConfig  parent_instance;

  PicmanHueRange        range;

  gdouble             hue[7];
  gdouble             saturation[7];
  gdouble             lightness[7];

  gdouble             overlap;
};

struct _PicmanHueSaturationConfigClass
{
  PicmanImageMapConfigClass  parent_class;
};


GType   picman_hue_saturation_config_get_type    (void) G_GNUC_CONST;

void    picman_hue_saturation_config_reset_range (PicmanHueSaturationConfig *config);


#endif /* __PICMAN_HUE_SATURATION_CONFIG_H__ */
