/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandynamicsfactoryview.c
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

#include "core/picmancontext.h"
#include "core/picmandatafactory.h"

#include "picmandynamicsfactoryview.h"
#include "picmanmenufactory.h"
#include "picmanviewrenderer.h"


G_DEFINE_TYPE (PicmanDynamicsFactoryView, picman_dynamics_factory_view,
               PICMAN_TYPE_DATA_FACTORY_VIEW)


static void
picman_dynamics_factory_view_class_init (PicmanDynamicsFactoryViewClass *klass)
{
}

static void
picman_dynamics_factory_view_init (PicmanDynamicsFactoryView *view)
{
}

GtkWidget *
picman_dynamics_factory_view_new (PicmanViewType      view_type,
                                PicmanDataFactory  *factory,
                                PicmanContext      *context,
                                gint              view_size,
                                gint              view_border_width,
                                PicmanMenuFactory  *menu_factory)
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

  return g_object_new (PICMAN_TYPE_DYNAMICS_FACTORY_VIEW,
                       "view-type",         view_type,
                       "data-factory",      factory,
                       "context",           context,
                       "view-size",         view_size,
                       "view-border-width", view_border_width,
                       "menu-factory",      menu_factory,
                       "menu-identifier",   "<Dynamics>",
                       "ui-path",           "/dynamics-popup",
                       "action-group",      "dynamics",
                       NULL);
}
