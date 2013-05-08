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

#include <string.h>

#undef GSEAL_ENABLE

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "picmancairo-wilber.h"
#include "picmandevices.h"
#include "picmandialogfactory.h"
#include "picmandockwindow.h"
#include "picmanhelp-ids.h"
#include "picmanpanedbox.h"
#include "picmantoolbox.h"
#include "picmantoolbox-color-area.h"
#include "picmantoolbox-dnd.h"
#include "picmantoolbox-image-area.h"
#include "picmantoolbox-indicator-area.h"
#include "picmantoolpalette.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"
#include "gtkhwrapbox.h"

#include "about.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_CONTEXT
};


struct _PicmanToolboxPrivate
{
  PicmanContext       *context;

  GtkWidget         *vbox;

  GtkWidget         *header;
  GtkWidget         *tool_palette;
  GtkWidget         *area_wbox;
  GtkWidget         *color_area;
  GtkWidget         *foo_area;
  GtkWidget         *image_area;

  gint               area_rows;
  gint               area_columns;

  PicmanPanedBox      *drag_handler;

  gboolean           in_destruction;
};


static void        picman_toolbox_constructed             (GObject        *object);
static void        picman_toolbox_dispose                 (GObject        *object);
static void        picman_toolbox_set_property            (GObject        *object,
                                                         guint           property_id,
                                                         const GValue   *value,
                                                         GParamSpec     *pspec);
static void        picman_toolbox_get_property            (GObject        *object,
                                                         guint           property_id,
                                                         GValue         *value,
                                                         GParamSpec     *pspec);
static void        picman_toolbox_size_allocate           (GtkWidget      *widget,
                                                         GtkAllocation  *allocation);
static gboolean    picman_toolbox_button_press_event      (GtkWidget      *widget,
                                                         GdkEventButton *event);
static void        picman_toolbox_drag_leave              (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         guint           time,
                                                         PicmanToolbox    *toolbox);
static gboolean    picman_toolbox_drag_motion             (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         gint            x,
                                                         gint            y,
                                                         guint           time,
                                                         PicmanToolbox    *toolbox);
static gboolean    picman_toolbox_drag_drop               (GtkWidget      *widget,
                                                         GdkDragContext *context,
                                                         gint            x,
                                                         gint            y,
                                                         guint           time,
                                                         PicmanToolbox    *toolbox);
static gchar     * picman_toolbox_get_description         (PicmanDock       *dock,
                                                         gboolean        complete);
static void        picman_toolbox_set_host_geometry_hints (PicmanDock       *dock,
                                                         GtkWindow      *window);
static void        picman_toolbox_book_added              (PicmanDock       *dock,
                                                         PicmanDockbook   *dockbook);
static void        picman_toolbox_book_removed            (PicmanDock       *dock,
                                                         PicmanDockbook   *dockbook);
static void        picman_toolbox_size_request_wilber     (GtkWidget      *widget,
                                                         GtkRequisition *requisition,
                                                         PicmanToolbox    *toolbox);
static gboolean    picman_toolbox_expose_wilber           (GtkWidget      *widget,
                                                         GdkEventExpose *event);
static GtkWidget * toolbox_create_color_area            (PicmanToolbox    *toolbox,
                                                         PicmanContext    *context);
static GtkWidget * toolbox_create_foo_area              (PicmanToolbox    *toolbox,
                                                         PicmanContext    *context);
static GtkWidget * toolbox_create_image_area            (PicmanToolbox    *toolbox,
                                                         PicmanContext    *context);
static void        toolbox_area_notify                  (PicmanGuiConfig  *config,
                                                         GParamSpec     *pspec,
                                                         GtkWidget      *area);
static void        toolbox_paste_received               (GtkClipboard   *clipboard,
                                                         const gchar    *text,
                                                         gpointer        data);


G_DEFINE_TYPE (PicmanToolbox, picman_toolbox, PICMAN_TYPE_DOCK)

#define parent_class picman_toolbox_parent_class


