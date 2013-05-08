/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmantoolpalette.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
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
#include "core/picmancontext.h"
#include "core/picmantoolinfo.h"

#include "picmandialogfactory.h"
#include "picmantoolbox.h"
#include "picmantoolpalette.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"
#include "picmanwindowstrategy.h"

#include "picman-intl.h"


#define DEFAULT_TOOL_ICON_SIZE GTK_ICON_SIZE_BUTTON
#define DEFAULT_BUTTON_RELIEF  GTK_RELIEF_NONE

#define TOOL_BUTTON_DATA_KEY   "picman-tool-palette-item"
#define TOOL_INFO_DATA_KEY     "picman-tool-info"


typedef struct _PicmanToolPalettePrivate PicmanToolPalettePrivate;

struct _PicmanToolPalettePrivate
{
  PicmanToolbox *toolbox;

  gint         tool_rows;
  gint         tool_columns;
};

#define GET_PRIVATE(p) G_TYPE_INSTANCE_GET_PRIVATE (p, \
                                                    PICMAN_TYPE_TOOL_PALETTE, \
                                                    PicmanToolPalettePrivate)


static void     picman_tool_palette_size_allocate       (GtkWidget       *widget,
                                                       GtkAllocation   *allocation);
static void     picman_tool_palette_style_set           (GtkWidget       *widget,
                                                       GtkStyle        *previous_style);

static void     picman_tool_palette_tool_changed        (PicmanContext     *context,
                                                       PicmanToolInfo    *tool_info,
                                                       PicmanToolPalette *palette);
static void     picman_tool_palette_tool_reorder        (PicmanContainer   *container,
                                                       PicmanToolInfo    *tool_info,
                                                       gint             index,
                                                       PicmanToolPalette *palette);
static void     picman_tool_palette_tool_button_toggled (GtkWidget       *widget,
                                                       PicmanToolPalette *palette);
static gboolean picman_tool_palette_tool_button_press   (GtkWidget       *widget,
                                                       GdkEventButton  *bevent,
                                                       PicmanToolPalette *palette);
static void     picman_tool_palette_initialize_tools    (PicmanToolPalette *palette);


G_DEFINE_TYPE (PicmanToolPalette, picman_tool_palette, GTK_TYPE_TOOL_PALETTE)

#define parent_class picman_tool_palette_parent_class


static void
picman_tool_palette_class_init (PicmanToolPaletteClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->size_allocate         = picman_tool_palette_size_allocate;
  widget_class->style_set             = picman_tool_palette_style_set;

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_enum ("tool-icon-size",
                                                              NULL, NULL,
                                                              GTK_TYPE_ICON_SIZE,
                                                              DEFAULT_TOOL_ICON_SIZE,
                                                              PICMAN_PARAM_READABLE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_enum ("button-relief",
                                                              NULL, NULL,
                                                              GTK_TYPE_RELIEF_STYLE,
                                                              DEFAULT_BUTTON_RELIEF,
                                                              PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanToolPalettePrivate));
}

static void
picman_tool_palette_init (PicmanToolPalette *palette)
{
}

static void
picman_tool_palette_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  PicmanToolPalettePrivate *private = GET_PRIVATE (widget);
  gint                    button_width;
  gint                    button_height;

  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  if (picman_tool_palette_get_button_size (PICMAN_TOOL_PALETTE (widget),
                                         &button_width, &button_height))
    {
      Picman  *picman = picman_toolbox_get_context (private->toolbox)->picman;
      GList *list;
      gint   n_tools;
      gint   tool_rows;
      gint   tool_columns;

      for (list = picman_get_tool_info_iter (picman), n_tools = 0;
           list;
           list = list->next)
        {
          PicmanToolInfo *tool_info = list->data;

          if (tool_info->visible)
            n_tools++;
        }

      tool_columns = MAX (1, (allocation->width / button_width));
      tool_rows    = n_tools / tool_columns;

      if (n_tools % tool_columns)
        tool_rows++;

      if (private->tool_rows    != tool_rows  ||
          private->tool_columns != tool_columns)
        {
          private->tool_rows    = tool_rows;
          private->tool_columns = tool_columns;

          gtk_widget_set_size_request (widget, -1,
                                       tool_rows * button_height);
        }
    }
}

