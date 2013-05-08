/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansettingseditor.c
 * Copyright (C) 2008-2011 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanlist.h"
#include "core/picmanviewable.h"

#include "picmancontainertreestore.h"
#include "picmancontainertreeview.h"
#include "picmancontainerview.h"
#include "picmansettingseditor.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_CONFIG,
  PROP_CONTAINER,
  PROP_FILENAME
};


typedef struct _PicmanSettingsEditorPrivate PicmanSettingsEditorPrivate;

struct _PicmanSettingsEditorPrivate
{
  Picman          *picman;
  GObject       *config;
  PicmanContainer *container;
  GObject       *selected_setting;

  GtkWidget     *view;
  GtkWidget     *import_button;
  GtkWidget     *export_button;
  GtkWidget     *delete_button;
};

#define GET_PRIVATE(item) G_TYPE_INSTANCE_GET_PRIVATE (item, \
                                                       PICMAN_TYPE_SETTINGS_EDITOR, \
                                                       PicmanSettingsEditorPrivate)


static void   picman_settings_editor_constructed    (GObject             *object);
static void   picman_settings_editor_finalize       (GObject             *object);
static void   picman_settings_editor_set_property   (GObject             *object,
                                                   guint                property_id,
                                                   const GValue        *value,
                                                   GParamSpec          *pspec);
static void   picman_settings_editor_get_property   (GObject             *object,
                                                   guint                property_id,
                                                   GValue              *value,
                                                   GParamSpec          *pspec);

static gboolean
          picman_settings_editor_row_separator_func (GtkTreeModel        *model,
                                                   GtkTreeIter         *iter,
                                                   gpointer             data);
static void   picman_settings_editor_select_item    (PicmanContainerView   *view,
                                                   PicmanViewable        *viewable,
                                                   gpointer             insert_data,
                                                   PicmanSettingsEditor  *editor);
static void   picman_settings_editor_import_clicked (GtkWidget           *widget,
                                                   PicmanSettingsEditor  *editor);
static void   picman_settings_editor_export_clicked (GtkWidget           *widget,
                                                   PicmanSettingsEditor  *editor);
static void   picman_settings_editor_delete_clicked (GtkWidget           *widget,
                                                   PicmanSettingsEditor  *editor);
static void   picman_settings_editor_name_edited    (GtkCellRendererText *cell,
                                                   const gchar         *path_str,
                                                   const gchar         *new_name,
                                                   PicmanSettingsEditor  *editor);


G_DEFINE_TYPE (PicmanSettingsEditor, picman_settings_editor, GTK_TYPE_BOX)

#define parent_class picman_settings_editor_parent_class


static void
picman_settings_editor_class_init (PicmanSettingsEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_settings_editor_constructed;
  object_class->finalize     = picman_settings_editor_finalize;
  object_class->set_property = picman_settings_editor_set_property;
  object_class->get_property = picman_settings_editor_get_property;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONFIG,
                                   g_param_spec_object ("config",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONFIG,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_CONTAINER,
                                   g_param_spec_object ("container",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanSettingsEditorPrivate));
}

static void
picman_settings_editor_init (PicmanSettingsEditor *editor)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (editor),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (editor), 6);
}

static void
picman_settings_editor_constructed (GObject *object)
{
  PicmanSettingsEditor        *editor  = PICMAN_SETTINGS_EDITOR (object);
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (object);
  PicmanContainerTreeView     *tree_view;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_PICMAN (private->picman));
  g_assert (PICMAN_IS_CONFIG (private->config));
  g_assert (PICMAN_IS_CONTAINER (private->container));

  private->view = picman_container_tree_view_new (private->container,
                                                picman_get_user_context (private->picman),
                                               16, 0);
  gtk_widget_set_size_request (private->view, 200, 200);
  gtk_box_pack_start (GTK_BOX (editor), private->view, TRUE, TRUE, 0);
  gtk_widget_show (private->view);

  tree_view = PICMAN_CONTAINER_TREE_VIEW (private->view);

  gtk_tree_view_set_row_separator_func (tree_view->view,
                                        picman_settings_editor_row_separator_func,
                                        private->view, NULL);

  g_signal_connect (tree_view, "select-item",
                    G_CALLBACK (picman_settings_editor_select_item),
                    editor);

  picman_container_tree_view_connect_name_edited (tree_view,
                                                G_CALLBACK (picman_settings_editor_name_edited),
                                                editor);

  private->import_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view),
                            GTK_STOCK_OPEN,
                            _("Import settings from a file"),
                            NULL,
                            G_CALLBACK (picman_settings_editor_import_clicked),
                            NULL,
                            editor);

  private->export_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view),
                            GTK_STOCK_SAVE,
                            _("Export the selected settings to a file"),
                            NULL,
                            G_CALLBACK (picman_settings_editor_export_clicked),
                            NULL,
                            editor);

  private->delete_button =
    picman_editor_add_button (PICMAN_EDITOR (tree_view),
                            GTK_STOCK_DELETE,
                            _("Delete the selected settings"),
                            NULL,
                            G_CALLBACK (picman_settings_editor_delete_clicked),
                            NULL,
                            editor);

  gtk_widget_set_sensitive (private->delete_button, FALSE);
}