static void
picman_toolbox_class_init (PicmanToolboxClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanDockClass  *dock_class   = PICMAN_DOCK_CLASS (klass);

  object_class->constructed           = picman_toolbox_constructed;
  object_class->dispose               = picman_toolbox_dispose;
  object_class->set_property          = picman_toolbox_set_property;
  object_class->get_property          = picman_toolbox_get_property;

  widget_class->size_allocate         = picman_toolbox_size_allocate;
  widget_class->button_press_event    = picman_toolbox_button_press_event;

  dock_class->get_description         = picman_toolbox_get_description;
  dock_class->set_host_geometry_hints = picman_toolbox_set_host_geometry_hints;
  dock_class->book_added              = picman_toolbox_book_added;
  dock_class->book_removed            = picman_toolbox_book_removed;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (PicmanToolboxPrivate));
}

static void
picman_toolbox_init (PicmanToolbox *toolbox)
{
  toolbox->p = G_TYPE_INSTANCE_GET_PRIVATE (toolbox,
                                            PICMAN_TYPE_TOOLBOX,
                                            PicmanToolboxPrivate);

  picman_help_connect (GTK_WIDGET (toolbox), picman_standard_help_func,
                     PICMAN_HELP_TOOLBOX, NULL);
}

static void
picman_toolbox_constructed (GObject *object)
{
  PicmanToolbox   *toolbox = PICMAN_TOOLBOX (object);
  PicmanGuiConfig *config;
  GtkWidget     *main_vbox;
  GdkDisplay    *display;
  GList         *list;

  g_assert (PICMAN_IS_CONTEXT (toolbox->p->context));

  config = PICMAN_GUI_CONFIG (toolbox->p->context->picman->config);

  main_vbox = picman_dock_get_main_vbox (PICMAN_DOCK (toolbox));

  toolbox->p->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), toolbox->p->vbox, FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (main_vbox), toolbox->p->vbox, 0);
  gtk_widget_show (toolbox->p->vbox);

  /* Use g_signal_connect() also for the toolbox itself so we can pass
   * data and reuse the same function for the vbox
   */
  g_signal_connect (toolbox, "drag-leave",
                    G_CALLBACK (picman_toolbox_drag_leave),
                    toolbox);
  g_signal_connect (toolbox, "drag-motion",
                    G_CALLBACK (picman_toolbox_drag_motion),
                    toolbox);
  g_signal_connect (toolbox, "drag-drop",
                    G_CALLBACK (picman_toolbox_drag_drop),
                    toolbox);
  g_signal_connect (toolbox->p->vbox, "drag-leave",
                    G_CALLBACK (picman_toolbox_drag_leave),
                    toolbox);
  g_signal_connect (toolbox->p->vbox, "drag-motion",
                    G_CALLBACK (picman_toolbox_drag_motion),
                    toolbox);
  g_signal_connect (toolbox->p->vbox, "drag-drop",
                    G_CALLBACK (picman_toolbox_drag_drop),
                    toolbox);

  toolbox->p->header = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (toolbox->p->header), GTK_SHADOW_NONE);
  gtk_box_pack_start (GTK_BOX (toolbox->p->vbox), toolbox->p->header,
                      FALSE, FALSE, 0);

  g_object_bind_property (config,             "toolbox-wilber",
                          toolbox->p->header, "visible",
                          G_BINDING_SYNC_CREATE);

  g_signal_connect (toolbox->p->header, "size-request",
                    G_CALLBACK (picman_toolbox_size_request_wilber),
                    toolbox);
  g_signal_connect (toolbox->p->header, "expose-event",
                    G_CALLBACK (picman_toolbox_expose_wilber),
                    toolbox);

  picman_help_set_help_data (toolbox->p->header,
                           _("Drop image files here to open them"), NULL);

  toolbox->p->tool_palette = picman_tool_palette_new ();
  picman_tool_palette_set_toolbox (PICMAN_TOOL_PALETTE (toolbox->p->tool_palette),
                                 toolbox);
  gtk_box_pack_start (GTK_BOX (toolbox->p->vbox), toolbox->p->tool_palette,
                      FALSE, FALSE, 0);
  gtk_widget_show (toolbox->p->tool_palette);

  toolbox->p->area_wbox = gtk_hwrap_box_new (FALSE);
  gtk_wrap_box_set_justify (GTK_WRAP_BOX (toolbox->p->area_wbox), GTK_JUSTIFY_TOP);
  gtk_wrap_box_set_line_justify (GTK_WRAP_BOX (toolbox->p->area_wbox),
                                 GTK_JUSTIFY_LEFT);
  gtk_wrap_box_set_aspect_ratio (GTK_WRAP_BOX (toolbox->p->area_wbox),
                                 2.0 / 15.0);

  gtk_box_pack_start (GTK_BOX (toolbox->p->vbox), toolbox->p->area_wbox,
                      FALSE, FALSE, 0);
  gtk_widget_show (toolbox->p->area_wbox);

  /* We need to know when the current device changes, so we can update
   * the correct tool - to do this we connect to motion events.
   * We can't just use EXTENSION_EVENTS_CURSOR though, since that
   * would get us extension events for the mouse pointer, and our
   * device would change to that and not change back. So we check
   * manually that all devices have a cursor, before establishing the check.
   */
  display = gtk_widget_get_display (GTK_WIDGET (toolbox));
  for (list = gdk_display_list_devices (display); list; list = list->next)
    if (! ((GdkDevice *) (list->data))->has_cursor)
      break;

  if (! list)  /* all devices have cursor */
    {
      gtk_widget_add_events (GTK_WIDGET (toolbox), GDK_POINTER_MOTION_MASK);
      picman_devices_add_widget (toolbox->p->context->picman, GTK_WIDGET (toolbox));
    }

  toolbox->p->color_area = toolbox_create_color_area (toolbox,
                                                      toolbox->p->context);
  gtk_wrap_box_pack_wrapped (GTK_WRAP_BOX (toolbox->p->area_wbox),
                             toolbox->p->color_area,
                             TRUE, TRUE, FALSE, TRUE, TRUE);
  if (config->toolbox_color_area)
    gtk_widget_show (toolbox->p->color_area);

  g_signal_connect_object (config, "notify::toolbox-color-area",
                           G_CALLBACK (toolbox_area_notify),
                           toolbox->p->color_area, 0);

  toolbox->p->foo_area = toolbox_create_foo_area (toolbox, toolbox->p->context);
  gtk_wrap_box_pack (GTK_WRAP_BOX (toolbox->p->area_wbox), toolbox->p->foo_area,
                     TRUE, TRUE, FALSE, TRUE);
  if (config->toolbox_foo_area)
    gtk_widget_show (toolbox->p->foo_area);

  g_signal_connect_object (config, "notify::toolbox-foo-area",
                           G_CALLBACK (toolbox_area_notify),
                           toolbox->p->foo_area, 0);

  toolbox->p->image_area = toolbox_create_image_area (toolbox,
                                                      toolbox->p->context);
  gtk_wrap_box_pack (GTK_WRAP_BOX (toolbox->p->area_wbox), toolbox->p->image_area,
                     TRUE, TRUE, FALSE, TRUE);
  if (config->toolbox_image_area)
    gtk_widget_show (toolbox->p->image_area);

  g_signal_connect_object (config, "notify::toolbox-image-area",
                           G_CALLBACK (toolbox_area_notify),
                           toolbox->p->image_area, 0);

  picman_toolbox_dnd_init (PICMAN_TOOLBOX (toolbox), toolbox->p->vbox);
}

