/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainerview.c
 * Copyright (C) 2001-2010 Michael Natterer <mitch@picman.org>
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanmarshal.h"
#include "core/picmantreehandler.h"
#include "core/picmanviewable.h"

#include "picmancontainerview.h"
#include "picmandnd.h"
#include "picmanviewrenderer.h"
#include "picmanuimanager.h"
#include "picmancontainertreeview.h"


enum
{
  SELECT_ITEM,
  ACTIVATE_ITEM,
  CONTEXT_ITEM,
  LAST_SIGNAL
};


#define PICMAN_CONTAINER_VIEW_GET_PRIVATE(obj) (picman_container_view_get_private ((PicmanContainerView *) (obj)))


typedef struct _PicmanContainerViewPrivate PicmanContainerViewPrivate;

struct _PicmanContainerViewPrivate
{
  PicmanContainer   *container;
  PicmanContext     *context;

  GHashTable      *item_hash;

  gint             view_size;
  gint             view_border_width;
  gboolean         reorderable;
  GtkSelectionMode selection_mode;

  /*  initialized by subclass  */
  GtkWidget       *dnd_widget;

  PicmanTreeHandler *name_changed_handler;
};


static void   picman_container_view_iface_base_init   (PicmanContainerViewInterface *view_iface);

static PicmanContainerViewPrivate *
              picman_container_view_get_private        (PicmanContainerView *view);

static void   picman_container_view_real_set_container (PicmanContainerView *view,
                                                      PicmanContainer     *container);
static void   picman_container_view_real_set_context   (PicmanContainerView *view,
                                                      PicmanContext       *context);
static void   picman_container_view_real_set_selection_mode (PicmanContainerView *view,
                                                           GtkSelectionMode   mode);

static void   picman_container_view_clear_items      (PicmanContainerView  *view);
static void   picman_container_view_real_clear_items (PicmanContainerView  *view);

static void   picman_container_view_add_container    (PicmanContainerView  *view,
                                                    PicmanContainer      *container);
static void   picman_container_view_add_foreach      (PicmanViewable       *viewable,
                                                    PicmanContainerView  *view);
static void   picman_container_view_add              (PicmanContainerView  *view,
                                                    PicmanViewable       *viewable,
                                                    PicmanContainer      *container);

static void   picman_container_view_remove_container (PicmanContainerView  *view,
                                                    PicmanContainer      *container);
static void   picman_container_view_remove_foreach   (PicmanViewable       *viewable,
                                                    PicmanContainerView  *view);
static void   picman_container_view_remove           (PicmanContainerView  *view,
                                                    PicmanViewable       *viewable,
                                                    PicmanContainer      *container);

static void   picman_container_view_reorder          (PicmanContainerView  *view,
                                                    PicmanViewable       *viewable,
                                                    gint                new_index,
                                                    PicmanContainer      *container);

static void   picman_container_view_freeze           (PicmanContainerView  *view,
                                                    PicmanContainer      *container);
static void   picman_container_view_thaw             (PicmanContainerView  *view,
                                                    PicmanContainer      *container);
static void   picman_container_view_name_changed     (PicmanViewable       *viewable,
                                                    PicmanContainerView  *view);

static void   picman_container_view_connect_context    (PicmanContainerView *view);
static void   picman_container_view_disconnect_context (PicmanContainerView *view);

static void   picman_container_view_context_changed  (PicmanContext        *context,
                                                    PicmanViewable       *viewable,
                                                    PicmanContainerView  *view);
static void   picman_container_view_viewable_dropped (GtkWidget          *widget,
                                                    gint                x,
                                                    gint                y,
                                                    PicmanViewable       *viewable,
                                                    gpointer            data);
static void  picman_container_view_button_viewable_dropped (GtkWidget    *widget,
                                                          gint          x,
                                                          gint          y,
                                                          PicmanViewable *viewable,
                                                          gpointer      data);