static void
picman_settings_editor_finalize (GObject *object)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (object);

  if (private->config)
    {
      g_object_unref (private->config);
      private->config = NULL;
    }

  if (private->container)
    {
      g_object_unref (private->container);
      private->container = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_settings_editor_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      private->picman = g_value_get_object (value); /* don't dup */
      break;

    case PROP_CONFIG:
      private->config = g_value_dup_object (value);
      break;

    case PROP_CONTAINER:
      private->container = g_value_dup_object (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_settings_editor_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, private->picman);
      break;

    case PROP_CONFIG:
      g_value_set_object (value, private->config);
      break;

    case PROP_CONTAINER:
      g_value_set_object (value, private->container);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_settings_editor_row_separator_func (GtkTreeModel *model,
                                         GtkTreeIter  *iter,
                                         gpointer      data)
{
  gchar *name = NULL;

  gtk_tree_model_get (model, iter,
                      PICMAN_CONTAINER_TREE_STORE_COLUMN_NAME, &name,
                      -1);
  g_free (name);

  return name == NULL;
}

static void
picman_settings_editor_select_item (PicmanContainerView  *view,
                                  PicmanViewable       *viewable,
                                  gpointer            insert_data,
                                  PicmanSettingsEditor *editor)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (editor);
  gboolean                   sensitive;

  private->selected_setting = G_OBJECT (viewable);

  sensitive = (private->selected_setting != NULL &&
               picman_object_get_name (private->selected_setting));

  gtk_widget_set_sensitive (private->export_button, sensitive);
  gtk_widget_set_sensitive (private->delete_button, sensitive);
}

static void
picman_settings_editor_import_clicked (GtkWidget          *widget,
                                     PicmanSettingsEditor *editor)
{
}

static void
picman_settings_editor_export_clicked (GtkWidget          *widget,
                                     PicmanSettingsEditor *editor)
{
}

static void
picman_settings_editor_delete_clicked (GtkWidget          *widget,
                                     PicmanSettingsEditor *editor)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (editor);

  if (private->selected_setting)
    {
      PicmanObject *new;

      new = picman_container_get_neighbor_of (private->container,
                                            PICMAN_OBJECT (private->selected_setting));

      /*  don't select the separator  */
      if (new && ! picman_object_get_name (new))
        new = NULL;

      picman_container_remove (private->container,
                             PICMAN_OBJECT (private->selected_setting));

      picman_container_view_select_item (PICMAN_CONTAINER_VIEW (private->view),
                                       PICMAN_VIEWABLE (new));
    }
}

static void
picman_settings_editor_name_edited (GtkCellRendererText *cell,
                                  const gchar         *path_str,
                                  const gchar         *new_name,
                                  PicmanSettingsEditor  *editor)
{
  PicmanSettingsEditorPrivate *private = GET_PRIVATE (editor);
  PicmanContainerTreeView     *tree_view;
  GtkTreePath               *path;
  GtkTreeIter                iter;

  tree_view = PICMAN_CONTAINER_TREE_VIEW (private->view);

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (tree_view->model, &iter, path))
    {
      PicmanViewRenderer *renderer;
      PicmanObject       *object;
      const gchar      *old_name;
      gchar            *name;

      gtk_tree_model_get (tree_view->model, &iter,
                          PICMAN_CONTAINER_TREE_STORE_COLUMN_RENDERER, &renderer,
                          -1);

      object = PICMAN_OBJECT (renderer->viewable);

      old_name = picman_object_get_name (object);

      if (! old_name) old_name = "";
      if (! new_name) new_name = "";

      name = g_strstrip (g_strdup (new_name));

      if (strlen (name) && strcmp (old_name, name))
        {
          guint t;

          g_object_get (object, "time", &t, NULL);

          if (t > 0)
            g_object_set (object, "time", 0, NULL);

          /*  set name after time so the object is reordered correctly  */
          picman_object_take_name (object, name);
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


/*  public functions  */

GtkWidget *
picman_settings_editor_new (Picman          *picman,
                          GObject       *config,
                          PicmanContainer *container)
{
  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_CONFIG (config), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);

  return g_object_new (PICMAN_TYPE_SETTINGS_EDITOR,
                       "picman",      picman,
                       "config",    config,
                       "container", container,
                       NULL);
}
