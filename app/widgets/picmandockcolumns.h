/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockcolumns.h
 * Copyright (C) 2009 Martin Nordholts <martinn@src.gnome.org>
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

#ifndef __PICMAN_DOCK_COLUMNS_H__
#define __PICMAN_DOCK_COLUMNS_H__


#define PICMAN_TYPE_DOCK_COLUMNS            (picman_dock_columns_get_type ())
#define PICMAN_DOCK_COLUMNS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCK_COLUMNS, PicmanDockColumns))
#define PICMAN_DOCK_COLUMNS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCK_COLUMNS, PicmanDockColumnsClass))
#define PICMAN_IS_DOCK_COLUMNS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCK_COLUMNS))
#define PICMAN_IS_DOCK_COLUMNS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCK_COLUMNS))
#define PICMAN_DOCK_COLUMNS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCK_COLUMNS, PicmanDockColumnsClass))


typedef struct _PicmanDockColumnsClass    PicmanDockColumnsClass;
typedef struct _PicmanDockColumnsPrivate  PicmanDockColumnsPrivate;

/**
 * PicmanDockColumns:
 *
 * A widget containing PicmanDocks so that dockables are arranged in
 * columns.
 */
struct _PicmanDockColumns
{
  GtkBox parent_instance;

  PicmanDockColumnsPrivate *p;
};

struct _PicmanDockColumnsClass
{
  GtkBoxClass parent_class;

  void (* dock_added)   (PicmanDockColumns *dock_columns,
                         PicmanDock        *dock);
  void (* dock_removed) (PicmanDockColumns *dock_columns,
                         PicmanDock        *dock);
};


GType               picman_dock_columns_get_type           (void) G_GNUC_CONST;
GtkWidget         * picman_dock_columns_new                (PicmanContext       *context,
                                                          PicmanDialogFactory *dialog_factory,
                                                          PicmanUIManager     *ui_manager);
void                picman_dock_columns_add_dock           (PicmanDockColumns   *dock_columns,
                                                          PicmanDock          *dock,
                                                          gint               index);
void                picman_dock_columns_prepare_dockbook   (PicmanDockColumns   *dock_columns,
                                                          gint               dock_index,
                                                          GtkWidget        **dockbook_p);
void                picman_dock_columns_remove_dock        (PicmanDockColumns   *dock_columns,
                                                          PicmanDock          *dock);
GList             * picman_dock_columns_get_docks          (PicmanDockColumns   *dock_columns);
PicmanContext       * picman_dock_columns_get_context        (PicmanDockColumns   *dock_columns);
void                picman_dock_columns_set_context        (PicmanDockColumns   *dock_columns,
                                                          PicmanContext       *context);
PicmanDialogFactory * picman_dock_columns_get_dialog_factory (PicmanDockColumns   *dock_columns);
PicmanUIManager     * picman_dock_columns_get_ui_manager     (PicmanDockColumns   *dock_columns);


#endif /* __PICMAN_DOCK_COLUMNS_H__ */
