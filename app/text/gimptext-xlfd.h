/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanText
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_XLFD_H__
#define __PICMAN_TEXT_XLFD_H__


/*  handle X Logical Font Descriptions for compat  */

gchar    * picman_text_font_name_from_xlfd (const gchar *xlfd);
gboolean   picman_text_font_size_from_xlfd (const gchar *xlfd,
                                          gdouble     *size,
                                          PicmanUnit    *size_unit);

void       picman_text_set_font_from_xlfd  (PicmanText    *text,
                                          const gchar *xlfd);


#endif /* __PICMAN_TEXT_COMPAT_H__ */
