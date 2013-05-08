/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandrawable-shadow.h
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

#ifndef __PICMAN_DRAWABLE_SHADOW_H__
#define __PICMAN_DRAWABLE_SHADOW_H__


GeglBuffer * picman_drawable_get_shadow_buffer   (PicmanDrawable *drawable);
void         picman_drawable_free_shadow_buffer  (PicmanDrawable *drawable);

void         picman_drawable_merge_shadow_buffer (PicmanDrawable *drawable,
                                                gboolean      push_undo,
                                                const gchar  *undo_desc);


#endif /* __PICMAN_DRAWABLE_SHADOW_H__ */
