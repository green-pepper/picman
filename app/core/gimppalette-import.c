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

#include "picmanchannel.h"
#include "picmancontainer.h"
#include "picmancontext.h"
#include "picmangradient.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanpalette.h"
#include "picmanpalette-import.h"
#include "picmanpalette-load.h"
#include "picmanpickable.h"

#include "picman-intl.h"


#define MAX_IMAGE_COLORS (10000 * 2)


/*  create a palette from a gradient  ****************************************/

PicmanPalette *
picman_palette_import_from_gradient (PicmanGradient *gradient,
                                   PicmanContext  *context,
                                   gboolean      reverse,
                                   const gchar  *palette_name,
                                   gint          n_colors)
{
  PicmanPalette         *palette;
  PicmanGradientSegment *seg = NULL;
  gdouble              dx, cur_x;
  PicmanRGB              color;
  gint                 i;

  g_return_val_if_fail (PICMAN_IS_GRADIENT (gradient), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (palette_name != NULL, NULL);
  g_return_val_if_fail (n_colors > 1, NULL);

  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));

  dx = 1.0 / (n_colors - 1);

  for (i = 0, cur_x = 0; i < n_colors; i++, cur_x += dx)
    {
      seg = picman_gradient_get_color_at (gradient, context,
                                        seg, cur_x, reverse, &color);
      picman_palette_add_entry (palette, -1, NULL, &color);
    }

  return palette;
}


/*  create a palette from a non-indexed image  *******************************/

typedef struct _ImgColors ImgColors;

struct _ImgColors
{
  guint  count;
  guint  r_adj;
  guint  g_adj;
  guint  b_adj;
  guchar r;
  guchar g;
  guchar b;
};

static gint count_color_entries = 0;

static GHashTable *
picman_palette_import_store_colors (GHashTable *table,
                                  guchar     *colors,
                                  guchar     *colors_real,
                                  gint        n_colors)
{
  gpointer   found_color = NULL;
  ImgColors *new_color;
  guint      key_colors = colors[0] * 256 * 256 + colors[1] * 256 + colors[2];

  if (table == NULL)
    {
      table = g_hash_table_new (g_direct_hash, g_direct_equal);
      count_color_entries = 0;
    }
  else
    {
      found_color = g_hash_table_lookup (table, GUINT_TO_POINTER (key_colors));
    }

  if (found_color == NULL)
    {
      if (count_color_entries > MAX_IMAGE_COLORS)
        {
          /* Don't add any more new ones */
          return table;
        }

      count_color_entries++;

      new_color = g_slice_new (ImgColors);

      new_color->count = 1;
      new_color->r_adj = 0;
      new_color->g_adj = 0;
      new_color->b_adj = 0;
      new_color->r     = colors[0];
      new_color->g     = colors[1];
      new_color->b     = colors[2];

      g_hash_table_insert (table, GUINT_TO_POINTER (key_colors), new_color);
    }
  else
    {
      new_color = found_color;

      if (new_color->count < (G_MAXINT - 1))
        new_color->count++;

      /* Now do the adjustments ...*/
      new_color->r_adj += (colors_real[0] - colors[0]);
      new_color->g_adj += (colors_real[1] - colors[1]);
      new_color->b_adj += (colors_real[2] - colors[2]);

      /* Boundary conditions */
      if(new_color->r_adj > (G_MAXINT - 255))
        new_color->r_adj /= new_color->count;

      if(new_color->g_adj > (G_MAXINT - 255))
        new_color->g_adj /= new_color->count;

      if(new_color->b_adj > (G_MAXINT - 255))
        new_color->b_adj /= new_color->count;
    }

  return table;
}

static void
picman_palette_import_create_list (gpointer key,
                                 gpointer value,
                                 gpointer user_data)
{
  GSList    **list      = user_data;
  ImgColors  *color_tab = value;

  *list = g_slist_prepend (*list, color_tab);
}

static gint
picman_palette_import_sort_colors (gconstpointer a,
                                 gconstpointer b)
{
  const ImgColors *s1 = a;
  const ImgColors *s2 = b;

  if(s1->count > s2->count)
    return -1;
  if(s1->count < s2->count)
    return 1;

  return 0;
}

static void
picman_palette_import_create_image_palette (gpointer data,
                                          gpointer user_data)
{
  PicmanPalette *palette   = user_data;
  ImgColors   *color_tab = data;
  gint         n_colors;
  gchar       *lab;
  PicmanRGB      color;

  n_colors = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (palette),
                                                 "import-n-colors"));

  if (picman_palette_get_n_colors (palette) >= n_colors)
    return;

  lab = g_strdup_printf ("%s (occurs %u)", _("Untitled"), color_tab->count);

  /* Adjust the colors to the mean of the the sample */
  picman_rgba_set_uchar
    (&color,
     (guchar) color_tab->r + (color_tab->r_adj / color_tab->count),
     (guchar) color_tab->g + (color_tab->g_adj / color_tab->count),
     (guchar) color_tab->b + (color_tab->b_adj / color_tab->count),
     255);

  picman_palette_add_entry (palette, -1, lab, &color);

  g_free (lab);
}

