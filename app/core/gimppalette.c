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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picman-utils.h"
#include "picmanpalette.h"
#include "picmanpalette-load.h"
#include "picmanpalette-save.h"
#include "picmantagged.h"
#include "picmantempbuf.h"

#include "picman-intl.h"

#define EPSILON 1e-10

/*  local function prototypes  */

static void          picman_palette_tagged_iface_init (PicmanTaggedInterface  *iface);

static void          picman_palette_finalize          (GObject              *object);

static gint64        picman_palette_get_memsize       (PicmanObject           *object,
                                                     gint64               *gui_size);

static void          picman_palette_get_preview_size  (PicmanViewable         *viewable,
                                                     gint                  size,
                                                     gboolean              popup,
                                                     gboolean              dot_for_dot,
                                                     gint                 *width,
                                                     gint                 *height);
static gboolean      picman_palette_get_popup_size    (PicmanViewable         *viewable,
                                                     gint                  width,
                                                     gint                  height,
                                                     gboolean              dot_for_dot,
                                                     gint                 *popup_width,
                                                     gint                 *popup_height);
static PicmanTempBuf * picman_palette_get_new_preview   (PicmanViewable         *viewable,
                                                     PicmanContext          *context,
                                                     gint                  width,
                                                     gint                  height);
static gchar       * picman_palette_get_description   (PicmanViewable         *viewable,
                                                     gchar               **tooltip);
static const gchar * picman_palette_get_extension     (PicmanData             *data);
static PicmanData    * picman_palette_duplicate         (PicmanData             *data);

static void          picman_palette_entry_free        (PicmanPaletteEntry     *entry);
static gint64        picman_palette_entry_get_memsize (PicmanPaletteEntry     *entry,
                                                     gint64               *gui_size);
static gchar       * picman_palette_get_checksum      (PicmanTagged           *tagged);


G_DEFINE_TYPE_WITH_CODE (PicmanPalette, picman_palette, PICMAN_TYPE_DATA,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_TAGGED,
                                                picman_palette_tagged_iface_init))

#define parent_class picman_palette_parent_class


static void
picman_palette_class_init (PicmanPaletteClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanDataClass     *data_class        = PICMAN_DATA_CLASS (klass);

  object_class->finalize           = picman_palette_finalize;

  picman_object_class->get_memsize   = picman_palette_get_memsize;

  viewable_class->default_stock_id = "gtk-select-color";
  viewable_class->get_preview_size = picman_palette_get_preview_size;
  viewable_class->get_popup_size   = picman_palette_get_popup_size;
  viewable_class->get_new_preview  = picman_palette_get_new_preview;
  viewable_class->get_description  = picman_palette_get_description;

  data_class->save                 = picman_palette_save;
  data_class->get_extension        = picman_palette_get_extension;
  data_class->duplicate            = picman_palette_duplicate;
}

static void
picman_palette_tagged_iface_init (PicmanTaggedInterface *iface)
{
  iface->get_checksum = picman_palette_get_checksum;
}

static void
picman_palette_init (PicmanPalette *palette)
{
  palette->colors    = NULL;
  palette->n_colors  = 0;
  palette->n_columns = 0;
}

