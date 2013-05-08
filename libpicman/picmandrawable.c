/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmandrawable.c
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#define PICMAN_DISABLE_DEPRECATION_WARNINGS

#include "picman.h"

#include "picmantilebackendplugin.h"


#define TILE_WIDTH  picman_tile_width()
#define TILE_HEIGHT picman_tile_height()


/**
 * picman_drawable_get:
 * @drawable_ID: the ID of the drawable
 *
 * This function creates a #PicmanDrawable structure for the core
 * drawable identified by @drawable_ID. The returned structure
 * contains some basic information about the drawable and can also
 * hold tile data for transfer to and from the core.
 *
 * Note that the name of this function is somewhat misleading, because
 * it suggests that it simply returns a handle.  This is not the case:
 * if the function is called multiple times, it creates separate tile
 * lists each time, which will usually produce undesired results.
 *
 * When a plug-in has finished working with a drawable, before exiting
 * it should call picman_drawable_detach() to make sure that all tile data is
 * transferred back to the core.
 *
 * Return value: a new #PicmanDrawable wrapper
 **/
PicmanDrawable *
picman_drawable_get (gint32 drawable_ID)
{
  PicmanDrawable *drawable;
  gint          width;
  gint          height;
  gint          bpp;

  width  = picman_drawable_width  (drawable_ID);
  height = picman_drawable_height (drawable_ID);
  bpp    = picman_drawable_bpp    (drawable_ID);

  g_return_val_if_fail (width > 0 && height > 0 && bpp > 0, NULL);

  drawable = g_slice_new0 (PicmanDrawable);

  drawable->drawable_id  = drawable_ID;
  drawable->width        = width;
  drawable->height       = height;
  drawable->bpp          = bpp;
  drawable->ntile_rows   = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
  drawable->ntile_cols   = (width  + TILE_WIDTH  - 1) / TILE_WIDTH;

  return drawable;
}

/**
 * picman_drawable_detach:
 * @drawable: The #PicmanDrawable to detach from the core
 *
 * This function is called when a plug-in is finished working
 * with a drawable.  It forces all tile data held in the tile
 * list of the #PicmanDrawable to be transferred to the core, and
 * then frees all associated memory. You must not access the
 * @drawable after having called picman_drawable_detach().
 **/
void
picman_drawable_detach (PicmanDrawable *drawable)
{
  g_return_if_fail (drawable != NULL);

  picman_drawable_flush (drawable);

  if (drawable->tiles)
    g_free (drawable->tiles);

  if (drawable->shadow_tiles)
    g_free (drawable->shadow_tiles);

  g_slice_free (PicmanDrawable, drawable);
}

/**
 * picman_drawable_flush:
 * @drawable: The #PicmanDrawable whose tile data is to be transferred
 * to the core.
 *
 * This function causes all tile data in the tile list of @drawable to be
 * transferred to the core.  It is usually called in situations where a
 * plug-in acts on a drawable, and then needs to read the results of its
 * actions.  Data transferred back from the core will not generally be valid
 * unless picman_drawable_flush() has been called beforehand.
 **/
void
picman_drawable_flush (PicmanDrawable *drawable)
{
  PicmanTile *tiles;
  gint      n_tiles;
  gint      i;

  g_return_if_fail (drawable != NULL);

  if (drawable->tiles)
    {
      tiles   = drawable->tiles;
      n_tiles = drawable->ntile_rows * drawable->ntile_cols;

      for (i = 0; i < n_tiles; i++)
        if ((tiles[i].ref_count > 0) && tiles[i].dirty)
          picman_tile_flush (&tiles[i]);
    }

  if (drawable->shadow_tiles)
    {
      tiles   = drawable->shadow_tiles;
      n_tiles = drawable->ntile_rows * drawable->ntile_cols;

      for (i = 0; i < n_tiles; i++)
        if ((tiles[i].ref_count > 0) && tiles[i].dirty)
          picman_tile_flush (&tiles[i]);
    }

  /*  nuke all references to this drawable from the cache  */
  _picman_tile_cache_flush_drawable (drawable);
}

