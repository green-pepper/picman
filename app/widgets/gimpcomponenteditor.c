/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmancomponenteditor.c
 * Copyright (C) 2003-2005 Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanchannel.h"
#include "core/picmanimage.h"

#include "picmancellrendererviewable.h"
#include "picmancomponenteditor.h"
#include "picmandnd.h"
#include "picmandocked.h"
#include "picmanmenufactory.h"
#include "picmanviewrendererimage.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


enum
{
  COLUMN_CHANNEL,
  COLUMN_VISIBLE,
  COLUMN_RENDERER,
  COLUMN_NAME,
  N_COLUMNS
};


static void picman_component_editor_docked_iface_init (PicmanDockedInterface *iface);

static void picman_component_editor_set_context       (PicmanDocked          *docked,
                                                     PicmanContext         *context);

static void picman_component_editor_set_image         (PicmanImageEditor     *editor,
                                                     PicmanImage           *image);

static void picman_component_editor_create_components (PicmanComponentEditor *editor);
static void picman_component_editor_clear_components  (PicmanComponentEditor *editor);
static void picman_component_editor_clicked         (GtkCellRendererToggle *cellrenderertoggle,
                                                   gchar                 *path,
                                                   GdkModifierType        state,
                                                   PicmanComponentEditor   *editor);
static gboolean picman_component_editor_select        (GtkTreeSelection    *selection,
                                                     GtkTreeModel        *model,
                                                     GtkTreePath         *path,
                                                     gboolean             path_currently_selected,
                                                     gpointer             data);
static gboolean picman_component_editor_button_press  (GtkWidget           *widget,
                                                     GdkEventButton      *bevent,
                                                     PicmanComponentEditor *editor);
static void picman_component_editor_renderer_update   (PicmanViewRenderer    *renderer,
                                                     PicmanComponentEditor *editor);
static void picman_component_editor_mode_changed      (PicmanImage           *image,
                                                     PicmanComponentEditor *editor);
static void picman_component_editor_alpha_changed     (PicmanImage           *image,
                                                     PicmanComponentEditor *editor);
static void picman_component_editor_visibility_changed(PicmanImage           *image,
                                                     PicmanChannelType      channel,
                                                     PicmanComponentEditor *editor);
static void picman_component_editor_active_changed    (PicmanImage           *image,
                                                     PicmanChannelType      channel,
                                                     PicmanComponentEditor *editor);
static PicmanImage * picman_component_editor_drag_component (GtkWidget       *widget,
                                                         PicmanContext    **context,
                                                         PicmanChannelType *channel,
                                                         gpointer         data);


G_DEFINE_TYPE_WITH_CODE (PicmanComponentEditor, picman_component_editor,
                         PICMAN_TYPE_IMAGE_EDITOR,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCKED,
                                                picman_component_editor_docked_iface_init))

#define parent_class picman_component_editor_parent_class

static PicmanDockedInterface *parent_docked_iface = NULL;


static void
picman_component_editor_class_init (PicmanComponentEditorClass *klass)
{
  PicmanImageEditorClass *image_editor_class = PICMAN_IMAGE_EDITOR_CLASS (klass);

  image_editor_class->set_image = picman_component_editor_set_image;
}

static void
picman_component_editor_init (PicmanComponentEditor *editor)
{
  GtkWidget    *frame;
  GtkListStore *list;

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (editor), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  list = gtk_list_store_new (N_COLUMNS,
                             G_TYPE_INT,
                             G_TYPE_BOOLEAN,
                             PICMAN_TYPE_VIEW_RENDERER,
                             G_TYPE_STRING);
  editor->model = GTK_TREE_MODEL (list);

  editor->view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (editor->model));
  g_object_unref (list);

  gtk_tree_view_set_headers_visible (editor->view, FALSE);

  editor->eye_column = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (editor->view, editor->eye_column);

  editor->eye_cell = picman_cell_renderer_toggle_new (PICMAN_STOCK_VISIBLE);
  gtk_tree_view_column_pack_start (editor->eye_column, editor->eye_cell,
                                   FALSE);
  gtk_tree_view_column_set_attributes (editor->eye_column, editor->eye_cell,
                                       "active", COLUMN_VISIBLE,
                                       NULL);

  g_signal_connect (editor->eye_cell, "clicked",
                    G_CALLBACK (picman_component_editor_clicked),
                    editor);

  editor->renderer_cell = picman_cell_renderer_viewable_new ();
  gtk_tree_view_insert_column_with_attributes (editor->view,
                                               -1, NULL,
                                               editor->renderer_cell,
                                               "renderer", COLUMN_RENDERER,
                                               NULL);

  gtk_tree_view_insert_column_with_attributes (editor->view,
                                               -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", COLUMN_NAME,
                                               NULL);

  gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (editor->view));
  gtk_widget_show (GTK_WIDGET (editor->view));

  g_signal_connect (editor->view, "button-press-event",
                    G_CALLBACK (picman_component_editor_button_press),
                    editor);

  editor->selection = gtk_tree_view_get_selection (editor->view);
  gtk_tree_selection_set_mode (editor->selection, GTK_SELECTION_MULTIPLE);

  gtk_tree_selection_set_select_function (editor->selection,
                                          picman_component_editor_select,
                                          editor, NULL);

  picman_dnd_component_source_add (GTK_WIDGET (editor->view),
                                 picman_component_editor_drag_component,
                                 editor);
}

