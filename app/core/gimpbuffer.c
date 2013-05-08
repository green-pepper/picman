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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picman-babl.h"

#include "picman-utils.h"
#include "picmanbuffer.h"
#include "picmanimage.h"
#include "picmantempbuf.h"


static void          picman_buffer_finalize         (GObject       *object);

static gint64        picman_buffer_get_memsize      (PicmanObject    *object,
                                                   gint64        *gui_size);

static gboolean      picman_buffer_get_size         (PicmanViewable  *viewable,
                                                   gint          *width,
                                                   gint          *height);
static void          picman_buffer_get_preview_size (PicmanViewable  *viewable,
                                                   gint           size,
                                                   gboolean       is_popup,
                                                   gboolean       dot_for_dot,
                                                   gint          *popup_width,
                                                   gint          *popup_height);
static gboolean      picman_buffer_get_popup_size   (PicmanViewable  *viewable,
                                                   gint           width,
                                                   gint           height,
                                                   gboolean       dot_for_dot,
                                                   gint          *popup_width,
                                                   gint          *popup_height);
static PicmanTempBuf * picman_buffer_get_new_preview  (PicmanViewable  *viewable,
                                                   PicmanContext   *context,
                                                   gint           width,
                                                   gint           height);
static gchar       * picman_buffer_get_description  (PicmanViewable  *viewable,
                                                   gchar        **tooltip);


G_DEFINE_TYPE (PicmanBuffer, picman_buffer, PICMAN_TYPE_VIEWABLE)

#define parent_class picman_buffer_parent_class


static void
picman_buffer_class_init (PicmanBufferClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);

  object_class->finalize           = picman_buffer_finalize;

  picman_object_class->get_memsize   = picman_buffer_get_memsize;

  viewable_class->default_stock_id = "gtk-paste";
  viewable_class->get_size         = picman_buffer_get_size;
  viewable_class->get_preview_size = picman_buffer_get_preview_size;
  viewable_class->get_popup_size   = picman_buffer_get_popup_size;
  viewable_class->get_new_preview  = picman_buffer_get_new_preview;
  viewable_class->get_description  = picman_buffer_get_description;
}

static void
picman_buffer_init (PicmanBuffer *buffer)
{
}

