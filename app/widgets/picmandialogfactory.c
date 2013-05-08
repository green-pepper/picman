/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandialogfactory.c
 * Copyright (C) 2001-2008 Michael Natterer <mitch@picman.org>
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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmancursor.h"
#include "picmandialogfactory.h"
#include "picmandock.h"
#include "picmandockbook.h"
#include "picmandockable.h"
#include "picmandockcontainer.h"
#include "picmandockwindow.h"
#include "picmanmenufactory.h"
#include "picmansessioninfo.h"
#include "picmanwidgets-utils.h"

#include "picman-log.h"


enum
{
  DOCK_WINDOW_ADDED,
  DOCK_WINDOW_REMOVED,
  LAST_SIGNAL
};


struct _PicmanDialogFactoryPrivate
{
  PicmanContext      *context;
  PicmanMenuFactory  *menu_factory;

  GList            *open_dialogs;
  GList            *session_infos;

  GList            *registered_dialogs;

  PicmanDialogsState  dialog_state;
};


static void        picman_dialog_factory_dispose              (GObject                *object);
static void        picman_dialog_factory_finalize             (GObject                *object);
static GtkWidget * picman_dialog_factory_constructor          (PicmanDialogFactory      *factory,
                                                             PicmanDialogFactoryEntry *entry,
                                                             PicmanContext            *context,
                                                             PicmanUIManager          *ui_manager,
                                                             gint                    view_size);
static void        picman_dialog_factory_config_notify        (PicmanDialogFactory      *factory,
                                                             GParamSpec             *pspec,
                                                             PicmanGuiConfig          *config);
static void        picman_dialog_factory_set_widget_data      (GtkWidget              *dialog,
                                                             PicmanDialogFactory      *factory,
                                                             PicmanDialogFactoryEntry *entry);
static void        picman_dialog_factory_unset_widget_data    (GtkWidget              *dialog);
static gboolean    picman_dialog_factory_set_user_pos         (GtkWidget              *dialog,
                                                             GdkEventConfigure      *cevent,
                                                             gpointer                data);
static gboolean    picman_dialog_factory_dialog_configure     (GtkWidget              *dialog,
                                                             GdkEventConfigure      *cevent,
                                                             PicmanDialogFactory      *factory);
static void        picman_dialog_factory_hide                 (PicmanDialogFactory      *factory);
static void        picman_dialog_factory_show                 (PicmanDialogFactory      *factory);


G_DEFINE_TYPE (PicmanDialogFactory, picman_dialog_factory, PICMAN_TYPE_OBJECT)

#define parent_class picman_dialog_factory_parent_class

static guint factory_signals[LAST_SIGNAL] = { 0 };


/* Is set by dialogs.c to a dialog factory initialized there.
 *
 * FIXME: The layer above should not do this kind of initialization of
 * layers below.
 */
static PicmanDialogFactory *picman_toplevel_factory = NULL;


