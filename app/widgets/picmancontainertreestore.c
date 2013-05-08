/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancontainertreestore.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmanviewable.h"

#include "picmancellrendererviewable.h"
#include "picmancontainertreestore.h"
#include "picmancontainerview.h"
#include "picmanviewrenderer.h"


enum
{
  PROP_0,
  PROP_CONTAINER_VIEW,
  PROP_USE_NAME
};


typedef struct _PicmanContainerTreeStorePrivate PicmanContainerTreeStorePrivate;

struct _PicmanContainerTreeStorePrivate
{
  PicmanContainerView *container_view;
  GList             *renderer_cells;
  gboolean           use_name;
};

#define GET_PRIVATE(store) \
        G_TYPE_INSTANCE_GET_PRIVATE (store, \
                                     PICMAN_TYPE_CONTAINER_TREE_STORE, \
                                     PicmanContainerTreeStorePrivate)


static void   picman_container_tree_store_constructed     (GObject                *object);
static void   picman_container_tree_store_finalize        (GObject                *object);
static void   picman_container_tree_store_set_property    (GObject                *object,
                                                         guint                   property_id,
                                                         const GValue           *value,
                                                         GParamSpec             *pspec);
static void   picman_container_tree_store_get_property    (GObject                *object,
                                                         guint                   property_id,
                                                         GValue                 *value,
                                                         GParamSpec             *pspec);

static void   picman_container_tree_store_set             (PicmanContainerTreeStore *store,
                                                         GtkTreeIter            *iter,
                                                         PicmanViewable           *viewable);
static void   picman_container_tree_store_renderer_update (PicmanViewRenderer       *renderer,
                                                         PicmanContainerTreeStore *store);


G_DEFINE_TYPE (PicmanContainerTreeStore, picman_container_tree_store,
               GTK_TYPE_TREE_STORE)

#define parent_class picman_container_tree_store_parent_class


static void
picman_container_tree_store_class_init (PicmanContainerTreeStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_container_tree_store_constructed;
  object_class->finalize     = picman_container_tree_store_finalize;
  object_class->set_property = picman_container_tree_store_set_property;
  object_class->get_property = picman_container_tree_store_get_property;

  g_object_class_install_property (object_class, PROP_CONTAINER_VIEW,
                                   g_param_spec_object ("container-view",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER_VIEW,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_USE_NAME,
                                   g_param_spec_boolean ("use-name",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanContainerTreeStorePrivate));
}

static void
picman_container_tree_store_init (PicmanContainerTreeStore *store)
{
}

static void
picman_container_tree_store_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);
}

static void
picman_container_tree_store_finalize (GObject *object)
{
  PicmanContainerTreeStorePrivate *private = GET_PRIVATE (object);

  if (private->renderer_cells)
    {
      g_list_free (private->renderer_cells);
      private->renderer_cells = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_container_tree_store_set_property (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  PicmanContainerTreeStorePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_CONTAINER_VIEW:
      private->container_view = g_value_get_object (value); /* don't ref */
      break;
    case PROP_USE_NAME:
      private->use_name = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_container_tree_store_get_property (GObject    *object,
                                        guint       property_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  PicmanContainerTreeStorePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_CONTAINER_VIEW:
      g_value_set_object (value, private->container_view);
      break;
    case PROP_USE_NAME:
      g_value_set_boolean (value, private->use_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/*  public functions  */

GtkTreeModel *
picman_container_tree_store_new (PicmanContainerView *container_view,
                               gint               n_columns,
                               GType             *types)
{
  PicmanContainerTreeStore *store;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_VIEW (container_view), NULL);
  g_return_val_if_fail (n_columns >= PICMAN_CONTAINER_TREE_STORE_N_COLUMNS, NULL);
  g_return_val_if_fail (types != NULL, NULL);

  store = g_object_new (PICMAN_TYPE_CONTAINER_TREE_STORE,
                        "container-view", container_view,
                        NULL);

  gtk_tree_store_set_column_types (GTK_TREE_STORE (store), n_columns, types);

  return GTK_TREE_MODEL (store);
}

void
picman_container_tree_store_add_renderer_cell (PicmanContainerTreeStore *store,
                                             GtkCellRenderer        *cell)
{
  PicmanContainerTreeStorePrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));
  g_return_if_fail (PICMAN_IS_CELL_RENDERER_VIEWABLE (cell));

  private = GET_PRIVATE (store);

  private->renderer_cells = g_list_prepend (private->renderer_cells, cell);
}

void
picman_container_tree_store_set_use_name (PicmanContainerTreeStore *store,
                                        gboolean                use_name)
{
  PicmanContainerTreeStorePrivate *private;

  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));

  private = GET_PRIVATE (store);

  if (private->use_name != use_name)
    {
      private->use_name = use_name ? TRUE : FALSE;
      g_object_notify (G_OBJECT (store), "use-name");
    }
}