static void
picman_component_editor_docked_iface_init (PicmanDockedInterface *iface)
{
  parent_docked_iface = g_type_interface_peek_parent (iface);

  if (! parent_docked_iface)
    parent_docked_iface = g_type_default_interface_peek (PICMAN_TYPE_DOCKED);

  iface->set_context = picman_component_editor_set_context;
}

static void
picman_component_editor_set_context (PicmanDocked  *docked,
                                   PicmanContext *context)
{
  PicmanComponentEditor *editor = PICMAN_COMPONENT_EDITOR (docked);
  GtkTreeIter          iter;
  gboolean             iter_valid;

  parent_docked_iface->set_context (docked, context);

  for (iter_valid = gtk_tree_model_get_iter_first (editor->model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (editor->model, &iter))
    {
      PicmanViewRenderer *renderer;

      gtk_tree_model_get (editor->model, &iter,
                          COLUMN_RENDERER, &renderer,
                          -1);

      picman_view_renderer_set_context (renderer, context);
      g_object_unref (renderer);
    }
}

static void
picman_component_editor_set_image (PicmanImageEditor *editor,
                                 PicmanImage       *image)
{
  PicmanComponentEditor *component_editor = PICMAN_COMPONENT_EDITOR (editor);

  if (editor->image)
    {
      picman_component_editor_clear_components (component_editor);

      g_signal_handlers_disconnect_by_func (editor->image,
                                            picman_component_editor_mode_changed,
                                            component_editor);
      g_signal_handlers_disconnect_by_func (editor->image,
                                            picman_component_editor_alpha_changed,
                                            component_editor);
      g_signal_handlers_disconnect_by_func (editor->image,
                                            picman_component_editor_visibility_changed,
                                            component_editor);
      g_signal_handlers_disconnect_by_func (editor->image,
                                            picman_component_editor_active_changed,
                                            component_editor);
    }

  PICMAN_IMAGE_EDITOR_CLASS (parent_class)->set_image (editor, image);

  if (editor->image)
    {
      picman_component_editor_create_components (component_editor);

      g_signal_connect (editor->image, "mode-changed",
                        G_CALLBACK (picman_component_editor_mode_changed),
                        component_editor);
      g_signal_connect (editor->image, "alpha-changed",
                        G_CALLBACK (picman_component_editor_alpha_changed),
                        component_editor);
      g_signal_connect (editor->image, "component-visibility-changed",
                        G_CALLBACK (picman_component_editor_visibility_changed),
                        component_editor);
      g_signal_connect (editor->image, "component-active-changed",
                        G_CALLBACK (picman_component_editor_active_changed),
                        component_editor);
    }
}

GtkWidget *
picman_component_editor_new (gint             view_size,
                           PicmanMenuFactory *menu_factory)
{
  PicmanComponentEditor *editor;

  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  editor = g_object_new (PICMAN_TYPE_COMPONENT_EDITOR,
                         "menu-factory",    menu_factory,
                         "menu-identifier", "<Channels>",
                         "ui-path",         "/channels-popup",
                         NULL);

  picman_component_editor_set_view_size (editor, view_size);

  return GTK_WIDGET (editor);
}

void
picman_component_editor_set_view_size (PicmanComponentEditor *editor,
                                     gint                 view_size)
{
  GtkWidget   *tree_widget;
  GtkStyle    *tree_style;
  GtkIconSize  icon_size;
  GtkTreeIter  iter;
  gboolean     iter_valid;

  g_return_if_fail (PICMAN_IS_COMPONENT_EDITOR (editor));
  g_return_if_fail (view_size >  0 &&
                    view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE);

  tree_widget = GTK_WIDGET (editor->view);
  tree_style  = gtk_widget_get_style (tree_widget);

  icon_size = picman_get_icon_size (tree_widget,
                                  PICMAN_STOCK_VISIBLE,
                                  GTK_ICON_SIZE_BUTTON,
                                  view_size -
                                  2 * tree_style->xthickness,
                                  view_size -
                                  2 * tree_style->ythickness);

  g_object_set (editor->eye_cell,
                "stock-size", icon_size,
                NULL);

  for (iter_valid = gtk_tree_model_get_iter_first (editor->model, &iter);
       iter_valid;
       iter_valid = gtk_tree_model_iter_next (editor->model, &iter))
    {
      PicmanViewRenderer *renderer;

      gtk_tree_model_get (editor->model, &iter,
                          COLUMN_RENDERER, &renderer,
                          -1);

      picman_view_renderer_set_size (renderer, view_size, 1);
      g_object_unref (renderer);
    }

  editor->view_size = view_size;

  gtk_tree_view_columns_autosize (editor->view);
}

static void
picman_component_editor_create_components (PicmanComponentEditor *editor)
{
  PicmanImage       *image        = PICMAN_IMAGE_EDITOR (editor)->image;
  gint             n_components = 0;
  PicmanChannelType  components[MAX_CHANNELS];
  GEnumClass      *enum_class;
  gint             i;

  switch (picman_image_get_base_type (image))
    {
    case PICMAN_RGB:
      n_components  = 3;
      components[0] = PICMAN_RED_CHANNEL;
      components[1] = PICMAN_GREEN_CHANNEL;
      components[2] = PICMAN_BLUE_CHANNEL;
      break;

    case PICMAN_GRAY:
      n_components  = 1;
      components[0] = PICMAN_GRAY_CHANNEL;
      break;

    case PICMAN_INDEXED:
      n_components  = 1;
      components[0] = PICMAN_INDEXED_CHANNEL;
      break;
    }

  if (picman_image_has_alpha (image))
    components[n_components++] = PICMAN_ALPHA_CHANNEL;

  enum_class = g_type_class_ref (PICMAN_TYPE_CHANNEL_TYPE);

  for (i = 0; i < n_components; i++)
    {
      PicmanViewRenderer *renderer;
      GtkTreeIter       iter;
      GEnumValue       *enum_value;
      const gchar      *desc;
      gboolean          visible;

      visible = picman_image_get_component_visible (image, components[i]);

      renderer = picman_view_renderer_new (PICMAN_IMAGE_EDITOR (editor)->context,
                                         G_TYPE_FROM_INSTANCE (image),
                                         editor->view_size, 1, FALSE);
      picman_view_renderer_set_viewable (renderer, PICMAN_VIEWABLE (image));
      picman_view_renderer_remove_idle (renderer);

      PICMAN_VIEW_RENDERER_IMAGE (renderer)->channel = components[i];

      g_signal_connect (renderer, "update",
                        G_CALLBACK (picman_component_editor_renderer_update),
                        editor);

      enum_value = g_enum_get_value (enum_class, components[i]);
      desc = picman_enum_value_get_desc (enum_class, enum_value);

      gtk_list_store_append (GTK_LIST_STORE (editor->model), &iter);

      gtk_list_store_set (GTK_LIST_STORE (editor->model), &iter,
                          COLUMN_CHANNEL,  components[i],
                          COLUMN_VISIBLE,  visible,
                          COLUMN_RENDERER, renderer,
                          COLUMN_NAME,     desc,
                          -1);

      g_object_unref (renderer);

      if (picman_image_get_component_active (image, components[i]))
        gtk_tree_selection_select_iter (editor->selection, &iter);
    }

  g_type_class_unref (enum_class);
}

static void
picman_component_editor_clear_components (PicmanComponentEditor *editor)
{
  gtk_list_store_clear (GTK_LIST_STORE (editor->model));

  /*  Clear the renderer so that it don't reference the viewable.
   *  See bug #149906.
   */
  g_object_set (editor->renderer_cell, "renderer", NULL, NULL);
}

static void
picman_component_editor_clicked (GtkCellRendererToggle *cellrenderertoggle,
                               gchar                 *path_str,
                               GdkModifierType        state,
                               PicmanComponentEditor   *editor)
{
  GtkTreePath *path;
  GtkTreeIter  iter;

  path = gtk_tree_path_new_from_string (path_str);

  if (gtk_tree_model_get_iter (editor->model, &iter, path))
    {
      PicmanImage       *image = PICMAN_IMAGE_EDITOR (editor)->image;
      PicmanChannelType  channel;
      gboolean         active;

      gtk_tree_model_get (editor->model, &iter,
                          COLUMN_CHANNEL, &channel,
                          -1);
      g_object_get (cellrenderertoggle,
                    "active", &active,
                    NULL);

      picman_image_set_component_visible (image, channel, !active);
      picman_image_flush (image);
    }

  gtk_tree_path_free (path);
}

static gboolean
picman_component_editor_select (GtkTreeSelection *selection,
                              GtkTreeModel     *model,
                              GtkTreePath      *path,
                              gboolean          path_currently_selected,
                              gpointer          data)
{
  PicmanComponentEditor *editor = PICMAN_COMPONENT_EDITOR (data);
  GtkTreeIter          iter;
  PicmanChannelType      channel;
  gboolean             active;

  gtk_tree_model_get_iter (editor->model, &iter, path);
  gtk_tree_model_get (editor->model, &iter,
                      COLUMN_CHANNEL, &channel,
                      -1);

  active = picman_image_get_component_active (PICMAN_IMAGE_EDITOR (editor)->image,
                                            channel);

  return active != path_currently_selected;
}

static gboolean
picman_component_editor_button_press (GtkWidget           *widget,
                                    GdkEventButton      *bevent,
                                    PicmanComponentEditor *editor)
{
  GtkTreeViewColumn *column;
  GtkTreePath       *path;

  editor->clicked_component = -1;

  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
                                     bevent->x,
                                     bevent->y,
                                     &path, &column, NULL, NULL))
    {
      GtkTreeIter     iter;
      PicmanChannelType channel;
      gboolean        active;

      active = gtk_tree_selection_path_is_selected (editor->selection, path);

      gtk_tree_model_get_iter (editor->model, &iter, path);

      gtk_tree_path_free (path);

      gtk_tree_model_get (editor->model, &iter,
                          COLUMN_CHANNEL, &channel,
                          -1);

      editor->clicked_component = channel;

      if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
        {
          picman_editor_popup_menu (PICMAN_EDITOR (editor), NULL, NULL);
        }
      else if (bevent->type == GDK_BUTTON_PRESS && bevent->button == 1 &&
               column != editor->eye_column)
        {
          PicmanImage *image = PICMAN_IMAGE_EDITOR (editor)->image;

          picman_image_set_component_active (image, channel, ! active);
          picman_image_flush (image);
        }
    }

  return FALSE;
}