static void
picman_dialog_factory_class_init (PicmanDialogFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose  = picman_dialog_factory_dispose;
  object_class->finalize = picman_dialog_factory_finalize;

  factory_signals[DOCK_WINDOW_ADDED] =
    g_signal_new ("dock-window-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanDialogFactoryClass, dock_window_added),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DOCK_WINDOW);

  factory_signals[DOCK_WINDOW_REMOVED] =
    g_signal_new ("dock-window-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanDialogFactoryClass, dock_window_removed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DOCK_WINDOW);

  g_type_class_add_private (klass, sizeof (PicmanDialogFactoryPrivate));
}

static void
picman_dialog_factory_init (PicmanDialogFactory *factory)
{
  factory->p = G_TYPE_INSTANCE_GET_PRIVATE (factory,
                                            PICMAN_TYPE_DIALOG_FACTORY,
                                            PicmanDialogFactoryPrivate);
  factory->p->dialog_state = PICMAN_DIALOGS_SHOWN;
}

static void
picman_dialog_factory_dispose (GObject *object)
{
  PicmanDialogFactory *factory = PICMAN_DIALOG_FACTORY (object);
  GList             *list;

  /*  start iterating from the beginning each time we destroyed a
   *  toplevel because destroying a dock may cause lots of items
   *  to be removed from factory->p->open_dialogs
   */
  while (factory->p->open_dialogs)
    {
      for (list = factory->p->open_dialogs; list; list = g_list_next (list))
        {
          if (gtk_widget_is_toplevel (list->data))
            {
              gtk_widget_destroy (GTK_WIDGET (list->data));
              break;
            }
        }

      /*  the list being non-empty without any toplevel is an error,
       *  so eek and chain up
       */
      if (! list)
        {
          g_warning ("%s: %d stale non-toplevel entries in factory->p->open_dialogs",
                     G_STRFUNC, g_list_length (factory->p->open_dialogs));
          break;
        }
    }

  if (factory->p->open_dialogs)
    {
      g_list_free (factory->p->open_dialogs);
      factory->p->open_dialogs = NULL;
    }

  if (factory->p->session_infos)
    {
      g_list_free_full (factory->p->session_infos,
                        (GDestroyNotify) g_object_unref);
      factory->p->session_infos = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_dialog_factory_finalize (GObject *object)
{
  PicmanDialogFactory *factory = PICMAN_DIALOG_FACTORY (object);
  GList             *list;

  for (list = factory->p->registered_dialogs; list; list = g_list_next (list))
    {
      PicmanDialogFactoryEntry *entry = list->data;

      g_free (entry->identifier);
      g_free (entry->name);
      g_free (entry->blurb);
      g_free (entry->stock_id);
      g_free (entry->help_id);

      g_slice_free (PicmanDialogFactoryEntry, entry);
    }

  if (factory->p->registered_dialogs)
    {
      g_list_free (factory->p->registered_dialogs);
      factory->p->registered_dialogs = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

PicmanDialogFactory *
picman_dialog_factory_new (const gchar           *name,
                         PicmanContext           *context,
                         PicmanMenuFactory       *menu_factory)
{
  PicmanDialogFactory *factory;
  PicmanGuiConfig     *config;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (! menu_factory || PICMAN_IS_MENU_FACTORY (menu_factory),
                        NULL);

  factory = g_object_new (PICMAN_TYPE_DIALOG_FACTORY, NULL);

  picman_object_set_name (PICMAN_OBJECT (factory), name);

  config = PICMAN_GUI_CONFIG (context->picman->config);

  factory->p->context      = context;
  factory->p->menu_factory = menu_factory;
  factory->p->dialog_state = (config->hide_docks ?
                              PICMAN_DIALOGS_HIDDEN_EXPLICITLY :
                              PICMAN_DIALOGS_SHOWN);

  g_signal_connect_object (config, "notify::hide-docks",
                           G_CALLBACK (picman_dialog_factory_config_notify),
                           factory, G_CONNECT_SWAPPED);

  return factory;
}

void
picman_dialog_factory_register_entry (PicmanDialogFactory    *factory,
                                    const gchar          *identifier,
                                    const gchar          *name,
                                    const gchar          *blurb,
                                    const gchar          *stock_id,
                                    const gchar          *help_id,
                                    PicmanDialogNewFunc     new_func,
                                    PicmanDialogRestoreFunc restore_func,
                                    gint                  view_size,
                                    gboolean              singleton,
                                    gboolean              session_managed,
                                    gboolean              remember_size,
                                    gboolean              remember_if_open,
                                    gboolean              hideable,
                                    gboolean              image_window,
                                    gboolean              dockable)
{
  PicmanDialogFactoryEntry *entry;

  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));
  g_return_if_fail (identifier != NULL);

  entry = g_slice_new0 (PicmanDialogFactoryEntry);

  entry->identifier       = g_strdup (identifier);
  entry->name             = g_strdup (name);
  entry->blurb            = g_strdup (blurb);
  entry->stock_id         = g_strdup (stock_id);
  entry->help_id          = g_strdup (help_id);
  entry->new_func         = new_func;
  entry->restore_func     = restore_func;
  entry->view_size        = view_size;
  entry->singleton        = singleton ? TRUE : FALSE;
  entry->session_managed  = session_managed ? TRUE : FALSE;
  entry->remember_size    = remember_size ? TRUE : FALSE;
  entry->remember_if_open = remember_if_open ? TRUE : FALSE;
  entry->hideable         = hideable ? TRUE : FALSE;
  entry->image_window     = image_window ? TRUE : FALSE;
  entry->dockable         = dockable ? TRUE : FALSE;

  factory->p->registered_dialogs = g_list_prepend (factory->p->registered_dialogs,
                                                   entry);
}

PicmanDialogFactoryEntry *
picman_dialog_factory_find_entry (PicmanDialogFactory *factory,
                                const gchar       *identifier)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  for (list = factory->p->registered_dialogs; list; list = g_list_next (list))
    {
      PicmanDialogFactoryEntry *entry = list->data;

      if (! strcmp (identifier, entry->identifier))
        return entry;
    }

  return NULL;
}

PicmanSessionInfo *
picman_dialog_factory_find_session_info (PicmanDialogFactory *factory,
                                       const gchar       *identifier)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  for (list = factory->p->session_infos; list; list = g_list_next (list))
    {
      PicmanSessionInfo *info = list->data;

      if (picman_session_info_get_factory_entry (info) &&
          g_str_equal (identifier,
                       picman_session_info_get_factory_entry (info)->identifier))
        {
          return info;
        }
    }

  return NULL;
}

GtkWidget *
picman_dialog_factory_find_widget (PicmanDialogFactory *factory,
                                 const gchar       *identifiers)
{
  GtkWidget  *widget = NULL;
  gchar     **ids;
  gint        i;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (identifiers != NULL, NULL);

  ids = g_strsplit (identifiers, "|", 0);

  for (i = 0; ids[i]; i++)
    {
      PicmanSessionInfo *info;

      info = picman_dialog_factory_find_session_info (factory, ids[i]);

      if (info)
        {
          widget =  picman_session_info_get_widget (info);

          if (widget)
            break;
        }
    }

  g_strfreev (ids);

  return widget;
}

/**
 * picman_dialog_factory_dialog_sane:
 * @factory:
 * @widget_factory:
 * @widget_entry:
 * @widget:
 *
 * Makes sure that the @widget with the given @widget_entry that was
 * created by the given @widget_factory belongs to @efactory.
 *
 * Returns: %TRUE if that is the case, %FALSE otherwise.
 **/
static gboolean
picman_dialog_factory_dialog_sane (PicmanDialogFactory      *factory,
                                 PicmanDialogFactory      *widget_factory,
                                 PicmanDialogFactoryEntry *widget_entry,
                                 GtkWidget              *widget)
{
  if (! widget_factory || ! widget_entry)
    {
      g_warning ("%s: dialog was not created by a PicmanDialogFactory",
                 G_STRFUNC);
      return FALSE;
    }

  if (widget_factory != factory)
    {
      g_warning ("%s: dialog was created by a different PicmanDialogFactory",
                 G_STRFUNC);
      return FALSE;
    }

  return TRUE;
}

/**
 * picman_dialog_factory_dialog_new_internal:
 * @factory:
 * @screen:
 * @context:
 * @ui_manager:
 * @identifier:
 * @view_size:
 * @return_existing:   If %TRUE, (or if the dialog is a singleton),
 *                     don't create a new dialog if it exists, instead
 *                     return the existing one
 * @present:           If %TRUE, the toplevel that contains the dialog (if any)
 *                     will be gtk_window_present():ed
 * @create_containers: If %TRUE, then containers for the
 *                     dialog/dockable will be created as well. If you
 *                     want to manage your own containers, pass %FALSE
 *
 * This is the lowest level dialog factory creation function.
 *
 * Returns: A created or existing #GtkWidget.
 **/
static GtkWidget *
picman_dialog_factory_dialog_new_internal (PicmanDialogFactory *factory,
                                         GdkScreen         *screen,
                                         PicmanContext       *context,
                                         PicmanUIManager     *ui_manager,
                                         const gchar       *identifier,
                                         gint               view_size,
                                         gboolean           return_existing,
                                         gboolean           present,
                                         gboolean           create_containers)
{
  PicmanDialogFactoryEntry *entry    = NULL;
  GtkWidget              *dialog   = NULL;
  GtkWidget              *toplevel = NULL;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  entry = picman_dialog_factory_find_entry (factory, identifier);

  if (! entry)
    {
      g_warning ("%s: no entry registered for \"%s\"",
                 G_STRFUNC, identifier);
      return NULL;
    }

  if (! entry->new_func)
    {
      g_warning ("%s: entry for \"%s\" has no constructor",
                 G_STRFUNC, identifier);
      return NULL;
    }

  /*  a singleton dialog is always returned if it already exisits  */
  if (return_existing || entry->singleton)
    {
      dialog = picman_dialog_factory_find_widget (factory, identifier);
    }

  /*  create the dialog if it was not found  */
  if (! dialog)
    {
      GtkWidget *dock              = NULL;
      GtkWidget *dock_window       = NULL;

      /* What follows is special-case code for some entires. At some
       * point we might want to abstract this block of code away.
       */
      if (create_containers)
        {
          if (entry->dockable)
            {
              GtkWidget *dockbook;

              /*  It doesn't make sense to have a dockable without a dock
               *  so create one. Create a new dock _before_ creating the
               *  dialog. We do this because the new dockable needs to be
               *  created in its dock's context.
               */
              dock     = picman_dock_with_window_new (factory,
                                                    screen,
                                                    FALSE /*toolbox*/);
              dockbook = picman_dockbook_new (factory->p->menu_factory);

              picman_dock_add_book (PICMAN_DOCK (dock),
                                  PICMAN_DOCKBOOK (dockbook),
                                  0);
            }
          else if (strcmp ("picman-toolbox", entry->identifier) == 0)
            {
              PicmanDockContainer *dock_container;

              dock_window = picman_dialog_factory_dialog_new (factory,
                                                            screen,
                                                            NULL /*ui_manager*/,
                                                            "picman-toolbox-window",
                                                            -1 /*view_size*/,
                                                            FALSE /*present*/);

              /* When we get a dock window, we also get a UI
               * manager
               */
              dock_container = PICMAN_DOCK_CONTAINER (dock_window);
              ui_manager     = picman_dock_container_get_ui_manager (dock_container);
            }
        }

      /*  Create the new dialog in the appropriate context which is
       *  - the passed context if not NULL
       *  - the newly created dock's context if we just created it
       *  - the factory's context, which happens when raising a toplevel
       *    dialog was the original request.
       */
      if (view_size < PICMAN_VIEW_SIZE_TINY)
        view_size = entry->view_size;

      if (context)
        dialog = picman_dialog_factory_constructor (factory, entry,
                                                  context,
                                                  ui_manager,
                                                  view_size);
      else if (dock)
        dialog = picman_dialog_factory_constructor (factory, entry,
                                                  picman_dock_get_context (PICMAN_DOCK (dock)),
                                                  picman_dock_get_ui_manager (PICMAN_DOCK (dock)),
                                                  view_size);
      else
        dialog = picman_dialog_factory_constructor (factory, entry,
                                                  factory->p->context,
                                                  ui_manager,
                                                  view_size);

      if (dialog)
        {
          picman_dialog_factory_set_widget_data (dialog, factory, entry);

          /*  If we created a dock before, the newly created dialog is
           *  supposed to be a PicmanDockable.
           */
          if (dock)
            {
              if (PICMAN_IS_DOCKABLE (dialog))
                {
                  picman_dock_add (PICMAN_DOCK (dock), PICMAN_DOCKABLE (dialog),
                                 0, 0);

                  gtk_widget_show (dock);
                }
              else
                {
                  g_warning ("%s: PicmanDialogFactory is a dockable factory "
                             "but constructor for \"%s\" did not return a "
                             "PicmanDockable",
                             G_STRFUNC, identifier);

                  gtk_widget_destroy (dialog);
                  gtk_widget_destroy (dock);

                  dialog = NULL;
                  dock   = NULL;
                }
            }
          else if (dock_window)
            {
              if (PICMAN_IS_DOCK (dialog))
                {
                  picman_dock_window_add_dock (PICMAN_DOCK_WINDOW (dock_window),
                                             PICMAN_DOCK (dialog),
                                             -1 /*index*/);

                  gtk_widget_set_visible (dialog, present);
                  gtk_widget_set_visible (dock_window, present);
                }
              else
                {
                  g_warning ("%s: PicmanDialogFactory is a dock factory entry "
                             "but constructor for \"%s\" did not return a "
                             "PicmanDock",
                             G_STRFUNC, identifier);

                  gtk_widget_destroy (dialog);
                  gtk_widget_destroy (dock_window);

                  dialog      = NULL;
                  dock_window = NULL;
                }
            }
        }
      else if (dock)
        {
          g_warning ("%s: constructor for \"%s\" returned NULL",
                     G_STRFUNC, identifier);

          gtk_widget_destroy (dock);

          dock = NULL;
        }

      if (dialog)
        picman_dialog_factory_add_dialog (factory, dialog);
    }

  /*  Finally, if we found an existing dialog or created a new one, raise it.
   */
  if (! dialog)
    return NULL;

  if (gtk_widget_is_toplevel (dialog))
    {
      gtk_window_set_screen (GTK_WINDOW (dialog), screen);

      toplevel = dialog;
    }
  else if (PICMAN_IS_DOCK (dialog))
    {
      toplevel = gtk_widget_get_toplevel (dialog);
    }
  else if (PICMAN_IS_DOCKABLE (dialog))
    {
      PicmanDockable *dockable = PICMAN_DOCKABLE (dialog);

      if (picman_dockable_get_dockbook (dockable) &&
          picman_dockbook_get_dock (picman_dockable_get_dockbook (dockable)))
        {
          GtkNotebook *notebook = GTK_NOTEBOOK (picman_dockable_get_dockbook (dockable));
          gint         num      = gtk_notebook_page_num (notebook, dialog);

          if (num != -1)
            {
              gtk_notebook_set_current_page (notebook, num);

              picman_dockable_blink (dockable);
            }
        }

      toplevel = gtk_widget_get_toplevel (dialog);
    }

  if (present && GTK_IS_WINDOW (toplevel))
    {
      /*  Work around focus-stealing protection, or whatever makes the
       *  dock appear below the one where we clicked a button to open
       *  it. See bug #630173.
       */
      gtk_widget_show_now (toplevel);
      gdk_window_raise (gtk_widget_get_window (toplevel));
    }

  return dialog;
}

/**
 * picman_dialog_factory_dialog_new:
 * @factory:      a #PicmanDialogFactory
 * @screen:       the #GdkScreen the dialog should appear on
 * @ui_manager:   A #PicmanUIManager, if applicable.
 * @identifier:   the identifier of the dialog as registered with
 *                picman_dialog_factory_register_entry()
 * @view_size:    the initial preview size
 * @present:      whether gtk_window_present() should be called
 *
 * Creates a new toplevel dialog or a #PicmanDockable, depending on whether
 * %factory is a toplevel of dockable factory.
 *
 * Return value: the newly created dialog or an already existing singleton
 *               dialog.
 **/
GtkWidget *
picman_dialog_factory_dialog_new (PicmanDialogFactory *factory,
                                GdkScreen         *screen,
                                PicmanUIManager     *ui_manager,
                                const gchar       *identifier,
                                gint               view_size,
                                gboolean           present)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  return picman_dialog_factory_dialog_new_internal (factory,
                                                  screen,
                                                  factory->p->context,
                                                  ui_manager,
                                                  identifier,
                                                  view_size,
                                                  FALSE /*return_existing*/,
                                                  present,
                                                  FALSE /*create_containers*/);
}

PicmanContext *
picman_dialog_factory_get_context (PicmanDialogFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);

  return factory->p->context;
}

