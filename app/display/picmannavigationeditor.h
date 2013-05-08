/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmannavigationeditor.h
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * partly based on app/nav_window
 * Copyright (C) 1999 Andy Thomas <alt@picman.org>
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

#ifndef __PICMAN_NAVIGATION_EDITOR_H__
#define __PICMAN_NAVIGATION_EDITOR_H__


#include "widgets/picmaneditor.h"


#define PICMAN_TYPE_NAVIGATION_EDITOR            (picman_navigation_editor_get_type ())
#define PICMAN_NAVIGATION_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_NAVIGATION_EDITOR, PicmanNavigationEditor))
#define PICMAN_NAVIGATION_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_NAVIGATION_EDITOR, PicmanNavigationEditorClass))
#define PICMAN_IS_NAVIGATION_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_NAVIGATION_EDITOR))
#define PICMAN_IS_NAVIGATION_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_NAVIGATION_EDITOR))
#define PICMAN_NAVIGATION_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_NAVIGATION_EDITOR, PicmanNavigationEditorClass))


typedef struct _PicmanNavigationEditorClass  PicmanNavigationEditorClass;

struct _PicmanNavigationEditor
{
  PicmanEditor        parent_instance;

  PicmanContext      *context;
  PicmanDisplayShell *shell;

  GtkWidget        *view;
  GtkWidget        *zoom_label;
  GtkAdjustment    *zoom_adjustment;

  GtkWidget        *zoom_out_button;
  GtkWidget        *zoom_in_button;
  GtkWidget        *zoom_100_button;
  GtkWidget        *zoom_fit_in_button;
  GtkWidget        *zoom_fill_button;
  GtkWidget        *shrink_wrap_button;

  guint             scale_timeout;
};

struct _PicmanNavigationEditorClass
{
  PicmanEditorClass  parent_class;
};


GType       picman_navigation_editor_get_type  (void) G_GNUC_CONST;

GtkWidget * picman_navigation_editor_new       (PicmanMenuFactory  *menu_factory);
void        picman_navigation_editor_popup     (PicmanDisplayShell *shell,
                                              GtkWidget        *widget,
                                              gint              click_x,
                                              gint              click_y);


#endif  /*  __PICMAN_NAVIGATION_EDITOR_H__  */
