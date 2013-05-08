/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picman-contexts.h
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

#ifndef __PICMAN_CONTEXTS_H__
#define __PICMAN_CONTEXTS_H__


void       picman_contexts_init  (Picman    *picman);
void       picman_contexts_exit  (Picman    *picman);

gboolean   picman_contexts_load  (Picman    *picman,
                                GError **error);
gboolean   picman_contexts_save  (Picman    *picman,
                                GError **error);

gboolean   picman_contexts_clear (Picman    *picman,
                                GError **error);


#endif  /*  __PICMAN_CONTEXTS_H__  */