static gint  picman_container_view_real_get_selected (PicmanContainerView    *view,
                                                    GList               **list);


static guint view_signals[LAST_SIGNAL] = { 0 };


GType
picman_container_view_interface_get_type (void)
{
  static GType iface_type = 0;

  if (! iface_type)
    {
      const GTypeInfo iface_info =
      {
        sizeof (PicmanContainerViewInterface),
        (GBaseInitFunc)     picman_container_view_iface_base_init,
        (GBaseFinalizeFunc) NULL,
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                           "PicmanContainerViewInterface",
                                           &iface_info,
                                           0);

      g_type_interface_add_prerequisite (iface_type, GTK_TYPE_WIDGET);
    }

  return iface_type;
}

static void
picman_container_view_iface_base_init (PicmanContainerViewInterface *view_iface)
{
  if (view_iface->set_container)
    return;

  view_signals[SELECT_ITEM] =
    g_signal_new ("select-item",
                  G_TYPE_FROM_INTERFACE (view_iface),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanContainerViewInterface, select_item),
                  NULL, NULL,
                  picman_marshal_BOOLEAN__OBJECT_POINTER,
                  G_TYPE_BOOLEAN, 2,
                  PICMAN_TYPE_OBJECT,
                  G_TYPE_POINTER);

  view_signals[ACTIVATE_ITEM] =
    g_signal_new ("activate-item",
                  G_TYPE_FROM_INTERFACE (view_iface),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContainerViewInterface, activate_item),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_OBJECT,
                  G_TYPE_POINTER);

  view_signals[CONTEXT_ITEM] =
    g_signal_new ("context-item",
                  G_TYPE_FROM_INTERFACE (view_iface),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanContainerViewInterface, context_item),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT_POINTER,
                  G_TYPE_NONE, 2,
                  PICMAN_TYPE_OBJECT,
                  G_TYPE_POINTER);

  view_iface->select_item        = NULL;
  view_iface->activate_item      = NULL;
  view_iface->context_item       = NULL;

  view_iface->set_container      = picman_container_view_real_set_container;
  view_iface->set_context        = picman_container_view_real_set_context;
  view_iface->set_selection_mode = picman_container_view_real_set_selection_mode;
  view_iface->insert_item        = NULL;
  view_iface->insert_item_after  = NULL;
  view_iface->remove_item        = NULL;
  view_iface->reorder_item       = NULL;
  view_iface->rename_item        = NULL;
  view_iface->clear_items        = picman_container_view_real_clear_items;
  view_iface->set_view_size      = NULL;
  view_iface->get_selected       = picman_container_view_real_get_selected;

  view_iface->insert_data_free   = NULL;
  view_iface->model_is_tree      = FALSE;

  g_object_interface_install_property (view_iface,
                                       g_param_spec_object ("container",
                                                            NULL, NULL,
                                                            PICMAN_TYPE_CONTAINER,
                                                            PICMAN_PARAM_READWRITE));

  g_object_interface_install_property (view_iface,
                                       g_param_spec_object ("context",
                                                            NULL, NULL,
                                                            PICMAN_TYPE_CONTEXT,
                                                            PICMAN_PARAM_READWRITE));

  g_object_interface_install_property (view_iface,
                                       g_param_spec_enum ("selection-mode",
                                                          NULL, NULL,
                                                          GTK_TYPE_SELECTION_MODE,
                                                          GTK_SELECTION_SINGLE,
                                                          PICMAN_PARAM_READWRITE));

  g_object_interface_install_property (view_iface,
                                       g_param_spec_boolean ("reorderable",
                                                             NULL, NULL,
                                                             FALSE,
                                                             PICMAN_PARAM_READWRITE));

  g_object_interface_install_property (view_iface,
                                       g_param_spec_int ("view-size",
                                                         NULL, NULL,
                                                         1, PICMAN_VIEWABLE_MAX_PREVIEW_SIZE,
                                                         PICMAN_VIEW_SIZE_MEDIUM,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_object_interface_install_property (view_iface,
                                       g_param_spec_int ("view-border-width",
                                                         NULL, NULL,
                                                         0,
                                                         PICMAN_VIEW_MAX_BORDER_WIDTH,
                                                         1,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));
}

static void
picman_container_view_private_dispose (PicmanContainerView        *view,
                                     PicmanContainerViewPrivate *private)
{
  if (private->container)
    picman_container_view_set_container (view, NULL);

  if (private->context)
    picman_container_view_set_context (view, NULL);
}

static void
picman_container_view_private_finalize (PicmanContainerViewPrivate *private)
{
  if (private->item_hash)
    {
      g_hash_table_destroy (private->item_hash);
      private->item_hash = NULL;
    }

  g_slice_free (PicmanContainerViewPrivate, private);
}

static PicmanContainerViewPrivate *
picman_container_view_get_private (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private;

  static GQuark private_key = 0;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), NULL);

  if (! private_key)
    private_key = g_quark_from_static_string ("picman-container-view-private");

  private = g_object_get_qdata ((GObject *) view, private_key);

  if (! private)
    {
      PicmanContainerViewInterface *view_iface;

      view_iface = PICMAN_CONTAINER_VIEW_GET_INTERFACE (view);

      private = g_slice_new0 (PicmanContainerViewPrivate);

      private->view_border_width = 1;

      private->item_hash = g_hash_table_new_full (g_direct_hash,
                                                  g_direct_equal,
                                                  NULL,
                                                  view_iface->insert_data_free);

      g_object_set_qdata_full ((GObject *) view, private_key, private,
                               (GDestroyNotify) picman_container_view_private_finalize);

      g_signal_connect (view, "destroy",
                        G_CALLBACK (picman_container_view_private_dispose),
                        private);
    }

  return private;
}

