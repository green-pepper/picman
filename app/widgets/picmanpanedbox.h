/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanpanedbox.h
 * Copyright (C) 2001-2005 Michael Natterer <mitch@picman.org>
 * Copyright (C)      2009 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef __PICMAN_PANED_BOX_H__
#define __PICMAN_PANED_BOX_H__


#define PICMAN_TYPE_PANED_BOX            (picman_paned_box_get_type ())
#define PICMAN_PANED_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_PANED_BOX, PicmanPanedBox))
#define PICMAN_PANED_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_PANED_BOX, PicmanPanedBoxClass))
#define PICMAN_IS_PANED_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_PANED_BOX))
#define PICMAN_IS_PANED_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_PANED_BOX))
#define PICMAN_PANED_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_PANED_BOX, PicmanPanedBoxClass))


typedef struct _PicmanPanedBoxClass    PicmanPanedBoxClass;
typedef struct _PicmanPanedBoxPrivate  PicmanPanedBoxPrivate;

/**
 * PicmanPanedBox:
 *
 * A #GtkBox with the children separated by #GtkPaned:s and basic
 * docking mechanisms.
 */
struct _PicmanPanedBox
{
  GtkBox parent_instance;

  PicmanPanedBoxPrivate *p;
};

struct _PicmanPanedBoxClass
{
  GtkBoxClass parent_class;
};


GType               picman_paned_box_get_type              (void) G_GNUC_CONST;
GtkWidget         * picman_paned_box_new                   (gboolean                 homogeneous,
                                                          gint                     spacing,
                                                          GtkOrientation           orientation);
void                picman_paned_box_set_dropped_cb        (PicmanPanedBox            *paned_box,
                                                          PicmanPanedBoxDroppedFunc  dropped_cb,
                                                          gpointer                 dropped_cb_data);
void                picman_paned_box_add_widget            (PicmanPanedBox            *paned_box,
                                                          GtkWidget               *widget,
                                                          gint                     index);
void                picman_paned_box_remove_widget         (PicmanPanedBox            *paned_box,
                                                          GtkWidget               *widget);
gboolean            picman_paned_box_will_handle_drag      (PicmanPanedBox            *paned_box,
                                                          GtkWidget               *widget,
                                                          GdkDragContext          *context,
                                                          gint                     x,
                                                          gint                     y,
                                                          gint                     time);
void                picman_paned_box_set_drag_handler      (PicmanPanedBox            *paned_box,
                                                          PicmanPanedBox            *drag_handler);


#endif /* __PICMAN_PANED_BOX_H__ */