static void
picman_palette_finalize (GObject *object)
{
  PicmanPalette *palette = PICMAN_PALETTE (object);

  if (palette->colors)
    {
      g_list_free_full (palette->colors,
                        (GDestroyNotify) picman_palette_entry_free);
      palette->colors = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_palette_get_memsize (PicmanObject *object,
                          gint64     *gui_size)
{
  PicmanPalette *palette = PICMAN_PALETTE (object);
  gint64       memsize = 0;

  memsize += picman_g_list_get_memsize_foreach (palette->colors,
                                              (PicmanMemsizeFunc)
                                              picman_palette_entry_get_memsize,
                                              gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_palette_get_preview_size (PicmanViewable *viewable,
                               gint          size,
                               gboolean      popup,
                               gboolean      dot_for_dot,
                               gint         *width,
                               gint         *height)
{
  *width  = size;
  *height = 1 + size / 2;
}

static gboolean
picman_palette_get_popup_size (PicmanViewable *viewable,
                             gint          width,
                             gint          height,
                             gboolean      dot_for_dot,
                             gint         *popup_width,
                             gint         *popup_height)
{
  PicmanPalette *palette = PICMAN_PALETTE (viewable);
  gint         p_width;
  gint         p_height;

  if (! palette->n_colors)
    return FALSE;

  if (palette->n_columns)
    p_width = palette->n_columns;
  else
    p_width = MIN (palette->n_colors, 16);

  p_height = MAX (1, palette->n_colors / p_width);

  if (p_width * 4 > width || p_height * 4 > height)
    {
      *popup_width  = p_width  * 4;
      *popup_height = p_height * 4;

      return TRUE;
    }

  return FALSE;
}

static PicmanTempBuf *
picman_palette_get_new_preview (PicmanViewable *viewable,
                              PicmanContext  *context,
                              gint          width,
                              gint          height)
{
  PicmanPalette *palette  = PICMAN_PALETTE (viewable);
  PicmanTempBuf *temp_buf;
  guchar      *buf;
  guchar      *b;
  GList       *list;
  gint         columns;
  gint         rows;
  gint         cell_size;
  gint         x, y;

  temp_buf = picman_temp_buf_new (width, height, babl_format ("R'G'B' u8"));
  memset (picman_temp_buf_get_data (temp_buf), 255, width * height * 3);

  if (palette->n_columns > 1)
    cell_size = MAX (4, width / palette->n_columns);
  else
    cell_size = 4;

  columns = width  / cell_size;
  rows    = height / cell_size;

  buf = picman_temp_buf_get_data (temp_buf);
  b   = g_new (guchar, width * 3);

  list = palette->colors;

  for (y = 0; y < rows && list; y++)
    {
      gint i;

      memset (b, 255, width * 3);

      for (x = 0; x < columns && list; x++)
        {
          PicmanPaletteEntry *entry = list->data;

          list = g_list_next (list);

          picman_rgb_get_uchar (&entry->color,
                              &b[x * cell_size * 3 + 0],
                              &b[x * cell_size * 3 + 1],
                              &b[x * cell_size * 3 + 2]);

          for (i = 1; i < cell_size; i++)
            {
              b[(x * cell_size + i) * 3 + 0] = b[(x * cell_size) * 3 + 0];
              b[(x * cell_size + i) * 3 + 1] = b[(x * cell_size) * 3 + 1];
              b[(x * cell_size + i) * 3 + 2] = b[(x * cell_size) * 3 + 2];
            }
        }

      for (i = 0; i < cell_size; i++)
        memcpy (buf + ((y * cell_size + i) * width) * 3, b, width * 3);
    }

  g_free (b);

  return temp_buf;
}

static gchar *
picman_palette_get_description (PicmanViewable  *viewable,
                              gchar        **tooltip)
{
  PicmanPalette *palette = PICMAN_PALETTE (viewable);

  return g_strdup_printf ("%s (%d)",
                          picman_object_get_name (palette),
                          palette->n_colors);
}

PicmanData *
picman_palette_new (PicmanContext *context,
                  const gchar *name)
{
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (*name != '\0', NULL);

  return g_object_new (PICMAN_TYPE_PALETTE,
                       "name", name,
                       NULL);
}

PicmanData *
picman_palette_get_standard (PicmanContext *context)
{
  static PicmanData *standard_palette = NULL;

  if (! standard_palette)
    {
      standard_palette = picman_palette_new (context, "Standard");

      picman_data_clean (standard_palette);
      picman_data_make_internal (standard_palette, "picman-palette-standard");

      g_object_add_weak_pointer (G_OBJECT (standard_palette),
                                 (gpointer *) &standard_palette);
    }

  return standard_palette;
}

static const gchar *
picman_palette_get_extension (PicmanData *data)
{
  return PICMAN_PALETTE_FILE_EXTENSION;
}

static PicmanData *
picman_palette_duplicate (PicmanData *data)
{
  PicmanPalette *palette = PICMAN_PALETTE (data);
  PicmanPalette *new;
  GList       *list;

  new = g_object_new (PICMAN_TYPE_PALETTE, NULL);

  new->n_columns = palette->n_columns;

  for (list = palette->colors; list; list = g_list_next (list))
    {
      PicmanPaletteEntry *entry = list->data;

      picman_palette_add_entry (new, -1, entry->name, &entry->color);
    }

  return PICMAN_DATA (new);
}

static gchar *
picman_palette_get_checksum (PicmanTagged *tagged)
{
  PicmanPalette *palette         = PICMAN_PALETTE (tagged);
  gchar       *checksum_string = NULL;

  if (palette->n_colors > 0)
    {
      GChecksum *checksum       = g_checksum_new (G_CHECKSUM_MD5);
      GList     *color_iterator = palette->colors;

      g_checksum_update (checksum, (const guchar *) &palette->n_colors, sizeof (palette->n_colors));
      g_checksum_update (checksum, (const guchar *) &palette->n_columns, sizeof (palette->n_columns));

      while (color_iterator)
        {
          PicmanPaletteEntry *entry = (PicmanPaletteEntry *) color_iterator->data;

          g_checksum_update (checksum, (const guchar *) &entry->color, sizeof (entry->color));
          if (entry->name)
            g_checksum_update (checksum, (const guchar *) entry->name, strlen (entry->name));

          color_iterator = g_list_next (color_iterator);
        }

      checksum_string = g_strdup (g_checksum_get_string (checksum));

      g_checksum_free (checksum);
    }

  return checksum_string;
}


/*  public functions  */

GList *
picman_palette_get_colors (PicmanPalette *palette)
{
  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), NULL);

  return palette->colors;
}

gint
picman_palette_get_n_colors (PicmanPalette *palette)
{
  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), 0);

  return palette->n_colors;
}

