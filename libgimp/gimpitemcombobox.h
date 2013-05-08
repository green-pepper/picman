/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanitemcombobox.h
 * Copyright (C) 2004 Sven Neumann <sven@picman.org>
 * Copyright (C) 2006 Simon Budig <simon@picman.org>
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

#if !defined (__PICMAN_UI_H_INSIDE__) && !defined (PICMAN_COMPILATION)
#error "Only <libpicman/picmanui.h> can be included directly."
#endif

#ifndef __PICMAN_ITEM_COMBO_BOX_H__
#define __PICMAN_ITEM_COMBO_BOX_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


#define PICMAN_TYPE_DRAWABLE_COMBO_BOX    (picman_drawable_combo_box_get_type ())
#define PICMAN_DRAWABLE_COMBO_BOX(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DRAWABLE_COMBO_BOX, PicmanDrawableComboBox))
#define PICMAN_IS_DRAWABLE_COMBO_BOX(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DRAWABLE_COMBO_BOX))

#define PICMAN_TYPE_CHANNEL_COMBO_BOX     (picman_channel_combo_box_get_type ())
#define PICMAN_CHANNEL_COMBO_BOX(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_CHANNEL_COMBO_BOX, PicmanChannelComboBox))
#define PICMAN_IS_CHANNEL_COMBO_BOX(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_CHANNEL_COMBO_BOX))

#define PICMAN_TYPE_LAYER_COMBO_BOX       (picman_layer_combo_box_get_type ())
#define PICMAN_LAYER_COMBO_BOX(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LAYER_COMBO_BOX, PicmanLayerComboBox))
#define PICMAN_IS_LAYER_COMBO_BOX(obj)    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LAYER_COMBO_BOX))

#define PICMAN_TYPE_VECTORS_COMBO_BOX     (picman_vectors_combo_box_get_type ())
#define PICMAN_VECTORS_COMBO_BOX(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_VECTORS_COMBO_BOX, PicmanVectorsComboBox))
#define PICMAN_IS_VECTORS_COMBO_BOX(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_VECTORS_COMBO_BOX))


typedef gboolean (* PicmanItemConstraintFunc) (gint32   image_id,
                                             gint32   item_id,
                                             gpointer data);

typedef PicmanItemConstraintFunc PicmanVectorsConstraintFunc;
typedef PicmanItemConstraintFunc PicmanDrawableConstraintFunc;


GType       picman_drawable_combo_box_get_type (void) G_GNUC_CONST;
GType       picman_channel_combo_box_get_type  (void) G_GNUC_CONST;
GType       picman_layer_combo_box_get_type    (void) G_GNUC_CONST;
GType       picman_vectors_combo_box_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_drawable_combo_box_new (PicmanDrawableConstraintFunc constraint,
                                         gpointer                   data);
GtkWidget * picman_channel_combo_box_new  (PicmanDrawableConstraintFunc constraint,
                                         gpointer                   data);
GtkWidget * picman_layer_combo_box_new    (PicmanDrawableConstraintFunc constraint,
                                         gpointer                   data);
GtkWidget * picman_vectors_combo_box_new  (PicmanVectorsConstraintFunc  constraint,
                                         gpointer                   data);


G_END_DECLS

#endif /* __PICMAN_ITEM_COMBO_BOX_H__ */
