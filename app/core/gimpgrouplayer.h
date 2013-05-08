/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGroupLayer
 * Copyright (C) 2009  Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_GROUP_LAYER_H__
#define __PICMAN_GROUP_LAYER_H__


#include "core/picmanlayer.h"


#define PICMAN_TYPE_GROUP_LAYER            (picman_group_layer_get_type ())
#define PICMAN_GROUP_LAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GROUP_LAYER, PicmanGroupLayer))
#define PICMAN_GROUP_LAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GROUP_LAYER, PicmanGroupLayerClass))
#define PICMAN_IS_GROUP_LAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GROUP_LAYER))
#define PICMAN_IS_GROUP_LAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GROUP_LAYER))
#define PICMAN_GROUP_LAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GROUP_LAYER, PicmanGroupLayerClass))


typedef struct _PicmanGroupLayerClass PicmanGroupLayerClass;

struct _PicmanGroupLayer
{
  PicmanLayer  parent_instance;
};

struct _PicmanGroupLayerClass
{
  PicmanLayerClass  parent_class;
};


GType            picman_group_layer_get_type       (void) G_GNUC_CONST;

PicmanLayer      * picman_group_layer_new            (PicmanImage      *image);

PicmanProjection * picman_group_layer_get_projection (PicmanGroupLayer *group);

void             picman_group_layer_suspend_resize (PicmanGroupLayer *group,
                                                  gboolean        push_undo);
void             picman_group_layer_resume_resize  (PicmanGroupLayer *group,
                                                  gboolean        push_undo);


#endif /* __PICMAN_GROUP_LAYER_H__ */
