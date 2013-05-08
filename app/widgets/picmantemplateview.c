/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantemplateview.c
 * Copyright (C) 2003 Michael Natterer <mitch@picman.org>
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

#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmantemplate.h"

#include "picmancontainertreestore.h"
#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmanmenufactory.h"
#include "picmantemplateview.h"
#include "picmandnd.h"
#include "picmanhelp-ids.h"
#include "picmanviewrenderer.h"
#include "picmanuimanager.h"

#include "picman-intl.h"


static void picman_template_view_activate_item    (PicmanContainerEditor *editor,
                                                 PicmanViewable        *viewable);
static void picman_template_view_tree_name_edited (GtkCellRendererText *cell,
                                                 const gchar         *path_str,
                                                 const gchar         *new_name,
                                                 PicmanTemplateView    *view);


G_DEFINE_TYPE (PicmanTemplateView, picman_template_view,
               PICMAN_TYPE_CONTAINER_EDITOR);

#define parent_class picman_template_view_parent_class


static void
picman_template_view_class_init (PicmanTemplateViewClass *klass)
{
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  editor_class->activate_item = picman_template_view_activate_item;
}

static void
picman_template_view_init (PicmanTemplateView *view)
{
  view->create_button    = NULL;
  view->new_button       = NULL;
  view->duplicate_button = NULL;
  view->edit_button      = NULL;
  view->delete_button    = NULL;
}

GtkWidget *
picman_template_view_new (PicmanViewType     view_type,
                        PicmanContainer   *container,
                        PicmanContext     *context,
                        gint             view_size,
                        gint             view_border_width,
                        PicmanMenuFactory *menu_factory)
{
  PicmanTemplateView    *template_view;
  PicmanContainerEditor *editor;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  template_view = g_object_new (PICMAN_TYPE_TEMPLATE_VIEW,
                                "view-type",         view_type,
                                "container",         container,
                                "context",           context,
                                "view-size",         view_size,
                                "view-border-width", view_border_width,
                                "menu-factory",      menu_factory,
                                "menu-identifier",   "<Templates>",
                                "ui-path",           "/templates-popup",
                                NULL);

  editor = PICMAN_CONTAINER_EDITOR (template_view);

  if (PICMAN_IS_CONTAINER_TREE_VIEW (editor->view))
    {
      PicmanContainerTreeView *tree_view;

      tree_view = PICMAN_CONTAINER_TREE_VIEW (editor->view);

      picman_container_tree_view_connect_name_edited (tree_view,
                                                    G_CALLBACK (picman_template_view_tree_name_edited),
                                                    template_view);
    }

  template_view->create_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "templates",
                                   "templates-create-image", NULL);

  template_view->new_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "templates",
                                   "templates-new", NULL);

  template_view->duplicate_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "templates",
                                   "templates-duplicate", NULL);

  template_view->edit_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "templates",
                                   "templates-edit", NULL);

  template_view->delete_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "templates",
                                   "templates-delete", NULL);

  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (template_view->create_button),
                                  PICMAN_TYPE_TEMPLATE);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (template_view->duplicate_button),
                                  PICMAN_TYPE_TEMPLATE);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (template_view->edit_button),
                                  PICMAN_TYPE_TEMPLATE);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (template_view->delete_button),
                                  PICMAN_TYPE_TEMPLATE);

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                          editor);

  return GTK_WIDGET (template_view);
}

static void
picman_template_view_activate_item (PicmanContainerEditor *editor,
                                  PicmanViewable        *viewable)
{
  PicmanTemplateView *view = PICMAN_TEMPLATE_VIEW (editor);
  PicmanContainer    *container;

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item (editor, viewable);

  container = picman_container_view_get_container (editor->view);

  if (viewable && picman_container_have (container, PICMAN_OBJECT (viewable)))
    {
      gtk_button_clicked (GTK_BUTTON (view->create_button));
    }
}

static void
picman_template_view_tree_name_edited (GtkCellRendererText *cell,
                                     const gchar         *path_str,
                                     const gchar         *new_name,
                                     PicmanTemplateView    *view)
{
  PicmanContainerTreeView *tree_view;
  GtkTreePath           *path;
  GtkTreeIter            iter;

  tree_view = PICMAN_CONTAINER_TREE_VIEW (PICMAN_CONTAINER_EDITOR (view)->view);

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      PicmanObject       *object;
      const gchar      *old_name;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      object = PICMAN_OBJECT (renderer->viewable);

      old_name = picman_object_get_name (object);

      if (! old_name) old_name = "";
      if (! new_name) new_name = "";

      if (strcmp (old_name, new_name))
        {
          picman_object_set_name (object, new_name);
        }
      else
        {
          gchar *name = picman_viewable_get_description (renderer->viewable,
                                                       NULL);

          gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), &iter,
                              PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, name,
                              -1);
          g_free (name);
        }

      g_object_unref (renderer);
    }

  gtk_tree_path_free (path);
}