static gboolean
picman_component_editor_get_iter (PicmanComponentEditor *editor,
                                PicmanChannelType      channel,
                                GtkTreeIter         *iter)
{
  gint index;

  index = picman_image_get_component_index (PICMAN_IMAGE_EDITOR (editor)->image,
                                          channel);

  if (index != -1)
    return gtk_tree_model_iter_nth_child (editor->model, iter, NULL, index);

  return FALSE;
}

static void
picman_component_editor_renderer_update (PicmanViewRenderer    *renderer,
                                       PicmanComponentEditor *editor)
{
  PicmanChannelType channel = PICMAN_VIEW_RENDERER_IMAGE (renderer)->channel;
  GtkTreeIter     iter;

  if (picman_component_editor_get_iter (editor, channel, &iter))
    {
      GtkTreePath *path;

      path = gtk_tree_model_get_path (editor->model, &iter);
      gtk_tree_model_row_changed (editor->model, path, &iter);
      gtk_tree_path_free (path);
    }
}

static void
picman_component_editor_mode_changed (PicmanImage           *image,
                                    PicmanComponentEditor *editor)
{
  picman_component_editor_clear_components (editor);
  picman_component_editor_create_components (editor);
}

static void
picman_component_editor_alpha_changed (PicmanImage           *image,
                                     PicmanComponentEditor *editor)
{
  picman_component_editor_clear_components (editor);
  picman_component_editor_create_components (editor);
}

