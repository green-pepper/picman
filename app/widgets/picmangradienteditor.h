/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Gradient editor module copyight (C) 1996-1997 Federico Mena Quintero
 * federico@nuclecu.unam.mx
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

#ifndef __PICMAN_GRADIENT_EDITOR_H__
#define __PICMAN_GRADIENT_EDITOR_H__


#include "picmandataeditor.h"


#define GRAD_NUM_COLORS 10


typedef enum
{
  GRAD_DRAG_NONE = 0,
  GRAD_DRAG_LEFT,
  GRAD_DRAG_MIDDLE,
  GRAD_DRAG_ALL
} GradientEditorDragMode;


#define PICMAN_TYPE_GRADIENT_EDITOR            (picman_gradient_editor_get_type ())
#define PICMAN_GRADIENT_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_GRADIENT_EDITOR, PicmanGradientEditor))
#define PICMAN_GRADIENT_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_GRADIENT_EDITOR, PicmanGradientEditorClass))
#define PICMAN_IS_GRADIENT_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_GRADIENT_EDITOR))
#define PICMAN_IS_GRADIENT_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_GRADIENT_EDITOR))
#define PICMAN_GRADIENT_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_GRADIENT_EDITOR, PicmanGradientEditorClass))


typedef struct _PicmanGradientEditorClass PicmanGradientEditorClass;

struct _PicmanGradientEditor
{
  PicmanDataEditor  parent_instance;

  GtkWidget      *current_color;
  GtkWidget      *hint_label1;
  GtkWidget      *hint_label2;
  GtkWidget      *hint_label3;
  GtkWidget      *hint_label4;
  GtkWidget      *scrollbar;
  GtkWidget      *control;

  /*  Zoom and scrollbar  */
  guint           zoom_factor;
  GtkAdjustment  *scroll_data;

  /*  Color dialog  */
  GtkWidget      *color_dialog;

  /*  Gradient view  */
  gint            view_last_x;
  gboolean        view_button_down;

  /*  Gradient control  */
  PicmanGradientSegment    *control_drag_segment; /* Segment which is being dragged */
  PicmanGradientSegment    *control_sel_l;        /* Left segment of selection */
  PicmanGradientSegment    *control_sel_r;        /* Right segment of selection */
  GradientEditorDragMode  control_drag_mode;    /* What is being dragged? */
  guint32                 control_click_time;   /* Time when mouse was pressed */
  gboolean                control_compress;     /* Compressing/expanding handles */
  gint                    control_last_x;       /* Last mouse position when dragging */
  gdouble                 control_last_gx;      /* Last position (wrt gradient) when dragging */
  gdouble                 control_orig_pos;     /* Original click position when dragging */

  /*  Split uniformly dialog  */
  gint          split_parts;

  /*  Replicate dialog  */
  gint          replicate_times;

  /*  Saved colors  */
  PicmanRGB       saved_colors[GRAD_NUM_COLORS];

  /*  Color dialogs  */
  PicmanGradientSegment *left_saved_segments;
  gboolean             left_saved_dirty;

  PicmanGradientSegment *right_saved_segments;
  gboolean             right_saved_dirty;
};

struct _PicmanGradientEditorClass
{
  PicmanDataEditorClass  parent_class;
};


GType       picman_gradient_editor_get_type (void) G_GNUC_CONST;

GtkWidget * picman_gradient_editor_new      (PicmanContext        *context,
                                           PicmanMenuFactory    *menu_factory);

void        picman_gradient_editor_update   (PicmanGradientEditor *editor);
void        picman_gradient_editor_zoom     (PicmanGradientEditor *editor,
                                           PicmanZoomType        zoom_type);


#endif  /* __PICMAN_GRADIENT_EDITOR_H__ */
