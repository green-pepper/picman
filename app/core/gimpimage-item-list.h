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

#ifndef __PICMAN_IMAGE_ITEM_LIST_H__
#define __PICMAN_IMAGE_ITEM_LIST_H__


void    picman_image_item_list_translate (PicmanImage              *image,
                                        GList                  *list,
                                        gint                    offset_x,
                                        gint                    offset_y,
                                        gboolean                push_undo);
void    picman_image_item_list_flip      (PicmanImage              *image,
                                        GList                  *list,
                                        PicmanContext            *context,
                                        PicmanOrientationType     flip_type,
                                        gdouble                 axis,
                                        gboolean                clip_result);
void    picman_image_item_list_rotate    (PicmanImage              *image,
                                        GList                  *list,
                                        PicmanContext            *context,
                                        PicmanRotationType        rotate_type,
                                        gdouble                 center_x,
                                        gdouble                 center_y,
                                        gboolean                clip_result);
void    picman_image_item_list_transform (PicmanImage              *image,
                                        GList                  *list,
                                        PicmanContext            *context,
                                        const PicmanMatrix3      *matrix,
                                        PicmanTransformDirection  direction,
                                        PicmanInterpolationType   interpolation_type,
                                        gint                    recursion_level,
                                        PicmanTransformResize     clip_result,
                                        PicmanProgress           *progress);

GList * picman_image_item_list_get_list  (const PicmanImage        *image,
                                        const PicmanItem         *exclude,
                                        PicmanItemTypeMask        type,
                                        PicmanItemSet             set);

GList * picman_image_item_list_filter    (const PicmanItem         *exclude,
                                        GList                  *list,
                                        gboolean                remove_children,
                                        gboolean                remove_locked);


#endif /* __PICMAN_IMAGE_ITEM_LIST_H__ */
