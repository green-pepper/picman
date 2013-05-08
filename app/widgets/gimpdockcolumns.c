/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockcolumns.c
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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "widgets-types.h"

#include "menus/menus.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmandialogfactory.h"
#include "picmandock.h"
#include "picmandockable.h"
#include "picmandockbook.h"
#include "picmandockcolumns.h"
#include "picmanmenudock.h"
#include "picmanpanedbox.h"
#include "picmantoolbox.h"
#include "picmanuimanager.h"

#include "picman-log.h"


enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_DIALOG_FACTORY,
  PROP_UI_MANAGER
};

enum
{
  DOCK_ADDED,
  DOCK_REMOVED,
  LAST_SIGNAL
};


struct _PicmanDockColumnsPrivate
{
  PicmanContext       *context;
  PicmanDialogFactory *dialog_factory;
  PicmanUIManager     *ui_manager;

  GList             *docks;

  GtkWidget         *paned_hbox;
};


static void      picman_dock_columns_dispose           (GObject         *object);
static void      picman_dock_columns_set_property      (GObject         *object,
                                                      guint            property_id,
                                                      const GValue    *value,
                                                      GParamSpec      *pspec);
static void      picman_dock_columns_get_property      (GObject         *object,
                                                      guint            property_id,
                                                      GValue          *value,
                                                      GParamSpec      *pspec);
static gboolean  picman_dock_columns_dropped_cb        (GtkWidget       *source,
                                                      gint             insert_index,
                                                      gpointer         data);
static void      picman_dock_columns_real_dock_added   (PicmanDockColumns *dock_columns,
                                                      PicmanDock        *dock);
static void      picman_dock_columns_real_dock_removed (PicmanDockColumns *dock_columns,
                                                      PicmanDock        *dock);
static void      picman_dock_columns_dock_book_removed (PicmanDockColumns *dock_columns,
                                                      PicmanDockbook    *dockbook,
                                                      PicmanDock        *dock);


G_DEFINE_TYPE (PicmanDockColumns, picman_dock_columns, GTK_TYPE_BOX)

#define parent_class picman_dock_columns_parent_class

static guint dock_columns_signals[LAST_SIGNAL] = { 0 };


