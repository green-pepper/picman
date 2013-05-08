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

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picman.h"
#include "picmancontainer.h"
#include "picmandatafactory.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-private.h"
#include "picmanimage-undo-push.h"
#include "picmanpalette.h"

#include "picman-intl.h"


/*  local function prototype  */

void   picman_image_colormap_set_palette_entry (PicmanImage *image,
                                              gint       index);


/*  public functions  */

void
picman_image_colormap_init (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanContainer    *palettes;
  gchar            *palette_name;
  gchar            *palette_id;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap == NULL);
  g_return_if_fail (private->palette == NULL);

  palette_name = g_strdup_printf (_("Colormap of Image #%d (%s)"),
                                  picman_image_get_ID (image),
                                  picman_image_get_display_name (image));
  palette_id = g_strdup_printf ("picman-indexed-image-palette-%d",
                                picman_image_get_ID (image));

  private->n_colors = 0;
  private->colormap = g_new0 (guchar, PICMAN_IMAGE_COLORMAP_SIZE);
  private->palette  = PICMAN_PALETTE (picman_palette_new (NULL, palette_name));

  if (! private->babl_palette_rgb)
    {
      gchar *format_name = g_strdup_printf ("-picman-indexed-format-%d",
                                            picman_image_get_ID (image));

      babl_new_palette (format_name,
                        &private->babl_palette_rgb,
                        &private->babl_palette_rgba);

      g_free (format_name);
    }

  picman_palette_set_columns (private->palette, 16);

  picman_data_make_internal (PICMAN_DATA (private->palette), palette_id);

  palettes = picman_data_factory_get_container (image->picman->palette_factory);

  picman_container_add (palettes, PICMAN_OBJECT (private->palette));

  g_free (palette_name);
  g_free (palette_id);
}

void
picman_image_colormap_dispose (PicmanImage *image)
{
  PicmanImagePrivate *private;
  PicmanContainer    *palettes;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap != NULL);
  g_return_if_fail (PICMAN_IS_PALETTE (private->palette));

  palettes = picman_data_factory_get_container (image->picman->palette_factory);

  picman_container_remove (palettes, PICMAN_OBJECT (private->palette));
}

void
picman_image_colormap_free (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap != NULL);
  g_return_if_fail (PICMAN_IS_PALETTE (private->palette));

  g_free (private->colormap);
  private->colormap = NULL;

  g_object_unref (private->palette);
  private->palette = NULL;

  /* don't touch the image's babl_palettes because we might still have
   * buffers with that palette on the undo stack, and on undoing the
   * image back to indexed, we must have exactly these palettes around
   */
}

const Babl *
picman_image_colormap_get_rgb_format (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->babl_palette_rgb;
}

const Babl *
picman_image_colormap_get_rgba_format (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->babl_palette_rgba;
}

PicmanPalette *
picman_image_get_colormap_palette (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->palette;
}

const guchar *
picman_image_get_colormap (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->colormap;
}

gint
picman_image_get_colormap_size (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), 0);

  return PICMAN_IMAGE_GET_PRIVATE (image)->n_colors;
}

void
picman_image_set_colormap (PicmanImage    *image,
                         const guchar *colormap,
                         gint          n_colors,
                         gboolean      push_undo)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (colormap != NULL || n_colors == 0);
  g_return_if_fail (n_colors >= 0 && n_colors <= 256);

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (push_undo)
    picman_image_undo_push_image_colormap (image, C_("undo-type", "Set Colormap"));

  if (private->colormap)
    memset (private->colormap, 0, PICMAN_IMAGE_COLORMAP_SIZE);

  if (colormap)
    {
      if (! private->colormap)
        {
          picman_image_colormap_init (image);
        }

      memcpy (private->colormap, colormap, n_colors * 3);
    }
  else if (private->colormap)
    {
      picman_image_colormap_dispose (image);
      picman_image_colormap_free (image);
    }

  private->n_colors = n_colors;

  if (private->palette)
    {
      PicmanPaletteEntry *entry;
      gint              i;

      picman_data_freeze (PICMAN_DATA (private->palette));

      while ((entry = picman_palette_get_entry (private->palette, 0)))
        picman_palette_delete_entry (private->palette, entry);

      for (i = 0; i < private->n_colors; i++)
        picman_image_colormap_set_palette_entry (image, i);

      picman_data_thaw (PICMAN_DATA (private->palette));
    }

  picman_image_colormap_changed (image, -1);
}

void
picman_image_get_colormap_entry (PicmanImage *image,
                               gint       color_index,
                               PicmanRGB   *color)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap != NULL);
  g_return_if_fail (color_index >= 0 && color_index < private->n_colors);
  g_return_if_fail (color != NULL);

  picman_rgba_set_uchar (color,
                       private->colormap[color_index * 3],
                       private->colormap[color_index * 3 + 1],
                       private->colormap[color_index * 3 + 2],
                       255);
}

void
picman_image_set_colormap_entry (PicmanImage     *image,
                               gint           color_index,
                               const PicmanRGB *color,
                               gboolean       push_undo)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap != NULL);
  g_return_if_fail (color_index >= 0 && color_index < private->n_colors);
  g_return_if_fail (color != NULL);

  if (push_undo)
    picman_image_undo_push_image_colormap (image,
                                         C_("undo-type", "Change Colormap entry"));

  picman_rgb_get_uchar (color,
                      &private->colormap[color_index * 3],
                      &private->colormap[color_index * 3 + 1],
                      &private->colormap[color_index * 3 + 2]);

  if (private->palette)
    picman_image_colormap_set_palette_entry (image, color_index);

  picman_image_colormap_changed (image, color_index);
}

void
picman_image_add_colormap_entry (PicmanImage     *image,
                               const PicmanRGB *color)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  g_return_if_fail (private->colormap != NULL);
  g_return_if_fail (private->n_colors < 256);
  g_return_if_fail (color != NULL);

  picman_image_undo_push_image_colormap (image,
                                       C_("undo-type", "Add Color to Colormap"));

  picman_rgb_get_uchar (color,
                      &private->colormap[private->n_colors * 3],
                      &private->colormap[private->n_colors * 3 + 1],
                      &private->colormap[private->n_colors * 3 + 2]);

  if (private->palette)
    picman_image_colormap_set_palette_entry (image, private->n_colors - 1);

  private->n_colors++;

  picman_image_colormap_changed (image, -1);
}


/*  private functions  */

void
picman_image_colormap_set_palette_entry (PicmanImage *image,
                                       gint       index)
{
  PicmanImagePrivate *private = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanRGB           color;
  gchar             name[64];

  picman_rgba_set_uchar (&color,
                       private->colormap[3 * index + 0],
                       private->colormap[3 * index + 1],
                       private->colormap[3 * index + 2],
                       255);

  g_snprintf (name, sizeof (name), "#%d", index);

  if (picman_palette_get_n_colors (private->palette) < private->n_colors)
    picman_palette_add_entry (private->palette, index, name, &color);
  else
    picman_palette_set_entry (private->palette, index, name, &color);
}
