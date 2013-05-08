/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * picmancontainer-filter.c
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CONTAINER_FILTER_H__
#define __PICMAN_CONTAINER_FILTER_H__


PicmanContainer * picman_container_filter         (const PicmanContainer  *container,
                                               PicmanObjectFilterFunc  filter,
                                               gpointer              user_data);
PicmanContainer * picman_container_filter_by_name (const PicmanContainer  *container,
                                               const gchar          *regexp,
                                               GError              **error);

gchar        ** picman_container_get_filtered_name_array
                                              (const PicmanContainer  *container,
                                               const gchar          *regexp,
                                               gint                 *length);


#endif  /* __PICMAN_CONTAINER_FILTER_H__ */
