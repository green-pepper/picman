/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanTextLayer
 * Copyright (C) 2002-2003  Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_TEXT_LAYER_H__
#define __PICMAN_TEXT_LAYER_H__


#include "core/picmanlayer.h"


#define PICMAN_TYPE_TEXT_LAYER            (picman_text_layer_get_type ())
#define PICMAN_TEXT_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_TEXT_LAYER, PicmanTextLayer))
#define PICMAN_TEXT_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_TEXT_LAYER, PicmanTextLayerClass))
#define PICMAN_IS_TEXT_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_TEXT_LAYER))
#define PICMAN_IS_TEXT_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_TEXT_LAYER))
#define PICMAN_TEXT_LAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_TEXT_LAYER, PicmanTextLayerClass))


typedef struct _PicmanTextLayerClass PicmanTextLayerClass;

struct _PicmanTextLayer
{
  PicmanLayer     layer;

  PicmanText     *text;
  const gchar  *text_parasite;  /*  parasite name that this text was set from,
                                 *  and that should be removed when the text
                                 *  is changed.
                                 */
  gboolean      auto_rename;
  gboolean      modified;

  const Babl   *convert_format;
};

struct _PicmanTextLayerClass
{
  PicmanLayerClass  parent_class;
};


GType       picman_text_layer_get_type    (void) G_GNUC_CONST;

PicmanLayer * picman_text_layer_new         (PicmanImage     *image,
                                         PicmanText      *text);
PicmanText  * picman_text_layer_get_text    (PicmanTextLayer *layer);
void        picman_text_layer_set_text    (PicmanTextLayer *layer,
                                         PicmanText      *text);
void        picman_text_layer_discard     (PicmanTextLayer *layer);
void        picman_text_layer_set         (PicmanTextLayer *layer,
                                         const gchar   *undo_desc,
                                         const gchar   *first_property_name,
                                         ...) G_GNUC_NULL_TERMINATED;

gboolean    picman_item_is_text_layer     (PicmanItem      *item);


#endif /* __PICMAN_TEXT_LAYER_H__ */
