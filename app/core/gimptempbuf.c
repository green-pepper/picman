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

#include "config.h"

#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <glib/gstdio.h>

#include "core-types.h"

#include "picmantempbuf.h"


struct _PicmanTempBuf
{
  gint        ref_count;
  gint        width;
  gint        height;
  const Babl *format;
  guchar     *data;
};


PicmanTempBuf *
picman_temp_buf_new (gint        width,
                   gint        height,
                   const Babl *format)
{
  PicmanTempBuf *temp;

  g_return_val_if_fail (width > 0 && height > 0, NULL);
  g_return_val_if_fail (format != NULL, NULL);

  temp = g_slice_new (PicmanTempBuf);

  temp->ref_count = 1;
  temp->width     = width;
  temp->height    = height;
  temp->format    = format;
  temp->data      = g_new (guchar,
                           width * height *
                           babl_format_get_bytes_per_pixel (format));

  return temp;
}

PicmanTempBuf *
picman_temp_buf_copy (const PicmanTempBuf *src)
{
  PicmanTempBuf *dest;

  g_return_val_if_fail (src != NULL, NULL);

  dest = picman_temp_buf_new (src->width, src->height, src->format);

  memcpy (picman_temp_buf_get_data (dest),
          picman_temp_buf_get_data (src),
          picman_temp_buf_get_data_size (src));

  return dest;
}

PicmanTempBuf *
picman_temp_buf_ref (PicmanTempBuf *buf)
{
  g_return_val_if_fail (buf != NULL, NULL);

  buf->ref_count++;

  return buf;
}

void
picman_temp_buf_unref (PicmanTempBuf *buf)
{
  g_return_if_fail (buf != NULL);
  g_return_if_fail (buf->ref_count > 0);

  buf->ref_count--;

  if (buf->ref_count < 1)
    {
      if (buf->data)
        g_free (buf->data);

      g_slice_free (PicmanTempBuf, buf);
    }
}

PicmanTempBuf *
picman_temp_buf_scale (const PicmanTempBuf *src,
                     gint               new_width,
                     gint               new_height)
{
  PicmanTempBuf  *dest;
  const guchar *src_data;
  guchar       *dest_data;
  gdouble       x_ratio;
  gdouble       y_ratio;
  gint          bytes;
  gint          loop1;
  gint          loop2;

  g_return_val_if_fail (src != NULL, NULL);
  g_return_val_if_fail (new_width > 0 && new_height > 0, NULL);

  if (new_width == src->width && new_height == src->height)
    return picman_temp_buf_copy (src);

  dest = picman_temp_buf_new (new_width,
                            new_height,
                            src->format);

  src_data  = picman_temp_buf_get_data (src);
  dest_data = picman_temp_buf_get_data (dest);

  x_ratio = (gdouble) src->width  / (gdouble) new_width;
  y_ratio = (gdouble) src->height / (gdouble) new_height;

  bytes = babl_format_get_bytes_per_pixel (src->format);

  for (loop1 = 0 ; loop1 < new_height ; loop1++)
    {
      for (loop2 = 0 ; loop2 < new_width ; loop2++)
        {
          const guchar *src_pixel;
          guchar       *dest_pixel;
          gint          i;

          src_pixel = src_data +
            (gint) (loop2 * x_ratio) * bytes +
            (gint) (loop1 * y_ratio) * bytes * src->width;

          dest_pixel = dest_data +
            (loop2 + loop1 * new_width) * bytes;

          for (i = 0 ; i < bytes; i++)
            *dest_pixel++ = *src_pixel++;
        }
    }

  return dest;
}

gint
picman_temp_buf_get_width (const PicmanTempBuf *buf)
{
  return buf->width;
}

gint
picman_temp_buf_get_height (const PicmanTempBuf *buf)
{
  return buf->height;
}

const Babl *
picman_temp_buf_get_format (const PicmanTempBuf *buf)
{
  return buf->format;
}

void
picman_temp_buf_set_format (PicmanTempBuf *buf,
                          const Babl  *format)
{
  g_return_if_fail (babl_format_get_bytes_per_pixel (buf->format) ==
                    babl_format_get_bytes_per_pixel (format));

  buf->format = format;
}

guchar *
picman_temp_buf_get_data (const PicmanTempBuf *buf)
{
  return buf->data;
}

gsize
picman_temp_buf_get_data_size (const PicmanTempBuf *buf)
{
  return babl_format_get_bytes_per_pixel (buf->format) * buf->width * buf->height;
}

guchar *
picman_temp_buf_data_clear (PicmanTempBuf *buf)
{
  memset (buf->data, 0, picman_temp_buf_get_data_size (buf));

  return buf->data;
}

gsize
picman_temp_buf_get_memsize (const PicmanTempBuf *buf)
{
  if (buf)
    return (sizeof (PicmanTempBuf) + picman_temp_buf_get_data_size (buf));

  return 0;
}

GeglBuffer  *
picman_temp_buf_create_buffer (PicmanTempBuf *temp_buf)
{
  GeglBuffer *buffer;

  g_return_val_if_fail (temp_buf != NULL, NULL);

  buffer =
    gegl_buffer_linear_new_from_data (picman_temp_buf_get_data (temp_buf),
                                      temp_buf->format,
                                      GEGL_RECTANGLE (0, 0,
                                                      temp_buf->width,
                                                      temp_buf->height),
                                      GEGL_AUTO_ROWSTRIDE,
                                      (GDestroyNotify) picman_temp_buf_unref,
                                      picman_temp_buf_ref (temp_buf));

  g_object_set_data (G_OBJECT (buffer), "picman-temp-buf", temp_buf);

  return buffer;
}

PicmanTempBuf *
picman_gegl_buffer_get_temp_buf (GeglBuffer *buffer)
{
  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);

  return g_object_get_data (G_OBJECT (buffer), "picman-temp-buf");
}