/**
 * picman_container_view_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 *
 * Installs the necessary properties for a class implementing
 * #PicmanContainerView. A #PicmanContainerViewProp property is installed
 * for each property, using the values from the #PicmanContainerViewProp
 * enumeration. The caller must make sure itself that the enumeration
 * values don't collide with some other property values they
 * are using (that's what %PICMAN_CONTAINER_VIEW_PROP_LAST is good for).
 **/
void
picman_container_view_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_CONTAINER,
                                    "container");
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_CONTEXT,
                                    "context");
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_SELECTION_MODE,
                                    "selection-mode");
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_REORDERABLE,
                                    "reorderable");
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE,
                                    "view-size");
  g_object_class_override_property (klass,
                                    PICMAN_CONTAINER_VIEW_PROP_VIEW_BORDER_WIDTH,
                                    "view-border-width");
}

PicmanContainer *
picman_container_view_get_container (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), NULL);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return private->container;
}

void
picman_container_view_set_container (PicmanContainerView *view,
                                   PicmanContainer     *container)
{
  PicmanContainerViewPrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (container == NULL || PICMAN_IS_CONTAINER (container));
  if (container)
    g_return_if_fail (g_type_is_a (picman_container_get_children_type (container),
                                   PICMAN_TYPE_VIEWABLE));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (container != private->container)
    {
      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->set_container (view, container);

      g_object_notify (G_OBJECT (view), "container");
    }
}