PicmanTile *
picman_drawable_get_tile (PicmanDrawable *drawable,
                        gboolean      shadow,
                        gint          row,
                        gint          col)
{
  PicmanTile *tiles;
  guint     right_tile;
  guint     bottom_tile;
  gint      n_tiles;
  gint      tile_num;
  gint      i, j, k;

  g_return_val_if_fail (drawable != NULL, NULL);

  if (shadow)
    tiles = drawable->shadow_tiles;
  else
    tiles = drawable->tiles;

  if (! tiles)
    {
      n_tiles = drawable->ntile_rows * drawable->ntile_cols;
      tiles = g_new (PicmanTile, n_tiles);

      right_tile  = (drawable->width  -
                     ((drawable->ntile_cols - 1) * TILE_WIDTH));
      bottom_tile = (drawable->height -
                     ((drawable->ntile_rows - 1) * TILE_HEIGHT));

      for (i = 0, k = 0; i < drawable->ntile_rows; i++)
        {
          for (j = 0; j < drawable->ntile_cols; j++, k++)
            {
              tiles[k].bpp       = drawable->bpp;
              tiles[k].tile_num  = k;
              tiles[k].ref_count = 0;
              tiles[k].dirty     = FALSE;
              tiles[k].shadow    = shadow;
              tiles[k].data      = NULL;
              tiles[k].drawable  = drawable;

              if (j == (drawable->ntile_cols - 1))
                tiles[k].ewidth  = right_tile;
              else
                tiles[k].ewidth  = TILE_WIDTH;

              if (i == (drawable->ntile_rows - 1))
                tiles[k].eheight = bottom_tile;
              else
                tiles[k].eheight = TILE_HEIGHT;
            }
        }

      if (shadow)
        drawable->shadow_tiles = tiles;
      else
        drawable->tiles = tiles;
    }

  tile_num = row * drawable->ntile_cols + col;

  return &tiles[tile_num];
}

PicmanTile *
picman_drawable_get_tile2 (PicmanDrawable *drawable,
                         gboolean      shadow,
                         gint          x,
                         gint          y)
{
  gint row;
  gint col;

  g_return_val_if_fail (drawable != NULL, NULL);

  col = x / TILE_WIDTH;
  row = y / TILE_HEIGHT;

  return picman_drawable_get_tile (drawable, shadow, row, col);
}

void
picman_drawable_get_color_uchar (gint32         drawable_ID,
                               const PicmanRGB *color,
                               guchar        *color_uchar)
{
  g_return_if_fail (color != NULL);
  g_return_if_fail (color_uchar != NULL);

  switch (picman_drawable_type (drawable_ID))
    {
    case PICMAN_RGB_IMAGE:
      picman_rgb_get_uchar (color,
                          &color_uchar[0], &color_uchar[1], &color_uchar[2]);
      color_uchar[3] = 255;
      break;

    case PICMAN_RGBA_IMAGE:
      picman_rgba_get_uchar (color,
                           &color_uchar[0], &color_uchar[1], &color_uchar[2],
                           &color_uchar[3]);
      break;

    case PICMAN_GRAY_IMAGE:
      color_uchar[0] = picman_rgb_luminance_uchar (color);
      color_uchar[1] = 255;
      break;

    case PICMAN_GRAYA_IMAGE:
      color_uchar[0] = picman_rgb_luminance_uchar (color);
      picman_rgba_get_uchar (color, NULL, NULL, NULL, &color_uchar[1]);
      break;

    default:
      break;
    }
}

guchar *
picman_drawable_get_thumbnail_data (gint32  drawable_ID,
                                  gint   *width,
                                  gint   *height,
                                  gint   *bpp)
{
  gint    ret_width;
  gint    ret_height;
  guchar *image_data;
  gint    data_size;

  _picman_drawable_thumbnail (drawable_ID,
                            *width,
                            *height,
                            &ret_width,
                            &ret_height,
                            bpp,
                            &data_size,
                            &image_data);

  *width  = ret_width;
  *height = ret_height;

  return image_data;
}

