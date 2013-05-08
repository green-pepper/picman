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

#ifndef __PICMAN_IMAGE_GUIDES_H__
#define __PICMAN_IMAGE_GUIDES_H__


/*  public guide adding API
 */
PicmanGuide * picman_image_add_hguide     (PicmanImage *image,
                                       gint       position,
                                       gboolean   push_undo);
PicmanGuide * picman_image_add_vguide     (PicmanImage *image,
                                       gint       position,
                                       gboolean   push_undo);

/*  internal guide adding API, does not check the guide's position and
 *  is publically declared only to be used from undo
 */
void        picman_image_add_guide      (PicmanImage *image,
                                       PicmanGuide *guide,
                                       gint       position);

void        picman_image_remove_guide   (PicmanImage *image,
                                       PicmanGuide *guide,
                                       gboolean   push_undo);
void        picman_image_move_guide     (PicmanImage *image,
                                       PicmanGuide *guide,
                                       gint       position,
                                       gboolean   push_undo);

GList     * picman_image_get_guides     (PicmanImage *image);
PicmanGuide * picman_image_get_guide      (PicmanImage *image,
                                       guint32    id);
PicmanGuide * picman_image_get_next_guide (PicmanImage *image,
                                       guint32    id,
                                       gboolean  *guide_found);

PicmanGuide * picman_image_find_guide     (PicmanImage *image,
                                       gdouble    x,
                                       gdouble    y,
                                       gdouble    epsilon_x,
                                       gdouble    epsilon_y);


#endif /* __PICMAN_IMAGE_GUIDES_H__ */