static void
picman_toolbox_dispose (GObject *object)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (object);

  toolbox->p->in_destruction = TRUE;

  if (toolbox->p->context)
    {
      g_object_unref (toolbox->p->context);
      toolbox->p->context = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);

  toolbox->p->in_destruction = FALSE;
}

static void
picman_toolbox_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      toolbox->p->context = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_toolbox_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, toolbox->p->context);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_toolbox_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation)
{
  PicmanToolbox    *toolbox = PICMAN_TOOLBOX (widget);
  PicmanGuiConfig  *config;
  GtkRequisition  color_requisition;
  GtkRequisition  foo_requisition;
  GtkRequisition  image_requisition;
  gint            width;
  gint            height;
  gint            n_areas;
  gint            area_rows;
  gint            area_columns;

  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  config = PICMAN_GUI_CONFIG (toolbox->p->context->picman->config);

  gtk_widget_size_request (toolbox->p->color_area, &color_requisition);
  gtk_widget_size_request (toolbox->p->foo_area,   &foo_requisition);
  gtk_widget_size_request (toolbox->p->image_area, &image_requisition);

  width  = MAX (color_requisition.width,
                MAX (foo_requisition.width,
                     image_requisition.width));
  height = MAX (color_requisition.height,
                MAX (foo_requisition.height,
                     image_requisition.height));

  n_areas = (config->toolbox_color_area +
             config->toolbox_foo_area   +
             config->toolbox_image_area);

  area_columns = MAX (1, (allocation->width / width));
  area_rows    = n_areas / area_columns;

  if (n_areas % area_columns)
    area_rows++;

  if (toolbox->p->area_rows    != area_rows  ||
      toolbox->p->area_columns != area_columns)
    {
      toolbox->p->area_rows    = area_rows;
      toolbox->p->area_columns = area_columns;

      gtk_widget_set_size_request (toolbox->p->area_wbox, -1,
                                   area_rows * height);
    }
}