guchar *
picman_drawable_get_sub_thumbnail_data (gint32  drawable_ID,
                                      gint    src_x,
                                      gint    src_y,
                                      gint    src_width,
                                      gint    src_height,
                                      gint   *dest_width,
                                      gint   *dest_height,
                                      gint   *bpp)
{
  gint    ret_width;
  gint    ret_height;
  guchar *image_data;
  gint    data_size;

  _picman_drawable_sub_thumbnail (drawable_ID,
                                src_x, src_y,
                                src_width, src_height,
                                *dest_width,
                                *dest_height,
                                &ret_width,
                                &ret_height,
                                bpp,
                                &data_size,
                                &image_data);

  *dest_width  = ret_width;
  *dest_height = ret_height;

  return image_data;
}

/**
 * picman_drawable_is_valid:
 * @drawable_ID: The drawable to check.
 *
 * Deprecated: Use picman_item_is_valid() instead.
 *
 * Returns: Whether the drawable ID is valid.
 *
 * Since: PICMAN 2.4
 */
gboolean
picman_drawable_is_valid (gint32 drawable_ID)
{
  return picman_item_is_valid (drawable_ID);
}

/**
 * picman_drawable_is_layer:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_is_layer() instead.
 *
 * Returns: TRUE if the drawable is a layer, FALSE otherwise.
 */
gboolean
picman_drawable_is_layer (gint32 drawable_ID)
{
  return picman_item_is_layer (drawable_ID);
}

/**
 * picman_drawable_is_text_layer:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_is_text_layer() instead.
 *
 * Returns: TRUE if the drawable is a text layer, FALSE otherwise.
 *
 * Since: PICMAN 2.6
 */
gboolean
picman_drawable_is_text_layer (gint32 drawable_ID)
{
  return picman_item_is_text_layer (drawable_ID);
}

/**
 * picman_drawable_is_layer_mask:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_is_layer_mask() instead.
 *
 * Returns: TRUE if the drawable is a layer mask, FALSE otherwise.
 */
gboolean
picman_drawable_is_layer_mask (gint32 drawable_ID)
{
  return picman_item_is_layer_mask (drawable_ID);
}

/**
 * picman_drawable_is_channel:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_is_channel() instead.
 *
 * Returns: TRUE if the drawable is a channel, FALSE otherwise.
 */
gboolean
picman_drawable_is_channel (gint32 drawable_ID)
{
  return picman_item_is_channel (drawable_ID);
}

/**
 * picman_drawable_delete:
 * @drawable_ID: The drawable to delete.
 *
 * Deprecated: Use picman_item_delete() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_drawable_delete (gint32 drawable_ID)
{
  return picman_item_delete (drawable_ID);
}

/**
 * picman_drawable_get_image:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_get_image() instead.
 *
 * Returns: The drawable's image.
 */
gint32
picman_drawable_get_image (gint32 drawable_ID)
{
  return picman_item_get_image (drawable_ID);
}

/**
 * picman_drawable_get_name:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_get_name() instead.
 *
 * Returns: The drawable name.
 */
gchar *
picman_drawable_get_name (gint32 drawable_ID)
{
  return picman_item_get_name (drawable_ID);
}

/**
 * picman_drawable_set_name:
 * @drawable_ID: The drawable.
 * @name: The new drawable name.
 *
 * Deprecated: Use picman_item_set_name() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_drawable_set_name (gint32       drawable_ID,
                        const gchar *name)
{
  return picman_item_set_name (drawable_ID, name);
}

/**
 * picman_drawable_get_visible:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_get_visible() instead.
 *
 * Returns: The drawable visibility.
 */
gboolean
picman_drawable_get_visible (gint32 drawable_ID)
{
  return picman_item_get_visible (drawable_ID);
}

/**
 * picman_drawable_set_visible:
 * @drawable_ID: The drawable.
 * @visible: The new drawable visibility.
 *
 * Deprecated: Use picman_item_set_visible() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_drawable_set_visible (gint32   drawable_ID,
                           gboolean visible)
{
  return picman_item_set_visible (drawable_ID, visible);
}

/**
 * picman_drawable_get_linked:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_get_linked() instead.
 *
 * Returns: The drawable linked state (for moves).
 */
gboolean
picman_drawable_get_linked (gint32 drawable_ID)
{
  return picman_item_get_linked (drawable_ID);
}

