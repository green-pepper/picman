/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandialogfactory.h
 * Copyright (C) 2001 Michael Natterer <mitch@picman.org>
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

#ifndef __PICMAN_DIALOG_FACTORY_H__
#define __PICMAN_DIALOG_FACTORY_H__


#include "core/picmanobject.h"

#define PICMAN_DIALOG_VISIBILITY_KEY "picman-dialog-visibility"

typedef enum
{
  PICMAN_DIALOG_VISIBILITY_UNKNOWN = 0,
  PICMAN_DIALOG_VISIBILITY_INVISIBLE,
  PICMAN_DIALOG_VISIBILITY_VISIBLE,
  PICMAN_DIALOG_VISIBILITY_HIDDEN
} PicmanDialogVisibilityState;


/* In order to support constructors of various types, these functions
 * takes the union of the set of arguments required for each type of
 * widget constructor. If this set becomes too big we can consider
 * passing a struct or use varargs.
 */ 
typedef GtkWidget * (* PicmanDialogNewFunc)     (PicmanDialogFactory      *factory,
                                               PicmanContext            *context,
                                               PicmanUIManager          *ui_manager,
                                               gint                    view_size);


struct _PicmanDialogFactoryEntry
{
  gchar                *identifier;
  gchar                *name;
  gchar                *blurb;
  gchar                *stock_id;
  gchar                *help_id;

  PicmanDialogNewFunc     new_func;
  PicmanDialogRestoreFunc restore_func;
  gint                  view_size;

  gboolean              singleton;
  gboolean              session_managed;
  gboolean              remember_size;
  gboolean              remember_if_open;

  /* If TRUE the visibility of the dialog is toggleable */
  gboolean              hideable;

  /* If TRUE the entry is for a PicmanImageWindow, FALSE otherwise */
  gboolean              image_window;

  /* If TRUE the entry is for a dockable, FALSE otherwise */
  gboolean              dockable;
};


#define PICMAN_TYPE_DIALOG_FACTORY            (picman_dialog_factory_get_type ())
#define PICMAN_DIALOG_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PICMAN_TYPE_DIALOG_FACTORY, PicmanDialogFactory))
#define PICMAN_DIALOG_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PICMAN_TYPE_DIALOG_FACTORY, PicmanDialogFactoryClass))
#define PICMAN_IS_DIALOG_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PICMAN_TYPE_DIALOG_FACTORY))
#define PICMAN_IS_DIALOG_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PICMAN_TYPE_DIALOG_FACTORY))
#define PICMAN_DIALOG_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PICMAN_TYPE_DIALOG_FACTORY, PicmanDialogFactoryClass))


typedef struct _PicmanDialogFactoryPrivate  PicmanDialogFactoryPrivate;
typedef struct _PicmanDialogFactoryClass    PicmanDialogFactoryClass;

/**
 * PicmanDialogFactory:
 *
 * A factory with the main purpose of creating toplevel windows and
 * position them according to the session information kept within the
 * factory. Over time it has accumulated more functionality than this.
 */
struct _PicmanDialogFactory
{
  PicmanObject                parent_instance;

  PicmanDialogFactoryPrivate *p;
};

struct _PicmanDialogFactoryClass
{
  PicmanObjectClass  parent_class;

  void (* dock_window_added)   (PicmanDialogFactory *factory,
                                PicmanDockWindow    *dock_window);
  void (* dock_window_removed) (PicmanDialogFactory *factory,
                                PicmanDockWindow    *dock_window);
};


GType               picman_dialog_factory_get_type             (void) G_GNUC_CONST;
PicmanDialogFactory * picman_dialog_factory_new                  (const gchar             *name,
                                                              PicmanContext             *context,
                                                              PicmanMenuFactory         *menu_factory);