static void
picman_container_view_real_set_container (PicmanContainerView *view,
                                        PicmanContainer     *container)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (private->container)
    {
      picman_container_view_select_item (view, NULL);

      /* freeze/thaw is only supported for the toplevel container */
      g_signal_handlers_disconnect_by_func (private->container,
                                            picman_container_view_freeze,
                                            view);
      g_signal_handlers_disconnect_by_func (private->container,
                                            picman_container_view_thaw,
                                            view);

      if (! picman_container_frozen (private->container))
        picman_container_view_remove_container (view, private->container);

      if (private->context)
        picman_container_view_disconnect_context (view);
    }

  private->container = container;

  if (private->container)
    {
      if (! picman_container_frozen (private->container))
        picman_container_view_add_container (view, private->container);

      /* freeze/thaw is only supported for the toplevel container */
      g_signal_connect_object (private->container, "freeze",
                               G_CALLBACK (picman_container_view_freeze),
                               view,
                               G_CONNECT_SWAPPED);
      g_signal_connect_object (private->container, "thaw",
                               G_CALLBACK (picman_container_view_thaw),
                               view,
                               G_CONNECT_SWAPPED);

      if (private->context)
        picman_container_view_connect_context (view);
    }
}

PicmanContext *
picman_container_view_get_context (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), NULL);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return private->context;
}

void
picman_container_view_set_context (PicmanContainerView *view,
                                 PicmanContext       *context)
{
  PicmanContainerViewPrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (context != private->context)
    {
      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->set_context (view, context);

      g_object_notify (G_OBJECT (view), "context");
    }
}

static void
picman_container_view_real_set_context (PicmanContainerView *view,
                                      PicmanContext       *context)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (private->context)
    {
      if (private->container)
        picman_container_view_disconnect_context (view);

      g_object_unref (private->context);
    }

  private->context = context;

  if (private->context)
    {
      g_object_ref (private->context);

      if (private->container)
        picman_container_view_connect_context (view);
    }
}

GtkSelectionMode
picman_container_view_get_selection_mode (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return private->selection_mode;
}

void
picman_container_view_set_selection_mode (PicmanContainerView *view,
                                        GtkSelectionMode   mode)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (mode == GTK_SELECTION_SINGLE ||
                    mode == GTK_SELECTION_MULTIPLE);

  PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->set_selection_mode (view, mode);
}

static void
picman_container_view_real_set_selection_mode (PicmanContainerView *view,
                                             GtkSelectionMode   mode)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  private->selection_mode = mode;
}

gint
picman_container_view_get_view_size (PicmanContainerView *view,
                                   gint              *view_border_width)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), 0);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (view_border_width)
    *view_border_width = private->view_border_width;

  return private->view_size;
}

void
picman_container_view_set_view_size (PicmanContainerView *view,
                                   gint               view_size,
                                   gint               view_border_width)
{
  PicmanContainerViewPrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (view_size >  0 &&
                    view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);
  g_return_if_fail (view_border_width >= 0 &&
                    view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (private->view_size         != view_size ||
      private->view_border_width != view_border_width)
    {
      private->view_size         = view_size;
      private->view_border_width = view_border_width;

      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->set_view_size (view);

      g_object_freeze_notify (G_OBJECT (view));
      g_object_notify (G_OBJECT (view), "view-size");
      g_object_notify (G_OBJECT (view), "view-border-width");
      g_object_thaw_notify (G_OBJECT (view));
    }
}

gboolean
picman_container_view_get_reorderable (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), FALSE);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return private->reorderable;
}

void
picman_container_view_set_reorderable (PicmanContainerView *view,
                                     gboolean           reorderable)
{
  PicmanContainerViewPrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  private->reorderable = reorderable ? TRUE : FALSE;
  g_object_notify (G_OBJECT (view), "reorderable");
}

GtkWidget *
picman_container_view_get_dnd_widget (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), NULL);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return private->dnd_widget;
}

void
picman_container_view_set_dnd_widget (PicmanContainerView *view,
                                    GtkWidget         *dnd_widget)
{
  PicmanContainerViewPrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (dnd_widget == NULL || GTK_IS_WIDGET (dnd_widget));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  private->dnd_widget = dnd_widget;
}

void
picman_container_view_enable_dnd (PicmanContainerView *view,
                                GtkButton         *button,
                                GType              children_type)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (GTK_IS_BUTTON (button));

  picman_dnd_viewable_dest_add (GTK_WIDGET (button),
                              children_type,
                              picman_container_view_button_viewable_dropped,
                              view);
}

