/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandatafactoryview.c
 * Copyright (C) 2001-2003 Michael Natterer <mitch@picman.org>
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

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandata.h"
#include "core/picmandatafactory.h"
#include "core/picmanlist.h"
#include "core/picmanmarshal.h"
#include "core/picmantaggedcontainer.h"

#include "picmancombotagentry.h"
#include "picmancontainertreestore.h"
#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmandatafactoryview.h"
#include "picmandnd.h"
#include "picmanmenufactory.h"
#include "picmantagentry.h"
#include "picmanuimanager.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_DATA_FACTORY,
  PROP_ACTION_GROUP
};


struct _PicmanDataFactoryViewPriv
{
  PicmanDataFactory *factory;
  gchar           *action_group;

  PicmanContainer   *tagged_container;
  GtkWidget       *query_tag_entry;
  GtkWidget       *assign_tag_entry;
  GList           *selected_items;

  GtkWidget       *edit_button;
  GtkWidget       *new_button;
  GtkWidget       *duplicate_button;
  GtkWidget       *delete_button;
  GtkWidget       *refresh_button;
};


static GObject *
              picman_data_factory_view_constructor    (GType                type,
                                                     guint                n_construct_params,
                                                     GObjectConstructParam *construct_params);
static void   picman_data_factory_view_constructed    (GObject             *object);
static void   picman_data_factory_view_dispose        (GObject             *object);
static void   picman_data_factory_view_set_property   (GObject             *object,
                                                     guint                property_id,
                                                     const GValue        *value,
                                                     GParamSpec          *pspec);
static void   picman_data_factory_view_get_property   (GObject             *object,
                                                     guint                property_id,
                                                     GValue              *value,
                                                     GParamSpec          *pspec);

static void   picman_data_factory_view_activate_item  (PicmanContainerEditor *editor,
                                                     PicmanViewable        *viewable);
static void   picman_data_factory_view_select_item    (PicmanContainerEditor *editor,
                                                     PicmanViewable        *viewable);
static void picman_data_factory_view_tree_name_edited (GtkCellRendererText *cell,
                                                     const gchar         *path,
                                                     const gchar         *name,
                                                     PicmanDataFactoryView *view);


G_DEFINE_TYPE (PicmanDataFactoryView, picman_data_factory_view,
               PICMAN_TYPE_CONTAINER_EDITOR)

#define parent_class picman_data_factory_view_parent_class


static void
picman_data_factory_view_class_init (PicmanDataFactoryViewClass *klass)
{
  GObjectClass             *object_class = G_OBJECT_CLASS (klass);
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  object_class->constructor   = picman_data_factory_view_constructor;
  object_class->constructed   = picman_data_factory_view_constructed;
  object_class->dispose       = picman_data_factory_view_dispose;
  object_class->set_property  = picman_data_factory_view_set_property;
  object_class->get_property  = picman_data_factory_view_get_property;

  editor_class->select_item   = picman_data_factory_view_select_item;
  editor_class->activate_item = picman_data_factory_view_activate_item;

  g_object_class_install_property (object_class, PROP_DATA_FACTORY,
                                   g_param_spec_object ("data-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DATA_FACTORY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_ACTION_GROUP,
                                   g_param_spec_string ("action-group",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanDataFactoryViewPriv));
}

static void
picman_data_factory_view_init (PicmanDataFactoryView *view)
{
  view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view,
                                            PICMAN_TYPE_DATA_FACTORY_VIEW,
                                            PicmanDataFactoryViewPriv);

  view->priv->tagged_container = NULL;
  view->priv->query_tag_entry  = NULL;
  view->priv->assign_tag_entry = NULL;
  view->priv->selected_items   = NULL;
  view->priv->edit_button      = NULL;
  view->priv->new_button       = NULL;
  view->priv->duplicate_button = NULL;
  view->priv->delete_button    = NULL;
  view->priv->refresh_button   = NULL;
}

static GObject *
picman_data_factory_view_constructor (GType                  type,
                                    guint                  n_construct_params,
                                    GObjectConstructParam *construct_params)
{
  PicmanDataFactoryView *factory_view;
  GObject             *object;

  object = G_OBJECT_CLASS (parent_class)->constructor (type,
                                                       n_construct_params,
                                                       construct_params);

  factory_view = PICMAN_DATA_FACTORY_VIEW (object);

  g_assert (PICMAN_IS_DATA_FACTORY (factory_view->priv->factory));
  g_assert (factory_view->priv->action_group != NULL);

  factory_view->priv->tagged_container =
    picman_tagged_container_new (picman_data_factory_get_container (factory_view->priv->factory));

  /* this must happen in constructor(), because doing it in
   * set_property() warns about wrong construct property usage
   */
  g_object_set (object,
                "container", factory_view->priv->tagged_container,
                NULL);

  return object;
}

