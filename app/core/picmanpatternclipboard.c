/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpatternclipboard.c
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
#include "picmanpatternclipboard.h"
#include "picmanimage.h"
#include "picmanpickable.h"
#include "picmantempbuf.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN
};


/*  local function prototypes  */

static void       picman_pattern_clipboard_constructed  (GObject      *object);
static void       picman_pattern_clipboard_set_property (GObject      *object,
                                                       guint         property_id,
                                                       const GValue *value,
                                                       GParamSpec   *pspec);
static void       picman_pattern_clipboard_get_property (GObject      *object,
                                                       guint         property_id,
                                                       GValue       *value,
                                                       GParamSpec   *pspec);
#if 0
static PicmanData * picman_pattern_clipboard_duplicate    (PicmanData     *data);
#endif

static void     picman_pattern_clipboard_buffer_changed (Picman         *picman,
                                                       PicmanPattern  *pattern);


G_DEFINE_TYPE (PicmanPatternClipboard, picman_pattern_clipboard, PICMAN_TYPE_PATTERN)

#define parent_class picman_pattern_clipboard_parent_class


static void
picman_pattern_clipboard_class_init (PicmanPatternClipboardClass *klass)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (klass);
#if 0
  PicmanDataClass *data_class   = PICMAN_DATA_CLASS (klass);
#endif

  object_class->constructed  = picman_pattern_clipboard_constructed;
  object_class->set_property = picman_pattern_clipboard_set_property;
  object_class->get_property = picman_pattern_clipboard_get_property;

#if 0
  data_class->duplicate      = picman_pattern_clipboard_duplicate;
#endif

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman", NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_pattern_clipboard_init (PicmanPatternClipboard *pattern)
{
  pattern->picman = NULL;
}

static void
picman_pattern_clipboard_constructed (GObject *object)
{
  PicmanPatternClipboard *pattern = PICMAN_PATTERN_CLIPBOARD (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (pattern->picman));

  g_signal_connect_object (pattern->picman, "buffer-changed",
                           G_CALLBACK (picman_pattern_clipboard_buffer_changed),
                           pattern, 0);

  picman_pattern_clipboard_buffer_changed (pattern->picman, PICMAN_PATTERN (pattern));
}

static void
picman_pattern_clipboard_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanPatternClipboard *pattern = PICMAN_PATTERN_CLIPBOARD (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      pattern->picman = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_pattern_clipboard_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanPatternClipboard *pattern = PICMAN_PATTERN_CLIPBOARD (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, pattern->picman);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

#if 0
static PicmanData *
picman_pattern_clipboard_duplicate (PicmanData *data)
{
  PicmanPatternClipboard *pattern = PICMAN_PATTERN_CLIPBOARD (data);

  return picman_pattern_clipboard_new (pattern->picman);
}
#endif

PicmanData *
picman_pattern_clipboard_new (Picman *picman)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);

  return g_object_new (PICMAN_TYPE_PATTERN_CLIPBOARD,
                       "name", _("Clipboard"),
                       "picman", picman,
                       NULL);
}


/*  private functions  */

static void
picman_pattern_clipboard_buffer_changed (Picman        *picman,
                                       PicmanPattern *pattern)
{
  if (pattern->mask)
    {
      picman_temp_buf_unref (pattern->mask);
      pattern->mask = NULL;
    }

  if (picman->global_buffer)
    {
      PicmanBuffer *buffer = picman->global_buffer;
      gint        width;
      gint        height;

      width  = MIN (picman_buffer_get_width  (buffer), 2048);
      height = MIN (picman_buffer_get_height (buffer), 2048);

      pattern->mask = picman_temp_buf_new (width, height,
                                         picman_buffer_get_format (buffer));

      gegl_buffer_get (picman_buffer_get_buffer (buffer),
                       GEGL_RECTANGLE (0, 0, width, height), 1.0,
                       NULL,
                       picman_temp_buf_get_data (pattern->mask),
                       GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
    }
  else
    {
      pattern->mask = picman_temp_buf_new (16, 16, babl_format ("R'G'B' u8"));
      memset (picman_temp_buf_get_data (pattern->mask), 255, 16 * 16 * 3);
    }

  picman_data_dirty (PICMAN_DATA (pattern));
}