PicmanMenuFactory *
picman_dialog_factory_get_menu_factory (PicmanDialogFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);

  return factory->p->menu_factory;
}

GList *
picman_dialog_factory_get_open_dialogs (PicmanDialogFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);

  return factory->p->open_dialogs;
}

GList *
picman_dialog_factory_get_session_infos (PicmanDialogFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);

  return factory->p->session_infos;
}

void
picman_dialog_factory_add_session_info (PicmanDialogFactory *factory,
                                      PicmanSessionInfo   *info)
{
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));
  g_return_if_fail (PICMAN_IS_SESSION_INFO (info));

  /* We want to append rather than prepend so that the serialized
   * order in sessionrc remains the same
   */
  factory->p->session_infos = g_list_append (factory->p->session_infos,
                                             g_object_ref (info));
}

/**
 * picman_dialog_factory_dialog_raise:
 * @factory:      a #PicmanDialogFactory
 * @screen:       the #GdkScreen the dialog should appear on
 * @identifiers:  a '|' separated list of identifiers of dialogs as
 *                registered with picman_dialog_factory_register_entry()
 * @view_size:    the initial preview size if a dialog needs to be created
 *
 * Raises any of a list of already existing toplevel dialog or
 * #PicmanDockable if it was already created by this %facory.
 *
 * Implicitly creates the first dialog in the list if none of the dialogs
 * were found.
 *
 * Return value: the raised or newly created dialog.
 **/