gboolean
picman_container_view_select_item (PicmanContainerView *view,
                                 PicmanViewable      *viewable)
{
  PicmanContainerViewPrivate *private;
  gboolean                  success = FALSE;
  gpointer                  insert_data;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), FALSE);
  g_return_val_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable), FALSE);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  g_signal_emit (view, view_signals[SELECT_ITEM], 0,
                 viewable, insert_data, &success);

  return success;
}

void
picman_container_view_activate_item (PicmanContainerView *view,
                                   PicmanViewable      *viewable)
{
  PicmanContainerViewPrivate *private;
  gpointer                  insert_data;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  g_signal_emit (view, view_signals[ACTIVATE_ITEM], 0,
                 viewable, insert_data);
}

void
picman_container_view_context_item (PicmanContainerView *view,
                                  PicmanViewable      *viewable)
{
  PicmanContainerViewPrivate *private;
  gpointer                  insert_data;

  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  g_signal_emit (view, view_signals[CONTEXT_ITEM], 0,
                 viewable, insert_data);
}

gpointer
picman_container_view_lookup (PicmanContainerView *view,
                            PicmanViewable      *viewable)
{
  PicmanContainerViewPrivate *private;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), NULL);
  g_return_val_if_fail (viewable == NULL || PICMAN_IS_VIEWABLE (viewable), NULL);

  /*  we handle the NULL viewable here as a workaround for bug #149906 */
  if (! viewable)
    return NULL;

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  return g_hash_table_lookup (private->item_hash, viewable);
}

gboolean
picman_container_view_item_selected (PicmanContainerView *view,
                                   PicmanViewable      *viewable)
{
  PicmanContainerViewPrivate *private;
  gboolean                  success;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), FALSE);
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  /* HACK */
  if (private->container && private->context)
    {
      GType        children_type;
      const gchar *signal_name;

      children_type = picman_container_get_children_type (private->container);
      signal_name   = picman_context_type_to_signal_name (children_type);

      if (signal_name)
        {
          picman_context_set_by_type (private->context, children_type,
                                    PICMAN_OBJECT (viewable));
          return TRUE;
        }
    }

  success = picman_container_view_select_item (view, viewable);

#if 0
  if (success && private->container && private->context)
    {
      PicmanContext *context;
      GType        children_type;

      /*  ref and remember the context because private->context may
       *  become NULL by calling picman_context_set_by_type()
       */
      context       = g_object_ref (private->context);
      children_type = picman_container_get_children_type (private->container);

      g_signal_handlers_block_by_func (context,
                                       picman_container_view_context_changed,
                                       view);

      picman_context_set_by_type (context, children_type, PICMAN_OBJECT (viewable));

      g_signal_handlers_unblock_by_func (context,
                                         picman_container_view_context_changed,
                                         view);

      g_object_unref (context);
    }
#endif

  return success;
}

gboolean
picman_container_view_multi_selected (PicmanContainerView *view,
                                    GList             *items)
{
  guint                     selected_count;
  gboolean                  success = FALSE;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), FALSE);

  selected_count = g_list_length (items);

  if (selected_count == 0)
    {
      /* do nothing */
    }
  else if (selected_count == 1)
    {
      success = picman_container_view_item_selected (view, items->data);
    }
  else
    {
      success = FALSE;
      g_signal_emit (view, view_signals[SELECT_ITEM], 0,
                     NULL, items, &success);
    }

  return success;
}

gint
picman_container_view_get_selected (PicmanContainerView  *view,
                                  GList             **list)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (view), 0);

  return PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->get_selected (view, list);
}

static gint
picman_container_view_real_get_selected (PicmanContainerView    *view,
                                       GList               **list)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  GType                     children_type;
  PicmanObject               *object;

  if (list)
    *list = NULL;

  if (! private->container || ! private->context)
    return 0;

  children_type = picman_container_get_children_type (private->container);
  object = picman_context_get_by_type (private->context,
                                     children_type);

  if (list && object)
    *list = g_list_append (*list, object);

  return object ? 1 : 0;
}

