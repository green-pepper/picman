/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * File utitility functions for PicmanConfig.
 * Copyright (C) 2001-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CONFIG_FILE_H__
#define __PICMAN_CONFIG_FILE_H__


gboolean   picman_config_file_copy             (const gchar        *source,
                                              const gchar        *dest,
                                              const gchar        *old_options_regexp,
                                              GRegexEvalCallback  update_callback,
                                              GError      **error);
gboolean   picman_config_file_backup_on_error  (const gchar  *filename,
                                              const gchar  *name,
                                              GError      **error);


#endif  /* __PICMAN_CONFIG_FILE_H__ */