GtkWidget *
picman_dialog_factory_dialog_raise (PicmanDialogFactory *factory,
                                  GdkScreen         *screen,
                                  const gchar       *identifiers,
                                  gint               view_size)
{
  GtkWidget *dialog;
  gchar    **ids;
  gint       i;

  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);
  g_return_val_if_fail (identifiers != NULL, NULL);

  /*  If the identifier is a list, try to find a matching dialog and
   *  raise it. If there's no match, use the first list item.
   *
   *  (we split the identifier list manually here because we must pass
   *  a single identifier, not a list, to new_internal() below)
   */
  ids = g_strsplit (identifiers, "|", 0);
  for (i = 0; ids[i]; i++)
    {
      if (picman_dialog_factory_find_widget (factory, ids[i]))
        break;
    }

  dialog = picman_dialog_factory_dialog_new_internal (factory,
                                                    screen,
                                                    NULL,
                                                    NULL,
                                                    ids[i] ? ids[i] : ids[0],
                                                    view_size,
                                                    TRUE /*return_existing*/,
                                                    TRUE /*present*/,
                                                    TRUE /*create_containers*/);
  g_strfreev (ids);

  return dialog;
}

/**
 * picman_dialog_factory_dockable_new:
 * @factory:      a #PicmanDialogFactory
 * @dock:         a #PicmanDock crated by this %factory.
 * @identifier:   the identifier of the dialog as registered with
 *                picman_dialog_factory_register_entry()
 * @view_size:
 *
 * Creates a new #PicmanDockable in the context of the #PicmanDock it will be
 * added to.
 *
 * Implicitly raises & returns an already existing singleton dockable,
 * so callers should check that picman_dockable_get_dockbook (dockable)
 * is NULL before trying to add it to it's #PicmanDockbook.
 *
 * Return value: the newly created #PicmanDockable or an already existing
 *               singleton dockable.
 **/
