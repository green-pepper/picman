/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrusheditor.h
 * Copyright 1998 Jay Cox <jaycox@earthlink.net>
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

#ifndef  __PICMAN_BRUSH_EDITOR_H__
#define  __PICMAN_BRUSH_EDITOR_H__


#include "picmandataeditor.h"


#define PICMAN_TYPE_BRUSH_EDITOR            (picman_brush_editor_get_type ())
#define PICMAN_BRUSH_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_EDITOR, PicmanBrushEditor))
#define PICMAN_BRUSH_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_EDITOR, PicmanBrushEditorClass))
#define PICMAN_IS_BRUSH_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_EDITOR))
#define PICMAN_IS_BRUSH_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_EDITOR))
#define PICMAN_BRUSH_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_EDITOR, PicmanBrushEditorClass))


typedef struct _PicmanBrushEditorClass PicmanBrushEditorClass;

struct _PicmanBrushEditor
{
  PicmanDataEditor  parent_instance;

  GtkWidget      *shape_group;
  GtkWidget      *options_box;
  GtkAdjustment  *radius_data;
  GtkAdjustment  *spikes_data;
  GtkAdjustment  *hardness_data;
  GtkAdjustment  *angle_data;
  GtkAdjustment  *aspect_ratio_data;
  GtkAdjustment  *spacing_data;
};

struct _PicmanBrushEditorClass
{
  PicmanDataEditorClass  parent_class;
};


GType       picman_brush_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_brush_editor_new      (PicmanContext     *context,
                                        PicmanMenuFactory *menu_factory);


#endif  /*  __PICMAN_BRUSH_EDITOR_H__  */
