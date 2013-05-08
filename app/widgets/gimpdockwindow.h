/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockwindow.h
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

#ifndef __PICMAN_DOCK_WINDOW_H__
#define __PICMAN_DOCK_WINDOW_H__


#include "widgets/picmanwindow.h"


#define PICMAN_TYPE_DOCK_WINDOW            (picman_dock_window_get_type ())
#define PICMAN_DOCK_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCK_WINDOW, PicmanDockWindow))
#define PICMAN_DOCK_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCK_WINDOW, PicmanDockWindowClass))
#define PICMAN_IS_DOCK_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCK_WINDOW))
#define PICMAN_IS_DOCK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCK_WINDOW))
#define PICMAN_DOCK_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCK_WINDOW, PicmanDockWindowClass))


typedef struct _PicmanDockWindowClass    PicmanDockWindowClass;
typedef struct _PicmanDockWindowPrivate  PicmanDockWindowPrivate;

/**
 * PicmanDockWindow:
 *
 * A top-level window containing PicmanDocks.
 */
struct _PicmanDockWindow
{
  PicmanWindow  parent_instance;

  PicmanDockWindowPrivate *p;
};

struct _PicmanDockWindowClass
{
  PicmanWindowClass  parent_class;
};


GType               picman_dock_window_get_type               (void) G_GNUC_CONST;
GtkWidget         * picman_dock_window_new                    (const gchar       *role,
                                                             const gchar       *ui_manager_name,
                                                             gboolean           allow_dockbook_absence,
                                                             PicmanDialogFactory *factory,
                                                             PicmanContext       *context);
gint                picman_dock_window_get_id                 (PicmanDockWindow    *dock_window);
void                picman_dock_window_add_dock               (PicmanDockWindow    *dock_window,
                                                             PicmanDock          *dock,
                                                             gint               index);
void                picman_dock_window_remove_dock            (PicmanDockWindow    *dock_window,
                                                             PicmanDock          *dock);
PicmanContext       * picman_dock_window_get_context            (PicmanDockWindow    *dock);
PicmanDialogFactory * picman_dock_window_get_dialog_factory     (PicmanDockWindow    *dock);
gboolean            picman_dock_window_get_auto_follow_active (PicmanDockWindow    *menu_dock);
void                picman_dock_window_set_auto_follow_active (PicmanDockWindow    *menu_dock,
                                                             gboolean           show);
gboolean            picman_dock_window_get_show_image_menu    (PicmanDockWindow    *menu_dock);
void                picman_dock_window_set_show_image_menu    (PicmanDockWindow    *menu_dock,
                                                             gboolean           show);
void                picman_dock_window_setup                  (PicmanDockWindow    *dock_window,
                                                             PicmanDockWindow    *template);
gboolean            picman_dock_window_has_toolbox            (PicmanDockWindow    *dock_window);

PicmanDockWindow    * picman_dock_window_from_dock              (PicmanDock          *dock);



#endif /* __PICMAN_DOCK_WINDOW_H__ */