gboolean
picman_container_tree_store_get_use_name (PicmanContainerTreeStore *store)
{
  g_return_val_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store), FALSE);

  return GET_PRIVATE (store)->use_name;
}

static gboolean
picman_container_tree_store_set_context_foreach (GtkTreeModel *model,
                                               GtkTreePath  *path,
                                               GtkTreeIter  *iter,
                                               gpointer      data)
{
  PicmanContext      *context = data;
  PicmanViewRenderer *renderer;

  gtk_tree_model_get (model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                      -1);

  picman_view_renderer_set_context (renderer, context);

  g_object_unref (renderer);

  return FALSE;
}

void
picman_container_tree_store_set_context (PicmanContainerTreeStore *store,
                                       PicmanContext            *context)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));

  gtk_tree_model_foreach (GTK_TREE_MODEL (store),
                          picman_container_tree_store_set_context_foreach,
                          context);
}

GtkTreeIter *
picman_container_tree_store_insert_item (PicmanContainerTreeStore *store,
                                       PicmanViewable           *viewable,
                                       GtkTreeIter            *parent,
                                       gint                    index)
{
  GtkTreeIter iter;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store), NULL);

  if (index == -1)
    gtk_tree_store_append (GTK_TREE_STORE (store), &iter, parent);
  else
    gtk_tree_store_insert (GTK_TREE_STORE (store), &iter, parent, index);

  picman_container_tree_store_set (store, &iter, viewable);

  return gtk_tree_iter_copy (&iter);
}

void
picman_container_tree_store_remove_item (PicmanContainerTreeStore *store,
                                       PicmanViewable           *viewable,
                                       GtkTreeIter            *iter)
{
  if (iter)
    {
      gtk_tree_store_remove (GTK_TREE_STORE (store), iter);

      /*  If the store is empty after this remove, clear out renderers
       *  from all cells so they don't keep refing the viewables
       *  (see bug #149906).
       */
      if (! gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL))
        {
          PicmanContainerTreeStorePrivate *private = GET_PRIVATE (store);
          GList                         *list;

          for (list = private->renderer_cells; list; list = list->next)
            g_object_set (list->data, "renderer", NULL, NULL);
        }
    }
}

void
picman_container_tree_store_reorder_item (PicmanContainerTreeStore *store,
                                        PicmanViewable           *viewable,
                                        gint                    new_index,
                                        GtkTreeIter            *iter)
{
  PicmanContainerTreeStorePrivate *private;
  PicmanViewable                  *parent;
  PicmanContainer                 *container;

  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));

  private = GET_PRIVATE (store);

  if (! iter)
    return;

  parent = picman_viewable_get_parent (viewable);

  if (parent)
    container = picman_viewable_get_children (parent);
  else
    container = picman_container_view_get_container (private->container_view);

  if (new_index == -1 ||
      new_index == picman_container_get_n_children (container) - 1)
    {
      gtk_tree_store_move_before (GTK_TREE_STORE (store), iter, NULL);
    }
  else if (new_index == 0)
    {
      gtk_tree_store_move_after (GTK_TREE_STORE (store), iter, NULL);
    }
  else
    {
      GtkTreePath *path;
      GtkTreeIter  place_iter;
      gint         depth;
      gint        *indices;
      gint         old_index;

      path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
      indices = gtk_tree_path_get_indices (path);

      depth = gtk_tree_path_get_depth (path);

      old_index = indices[depth - 1];

      if (new_index != old_index)
        {
          indices[depth - 1] = new_index;

          gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &place_iter, path);

          if (new_index > old_index)
            gtk_tree_store_move_after (GTK_TREE_STORE (store),
                                       iter, &place_iter);
          else
            gtk_tree_store_move_before (GTK_TREE_STORE (store),
                                        iter, &place_iter);
        }

      gtk_tree_path_free (path);
    }
}

gboolean
picman_container_tree_store_rename_item (PicmanContainerTreeStore *store,
                                       PicmanViewable           *viewable,
                                       GtkTreeIter            *iter)
{
  gboolean new_name_shorter = FALSE;

  g_return_val_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store), FALSE);

  if (iter)
    {
      PicmanContainerTreeStorePrivate *private = GET_PRIVATE (store);
      gchar                         *name;
      gchar                         *old_name;

      if (private->use_name)
        name = (gchar *) picman_object_get_name (viewable);
      else
        name = picman_viewable_get_description (viewable, NULL);

      gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, &old_name,
                          -1);

      gtk_tree_store_set (GTK_TREE_STORE (store), iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, name,
                          -1);

      if (name && old_name && strlen (name) < strlen (old_name))
        new_name_shorter = TRUE;

      if (! private->use_name)
        g_free (name);

      g_free (old_name);
    }

  return new_name_shorter;
}

