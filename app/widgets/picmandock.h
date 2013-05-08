/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandock.h
 * Copyright (C) 2001-2005 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DOCK_H__
#define __PICMAN_DOCK_H__


#define PICMAN_TYPE_DOCK            (picman_dock_get_type ())
#define PICMAN_DOCK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCK, PicmanDock))
#define PICMAN_DOCK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCK, PicmanDockClass))
#define PICMAN_IS_DOCK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCK))
#define PICMAN_IS_DOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCK))
#define PICMAN_DOCK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCK, PicmanDockClass))


/* String used to separate dockables, e.g. "Tool Options, Layers" */
#define PICMAN_DOCK_DOCKABLE_SEPARATOR C_("dock", ", ")

/* String used to separate books (GtkNotebooks) within a dock,
   e.g. "Tool Options, Layers - Brushes"
 */
#define PICMAN_DOCK_BOOK_SEPARATOR C_("dock", " - ")

/* String used to separate dock columns,
   e.g. "Tool Options, Layers - Brushes | Gradients"
 */
#define PICMAN_DOCK_COLUMN_SEPARATOR C_("dock", " | ")


typedef struct _PicmanDockClass    PicmanDockClass;
typedef struct _PicmanDockPrivate  PicmanDockPrivate;

/**
 * PicmanDock:
 *
 * Contains a column of PicmanDockbooks.
 */
struct _PicmanDock
{
  GtkBox           parent_instance;

  PicmanDockPrivate *p;
};

struct _PicmanDockClass
{
  GtkBoxClass  parent_class;

  /*  virtual functions  */
  gchar * (* get_description)         (PicmanDock       *dock,
                                       gboolean        complete);
  void    (* set_host_geometry_hints) (PicmanDock       *dock,
                                       GtkWindow      *window);

  /*  signals  */
  void    (* book_added)              (PicmanDock       *dock,
                                       PicmanDockbook   *dockbook);
  void    (* book_removed)            (PicmanDock       *dock,
                                       PicmanDockbook   *dockbook);
  void    (* description_invalidated) (PicmanDock       *dock);
  void    (* geometry_invalidated)    (PicmanDock       *dock);
};


GType               picman_dock_get_type                (void) G_GNUC_CONST;

gchar             * picman_dock_get_description         (PicmanDock       *dock,
                                                       gboolean        complete);
void                picman_dock_set_host_geometry_hints (PicmanDock       *dock,
                                                       GtkWindow      *window);
void                picman_dock_invalidate_geometry     (PicmanDock       *dock);
void                picman_dock_update_with_context     (PicmanDock       *dock,
                                                       PicmanContext    *context);
PicmanContext       * picman_dock_get_context             (PicmanDock       *dock);
PicmanDialogFactory * picman_dock_get_dialog_factory      (PicmanDock       *dock);
PicmanUIManager     * picman_dock_get_ui_manager          (PicmanDock       *dock);
GList             * picman_dock_get_dockbooks           (PicmanDock       *dock);
gint                picman_dock_get_n_dockables         (PicmanDock       *dock);
GtkWidget         * picman_dock_get_main_vbox           (PicmanDock       *dock);
GtkWidget         * picman_dock_get_vbox                (PicmanDock       *dock);
gint                picman_dock_get_id                  (PicmanDock       *dock);
void                picman_dock_set_id                  (PicmanDock       *dock,
                                                       gint            ID);

void                picman_dock_add                     (PicmanDock       *dock,
                                                       PicmanDockable   *dockable,
                                                       gint            book,
                                                       gint            index);
void                picman_dock_remove                  (PicmanDock       *dock,
                                                       PicmanDockable   *dockable);

void                picman_dock_add_book                (PicmanDock       *dock,
                                                       PicmanDockbook   *dockbook,
                                                       gint            index);
void                picman_dock_remove_book             (PicmanDock       *dock,
                                                       PicmanDockbook   *dockbook);
void                picman_dock_temp_add                (PicmanDock       *dock,
                                                       GtkWidget      *widget);
void                picman_dock_temp_remove             (PicmanDock       *dock,
                                                       GtkWidget      *widget);


#endif /* __PICMAN_DOCK_H__ */