static void
picman_data_factory_view_constructed (GObject *object)
{
  PicmanDataFactoryView     *factory_view = PICMAN_DATA_FACTORY_VIEW (object);
  PicmanDataFactoryViewPriv *priv         = factory_view->priv;
  PicmanContainerEditor     *editor       = PICMAN_CONTAINER_EDITOR (object);
  gchar                   *str;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman_container_editor_set_selection_mode (editor, GTK_SELECTION_MULTIPLE);

  if (PICMAN_IS_CONTAINER_TREE_VIEW (editor->view))
    {
      PicmanContainerTreeView *tree_view;

      tree_view = PICMAN_CONTAINER_TREE_VIEW (editor->view);

      picman_container_tree_view_connect_name_edited (tree_view,
                                                    G_CALLBACK (picman_data_factory_view_tree_name_edited),
                                                    factory_view);
    }

  str = g_strdup_printf ("%s-edit", priv->action_group);
  priv->edit_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view),
                                   priv->action_group,
                                   str, NULL);
  g_free (str);

  if (picman_data_factory_view_has_data_new_func (factory_view))
    {
      str = g_strdup_printf ("%s-new", priv->action_group);
      priv->new_button =
        picman_editor_add_action_button (PICMAN_EDITOR (editor->view),
                                       priv->action_group,
                                       str, NULL);
      g_free (str);
    }

  str = g_strdup_printf ("%s-duplicate", priv->action_group);
  priv->duplicate_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view),
                                   priv->action_group,
                                   str, NULL);
  g_free (str);

  str = g_strdup_printf ("%s-delete", priv->action_group);
  priv->delete_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view),
                                   priv->action_group,
                                   str, NULL);
  g_free (str);

  str = g_strdup_printf ("%s-refresh", priv->action_group);
  priv->refresh_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view),
                                   priv->action_group,
                                   str, NULL);
  g_free (str);

  /* Query tag entry */
  priv->query_tag_entry =
    picman_combo_tag_entry_new (PICMAN_TAGGED_CONTAINER (priv->tagged_container),
                              PICMAN_TAG_ENTRY_MODE_QUERY);
  gtk_box_pack_start (GTK_BOX (editor->view),
                      priv->query_tag_entry,
                      FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (editor->view),
                         priv->query_tag_entry, 0);
  gtk_widget_show (priv->query_tag_entry);

  /* Assign tag entry */
  priv->assign_tag_entry =
    picman_combo_tag_entry_new (PICMAN_TAGGED_CONTAINER (priv->tagged_container),
                              PICMAN_TAG_ENTRY_MODE_ASSIGN);
  picman_tag_entry_set_selected_items (PICMAN_TAG_ENTRY (priv->assign_tag_entry),
                                     priv->selected_items);
  g_list_free (priv->selected_items);
  priv->selected_items = NULL;
  gtk_box_pack_start (GTK_BOX (editor->view),
                      priv->assign_tag_entry,
                      FALSE, FALSE, 0);
  gtk_widget_show (priv->assign_tag_entry);

  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (priv->edit_button),
                                  picman_data_factory_get_data_type (priv->factory));
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (priv->duplicate_button),
                                  picman_data_factory_get_data_type (priv->factory));
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (priv->delete_button),
                                  picman_data_factory_get_data_type (priv->factory));

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                          editor);
}

static void
picman_data_factory_view_dispose (GObject *object)
{
  PicmanDataFactoryView *factory_view = PICMAN_DATA_FACTORY_VIEW (object);

  if (factory_view->priv->tagged_container)
    {
      g_object_unref (factory_view->priv->tagged_container);
      factory_view->priv->tagged_container = NULL;
    }

  if (factory_view->priv->factory)
    {
      g_object_unref (factory_view->priv->factory);
      factory_view->priv->factory = NULL;
    }

  if (factory_view->priv->action_group)
    {
      g_free (factory_view->priv->action_group);
      factory_view->priv->action_group = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_data_factory_view_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  PicmanDataFactoryView *factory_view = PICMAN_DATA_FACTORY_VIEW (object);

  switch (property_id)
    {
    case PROP_DATA_FACTORY:
      factory_view->priv->factory = g_value_dup_object (value);
      break;

    case PROP_ACTION_GROUP:
      factory_view->priv->action_group = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_data_factory_view_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  PicmanDataFactoryView *factory_view = PICMAN_DATA_FACTORY_VIEW (object);

  switch (property_id)
    {
    case PROP_DATA_FACTORY:
      g_value_set_object (value, factory_view->priv->factory);
      break;

    case PROP_ACTION_GROUP:
      g_value_set_string (value, factory_view->priv->action_group);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

GtkWidget *
picman_data_factory_view_new (PicmanViewType      view_type,
                            PicmanDataFactory  *factory,
                            PicmanContext      *context,
                            gint              view_size,
                            gint              view_border_width,
                            PicmanMenuFactory  *menu_factory,
                            const gchar      *menu_identifier,
                            const gchar      *ui_path,
                            const gchar      *action_group)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (action_group != NULL, NULL);

  return g_object_new (PICMAN_TYPE_DATA_FACTORY_VIEW,
                       "view-type",         view_type,
                       "data-factory",      factory,
                       "context",           context,
                       "view-size",         view_size,
                       "view-border-width", view_border_width,
                       "menu-factory",      menu_factory,
                       "menu-identifier",   menu_identifier,
                       "ui-path",           ui_path,
                       "action-group",      action_group,
                       NULL);
}

GtkWidget *
picman_data_factory_view_get_edit_button (PicmanDataFactoryView *factory_view)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), NULL);

  return factory_view->priv->edit_button;
}

