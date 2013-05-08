/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmandrawable.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__PICMAN_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picman.h> can be included directly."
#endif

#ifndef __PICMAN_DRAWABLE_H__
#define __PICMAN_DRAWABLE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */

struct _PicmanDrawable
{
  gint32    drawable_id;   /* drawable ID */
  guint     width;         /* width of drawble */
  guint     height;        /* height of drawble */
  guint     bpp;           /* bytes per pixel of drawable */
  guint     ntile_rows;    /* # of tile rows */
  guint     ntile_cols;    /* # of tile columns */
  PicmanTile *tiles;         /* the normal tiles */
  PicmanTile *shadow_tiles;  /* the shadow tiles */
};


GeglBuffer   * picman_drawable_get_buffer             (gint32         drawable_ID);
GeglBuffer   * picman_drawable_get_shadow_buffer      (gint32         drawable_ID);

const Babl   * picman_drawable_get_format             (gint32         drawable_ID);

PICMAN_DEPRECATED_FOR(picman_drawable_get_buffer)
PicmanDrawable * picman_drawable_get                    (gint32         drawable_ID);
PICMAN_DEPRECATED
void           picman_drawable_detach                 (PicmanDrawable  *drawable);
PICMAN_DEPRECATED_FOR(gegl_buffer_flush)
void           picman_drawable_flush                  (PicmanDrawable  *drawable);
PICMAN_DEPRECATED_FOR(picman_drawable_get_buffer)
PicmanTile     * picman_drawable_get_tile               (PicmanDrawable  *drawable,
                                                     gboolean       shadow,
                                                     gint           row,
                                                     gint           col);
PICMAN_DEPRECATED_FOR(picman_drawable_get_buffer)
PicmanTile     * picman_drawable_get_tile2              (PicmanDrawable  *drawable,
                                                     gboolean       shadow,
                                                     gint           x,
                                                     gint           y);

PICMAN_DEPRECATED
void           picman_drawable_get_color_uchar        (gint32         drawable_ID,
                                                     const PicmanRGB *color,
                                                     guchar        *color_uchar);

guchar       * picman_drawable_get_thumbnail_data     (gint32         drawable_ID,
                                                     gint          *width,
                                                     gint          *height,
                                                     gint          *bpp);
guchar       * picman_drawable_get_sub_thumbnail_data (gint32         drawable_ID,
                                                     gint           src_x,
                                                     gint           src_y,
                                                     gint           src_width,
                                                     gint           src_height,
                                                     gint          *dest_width,
                                                     gint          *dest_height,
                                                     gint          *bpp);

PICMAN_DEPRECATED_FOR(picman_item_is_valid)
gboolean       picman_drawable_is_valid               (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_is_layer)
gboolean       picman_drawable_is_layer               (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_is_text_layer)
gboolean       picman_drawable_is_text_layer          (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_is_layer_mask)
gboolean       picman_drawable_is_layer_mask          (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_is_channel)
gboolean       picman_drawable_is_channel             (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_delete)
gboolean       picman_drawable_delete                 (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_get_image)
gint32         picman_drawable_get_image              (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_get_name)
gchar*         picman_drawable_get_name               (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_set_name)
gboolean       picman_drawable_set_name               (gint32              drawable_ID,
                                                     const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_item_get_visible)
gboolean       picman_drawable_get_visible            (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_get_visible)
gboolean       picman_drawable_set_visible            (gint32              drawable_ID,
                                                     gboolean            visible);
PICMAN_DEPRECATED_FOR(picman_item_get_linked)
gboolean       picman_drawable_get_linked             (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_set_linked)
gboolean       picman_drawable_set_linked             (gint32              drawable_ID,
                                                     gboolean            linked);
PICMAN_DEPRECATED_FOR(picman_item_get_tattoo)
gint           picman_drawable_get_tattoo             (gint32              drawable_ID);
PICMAN_DEPRECATED_FOR(picman_item_set_tattoo)
gboolean       picman_drawable_set_tattoo             (gint32              drawable_ID,
                                                     gint                tattoo);
PICMAN_DEPRECATED_FOR(picman_item_get_parasite)
PicmanParasite * picman_drawable_parasite_find          (gint32              drawable_ID,
                                                     const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_item_attach_parasite)
gboolean       picman_drawable_parasite_attach        (gint32              drawable_ID,
                                                     const PicmanParasite *parasite);
PICMAN_DEPRECATED_FOR(picman_item_detach_parasite)
gboolean       picman_drawable_parasite_detach        (gint32              drawable_ID,
                                                     const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_item_get_parasite_list)
gboolean       picman_drawable_parasite_list          (gint32              drawable_ID,
                                                     gint               *num_parasites,
                                                     gchar            ***parasites);
PICMAN_DEPRECATED_FOR(picman_item_attach_parasite)
gboolean       picman_drawable_attach_new_parasite    (gint32              drawable_ID,
                                                     const gchar        *name,
                                                     gint                flags,
                                                     gint                size,
                                                     gconstpointer       data);

G_END_DECLS

#endif /* __PICMAN_DRAWABLE_H__ */
