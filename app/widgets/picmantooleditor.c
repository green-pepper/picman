/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantooleditor.c
 * Copyright (C) 2001-2009 Michael Natterer <mitch@picman.org>
 *                         Stephen Griffiths <scgmk5@gmail.com>
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

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmantoolinfo.h"

#include "picmancontainertreestore.h"
#include "picmancontainerview.h"
#include "picmanviewrenderer.h"
#include "picmantooleditor.h"
#include "picmanhelp-ids.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


typedef struct _PicmanToolEditorPrivate PicmanToolEditorPrivate;

struct _PicmanToolEditorPrivate
{
  GtkTreeModel  *model;
  PicmanContext   *context;
  PicmanContainer *container;
  GtkWidget     *scrolled;

  GtkWidget     *raise_button;
  GtkWidget     *lower_button;
  GtkWidget     *reset_button;

  /* State of tools at creation of the editor, stored to support
   * reverting changes
   */
  gchar        **initial_tool_order;
  gboolean      *initial_tool_visibility;
  gint           n_tools;

  GQuark         visible_handler_id;
  GList         *default_tool_order;
};


static void   picman_tool_editor_dispose     (GObject               *object);
static void   picman_tool_editor_finalize    (GObject               *object);

static void   picman_tool_editor_visible_notify
                                           (PicmanToolInfo          *tool_info,
                                            GParamSpec            *pspec,
                                            PicmanToolEditor        *tool_editor);
static void   picman_tool_editor_eye_data_func
                                           (GtkTreeViewColumn     *tree_column,
                                            GtkCellRenderer       *cell,
                                            GtkTreeModel          *tree_model,
                                            GtkTreeIter           *iter,
                                            gpointer               data);
static void   picman_tool_editor_eye_clicked (GtkCellRendererToggle *toggle,
                                            gchar                 *path_str,
                                            GdkModifierType        state,
                                            PicmanToolEditor        *tool_editor);

static void   picman_tool_editor_raise_clicked
                                           (GtkButton             *button,
                                            PicmanToolEditor        *tool_editor);
static void   picman_tool_editor_raise_extend_clicked
                                           (GtkButton             *button,
                                            GdkModifierType        mask,
                                            PicmanToolEditor        *tool_editor);
static void   picman_tool_editor_lower_clicked
                                           (GtkButton             *button,
                                            PicmanToolEditor        *tool_editor);
static void   picman_tool_editor_lower_extend_clicked
                                           (GtkButton             *button,
                                            GdkModifierType        mask,
                                            PicmanToolEditor        *tool_editor);
static void   picman_tool_editor_reset_clicked
                                           (GtkButton             *button,
                                            PicmanToolEditor        *tool_editor);


G_DEFINE_TYPE (PicmanToolEditor, picman_tool_editor, PICMAN_TYPE_CONTAINER_TREE_VIEW)

#define parent_class picman_tool_editor_parent_class

#define PICMAN_TOOL_EDITOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                           PICMAN_TYPE_TOOL_EDITOR, \
                                           PicmanToolEditorPrivate))


static void
picman_tool_editor_class_init (PicmanToolEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose  = picman_tool_editor_dispose;
  object_class->finalize = picman_tool_editor_finalize;

  g_type_class_add_private (klass, sizeof (PicmanToolEditorPrivate));
}

static void
picman_tool_editor_init (PicmanToolEditor *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);

  priv->model                   = NULL;
  priv->context                 = NULL;
  priv->container               = NULL;
  priv->scrolled                = NULL;

  priv->visible_handler_id      = 0;
  priv->default_tool_order      = NULL;

  priv->initial_tool_order      = NULL;
  priv->initial_tool_visibility = NULL;
  priv->n_tools                 = 0;

  priv->raise_button            = NULL;
  priv->lower_button            = NULL;
  priv->reset_button            = NULL;
}

