/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantoolpresetfactoryview.c
 * Copyright (C) 2010 Alexia Death
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

#include "core/picmancontext.h"
#include "core/picmandatafactory.h"
#include "core/picmanviewable.h"

#include "picmaneditor.h"
#include "picmanmenufactory.h"
#include "picmantoolpresetfactoryview.h"
#include "picmanviewrenderer.h"


G_DEFINE_TYPE (PicmanToolPresetFactoryView, picman_tool_preset_factory_view,
               PICMAN_TYPE_DATA_FACTORY_VIEW)


static void
picman_tool_preset_factory_view_class_init (PicmanToolPresetFactoryViewClass *klass)
{
}

static void
picman_tool_preset_factory_view_init (PicmanToolPresetFactoryView *view)
{
}

GtkWidget *
picman_tool_preset_factory_view_new (PicmanViewType      view_type,
                                   PicmanDataFactory  *factory,
                                   PicmanContext      *context,
                                   gint              view_size,
                                   gint              view_border_width,
                                   PicmanMenuFactory  *menu_factory)
{
  PicmanToolPresetFactoryView *factory_view;

  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  factory_view = g_object_new (PICMAN_TYPE_TOOL_PRESET_FACTORY_VIEW,
                               "view-type",         view_type,
                               "data-factory",      factory,
                               "context",           context,
                               "view-size",         view_size,
                               "view-border-width", view_border_width,
                               "menu-factory",      menu_factory,
                               "menu-identifier",   "<ToolPresets>",
                               "ui-path",           "/tool-presets-popup",
                               "action-group",      "tool-presets",
                               NULL);

  gtk_widget_hide (picman_data_factory_view_get_duplicate_button (PICMAN_DATA_FACTORY_VIEW (factory_view)));

  return GTK_WIDGET (factory_view);
}
