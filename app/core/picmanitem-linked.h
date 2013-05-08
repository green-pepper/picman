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

#ifndef __PICMAN_ITEM_LINKED_H__
#define __PICMAN_ITEM_LINKED_H__


gboolean picman_item_linked_is_locked (const PicmanItem         *item);

void     picman_item_linked_translate (PicmanItem               *item,
                                     gint                    offset_x,
                                     gint                    offset_y,
                                     gboolean                push_undo);
void     picman_item_linked_flip      (PicmanItem               *item,
                                     PicmanContext            *context,
                                     PicmanOrientationType     flip_type,
                                     gdouble                 axis,
                                     gboolean                clip_result);
void     picman_item_linked_rotate    (PicmanItem               *item,
                                     PicmanContext            *context,
                                     PicmanRotationType        rotate_type,
                                     gdouble                 center_x,
                                     gdouble                 center_y,
                                     gboolean                clip_result);
void     picman_item_linked_transform (PicmanItem               *item,
                                     PicmanContext            *context,
                                     const PicmanMatrix3      *matrix,
                                     PicmanTransformDirection  direction,
                                     PicmanInterpolationType   interpolation_type,
                                     gint                    recursion_level,
                                     PicmanTransformResize     clip_result,
                                     PicmanProgress           *progress);


#endif /* __PICMAN_ITEM_LINKED_H__ */
