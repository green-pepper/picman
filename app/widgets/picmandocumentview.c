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

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimagefile.h"

#include "picmancontainerview.h"
#include "picmandocumentview.h"
#include "picmandnd.h"
#include "picmaneditor.h"
#include "picmanmenufactory.h"
#include "picmanuimanager.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"

#include "picman-intl.h"


static void    picman_document_view_activate_item (PicmanContainerEditor *editor,
                                                 PicmanViewable        *viewable);
static GList * picman_document_view_drag_uri_list (GtkWidget           *widget,
                                                 gpointer             data);


G_DEFINE_TYPE (PicmanDocumentView, picman_document_view,
               PICMAN_TYPE_CONTAINER_EDITOR)

#define parent_class picman_document_view_parent_class


static void
picman_document_view_class_init (PicmanDocumentViewClass *klass)
{
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  editor_class->activate_item = picman_document_view_activate_item;
}

static void
picman_document_view_init (PicmanDocumentView *view)
{
  view->open_button    = NULL;
  view->remove_button  = NULL;
  view->refresh_button = NULL;
}

GtkWidget *
picman_document_view_new (PicmanViewType     view_type,
                        PicmanContainer   *container,
                        PicmanContext     *context,
                        gint             view_size,
                        gint             view_border_width,
                        PicmanMenuFactory *menu_factory)
{
  PicmanDocumentView    *document_view;
  PicmanContainerEditor *editor;

  g_return_val_if_fail (PICMAN_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, FALSE);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        FALSE);
  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  document_view = g_object_new (PICMAN_TYPE_DOCUMENT_VIEW,
                                "view-type",         view_type,
                                "container",         container,
                                "context",           context,
                                "view-size",         view_size,
                                "view-border-width", view_border_width,
                                "menu-factory",      menu_factory,
                                "menu-identifier",   "<Documents>",
                                "ui-path",           "/documents-popup",
                                NULL);

  editor = PICMAN_CONTAINER_EDITOR (document_view);

  document_view->open_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "documents",
                                   "documents-open",
                                   "documents-raise-or-open",
                                   GDK_SHIFT_MASK,
                                   "documents-file-open-dialog",
                                   picman_get_toggle_behavior_mask (),
                                   NULL);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (document_view->open_button),
                                  PICMAN_TYPE_IMAGEFILE);

  document_view->remove_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "documents",
                                   "documents-remove", NULL);
  picman_container_view_enable_dnd (editor->view,
                                  GTK_BUTTON (document_view->remove_button),
                                  PICMAN_TYPE_IMAGEFILE);

  picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "documents",
                                 "documents-clear", NULL);

  document_view->refresh_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "documents",
                                   "documents-recreate-preview",
                                   "documents-reload-previews",
                                   GDK_SHIFT_MASK,
                                   "documents-remove-dangling",
                                   picman_get_toggle_behavior_mask (),
                                   NULL);

  if (view_type == PICMAN_VIEW_TYPE_LIST)
    {
      GtkWidget *dnd_widget;

      dnd_widget = picman_container_view_get_dnd_widget (editor->view);

      picman_dnd_uri_list_source_add (dnd_widget,
                                    picman_document_view_drag_uri_list,
                                    editor);
    }

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                          editor);

  return GTK_WIDGET (document_view);
}

static void
picman_document_view_activate_item (PicmanContainerEditor *editor,
                                  PicmanViewable        *viewable)
{
  PicmanDocumentView *view = PICMAN_DOCUMENT_VIEW (editor);
  PicmanContainer    *container;

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item (editor, viewable);

  container = picman_container_view_get_container (editor->view);

  if (viewable && picman_container_have (container, PICMAN_OBJECT (viewable)))
    {
      gtk_button_clicked (GTK_BUTTON (view->open_button));
    }
}

static GList *
picman_document_view_drag_uri_list (GtkWidget *widget,
                                  gpointer   data)
{
  PicmanViewable *viewable = picman_dnd_get_drag_data (widget);

  if (viewable)
    {
      const gchar *uri = picman_object_get_name (viewable);

      return g_list_append (NULL, g_strdup (uri));
    }

  return NULL;
}
