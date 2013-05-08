/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
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

#if !defined (__PICMAN_BASE_H_INSIDE__) && !defined (PICMAN_BASE_COMPILATION)
#error "Only <libpicmanbase/picmanbase.h> can be included directly."
#endif

#ifndef __PICMAN_UTILS_H__
#define __PICMAN_UTILS_H__

G_BEGIN_DECLS


gchar         * picman_utf8_strtrim            (const gchar  *str,
                                              gint          max_chars) G_GNUC_MALLOC;
gchar         * picman_any_to_utf8             (const gchar  *str,
                                              gssize        len,
                                              const gchar  *warning_format,
                                              ...) G_GNUC_PRINTF (3, 4) G_GNUC_MALLOC;
const gchar   * picman_filename_to_utf8        (const gchar  *filename);

gchar         * picman_strip_uline             (const gchar  *str) G_GNUC_MALLOC;
gchar         * picman_escape_uline            (const gchar  *str) G_GNUC_MALLOC;

gchar         * picman_canonicalize_identifier (const gchar  *identifier) G_GNUC_MALLOC;

PicmanEnumDesc  * picman_enum_get_desc           (GEnumClass   *enum_class,
                                              gint          value);
gboolean        picman_enum_get_value          (GType         enum_type,
                                              gint          value,
                                              const gchar **value_name,
                                              const gchar **value_nick,
                                              const gchar **value_desc,
                                              const gchar **value_help);
const gchar   * picman_enum_value_get_desc     (GEnumClass   *enum_class,
                                              GEnumValue   *enum_value);
const gchar   * picman_enum_value_get_help     (GEnumClass   *enum_class,
                                              GEnumValue   *enum_value);

PicmanFlagsDesc * picman_flags_get_first_desc    (GFlagsClass  *flags_class,
                                              guint         value);
gboolean        picman_flags_get_first_value   (GType         flags_type,
                                              guint         value,
                                              const gchar **value_name,
                                              const gchar **value_nick,
                                              const gchar **value_desc,
                                              const gchar **value_help);
const gchar   * picman_flags_value_get_desc    (GFlagsClass  *flags_class,
                                              GFlagsValue  *flags_value);
const gchar   * picman_flags_value_get_help    (GFlagsClass  *flags_class,
                                              GFlagsValue  *flags_value);


G_END_DECLS

#endif  /* __PICMAN_UTILS_H__ */
