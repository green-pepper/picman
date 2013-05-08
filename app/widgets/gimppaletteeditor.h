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

#ifndef __PICMAN_PALETTE_EDITOR_H__
#define __PICMAN_PALETTE_EDITOR_H__


#include "picmandataeditor.h"


#define PICMAN_TYPE_PALETTE_EDITOR            (picman_palette_editor_get_type ())
#define PICMAN_PALETTE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PALETTE_EDITOR, PicmanPaletteEditor))
#define PICMAN_PALETTE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PALETTE_EDITOR, PicmanPaletteEditorClass))
#define PICMAN_IS_PALETTE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PALETTE_EDITOR))
#define PICMAN_IS_PALETTE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PALETTE_EDITOR))
#define PICMAN_PALETTE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PALETTE_EDITOR, PicmanPaletteEditorClass))


typedef struct _PicmanPaletteEditorClass PicmanPaletteEditorClass;

struct _PicmanPaletteEditor
{
  PicmanDataEditor    parent_instance;

  GtkWidget        *view;

  GtkWidget        *color_name;
  GtkAdjustment    *columns_data;

  GtkWidget        *color_dialog;

  PicmanPaletteEntry *color;

  gfloat            zoom_factor;  /* range from 0.1 to 4.0 */
  gint              col_width;
  gint              last_width;
  gint              columns;
};

struct _PicmanPaletteEditorClass
{
  PicmanDataEditorClass  parent_class;
};


GType       picman_palette_editor_get_type   (void) G_GNUC_CONST;

GtkWidget * picman_palette_editor_new        (PicmanContext        *context,
                                            PicmanMenuFactory    *menu_factory);

void        picman_palette_editor_pick_color (PicmanPaletteEditor  *editor,
                                            const PicmanRGB      *color,
                                            PicmanColorPickState  pick_state);
void        picman_palette_editor_zoom       (PicmanPaletteEditor  *editor,
                                            PicmanZoomType        zoom_type);

gint        picman_palette_editor_get_index  (PicmanPaletteEditor *editor,
                                            const PicmanRGB     *search);
gboolean    picman_palette_editor_set_index  (PicmanPaletteEditor *editor,
                                            gint               index,
                                            PicmanRGB           *color);

gint        picman_palette_editor_max_index  (PicmanPaletteEditor *editor);


#endif /* __PICMAN_PALETTE_EDITOR_H__ */