PicmanPaletteEntry *
picman_palette_add_entry (PicmanPalette   *palette,
                        gint           position,
                        const gchar   *name,
                        const PicmanRGB *color)
{
  PicmanPaletteEntry *entry;

  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), NULL);
  g_return_val_if_fail (color != NULL, NULL);

  entry = g_slice_new0 (PicmanPaletteEntry);

  entry->color = *color;
  entry->name  = g_strdup (name ? name : _("Untitled"));

  if (position < 0 || position >= palette->n_colors)
    {
      entry->position = palette->n_colors;
      palette->colors = g_list_append (palette->colors, entry);
    }
  else
    {
      GList *list;

      entry->position = position;
      palette->colors = g_list_insert (palette->colors, entry, position);

      /* renumber the displaced entries */
      for (list = g_list_nth (palette->colors, position + 1);
           list;
           list = g_list_next (list))
        {
          PicmanPaletteEntry *entry2 = list->data;

          entry2->position += 1;
        }
    }

  palette->n_colors += 1;

  picman_data_dirty (PICMAN_DATA (palette));

  return entry;
}

void
picman_palette_delete_entry (PicmanPalette      *palette,
                           PicmanPaletteEntry *entry)
{
  GList *list;
  gint   pos = 0;

  g_return_if_fail (PICMAN_IS_PALETTE (palette));
  g_return_if_fail (entry != NULL);

  if (g_list_find (palette->colors, entry))
    {
      pos = entry->position;
      picman_palette_entry_free (entry);

      palette->colors = g_list_remove (palette->colors, entry);

      palette->n_colors--;

      for (list = g_list_nth (palette->colors, pos);
           list;
           list = g_list_next (list))
        {
          entry = (PicmanPaletteEntry *) list->data;

          entry->position = pos++;
        }

      picman_data_dirty (PICMAN_DATA (palette));
    }
}

