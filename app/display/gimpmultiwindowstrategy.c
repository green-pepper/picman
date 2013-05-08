/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanmultiwindowstrategy.c
 * Copyright (C) 2011 Martin Nordholts <martinn@src.gnome.org>
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

#include "display-types.h"

#include "core/picman.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmanwindowstrategy.h"

#include "picmanmultiwindowstrategy.h"


static void        picman_multi_window_strategy_window_strategy_iface_init (PicmanWindowStrategyInterface *iface);
static GtkWidget * picman_multi_window_strategy_show_dockable_dialog       (PicmanWindowStrategy          *strategy,
                                                                          Picman                        *picman,
                                                                          PicmanDialogFactory           *factory,
                                                                          GdkScreen                   *screen,
                                                                          const gchar                 *identifiers);


G_DEFINE_TYPE_WITH_CODE (PicmanMultiWindowStrategy, picman_multi_window_strategy, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_WINDOW_STRATEGY,
                                                picman_multi_window_strategy_window_strategy_iface_init))

#define parent_class picman_multi_window_strategy_parent_class


static void
picman_multi_window_strategy_class_init (PicmanMultiWindowStrategyClass *klass)
{
}

static void
picman_multi_window_strategy_init (PicmanMultiWindowStrategy *strategy)
{
}

static void
picman_multi_window_strategy_window_strategy_iface_init (PicmanWindowStrategyInterface *iface)
{
  iface->show_dockable_dialog = picman_multi_window_strategy_show_dockable_dialog;
}

static GtkWidget *
picman_multi_window_strategy_show_dockable_dialog (PicmanWindowStrategy *strategy,
                                                 Picman               *picman,
                                                 PicmanDialogFactory  *factory,
                                                 GdkScreen          *screen,
                                                 const gchar        *identifiers)
{
  return picman_dialog_factory_dialog_raise (factory, screen, identifiers, -1);
}

PicmanObject *
picman_multi_window_strategy_get_singleton (void)
{
  static PicmanObject *singleton = NULL;

  if (! singleton)
    singleton = g_object_new (PICMAN_TYPE_MULTI_WINDOW_STRATEGY, NULL);

  return singleton;
}
