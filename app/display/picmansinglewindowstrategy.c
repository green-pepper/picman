/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmansinglewindowstrategy.c
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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "display-types.h"

#include "core/picman.h"

#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockbook.h"
#include "widgets/picmandockcolumns.h"
#include "widgets/picmanwindowstrategy.h"

#include "picmanimagewindow.h"
#include "picmansinglewindowstrategy.h"


static void        picman_single_window_strategy_window_strategy_iface_init (PicmanWindowStrategyInterface *iface);
static GtkWidget * picman_single_window_strategy_show_dockable_dialog       (PicmanWindowStrategy          *strategy,
                                                                           Picman                        *picman,
                                                                           PicmanDialogFactory           *factory,
                                                                           GdkScreen                   *screen,
                                                                           const gchar                 *identifiers);


G_DEFINE_TYPE_WITH_CODE (PicmanSingleWindowStrategy, picman_single_window_strategy, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_WINDOW_STRATEGY,
                                                picman_single_window_strategy_window_strategy_iface_init))

#define parent_class picman_single_window_strategy_parent_class


static void
picman_single_window_strategy_class_init (PicmanSingleWindowStrategyClass *klass)
{
}

static void
picman_single_window_strategy_init (PicmanSingleWindowStrategy *strategy)
{
}

static void
picman_single_window_strategy_window_strategy_iface_init (PicmanWindowStrategyInterface *iface)
{
  iface->show_dockable_dialog = picman_single_window_strategy_show_dockable_dialog;
}

static GtkWidget *
picman_single_window_strategy_show_dockable_dialog (PicmanWindowStrategy *strategy,
                                                  Picman               *picman,
                                                  PicmanDialogFactory  *factory,
                                                  GdkScreen          *screen,
                                                  const gchar        *identifiers)
{
  GList           *windows = picman_get_image_windows (picman);
  GtkWidget       *widget  = NULL;
  PicmanImageWindow *window;

  g_return_val_if_fail (windows != NULL, NULL);

  /* In single-window mode, there should only be one window... */
  window = PICMAN_IMAGE_WINDOW (windows->data);

  if (strcmp ("picman-toolbox", identifiers) == 0)
    {
      /* Only allow one toolbox... */
      if (! picman_image_window_has_toolbox (window))
        {
          PicmanDockColumns *columns;
          PicmanUIManager   *ui_manager = picman_image_window_get_ui_manager (window);

          widget = picman_dialog_factory_dialog_new (factory,
                                                   screen,
                                                   ui_manager,
                                                   "picman-toolbox",
                                                   -1 /*view_size*/,
                                                   FALSE /*present*/);
          gtk_widget_show (widget);

          columns = picman_image_window_get_left_docks (window);
          picman_dock_columns_add_dock (columns,
                                      PICMAN_DOCK (widget),
                                      -1 /*index*/);
        }
    }
  else if (picman_dialog_factory_find_widget (factory, identifiers))
    {
      /* if the dialog is already open, simply raise it */
      return picman_dialog_factory_dialog_raise (factory, screen, identifiers, -1);
   }
  else
    {
      GtkWidget *dockbook;

      dockbook = picman_image_window_get_default_dockbook (window);

      if (! dockbook)
        {
          PicmanDockColumns *dock_columns;

          /* No dock, need to add one */
          dock_columns = picman_image_window_get_right_docks (window);
          picman_dock_columns_prepare_dockbook (dock_columns,
                                              -1 /*index*/,
                                              &dockbook);
        }

      widget = picman_dockbook_add_from_dialog_factory (PICMAN_DOCKBOOK (dockbook),
                                                        identifiers,
                                                        -1 /*index*/);
    }


  g_list_free (windows);

  return widget;
}

PicmanObject *
picman_single_window_strategy_get_singleton (void)
{
  static PicmanObject *singleton = NULL;

  if (! singleton)
    singleton = g_object_new (PICMAN_TYPE_SINGLE_WINDOW_STRATEGY, NULL);

  return singleton;
}
