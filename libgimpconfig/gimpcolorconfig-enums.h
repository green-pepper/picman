/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanColorConfig enums
 * Copyright (C) 2004  Stefan Döhla <stefan@doehla.de>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __PICMAN_COLOR_CONFIG_ENUMS_H__
#define __PICMAN_COLOR_CONFIG_ENUMS_H__


#define PICMAN_TYPE_COLOR_MANAGEMENT_MODE (picman_color_management_mode_get_type ())

GType picman_color_management_mode_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_MANAGEMENT_OFF,       /*< desc="No color management"   >*/
  PICMAN_COLOR_MANAGEMENT_DISPLAY,   /*< desc="Color managed display" >*/
  PICMAN_COLOR_MANAGEMENT_SOFTPROOF  /*< desc="Print simulation"      >*/
} PicmanColorManagementMode;


#define PICMAN_TYPE_COLOR_RENDERING_INTENT \
  (picman_color_rendering_intent_get_type ())

GType picman_color_rendering_intent_get_type (void) G_GNUC_CONST;

typedef enum
{
  PICMAN_COLOR_RENDERING_INTENT_PERCEPTUAL,            /*< desc="Perceptual"            >*/
  PICMAN_COLOR_RENDERING_INTENT_RELATIVE_COLORIMETRIC, /*< desc="Relative colorimetric" >*/
  PICMAN_COLOR_RENDERING_INTENT_SATURATION,            /*< desc="Saturation"            >*/
  PICMAN_COLOR_RENDERING_INTENT_ABSOLUTE_COLORIMETRIC  /*< desc="Absolute colorimetric" >*/
} PicmanColorRenderingIntent;



#endif /* PICMAN_COLOR_CONFIG_ENUMS_H__ */