static PicmanPalette *
picman_palette_import_make_palette (GHashTable  *table,
                                  const gchar *palette_name,
                                  PicmanContext *context,
                                  gint         n_colors)
{
  PicmanPalette *palette;
  GSList      *list = NULL;
  GSList      *iter;

  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));

  if (! table)
    return palette;

  g_hash_table_foreach (table, picman_palette_import_create_list, &list);
  list = g_slist_sort (list, picman_palette_import_sort_colors);

  g_object_set_data (G_OBJECT (palette), "import-n-colors",
                     GINT_TO_POINTER (n_colors));

  g_slist_foreach (list, picman_palette_import_create_image_palette, palette);

  g_object_set_data (G_OBJECT (palette), "import-n-colors", NULL);

  /*  Free up used memory
   *  Note the same structure is on both the hash list and the sorted
   *  list. So only delete it once.
   */
  g_hash_table_destroy (table);

  for (iter = list; iter; iter = iter->next)
    g_slice_free (ImgColors, iter->data);

  g_slist_free (list);

  return palette;
}

static GHashTable *
picman_palette_import_extract (PicmanImage     *image,
                             PicmanPickable  *pickable,
                             gint           pickable_off_x,
                             gint           pickable_off_y,
                             gboolean       selection_only,
                             gint           x,
                             gint           y,
                             gint           width,
                             gint           height,
                             gint           n_colors,
                             gint           threshold)
{
  GeglBuffer         *buffer;
  GeglBufferIterator *iter;
  GeglRectangle      *mask_roi = NULL;
  GeglRectangle       rect     = { x, y, width, height };
  GHashTable         *colors   = NULL;
  const Babl         *format;
  gint                bpp;
  gint                mask_bpp = 0;

  buffer = picman_pickable_get_buffer (pickable);
  format = babl_format ("R'G'B'A u8");

  iter = gegl_buffer_iterator_new (buffer, &rect, 0, format,
                                   GEGL_BUFFER_READ, GEGL_ABYSS_NONE);
  bpp = babl_format_get_bytes_per_pixel (format);

  if (selection_only &&
      ! picman_channel_is_empty (picman_image_get_mask (image)))
    {
      PicmanDrawable *mask = PICMAN_DRAWABLE (picman_image_get_mask (image));

      rect.x = x + pickable_off_x;
      rect.y = y + pickable_off_y;

      buffer = picman_drawable_get_buffer (mask);
      format = babl_format ("Y u8");

      gegl_buffer_iterator_add (iter, buffer, &rect, 0, format,
                                GEGL_BUFFER_READ, GEGL_ABYSS_NONE);
      mask_roi = &iter->roi[1];
      mask_bpp = babl_format_get_bytes_per_pixel (format);
    }

  while (gegl_buffer_iterator_next (iter))
    {
      const guchar *data      = iter->data[0];
      const guchar *mask_data = NULL;

      if (mask_roi)
        mask_data = iter->data[1];

      while (iter->length--)
        {
          /*  ignore unselected, and completely transparent pixels  */
          if ((! mask_data || *mask_data) && data[ALPHA])
            {
              guchar rgba[MAX_CHANNELS]     = { 0, };
              guchar rgb_real[MAX_CHANNELS] = { 0, };

              memcpy (rgba, data, 4);
              memcpy (rgb_real, rgba, 4);

              rgba[0] = (rgba[0] / threshold) * threshold;
              rgba[1] = (rgba[1] / threshold) * threshold;
              rgba[2] = (rgba[2] / threshold) * threshold;

              colors = picman_palette_import_store_colors (colors,
                                                         rgba, rgb_real,
                                                         n_colors);
            }

          data += bpp;

          if (mask_data)
            mask_data += mask_bpp;
        }
    }

  return colors;
}

PicmanPalette *
picman_palette_import_from_image (PicmanImage   *image,
                                PicmanContext *context,
                                const gchar *palette_name,
                                gint         n_colors,
                                gint         threshold,
                                gboolean     selection_only)
{
  PicmanProjection *projection;
  GHashTable     *colors;
  gint            x, y;
  gint            width, height;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (palette_name != NULL, NULL);
  g_return_val_if_fail (n_colors > 1, NULL);
  g_return_val_if_fail (threshold > 0, NULL);

  projection = picman_image_get_projection (image);

  picman_pickable_flush (PICMAN_PICKABLE (projection));

  if (selection_only)
    {
      picman_channel_bounds (picman_image_get_mask (image),
                           &x, &y, &width, &height);

      width  -= x;
      height -= y;
    }
  else
    {
      x      = 0;
      y      = 0;
      width  = picman_image_get_width  (image);
      height = picman_image_get_height (image);
    }

  colors = picman_palette_import_extract (image,
                                        PICMAN_PICKABLE (projection),
                                        0, 0,
                                        selection_only,
                                        x, y, width, height,
                                        n_colors, threshold);

  return picman_palette_import_make_palette (colors, palette_name, context,
                                           n_colors);
}


