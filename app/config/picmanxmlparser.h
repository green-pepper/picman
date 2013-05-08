/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * PicmanXmlParser
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

#ifndef __PICMAN_XML_PARSER_H__
#define __PICMAN_XML_PARSER_H__


PicmanXmlParser * picman_xml_parser_new              (const GMarkupParser *markup_parser,
                                                  gpointer             user_data);
gboolean        picman_xml_parser_parse_file       (PicmanXmlParser       *parser,
                                                  const gchar         *filename,
                                                  GError             **error);
gboolean        picman_xml_parser_parse_fd         (PicmanXmlParser       *parser,
                                                  gint                 fd,
                                                  GError             **error);
gboolean        picman_xml_parser_parse_io_channel (PicmanXmlParser       *parser,
                                                  GIOChannel          *io,
                                                  GError             **error);
gboolean        picman_xml_parser_parse_buffer     (PicmanXmlParser       *parser,
                                                  const gchar         *buffer,
                                                  gssize               len,
                                                  GError             **error);
void            picman_xml_parser_free             (PicmanXmlParser       *parser);


#endif  /* __PICMAN_XML_PARSER_H__ */
