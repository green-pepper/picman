/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * picmanimage.h
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

#ifndef __PICMAN_IMAGE_H__
#define __PICMAN_IMAGE_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


guchar       * picman_image_get_colormap            (gint32              image_ID,
                                                   gint               *num_colors);
gboolean       picman_image_set_colormap            (gint32              image_ID,
                                                   const guchar       *colormap,
                                                   gint                num_colors);

guchar       * picman_image_get_thumbnail_data      (gint32              image_ID,
                                                   gint               *width,
                                                   gint               *height,
                                                   gint               *bpp);

PICMAN_DEPRECATED_FOR(picman_image_get_colormap)
guchar       * picman_image_get_cmap                (gint32              image_ID,
                                                   gint               *num_colors);
PICMAN_DEPRECATED_FOR(picman_image_set_colormap)
gboolean       picman_image_set_cmap                (gint32              image_ID,
                                                   const guchar       *cmap,
                                                   gint                num_colors);
PICMAN_DEPRECATED_FOR(picman_image_get_item_position)
gint           picman_image_get_layer_position      (gint32              image_ID,
                                                   gint32              layer_ID);
PICMAN_DEPRECATED_FOR(picman_image_raise_item)
gboolean       picman_image_raise_layer             (gint32              image_ID,
                                                   gint32              layer_ID);
PICMAN_DEPRECATED_FOR(picman_image_lower_item)
gboolean       picman_image_lower_layer             (gint32              image_ID,
                                                   gint32              layer_ID);
PICMAN_DEPRECATED_FOR(picman_image_raise_item_to_top)
gboolean       picman_image_raise_layer_to_top      (gint32              image_ID,
                                                   gint32              layer_ID);
PICMAN_DEPRECATED_FOR(picman_image_lower_item_to_bottom)
gboolean       picman_image_lower_layer_to_bottom   (gint32              image_ID,
                                                   gint32              layer_ID);
PICMAN_DEPRECATED_FOR(picman_image_get_item_position)
gint           picman_image_get_channel_position    (gint32              image_ID,
                                                   gint32              channel_ID);
PICMAN_DEPRECATED_FOR(picman_image_raise_item)
gboolean       picman_image_raise_channel           (gint32              image_ID,
                                                   gint32              channel_ID);
PICMAN_DEPRECATED_FOR(picman_image_lower_item)
gboolean       picman_image_lower_channel           (gint32              image_ID,
                                                   gint32              channel_ID);
PICMAN_DEPRECATED_FOR(picman_image_get_item_position)
gint           picman_image_get_vectors_position    (gint32              image_ID,
                                                   gint32              vectors_ID);
PICMAN_DEPRECATED_FOR(picman_image_raise_item)
gboolean       picman_image_raise_vectors           (gint32              image_ID,
                                                   gint32              vectors_ID);
PICMAN_DEPRECATED_FOR(picman_image_lower_item)
gboolean       picman_image_lower_vectors           (gint32              image_ID,
                                                   gint32              vectors_ID);
PICMAN_DEPRECATED_FOR(picman_image_raise_item_to_top)
gboolean       picman_image_raise_vectors_to_top    (gint32              image_ID,
                                                   gint32              vectors_ID);
PICMAN_DEPRECATED_FOR(picman_image_lower_item_to_bottom)
gboolean       picman_image_lower_vectors_to_bottom (gint32              image_ID,
                                                   gint32              vectors_ID);
PICMAN_DEPRECATED_FOR(picman_image_get_parasite)
PicmanParasite * picman_image_parasite_find           (gint32              image_ID,
                                                   const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_image_attach_parasite)
gboolean       picman_image_parasite_attach         (gint32              image_ID,
                                                   const PicmanParasite *parasite);
PICMAN_DEPRECATED_FOR(picman_image_detach_parasite)
gboolean       picman_image_parasite_detach         (gint32              image_ID,
                                                   const gchar        *name);
PICMAN_DEPRECATED_FOR(picman_image_get_parasite_list)
gboolean       picman_image_parasite_list           (gint32              image_ID,
                                                   gint               *num_parasites,
                                                   gchar            ***parasites);
PICMAN_DEPRECATED_FOR(picman_image_attach_parasite)
gboolean       picman_image_attach_new_parasite     (gint32              image_ID,
                                                   const gchar        *name,
                                                   gint                flags,
                                                   gint                size,
                                                   gconstpointer       data);

G_END_DECLS

#endif /* __PICMAN_IMAGE_H__ */