GtkWidget *
picman_dialog_factory_dockable_new (PicmanDialogFactory *factory,
                                  PicmanDock          *dock,
                                  const gchar       *identifier,
                                  gint               view_size)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_DOCK (dock), NULL);
  g_return_val_if_fail (identifier != NULL, NULL);

  return picman_dialog_factory_dialog_new_internal (factory,
                                                  gtk_widget_get_screen (GTK_WIDGET (dock)),
                                                  picman_dock_get_context (dock),
                                                  picman_dock_get_ui_manager (dock),
                                                  identifier,
                                                  view_size,
                                                  FALSE /*return_existing*/,
                                                  FALSE /*present*/,
                                                  FALSE /*create_containers*/);
}

void
picman_dialog_factory_add_dialog (PicmanDialogFactory *factory,
                                GtkWidget         *dialog)
{
  PicmanDialogFactory      *dialog_factory = NULL;
  PicmanDialogFactoryEntry *entry          = NULL;
  PicmanSessionInfo        *info           = NULL;
  GList                  *list           = NULL;
  gboolean                toplevel       = FALSE;

  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));
  g_return_if_fail (GTK_IS_WIDGET (dialog));

  if (g_list_find (factory->p->open_dialogs, dialog))
    {
      g_warning ("%s: dialog already registered", G_STRFUNC);
      return;
    }

  dialog_factory = picman_dialog_factory_from_widget (dialog, &entry);

  if (! picman_dialog_factory_dialog_sane (factory,
                                         dialog_factory,
                                         entry,
                                         dialog))
    return;

  toplevel = gtk_widget_is_toplevel (dialog);

  if (entry)
    {
      /* dialog is a toplevel (but not a PicmanDockWindow) or a PicmanDockable */

      PICMAN_LOG (DIALOG_FACTORY, "adding %s \"%s\"",
                toplevel ? "toplevel" : "dockable",
                entry->identifier);

      for (list = factory->p->session_infos; list; list = g_list_next (list))
        {
          PicmanSessionInfo *current_info = list->data;

          if (picman_session_info_get_factory_entry (current_info) == entry)
            {
              if (picman_session_info_get_widget (current_info))
                {
                  if (picman_session_info_is_singleton (current_info))
                    {
                      g_warning ("%s: singleton dialog \"%s\" created twice",
                                 G_STRFUNC, entry->identifier);

                      PICMAN_LOG (DIALOG_FACTORY,
                                "corrupt session info: %p (widget %p)",
                                current_info,
                                picman_session_info_get_widget (current_info));

                      return;
                    }

                  continue;
                }

              picman_session_info_set_widget (current_info, dialog);

              PICMAN_LOG (DIALOG_FACTORY,
                        "updating session info %p (widget %p) for %s \"%s\"",
                        current_info,
                        picman_session_info_get_widget (current_info),
                        toplevel ? "toplevel" : "dockable",
                        entry->identifier);

              if (toplevel &&
                  picman_session_info_is_session_managed (current_info) &&
                  ! gtk_widget_get_visible (dialog))
                {
                  picman_session_info_apply_geometry (current_info);
                }

              info = current_info;
              break;
            }
        }

      if (! info)
        {
          info = picman_session_info_new ();

          picman_session_info_set_widget (info, dialog);

          PICMAN_LOG (DIALOG_FACTORY,
                    "creating session info %p (widget %p) for %s \"%s\"",
                    info,
                    picman_session_info_get_widget (info),
                    toplevel ? "toplevel" : "dockable",
                    entry->identifier);

          picman_session_info_set_factory_entry (info, entry);

          if (picman_session_info_is_session_managed (info))
            {
              /* Make the dialog show up at the user position the
               * first time it is shown. After it has been shown the
               * first time we don't want it to show at the mouse the
               * next time. Think of the use cases "hide and show with
               * tab" and "change virtual desktops"
               */
              PICMAN_LOG (WM, "setting GTK_WIN_POS_MOUSE for %p (\"%s\")\n",
                        dialog, entry->identifier);

              gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
              g_signal_connect (dialog, "configure-event",
                                G_CALLBACK (picman_dialog_factory_set_user_pos),
                                NULL);
            }

          picman_dialog_factory_add_session_info (factory, info);
          g_object_unref (info);
        }
    }

  /* Some special logic for dock windows */
  if (PICMAN_IS_DOCK_WINDOW (dialog))
    {
      g_signal_emit (factory, factory_signals[DOCK_WINDOW_ADDED], 0, dialog);
    }

  factory->p->open_dialogs = g_list_prepend (factory->p->open_dialogs, dialog);

  g_signal_connect_object (dialog, "destroy",
                           G_CALLBACK (picman_dialog_factory_remove_dialog),
                           factory,
                           G_CONNECT_SWAPPED);

  if (picman_session_info_is_session_managed (info))
    g_signal_connect_object (dialog, "configure-event",
                             G_CALLBACK (picman_dialog_factory_dialog_configure),
                             factory,
                             0);
}

void
picman_dialog_factory_add_foreign (PicmanDialogFactory *factory,
                                 const gchar       *identifier,
                                 GtkWidget         *dialog)
{
  PicmanDialogFactory      *dialog_factory;
  PicmanDialogFactoryEntry *entry;

  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));
  g_return_if_fail (identifier != NULL);
  g_return_if_fail (GTK_IS_WIDGET (dialog));
  g_return_if_fail (gtk_widget_is_toplevel (dialog));

  dialog_factory = picman_dialog_factory_from_widget (dialog, &entry);

  if (dialog_factory || entry)
    {
      g_warning ("%s: dialog was created by a PicmanDialogFactory",
                 G_STRFUNC);
      return;
    }

  entry = picman_dialog_factory_find_entry (factory, identifier);

  if (! entry)
    {
      g_warning ("%s: no entry registered for \"%s\"",
                 G_STRFUNC, identifier);
      return;
    }

  if (entry->new_func)
    {
      g_warning ("%s: entry for \"%s\" has a constructor (is not foreign)",
                 G_STRFUNC, identifier);
      return;
    }

  picman_dialog_factory_set_widget_data (dialog, factory, entry);

  picman_dialog_factory_add_dialog (factory, dialog);
}

