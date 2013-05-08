/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushclipboard.c
 * Copyright (C) 2006 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>

#include "core-types.h"

#include "picman.h"
#include "picmanbuffer.h"
#include "picmanbrushclipboard.h"
#include "picmanimage.h"
#include "picmantempbuf.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN
};


/*  local function prototypes  */

static void       picman_brush_clipboard_constructed  (GObject      *object);
static void       picman_brush_clipboard_set_property (GObject      *object,
                                                     guint         property_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec);
static void       picman_brush_clipboard_get_property (GObject      *object,
                                                     guint         property_id,
                                                     GValue       *value,
                                                     GParamSpec   *pspec);
#if 0
static PicmanData * picman_brush_clipboard_duplicate    (PicmanData     *data);
#endif

static void     picman_brush_clipboard_buffer_changed (Picman         *picman,
                                                     PicmanBrush    *brush);


G_DEFINE_TYPE (PicmanBrushClipboard, picman_brush_clipboard, PICMAN_TYPE_BRUSH)

#define parent_class picman_brush_clipboard_parent_class


static void
picman_brush_clipboard_class_init (PicmanBrushClipboardClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
#if 0
  PicmanDataClass *data_class   = PICMAN_DATA_CLASS (klass);
#endif

  object_class->constructed  = picman_brush_clipboard_constructed;
  object_class->set_property = picman_brush_clipboard_set_property;
  object_class->get_property = picman_brush_clipboard_get_property;

#if 0
  data_class->duplicate      = picman_brush_clipboard_duplicate;
#endif

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_brush_clipboard_init (PicmanBrushClipboard *brush)
{
  brush->picman = NULL;
}

static void
picman_brush_clipboard_constructed (GObject *object)
{
  PicmanBrushClipboard *brush = PICMAN_BRUSH_CLIPBOARD (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (brush->picman));

  g_signal_connect_object (brush->picman, "buffer-changed",
                           G_CALLBACK (picman_brush_clipboard_buffer_changed),
                           brush, 0);

  picman_brush_clipboard_buffer_changed (brush->picman, PICMAN_BRUSH (brush));
}

static void
picman_brush_clipboard_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanBrushClipboard *brush = PICMAN_BRUSH_CLIPBOARD (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      brush->picman = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_brush_clipboard_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanBrushClipboard *brush = PICMAN_BRUSH_CLIPBOARD (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, brush->picman);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

#if 0
static PicmanData *
picman_brush_clipboard_duplicate (PicmanData *data)
{
  PicmanBrushClipboard *brush = PICMAN_BRUSH_CLIPBOARD (data);

  return picman_brush_clipboard_new (brush->picman);
}
#endif

PicmanData *
picman_brush_clipboard_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_BRUSH_CLIPBOARD,
                       "name", _("Clipboard"),
                       "picman", picman,
                       NULL);
}


/*  private functions  */

static void
picman_brush_clipboard_buffer_changed (Picman      *picman,
                                     PicmanBrush *brush)
{
  gint width;
  gint height;

  if (brush->mask)
    {
      picman_temp_buf_unref (brush->mask);
      brush->mask = NULL;
    }

  if (brush->pixmap)
    {
      picman_temp_buf_unref (brush->pixmap);
      brush->pixmap = NULL;
    }

  if (picman->global_buffer)
    {
      GeglBuffer *buffer = picman_buffer_get_buffer (picman->global_buffer);
      const Babl *format = gegl_buffer_get_format (buffer);
      GeglBuffer *dest_buffer;

      width  = MIN (picman_buffer_get_width  (picman->global_buffer), 2048);
      height = MIN (picman_buffer_get_height (picman->global_buffer), 2048);

      brush->mask   = picman_temp_buf_new (width, height,
                                         babl_format ("Y u8"));
      brush->pixmap = picman_temp_buf_new (width, height,
                                         babl_format ("R'G'B' u8"));

      /*  copy the alpha channel into the brush's mask  */
      if (babl_format_has_alpha (format))
        {
          dest_buffer = picman_temp_buf_create_buffer (brush->mask);

          gegl_buffer_set_format (dest_buffer, babl_format ("A u8"));
          gegl_buffer_copy (buffer, NULL, dest_buffer, NULL);

          g_object_unref (dest_buffer);
        }
      else
        {
          memset (picman_temp_buf_get_data (brush->mask), 255,
                  width * height);
        }

      /*  copy the color channels into the brush's pixmap  */
      dest_buffer = picman_temp_buf_create_buffer (brush->pixmap);

      gegl_buffer_copy (buffer, NULL, dest_buffer, NULL);

      g_object_unref (dest_buffer);
    }
  else
    {
      width  = 17;
      height = 17;

      brush->mask = picman_temp_buf_new (width, height, babl_format ("Y u8"));
      picman_temp_buf_data_clear (brush->mask);
    }

  brush->x_axis.x = width / 2;
  brush->x_axis.y = 0;
  brush->y_axis.x = 0;
  brush->y_axis.y = height / 2;

  picman_data_dirty (PICMAN_DATA (brush));
}
