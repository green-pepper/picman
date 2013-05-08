/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandocumentview.c
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

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"

#include "picmancontainerview.h"
#include "picmaneditor.h"
#include "picmanimageview.h"
#include "picmandnd.h"
#include "picmanmenufactory.h"
#include "picmanuimanager.h"
#include "picmanviewrenderer.h"

#include "picman-intl.h"


static void   picman_image_view_activate_item (PicmanContainerEditor *editor,
                                             PicmanViewable        *viewable);


G_DEFINE_TYPE (PicmanImageView, picman_image_view, PICMAN_TYPE_CONTAINER_EDITOR)

#define parent_class picman_image_view_parent_class


static void
picman_image_view_class_init (PicmanImageViewClass *klass)
{
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  editor_class->activate_item = picman_image_view_activate_item;
}

static void
picman_image_view_init (PicmanImageView *view)
{
  view->raise_button  = NULL;
  view->new_button    = NULL;
  view->delete_button = NULL;
}

GtkWidget *
picman_image_view_new (PicmanViewType     view_type,
                     PicmanContainer   *container,
                     PicmanContext     *context,
                     gint             view_size,
                     gint             view_border_width,
                     PicmanMenuFactory *menu_factory)
{
  PicmanImageView       *image_view;
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

  image_view = g_object_new (PICMAN_TYPE_IMAGE_VIEW,
                             "view-type",         view_type,
                             "container",         container,
                             "context",           context,
                             "view-size",         view_size,
                             "view-border-width", view_border_width,
                             "menu-factory",      menu_factory,
                             "menu-identifier",   "<Images>",
                             "ui-path",           "/images-popup",
                             NULL);

  editor = PICMAN_CONTAINER_EDITOR (image_view);

  image_view->raise_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "images",
                                   "images-raise-views", NULL);

  image_view->new_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "images",
                                   "images-new-view", NULL);

  image_view->delete_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "images",
                                   "images-delete", NULL);

  if (view_type == PICMAN_VIEW_TYPE_LIST)
    {
      GtkWidget *dnd_widget;

      dnd_widget = picman_container_view_get_dnd_widget (editor->view);

      picman_dnd_xds_source_add (dnd_widget,
                               (PicmanDndDragViewableFunc) picman_dnd_get_drag_data,
                               NULL);
    }

  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (image_view->raise_button),
                                  PICMAN_TYPE_IMAGE);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (image_view->new_button),
                                  PICMAN_TYPE_IMAGE);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (image_view->delete_button),
                                  PICMAN_TYPE_IMAGE);

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                          editor);

  return GTK_WIDGET (image_view);
}

static void
picman_image_view_activate_item (PicmanContainerEditor *editor,
                               PicmanViewable        *viewable)
{
  PicmanImageView *view = PICMAN_IMAGE_VIEW (editor);
  PicmanContainer *container;

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item (editor, viewable);

  container = picman_container_view_get_container (editor->view);

  if (viewable && picman_container_have (container, PICMAN_OBJECT (viewable)))
    {
      gtk_button_clicked (GTK_BUTTON (view->raise_button));
    }
}