static void
picman_tool_palette_style_set (GtkWidget *widget,
                             GtkStyle  *previous_style)
{
  PicmanToolPalettePrivate *private = GET_PRIVATE (widget);
  Picman                   *picman;
  GtkIconSize             tool_icon_size;
  GtkReliefStyle          relief;
  GList                  *list;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);

  if (! picman_toolbox_get_context (private->toolbox))
    return;

  picman = picman_toolbox_get_context (private->toolbox)->picman;

  gtk_widget_style_get (widget,
                        "tool-icon-size", &tool_icon_size,
                        "button-relief",  &relief,
                        NULL);

  gtk_tool_palette_set_icon_size (GTK_TOOL_PALETTE (widget), tool_icon_size);

  for (list = picman_get_tool_info_iter (picman);
       list;
       list = g_list_next (list))
    {
      PicmanToolInfo *tool_info = list->data;
      GtkWidget    *tool_button;

      tool_button = g_object_get_data (G_OBJECT (tool_info),
                                       TOOL_BUTTON_DATA_KEY);

      if (tool_button)
        {
          GtkWidget *button = gtk_bin_get_child (GTK_BIN (tool_button));

          gtk_button_set_relief (GTK_BUTTON (button), relief);
        }
    }

  picman_dock_invalidate_geometry (PICMAN_DOCK (private->toolbox));
}

GtkWidget *
picman_tool_palette_new (void)
{
  return g_object_new (PICMAN_TYPE_TOOL_PALETTE, NULL);
}

void
picman_tool_palette_set_toolbox (PicmanToolPalette *palette,
                               PicmanToolbox     *toolbox)
{
  PicmanToolPalettePrivate *private;
  PicmanContext            *context;

  g_return_if_fail (PICMAN_IS_TOOL_PALETTE (palette));
  g_return_if_fail (PICMAN_IS_TOOLBOX (toolbox));

  private = GET_PRIVATE (palette);

  private->toolbox = toolbox;
  context          = picman_toolbox_get_context (toolbox);

  /**
   * We must wait until PicmanToolbox has a parent so we can use
   * PicmanDock::get_ui_manager() and ::get_dialog_factory().
   */
  g_signal_connect_swapped (private->toolbox, "parent-set",
                            G_CALLBACK (picman_tool_palette_initialize_tools),
                            palette);

  g_signal_connect_object (context->picman->tool_info_list, "reorder",
                           G_CALLBACK (picman_tool_palette_tool_reorder),
                           palette, 0);

  g_signal_connect_object (context, "tool-changed",
                           G_CALLBACK (picman_tool_palette_tool_changed),
                           palette,
                           0);

}

gboolean
picman_tool_palette_get_button_size (PicmanToolPalette *palette,
                                   gint            *width,
                                   gint            *height)
{
  PicmanToolPalettePrivate *private;
  PicmanToolInfo           *tool_info;
  GtkWidget              *tool_button;

  g_return_val_if_fail (PICMAN_IS_TOOL_PALETTE (palette), FALSE);
  g_return_val_if_fail (width != NULL, FALSE);
  g_return_val_if_fail (height != NULL, FALSE);

  private = GET_PRIVATE (palette);

  tool_info   = picman_get_tool_info (picman_toolbox_get_context (private->toolbox)->picman,
                                    "picman-rect-select-tool");
  tool_button = g_object_get_data (G_OBJECT (tool_info), TOOL_BUTTON_DATA_KEY);

  if (tool_button)
    {
      GtkRequisition button_requisition;

      gtk_widget_size_request (tool_button, &button_requisition);

      *width  = button_requisition.width;
      *height = button_requisition.height;

      return TRUE;
    }

  return FALSE;
}


/*  private functions  */

static void
picman_tool_palette_tool_changed (PicmanContext      *context,
                                PicmanToolInfo     *tool_info,
                                PicmanToolPalette  *palette)
{
  if (tool_info)
    {
      GtkWidget *tool_button = g_object_get_data (G_OBJECT (tool_info),
                                                  TOOL_BUTTON_DATA_KEY);

      if (tool_button &&
          ! gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (tool_button)))
        {
          g_signal_handlers_block_by_func (tool_button,
                                           picman_tool_palette_tool_button_toggled,
                                           palette);

          gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (tool_button),
                                             TRUE);

          g_signal_handlers_unblock_by_func (tool_button,
                                             picman_tool_palette_tool_button_toggled,
                                             palette);
        }
    }
}

static void
picman_tool_palette_tool_reorder (PicmanContainer   *container,
                                PicmanToolInfo    *tool_info,
                                gint             index,
                                PicmanToolPalette *palette)
{
  if (tool_info)
    {
      GtkWidget *button = g_object_get_data (G_OBJECT (tool_info),
                                             TOOL_BUTTON_DATA_KEY);
      GtkWidget *group  = gtk_widget_get_parent (button);

      gtk_tool_item_group_set_item_position (GTK_TOOL_ITEM_GROUP (group),
                                             GTK_TOOL_ITEM (button), index);
    }
}

