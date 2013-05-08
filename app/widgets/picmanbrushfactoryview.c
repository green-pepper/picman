/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanbrushfactoryview.c
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
#include "core/picmanbrush.h"
#include "core/picmanbrushgenerated.h"
#include "core/picmandatafactory.h"

#include "picmanbrushfactoryview.h"
#include "picmancontainerview.h"
#include "picmanmenufactory.h"
#include "picmanspinscale.h"
#include "picmanviewrenderer.h"

#include "picman-intl.h"


static void   picman_brush_factory_view_dispose         (GObject              *object);

static void   picman_brush_factory_view_select_item     (PicmanContainerEditor  *editor,
                                                       PicmanViewable         *viewable);

static void   picman_brush_factory_view_spacing_changed (PicmanBrush            *brush,
                                                       PicmanBrushFactoryView *view);
static void   picman_brush_factory_view_spacing_update  (GtkAdjustment        *adjustment,
                                                       PicmanBrushFactoryView *view);


G_DEFINE_TYPE (PicmanBrushFactoryView, picman_brush_factory_view,
               PICMAN_TYPE_DATA_FACTORY_VIEW)

#define parent_class picman_brush_factory_view_parent_class


static void
picman_brush_factory_view_class_init (PicmanBrushFactoryViewClass *klass)
{
  GObjectClass             *object_class = G_OBJECT_CLASS (klass);
  PicmanContainerEditorClass *editor_class = PICMAN_CONTAINER_EDITOR_CLASS (klass);

  object_class->dispose     = picman_brush_factory_view_dispose;

  editor_class->select_item = picman_brush_factory_view_select_item;
}

static void
picman_brush_factory_view_init (PicmanBrushFactoryView *view)
{
  view->spacing_adjustment =
    GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 1.0, 5000.0,
                                        1.0, 10.0, 0.0));

  view->spacing_scale = picman_spin_scale_new (view->spacing_adjustment,
                                             _("Spacing"), 1);
  picman_spin_scale_set_scale_limits (PICMAN_SPIN_SCALE (view->spacing_scale),
                                   1.0, 200.0);
  picman_help_set_help_data (view->spacing_scale,
                           _("Percentage of width of brush"),
                           NULL);

  g_signal_connect (view->spacing_adjustment, "value-changed",
                    G_CALLBACK (picman_brush_factory_view_spacing_update),
                    view);
}

static void
picman_brush_factory_view_dispose (GObject *object)
{
  PicmanBrushFactoryView *view   = PICMAN_BRUSH_FACTORY_VIEW (object);
  PicmanContainerEditor  *editor = PICMAN_CONTAINER_EDITOR (object);

  if (view->spacing_changed_handler_id)
    {
      PicmanDataFactory *factory;
      PicmanContainer   *container;

      factory   = picman_data_factory_view_get_data_factory (PICMAN_DATA_FACTORY_VIEW (editor));
      container = picman_data_factory_get_container (factory);

      picman_container_remove_handler (container,
                                     view->spacing_changed_handler_id);

      view->spacing_changed_handler_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

GtkWidget *
picman_brush_factory_view_new (PicmanViewType     view_type,
                             PicmanDataFactory *factory,
                             PicmanContext     *context,
                             gboolean         change_brush_spacing,
                             gint             view_size,
                             gint             view_border_width,
                             PicmanMenuFactory *menu_factory)
{
  PicmanBrushFactoryView *factory_view;
  PicmanContainerEditor  *editor;

  g_return_val_if_fail (PICMAN_IS_DATA_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (view_size > 0 &&
                        view_size <= PICMAN_VIEWABLE_MAX_PREVIEW_SIZE, NULL);
  g_return_val_if_fail (view_border_width >= 0 &&
                        view_border_width <= PICMAN_VIEW_MAX_BORDER_WIDTH,
                        NULL);
  g_return_val_if_fail (menu_factory == NULL ||
                        PICMAN_IS_MENU_FACTORY (menu_factory), NULL);

  factory_view = g_object_new (PICMAN_TYPE_BRUSH_FACTORY_VIEW,
                               "view-type",         view_type,
                               "data-factory",      factory,
                               "context",           context,
                               "view-size",         view_size,
                               "view-border-width", view_border_width,
                               "menu-factory",      menu_factory,
                               "menu-identifier",   "<Brushes>",
                               "ui-path",           "/brushes-popup",
                               "action-group",      "brushes",
                               NULL);

  factory_view->change_brush_spacing = change_brush_spacing;

  editor = PICMAN_CONTAINER_EDITOR (factory_view);

  gtk_box_pack_end (GTK_BOX (editor->view), factory_view->spacing_scale,
                    FALSE, FALSE, 0);
  gtk_widget_show (factory_view->spacing_scale);

  factory_view->spacing_changed_handler_id =
    picman_container_add_handler (picman_data_factory_get_container (factory), "spacing-changed",
                                G_CALLBACK (picman_brush_factory_view_spacing_changed),
                                factory_view);

  return GTK_WIDGET (factory_view);
}

static void
picman_brush_factory_view_select_item (PicmanContainerEditor *editor,
                                     PicmanViewable        *viewable)
{
  PicmanBrushFactoryView *view = PICMAN_BRUSH_FACTORY_VIEW (editor);
  PicmanContainer        *container;
  gboolean              spacing_sensitive = FALSE;

  if (PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->select_item)
    PICMAN_CONTAINER_EDITOR_CLASS (parent_class)->select_item (editor, viewable);

  container = picman_container_view_get_container (editor->view);

  if (viewable && picman_container_have (container, PICMAN_OBJECT (viewable)))
    {
      PicmanBrush *brush = PICMAN_BRUSH (viewable);

      spacing_sensitive = TRUE;

      g_signal_handlers_block_by_func (view->spacing_adjustment,
                                       picman_brush_factory_view_spacing_update,
                                       view);

      gtk_adjustment_set_value (view->spacing_adjustment,
                                picman_brush_get_spacing (brush));

      g_signal_handlers_unblock_by_func (view->spacing_adjustment,
                                         picman_brush_factory_view_spacing_update,
                                         view);
    }

  gtk_widget_set_sensitive (view->spacing_scale, spacing_sensitive);
}

static void
picman_brush_factory_view_spacing_changed (PicmanBrush            *brush,
                                         PicmanBrushFactoryView *view)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (view);
  PicmanContext         *context;

  context = picman_container_view_get_context (editor->view);

  if (brush == picman_context_get_brush (context))
    {
      g_signal_handlers_block_by_func (view->spacing_adjustment,
                                       picman_brush_factory_view_spacing_update,
                                       view);

      gtk_adjustment_set_value (view->spacing_adjustment,
                                picman_brush_get_spacing (brush));

      g_signal_handlers_unblock_by_func (view->spacing_adjustment,
                                         picman_brush_factory_view_spacing_update,
                                         view);
    }
}

static void
picman_brush_factory_view_spacing_update (GtkAdjustment        *adjustment,
                                        PicmanBrushFactoryView *view)
{
  PicmanContainerEditor *editor = PICMAN_CONTAINER_EDITOR (view);
  PicmanContext         *context;
  PicmanBrush           *brush;

  context = picman_container_view_get_context (editor->view);

  brush = picman_context_get_brush (context);

  if (brush && view->change_brush_spacing)
    {
      g_signal_handlers_block_by_func (brush,
                                       picman_brush_factory_view_spacing_changed,
                                       view);

      picman_brush_set_spacing (brush, gtk_adjustment_get_value (adjustment));

      g_signal_handlers_unblock_by_func (brush,
                                         picman_brush_factory_view_spacing_changed,
                                         view);
    }
}
