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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picmanpattern.h"
#include "picmanpattern-load.h"
#include "picmantagged.h"
#include "picmantempbuf.h"

#include "picman-intl.h"


static void          picman_pattern_tagged_iface_init (PicmanTaggedInterface  *iface);
static void          picman_pattern_finalize          (GObject              *object);

static gint64        picman_pattern_get_memsize       (PicmanObject           *object,
                                                     gint64               *gui_size);

static gboolean      picman_pattern_get_size          (PicmanViewable         *viewable,
                                                     gint                 *width,
                                                     gint                 *height);
static PicmanTempBuf * picman_pattern_get_new_preview   (PicmanViewable         *viewable,
                                                     PicmanContext          *context,
                                                     gint                  width,
                                                     gint                  height);
static gchar       * picman_pattern_get_description   (PicmanViewable         *viewable,
                                                     gchar               **tooltip);

static const gchar * picman_pattern_get_extension     (PicmanData             *data);
static PicmanData    * picman_pattern_duplicate         (PicmanData             *data);

static gchar       * picman_pattern_get_checksum      (PicmanTagged           *tagged);


G_DEFINE_TYPE_WITH_CODE (PicmanPattern, picman_pattern, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_TAGGED,
                                                picman_pattern_tagged_iface_init))

#define parent_class picman_pattern_parent_class


static void
picman_pattern_class_init (PicmanPatternClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanDataClass     *data_class        = PICMAN_DATA_CLASS (klass);

  object_class->finalize           = picman_pattern_finalize;

  picman_object_class->get_memsize   = picman_pattern_get_memsize;

  viewable_class->default_stock_id = "picman-tool-bucket-fill";
  viewable_class->get_size         = picman_pattern_get_size;
  viewable_class->get_new_preview  = picman_pattern_get_new_preview;
  viewable_class->get_description  = picman_pattern_get_description;

  data_class->get_extension        = picman_pattern_get_extension;
  data_class->duplicate            = picman_pattern_duplicate;
}

static void
picman_pattern_tagged_iface_init (PicmanTaggedInterface *iface)
{
  iface->get_checksum = picman_pattern_get_checksum;
}

static void
picman_pattern_init (PicmanPattern *pattern)
{
  pattern->mask = NULL;
}

static void
picman_pattern_finalize (GObject *object)
{
  PicmanPattern *pattern = PICMAN_PATTERN (object);

  if (pattern->mask)
    {
      picman_temp_buf_unref (pattern->mask);
      pattern->mask = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_pattern_get_memsize (PicmanObject *object,
                          gint64     *gui_size)
{
  PicmanPattern *pattern = PICMAN_PATTERN (object);
  gint64       memsize = 0;

  memsize += picman_temp_buf_get_memsize (pattern->mask);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_pattern_get_size (PicmanViewable *viewable,
                       gint         *width,
                       gint         *height)
{
  PicmanPattern *pattern = PICMAN_PATTERN (viewable);

  *width  = picman_temp_buf_get_width  (pattern->mask);
  *height = picman_temp_buf_get_height (pattern->mask);

  return TRUE;
}

static PicmanTempBuf *
picman_pattern_get_new_preview (PicmanViewable *viewable,
                              PicmanContext  *context,
                              gint          width,
                              gint          height)
{
  PicmanPattern *pattern = PICMAN_PATTERN (viewable);
  PicmanTempBuf *temp_buf;
  GeglBuffer  *src_buffer;
  GeglBuffer  *dest_buffer;
  gint         copy_width;
  gint         copy_height;

  copy_width  = MIN (width,  picman_temp_buf_get_width  (pattern->mask));
  copy_height = MIN (height, picman_temp_buf_get_height (pattern->mask));

  temp_buf = picman_temp_buf_new (copy_width, copy_height,
                                picman_temp_buf_get_format (pattern->mask));

  src_buffer  = picman_temp_buf_create_buffer (pattern->mask);
  dest_buffer = picman_temp_buf_create_buffer (temp_buf);

  gegl_buffer_copy (src_buffer,  GEGL_RECTANGLE (0, 0, copy_width, copy_height),
                    dest_buffer, GEGL_RECTANGLE (0, 0, 0, 0));

  g_object_unref (src_buffer);
  g_object_unref (dest_buffer);

  return temp_buf;
}

static gchar *
picman_pattern_get_description (PicmanViewable  *viewable,
                              gchar        **tooltip)
{
  PicmanPattern *pattern = PICMAN_PATTERN (viewable);

  return g_strdup_printf ("%s (%d × %d)",
                          picman_object_get_name (pattern),
                          picman_temp_buf_get_width  (pattern->mask),
                          picman_temp_buf_get_height (pattern->mask));
}

static const gchar *
picman_pattern_get_extension (PicmanData *data)
{
  return PICMAN_PATTERN_FILE_EXTENSION;
}

static PicmanData *
picman_pattern_duplicate (PicmanData *data)
{
  PicmanPattern *pattern = g_object_new (PICMAN_TYPE_PATTERN, NULL);

  pattern->mask = picman_temp_buf_copy (PICMAN_PATTERN (data)->mask);

  return PICMAN_DATA (pattern);
}

static gchar *
picman_pattern_get_checksum (PicmanTagged *tagged)
{
  PicmanPattern *pattern         = PICMAN_PATTERN (tagged);
  gchar       *checksum_string = NULL;

  if (pattern->mask)
    {
      GChecksum *checksum = g_checksum_new (G_CHECKSUM_MD5);

      g_checksum_update (checksum, picman_temp_buf_get_data (pattern->mask),
                         picman_temp_buf_get_data_size (pattern->mask));

      checksum_string = g_strdup (g_checksum_get_string (checksum));

      g_checksum_free (checksum);
    }

  return checksum_string;
}

PicmanData *
picman_pattern_new (PicmanContext *context,
                  const gchar *name)
{
  PicmanPattern *pattern;
  guchar      *data;
  gint         row, col;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (name[0] != '\n', NULL);

  pattern = g_object_new (PICMAN_TYPE_PATTERN,
                          "name", name,
                          NULL);

  pattern->mask = picman_temp_buf_new (32, 32, babl_format ("R'G'B' u8"));

  data = picman_temp_buf_get_data (pattern->mask);

  for (row = 0; row < picman_temp_buf_get_height (pattern->mask); row++)
    for (col = 0; col < picman_temp_buf_get_width (pattern->mask); col++)
      {
        memset (data, (col % 2) && (row % 2) ? 255 : 0, 3);
        data += 3;
      }

  return PICMAN_DATA (pattern);
}

PicmanData *
picman_pattern_get_standard (PicmanContext *context)
{
  static PicmanData *standard_pattern = NULL;

  if (! standard_pattern)
    {
      standard_pattern = picman_pattern_new (context, "Standard");

      picman_data_clean (standard_pattern);
      picman_data_make_internal (standard_pattern, "picman-pattern-standard");

      g_object_add_weak_pointer (G_OBJECT (standard_pattern),
                                 (gpointer *) &standard_pattern);
    }

  return standard_pattern;
}

PicmanTempBuf *
picman_pattern_get_mask (const PicmanPattern *pattern)
{
  g_return_val_if_fail (PICMAN_IS_PATTERN (pattern), NULL);

  return pattern->mask;
}

GeglBuffer *
picman_pattern_create_buffer (const PicmanPattern *pattern)
{
  g_return_val_if_fail (PICMAN_IS_PATTERN (pattern), NULL);

  return picman_temp_buf_create_buffer (pattern->mask);
}
