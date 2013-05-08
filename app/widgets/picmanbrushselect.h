/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushselect.h
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_BRUSH_SELECT_H__
#define __PICMAN_BRUSH_SELECT_H__

#include "picmanpdbdialog.h"

G_BEGIN_DECLS


#define PICMAN_TYPE_BRUSH_SELECT            (picman_brush_select_get_type ())
#define PICMAN_BRUSH_SELECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_BRUSH_SELECT, PicmanBrushSelect))
#define PICMAN_BRUSH_SELECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_BRUSH_SELECT, PicmanBrushSelectClass))
#define PICMAN_IS_BRUSH_SELECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_BRUSH_SELECT))
#define PICMAN_IS_BRUSH_SELECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_BRUSH_SELECT))
#define PICMAN_BRUSH_SELECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_BRUSH_SELECT, PicmanBrushSelectClass))


typedef struct _PicmanBrushSelectClass  PicmanBrushSelectClass;

struct _PicmanBrushSelect
{
  PicmanPdbDialog         parent_instance;

  gdouble               initial_opacity;
  PicmanLayerModeEffects  initial_mode;

  gint                  spacing;
  GtkAdjustment        *opacity_data;
  GtkWidget            *paint_mode_menu;
};

struct _PicmanBrushSelectClass
{
  PicmanPdbDialogClass  parent_class;
};


GType  picman_brush_select_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __PICMAN_BRUSH_SELECT_H__ */