/*  create a palette from an indexed image  **********************************/

PicmanPalette *
picman_palette_import_from_indexed_image (PicmanImage   *image,
                                        PicmanContext *context,
                                        const gchar *palette_name)
{
  PicmanPalette  *palette;
  const guchar *colormap;
  guint         n_colors;
  gint          count;
  PicmanRGB       color;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (picman_image_get_base_type (image) == PICMAN_INDEXED, NULL);
  g_return_val_if_fail (palette_name != NULL, NULL);

  palette = PICMAN_PALETTE (picman_palette_new (context, palette_name));

  colormap = picman_image_get_colormap (image);
  n_colors = picman_image_get_colormap_size (image);

  for (count = 0; count < n_colors; ++count)
    {
      gchar name[256];

      g_snprintf (name, sizeof (name), _("Index %d"), count);

      picman_rgba_set_uchar (&color,
                           colormap[count * 3 + 0],
                           colormap[count * 3 + 1],
                           colormap[count * 3 + 2],
                           255);

      picman_palette_add_entry (palette, -1, name, &color);
    }

  return palette;
}


/*  create a palette from a drawable  ****************************************/

PicmanPalette *
picman_palette_import_from_drawable (PicmanDrawable *drawable,
                                   PicmanContext  *context,
                                   const gchar  *palette_name,
                                   gint          n_colors,
                                   gint          threshold,
                                   gboolean      selection_only)
{
  GHashTable *colors = NULL;
  gint        x, y;
  gint        width, height;
  gint        off_x, off_y;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (palette_name != NULL, NULL);
  g_return_val_if_fail (n_colors > 1, NULL);
  g_return_val_if_fail (threshold > 0, NULL);

  if (selection_only)
    {
      if (! picman_item_mask_intersect (PICMAN_ITEM (drawable),
                                      &x, &y, &width, &height))
        return NULL;
    }
  else
    {
      x      = 0;
      y      = 0;
      width  = picman_item_get_width  (PICMAN_ITEM (drawable));
      height = picman_item_get_height (PICMAN_ITEM (drawable));
    }

  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  colors =
    picman_palette_import_extract (picman_item_get_image (PICMAN_ITEM (drawable)),
                                 PICMAN_PICKABLE (drawable),
                                 off_x, off_y,
                                 selection_only,
                                 x, y, width, height,
                                 n_colors, threshold);

  return picman_palette_import_make_palette (colors, palette_name, context,
                                           n_colors);
}


/*  create a palette from a file  **********************************/

PicmanPalette *
picman_palette_import_from_file (PicmanContext  *context,
                               const gchar  *filename,
                               const gchar  *palette_name,
                               GError      **error)
{
  GList *palette_list = NULL;

  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (palette_name != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  switch (picman_palette_load_detect_format (filename))
    {
    case PICMAN_PALETTE_FILE_FORMAT_GPL:
      palette_list = picman_palette_load (context, filename, error);
      break;

    case PICMAN_PALETTE_FILE_FORMAT_ACT:
      palette_list = picman_palette_load_act (context, filename, error);
      break;

    case PICMAN_PALETTE_FILE_FORMAT_RIFF_PAL:
      palette_list = picman_palette_load_riff (context, filename, error);
      break;

    case PICMAN_PALETTE_FILE_FORMAT_PSP_PAL:
      palette_list = picman_palette_load_psp (context, filename, error);
      break;

    case PICMAN_PALETTE_FILE_FORMAT_ACO:
      palette_list = picman_palette_load_aco (context, filename, error);
      break;

    case PICMAN_PALETTE_FILE_FORMAT_CSS:
      palette_list = picman_palette_load_css (context, filename, error);
      break;

    default:
      g_set_error (error,
                   PICMAN_DATA_ERROR, PICMAN_DATA_ERROR_READ,
                   _("Unknown type of palette file: %s"),
                   picman_filename_to_utf8 (filename));
      break;
    }

  if (palette_list)
    {
      PicmanPalette *palette = g_object_ref (palette_list->data);

      picman_object_set_name (PICMAN_OBJECT (palette), palette_name);

      g_list_free_full (palette_list, (GDestroyNotify) g_object_unref);

      return palette;
    }

  return NULL;
}
