/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * file-utils.h
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

#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__


#include <gdk-pixbuf/gdk-pixbuf.h>


gboolean      file_utils_filename_is_uri      (const gchar   *filename,
                                               GError       **error);
gchar       * file_utils_filename_to_uri      (Picman          *picman,
                                               const gchar   *filename,
                                               GError       **error);
gchar       * file_utils_any_to_uri           (Picman          *picman,
                                               const gchar   *filename_or_uri,
                                               GError       **error);
gchar       * file_utils_filename_from_uri    (const gchar   *uri);
gchar       * file_utils_uri_with_new_ext     (const gchar   *uri,
                                               const gchar   *uri_with_ext);
const gchar * file_utils_uri_get_ext          (const gchar   *uri);

gchar       * file_utils_uri_to_utf8_filename (const gchar   *uri);

gchar       * file_utils_uri_display_basename (const gchar   *uri);
gchar       * file_utils_uri_display_name     (const gchar   *uri);

GdkPixbuf   * file_utils_load_thumbnail       (const gchar   *filename);
gboolean      file_utils_save_thumbnail       (PicmanImage     *image,
                                               const gchar   *filename);


#endif /* __FILE_UTILS_H__ */