void                picman_dialog_factory_register_entry       (PicmanDialogFactory       *factory,
                                                              const gchar             *identifier,
                                                              const gchar             *name,
                                                              const gchar             *blurb,
                                                              const gchar             *stock_id,
                                                              const gchar             *help_id,
                                                              PicmanDialogNewFunc        new_func,
                                                              PicmanDialogRestoreFunc    restore_func,
                                                              gint                     view_size,
                                                              gboolean                 singleton,
                                                              gboolean                 session_managed,
                                                              gboolean                 remember_size,
                                                              gboolean                 remember_if_open,
                                                              gboolean                 hideable,
                                                              gboolean                 image_window,
                                                              gboolean                 dockable);
PicmanDialogFactoryEntry *
                    picman_dialog_factory_find_entry           (PicmanDialogFactory       *factory,
                                                              const gchar             *identifier);
PicmanSessionInfo *   picman_dialog_factory_find_session_info    (PicmanDialogFactory       *factory,
                                                              const gchar             *identifier);
GtkWidget *         picman_dialog_factory_find_widget          (PicmanDialogFactory       *factory,
                                                              const gchar             *identifiers);
GtkWidget *         picman_dialog_factory_dialog_new           (PicmanDialogFactory       *factory,
                                                              GdkScreen               *screen,
                                                              PicmanUIManager           *ui_manager,
                                                              const gchar             *identifier,
                                                              gint                     view_size,
                                                              gboolean                 present);
PicmanContext *       picman_dialog_factory_get_context          (PicmanDialogFactory       *factory);
PicmanMenuFactory *   picman_dialog_factory_get_menu_factory     (PicmanDialogFactory       *factory);
GList *             picman_dialog_factory_get_open_dialogs     (PicmanDialogFactory       *factory);
GList *             picman_dialog_factory_get_session_infos    (PicmanDialogFactory       *factory);
void                picman_dialog_factory_add_session_info     (PicmanDialogFactory       *factory,
                                                              PicmanSessionInfo         *info);
GtkWidget *         picman_dialog_factory_dialog_raise         (PicmanDialogFactory       *factory,
                                                              GdkScreen               *screen,
                                                              const gchar             *identifiers,
                                                              gint                     view_size);
GtkWidget *         picman_dialog_factory_dockable_new         (PicmanDialogFactory       *factory,
                                                              PicmanDock                *dock,
                                                              const gchar             *identifier,
                                                              gint                     view_size);
void                picman_dialog_factory_add_dialog           (PicmanDialogFactory       *factory,
                                                              GtkWidget               *dialog);
void                picman_dialog_factory_add_foreign          (PicmanDialogFactory       *factory,
                                                              const gchar             *identifier,
                                                              GtkWidget               *dialog);
void                picman_dialog_factory_remove_dialog        (PicmanDialogFactory       *factory,
                                                              GtkWidget               *dialog);
void                picman_dialog_factory_hide_dialog          (GtkWidget               *dialog);
void                picman_dialog_factory_save                 (PicmanDialogFactory       *factory,
                                                              PicmanConfigWriter        *writer);
void                picman_dialog_factory_restore              (PicmanDialogFactory       *factory);
void                picman_dialog_factory_set_state            (PicmanDialogFactory       *factory,
                                                              PicmanDialogsState         state);
PicmanDialogsState    picman_dialog_factory_get_state            (PicmanDialogFactory       *factory);
void                picman_dialog_factory_show_with_display    (PicmanDialogFactory       *factory);
void                picman_dialog_factory_hide_with_display    (PicmanDialogFactory       *factory);
void                picman_dialog_factory_set_busy             (PicmanDialogFactory       *factory);
void                picman_dialog_factory_unset_busy           (PicmanDialogFactory       *factory);
PicmanDialogFactory * picman_dialog_factory_from_widget          (GtkWidget               *dialog,
                                                              PicmanDialogFactoryEntry **entry);
void                picman_dialog_factory_set_has_min_size     (GtkWindow               *window,
                                                              gboolean                 has_min_size);
gboolean            picman_dialog_factory_get_has_min_size     (GtkWindow               *window);

PicmanDialogFactory * picman_dialog_factory_get_singleton        (void);
void                picman_dialog_factory_set_singleton        (PicmanDialogFactory       *factory);


#endif  /*  __PICMAN_DIALOG_FACTORY_H__  */
