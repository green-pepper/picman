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

#ifndef __PICMAN_TEMP_BUF_H__
#define __PICMAN_TEMP_BUF_H__


PicmanTempBuf * picman_temp_buf_new             (gint               width,
                                             gint               height,
                                             const Babl        *fomat) G_GNUC_WARN_UNUSED_RESULT;
PicmanTempBuf * picman_temp_buf_copy            (const PicmanTempBuf *src) G_GNUC_WARN_UNUSED_RESULT;

PicmanTempBuf * picman_temp_buf_ref             (PicmanTempBuf       *buf);
void          picman_temp_buf_unref           (PicmanTempBuf       *buf);

PicmanTempBuf * picman_temp_buf_scale           (const PicmanTempBuf *buf,
                                             gint               width,
                                             gint               height) G_GNUC_WARN_UNUSED_RESULT;

gint          picman_temp_buf_get_width       (const PicmanTempBuf *buf);
gint          picman_temp_buf_get_height      (const PicmanTempBuf *buf);

const Babl  * picman_temp_buf_get_format      (const PicmanTempBuf *buf);
void          picman_temp_buf_set_format      (PicmanTempBuf       *buf,
                                             const Babl        *format);

guchar      * picman_temp_buf_get_data        (const PicmanTempBuf *buf);
gsize         picman_temp_buf_get_data_size   (const PicmanTempBuf *buf);

guchar      * picman_temp_buf_data_clear      (PicmanTempBuf       *buf);

gsize         picman_temp_buf_get_memsize     (const PicmanTempBuf *buf);

GeglBuffer  * picman_temp_buf_create_buffer   (PicmanTempBuf       *temp_buf) G_GNUC_WARN_UNUSED_RESULT;
PicmanTempBuf * picman_gegl_buffer_get_temp_buf (GeglBuffer        *buffer);



#endif  /*  __PICMAN_TEMP_BUF_H__  */
