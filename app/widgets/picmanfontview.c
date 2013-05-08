/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfontview.c
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontainer.h"
#include "core/picmancontext.h"

#include "picmancontainerview.h"
#include "picmaneditor.h"
#include "picmanfontview.h"
#include "picmanhelp-ids.h"
#include "picmanmenufactory.h"
#include "picmanuimanager.h"
#include "picmanviewrenderer.h"

#include "picman-intl.h"


static void   picman_font_view_activate_item (PicmanContainerEditor *editor,
                                            PicmanViewable        *viewable);


G_DEFINE_TYPE (PicmanFontView, picman_font_view, PICMAN_TYPE_CONTAINER_EDITOR)

#define parent_class picman_font_view_parent_class


static void
picman_font_view_class_init (PicmanFontViewClass *klass)
{
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  editor_class->activate_item = picman_font_view_activate_item;
}

static void
picman_font_view_init (PicmanFontView *view)
{
  view->refresh_button = NULL;
}

GtkWidget *
picman_font_view_new (PicmanViewType     view_type,
                    PicmanContainer   *container,
                    PicmanContext     *context,
                    gint             view_size,
                    gint             view_border_width,
                    PicmanMenuFactory *menu_factory)
{
  PicmanFontView        *font_view;
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

  font_view = g_object_new (PICMAN_TYPE_FONT_VIEW,
                            "view-type",         view_type,
                            "container",         container,
                            "context",           context,
                            "view-size",         view_size,
                            "view-border-width", view_border_width,
                            "menu-factory",      menu_factory,
                            "menu-identifier",   "<Fonts>",
                            "ui-path",           "/fonts-popup",
                            NULL);

  editor = PICMAN_CONTAINER_EDITOR (font_view);

  picman_container_view_set_reorderable (PICMAN_CONTAINER_VIEW (editor->view),
                                       FALSE);

  font_view->refresh_button =
    picman_editor_add_action_button (PICMAN_EDITOR (editor->view), "fonts",
                                   "fonts-refresh", NULL);

  picman_ui_manager_update (picman_editor_get_ui_manager (PICMAN_EDITOR (editor->view)),
                          editor);

  return GTK_WIDGET (font_view);
}

static void
picman_font_view_activate_item (PicmanContainerEditor *editor,
                              PicmanViewable        *viewable)
{
  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->activate_item (editor, viewable);
}