GtkWidget *
picman_data_factory_view_get_duplicate_button (PicmanDataFactoryView *factory_view)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), NULL);

  return factory_view->priv->duplicate_button;
}

PicmanDataFactory *
picman_data_factory_view_get_data_factory (PicmanDataFactoryView *factory_view)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), NULL);

  return factory_view->priv->factory;
}

GType
picman_data_factory_view_get_children_type (PicmanDataFactoryView *factory_view)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), G_TYPE_NONE);

  return picman_data_factory_get_data_type (factory_view->priv->factory);
}

gboolean
picman_data_factory_view_has_data_new_func (PicmanDataFactoryView *factory_view)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), FALSE);

  return picman_data_factory_has_data_new_func (factory_view->priv->factory);
}

gboolean
picman_data_factory_view_have (PicmanDataFactoryView *factory_view,
                             PicmanObject          *object)
{
  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY_VIEW (factory_view), FALSE);

  return picman_container_have (picman_data_factory_get_container (factory_view->priv->factory),
                              object);
}

static void
picman_data_factory_view_select_item (PicmanContainerEditor *editor,
                                    PicmanViewable        *viewable)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (editor);

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->select_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->select_item (editor, viewable);

  if (view->priv->assign_tag_entry)
    {
      PicmanContainerView *container_view = PICMAN_CONTAINER_VIEW (editor->view);
      GList             *active_items   = NULL;

      picman_container_view_get_selected (container_view, &active_items);
      picman_tag_entry_set_selected_items (PICMAN_TAG_ENTRY (view->priv->assign_tag_entry),
                                         active_items);
      g_list_free (active_items);
    }
  else if (viewable)
    {
      view->priv->selected_items = g_list_append (view->priv->selected_items,
                                                  viewable);
    }
}

static void
picman_data_factory_view_activate_item (PicmanContainerEditor *editor,
                                      PicmanViewable        *viewable)
{
  PicmanDataFactoryView *view = PICMAN_DATA_FACTORY_VIEW (editor);
  PicmanData            *data = PICMAN_DATA (viewable);

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item (editor, viewable);

  if (data && picman_data_factory_view_have (view,
                                           PICMAN_OBJECT (data)))
    {
      if (view->priv->edit_button &&
          gtk_widget_is_sensitive (view->priv->edit_button))
        gtk_button_clicked (GTK_BUTTON (view->priv->edit_button));
    }
}

static void
picman_data_factory_view_tree_name_edited (GtkCellRendererText *cell,
                                         const gchar         *path_str,
                                         const gchar         *new_name,
                                         PicmanDataFactoryView *view)
{
  PicmanContainerTreeView *tree_view;
  GtkTreePath           *path;
  GtkTreeIter            iter;

  tree_view = PICMAN_CONTAINER_TREE_VIEW (PICMAN_CONTAINER_EDITOR (view)->view);

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      PicmanData         *data;
      gchar            *name;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      data = PICMAN_DATA (renderer->viewable);

      if (! new_name)
        new_name = "";

      name = g_strstrip (g_strdup (new_name));

      if (picman_data_is_writable (data) && strlen (name))
        {
          picman_object_take_name (PICMAN_OBJECT (data), name);
        }
      else
        {
          g_free (name);

          name = picman_viewable_get_description (renderer->viewable, NULL);
          gtk_tree_store_set (GTK_TREE_STORE (tree_view->model), &iter,
                              PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, name,
                              -1);
          g_free (name);
        }

      g_object_unref (renderer);
    }

  gtk_tree_path_free (path);
}