static void
picman_tool_editor_dispose (GObject *object)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (object);

  if (priv->visible_handler_id)
    {
      picman_container_remove_handler (priv->container,
                                     priv->visible_handler_id);
      priv->visible_handler_id = 0;
    }

  priv->context      = NULL;
  priv->container    = NULL;

  priv->raise_button = NULL;
  priv->lower_button = NULL;
  priv->reset_button = NULL;

  priv->scrolled     = NULL;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_tool_editor_finalize (GObject *object)
{
  PicmanToolEditor *tool_editor;
  PicmanToolEditorPrivate *priv;

  tool_editor = PICMAN_TOOL_EDITOR (object);
  priv        = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);

  if (priv->initial_tool_order)
    {
      int i;

      for (i = 0; i < priv->n_tools; i++)
        {
          g_free (priv->initial_tool_order[i]);
        }

      g_free (priv->initial_tool_order);
      priv->initial_tool_order      = NULL;
    }

  if (priv->initial_tool_visibility)
    {
      g_slice_free1 (sizeof (gboolean) * priv->n_tools,
                     priv->initial_tool_visibility);

      priv->initial_tool_visibility = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
picman_tool_editor_new (PicmanContainer *container,
                      PicmanContext   *context,
                      GList         *default_tool_order,
                      gint           view_size,
                      gint           view_border_width)
{
  int                    i;
  PicmanToolEditor        *tool_editor;
  PicmanContainerTreeView *tree_view;
  PicmanContainerView     *container_view;
  GObject               *object;
  PicmanObject            *picman_object;
  PicmanToolEditorPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  object         = g_object_new (PICMAN_TYPE_TOOL_EDITOR, NULL);
  tool_editor    = PICMAN_TOOL_EDITOR (object);
  tree_view      = PICMAN_CONTAINER_TREE_VIEW (object);
  container_view = PICMAN_CONTAINER_VIEW (object);
  priv           = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);

  priv->container               = container;
  priv->context                 = context;
  priv->model                   = tree_view->model;
  priv->default_tool_order      = default_tool_order;
  priv->initial_tool_order      = picman_container_get_name_array (container,
                                                                 &priv->n_tools);
  priv->initial_tool_visibility = g_slice_alloc (sizeof (gboolean) *
                                                 priv->n_tools);
  for (i = 0; i < priv->n_tools; i++)
    {
      picman_object = picman_container_get_child_by_index (container, i);

      g_object_get (picman_object,
                    "visible", &(priv->initial_tool_visibility[i]), NULL);
    }

  picman_container_view_set_view_size (container_view,
                                     view_size, view_border_width);
  picman_container_view_set_container (container_view, priv->container);
  picman_container_view_set_context (container_view, context);
  picman_container_view_set_reorderable (container_view, TRUE);
  picman_editor_set_show_name (PICMAN_EDITOR (tree_view), FALSE);

  /* Construct tree view */
  {
    PicmanContainerTreeView *tree_view   = PICMAN_CONTAINER_TREE_VIEW (tool_editor);
    GtkWidget             *tree_widget = GTK_WIDGET (tree_view);
    GtkStyle              *tree_style  = gtk_widget_get_style (tree_widget);
    GtkTreeViewColumn     *column;
    GtkCellRenderer       *eye_cell;
    GtkIconSize            icon_size;

    column    = gtk_tree_view_column_new ();
    gtk_tree_view_insert_column (tree_view->view, column, 0);
    eye_cell  = picman_cell_renderer_toggle_new (PICMAN_STOCK_VISIBLE);
    icon_size = picman_get_icon_size (GTK_WIDGET (tool_editor),
                                    PICMAN_STOCK_VISIBLE,
                                    GTK_ICON_SIZE_BUTTON,
                                    view_size -
                                    2 * tree_style->xthickness,
                                    view_size -
                                    2 * tree_style->ythickness);

    g_object_set (eye_cell, "stock-size", icon_size, NULL);
    gtk_tree_view_column_pack_start (column, eye_cell, FALSE);
    gtk_tree_view_column_set_cell_data_func  (column, eye_cell,
                                              picman_tool_editor_eye_data_func,
                                              tree_view, NULL);

    picman_container_tree_view_add_toggle_cell (tree_view, eye_cell);

    g_signal_connect (eye_cell, "clicked",
                      G_CALLBACK (picman_tool_editor_eye_clicked),
                      tool_editor);

    priv->visible_handler_id =
      picman_container_add_handler (container, "notify::visible",
                                  G_CALLBACK (picman_tool_editor_visible_notify),
                                  tool_editor);
  }

  /* buttons */
  priv->raise_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view), GTK_STOCK_GO_UP,
                            _("Raise this tool"),
                            _("Raise this tool to the top"),
                            G_CALLBACK (picman_tool_editor_raise_clicked),
                            G_CALLBACK (picman_tool_editor_raise_extend_clicked),
                            tool_editor);

  priv->lower_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view), GTK_STOCK_GO_DOWN,
                            _("Lower this tool"),
                            _("Lower this tool to the bottom"),
                            G_CALLBACK (picman_tool_editor_lower_clicked),
                            G_CALLBACK (picman_tool_editor_lower_extend_clicked),
                            tool_editor);

  priv->reset_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view), PICMAN_STOCK_RESET,
                            _("Reset tool order and visibility"), NULL,
                            G_CALLBACK (picman_tool_editor_reset_clicked), NULL,
                            tool_editor);

  return GTK_WIDGET (tool_editor);
}

/**
 * picman_tool_editor_revert_changes:
 * @tool_editor:
 *
 * Reverts the tool order and visibility to the state at creation.
 **/
void
picman_tool_editor_revert_changes (PicmanToolEditor *tool_editor)
{
  int i;
  PicmanToolEditorPrivate *priv;

  priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);

  for (i = 0; i < priv->n_tools; i++)
    {
      PicmanObject *object;

      object = picman_container_get_child_by_name (priv->container,
                                                 priv->initial_tool_order[i]);

      picman_container_reorder (priv->container, object, i);
      g_object_set (object, "visible", priv->initial_tool_visibility[i], NULL);
    }
}

