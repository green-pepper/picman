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

#ifndef __PICMAN_COLORMAP_EDITOR_H__
#define __PICMAN_COLORMAP_EDITOR_H__


#include "picmanimageeditor.h"


#define PICMAN_TYPE_COLORMAP_EDITOR            (picman_colormap_editor_get_type ())
#define PICMAN_COLORMAP_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_COLORMAP_EDITOR, PicmanColormapEditor))
#define PICMAN_COLORMAP_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_COLORMAP_EDITOR, PicmanColormapEditorClass))
#define PICMAN_IS_COLORMAP_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_COLORMAP_EDITOR))
#define PICMAN_IS_COLORMAP_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_COLORMAP_EDITOR))
#define PICMAN_COLORMAP_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_COLORMAP_EDITOR, PicmanColormapEditorClass))


typedef struct _PicmanColormapEditorClass PicmanColormapEditorClass;

struct _PicmanColormapEditor
{
  PicmanImageEditor  parent_instance;

  GtkWidget       *view;
  gint             col_index;

  PangoLayout     *layout;

  GtkAdjustment   *index_adjustment;
  GtkWidget       *index_spinbutton;
  GtkWidget       *color_entry;

  GtkWidget       *color_dialog;
};

struct _PicmanColormapEditorClass
{
  PicmanImageEditorClass  parent_class;
};


GType       picman_colormap_editor_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_colormap_editor_new       (PicmanMenuFactory    *menu_factory);

gint        picman_colormap_editor_get_index (PicmanColormapEditor *editor,
                                            const PicmanRGB      *search);
gboolean    picman_colormap_editor_set_index (PicmanColormapEditor *editor,
                                            gint                index,
                                            PicmanRGB            *color);

gint        picman_colormap_editor_max_index (PicmanColormapEditor *editor);


#endif /* __PICMAN_COLORMAP_EDITOR_H__ */