static void
picman_dock_columns_class_init (PicmanDockColumnsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose      = picman_dock_columns_dispose;
  object_class->set_property = picman_dock_columns_set_property;
  object_class->get_property = picman_dock_columns_get_property;

  klass->dock_added   = picman_dock_columns_real_dock_added;
  klass->dock_removed = picman_dock_columns_real_dock_removed;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_DIALOG_FACTORY,
                                   g_param_spec_object ("dialog-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DIALOG_FACTORY,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_UI_MANAGER,
                                   g_param_spec_object ("ui-manager",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_UI_MANAGER,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  dock_columns_signals[DOCK_ADDED] =
    g_signal_new ("dock-added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDockColumnsClass, dock_added),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DOCK);

  dock_columns_signals[DOCK_REMOVED] =
    g_signal_new ("dock-removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDockColumnsClass, dock_removed),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DOCK);

  g_type_class_add_private (klass, sizeof (PicmanDockColumnsPrivate));
}

static void
picman_dock_columns_init (PicmanDockColumns *dock_columns)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (dock_columns),
                                  GTK_ORIENTATION_HORIZONTAL);

  dock_columns->p = G_TYPE_INSTANCE_GET_PRIVATE (dock_columns,
                                                 PICMAN_TYPE_DOCK_COLUMNS,
                                                 PicmanDockColumnsPrivate);

  dock_columns->p->paned_hbox = picman_paned_box_new (FALSE, 0,
                                                    GTK_ORIENTATION_HORIZONTAL);
  picman_paned_box_set_dropped_cb (PICMAN_PANED_BOX (dock_columns->p->paned_hbox),
                                 picman_dock_columns_dropped_cb,
                                 dock_columns);
  gtk_box_pack_start (GTK_BOX (dock_columns), dock_columns->p->paned_hbox,
                      TRUE, TRUE, 0);
  gtk_widget_show (dock_columns->p->paned_hbox);
}

static void
picman_dock_columns_dispose (GObject *object)
{
  PicmanDockColumns *dock_columns = PICMAN_DOCK_COLUMNS (object);

  while (dock_columns->p->docks)
    {
      PicmanDock *dock = dock_columns->p->docks->data;

      g_object_ref (dock);
      picman_dock_columns_remove_dock (dock_columns, dock);
      gtk_widget_destroy (GTK_WIDGET (dock));
      g_object_unref (dock);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_dock_columns_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanDockColumns *dock_columns = PICMAN_DOCK_COLUMNS (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      dock_columns->p->context = g_value_get_object (value);
      break;
    case PROP_DIALOG_FACTORY:
      dock_columns->p->dialog_factory = g_value_get_object (value);
      break;
    case PROP_UI_MANAGER:
      dock_columns->p->ui_manager = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dock_columns_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanDockColumns *dock_columns = PICMAN_DOCK_COLUMNS (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, dock_columns->p->context);
      break;
    case PROP_DIALOG_FACTORY:
      g_value_set_object (value, dock_columns->p->dialog_factory);
      break;
    case PROP_UI_MANAGER:
      g_value_set_object (value, dock_columns->p->ui_manager);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_dock_columns_dropped_cb (GtkWidget *source,
                              gint       insert_index,
                              gpointer   data)
{
  PicmanDockColumns *dock_columns = PICMAN_DOCK_COLUMNS (data);
  PicmanDockable    *dockable     = picman_dockbook_drag_source_to_dockable (source);
  GtkWidget       *dockbook     = NULL;

  if (! dockable)
    return FALSE;

  /* Create a new dock (including a new dockbook) */
  picman_dock_columns_prepare_dockbook (dock_columns,
                                      insert_index,
                                      &dockbook);

  /* Move the dockable to the new dockbook */
  g_object_ref (dockbook);
  g_object_ref (dockable);
  picman_dockbook_remove (picman_dockable_get_dockbook (dockable), dockable);
  picman_dockbook_add (PICMAN_DOCKBOOK (dockbook), dockable, -1);
  g_object_unref (dockable);
  g_object_unref (dockbook);

  return TRUE;
}

static void
picman_dock_columns_real_dock_added (PicmanDockColumns *dock_columns,
                                   PicmanDock        *dock)
{
}

static void
picman_dock_columns_real_dock_removed (PicmanDockColumns *dock_columns,
                                     PicmanDock        *dock)
{
}

static void
picman_dock_columns_dock_book_removed (PicmanDockColumns *dock_columns,
                                     PicmanDockbook    *dockbook,
                                     PicmanDock        *dock)
{
  g_return_if_fail (PICMAN_IS_DOCK (dock));

  if (picman_dock_get_dockbooks (dock) == NULL &&
      ! PICMAN_IS_TOOLBOX (dock) &&
      gtk_widget_get_parent (GTK_WIDGET (dock)) != NULL)
    picman_dock_columns_remove_dock (dock_columns, dock);
}


/**
 * picman_dock_columns_new:
 * @context:
 *
 * Returns: A new #PicmanDockColumns.
 **/
GtkWidget *
picman_dock_columns_new (PicmanContext       *context,
                       PicmanDialogFactory *dialog_factory,
                       PicmanUIManager     *ui_manager)
{
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (dialog_factory), NULL);
  g_return_val_if_fail (PICMAN_IS_UI_MANAGER (ui_manager), NULL);

  return g_object_new (PICMAN_TYPE_DOCK_COLUMNS,
                       "context",        context,
                       "dialog-factory", dialog_factory,
                       "ui-manager",     ui_manager,
                       NULL);
}

/**
 * picman_dock_columns_add_dock:
 * @dock_columns:
 * @dock:
 *
 * Add a dock, added to a horizontal PicmanPanedBox.
 **/
void
picman_dock_columns_add_dock (PicmanDockColumns *dock_columns,
                            PicmanDock        *dock,
                            gint             index)
{
  g_return_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns));
  g_return_if_fail (PICMAN_IS_DOCK (dock));

  PICMAN_LOG (DND, "Adding PicmanDock %p to PicmanDockColumns %p", dock, dock_columns);

  dock_columns->p->docks = g_list_append (dock_columns->p->docks, dock);

  picman_dock_update_with_context (dock, dock_columns->p->context);

  picman_paned_box_add_widget (PICMAN_PANED_BOX (dock_columns->p->paned_hbox),
                             GTK_WIDGET (dock),
                             index);

  g_signal_connect_object (dock, "book-removed",
                           G_CALLBACK (picman_dock_columns_dock_book_removed),
                           dock_columns,
                           G_CONNECT_SWAPPED);

  g_signal_emit (dock_columns, dock_columns_signals[DOCK_ADDED], 0, dock);
}

/**
 * picman_dock_columns_prepare_dockbook:
 * @dock_columns:
 * @dock_index:
 * @dockbook_p:
 *
 * Create a new dock and add it to the dock columns with the given
 * dock_index insert index, then create and add a dockbook and put it
 * in the dock.
 **/
void
picman_dock_columns_prepare_dockbook (PicmanDockColumns  *dock_columns,
                                    gint              dock_index,
                                    GtkWidget       **dockbook_p)
{
  GtkWidget *dock;
  GtkWidget *dockbook;

  dock = picman_menu_dock_new ();
  picman_dock_columns_add_dock (dock_columns, PICMAN_DOCK (dock), dock_index);

  dockbook = picman_dockbook_new (global_menu_factory);
  picman_dock_add_book (PICMAN_DOCK (dock), PICMAN_DOCKBOOK (dockbook), -1);

  gtk_widget_show (GTK_WIDGET (dock));

  if (dockbook_p)
    *dockbook_p = dockbook;
}


void
picman_dock_columns_remove_dock (PicmanDockColumns *dock_columns,
                               PicmanDock        *dock)
{
  g_return_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns));
  g_return_if_fail (PICMAN_IS_DOCK (dock));

  PICMAN_LOG (DND, "Removing PicmanDock %p from PicmanDockColumns %p", dock, dock_columns);

  dock_columns->p->docks = g_list_remove (dock_columns->p->docks, dock);

  picman_dock_update_with_context (dock, NULL);

  g_signal_handlers_disconnect_by_func (dock,
                                        picman_dock_columns_dock_book_removed,
                                        dock_columns);

  g_object_ref (dock);
  picman_paned_box_remove_widget (PICMAN_PANED_BOX (dock_columns->p->paned_hbox),
                                GTK_WIDGET (dock));

  g_signal_emit (dock_columns, dock_columns_signals[DOCK_REMOVED], 0, dock);
  g_object_unref (dock);
}

GList *
picman_dock_columns_get_docks (PicmanDockColumns *dock_columns)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns), NULL);

  return dock_columns->p->docks;
}

PicmanContext *
picman_dock_columns_get_context (PicmanDockColumns *dock_columns)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns), NULL);

  return dock_columns->p->context;
}

void
picman_dock_columns_set_context (PicmanDockColumns *dock_columns,
                               PicmanContext     *context)
{
  g_return_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns));

  dock_columns->p->context = context;
}

PicmanDialogFactory *
picman_dock_columns_get_dialog_factory (PicmanDockColumns *dock_columns)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns), NULL);

  return dock_columns->p->dialog_factory;
}

PicmanUIManager *
picman_dock_columns_get_ui_manager (PicmanDockColumns *dock_columns)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_COLUMNS (dock_columns), NULL);

  return dock_columns->p->ui_manager;
}