/**
 * picman_drawable_set_linked:
 * @drawable_ID: The drawable.
 * @linked: The new drawable linked state.
 *
 * Deprecated: Use picman_item_set_linked() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_drawable_set_linked (gint32   drawable_ID,
                          gboolean linked)
{
  return picman_item_set_linked (drawable_ID, linked);
}

/**
 * picman_drawable_get_tattoo:
 * @drawable_ID: The drawable.
 *
 * Deprecated: Use picman_item_get_tattoo() instead.
 *
 * Returns: The drawable tattoo.
 */
gint
picman_drawable_get_tattoo (gint32 drawable_ID)
{
  return picman_item_get_tattoo (drawable_ID);
}

/**
 * picman_drawable_set_tattoo:
 * @drawable_ID: The drawable.
 * @tattoo: The new drawable tattoo.
 *
 * Deprecated: Use picman_item_set_tattoo() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
picman_drawable_set_tattoo (gint32 drawable_ID,
                          gint   tattoo)
{
  return picman_item_set_tattoo (drawable_ID, tattoo);
}

/**
 * picman_drawable_parasite_find:
 * @drawable_ID: The drawable.
 * @name: The name of the parasite to find.
 *
 * Deprecated: Use picman_item_get_parasite() instead.
 *
 * Returns: The found parasite.
 **/
PicmanParasite *
picman_drawable_parasite_find (gint32       drawable_ID,
                             const gchar *name)
{
  return picman_item_get_parasite (drawable_ID, name);
}

/**
 * picman_drawable_parasite_attach:
 * @drawable_ID: The drawable.
 * @parasite: The parasite to attach to a drawable.
 *
 * Deprecated: Use picman_item_attach_parasite() instead.
 *
 * Returns: TRUE on success.
 **/
gboolean
picman_drawable_parasite_attach (gint32              drawable_ID,
                               const PicmanParasite *parasite)
{
  return picman_item_attach_parasite (drawable_ID, parasite);
}

/**
 * picman_drawable_parasite_detach:
 * @drawable_ID: The drawable.
 * @name: The name of the parasite to detach from a drawable.
 *
 * Deprecated: Use picman_item_detach_parasite() instead.
 *
 * Returns: TRUE on success.
 **/
gboolean
picman_drawable_parasite_detach (gint32       drawable_ID,
                               const gchar *name)
{
  return picman_item_detach_parasite (drawable_ID, name);
}

/**
 * picman_drawable_parasite_list:
 * @drawable_ID: The drawable.
 * @num_parasites: The number of attached parasites.
 * @parasites: The names of currently attached parasites.
 *
 * Deprecated: Use picman_item_get_parasite_list() instead.
 *
 * Returns: TRUE on success.
 **/
gboolean
picman_drawable_parasite_list (gint32    drawable_ID,
                             gint     *num_parasites,
                             gchar  ***parasites)
{
  *parasites = picman_item_get_parasite_list (drawable_ID, num_parasites);

  return *parasites != NULL;
}

/**
 * picman_drawable_attach_new_parasite:
 * @drawable_ID: the ID of the #PicmanDrawable to attach the #PicmanParasite to.
 * @name: the name of the #PicmanParasite to create and attach.
 * @flags: the flags set on the #PicmanParasite.
 * @size: the size of the parasite data in bytes.
 * @data: a pointer to the data attached with the #PicmanParasite.
 *
 * Convenience function that creates a parasite and attaches it
 * to PICMAN.
 *
 * Deprecated: use picman_item_attach_parasite() instead.
 *
 * Return value: TRUE on successful creation and attachment of
 * the new parasite.
 *
 * See Also: picman_drawable_parasite_attach()
 */
gboolean
picman_drawable_attach_new_parasite (gint32          drawable_ID,
                                   const gchar    *name,
                                   gint            flags,
                                   gint            size,
                                   gconstpointer   data)
{
  PicmanParasite *parasite = picman_parasite_new (name, flags, size, data);
  gboolean      success;

  success = picman_item_attach_parasite (drawable_ID, parasite);

  picman_parasite_free (parasite);

  return success;
}