void
picman_dialog_factory_remove_dialog (PicmanDialogFactory *factory,
                                   GtkWidget         *dialog)
{
  PicmanDialogFactory      *dialog_factory;
  PicmanDialogFactoryEntry *entry;
  GList                  *list;

  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));
  g_return_if_fail (GTK_IS_WIDGET (dialog));

  if (! g_list_find (factory->p->open_dialogs, dialog))
    {
      g_warning ("%s: dialog not registered", G_STRFUNC);
      return;
    }

  factory->p->open_dialogs = g_list_remove (factory->p->open_dialogs, dialog);

  dialog_factory = picman_dialog_factory_from_widget (dialog, &entry);

  if (! picman_dialog_factory_dialog_sane (factory,
                                         dialog_factory,
                                         entry,
                                         dialog))
    return;

  PICMAN_LOG (DIALOG_FACTORY, "removing \"%s\" (dialog = %p)",
            entry->identifier,
            dialog);

  for (list = factory->p->session_infos; list; list = g_list_next (list))
    {
      PicmanSessionInfo *session_info = list->data;

      if (picman_session_info_get_widget (session_info) == dialog)
        {
          PICMAN_LOG (DIALOG_FACTORY,
                    "clearing session info %p (widget %p) for \"%s\"",
                    session_info, picman_session_info_get_widget (session_info),
                    entry->identifier);

          picman_session_info_set_widget (session_info, NULL);

          picman_dialog_factory_unset_widget_data (dialog);

          g_signal_handlers_disconnect_by_func (dialog,
                                                picman_dialog_factory_set_user_pos,
                                                NULL);
          g_signal_handlers_disconnect_by_func (dialog,
                                                picman_dialog_factory_remove_dialog,
                                                factory);

          if (picman_session_info_is_session_managed (session_info))
            g_signal_handlers_disconnect_by_func (dialog,
                                                  picman_dialog_factory_dialog_configure,
                                                  factory);

          if (PICMAN_IS_DOCK_WINDOW (dialog))
            {
              /*  don't save session info for empty docks  */
              factory->p->session_infos = g_list_remove (factory->p->session_infos,
                                                         session_info);
              g_object_unref (session_info);

              g_signal_emit (factory, factory_signals[DOCK_WINDOW_REMOVED], 0,
                             dialog);
            }

          break;
        }
    }
}

void
picman_dialog_factory_hide_dialog (GtkWidget *dialog)
{
  PicmanDialogFactory *factory = NULL;

  g_return_if_fail (GTK_IS_WIDGET (dialog));
  g_return_if_fail (gtk_widget_is_toplevel (dialog));

  if (! (factory = picman_dialog_factory_from_widget (dialog, NULL)))
    {
      g_warning ("%s: dialog was not created by a PicmanDialogFactory",
                 G_STRFUNC);
      return;
    }

  gtk_widget_hide (dialog);

  if (factory->p->dialog_state != PICMAN_DIALOGS_SHOWN)
    g_object_set_data (G_OBJECT (dialog), PICMAN_DIALOG_VISIBILITY_KEY,
                       GINT_TO_POINTER (PICMAN_DIALOG_VISIBILITY_INVISIBLE));
}

void
picman_dialog_factory_set_state (PicmanDialogFactory *factory,
                               PicmanDialogsState   state)
{
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));

  factory->p->dialog_state = state;

  if (state == PICMAN_DIALOGS_SHOWN)
    {
      picman_dialog_factory_show (factory);
    }
  else
    {
      picman_dialog_factory_hide (factory);
    }
}

PicmanDialogsState
picman_dialog_factory_get_state (PicmanDialogFactory *factory)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), 0);

  return factory->p->dialog_state;
}

void
picman_dialog_factory_show_with_display (PicmanDialogFactory *factory)
{
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));

  if (factory->p->dialog_state == PICMAN_DIALOGS_HIDDEN_WITH_DISPLAY)
    {
      picman_dialog_factory_set_state (factory, PICMAN_DIALOGS_SHOWN);
    }
}

void
picman_dialog_factory_hide_with_display (PicmanDialogFactory *factory)
{
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));

  if (factory->p->dialog_state == PICMAN_DIALOGS_SHOWN)
    {
      picman_dialog_factory_set_state (factory, PICMAN_DIALOGS_HIDDEN_WITH_DISPLAY);
    }
}

static GQuark picman_dialog_factory_key       = 0;
static GQuark picman_dialog_factory_entry_key = 0;

PicmanDialogFactory *
picman_dialog_factory_from_widget (GtkWidget               *dialog,
                                 PicmanDialogFactoryEntry **entry)
{
  g_return_val_if_fail (GTK_IS_WIDGET (dialog), NULL);

  if (! picman_dialog_factory_key)
    {
      picman_dialog_factory_key =
        g_quark_from_static_string ("picman-dialog-factory");

      picman_dialog_factory_entry_key =
        g_quark_from_static_string ("picman-dialog-factory-entry");
    }

  if (entry)
    *entry = g_object_get_qdata (G_OBJECT (dialog),
                                 picman_dialog_factory_entry_key);

  return g_object_get_qdata (G_OBJECT (dialog), picman_dialog_factory_key);
}

#define PICMAN_DIALOG_FACTORY_MIN_SIZE_KEY "picman-dialog-factory-min-size"

void
picman_dialog_factory_set_has_min_size (GtkWindow *window,
                                      gboolean   has_min_size)
{
  g_return_if_fail (GTK_IS_WINDOW (window));

  g_object_set_data (G_OBJECT (window), PICMAN_DIALOG_FACTORY_MIN_SIZE_KEY,
                     GINT_TO_POINTER (has_min_size ? TRUE : FALSE));
}

