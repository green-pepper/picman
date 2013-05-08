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

#ifndef __PICMAN_GROUP_LAYER_UNDO_H__
#define __PICMAN_GROUP_LAYER_UNDO_H__


#include "picmanitemundo.h"


#define PICMAN_TYPE_GROUP_LAYER_UNDO            (picman_group_layer_undo_get_type ())
#define PICMAN_GROUP_LAYER_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GROUP_LAYER_UNDO, PicmanGroupLayerUndo))
#define PICMAN_GROUP_LAYER_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GROUP_LAYER_UNDO, PicmanGroupLayerUndoClass))
#define PICMAN_IS_GROUP_LAYER_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GROUP_LAYER_UNDO))
#define PICMAN_IS_GROUP_LAYER_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GROUP_LAYER_UNDO))
#define PICMAN_GROUP_LAYER_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GROUP_LAYER_UNDO, PicmanGroupLayerUndoClass))


typedef struct _PicmanGroupLayerUndoClass PicmanGroupLayerUndoClass;

struct _PicmanGroupLayerUndo
{
  PicmanItemUndo       parent_instance;

  PicmanImageBaseType  prev_type;
  PicmanPrecision      prev_precision;
};

struct _PicmanGroupLayerUndoClass
{
  PicmanItemUndoClass  parent_class;
};


GType   picman_group_layer_undo_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_GROUP_LAYER_UNDO_H__ */