void
picman_container_view_item_activated (PicmanContainerView *view,
                                    PicmanViewable      *viewable)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  picman_container_view_activate_item (view, viewable);
}

void
picman_container_view_item_context (PicmanContainerView *view,
                                  PicmanViewable      *viewable)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_VIEW (view));
  g_return_if_fail (PICMAN_IS_VIEWABLE (viewable));

  picman_container_view_context_item (view, viewable);
}

void
picman_container_view_set_property (GObject      *object,
                                  guint         property_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (object);

  switch (property_id)
    {
    case PICMAN_CONTAINER_VIEW_PROP_CONTAINER:
      picman_container_view_set_container (view, g_value_get_object (value));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_CONTEXT:
      picman_container_view_set_context (view, g_value_get_object (value));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_SELECTION_MODE:
      picman_container_view_set_selection_mode (view, g_value_get_enum (value));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_REORDERABLE:
      picman_container_view_set_reorderable (view, g_value_get_boolean (value));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE:
    case PICMAN_CONTAINER_VIEW_PROP_VIEW_BORDER_WIDTH:
      {
        gint size, border;

        size = picman_container_view_get_view_size (view, &border);

        if (property_id == PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE)
          size = g_value_get_int (value);
        else
          border = g_value_get_int (value);

        picman_container_view_set_view_size (view, size, border);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

void
picman_container_view_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (object);

  switch (property_id)
    {
    case PICMAN_CONTAINER_VIEW_PROP_CONTAINER:
      g_value_set_object (value, picman_container_view_get_container (view));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_CONTEXT:
      g_value_set_object (value, picman_container_view_get_context (view));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_SELECTION_MODE:
      g_value_set_enum (value, picman_container_view_get_selection_mode (view));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_REORDERABLE:
      g_value_set_boolean (value, picman_container_view_get_reorderable (view));
      break;
    case PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE:
    case PICMAN_CONTAINER_VIEW_PROP_VIEW_BORDER_WIDTH:
      {
        gint size, border;

        size = picman_container_view_get_view_size (view, &border);

        if (property_id == PICMAN_CONTAINER_VIEW_PROP_VIEW_SIZE)
          g_value_set_int (value, size);
        else
          g_value_set_int (value, border);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_container_view_clear_items (PicmanContainerView *view)
{
  PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->clear_items (view);
}

static void
picman_container_view_real_clear_items (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  g_hash_table_remove_all (private->item_hash);
}

static void
picman_container_view_add_container (PicmanContainerView *view,
                                   PicmanContainer     *container)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  picman_container_foreach (container,
                          (GFunc) picman_container_view_add_foreach,
                          view);

  if (container == private->container)
    {
      GType              children_type;
      PicmanViewableClass *viewable_class;

      children_type  = picman_container_get_children_type (container);
      viewable_class = g_type_class_ref (children_type);

      private->name_changed_handler =
        picman_tree_handler_connect (container,
                                   viewable_class->name_changed_signal,
                                   G_CALLBACK (picman_container_view_name_changed),
                                   view);

      g_type_class_unref (viewable_class);
    }

  g_signal_connect_object (container, "add",
                           G_CALLBACK (picman_container_view_add),
                           view,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (container, "remove",
                           G_CALLBACK (picman_container_view_remove),
                           view,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (container, "reorder",
                           G_CALLBACK (picman_container_view_reorder),
                           view,
                           G_CONNECT_SWAPPED);
}

static void
picman_container_view_add_foreach (PicmanViewable      *viewable,
                                 PicmanContainerView *view)
{
  PicmanContainerViewInterface *view_iface;
  PicmanContainerViewPrivate   *private;
  PicmanViewable               *parent;
  PicmanContainer              *children;
  gpointer                    parent_insert_data = NULL;
  gpointer                    insert_data;

  view_iface = PICMAN_CONTAINER_VIEW_GET_INTERFACE (view);
  private    = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  parent = picman_viewable_get_parent (viewable);

  if (parent)
    parent_insert_data = g_hash_table_lookup (private->item_hash, parent);

  insert_data = view_iface->insert_item (view, viewable,
                                         parent_insert_data, -1);

  g_hash_table_insert (private->item_hash, viewable, insert_data);

  if (view_iface->insert_item_after)
    view_iface->insert_item_after (view, viewable, insert_data);

  children = picman_viewable_get_children (viewable);

  if (children)
    picman_container_view_add_container (view, children);
}

static void
picman_container_view_add (PicmanContainerView *view,
                         PicmanViewable      *viewable,
                         PicmanContainer     *container)
{
  PicmanContainerViewInterface *view_iface;
  PicmanContainerViewPrivate   *private;
  PicmanViewable               *parent;
  PicmanContainer              *children;
  gpointer                    parent_insert_data = NULL;
  gpointer                    insert_data;
  gint                        index;

  view_iface = PICMAN_CONTAINER_VIEW_GET_INTERFACE (view);
  private    = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  index = picman_container_get_child_index (container,
                                          PICMAN_OBJECT (viewable));

  parent = picman_viewable_get_parent (viewable);

  if (parent)
    parent_insert_data = g_hash_table_lookup (private->item_hash, parent);

  insert_data = view_iface->insert_item (view, viewable,
                                         parent_insert_data, index);

  g_hash_table_insert (private->item_hash, viewable, insert_data);

  if (view_iface->insert_item_after)
    view_iface->insert_item_after (view, viewable, insert_data);

  children = picman_viewable_get_children (viewable);

  if (children)
    picman_container_view_add_container (view, children);
}

static void
picman_container_view_remove_container (PicmanContainerView *view,
                                      PicmanContainer     *container)
{
  PicmanContainerViewInterface *view_iface;
  PicmanContainerViewPrivate   *private;

  view_iface = PICMAN_CONTAINER_VIEW_GET_INTERFACE (view);
  private    = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (container == private->container)
    {
      picman_tree_handler_disconnect (private->name_changed_handler);
      private->name_changed_handler = NULL;
    }

  g_signal_handlers_disconnect_by_func (container,
                                        picman_container_view_add,
                                        view);
  g_signal_handlers_disconnect_by_func (container,
                                        picman_container_view_remove,
                                        view);
  g_signal_handlers_disconnect_by_func (container,
                                        picman_container_view_reorder,
                                        view);

  if (! view_iface->model_is_tree && container == private->container)
    {
      picman_container_view_clear_items (view);
    }
  else
    {
      picman_container_foreach (container,
                              (GFunc) picman_container_view_remove_foreach,
                              view);
    }
}

static void
picman_container_view_remove_foreach (PicmanViewable      *viewable,
                                    PicmanContainerView *view)
{
  picman_container_view_remove (view, viewable, NULL);
}

static void
picman_container_view_remove (PicmanContainerView *view,
                            PicmanViewable      *viewable,
                            PicmanContainer     *unused)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  PicmanContainer            *children;
  gpointer                  insert_data;

  children = picman_viewable_get_children (viewable);

  if (children)
    picman_container_view_remove_container (view, children);

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  if (insert_data)
    {
      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->remove_item (view,
                                                             viewable,
                                                             insert_data);

      g_hash_table_remove (private->item_hash, viewable);
    }
}

static void
picman_container_view_reorder (PicmanContainerView *view,
                             PicmanViewable      *viewable,
                             gint               new_index,
                             PicmanContainer     *container)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  gpointer                  insert_data;

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  if (insert_data)
    {
      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->reorder_item (view,
                                                              viewable,
                                                              new_index,
                                                              insert_data);
    }
}

static void
picman_container_view_freeze (PicmanContainerView *view,
                            PicmanContainer     *container)
{
  picman_container_view_remove_container (view, container);
}

static void
picman_container_view_thaw (PicmanContainerView *view,
                          PicmanContainer     *container)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  picman_container_view_add_container (view, container);

  if (private->context)
    {
      GType        children_type;
      const gchar *signal_name;

      children_type = picman_container_get_children_type (private->container);
      signal_name   = picman_context_type_to_signal_name (children_type);

      if (signal_name)
        {
          PicmanObject *object;

          object = picman_context_get_by_type (private->context, children_type);

          picman_container_view_select_item (view, PICMAN_VIEWABLE (object));
        }
    }
}

static void
picman_container_view_name_changed (PicmanViewable      *viewable,
                                  PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  gpointer                  insert_data;

  insert_data = g_hash_table_lookup (private->item_hash, viewable);

  if (insert_data)
    {
      PICMAN_CONTAINER_VIEW_GET_INTERFACE (view)->rename_item (view,
                                                             viewable,
                                                             insert_data);
    }
}

static void
picman_container_view_connect_context (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  GType                     children_type;
  const gchar              *signal_name;

  children_type = picman_container_get_children_type (private->container);
  signal_name   = picman_context_type_to_signal_name (children_type);

  if (signal_name)
    {
      g_signal_connect_object (private->context, signal_name,
                               G_CALLBACK (picman_container_view_context_changed),
                               view,
                               0);

      if (private->dnd_widget)
        picman_dnd_viewable_dest_add (private->dnd_widget,
                                    children_type,
                                    picman_container_view_viewable_dropped,
                                    view);

      if (! picman_container_frozen (private->container))
        {
          PicmanObject *object = picman_context_get_by_type (private->context,
                                                         children_type);

          picman_container_view_select_item (view, PICMAN_VIEWABLE (object));
        }
    }
}

static void
picman_container_view_disconnect_context (PicmanContainerView *view)
{
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);
  GType                     children_type;
  const gchar              *signal_name;

  children_type = picman_container_get_children_type (private->container);
  signal_name   = picman_context_type_to_signal_name (children_type);

  if (signal_name)
    {
      g_signal_handlers_disconnect_by_func (private->context,
                                            picman_container_view_context_changed,
                                            view);

      if (private->dnd_widget)
        {
          gtk_drag_dest_unset (private->dnd_widget);
          picman_dnd_viewable_dest_remove (private->dnd_widget,
                                         children_type);
        }
    }
}

static void
picman_container_view_context_changed (PicmanContext       *context,
                                     PicmanViewable      *viewable,
                                     PicmanContainerView *view)
{
  if (! picman_container_view_select_item (view, viewable))
    g_warning ("%s: select_item() failed (should not happen)", G_STRFUNC);
}

static void
picman_container_view_viewable_dropped (GtkWidget    *widget,
                                      gint          x,
                                      gint          y,
                                      PicmanViewable *viewable,
                                      gpointer      data)
{
  PicmanContainerView        *view    = PICMAN_CONTAINER_VIEW (data);
  PicmanContainerViewPrivate *private = PICMAN_CONTAINER_VIEW_GET_PRIVATE (view);

  if (viewable && private->container &&
      picman_container_have (private->container, PICMAN_OBJECT (viewable)))
    {
      picman_container_view_item_selected (view, viewable);
    }
}

static void
picman_container_view_button_viewable_dropped (GtkWidget    *widget,
                                             gint          x,
                                             gint          y,
                                             PicmanViewable *viewable,
                                             gpointer      data)
{
  PicmanContainerView *view = PICMAN_CONTAINER_VIEW (data);

  if (viewable && picman_container_view_lookup (view, viewable))
    {
      picman_container_view_item_selected (view, viewable);

      gtk_button_clicked (GTK_BUTTON (widget));
    }
}