static void
picman_buffer_finalize (GObject *object)
{
  PicmanBuffer *buffer = PICMAN_BUFFER (object);

  if (buffer->buffer)
    {
      g_object_unref (buffer->buffer);
      buffer->buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_buffer_get_memsize (PicmanObject *object,
                         gint64     *gui_size)
{
  PicmanBuffer *buffer  = PICMAN_BUFFER (object);
  gint64      memsize = 0;

  memsize += picman_gegl_buffer_get_memsize (buffer->buffer);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_buffer_get_size (PicmanViewable *viewable,
                      gint         *width,
                      gint         *height)
{
  PicmanBuffer *buffer = PICMAN_BUFFER (viewable);

  *width  = picman_buffer_get_width (buffer);
  *height = picman_buffer_get_height (buffer);

  return TRUE;
}

static void
picman_buffer_get_preview_size (PicmanViewable *viewable,
                              gint          size,
                              gboolean      is_popup,
                              gboolean      dot_for_dot,
                              gint         *width,
                              gint         *height)
{
  PicmanBuffer *buffer = PICMAN_BUFFER (viewable);

  picman_viewable_calc_preview_size (picman_buffer_get_width (buffer),
                                   picman_buffer_get_height (buffer),
                                   size,
                                   size,
                                   dot_for_dot, 1.0, 1.0,
                                   width,
                                   height,
                                   NULL);
}

static gboolean
picman_buffer_get_popup_size (PicmanViewable *viewable,
                            gint          width,
                            gint          height,
                            gboolean      dot_for_dot,
                            gint         *popup_width,
                            gint         *popup_height)
{
  PicmanBuffer *buffer;
  gint        buffer_width;
  gint        buffer_height;

  buffer        = PICMAN_BUFFER (viewable);
  buffer_width  = picman_buffer_get_width (buffer);
  buffer_height = picman_buffer_get_height (buffer);

  if (buffer_width > width || buffer_height > height)
    {
      gboolean scaling_up;

      picman_viewable_calc_preview_size (buffer_width,
                                       buffer_height,
                                       width  * 2,
                                       height * 2,
                                       dot_for_dot, 1.0, 1.0,
                                       popup_width,
                                       popup_height,
                                       &scaling_up);

      if (scaling_up)
        {
          *popup_width  = buffer_width;
          *popup_height = buffer_height;
        }

      return TRUE;
    }

  return FALSE;
}

static PicmanTempBuf *
picman_buffer_get_new_preview (PicmanViewable *viewable,
                             PicmanContext  *context,
                             gint          width,
                             gint          height)
{
  PicmanBuffer  *buffer = PICMAN_BUFFER (viewable);
  const Babl  *format = picman_buffer_get_format (buffer);
  PicmanTempBuf *preview;

  if (babl_format_is_palette (format))
    format = picman_babl_format (PICMAN_RGB, PICMAN_PRECISION_U8,
                               babl_format_has_alpha (format));
  else
    format = picman_babl_format (picman_babl_format_get_base_type (format),
                               PICMAN_PRECISION_U8,
                               babl_format_has_alpha (format));

  preview = picman_temp_buf_new (width, height, format);

  gegl_buffer_get (buffer->buffer, GEGL_RECTANGLE (0, 0, width, height),
                   MIN ((gdouble) width  / (gdouble) picman_buffer_get_width (buffer),
                        (gdouble) height / (gdouble) picman_buffer_get_height (buffer)),
                   format,
                   picman_temp_buf_get_data (preview),
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  return preview;
}

static gchar *
picman_buffer_get_description (PicmanViewable  *viewable,
                             gchar        **tooltip)
{
  PicmanBuffer *buffer = PICMAN_BUFFER (viewable);

  return g_strdup_printf ("%s (%d × %d)",
                          picman_object_get_name (buffer),
                          picman_buffer_get_width (buffer),
                          picman_buffer_get_height (buffer));
}

PicmanBuffer *
picman_buffer_new (GeglBuffer    *buffer,
                 const gchar   *name,
                 gint           offset_x,
                 gint           offset_y,
                 gboolean       copy_pixels)
{
  PicmanBuffer *picman_buffer;

  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  picman_buffer = g_object_new (PICMAN_TYPE_BUFFER,
                              "name", name,
                              NULL);

  if (copy_pixels)
    picman_buffer->buffer = gegl_buffer_dup (buffer);
  else
    picman_buffer->buffer = g_object_ref (buffer);

  picman_buffer->offset_x = offset_x;
  picman_buffer->offset_y = offset_y;

  return picman_buffer;
}

PicmanBuffer *
picman_buffer_new_from_pixbuf (GdkPixbuf   *pixbuf,
                             const gchar *name,
                             gint         offset_x,
                             gint         offset_y)
{
  PicmanBuffer *picman_buffer;
  GeglBuffer *buffer;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  buffer = picman_pixbuf_create_buffer (pixbuf);

  picman_buffer = picman_buffer_new (buffer, name,
                                 offset_x, offset_y, FALSE);

  g_object_unref (buffer);

  return picman_buffer;
}

gint
picman_buffer_get_width (const PicmanBuffer *buffer)
{
  g_return_val_if_fail (PICMAN_IS_BUFFER (buffer), 0);

  return gegl_buffer_get_width (buffer->buffer);
}

gint
picman_buffer_get_height (const PicmanBuffer *buffer)
{
  g_return_val_if_fail (PICMAN_IS_BUFFER (buffer), 0);

  return gegl_buffer_get_height (buffer->buffer);
}

const Babl *
picman_buffer_get_format (const PicmanBuffer *buffer)
{
  g_return_val_if_fail (PICMAN_IS_BUFFER (buffer), NULL);

  return gegl_buffer_get_format (buffer->buffer);
}

GeglBuffer *
picman_buffer_get_buffer (const PicmanBuffer *buffer)
{
  g_return_val_if_fail (PICMAN_IS_BUFFER (buffer), NULL);

  return buffer->buffer;
}
