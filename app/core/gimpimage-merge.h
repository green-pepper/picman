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

#ifndef __PICMAN_IMAGE_MERGE_H__
#define __PICMAN_IMAGE_MERGE_H__


PicmanLayer   * picman_image_merge_visible_layers  (PicmanImage      *image,
                                                PicmanContext    *context,
                                                PicmanMergeType   merge_type,
                                                gboolean        merge_active_group,
                                                gboolean        discard_invisible);
PicmanLayer   * picman_image_merge_down            (PicmanImage      *image,
                                                PicmanLayer      *current_layer,
                                                PicmanContext    *context,
                                                PicmanMergeType   merge_type,
                                                GError        **error);
PicmanLayer   * picman_image_merge_group_layer     (PicmanImage      *image,
                                                PicmanGroupLayer *group);

PicmanLayer   * picman_image_flatten               (PicmanImage      *image,
                                                PicmanContext    *context);

PicmanVectors * picman_image_merge_visible_vectors (PicmanImage      *image,
                                                GError        **error);


#endif /* __PICMAN_IMAGE_MERGE_H__ */