void
picman_container_tree_store_clear_items (PicmanContainerTreeStore *store)
{
  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));

  gtk_tree_store_clear (GTK_TREE_STORE (store));

  /*  If the store is empty after this remove, clear out renderers
   *  from all cells so they don't keep refing the viewables
   *  (see bug #149906).
   */
  if (! gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), NULL))
    {
      PicmanContainerTreeStorePrivate *private = GET_PRIVATE (store);
      GList                         *list;

      for (list = private->renderer_cells; list; list = list->next)
        g_object_set (list->data, "renderer", NULL, NULL);
    }
}

typedef struct
{
  gint view_size;
  gint border_width;
} SetSizeForeachData;

static gboolean
picman_container_tree_store_set_view_size_foreach (GtkTreeModel *model,
                                                 GtkTreePath  *path,
                                                 GtkTreeIter  *iter,
                                                 gpointer      data)
{
  SetSizeForeachData *size_data = data;
  PicmanViewRenderer   *renderer;

  gtk_tree_model_get (model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                      -1);

  picman_view_renderer_set_size (renderer,
                               size_data->view_size,
                               size_data->border_width);

  g_object_unref (renderer);

  return FALSE;
}

void
picman_container_tree_store_set_view_size (PicmanContainerTreeStore *store)
{
  PicmanContainerTreeStorePrivate *private;
  SetSizeForeachData             size_data;

  g_return_if_fail (PICMAN_IS_CONTAINER_TREE_STORE (store));

  private = GET_PRIVATE (store);

  size_data.view_size =
    picman_container_view_get_view_size (private->container_view,
                                       &size_data.border_width);

  gtk_tree_model_foreach (GTK_TREE_MODEL (store),
                          picman_container_tree_store_set_view_size_foreach,
                          &size_data);
}


/*  private functions  */

void
picman_container_tree_store_columns_init (GType *types,
                                        gint  *n_types)
{
  g_return_if_fail (types != NULL);
  g_return_if_fail (n_types != NULL);
  g_return_if_fail (*n_types == 0);

  g_assert (PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER ==
            picman_container_tree_store_columns_add (types, n_types,
                                                   PICMAN_TYPE_VIEW_RENDERER));

  g_assert (PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME ==
            picman_container_tree_store_columns_add (types, n_types,
                                                   G_TYPE_STRING));

  g_assert (PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_ATTRIBUTES ==
            picman_container_tree_store_columns_add (types, n_types,
                                                   PANGO_TYPE_ATTR_LIST));

  g_assert (PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE ==
            picman_container_tree_store_columns_add (types, n_types,
                                                   G_TYPE_BOOLEAN));

  g_assert (PICMAN_CONTAINER_TREE_STORE_COLUMN_USER_DATA ==
            picman_container_tree_store_columns_add (types, n_types,
                                                   G_TYPE_POINTER));
}

gint
picman_container_tree_store_columns_add (GType *types,
                                       gint  *n_types,
                                       GType  type)
{
  g_return_val_if_fail (types != NULL, 0);
  g_return_val_if_fail (n_types != NULL, 0);
  g_return_val_if_fail (*n_types >= 0, 0);

  types[*n_types] = type;
  (*n_types)++;

  return *n_types - 1;
}

static void
picman_container_tree_store_set (PicmanContainerTreeStore *store,
                               GtkTreeIter            *iter,
                               PicmanViewable           *viewable)
{
  PicmanContainerTreeStorePrivate *private = GET_PRIVATE (store);
  PicmanContext                   *context;
  PicmanViewRenderer              *renderer;
  gchar                         *name;
  gint                           view_size;
  gint                           border_width;

  context = picman_container_view_get_context (private->container_view);

  view_size = picman_container_view_get_view_size (private->container_view,
                                                 &border_width);

  renderer = picman_view_renderer_new (context,
                                     G_TYPE_FROM_INSTANCE (viewable),
                                     view_size, border_width,
                                     FALSE);
  picman_view_renderer_set_viewable (renderer, viewable);
  picman_view_renderer_remove_idle (renderer);

  g_signal_connect (renderer, "update",
                    G_CALLBACK (picman_container_tree_store_renderer_update),
                    store);

  if (private->use_name)
    name = (gchar *) picman_object_get_name (viewable);
  else
    name = picman_viewable_get_description (viewable, NULL);

  gtk_tree_store_set (GTK_TREE_STORE (store), iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER,       renderer,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME,           name,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME_SENSITIVE, TRUE,
                      -1);

  if (! private->use_name)
    g_free (name);

  g_object_unref (renderer);
}

static void
picman_container_tree_store_renderer_update (PicmanViewRenderer       *renderer,
                                           PicmanContainerTreeStore *store)
{
  PicmanContainerTreeStorePrivate *private = GET_PRIVATE (store);
  GtkTreeIter                   *iter;

  iter = picman_container_view_lookup (private->container_view,
                                     renderer->viewable);

  if (iter)
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), iter);
      gtk_tree_model_row_changed (GTK_TREE_MODEL (store), path, iter);
      gtk_tree_path_free (path);
    }
}
