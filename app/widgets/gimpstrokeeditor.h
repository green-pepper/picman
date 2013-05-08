/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanstrokeeditor.h
 * Copyright (C) 2003 Sven Neumann <sven@picman.org>
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

#ifndef __PICMAN_STROKE_EDITOR_H__
#define __PICMAN_STROKE_EDITOR_H__


#include "picmanfilleditor.h"


#define PICMAN_TYPE_STROKE_EDITOR            (picman_stroke_editor_get_type ())
#define PICMAN_STROKE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_STROKE_EDITOR, PicmanStrokeEditor))
#define PICMAN_STROKE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_STROKE_EDITOR, PicmanStrokeEditorClass))
#define PICMAN_IS_STROKE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_STROKE_EDITOR))
#define PICMAN_IS_STROKE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_STROKE_EDITOR))
#define PICMAN_STROKE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_STROKE_EDITOR, PicmanStrokeEditorClass))


typedef struct _PicmanStrokeEditorClass PicmanStrokeEditorClass;

struct _PicmanStrokeEditor
{
  PicmanFillEditor  parent_instance;

  gdouble         resolution;
};

struct _PicmanStrokeEditorClass
{
  PicmanFillEditorClass  parent_class;
};


GType       picman_stroke_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_stroke_editor_new      (PicmanStrokeOptions *options,
                                         gdouble            resolution,
                                         gboolean           edit_context);


#endif /* __PICMAN_STROKE_EDITOR_H__ */