gboolean
picman_dialog_factory_get_has_min_size (GtkWindow *window)
{
  g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);

  return GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window),
                                             PICMAN_DIALOG_FACTORY_MIN_SIZE_KEY));
}


/*  private functions  */

static GtkWidget *
picman_dialog_factory_constructor (PicmanDialogFactory      *factory,
                                 PicmanDialogFactoryEntry *entry,
                                 PicmanContext            *context,
                                 PicmanUIManager          *ui_manager,
                                 gint                    view_size)
{
  GtkWidget *widget;

  widget = entry->new_func (factory, context, ui_manager, view_size);

  /* The entry is for a dockable, so we simply need to put the created
   * widget in a dockable
   */
  if (widget && entry->dockable)
    {
      GtkWidget *dockable = NULL;

      dockable = picman_dockable_new (entry->name, entry->blurb,
                                    entry->stock_id, entry->help_id);
      gtk_container_add (GTK_CONTAINER (dockable), widget);
      gtk_widget_show (widget);

      /* EEK */
      g_object_set_data (G_OBJECT (dockable), "picman-dialog-identifier",
                         entry->identifier);

      /* Return the dockable instead */
      widget = dockable;
    }

  return widget;
}

static void
picman_dialog_factory_config_notify (PicmanDialogFactory *factory,
                                   GParamSpec        *pspec,
                                   PicmanGuiConfig     *config)
{
  PicmanDialogsState state     = picman_dialog_factory_get_state (factory);
  PicmanDialogsState new_state = state;

  /* Make sure the state and config are in sync */
  if (config->hide_docks && state == PICMAN_DIALOGS_SHOWN)
    new_state = PICMAN_DIALOGS_HIDDEN_EXPLICITLY;
  else if (! config->hide_docks)
    new_state = PICMAN_DIALOGS_SHOWN;

  if (state != new_state)
    picman_dialog_factory_set_state (factory, new_state);
}

static void
picman_dialog_factory_set_widget_data (GtkWidget              *dialog,
                                     PicmanDialogFactory      *factory,
                                     PicmanDialogFactoryEntry *entry)
{
  g_return_if_fail (GTK_IS_WIDGET (dialog));
  g_return_if_fail (PICMAN_IS_DIALOG_FACTORY (factory));

  if (! picman_dialog_factory_key)
    {
      picman_dialog_factory_key =
        g_quark_from_static_string ("picman-dialog-factory");

      picman_dialog_factory_entry_key =
        g_quark_from_static_string ("picman-dialog-factory-entry");
    }

  g_object_set_qdata (G_OBJECT (dialog), picman_dialog_factory_key, factory);

  if (entry)
    g_object_set_qdata (G_OBJECT (dialog), picman_dialog_factory_entry_key,
                        entry);
}

static void
picman_dialog_factory_unset_widget_data (GtkWidget *dialog)
{
  g_return_if_fail (GTK_IS_WIDGET (dialog));

  if (! picman_dialog_factory_key)
    return;

  g_object_set_qdata (G_OBJECT (dialog), picman_dialog_factory_key,       NULL);
  g_object_set_qdata (G_OBJECT (dialog), picman_dialog_factory_entry_key, NULL);
}

static gboolean
picman_dialog_factory_set_user_pos (GtkWidget         *dialog,
                                  GdkEventConfigure *cevent,
                                  gpointer           data)
{
  GdkWindowHints geometry_mask;

  /* Not only set geometry hints, also reset window position */
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_NONE);
  g_signal_handlers_disconnect_by_func (dialog,
                                        picman_dialog_factory_set_user_pos,
                                        data);

  PICMAN_LOG (WM, "setting GDK_HINT_USER_POS for %p\n", dialog);
  geometry_mask = GDK_HINT_USER_POS;

  if (picman_dialog_factory_get_has_min_size (GTK_WINDOW (dialog)))
    geometry_mask |= GDK_HINT_MIN_SIZE;

  gtk_window_set_geometry_hints (GTK_WINDOW (dialog), NULL, NULL,
                                 geometry_mask);

  return FALSE;
}

static gboolean
picman_dialog_factory_dialog_configure (GtkWidget         *dialog,
                                      GdkEventConfigure *cevent,
                                      PicmanDialogFactory *factory)
{
  PicmanDialogFactory      *dialog_factory;
  PicmanDialogFactoryEntry *entry;
  GList                  *list;

  if (! g_list_find (factory->p->open_dialogs, dialog))
    {
      g_warning ("%s: dialog not registered", G_STRFUNC);
      return FALSE;
    }

  dialog_factory = picman_dialog_factory_from_widget (dialog, &entry);

  if (! picman_dialog_factory_dialog_sane (factory,
                                         dialog_factory,
                                         entry,
                                         dialog))
    return FALSE;

  for (list = factory->p->session_infos; list; list = g_list_next (list))
    {
      PicmanSessionInfo *session_info = list->data;

      if (picman_session_info_get_widget (session_info) == dialog)
        {
          picman_session_info_read_geometry (session_info, cevent);

          PICMAN_LOG (DIALOG_FACTORY,
                    "updated session info for \"%s\" from window geometry "
                    "(x=%d y=%d  %dx%d)",
                    entry->identifier,
                    picman_session_info_get_x (session_info),
                    picman_session_info_get_y (session_info),
                    picman_session_info_get_width (session_info),
                    picman_session_info_get_height (session_info));

          break;
        }
    }

  return FALSE;
}

