/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanhelp.h
 * Copyright (C) 1999-2000 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_HELP_H__
#define __PICMAN_HELP_H__


/*  the main help function
 *
 *  there should be no need to use it directly
 */
void       picman_help_show (Picman         *picman,
                           PicmanProgress *progress,
                           const gchar  *help_domain,
                           const gchar  *help_id);


/*  checks if the user manual is installed locally
 */
gboolean   picman_help_user_manual_is_installed (Picman *picman);

/*  the configuration changed with respect to the location
 *  of the user manual, invalidate the cached information
 */
void       picman_help_user_manual_changed      (Picman *picman);


#endif /* __PICMAN_HELP_H__ */
