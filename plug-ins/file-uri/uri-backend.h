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

#ifndef __URI_BACKEND_H__
#define __URI_BACKEND_H__


gboolean      uri_backend_init               (const gchar  *plugin_name,
                                              gboolean      run,
                                              PicmanRunMode   run_mode,
                                              GError      **error);
void          uri_backend_shutdown           (void);

const gchar * uri_backend_get_load_help      (void);
const gchar * uri_backend_get_save_help      (void);

const gchar * uri_backend_get_load_protocols (void);
const gchar * uri_backend_get_save_protocols (void);

gboolean      uri_backend_load_image         (const gchar  *uri,
                                              const gchar  *tmpname,
                                              PicmanRunMode   run_mode,
                                              GError      **error);
gboolean      uri_backend_save_image         (const gchar  *uri,
                                              const gchar  *tmpname,
                                              PicmanRunMode   run_mode,
                                              GError      **error);
gchar       * uri_backend_map_image          (const gchar  *uri,
                                              PicmanRunMode   run_mode);


#endif /* __URI_BACKEND_H__ */
