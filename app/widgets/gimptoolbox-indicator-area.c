/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmancontext.h"
#include "core/picmangradient.h"
#include "core/picmanpattern.h"

#include "picmandialogfactory.h"
#include "picmandnd.h"
#include "picmanview.h"
#include "picmantoolbox.h"
#include "picmantoolbox-indicator-area.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


#define CELL_SIZE        24  /*  The size of the previews                  */
#define GRAD_CELL_WIDTH  52  /*  The width of the gradient preview         */
#define GRAD_CELL_HEIGHT 12  /*  The height of the gradient preview        */
#define CELL_SPACING      2  /*  How much between brush and pattern cells  */


static void
brush_preview_clicked (GtkWidget       *widget,
                       GdkModifierType  state,
                       PicmanToolbox     *toolbox)
{
  PicmanContext *context = picman_toolbox_get_context (toolbox);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                             context->picman,
                                             picman_dock_get_dialog_factory (PICMAN_DOCK (toolbox)),
                                             gtk_widget_get_screen (widget),
                                             "picman-brush-grid|picman-brush-list");
}

static void
brush_preview_drop_brush (GtkWidget    *widget,
                          gint          x,
                          gint          y,
                          PicmanViewable *viewable,
                          gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  picman_context_set_brush (context, PICMAN_BRUSH (viewable));
}

static void
pattern_preview_clicked (GtkWidget       *widget,
                         GdkModifierType  state,
                         PicmanToolbox     *toolbox)
{
  PicmanContext *context = picman_toolbox_get_context (toolbox);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                             context->picman,
                                             picman_dock_get_dialog_factory (PICMAN_DOCK (toolbox)),
                                             gtk_widget_get_screen (widget),
                                             "picman-pattern-grid|picman-pattern-list");
}

static void
pattern_preview_drop_pattern (GtkWidget    *widget,
                              gint          x,
                              gint          y,
                              PicmanViewable *viewable,
                              gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  picman_context_set_pattern (context, PICMAN_PATTERN (viewable));
}

static void
gradient_preview_clicked (GtkWidget       *widget,
                          GdkModifierType  state,
                          PicmanToolbox     *toolbox)
{
  PicmanContext *context = picman_toolbox_get_context (toolbox);

  picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                             context->picman,
                                             picman_dock_get_dialog_factory (PICMAN_DOCK (toolbox)),
                                             gtk_widget_get_screen (widget),
                                             "picman-gradient-list|picman-gradient-grid");
}

static void
gradient_preview_drop_gradient (GtkWidget    *widget,
                                gint          x,
                                gint          y,
                                PicmanViewable *viewable,
                                gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  picman_context_set_gradient (context, PICMAN_GRADIENT (viewable));
}


/*  public functions  */

GtkWidget *
picman_toolbox_indicator_area_create (PicmanToolbox *toolbox)
{
  PicmanContext *context;
  GtkWidget   *indicator_table;
  GtkWidget   *brush_view;
  GtkWidget   *pattern_view;
  GtkWidget   *gradient_view;

  g_return_val_if_fail (PICMAN_IS_TOOLBOX (toolbox), NULL);

  context = picman_toolbox_get_context (toolbox);

  indicator_table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (indicator_table), CELL_SPACING);
  gtk_table_set_col_spacings (GTK_TABLE (indicator_table), CELL_SPACING);

  /*  brush view  */

  brush_view =
    picman_view_new_full_by_types (context,
                                 PICMAN_TYPE_VIEW, PICMAN_TYPE_BRUSH,
                                 CELL_SIZE, CELL_SIZE, 1,
                                 FALSE, TRUE, TRUE);
  picman_view_set_viewable (PICMAN_VIEW (brush_view),
                          PICMAN_VIEWABLE (picman_context_get_brush (context)));
  gtk_table_attach_defaults (GTK_TABLE (indicator_table), brush_view,
                             0, 1, 0, 1);
  gtk_widget_show (brush_view);

  picman_help_set_help_data (brush_view,
                           _("The active brush.\n"
                             "Click to open the Brush Dialog."), NULL);

  g_signal_connect_object (context, "brush-changed",
                           G_CALLBACK (picman_view_set_viewable),
                           brush_view,
                           G_CONNECT_SWAPPED);

  g_signal_connect (brush_view, "clicked",
                    G_CALLBACK (brush_preview_clicked),
                    toolbox);

  picman_dnd_viewable_dest_add (brush_view,
                              PICMAN_TYPE_BRUSH,
                              brush_preview_drop_brush,
                              context);

  /*  pattern view  */

  pattern_view =
    picman_view_new_full_by_types (context,
                                 PICMAN_TYPE_VIEW, PICMAN_TYPE_PATTERN,
                                 CELL_SIZE, CELL_SIZE, 1,
                                 FALSE, TRUE, TRUE);
  picman_view_set_viewable (PICMAN_VIEW (pattern_view),
                          PICMAN_VIEWABLE (picman_context_get_pattern (context)));

  gtk_table_attach_defaults (GTK_TABLE (indicator_table), pattern_view,
                             1, 2, 0, 1);
  gtk_widget_show (pattern_view);

  picman_help_set_help_data (pattern_view,
                           _("The active pattern.\n"
                             "Click to open the Pattern Dialog."), NULL);

  g_signal_connect_object (context, "pattern-changed",
                           G_CALLBACK (picman_view_set_viewable),
                           pattern_view,
                           G_CONNECT_SWAPPED);

  g_signal_connect (pattern_view, "clicked",
                    G_CALLBACK (pattern_preview_clicked),
                    toolbox);

  picman_dnd_viewable_dest_add (pattern_view,
                              PICMAN_TYPE_PATTERN,
                              pattern_preview_drop_pattern,
                              context);

  /*  gradient view  */

  gradient_view =
    picman_view_new_full_by_types (context,
                                 PICMAN_TYPE_VIEW, PICMAN_TYPE_GRADIENT,
                                 GRAD_CELL_WIDTH, GRAD_CELL_HEIGHT, 1,
                                 FALSE, TRUE, TRUE);
  picman_view_set_viewable (PICMAN_VIEW (gradient_view),
                          PICMAN_VIEWABLE (picman_context_get_gradient (context)));

  gtk_table_attach_defaults (GTK_TABLE (indicator_table), gradient_view,
                             0, 2, 1, 2);
  gtk_widget_show (gradient_view);

  picman_help_set_help_data (gradient_view,
                           _("The active gradient.\n"
                             "Click to open the Gradient Dialog."), NULL);

  g_signal_connect_object (context, "gradient-changed",
                           G_CALLBACK (picman_view_set_viewable),
                           gradient_view,
                           G_CONNECT_SWAPPED);

  g_signal_connect (gradient_view, "clicked",
                    G_CALLBACK (gradient_preview_clicked),
                    toolbox);

  picman_dnd_viewable_dest_add (gradient_view,
                              PICMAN_TYPE_GRADIENT,
                              gradient_preview_drop_gradient,
                              context);

  gtk_widget_show (indicator_table);

  return indicator_table;
}
