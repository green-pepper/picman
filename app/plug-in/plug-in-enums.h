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

#ifndef __PLUG_IN_ENUMS_H__
#define __PLUG_IN_ENUMS_H__


#define PICMAN_TYPE_PLUG_IN_IMAGE_TYPE (picman_plug_in_image_type_get_type ())

GType picman_plug_in_image_type_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_PLUG_IN_RGB_IMAGE      = 1 << 0,
  PICMAN_PLUG_IN_GRAY_IMAGE     = 1 << 1,
  PICMAN_PLUG_IN_INDEXED_IMAGE  = 1 << 2,
  PICMAN_PLUG_IN_RGBA_IMAGE     = 1 << 3,
  PICMAN_PLUG_IN_GRAYA_IMAGE    = 1 << 4,
  PICMAN_PLUG_IN_INDEXEDA_IMAGE = 1 << 5
} PicmanPlugInImageType;


#define PICMAN_TYPE_PLUG_CALL_MODE (picman_plug_in_call_mode_get_type ())

GType picman_plug_in_call_mode_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_PLUG_IN_CALL_NONE,
  PICMAN_PLUG_IN_CALL_RUN,
  PICMAN_PLUG_IN_CALL_QUERY,
  PICMAN_PLUG_IN_CALL_INIT
} PicmanPlugInCallMode;


#endif /* __PLUG_IN_ENUMS_H__ */
