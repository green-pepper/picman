/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockbook.h
 * Copyright (C) 2001-2007 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DOCKBOOK_H__
#define __PICMAN_DOCKBOOK_H__


#define PICMAN_TYPE_DOCKBOOK            (picman_dockbook_get_type ())
#define PICMAN_DOCKBOOK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DOCKBOOK, PicmanDockbook))
#define PICMAN_DOCKBOOK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DOCKBOOK, PicmanDockbookClass))
#define PICMAN_IS_DOCKBOOK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DOCKBOOK))
#define PICMAN_IS_DOCKBOOK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DOCKBOOK))
#define PICMAN_DOCKBOOK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DOCKBOOK, PicmanDockbookClass))


typedef struct _PicmanDockbookClass    PicmanDockbookClass;
typedef struct _PicmanDockbookPrivate  PicmanDockbookPrivate;

/**
 * PicmanDockbook:
 *
 * Holds PicmanDockables which are presented on different tabs using
 * GtkNotebook.
 */
struct _PicmanDockbook
{
  GtkNotebook parent_instance;

  PicmanDockbookPrivate *p;
};

struct _PicmanDockbookClass
{
  GtkNotebookClass parent_class;

  void (* dockable_added)     (PicmanDockbook *dockbook,
                               PicmanDockable *dockable);
  void (* dockable_removed)   (PicmanDockbook *dockbook,
                               PicmanDockable *dockable);
  void (* dockable_reordered) (PicmanDockbook *dockbook,
                               PicmanDockable *dockable);
};


GType           picman_dockbook_get_type                  (void) G_GNUC_CONST;
GtkWidget     * picman_dockbook_new                       (PicmanMenuFactory *menu_factory);
PicmanDock      * picman_dockbook_get_dock                  (PicmanDockbook    *dockbook);
void            picman_dockbook_set_dock                  (PicmanDockbook    *dockbook,
                                                         PicmanDock        *dock);
PicmanUIManager * picman_dockbook_get_ui_manager            (PicmanDockbook    *dockbook);
void            picman_dockbook_add                       (PicmanDockbook    *dockbook,
                                                         PicmanDockable    *dockable,
                                                         gint             position);
GtkWidget     * picman_dockbook_add_from_dialog_factory   (PicmanDockbook    *dockbook,
                                                         const gchar     *identifiers,
                                                         gint             position);
void            picman_dockbook_remove                    (PicmanDockbook    *dockbook,
                                                         PicmanDockable    *dockable);
void            picman_dockbook_update_with_context       (PicmanDockbook    *dockbook,
                                                         PicmanContext     *context);
GtkWidget    *  picman_dockbook_create_tab_widget         (PicmanDockbook    *dockbook,
                                                         PicmanDockable    *dockable);
void            picman_dockbook_update_auto_tab_style     (PicmanDockbook    *dockbook);
gboolean        picman_dockbook_drop_dockable             (PicmanDockbook    *dockbook,
                                                         GtkWidget       *drag_source);
void            picman_dockbook_set_drag_handler          (PicmanDockbook    *dockbook,
                                                         PicmanPanedBox    *drag_handler);
PicmanDockable *  picman_dockbook_drag_source_to_dockable   (GtkWidget       *drag_source);


#endif /* __PICMAN_DOCKBOOK_H__ */
