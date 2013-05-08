/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Thumbnail handling according to the Thumbnail Managing Standard.
 * http://triq.net/~pearl/thumbnail-spec/
 *
 * Copyright (C) 2001-2003  Sven Neumann <sven@picman.org>
 *                          Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_THUMB_H_INSIDE__) && !defined (PICMAN_THUMB_COMPILATION)
#error "Only <libpicmanthumb/picmanthumb.h> can be included directly."
#endif

#ifndef __PICMAN_THUMB_UTILS_H__
#define __PICMAN_THUMB_UTILS_H__

G_BEGIN_DECLS


gboolean            picman_thumb_init                   (const gchar    *creator,
                                                       const gchar    *thumb_basedir);

gchar             * picman_thumb_find_thumb             (const gchar    *uri,
                                                       PicmanThumbSize  *size) G_GNUC_MALLOC;

PicmanThumbFileType   picman_thumb_file_test              (const gchar    *filename,
                                                       gint64         *mtime,
                                                       gint64         *size,
                                                       gint           *err_no);

gchar             * picman_thumb_name_from_uri          (const gchar    *uri,
                                                       PicmanThumbSize   size) G_GNUC_MALLOC;
const gchar       * picman_thumb_get_thumb_dir          (PicmanThumbSize   size);
gboolean            picman_thumb_ensure_thumb_dir       (PicmanThumbSize   size,
                                                       GError        **error);
void                picman_thumbs_delete_for_uri        (const gchar    *uri);

gchar             * picman_thumb_name_from_uri_local    (const gchar    *uri,
                                                       PicmanThumbSize   size) G_GNUC_MALLOC;
gchar             * picman_thumb_get_thumb_dir_local    (const gchar    *dirname,
                                                       PicmanThumbSize   size) G_GNUC_MALLOC;
gboolean            picman_thumb_ensure_thumb_dir_local (const gchar    *dirname,
                                                       PicmanThumbSize   size,
                                                       GError        **error);
void                picman_thumbs_delete_for_uri_local  (const gchar    *uri);


/*  for internal use only   */
G_GNUC_INTERNAL void    _picman_thumbs_delete_others    (const gchar    *uri,
                                                       PicmanThumbSize   size);
G_GNUC_INTERNAL gchar * _picman_thumb_filename_from_uri (const gchar    *uri);


G_END_DECLS

#endif /* __PICMAN_THUMB_UTILS_H__ */