static void
picman_tool_palette_tool_button_toggled (GtkWidget       *widget,
                                       PicmanToolPalette *palette)
{
  PicmanToolPalettePrivate *private = GET_PRIVATE (palette);
  PicmanToolInfo           *tool_info;

  tool_info = g_object_get_data (G_OBJECT (widget), TOOL_INFO_DATA_KEY);

  if (gtk_toggle_tool_button_get_active (GTK_TOGGLE_TOOL_BUTTON (widget)))
    picman_context_set_tool (picman_toolbox_get_context (private->toolbox), tool_info);
}

static gboolean
picman_tool_palette_tool_button_press (GtkWidget       *widget,
                                     GdkEventButton  *event,
                                     PicmanToolPalette *palette)
{
  PicmanToolPalettePrivate *private = GET_PRIVATE (palette);

  if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
      PicmanContext *context = picman_toolbox_get_context (private->toolbox);
      PicmanDock    *dock    = PICMAN_DOCK (private->toolbox);

      picman_window_strategy_show_dockable_dialog (PICMAN_WINDOW_STRATEGY (picman_get_window_strategy (context->picman)),
                                                 context->picman,
                                                 picman_dock_get_dialog_factory (dock),
                                                 gtk_widget_get_screen (widget),
                                                 "picman-tool-options");
    }

  return FALSE;
}

static void
picman_tool_palette_initialize_tools (PicmanToolPalette *palette)
{
  PicmanContext            *context;
  PicmanToolInfo           *active_tool;
  GList                  *list;
  GSList                 *item_group = NULL;
  PicmanToolPalettePrivate *private    = GET_PRIVATE(palette);
  GtkWidget              *group;

  group = gtk_tool_item_group_new (_("Tools"));
  gtk_tool_item_group_set_label_widget (GTK_TOOL_ITEM_GROUP (group), NULL);
  gtk_container_add (GTK_CONTAINER (palette), group);
  gtk_widget_show (group);

  context     = picman_toolbox_get_context (private->toolbox);
  active_tool = picman_context_get_tool (context);

  for (list = picman_get_tool_info_iter (context->picman);
       list;
       list = g_list_next (list))
    {
      PicmanToolInfo  *tool_info = list->data;
      GtkToolItem   *item;
      const gchar   *stock_id;
      PicmanUIManager *ui_manager;

      stock_id = picman_viewable_get_stock_id (PICMAN_VIEWABLE (tool_info));

      item = gtk_radio_tool_button_new_from_stock (item_group, stock_id);
      item_group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (item));
      gtk_tool_item_group_insert (GTK_TOOL_ITEM_GROUP (group), item, -1);
      gtk_widget_show (GTK_WIDGET (item));

      g_object_bind_property (tool_info, "visible",
                              item,      "visible-horizontal",
                              G_BINDING_SYNC_CREATE);
      g_object_bind_property (tool_info, "visible",
                              item,      "visible-vertical",
                              G_BINDING_SYNC_CREATE);

      g_object_set_data (G_OBJECT (tool_info), TOOL_BUTTON_DATA_KEY, item);
      g_object_set_data (G_OBJECT (item)  ,    TOOL_INFO_DATA_KEY,   tool_info);

      if (tool_info == active_tool)
        gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (item), TRUE);

      g_signal_connect (item, "toggled",
                        G_CALLBACK (picman_tool_palette_tool_button_toggled),
                        palette);

      g_signal_connect (gtk_bin_get_child (GTK_BIN (item)), "button-press-event",
                        G_CALLBACK (picman_tool_palette_tool_button_press),
                        palette);

      ui_manager = picman_dock_get_ui_manager (PICMAN_DOCK (private->toolbox));
      if (ui_manager)
        {
          GtkAction   *action     = NULL;
          const gchar *identifier = NULL;
          gchar       *tmp        = NULL;
          gchar       *name       = NULL;

          identifier = picman_object_get_name (tool_info);

          tmp = g_strndup (identifier + strlen ("picman-"),
                           strlen (identifier) - strlen ("picman--tool"));
          name = g_strdup_printf ("tools-%s", tmp);
          g_free (tmp);

          action = picman_ui_manager_find_action (ui_manager, "tools", name);
          g_free (name);

          if (action)
            picman_widget_set_accel_help (GTK_WIDGET (item), action);
          else
            picman_help_set_help_data (GTK_WIDGET (item),
                                     tool_info->help, tool_info->help_id);
        }
    }

  /* We only need to initialize tools once */
  g_signal_handlers_disconnect_by_func (private->toolbox,
                                        picman_tool_palette_initialize_tools,
                                        palette);
}
