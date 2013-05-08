/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * picmanlayer.h
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

#ifndef __PICMAN_LAYER_H__
#define __PICMAN_LAYER_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


gint32   picman_layer_new                (gint32                image_ID,
                                        const gchar          *name,
                                        gint                  width,
                                        gint                  height,
                                        PicmanImageType         type,
                                        gdouble               opacity,
                                        PicmanLayerModeEffects  mode);
gint32   picman_layer_copy               (gint32                layer_ID);

gint32   picman_layer_new_from_pixbuf    (gint32                image_ID,
                                        const gchar          *name,
                                        GdkPixbuf            *pixbuf,
                                        gdouble               opacity,
                                        PicmanLayerModeEffects  mode,
                                        gdouble               progress_start,
                                        gdouble               progress_end);
gint32   picman_layer_new_from_surface   (gint32                image_ID,
                                        const gchar          *name,
                                        cairo_surface_t      *surface,
                                        gdouble               progress_start,
                                        gdouble               progress_end);

PICMAN_DEPRECATED_FOR(picman_layer_get_lock_alpha)
gboolean picman_layer_get_preserve_trans (gint32                layer_ID);
PICMAN_DEPRECATED_FOR(picman_layer_set_lock_alpha)
gboolean picman_layer_set_preserve_trans (gint32                layer_ID,
                                        gboolean              preserve_trans);

G_END_DECLS

#endif /* __PICMAN_LAYER_H__ */
