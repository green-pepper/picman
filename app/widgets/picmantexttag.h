/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextTag
 * Copyright (C) 2010  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_TEXT_TAG_H__
#define __PICMAN_TEXT_TAG_H__


/*  GtkTextTag property names  */

#define PICMAN_TEXT_PROP_NAME_SIZE     "size"
#define PICMAN_TEXT_PROP_NAME_BASELINE "rise"
#define PICMAN_TEXT_PROP_NAME_KERNING  "rise" /* FIXME */
#define PICMAN_TEXT_PROP_NAME_FONT     "font"
#define PICMAN_TEXT_PROP_NAME_COLOR    "foreground-gdk"


gint    picman_text_tag_get_size     (GtkTextTag *tag);
gint    picman_text_tag_get_baseline (GtkTextTag *tag);
gint    picman_text_tag_get_kerning  (GtkTextTag *tag);
gchar * picman_text_tag_get_font     (GtkTextTag *tag);
void    picman_text_tag_get_color    (GtkTextTag *tag,
                                    PicmanRGB    *color);


#endif /* __PICMAN_TEXT_TAG_H__ */