static gboolean
picman_toolbox_button_press_event (GtkWidget      *widget,
                                 GdkEventButton *event)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (widget);

  if (event->type == GDK_BUTTON_PRESS && event->button == 2)
    {
      GtkClipboard *clipboard;

      clipboard = gtk_widget_get_clipboard (widget, GDK_SELECTION_PRIMARY);
      gtk_clipboard_request_text (clipboard,
                                  toolbox_paste_received,
                                  g_object_ref (toolbox->p->context));

      return TRUE;
    }

  return FALSE;
}

static void
picman_toolbox_drag_leave (GtkWidget      *widget,
                         GdkDragContext *context,
                         guint           time,
                         PicmanToolbox    *toolbox)
{
  picman_highlight_widget (widget, FALSE);
}

static gboolean
picman_toolbox_drag_motion (GtkWidget      *widget,
                          GdkDragContext *context,
                          gint            x,
                          gint            y,
                          guint           time,
                          PicmanToolbox    *toolbox)
{
  gboolean other_will_handle = FALSE;
  gboolean we_will_handle    = FALSE;
  gboolean handled           = FALSE;

  other_will_handle = picman_paned_box_will_handle_drag (toolbox->p->drag_handler,
                                                       widget,
                                                       context,
                                                       x, y,
                                                       time);
  we_will_handle = (gtk_drag_dest_find_target (widget, context, NULL) !=
                    GDK_NONE);

  handled = ! other_will_handle && we_will_handle;
  gdk_drag_status (context, handled ? GDK_ACTION_MOVE : 0, time);
  picman_highlight_widget (widget, handled);
  return handled;
}

static gboolean
picman_toolbox_drag_drop (GtkWidget      *widget,
                        GdkDragContext *context,
                        gint            x,
                        gint            y,
                        guint           time,
                        PicmanToolbox    *toolbox)
{
  gboolean handled = FALSE;

  if (picman_paned_box_will_handle_drag (toolbox->p->drag_handler,
                                       widget,
                                       context,
                                       x, y,
                                       time))
    {
      /* Make event fall through to the drag handler */
      handled = FALSE;
    }
  else
    {
      GdkAtom target = gtk_drag_dest_find_target (widget, context, NULL);

      if (target != GDK_NONE)
        {
          /* The URI handlers etc will handle this */
          gtk_drag_get_data (widget, context, target, time);
          handled = TRUE;
        }
    }

  if (handled)
    gtk_drag_finish (context,
                     TRUE /*success*/,
                     (context->action == GDK_ACTION_MOVE) /*del*/,
                     time);

  return handled;
}