static void
picman_tool_editor_raise_clicked (GtkButton    *button,
                                PicmanToolEditor *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  PicmanToolInfo          *tool_info;

  tool_info = picman_context_get_tool (priv->context);

  if (tool_info)
    {
      gint index = picman_container_get_child_index (priv->container,
                                                   PICMAN_OBJECT (tool_info));

      if (index > 0)
        {
          picman_container_reorder (priv->container,
                                  PICMAN_OBJECT (tool_info), index - 1);
        }
    }
}

static void
picman_tool_editor_raise_extend_clicked (GtkButton       *button,
                                       GdkModifierType  mask,
                                       PicmanToolEditor    *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  PicmanToolInfo          *tool_info;

  tool_info = picman_context_get_tool (priv->context);

  if (tool_info && (mask & GDK_SHIFT_MASK))
    {
      gint index = picman_container_get_child_index (priv->container,
                                                   PICMAN_OBJECT (tool_info));

      if (index > 0)
        {
          picman_container_reorder (priv->container,
                                  PICMAN_OBJECT (tool_info), 0);
        }
    }
}

static void
picman_tool_editor_lower_clicked (GtkButton    *button,
                                PicmanToolEditor *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  PicmanToolInfo          *tool_info;

  tool_info = picman_context_get_tool (priv->context);

  if (tool_info)
    {
      gint index = picman_container_get_child_index (priv->container,
                                                   PICMAN_OBJECT (tool_info));

      if (index + 1 < picman_container_get_n_children (priv->container))
        {
          picman_container_reorder (priv->container,
                                  PICMAN_OBJECT (tool_info), index + 1);
        }
    }
}

static void
picman_tool_editor_lower_extend_clicked (GtkButton       *button,
                                       GdkModifierType  mask,
                                       PicmanToolEditor    *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  PicmanToolInfo          *tool_info;

  tool_info = picman_context_get_tool (priv->context);

  if (tool_info && (mask & GDK_SHIFT_MASK))
    {
      gint index = picman_container_get_n_children (priv->container) - 1;

      index = MAX (index, 0);

      picman_container_reorder (priv->container,
                              PICMAN_OBJECT (tool_info), index);
    }
}

static void
picman_tool_editor_reset_clicked (GtkButton    *button,
                                PicmanToolEditor *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  GList                 *list;
  gint                   i;

  for (list = priv->default_tool_order, i = 0;
       list;
       list = g_list_next (list), i++)
    {
      PicmanObject *object =
        picman_container_get_child_by_name (priv->container, list->data);

      if (object)
        {
          gboolean visible;
          gpointer data;

          picman_container_reorder (priv->container, object, i);
          data = g_object_get_data (G_OBJECT (object),
                                    "picman-tool-default-visible");

          visible = GPOINTER_TO_INT (data);
          g_object_set (object, "visible", visible, NULL);
        }
    }
}

static void
picman_tool_editor_visible_notify (PicmanToolInfo  *tool_info,
                                 GParamSpec    *pspec,
                                 PicmanToolEditor  *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  GtkTreeIter           *iter;

  iter = picman_container_view_lookup (PICMAN_CONTAINER_VIEW (tool_editor),
                                     PICMAN_VIEWABLE (tool_info));

  if (iter)
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (priv->model, iter);

      gtk_tree_model_row_changed (priv->model, path, iter);

      gtk_tree_path_free (path);
    }
}

static void
picman_tool_editor_eye_data_func (GtkTreeViewColumn *tree_column,
                                GtkCellRenderer   *cell,
                                GtkTreeModel      *tree_model,
                                GtkTreeIter       *iter,
                                gpointer           data)
{
  PicmanViewRenderer *renderer;
  gboolean          visible;

  gtk_tree_model_get (tree_model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                      -1);

  g_object_get (renderer->viewable, "visible", &visible, NULL);

  g_object_unref (renderer);

  g_object_set (cell, "active", visible, NULL);
}

static void
picman_tool_editor_eye_clicked (GtkCellRendererToggle *toggle,
                              gchar                 *path_str,
                              GdkModifierType        state,
                              PicmanToolEditor        *tool_editor)
{
  PicmanToolEditorPrivate *priv = PICMAN_TOOL_EDITOR_GET_PRIVATE (tool_editor);
  GtkTreePath           *path;
  GtkTreeIter            iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (priv->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      gboolean          active;

      g_object_get (toggle,
                    "active", &active,
                    NULL);
      gtk_tree_model_get (priv->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      g_object_set (renderer->viewable, "visible", ! active, NULL);

      g_object_unref (renderer);
    }

  gtk_tree_path_free (path);
}