/**
 * picman_drawable_get_buffer:
 * @drawable_ID: the ID of the #PicmanDrawable to get the buffer for.
 *
 * Returns a #GeglBuffer of a specified drawable. The buffer can be used
 * like any other GEGL buffer. Its data will we synced back with the core
 * drawable when the buffer gets destroyed, or when gegl_buffer_flush()
 * is called.
 *
 * Return value: The #GeglBuffer.
 *
 * See Also: picman_drawable_get_shadow_buffer()
 *
 * Since: PICMAN 2.10
 */
GeglBuffer *
picman_drawable_get_buffer (gint32 drawable_ID)
{
  picman_plugin_enable_precision ();

  if (picman_item_is_valid (drawable_ID))
    {
      PicmanDrawable *drawable;

      drawable = picman_drawable_get (drawable_ID);

      if (drawable)
        {
          GeglTileBackend *backend;
          GeglBuffer      *buffer;

          backend = _picman_tile_backend_plugin_new (drawable, FALSE);
          buffer = gegl_buffer_new_for_backend (NULL, backend);
          g_object_unref (backend);

          return buffer;
        }
    }

  return NULL;
}

/**
 * picman_drawable_get_shadow_buffer:
 * @drawable_ID: the ID of the #PicmanDrawable to get the buffer for.
 *
 * Returns a #GeglBuffer of a specified drawable's shadow tiles. The
 * buffer can be used like any other GEGL buffer. Its data will we
 * synced back with the core drawable's shadow tiles when the buffer
 * gets destroyed, or when gegl_buffer_flush() is called.
 *
 * Return value: The #GeglBuffer.
 *
 * See Also: picman_drawable_get_shadow_buffer()
 *
 * Since: PICMAN 2.10
 */
GeglBuffer *
picman_drawable_get_shadow_buffer (gint32 drawable_ID)
{
  PicmanDrawable *drawable;

  picman_plugin_enable_precision ();

  drawable = picman_drawable_get (drawable_ID);

  if (drawable)
    {
      GeglTileBackend *backend;
      GeglBuffer      *buffer;

      backend = _picman_tile_backend_plugin_new (drawable, TRUE);
      buffer = gegl_buffer_new_for_backend (NULL, backend);
      g_object_unref (backend);

      return buffer;
    }

  return NULL;
}

/**
 * picman_drawable_get_format:
 * @drawable_ID: the ID of the #PicmanDrawable to get the format for.
 *
 * Returns the #Babl format of the drawable.
 *
 * Return value: The #Babl format.
 *
 * Since: PICMAN 2.10
 */
const Babl *
picman_drawable_get_format (gint32 drawable_ID)
{
  static GHashTable *palette_formats = NULL;
  const Babl *format     = NULL;
  gchar      *format_str = _picman_drawable_get_format (drawable_ID);

  if (format_str)
    {
      if (picman_drawable_is_indexed (drawable_ID))
        {
          gint32      image_ID = picman_item_get_image (drawable_ID);
          guchar     *colormap;
          gint        n_colors;

          colormap = picman_image_get_colormap (image_ID, &n_colors);

          if (!palette_formats)
            palette_formats = g_hash_table_new (g_str_hash, g_str_equal);

          format = g_hash_table_lookup (palette_formats, format_str);

          if (!format)
            {
              const Babl *palette;
              const Babl *palette_alpha;

              babl_new_palette (format_str, &palette, &palette_alpha);
              g_hash_table_insert (palette_formats,
                                   (gpointer) babl_get_name (palette),
                                   (gpointer) palette);
              g_hash_table_insert (palette_formats,
                                   (gpointer) babl_get_name (palette_alpha),
                                   (gpointer) palette_alpha);

              if (picman_drawable_has_alpha (drawable_ID))
                format = palette_alpha;
              else
                format = palette;
            }

          if (colormap)
            {
              babl_palette_set_palette (format,
                                        babl_format ("R'G'B' u8"),
                                        colormap, n_colors);
              g_free (colormap);
            }
        }
      else
        {
          format = babl_format (format_str);
        }

      g_free (format_str);
    }

  return format;
}