static gchar *
picman_toolbox_get_description (PicmanDock *dock,
                              gboolean  complete)
{
  GString *desc      = g_string_new (_("Toolbox"));
  gchar   *dock_desc = PICMAN_DOCK_CLASS (parent_class)->get_description (dock,
                                                                        complete);

  if (dock_desc && strlen (dock_desc) > 0)
    {
      g_string_append (desc, PICMAN_DOCK_BOOK_SEPARATOR);
      g_string_append (desc, dock_desc);
    }

  g_free (dock_desc);

  return g_string_free (desc, FALSE /*free_segment*/);
}

static void
picman_toolbox_book_added (PicmanDock     *dock,
                         PicmanDockbook *dockbook)
{
  if (PICMAN_DOCK_CLASS (parent_class)->book_added)
    PICMAN_DOCK_CLASS (parent_class)->book_added (dock, dockbook);

  if (g_list_length (picman_dock_get_dockbooks (dock)) == 1)
    {
      picman_dock_invalidate_geometry (dock);
    }
}

static void
picman_toolbox_book_removed (PicmanDock     *dock,
                           PicmanDockbook *dockbook)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (dock);

  if (PICMAN_DOCK_CLASS (parent_class)->book_removed)
    PICMAN_DOCK_CLASS (parent_class)->book_removed (dock, dockbook);

  if (! picman_dock_get_dockbooks (dock) &&
      ! toolbox->p->in_destruction)
    {
      picman_dock_invalidate_geometry (dock);
    }
}

static void
picman_toolbox_set_host_geometry_hints (PicmanDock  *dock,
                                      GtkWindow *window)
{
  PicmanToolbox *toolbox = PICMAN_TOOLBOX (dock);
  gint         button_width;
  gint         button_height;

  if (picman_tool_palette_get_button_size (PICMAN_TOOL_PALETTE (toolbox->p->tool_palette),
                                         &button_width, &button_height))
    {
      GdkGeometry geometry;

      geometry.min_width   = 2 * button_width;
      geometry.min_height  = -1;
      geometry.base_width  = button_width;
      geometry.base_height = 0;
      geometry.width_inc   = button_width;
      geometry.height_inc  = 1;

      gtk_window_set_geometry_hints (window,
                                     NULL,
                                     &geometry,
                                     GDK_HINT_MIN_SIZE   |
                                     GDK_HINT_BASE_SIZE  |
                                     GDK_HINT_RESIZE_INC |
                                     GDK_HINT_USER_POS);

      picman_dialog_factory_set_has_min_size (window, TRUE);
    }
}

GtkWidget *
picman_toolbox_new (PicmanDialogFactory *factory,
                  PicmanContext       *context,
                  PicmanUIManager     *ui_manager)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (PICMAN_IS_UI_MANAGER (ui_manager), NULL);

  return g_object_new (PICMAN_TYPE_TOOLBOX,
                       "context", context,
                       NULL);
}

PicmanContext *
picman_toolbox_get_context (PicmanToolbox *toolbox)
{
  g_return_val_if_fail (PICMAN_IS_TOOLBOX (toolbox), NULL);

  return toolbox->p->context;
}

void
picman_toolbox_set_drag_handler (PicmanToolbox  *toolbox,
                               PicmanPanedBox *drag_handler)
{
  g_return_if_fail (PICMAN_IS_TOOLBOX (toolbox));

  toolbox->p->drag_handler = drag_handler;
}


/*  private functions  */

static void
picman_toolbox_size_request_wilber (GtkWidget      *widget,
                                  GtkRequisition *requisition,
                                  PicmanToolbox    *toolbox)
{
  gint button_width;
  gint button_height;

  if (picman_tool_palette_get_button_size (PICMAN_TOOL_PALETTE (toolbox->p->tool_palette),
                                         &button_width, &button_height))
    {
      requisition->width  = button_width  * PANGO_SCALE_SMALL;
      requisition->height = button_height * PANGO_SCALE_SMALL;
    }
  else
    {
      requisition->width  = 16;
      requisition->height = 16;
    }
}