gboolean
picman_palette_set_entry (PicmanPalette   *palette,
                        gint           position,
                        const gchar   *name,
                        const PicmanRGB *color)
{
  PicmanPaletteEntry *entry;

  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  entry = picman_palette_get_entry (palette, position);

  if (! entry)
    return FALSE;

  entry->color = *color;

  if (entry->name)
    g_free (entry->name);

  entry->name = g_strdup (name);

  picman_data_dirty (PICMAN_DATA (palette));

  return TRUE;
}

gboolean
picman_palette_set_entry_color (PicmanPalette   *palette,
                              gint           position,
                              const PicmanRGB *color)
{
  PicmanPaletteEntry *entry;

  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  entry = picman_palette_get_entry (palette, position);

  if (! entry)
    return FALSE;

  entry->color = *color;

  picman_data_dirty (PICMAN_DATA (palette));

  return TRUE;
}

gboolean
picman_palette_set_entry_name (PicmanPalette *palette,
                             gint         position,
                             const gchar *name)
{
  PicmanPaletteEntry *entry;

  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), FALSE);

  entry = picman_palette_get_entry (palette, position);

  if (! entry)
    return FALSE;

  if (entry->name)
    g_free (entry->name);

  entry->name = g_strdup (name);

  picman_data_dirty (PICMAN_DATA (palette));

  return TRUE;
}

PicmanPaletteEntry *
picman_palette_get_entry (PicmanPalette *palette,
                        gint         position)
{
  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), NULL);

  return g_list_nth_data (palette->colors, position);
}

void
picman_palette_set_columns (PicmanPalette *palette,
                          gint         columns)
{
  g_return_if_fail (PICMAN_IS_PALETTE (palette));

  columns = CLAMP (columns, 0, 64);

  if (palette->n_columns != columns)
    {
      palette->n_columns = columns;

      picman_data_dirty (PICMAN_DATA (palette));
    }
}

gint
picman_palette_get_columns (PicmanPalette *palette)
{
  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), 0);

  return palette->n_columns;
}

PicmanPaletteEntry *
picman_palette_find_entry (PicmanPalette      *palette,
                         const PicmanRGB    *color,
                         PicmanPaletteEntry *start_from)
{
  PicmanPaletteEntry *entry;

  g_return_val_if_fail (PICMAN_IS_PALETTE (palette), NULL);
  g_return_val_if_fail (color != NULL, NULL);

  if (! start_from)
    {
      GList *list;

      /* search from the start */

      for (list = palette->colors; list; list = g_list_next (list))
        {
          entry = (PicmanPaletteEntry *) list->data;
          if (picman_rgb_distance (&entry->color, color) < EPSILON)
            return entry;
        }
    }
  else if (picman_rgb_distance (&start_from->color, color) < EPSILON)
    {
      return start_from;
    }
  else
    {
      GList *old = g_list_find (palette->colors, start_from);
      GList *next;
      GList *prev;

      g_return_val_if_fail (old != NULL, NULL);

      next = old->next;
      prev = old->prev;

      /* proximity-based search */

      while (next || prev)
        {
          if (next)
            {
              entry = (PicmanPaletteEntry *) next->data;
              if (picman_rgb_distance (&entry->color, color) < EPSILON)
                return entry;

              next = next->next;
            }

          if (prev)
            {
              entry = (PicmanPaletteEntry *) prev->data;
              if (picman_rgb_distance (&entry->color, color) < EPSILON)
                return entry;

              prev = prev->prev;
            }
        }
    }

  return NULL;
}


/*  private functions  */

static void
picman_palette_entry_free (PicmanPaletteEntry *entry)
{
  g_return_if_fail (entry != NULL);

  g_free (entry->name);

  g_slice_free (PicmanPaletteEntry, entry);
}

static gint64
picman_palette_entry_get_memsize (PicmanPaletteEntry *entry,
                                gint64           *gui_size)
{
  gint64 memsize = sizeof (PicmanPaletteEntry);

  memsize += picman_string_get_memsize (entry->name);

  return memsize;
}
