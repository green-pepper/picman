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

#ifndef __PICMAN_LAYER_MASK_H__
#define __PICMAN_LAYER_MASK_H__


#include "picmanchannel.h"


#define PICMAN_TYPE_LAYER_MASK            (picman_layer_mask_get_type ())
#define PICMAN_LAYER_MASK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_LAYER_MASK, PicmanLayerMask))
#define PICMAN_LAYER_MASK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_LAYER_MASK, PicmanLayerMaskClass))
#define PICMAN_IS_LAYER_MASK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_LAYER_MASK))
#define PICMAN_IS_LAYER_MASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_LAYER_MASK))
#define PICMAN_LAYER_MASK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_LAYER_MASK, PicmanLayerMaskClass))


typedef struct _PicmanLayerMaskClass  PicmanLayerMaskClass;

struct _PicmanLayerMask
{
  PicmanChannel  parent_instance;

  PicmanLayer   *layer;
};

struct _PicmanLayerMaskClass
{
  PicmanChannelClass  parent_class;
};


/*  function declarations  */

GType           picman_layer_mask_get_type  (void) G_GNUC_CONST;

PicmanLayerMask * picman_layer_mask_new       (PicmanImage           *image,
                                           gint                 width,
                                           gint                 height,
                                           const gchar         *name,
                                           const PicmanRGB       *color);

void            picman_layer_mask_set_layer (PicmanLayerMask       *layer_mask,
                                           PicmanLayer           *layer);
PicmanLayer     * picman_layer_mask_get_layer (const PicmanLayerMask *layer_mask);


#endif /* __PICMAN_LAYER_MASK_H__ */