static gboolean
picman_toolbox_expose_wilber (GtkWidget      *widget,
                            GdkEventExpose *event)
{
  cairo_t *cr;

  cr = gdk_cairo_create (gtk_widget_get_window (widget));
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  picman_cairo_draw_toolbox_wilber (widget, cr);

  cairo_destroy (cr);

  return FALSE;
}

static GtkWidget *
toolbox_create_color_area (PicmanToolbox *toolbox,
                           PicmanContext *context)
{
  GtkWidget *alignment;
  GtkWidget *col_area;

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_set_border_width (GTK_CONTAINER (alignment), 2);

  picman_help_set_help_data (alignment, NULL, PICMAN_HELP_TOOLBOX_COLOR_AREA);

  col_area = picman_toolbox_color_area_create (toolbox, 54, 42);
  gtk_container_add (GTK_CONTAINER (alignment), col_area);
  gtk_widget_show (col_area);

  return alignment;
}

static GtkWidget *
toolbox_create_foo_area (PicmanToolbox *toolbox,
                         PicmanContext *context)
{
  GtkWidget *alignment;
  GtkWidget *foo_area;

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_set_border_width (GTK_CONTAINER (alignment), 2);

  picman_help_set_help_data (alignment, NULL, PICMAN_HELP_TOOLBOX_INDICATOR_AREA);

  foo_area = picman_toolbox_indicator_area_create (toolbox);
  gtk_container_add (GTK_CONTAINER (alignment), foo_area);
  gtk_widget_show (foo_area);

  return alignment;
}

static GtkWidget *
toolbox_create_image_area (PicmanToolbox *toolbox,
                           PicmanContext *context)
{
  GtkWidget *alignment;
  GtkWidget *image_area;

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_set_border_width (GTK_CONTAINER (alignment), 2);

  picman_help_set_help_data (alignment, NULL, PICMAN_HELP_TOOLBOX_IMAGE_AREA);

  image_area = picman_toolbox_image_area_create (toolbox, 52, 42);
  gtk_container_add (GTK_CONTAINER (alignment), image_area);
  gtk_widget_show (image_area);

  return alignment;
}

static void
toolbox_area_notify (PicmanGuiConfig *config,
                     GParamSpec    *pspec,
                     GtkWidget     *area)
{
  GtkWidget *parent = gtk_widget_get_parent (area);
  gboolean   visible;

  if (config->toolbox_color_area ||
      config->toolbox_foo_area   ||
      config->toolbox_image_area)
    {
      GtkRequisition req;

      gtk_widget_show (parent);

      gtk_widget_size_request (area, &req);
      gtk_widget_set_size_request (parent, req.width, req.height);
    }
  else
    {
      gtk_widget_hide (parent);
      gtk_widget_set_size_request (parent, -1, -1);
    }

  g_object_get (config, pspec->name, &visible, NULL);
  g_object_set (area, "visible", visible, NULL);
}

static void
toolbox_paste_received (GtkClipboard *clipboard,
                        const gchar  *text,
                        gpointer      data)
{
  PicmanContext *context = PICMAN_CONTEXT (data);

  if (text)
    {
      const gchar *newline = strchr (text, '\n');
      gchar       *copy;

      if (newline)
        copy = g_strndup (text, newline - text);
      else
        copy = g_strdup (text);

      g_strstrip (copy);

      if (strlen (copy))
        {
          PicmanImage         *image;
          PicmanPDBStatusType  status;
          GError            *error = NULL;

          image = file_open_with_display (context->picman, context, NULL,
                                          copy, FALSE, &status, &error);

          if (! image && status != PICMAN_PDB_CANCEL)
            {
              gchar *filename = file_utils_uri_display_name (copy);

              picman_message (context->picman, NULL, PICMAN_MESSAGE_ERROR,
                            _("Opening '%s' failed:\n\n%s"),
                            filename, error->message);

              g_clear_error (&error);
              g_free (filename);
            }
        }

      g_free (copy);
    }

  g_object_unref (context);
}