void
picman_dialog_factory_save (PicmanDialogFactory *factory,
                          PicmanConfigWriter  *writer)
{
  GList *infos;

  for (infos = factory->p->session_infos; infos; infos = g_list_next (infos))
    {
      PicmanSessionInfo *info = infos->data;

      /*  we keep session info entries for all toplevel dialogs created
       *  by the factory but don't save them if they don't want to be
       *  managed
       */
      if (! picman_session_info_is_session_managed (info) ||
          picman_session_info_get_factory_entry (info) == NULL)
        continue;

      if (picman_session_info_get_widget (info))
        picman_session_info_get_info (info);

      picman_config_writer_open (writer, "session-info");
      picman_config_writer_string (writer,
                                 picman_object_get_name (factory));

      PICMAN_CONFIG_GET_INTERFACE (info)->serialize (PICMAN_CONFIG (info),
                                                   writer,
                                                   NULL);

      picman_config_writer_close (writer);

      if (picman_session_info_get_widget (info))
        picman_session_info_clear_info (info);
    }
}

void
picman_dialog_factory_restore (PicmanDialogFactory *factory)
{
  GList *infos;

  for (infos = factory->p->session_infos; infos; infos = g_list_next (infos))
    {
      PicmanSessionInfo *info = infos->data;

      if (picman_session_info_get_open (info))
        {
          picman_session_info_restore (info, factory);
        }
      else
        {
          PICMAN_LOG (DIALOG_FACTORY,
                    "skipping to restore session info %p, not open",
                    info);
        }
    }
}

static void
picman_dialog_factory_hide (PicmanDialogFactory *factory)
{
  GList *list;

  for (list = factory->p->open_dialogs; list; list = g_list_next (list))
    {
      GtkWidget *widget = list->data;

      if (GTK_IS_WIDGET (widget) && gtk_widget_is_toplevel (widget))
        {
          PicmanDialogFactoryEntry    *entry      = NULL;
          PicmanDialogVisibilityState  visibility = PICMAN_DIALOG_VISIBILITY_UNKNOWN;

          picman_dialog_factory_from_widget (widget, &entry);
          if (! entry->hideable)
            continue;

          if (gtk_widget_get_visible (widget))
            {
              gtk_widget_hide (widget);
              visibility = PICMAN_DIALOG_VISIBILITY_HIDDEN;

              PICMAN_LOG (WM, "Hiding '%s' [%p]",
                        gtk_window_get_title (GTK_WINDOW (widget)),
                        widget);
            }
          else
            {
              visibility = PICMAN_DIALOG_VISIBILITY_INVISIBLE;
            }

          g_object_set_data (G_OBJECT (widget),
                             PICMAN_DIALOG_VISIBILITY_KEY,
                             GINT_TO_POINTER (visibility));
        }
    }
}

static void
picman_dialog_factory_show (PicmanDialogFactory *factory)
{
  GList *list;

  for (list = factory->p->open_dialogs; list; list = g_list_next (list))
    {
      GtkWidget *widget = list->data;

      if (GTK_IS_WIDGET (widget) && gtk_widget_is_toplevel (widget))
        {
          PicmanDialogVisibilityState visibility;

          visibility =
            GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
                                                PICMAN_DIALOG_VISIBILITY_KEY));

          if (! gtk_widget_get_visible (widget) &&
              visibility == PICMAN_DIALOG_VISIBILITY_HIDDEN)
            {
              PICMAN_LOG (WM, "Showing '%s' [%p]",
                        gtk_window_get_title (GTK_WINDOW (widget)),
                        widget);

              /* Don't use gtk_window_present() here, we don't want the
               * keyboard focus to move.
               */
              gtk_widget_show (widget);
              g_object_set_data (G_OBJECT (widget),
                                 PICMAN_DIALOG_VISIBILITY_KEY,
                                 GINT_TO_POINTER (PICMAN_DIALOG_VISIBILITY_VISIBLE));

              if (gtk_widget_get_visible (widget))
                gdk_window_raise (gtk_widget_get_window (widget));
            }
        }
    }
}

void
picman_dialog_factory_set_busy (PicmanDialogFactory *factory)
{
  GdkDisplay *display = NULL;
  GdkCursor  *cursor  = NULL;
  GList      *list;

  if (! factory)
    return;

  for (list = factory->p->open_dialogs; list; list = g_list_next (list))
    {
      GtkWidget *widget = list->data;

      if (GTK_IS_WIDGET (widget) && gtk_widget_is_toplevel (widget))
        {
          if (!display || display != gtk_widget_get_display (widget))
            {
              display = gtk_widget_get_display (widget);

              if (cursor)
                gdk_cursor_unref (cursor);

              cursor = picman_cursor_new (display,
                                        PICMAN_HANDEDNESS_RIGHT,
                                        GDK_WATCH,
                                        PICMAN_TOOL_CURSOR_NONE,
                                        PICMAN_CURSOR_MODIFIER_NONE);
            }

          if (gtk_widget_get_window (widget))
            gdk_window_set_cursor (gtk_widget_get_window (widget), cursor);
        }
    }

  if (cursor)
    gdk_cursor_unref (cursor);
}

void
picman_dialog_factory_unset_busy (PicmanDialogFactory *factory)
{
  GList *list;

  if (! factory)
    return;

  for (list = factory->p->open_dialogs; list; list = g_list_next (list))
    {
      GtkWidget *widget = list->data;

      if (GTK_IS_WIDGET (widget) && gtk_widget_is_toplevel (widget))
        {
          if (gtk_widget_get_window (widget))
            gdk_window_set_cursor (gtk_widget_get_window (widget), NULL);
        }
    }
}

/**
 * picman_dialog_factory_get_singleton:
 *
 * Returns: The toplevel PicmanDialogFactory instance.
 **/
PicmanDialogFactory *
picman_dialog_factory_get_singleton (void)
{
  g_return_val_if_fail (picman_toplevel_factory != NULL, NULL);

  return picman_toplevel_factory;
}

/**
 * picman_dialog_factory_set_singleton:
 * @:
 *
 * Set the toplevel PicmanDialogFactory instance. Must only be called by
 * dialogs_init()!.
 **/
void
picman_dialog_factory_set_singleton (PicmanDialogFactory *factory)
{
  g_return_if_fail (picman_toplevel_factory == NULL ||
                    factory               == NULL);

  picman_toplevel_factory = factory;
}
