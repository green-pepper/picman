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

#ifndef __PICMAN_IMAGE_UNDO_H__
#define __PICMAN_IMAGE_UNDO_H__


#include "picmanundo.h"


#define PICMAN_TYPE_IMAGE_UNDO            (picman_image_undo_get_type ())
#define PICMAN_IMAGE_UNDO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_IMAGE_UNDO, PicmanImageUndo))
#define PICMAN_IMAGE_UNDO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_IMAGE_UNDO, PicmanImageUndoClass))
#define PICMAN_IS_IMAGE_UNDO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_IMAGE_UNDO))
#define PICMAN_IS_IMAGE_UNDO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_IMAGE_UNDO))
#define PICMAN_IMAGE_UNDO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_IMAGE_UNDO, PicmanImageUndoClass))


typedef struct _PicmanImageUndoClass PicmanImageUndoClass;

struct _PicmanImageUndo
{
  PicmanUndo           parent_instance;

  PicmanImageBaseType  base_type;
  PicmanPrecision      precision;
  gint               width;
  gint               height;
  gint               previous_origin_x;
  gint               previous_origin_y;
  gint               previous_width;
  gint               previous_height;
  gdouble            xresolution;
  gdouble            yresolution;
  PicmanUnit           resolution_unit;
  PicmanGrid          *grid;
  gint               num_colors;
  guchar            *colormap;
  gchar             *parasite_name;
  PicmanParasite      *parasite;
};

struct _PicmanImageUndoClass
{
  PicmanUndoClass  parent_class;
};


GType   picman_image_undo_get_type (void) G_GNUC_CONST;


#endif /* __PICMAN_IMAGE_UNDO_H__ */