static void
picman_component_editor_visibility_changed (PicmanImage           *image,
                                          PicmanChannelType      channel,
                                          PicmanComponentEditor *editor)
{
  GtkTreeIter iter;

  if (picman_component_editor_get_iter (editor, channel, &iter))
    {
      gboolean visible = picman_image_get_component_visible (image, channel);

      gtk_list_store_set (GTK_LIST_STORE (editor->model), &iter,
                          COLUMN_VISIBLE, visible,
                          -1);
    }
}

static void
picman_component_editor_active_changed (PicmanImage           *image,
                                      PicmanChannelType      channel,
                                      PicmanComponentEditor *editor)
{
  GtkTreeIter iter;

  if (picman_component_editor_get_iter (editor, channel, &iter))
    {
      gboolean active = picman_image_get_component_active (image, channel);

      if (gtk_tree_selection_iter_is_selected (editor->selection, &iter) !=
          active)
        {
          if (active)
            gtk_tree_selection_select_iter (editor->selection, &iter);
          else
            gtk_tree_selection_unselect_iter (editor->selection, &iter);
        }
    }
}

static PicmanImage *
picman_component_editor_drag_component (GtkWidget        *widget,
                                      PicmanContext     **context,
                                      PicmanChannelType  *channel,
                                      gpointer          data)
{
  PicmanComponentEditor *editor = PICMAN_COMPONENT_EDITOR (data);

  if (PICMAN_IMAGE_EDITOR (editor)->image &&
      editor->clicked_component != -1)
    {
      if (channel)
        *channel = editor->clicked_component;

      if (context)
        *context = PICMAN_IMAGE_EDITOR (editor)->context;

      return PICMAN_IMAGE_EDITOR (editor)->image;
    }

  return NULL;
}
