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

#ifndef __PICMAN_CLIPBOARD_H__
#define __PICMAN_CLIPBOARD_H__


void         picman_clipboard_init       (Picman        *picman);
void         picman_clipboard_exit       (Picman        *picman);

gboolean     picman_clipboard_has_buffer (Picman        *picman);
gboolean     picman_clipboard_has_svg    (Picman        *picman);
gboolean     picman_clipboard_has_curve  (Picman        *picman);

PicmanBuffer * picman_clipboard_get_buffer (Picman        *picman);
gchar      * picman_clipboard_get_svg    (Picman        *picman,
                                        gsize       *svg_length);
PicmanCurve  * picman_clipboard_get_curve  (Picman        *picman);

void         picman_clipboard_set_buffer (Picman        *picman,
                                        PicmanBuffer  *buffer);
void         picman_clipboard_set_svg    (Picman        *picman,
                                        const gchar *svg);
void         picman_clipboard_set_text   (Picman        *picman,
                                        const gchar *text);
void         picman_clipboard_set_curve  (Picman        *picman,
                                        PicmanCurve   *curve);


#endif /* __PICMAN_CLIPBOARD_H__ */
