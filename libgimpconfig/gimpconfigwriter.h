/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanConfigWriter
 * Copyright (C) 2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_CONFIG_WRITER_H__
#define __PICMAN_CONFIG_WRITER_H__


PicmanConfigWriter * picman_config_writer_new_file     (const gchar       *filename,
                                                    gboolean           atomic,
                                                    const gchar       *header,
                                                    GError           **error);
PicmanConfigWriter * picman_config_writer_new_fd       (gint               fd);
PicmanConfigWriter * picman_config_writer_new_string   (GString           *string);

void               picman_config_writer_open         (PicmanConfigWriter  *writer,
                                                    const gchar       *name);
void               picman_config_writer_comment_mode (PicmanConfigWriter  *writer,
                                                    gboolean           enable);

void               picman_config_writer_print        (PicmanConfigWriter  *writer,
                                                    const gchar       *string,
                                                    gint               len);
void               picman_config_writer_printf       (PicmanConfigWriter  *writer,
                                                    const gchar       *format,
                                                    ...) G_GNUC_PRINTF (2, 3);
void               picman_config_writer_identifier   (PicmanConfigWriter  *writer,
                                                    const gchar       *identifier);
void               picman_config_writer_string       (PicmanConfigWriter  *writer,
                                                    const gchar       *string);
void               picman_config_writer_data         (PicmanConfigWriter  *writer,
                                                    gint               length,
                                                    const guint8      *data);
void               picman_config_writer_comment      (PicmanConfigWriter  *writer,
                                                    const gchar       *comment);
void               picman_config_writer_linefeed     (PicmanConfigWriter  *writer);


void               picman_config_writer_revert       (PicmanConfigWriter  *writer);
void               picman_config_writer_close        (PicmanConfigWriter  *writer);
gboolean           picman_config_writer_finish       (PicmanConfigWriter  *writer,
                                                    const gchar       *footer,
                                                    GError           **error);


#endif /* __PICMAN_CONFIG_WRITER_H__ */
