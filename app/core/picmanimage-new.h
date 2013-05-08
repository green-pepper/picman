/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1999 Spencer Kimball and Peter Mattis
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

#ifndef __PICMAN_IMAGE_NEW_H__
#define __PICMAN_IMAGE_NEW_H__


PicmanTemplate * picman_image_new_get_last_template (Picman            *picman,
                                                 PicmanImage       *image);
void           picman_image_new_set_last_template (Picman            *picman,
                                                 PicmanTemplate    *template);

PicmanImage    * picman_image_new_from_template     (Picman            *picman,
                                                 PicmanTemplate    *template,
                                                 PicmanContext     *context);
PicmanImage    * picman_image_new_from_drawable     (Picman            *picman,
                                                 PicmanDrawable    *drawable);
PicmanImage    * picman_image_new_from_component    (Picman            *picman,
                                                 PicmanImage       *image,
                                                 PicmanChannelType  component);
PicmanImage    * picman_image_new_from_buffer       (Picman            *picman,
                                                 PicmanImage       *invoke,
                                                 PicmanBuffer      *paste);
PicmanImage    * picman_image_new_from_pixbuf       (Picman            *picman,
                                                 GdkPixbuf       *pixbuf,
                                                 const gchar     *layer_name);


#endif /* __PICMAN_IMAGE_NEW__ */
