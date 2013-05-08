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

#ifndef __TEXT_ENUMS_H__
#define __TEXT_ENUMS_H__


#define PICMAN_TYPE_TEXT_BOX_MODE (picman_text_box_mode_get_type ())

GType picman_text_box_mode_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_TEXT_BOX_DYNAMIC, /*< desc="Dynamic" >*/
  PICMAN_TEXT_BOX_FIXED    /*< desc="Fixed"   >*/
} PicmanTextBoxMode;


#define PICMAN_TYPE_TEXT_OUTLINE (picman_text_outline_get_type ())

GType picman_text_outline_get_type (void) G_GNUC_CONST;

typedef enum /*< pdb-skip >*/
{
  PICMAN_TEXT_OUTLINE_NONE,
  PICMAN_TEXT_OUTLINE_STROKE_ONLY,
  PICMAN_TEXT_OUTLINE_STROKE_FILL
} PicmanTextOutline;


#endif /* __TEXT_ENUMS_H__ */
