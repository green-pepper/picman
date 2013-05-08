/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanscanner.h
 * Copyright (C) 2002  Sven Neumann <sven@picman.org>
 *                     Michael Natterer <mitch@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_CONFIG_H_INSIDE__) && !defined (PICMAN_CONFIG_COMPILATION)
#error "Only <libpicmanconfig/picmanconfig.h> can be included directly."
#endif

#ifndef __PICMAN_SCANNER_H__
#define __PICMAN_SCANNER_H__


GScanner * picman_scanner_new_file                 (const gchar  *filename,
                                                  GError      **error);
GScanner * picman_scanner_new_string               (const gchar  *text,
                                                  gint          text_len,
                                                  GError      **error);
void       picman_scanner_destroy                  (GScanner     *scanner);

gboolean   picman_scanner_parse_token              (GScanner     *scanner,
                                                  GTokenType    token);
gboolean   picman_scanner_parse_identifier         (GScanner     *scanner,
                                                  const gchar  *identifier);
gboolean   picman_scanner_parse_string             (GScanner     *scanner,
                                                  gchar       **dest);
gboolean   picman_scanner_parse_string_no_validate (GScanner     *scanner,
                                                  gchar       **dest);
gboolean   picman_scanner_parse_data               (GScanner     *scanner,
                                                  gint          length,
                                                  guint8      **dest);
gboolean   picman_scanner_parse_int                (GScanner     *scanner,
                                                  gint         *dest);
gboolean   picman_scanner_parse_float              (GScanner     *scanner,
                                                  gdouble      *dest);
gboolean   picman_scanner_parse_boolean            (GScanner     *scanner,
                                                  gboolean     *dest);
gboolean   picman_scanner_parse_color              (GScanner     *scanner,
                                                  PicmanRGB      *dest);
gboolean   picman_scanner_parse_matrix2            (GScanner     *scanner,
                                                  PicmanMatrix2  *dest);


#endif /* __PICMAN_SCANNER_H__ */
