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

#ifndef __PICMAN_DISPLAY_FOREACH_H__
#define __PICMAN_DISPLAY_FOREACH_H__


gboolean        picman_displays_dirty            (Picman      *picman);
PicmanContainer * picman_displays_get_dirty_images (Picman      *picman);
void            picman_displays_delete           (Picman      *picman);
void            picman_displays_close            (Picman      *picman);
void            picman_displays_reconnect        (Picman      *picman,
                                                PicmanImage *old,
                                                PicmanImage *new);

gint            picman_displays_get_num_visible  (Picman      *picman);

void            picman_displays_set_busy         (Picman      *picman);
void            picman_displays_unset_busy       (Picman      *picman);


#endif /*  __PICMAN_DISPLAY_FOREACH_H__  */
