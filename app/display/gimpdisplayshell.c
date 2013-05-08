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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"
#include "tools/tools-types.h"

#include "config/picmancoreconfig.h"
#include "config/picmandisplayconfig.h"
#include "config/picmandisplayoptions.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanchannel.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanimage-grid.h"
#include "core/picmanimage-guides.h"
#include "core/picmanimage-snap.h"
#include "core/picmanprojection.h"
#include "core/picmanmarshal.h"
#include "core/picmantemplate.h"

#include "widgets/picmandevices.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "tools/tool_manager.h"

#include "picmancanvas.h"
#include "picmancanvaslayerboundary.h"
#include "picmandisplay.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-callbacks.h"
#include "picmandisplayshell-cursor.h"
#include "picmandisplayshell-dnd.h"
#include "picmandisplayshell-expose.h"
#include "picmandisplayshell-filter.h"
#include "picmandisplayshell-handlers.h"
#include "picmandisplayshell-items.h"
#include "picmandisplayshell-progress.h"
#include "picmandisplayshell-render.h"
#include "picmandisplayshell-rotate.h"
#include "picmandisplayshell-scale.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-selection.h"
#include "picmandisplayshell-title.h"
#include "picmandisplayshell-tool-events.h"
#include "picmandisplayshell-transform.h"
#include "picmanimagewindow.h"
#include "picmanmotionbuffer.h"
#include "picmanstatusbar.h"

#include "about.h"
#include "picman-log.h"

#include "picman-intl.h"


/*  halfway between G_PRIORITY_HIGH_IDLE and G_PRIORITY_DEFAULT_IDLE - 1,
 *  so a bit higher than projection construction
 */
#define PICMAN_DISPLAY_SHELL_FILL_IDLE_PRIORITY \
        ((G_PRIORITY_HIGH_IDLE + G_PRIORITY_DEFAULT_IDLE) / 2 - 1)


enum
{
  PROP_0,
  PROP_POPUP_MANAGER,
  PROP_DISPLAY,
  PROP_UNIT,
  PROP_TITLE,
  PROP_STATUS,
  PROP_ICON
};

enum
{
  SCALED,
  SCROLLED,
  ROTATED,
  RECONNECT,
  LAST_SIGNAL
};


typedef struct _PicmanDisplayShellOverlay PicmanDisplayShellOverlay;

struct _PicmanDisplayShellOverlay
{
  gdouble          image_x;
  gdouble          image_y;
  PicmanHandleAnchor anchor;
  gint             spacing_x;
  gint             spacing_y;
};


/*  local function prototypes  */

static void      picman_color_managed_iface_init     (PicmanColorManagedInterface *iface);

static void      picman_display_shell_constructed    (GObject          *object);
static void      picman_display_shell_dispose        (GObject          *object);
static void      picman_display_shell_finalize       (GObject          *object);
static void      picman_display_shell_set_property   (GObject          *object,
                                                    guint             property_id,
                                                    const GValue     *value,
                                                    GParamSpec       *pspec);
static void      picman_display_shell_get_property   (GObject          *object,
                                                    guint             property_id,
                                                    GValue           *value,
                                                    GParamSpec       *pspec);

static void      picman_display_shell_unrealize      (GtkWidget        *widget);
static void      picman_display_shell_screen_changed (GtkWidget        *widget,
                                                    GdkScreen        *previous);
static gboolean  picman_display_shell_popup_menu     (GtkWidget        *widget);

static void      picman_display_shell_real_scaled    (PicmanDisplayShell *shell);
static void      picman_display_shell_real_rotated   (PicmanDisplayShell *shell);

static const guint8 * picman_display_shell_get_icc_profile
                                                   (PicmanColorManaged *managed,
                                                    gsize            *len);

static void      picman_display_shell_menu_position  (GtkMenu          *menu,
                                                    gint             *x,
                                                    gint             *y,
                                                    gpointer          data);
static void      picman_display_shell_zoom_button_callback
                                                   (PicmanDisplayShell *shell,
                                                    GtkWidget        *zoom_button);
static void      picman_display_shell_sync_config    (PicmanDisplayShell  *shell,
                                                    PicmanDisplayConfig *config);

static void      picman_display_shell_remove_overlay (GtkWidget        *canvas,
                                                    GtkWidget        *child,
                                                    PicmanDisplayShell *shell);
static void   picman_display_shell_transform_overlay (PicmanDisplayShell *shell,
                                                    GtkWidget        *child,
                                                    gdouble          *x,
                                                    gdouble          *y);


G_DEFINE_TYPE_WITH_CODE (PicmanDisplayShell, picman_display_shell,
                         GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROGRESS,
                                                picman_display_shell_progress_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_COLOR_MANAGED,
                                                picman_color_managed_iface_init))


#define parent_class picman_display_shell_parent_class

static guint display_shell_signals[LAST_SIGNAL] = { 0 };


static const gchar display_rc_style[] =
  "style \"check-button-style\"\n"
  "{\n"
  "  GtkToggleButton::child-displacement-x = 0\n"
  "  GtkToggleButton::child-displacement-y = 0\n"
  "}\n"
  "widget \"*\" style \"check-button-style\"";

static void
picman_display_shell_class_init (PicmanDisplayShellClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  display_shell_signals[SCALED] =
    g_signal_new ("scaled",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDisplayShellClass, scaled),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  display_shell_signals[SCROLLED] =
    g_signal_new ("scrolled",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDisplayShellClass, scrolled),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  display_shell_signals[ROTATED] =
    g_signal_new ("rotated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDisplayShellClass, rotated),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  display_shell_signals[RECONNECT] =
    g_signal_new ("reconnect",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDisplayShellClass, reconnect),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->constructed        = picman_display_shell_constructed;
  object_class->dispose            = picman_display_shell_dispose;
  object_class->finalize           = picman_display_shell_finalize;
  object_class->set_property       = picman_display_shell_set_property;
  object_class->get_property       = picman_display_shell_get_property;

  widget_class->unrealize          = picman_display_shell_unrealize;
  widget_class->screen_changed     = picman_display_shell_screen_changed;
  widget_class->popup_menu         = picman_display_shell_popup_menu;

  klass->scaled                    = picman_display_shell_real_scaled;
  klass->scrolled                  = NULL;
  klass->rotated                   = picman_display_shell_real_rotated;
  klass->reconnect                 = NULL;

  g_object_class_install_property (object_class, PROP_POPUP_MANAGER,
                                   g_param_spec_object ("popup-manager",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_UI_MANAGER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DISPLAY,
                                   g_param_spec_object ("display", NULL, NULL,
                                                        PICMAN_TYPE_DISPLAY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_UNIT,
                                   picman_param_spec_unit ("unit", NULL, NULL,
                                                         TRUE, FALSE,
                                                         PICMAN_UNIT_PIXEL,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_TITLE,
                                   g_param_spec_string ("title", NULL, NULL,
                                                        PICMAN_NAME,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_STATUS,
                                   g_param_spec_string ("status", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_ICON,
                                   g_param_spec_object ("icon", NULL, NULL,
                                                        GDK_TYPE_PIXBUF,
                                                        PICMAN_PARAM_READWRITE));

  gtk_rc_parse_string (display_rc_style);
}

static void
picman_color_managed_iface_init (PicmanColorManagedInterface *iface)
{
  iface->get_icc_profile = picman_display_shell_get_icc_profile;
}

static void
picman_display_shell_init (PicmanDisplayShell *shell)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (shell),
                                  GTK_ORIENTATION_VERTICAL);

  shell->options            = g_object_new (PICMAN_TYPE_DISPLAY_OPTIONS, NULL);
  shell->fullscreen_options = g_object_new (PICMAN_TYPE_DISPLAY_OPTIONS_FULLSCREEN, NULL);
  shell->no_image_options   = g_object_new (PICMAN_TYPE_DISPLAY_OPTIONS_NO_IMAGE, NULL);

  shell->zoom        = picman_zoom_model_new ();
  shell->dot_for_dot = TRUE;
  shell->scale_x     = 1.0;
  shell->scale_y     = 1.0;

  picman_display_shell_items_init (shell);

  shell->icon_size  = 32;

  shell->cursor_handedness = PICMAN_HANDEDNESS_RIGHT;
  shell->current_cursor    = (PicmanCursorType) -1;
  shell->tool_cursor       = PICMAN_TOOL_CURSOR_NONE;
  shell->cursor_modifier   = PICMAN_CURSOR_MODIFIER_NONE;
  shell->override_cursor   = (PicmanCursorType) -1;

  shell->motion_buffer   = picman_motion_buffer_new ();

  g_signal_connect (shell->motion_buffer, "stroke",
                    G_CALLBACK (picman_display_shell_buffer_stroke),
                    shell);
  g_signal_connect (shell->motion_buffer, "hover",
                    G_CALLBACK (picman_display_shell_buffer_hover),
                    shell);

  shell->zoom_focus_pointer_queue = g_queue_new ();

  gtk_widget_set_events (GTK_WIDGET (shell), (GDK_POINTER_MOTION_MASK    |
                                              GDK_BUTTON_PRESS_MASK      |
                                              GDK_KEY_PRESS_MASK         |
                                              GDK_KEY_RELEASE_MASK       |
                                              GDK_FOCUS_CHANGE_MASK      |
                                              GDK_VISIBILITY_NOTIFY_MASK |
                                              GDK_SCROLL_MASK));

  /*  zoom model callback  */
  g_signal_connect_swapped (shell->zoom, "zoomed",
                            G_CALLBACK (picman_display_shell_scale_changed),
                            shell);

  /*  active display callback  */
  g_signal_connect (shell, "button-press-event",
                    G_CALLBACK (picman_display_shell_events),
                    shell);
  g_signal_connect (shell, "button-release-event",
                    G_CALLBACK (picman_display_shell_events),
                    shell);
  g_signal_connect (shell, "key-press-event",
                    G_CALLBACK (picman_display_shell_events),
                    shell);

  picman_help_connect (GTK_WIDGET (shell), picman_standard_help_func,
                     PICMAN_HELP_IMAGE_WINDOW, NULL);
}

static void
picman_display_shell_constructed (GObject *object)
{
  PicmanDisplayShell      *shell = PICMAN_DISPLAY_SHELL (object);
  PicmanDisplayConfig     *config;
  PicmanImage             *image;
  PicmanColorDisplayStack *filter;
  GtkWidget             *upper_hbox;
  GtkWidget             *right_vbox;
  GtkWidget             *lower_hbox;
  GtkWidget             *inner_table;
  GtkWidget             *gtk_image;
  GdkScreen             *screen;
  GtkAction             *action;
  gint                   image_width;
  gint                   image_height;
  gint                   shell_width;
  gint                   shell_height;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  g_assert (PICMAN_IS_UI_MANAGER (shell->popup_manager));
  g_assert (PICMAN_IS_DISPLAY (shell->display));

  config = shell->display->config;
  image  = picman_display_get_image (shell->display);

  if (image)
    {
      image_width  = picman_image_get_width  (image);
      image_height = picman_image_get_height (image);
    }
  else
    {
      /* These values are arbitrary. The width is determined by the
       * menubar and the height is chosen to give a window aspect
       * ratio of roughly 3:1 (as requested by the UI team).
       */
      image_width  = PICMAN_DEFAULT_IMAGE_WIDTH;
      image_height = PICMAN_DEFAULT_IMAGE_HEIGHT / 3;
    }

  shell->dot_for_dot = config->default_dot_for_dot;

  screen = gtk_widget_get_screen (GTK_WIDGET (shell));

  if (config->monitor_res_from_gdk)
    {
      picman_get_screen_resolution (screen,
                                  &shell->monitor_xres, &shell->monitor_yres);
    }
  else
    {
      shell->monitor_xres = config->monitor_xres;
      shell->monitor_yres = config->monitor_yres;
    }

  /* adjust the initial scale -- so that window fits on screen. */
  if (image)
    {
      picman_display_shell_set_initial_scale (shell, 1.0, //scale,
                                            &shell_width, &shell_height);
    }
  else
    {
      shell_width  = -1;
      shell_height = image_height;
    }

  picman_display_shell_sync_config (shell, config);

  /*  GtkTable widgets are not able to shrink a row/column correctly if
   *  widgets are attached with GTK_EXPAND even if those widgets have
   *  other rows/columns in their rowspan/colspan where they could
   *  nicely expand without disturbing the row/column which is supposed
   *  to shrink. --Mitch
   *
   *  Changed the packing to use hboxes and vboxes which behave nicer:
   *
   *  shell
   *     |
   *     +-- upper_hbox
   *     |      |
   *     |      +-- inner_table
   *     |      |      |
   *     |      |      +-- origin
   *     |      |      +-- hruler
   *     |      |      +-- vruler
   *     |      |      +-- canvas
   *     |      |
   *     |      +-- right_vbox
   *     |             |
   *     |             +-- zoom_on_resize_button
   *     |             +-- vscrollbar
   *     |
   *     +-- lower_hbox
   *     |      |
   *     |      +-- quick_mask
   *     |      +-- hscrollbar
   *     |      +-- navbutton
   *     |
   *     +-- statusbar
   */

  /*  first, set up the container hierarchy  *********************************/

  /*  a hbox for the inner_table and the vertical scrollbar  */
  upper_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (shell), upper_hbox, TRUE, TRUE, 0);
  gtk_widget_show (upper_hbox);

  /*  the table containing origin, rulers and the canvas  */
  inner_table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacing (GTK_TABLE (inner_table), 0, 0);
  gtk_table_set_row_spacing (GTK_TABLE (inner_table), 0, 0);
  gtk_box_pack_start (GTK_BOX (upper_hbox), inner_table, TRUE, TRUE, 0);
  gtk_widget_show (inner_table);

  /*  the vbox containing the color button and the vertical scrollbar  */
  right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
  gtk_box_pack_start (GTK_BOX (upper_hbox), right_vbox, FALSE, FALSE, 0);
  gtk_widget_show (right_vbox);

  /*  the hbox containing the quickmask button, vertical scrollbar and
   *  the navigation button
   */
  lower_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
  gtk_box_pack_start (GTK_BOX (shell), lower_hbox, FALSE, FALSE, 0);
  gtk_widget_show (lower_hbox);

  /*  create the scrollbars  *************************************************/

  /*  the horizontal scrollbar  */
  shell->hsbdata = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, image_width,
                                                       1, 1, image_width));
  shell->hsb = gtk_scrollbar_new (GTK_ORIENTATION_HORIZONTAL, shell->hsbdata);
  gtk_widget_set_can_focus (shell->hsb, FALSE);

  /*  the vertical scrollbar  */
  shell->vsbdata = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, image_height,
                                                       1, 1, image_height));
  shell->vsb = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, shell->vsbdata);
  gtk_widget_set_can_focus (shell->vsb, FALSE);

  /*  create the contents of the inner_table  ********************************/

  /*  the menu popup button  */
  shell->origin = gtk_event_box_new ();

  gtk_image = gtk_image_new_from_stock (PICMAN_STOCK_MENU_RIGHT,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (shell->origin), gtk_image);
  gtk_widget_show (gtk_image);

  g_signal_connect (shell->origin, "button-press-event",
                    G_CALLBACK (picman_display_shell_origin_button_press),
                    shell);

  picman_help_set_help_data (shell->origin,
                           _("Access the image menu"),
                           PICMAN_HELP_IMAGE_WINDOW_ORIGIN);

  shell->canvas = picman_canvas_new (config);
  gtk_widget_set_size_request (shell->canvas, shell_width, shell_height);
  gtk_container_set_border_width (GTK_CONTAINER (shell->canvas), 10);

  g_signal_connect (shell->canvas, "remove",
                    G_CALLBACK (picman_display_shell_remove_overlay),
                    shell);

  picman_display_shell_dnd_init (shell);
  picman_display_shell_selection_init (shell);

  /*  the horizontal ruler  */
  shell->hrule = picman_ruler_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_set_events (GTK_WIDGET (shell->hrule),
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  picman_ruler_add_track_widget (PICMAN_RULER (shell->hrule), shell->canvas);
  g_signal_connect (shell->hrule, "button-press-event",
                    G_CALLBACK (picman_display_shell_hruler_button_press),
                    shell);

  picman_help_set_help_data (shell->hrule, NULL, PICMAN_HELP_IMAGE_WINDOW_RULER);

  /*  the vertical ruler  */
  shell->vrule = picman_ruler_new (GTK_ORIENTATION_VERTICAL);
  gtk_widget_set_events (GTK_WIDGET (shell->vrule),
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  picman_ruler_add_track_widget (PICMAN_RULER (shell->vrule), shell->canvas);
  g_signal_connect (shell->vrule, "button-press-event",
                    G_CALLBACK (picman_display_shell_vruler_button_press),
                    shell);

  /*  set the rulers as track widgets for each other, so we don't end up
   *  with one ruler wrongly being stuck a few pixels off while we are
   *  hovering the other
   */
  picman_ruler_add_track_widget (PICMAN_RULER (shell->hrule), shell->vrule);
  picman_ruler_add_track_widget (PICMAN_RULER (shell->vrule), shell->hrule);

  picman_help_set_help_data (shell->vrule, NULL, PICMAN_HELP_IMAGE_WINDOW_RULER);

  picman_devices_add_widget (shell->display->picman, shell->hrule);
  picman_devices_add_widget (shell->display->picman, shell->vrule);

  g_signal_connect (shell->canvas, "realize",
                    G_CALLBACK (picman_display_shell_canvas_realize),
                    shell);
  g_signal_connect (shell->canvas, "size-allocate",
                    G_CALLBACK (picman_display_shell_canvas_size_allocate),
                    shell);
  g_signal_connect (shell->canvas, "expose-event",
                    G_CALLBACK (picman_display_shell_canvas_expose),
                    shell);

  g_signal_connect (shell->canvas, "enter-notify-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "leave-notify-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "proximity-in-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "proximity-out-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "focus-in-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "focus-out-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "button-press-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "button-release-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "scroll-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "motion-notify-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "key-press-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);
  g_signal_connect (shell->canvas, "key-release-event",
                    G_CALLBACK (picman_display_shell_canvas_tool_events),
                    shell);

  /*  create the contents of the right_vbox  *********************************/

  shell->zoom_button = g_object_new (GTK_TYPE_CHECK_BUTTON,
                                     "draw-indicator", FALSE,
                                     "relief",         GTK_RELIEF_NONE,
                                     "width-request",  18,
                                     "height-request", 18,
                                     NULL);
  gtk_widget_set_can_focus (shell->zoom_button, FALSE);

  gtk_image = gtk_image_new_from_stock (PICMAN_STOCK_ZOOM_FOLLOW_WINDOW,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (shell->zoom_button), gtk_image);
  gtk_widget_show (gtk_image);

  picman_help_set_help_data (shell->zoom_button,
                           _("Zoom image when window size changes"),
                           PICMAN_HELP_IMAGE_WINDOW_ZOOM_FOLLOW_BUTTON);

  g_signal_connect_swapped (shell->zoom_button, "toggled",
                            G_CALLBACK (picman_display_shell_zoom_button_callback),
                            shell);

  /*  create the contents of the lower_hbox  *********************************/

  /*  the quick mask button  */
  shell->quick_mask_button = g_object_new (GTK_TYPE_CHECK_BUTTON,
                                           "draw-indicator", FALSE,
                                           "relief",         GTK_RELIEF_NONE,
                                           "width-request",  18,
                                           "height-request", 18,
                                           NULL);
  gtk_widget_set_can_focus (shell->quick_mask_button, FALSE);

  gtk_image = gtk_image_new_from_stock (PICMAN_STOCK_QUICK_MASK_OFF,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (shell->quick_mask_button), gtk_image);
  gtk_widget_show (gtk_image);

  action = picman_ui_manager_find_action (shell->popup_manager,
                                        "quick-mask", "quick-mask-toggle");
  if (action)
    picman_widget_set_accel_help (shell->quick_mask_button, action);
  else
    picman_help_set_help_data (shell->quick_mask_button,
                             _("Toggle Quick Mask"),
                             PICMAN_HELP_IMAGE_WINDOW_QUICK_MASK_BUTTON);

  g_signal_connect (shell->quick_mask_button, "toggled",
                    G_CALLBACK (picman_display_shell_quick_mask_toggled),
                    shell);
  g_signal_connect (shell->quick_mask_button, "button-press-event",
                    G_CALLBACK (picman_display_shell_quick_mask_button_press),
                    shell);

  /*  the navigation window button  */
  shell->nav_ebox = gtk_event_box_new ();

  gtk_image = gtk_image_new_from_stock (PICMAN_STOCK_NAVIGATION,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (shell->nav_ebox), gtk_image);
  gtk_widget_show (gtk_image);

  g_signal_connect (shell->nav_ebox, "button-press-event",
                    G_CALLBACK (picman_display_shell_navigation_button_press),
                    shell);

  picman_help_set_help_data (shell->nav_ebox,
                           _("Navigate the image display"),
                           PICMAN_HELP_IMAGE_WINDOW_NAV_BUTTON);

  /*  the statusbar  ********************************************************/

  shell->statusbar = picman_statusbar_new ();
  picman_statusbar_set_shell (PICMAN_STATUSBAR (shell->statusbar), shell);
  picman_help_set_help_data (shell->statusbar, NULL,
                           PICMAN_HELP_IMAGE_WINDOW_STATUS_BAR);
  gtk_box_pack_end (GTK_BOX (shell), shell->statusbar, FALSE, FALSE, 0);

  /*  pack all the widgets  **************************************************/

  /*  fill the inner_table  */
  gtk_table_attach (GTK_TABLE (inner_table), shell->origin, 0, 1, 0, 1,
                    GTK_FILL, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (inner_table), shell->hrule, 1, 2, 0, 1,
                    GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (inner_table), shell->vrule, 0, 1, 1, 2,
                    GTK_FILL, GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);
  gtk_table_attach (GTK_TABLE (inner_table), shell->canvas, 1, 2, 1, 2,
                    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                    GTK_EXPAND | GTK_SHRINK | GTK_FILL, 0, 0);

  /*  fill the right_vbox  */
  gtk_box_pack_start (GTK_BOX (right_vbox),
                      shell->zoom_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (right_vbox),
                      shell->vsb, TRUE, TRUE, 0);

  /*  fill the lower_hbox  */
  gtk_box_pack_start (GTK_BOX (lower_hbox),
                      shell->quick_mask_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (lower_hbox),
                      shell->hsb, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (lower_hbox),
                      shell->nav_ebox, FALSE, FALSE, 0);

  /*  show everything that is always shown ***********************************/

  gtk_widget_show (GTK_WIDGET (shell->canvas));

  /*  add display filter for color management  */

  filter = picman_display_shell_filter_new (shell,
                                          PICMAN_CORE_CONFIG (config)->color_management);

  if (filter)
    {
      picman_display_shell_filter_set (shell, filter);
      g_object_unref (filter);
    }

  if (image)
    {
      picman_display_shell_connect (shell);

      /* After connecting to the image we want to center it. Since we
       * not even finnished creating the display shell, we can safely
       * assume we will get a size-allocate later.
       */
      picman_display_shell_scroll_center_image_on_next_size_allocate (shell,
                                                                    TRUE,
                                                                    TRUE);
    }
  else
    {
#if 0
      /* Disabled because it sets GDK_POINTER_MOTION_HINT on
       * shell->canvas. For info see Bug 677375
       */
      picman_help_set_help_data (shell->canvas,
                               _("Drop image files here to open them"),
                               NULL);
#endif

      picman_statusbar_empty (PICMAN_STATUSBAR (shell->statusbar));
    }

  /* make sure the information is up-to-date */
  picman_display_shell_scale_changed (shell);
}

static void
picman_display_shell_dispose (GObject *object)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (object);

  if (shell->display && picman_display_get_shell (shell->display))
    picman_display_shell_disconnect (shell);

  shell->popup_manager = NULL;

  picman_display_shell_selection_free (shell);

  if (shell->filter_stack)
    picman_display_shell_filter_set (shell, NULL);

  if (shell->filter_idle_id)
    {
      g_source_remove (shell->filter_idle_id);
      shell->filter_idle_id = 0;
    }

  if (shell->mask_surface)
    {
      cairo_surface_destroy (shell->mask_surface);
      shell->mask_surface = NULL;
    }

  if (shell->checkerboard)
    {
      cairo_pattern_destroy (shell->checkerboard);
      shell->checkerboard = NULL;
    }

  if (shell->mask)
    {
      g_object_unref (shell->mask);
      shell->mask = NULL;
    }

  picman_display_shell_items_free (shell);

  if (shell->motion_buffer)
    {
      g_object_unref (shell->motion_buffer);
      shell->motion_buffer = NULL;
    }

  if (shell->zoom_focus_pointer_queue)
    {
      g_queue_free (shell->zoom_focus_pointer_queue);
      shell->zoom_focus_pointer_queue = NULL;
    }

  if (shell->title_idle_id)
    {
      g_source_remove (shell->title_idle_id);
      shell->title_idle_id = 0;
    }

  if (shell->fill_idle_id)
    {
      g_source_remove (shell->fill_idle_id);
      shell->fill_idle_id = 0;
    }

  if (shell->nav_popup)
    {
      gtk_widget_destroy (shell->nav_popup);
      shell->nav_popup = NULL;
    }

  if (shell->grid_dialog)
    {
      gtk_widget_destroy (shell->grid_dialog);
      shell->grid_dialog = NULL;
    }

  shell->display = NULL;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_display_shell_finalize (GObject *object)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (object);

  g_object_unref (shell->zoom);

  if (shell->rotate_transform)
    g_free (shell->rotate_transform);

  if (shell->rotate_untransform)
    g_free (shell->rotate_untransform);

  if (shell->options)
    g_object_unref (shell->options);

  if (shell->fullscreen_options)
    g_object_unref (shell->fullscreen_options);

  if (shell->no_image_options)
    g_object_unref (shell->no_image_options);

  if (shell->title)
    g_free (shell->title);

  if (shell->status)
    g_free (shell->status);

  if (shell->icon)
    g_object_unref (shell->icon);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_display_shell_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (object);

  switch (property_id)
    {
    case PROP_POPUP_MANAGER:
      shell->popup_manager = g_value_get_object (value);
      break;
    case PROP_DISPLAY:
      shell->display = g_value_get_object (value);
      break;
    case PROP_UNIT:
      picman_display_shell_set_unit (shell, g_value_get_int (value));
      break;
    case PROP_TITLE:
      g_free (shell->title);
      shell->title = g_value_dup_string (value);
      break;
    case PROP_STATUS:
      g_free (shell->status);
      shell->status = g_value_dup_string (value);
      break;
    case PROP_ICON:
      if (shell->icon)
        g_object_unref (shell->icon);
      shell->icon = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_shell_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (object);

  switch (property_id)
    {
    case PROP_POPUP_MANAGER:
      g_value_set_object (value, shell->popup_manager);
      break;
    case PROP_DISPLAY:
      g_value_set_object (value, shell->display);
      break;
    case PROP_UNIT:
      g_value_set_int (value, shell->unit);
      break;
    case PROP_TITLE:
      g_value_set_string (value, shell->title);
      break;
    case PROP_STATUS:
      g_value_set_string (value, shell->status);
      break;
    case PROP_ICON:
      g_value_set_object (value, shell->icon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_display_shell_unrealize (GtkWidget *widget)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (widget);

  if (shell->nav_popup)
    gtk_widget_unrealize (shell->nav_popup);

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_display_shell_screen_changed (GtkWidget *widget,
                                   GdkScreen *previous)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (widget);

  if (GTK_WIDGET_CLASS (parent_class)->screen_changed)
    GTK_WIDGET_CLASS (parent_class)->screen_changed (widget, previous);

  if (shell->display->config->monitor_res_from_gdk)
    {
      picman_get_screen_resolution (gtk_widget_get_screen (widget),
                                  &shell->monitor_xres,
                                  &shell->monitor_yres);
    }
  else
    {
      shell->monitor_xres = shell->display->config->monitor_xres;
      shell->monitor_yres = shell->display->config->monitor_yres;
    }
}

static gboolean
picman_display_shell_popup_menu (GtkWidget *widget)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (widget);

  picman_context_set_display (picman_get_user_context (shell->display->picman),
                            shell->display);

  picman_ui_manager_ui_popup (shell->popup_manager, "/dummy-menubar/image-popup",
                            GTK_WIDGET (shell),
                            picman_display_shell_menu_position,
                            shell->origin,
                            NULL, NULL);

  return TRUE;
}

static void
picman_display_shell_real_scaled (PicmanDisplayShell *shell)
{
  PicmanContext *user_context;

  if (! shell->display)
    return;

  picman_display_shell_title_update (shell);

  user_context = picman_get_user_context (shell->display->picman);

  if (shell->display == picman_context_get_display (user_context))
    picman_ui_manager_update (shell->popup_manager, shell->display);
}

static void
picman_display_shell_real_rotated (PicmanDisplayShell *shell)
{
  PicmanContext *user_context;

  if (! shell->display)
    return;

  user_context = picman_get_user_context (shell->display->picman);

  if (shell->display == picman_context_get_display (user_context))
    picman_ui_manager_update (shell->popup_manager, shell->display);
}

static const guint8 *
picman_display_shell_get_icc_profile (PicmanColorManaged *managed,
                                    gsize            *len)
{
  PicmanDisplayShell *shell = PICMAN_DISPLAY_SHELL (managed);
  PicmanImage        *image = picman_display_get_image (shell->display);

  if (image)
    return picman_color_managed_get_icc_profile (PICMAN_COLOR_MANAGED (image), len);

  return NULL;
}

static void
picman_display_shell_menu_position (GtkMenu  *menu,
                                  gint     *x,
                                  gint     *y,
                                  gpointer  data)
{
  picman_button_menu_position (GTK_WIDGET (data), menu, GTK_POS_RIGHT, x, y);
}

static void
picman_display_shell_zoom_button_callback (PicmanDisplayShell *shell,
                                         GtkWidget        *zoom_button)
{
  shell->zoom_on_resize =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (zoom_button));

  if (shell->zoom_on_resize &&
      picman_display_shell_scale_image_is_within_viewport (shell, NULL, NULL))
    {
      /* Implicitly make a View -> Fit Image in Window */
      picman_display_shell_scale_fit_in (shell);
    }
}

static void
picman_display_shell_sync_config (PicmanDisplayShell  *shell,
                                PicmanDisplayConfig *config)
{
  picman_config_sync (G_OBJECT (config->default_view),
                    G_OBJECT (shell->options), 0);
  picman_config_sync (G_OBJECT (config->default_fullscreen_view),
                    G_OBJECT (shell->fullscreen_options), 0);

  if (shell->display && picman_display_get_shell (shell->display))
    {
      /*  if the shell is already fully constructed, use proper API
       *  so the actions are updated accordingly.
       */
      picman_display_shell_set_snap_to_guides  (shell,
                                              config->default_snap_to_guides);
      picman_display_shell_set_snap_to_grid    (shell,
                                              config->default_snap_to_grid);
      picman_display_shell_set_snap_to_canvas  (shell,
                                              config->default_snap_to_canvas);
      picman_display_shell_set_snap_to_vectors (shell,
                                              config->default_snap_to_path);
    }
  else
    {
      /*  otherwise the shell is currently being constructed and
       *  display->shell is NULL.
       */
      shell->snap_to_guides  = config->default_snap_to_guides;
      shell->snap_to_grid    = config->default_snap_to_grid;
      shell->snap_to_canvas  = config->default_snap_to_canvas;
      shell->snap_to_vectors = config->default_snap_to_path;
    }
}

static void
picman_display_shell_remove_overlay (GtkWidget        *canvas,
                                   GtkWidget        *child,
                                   PicmanDisplayShell *shell)
{
  shell->children = g_list_remove (shell->children, child);
}

static void
picman_display_shell_transform_overlay (PicmanDisplayShell *shell,
                                      GtkWidget        *child,
                                      gdouble          *x,
                                      gdouble          *y)
{
  PicmanDisplayShellOverlay *overlay;
  GtkRequisition           requisition;

  overlay = g_object_get_data (G_OBJECT (child), "image-coords-overlay");

  picman_display_shell_transform_xy_f (shell,
                                     overlay->image_x,
                                     overlay->image_y,
                                     x, y);

  gtk_widget_size_request (child, &requisition);

  switch (overlay->anchor)
    {
    case PICMAN_HANDLE_ANCHOR_CENTER:
      *x -= requisition.width  / 2;
      *y -= requisition.height / 2;
      break;

    case PICMAN_HANDLE_ANCHOR_NORTH:
      *x -= requisition.width / 2;
      *y += overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_NORTH_WEST:
      *x += overlay->spacing_x;
      *y += overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_NORTH_EAST:
      *x -= requisition.width + overlay->spacing_x;
      *y += overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH:
      *x -= requisition.width / 2;
      *y -= requisition.height + overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH_WEST:
      *x += overlay->spacing_x;
      *y -= requisition.height + overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_SOUTH_EAST:
      *x -= requisition.width + overlay->spacing_x;
      *y -= requisition.height + overlay->spacing_y;
      break;

    case PICMAN_HANDLE_ANCHOR_WEST:
      *x += overlay->spacing_x;
      *y -= requisition.height / 2;
      break;

    case PICMAN_HANDLE_ANCHOR_EAST:
      *x -= requisition.width + overlay->spacing_x;
      *y -= requisition.height / 2;
      break;
    }
}


/*  public functions  */

GtkWidget *
picman_display_shell_new (PicmanDisplay       *display,
                        PicmanUnit           unit,
                        gdouble            scale,
                        PicmanUIManager     *popup_manager)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY (display), NULL);
  g_return_val_if_fail (PICMAN_IS_UI_MANAGER (popup_manager), NULL);

  return g_object_new (PICMAN_TYPE_DISPLAY_SHELL,
                       "popup-manager", popup_manager,
                       "display",       display,
                       "unit",          unit,
                       NULL);
}

void
picman_display_shell_add_overlay (PicmanDisplayShell *shell,
                                GtkWidget        *child,
                                gdouble           image_x,
                                gdouble           image_y,
                                PicmanHandleAnchor  anchor,
                                gint              spacing_x,
                                gint              spacing_y)
{
  PicmanDisplayShellOverlay *overlay;
  gdouble                  x, y;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (GTK_IS_WIDGET (shell));

  overlay = g_new0 (PicmanDisplayShellOverlay, 1);

  overlay->image_x   = image_x;
  overlay->image_y   = image_y;
  overlay->anchor    = anchor;
  overlay->spacing_x = spacing_x;
  overlay->spacing_y = spacing_y;

  g_object_set_data_full (G_OBJECT (child), "image-coords-overlay", overlay,
                          (GDestroyNotify) g_free);

  shell->children = g_list_prepend (shell->children, child);

  picman_display_shell_transform_overlay (shell, child, &x, &y);

  picman_overlay_box_add_child (PICMAN_OVERLAY_BOX (shell->canvas), child, 0.0, 0.0);
  picman_overlay_box_set_child_position (PICMAN_OVERLAY_BOX (shell->canvas),
                                       child, x, y);
}

void
picman_display_shell_move_overlay (PicmanDisplayShell *shell,
                                 GtkWidget        *child,
                                 gdouble           image_x,
                                 gdouble           image_y,
                                 PicmanHandleAnchor  anchor,
                                 gint              spacing_x,
                                 gint              spacing_y)
{
  PicmanDisplayShellOverlay *overlay;
  gdouble                  x, y;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (GTK_IS_WIDGET (shell));

  overlay = g_object_get_data (G_OBJECT (child), "image-coords-overlay");

  g_return_if_fail (overlay != NULL);

  overlay->image_x   = image_x;
  overlay->image_y   = image_y;
  overlay->anchor    = anchor;
  overlay->spacing_x = spacing_x;
  overlay->spacing_y = spacing_y;

  picman_display_shell_transform_overlay (shell, child, &x, &y);

  picman_overlay_box_set_child_position (PICMAN_OVERLAY_BOX (shell->canvas),
                                       child, x, y);
}

PicmanImageWindow *
picman_display_shell_get_window (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return PICMAN_IMAGE_WINDOW (gtk_widget_get_ancestor (GTK_WIDGET (shell),
                                                     PICMAN_TYPE_IMAGE_WINDOW));
}

PicmanStatusbar *
picman_display_shell_get_statusbar (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), NULL);

  return PICMAN_STATUSBAR (shell->statusbar);
}

void
picman_display_shell_present (PicmanDisplayShell *shell)
{
  PicmanImageWindow *window;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  window = picman_display_shell_get_window (shell);

  if (window)
    {
      picman_image_window_set_active_shell (window, shell);

      gtk_window_present (GTK_WINDOW (window));
    }
}

void
picman_display_shell_reconnect (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_DISPLAY (shell->display));
  g_return_if_fail (picman_display_get_image (shell->display) != NULL);

  if (shell->fill_idle_id)
    {
      g_source_remove (shell->fill_idle_id);
      shell->fill_idle_id = 0;
    }

  g_signal_emit (shell, display_shell_signals[RECONNECT], 0);

  picman_color_managed_profile_changed (PICMAN_COLOR_MANAGED (shell));

  picman_display_shell_scroll_clamp_and_update (shell);

  picman_display_shell_scaled (shell);

  picman_display_shell_expose_full (shell);
}

void
picman_display_shell_empty (PicmanDisplayShell *shell)
{
  PicmanContext *user_context;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_DISPLAY (shell->display));
  g_return_if_fail (picman_display_get_image (shell->display) == NULL);

  if (shell->fill_idle_id)
    {
      g_source_remove (shell->fill_idle_id);
      shell->fill_idle_id = 0;
    }

  picman_display_shell_selection_undraw (shell);

  picman_display_shell_unset_cursor (shell);

  picman_display_shell_sync_config (shell, shell->display->config);

  picman_display_shell_appearance_update (shell);
#if 0
  picman_help_set_help_data (shell->canvas,
                           _("Drop image files here to open them"), NULL);
#endif

  picman_statusbar_empty (PICMAN_STATUSBAR (shell->statusbar));

  shell->rotate_angle = 0.0;
  picman_display_shell_rotate_update_transform (shell);

  picman_display_shell_expose_full (shell);

  user_context = picman_get_user_context (shell->display->picman);

  if (shell->display == picman_context_get_display (user_context))
    picman_ui_manager_update (shell->popup_manager, shell->display);
}

static gboolean
picman_display_shell_fill_idle (PicmanDisplayShell *shell)
{
  GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (shell));

  shell->fill_idle_id = 0;

  if (GTK_IS_WINDOW (toplevel))
    {
      picman_display_shell_scale_shrink_wrap (shell, TRUE);

      gtk_window_present (GTK_WINDOW (toplevel));
    }

  return FALSE;
}

void
picman_display_shell_fill (PicmanDisplayShell *shell,
                         PicmanImage        *image,
                         PicmanUnit          unit,
                         gdouble           scale)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (PICMAN_IS_DISPLAY (shell->display));
  g_return_if_fail (PICMAN_IS_IMAGE (image));

  picman_display_shell_set_unit (shell, unit);
  picman_display_shell_set_initial_scale (shell, scale, NULL, NULL);
  picman_display_shell_scale_changed (shell);

  picman_display_shell_sync_config (shell, shell->display->config);

  picman_display_shell_appearance_update (shell);
#if 0
  picman_help_set_help_data (shell->canvas, NULL, NULL);
#endif

  picman_statusbar_fill (PICMAN_STATUSBAR (shell->statusbar));

  /* A size-allocate will always occur because the scrollbars will
   * become visible forcing the canvas to become smaller
   */
  picman_display_shell_scroll_center_image_on_next_size_allocate (shell,
                                                                TRUE,
                                                                TRUE);

  shell->fill_idle_id =
    g_idle_add_full (PICMAN_DISPLAY_SHELL_FILL_IDLE_PRIORITY,
                     (GSourceFunc) picman_display_shell_fill_idle, shell,
                     NULL);
}

/* We used to calculate the scale factor in the SCALEFACTOR_X() and
 * SCALEFACTOR_Y() macros. But since these are rather frequently
 * called and the values rarely change, we now store them in the
 * shell and call this function whenever they need to be recalculated.
 */
void
picman_display_shell_scale_changed (PicmanDisplayShell *shell)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  image = picman_display_get_image (shell->display);

  if (image)
    {
      picman_display_shell_calculate_scale_x_and_y (shell,
                                                  picman_zoom_model_get_factor (shell->zoom),
                                                  &shell->scale_x,
                                                  &shell->scale_y);
    }
  else
    {
      shell->scale_x = 1.0;
      shell->scale_y = 1.0;
    }
}

void
picman_display_shell_scaled (PicmanDisplayShell *shell)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_rotate_update_transform (shell);

  for (list = shell->children; list; list = g_list_next (list))
    {
      GtkWidget *child = list->data;
      gdouble    x, y;

      picman_display_shell_transform_overlay (shell, child, &x, &y);

      picman_overlay_box_set_child_position (PICMAN_OVERLAY_BOX (shell->canvas),
                                           child, x, y);
    }

  g_signal_emit (shell, display_shell_signals[SCALED], 0);
}

void
picman_display_shell_scrolled (PicmanDisplayShell *shell)
{
  GList *list;

  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_rotate_update_transform (shell);

  for (list = shell->children; list; list = g_list_next (list))
    {
      GtkWidget *child = list->data;
      gdouble    x, y;

      picman_display_shell_transform_overlay (shell, child, &x, &y);

      picman_overlay_box_set_child_position (PICMAN_OVERLAY_BOX (shell->canvas),
                                           child, x, y);
    }

  g_signal_emit (shell, display_shell_signals[SCROLLED], 0);
}

void
picman_display_shell_rotated (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_rotate_update_transform (shell);

  g_signal_emit (shell, display_shell_signals[ROTATED], 0);
}

void
picman_display_shell_set_unit (PicmanDisplayShell *shell,
                             PicmanUnit          unit)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (shell->unit != unit)
    {
      shell->unit = unit;

      picman_display_shell_scale_update_rulers (shell);

      picman_display_shell_scaled (shell);

      g_object_notify (G_OBJECT (shell), "unit");
    }
}

PicmanUnit
picman_display_shell_get_unit (PicmanDisplayShell *shell)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), PICMAN_UNIT_PIXEL);

  return shell->unit;
}

gboolean
picman_display_shell_snap_coords (PicmanDisplayShell *shell,
                                PicmanCoords       *coords,
                                gint              snap_offset_x,
                                gint              snap_offset_y,
                                gint              snap_width,
                                gint              snap_height)
{
  PicmanImage *image;
  gboolean   snap_to_guides  = FALSE;
  gboolean   snap_to_grid    = FALSE;
  gboolean   snap_to_canvas  = FALSE;
  gboolean   snap_to_vectors = FALSE;
  gboolean   snapped         = FALSE;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);
  g_return_val_if_fail (coords != NULL, FALSE);

  image = picman_display_get_image (shell->display);

  if (picman_display_shell_get_snap_to_guides (shell) &&
      picman_image_get_guides (image))
    {
      snap_to_guides = TRUE;
    }

  if (picman_display_shell_get_snap_to_grid (shell) &&
      picman_image_get_grid (image))
    {
      snap_to_grid = TRUE;
    }

  snap_to_canvas = picman_display_shell_get_snap_to_canvas (shell);

  if (picman_display_shell_get_snap_to_vectors (shell) &&
      picman_image_get_active_vectors (image))
    {
      snap_to_vectors = TRUE;
    }

  if (snap_to_guides || snap_to_grid || snap_to_canvas || snap_to_vectors)
    {
      gint    snap_distance;
      gdouble tx, ty;

      snap_distance = shell->display->config->snap_distance;

      if (snap_width > 0 && snap_height > 0)
        {
          snapped = picman_image_snap_rectangle (image,
                                               coords->x + snap_offset_x,
                                               coords->y + snap_offset_y,
                                               coords->x + snap_offset_x +
                                               snap_width,
                                               coords->y + snap_offset_y +
                                               snap_height,
                                               &tx,
                                               &ty,
                                               FUNSCALEX (shell, snap_distance),
                                               FUNSCALEY (shell, snap_distance),
                                               snap_to_guides,
                                               snap_to_grid,
                                               snap_to_canvas,
                                               snap_to_vectors);
        }
      else
        {
          snapped = picman_image_snap_point (image,
                                           coords->x + snap_offset_x,
                                           coords->y + snap_offset_y,
                                           &tx,
                                           &ty,
                                           FUNSCALEX (shell, snap_distance),
                                           FUNSCALEY (shell, snap_distance),
                                           snap_to_guides,
                                           snap_to_grid,
                                           snap_to_canvas,
                                           snap_to_vectors);
        }

      if (snapped)
        {
          coords->x = tx - snap_offset_x;
          coords->y = ty - snap_offset_y;
        }
    }

  return snapped;
}

gboolean
picman_display_shell_mask_bounds (PicmanDisplayShell *shell,
                                gint             *x1,
                                gint             *y1,
                                gint             *x2,
                                gint             *y2)
{
  PicmanImage *image;
  PicmanLayer *layer;
  gdouble    x1_f, y1_f;
  gdouble    x2_f, y2_f;
  gdouble    x3_f, y3_f;
  gdouble    x4_f, y4_f;

  g_return_val_if_fail (PICMAN_IS_DISPLAY_SHELL (shell), FALSE);
  g_return_val_if_fail (x1 != NULL, FALSE);
  g_return_val_if_fail (y1 != NULL, FALSE);
  g_return_val_if_fail (x2 != NULL, FALSE);
  g_return_val_if_fail (y2 != NULL, FALSE);

  image = picman_display_get_image (shell->display);

  /*  If there is a floating selection, handle things differently  */
  if ((layer = picman_image_get_floating_selection (image)))
    {
      gint off_x;
      gint off_y;

      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      if (! picman_channel_bounds (picman_image_get_mask (image),
                                 x1, y1, x2, y2))
        {
          *x1 = off_x;
          *y1 = off_y;
          *x2 = off_x + picman_item_get_width  (PICMAN_ITEM (layer));
          *y2 = off_y + picman_item_get_height (PICMAN_ITEM (layer));
        }
      else
        {
          *x1 = MIN (off_x, *x1);
          *y1 = MIN (off_y, *y1);
          *x2 = MAX (off_x + picman_item_get_width  (PICMAN_ITEM (layer)), *x2);
          *y2 = MAX (off_y + picman_item_get_height (PICMAN_ITEM (layer)), *y2);
        }
    }
  else if (! picman_channel_bounds (picman_image_get_mask (image),
                                  x1, y1, x2, y2))
    {
      return FALSE;
    }

  picman_display_shell_transform_xy_f (shell, *x1, *y1, &x1_f, &y1_f);
  picman_display_shell_transform_xy_f (shell, *x1, *y2, &x2_f, &y2_f);
  picman_display_shell_transform_xy_f (shell, *x2, *y1, &x3_f, &y3_f);
  picman_display_shell_transform_xy_f (shell, *x2, *y2, &x4_f, &y4_f);

  /*  Make sure the extents are within bounds  */
  *x1 = CLAMP (floor (MIN4 (x1_f, x2_f, x3_f, x4_f)), 0, shell->disp_width);
  *y1 = CLAMP (floor (MIN4 (y1_f, y2_f, y3_f, y4_f)), 0, shell->disp_height);
  *x2 = CLAMP (ceil (MAX4 (x1_f, x2_f, x3_f, x4_f)),  0, shell->disp_width);
  *y2 = CLAMP (ceil (MAX4 (y1_f, y2_f, y3_f, y4_f)),  0, shell->disp_height);

  return ((*x2 - *x1) > 0) && ((*y2 - *y1) > 0);
}

void
picman_display_shell_flush (PicmanDisplayShell *shell,
                          gboolean          now)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  picman_display_shell_title_update (shell);

  /* make sure the information is up-to-date */
  picman_display_shell_scale_changed (shell);

  picman_canvas_layer_boundary_set_layer (PICMAN_CANVAS_LAYER_BOUNDARY (shell->layer_boundary),
                                        picman_image_get_active_layer (picman_display_get_image (shell->display)));

  if (now)
    {
      gdk_window_process_updates (gtk_widget_get_window (shell->canvas),
                                  FALSE);
    }
  else
    {
      PicmanImageWindow *window = picman_display_shell_get_window (shell);
      PicmanContext     *context;

      if (window && picman_image_window_get_active_shell (window) == shell)
        {
          PicmanUIManager *manager = picman_image_window_get_ui_manager (window);

          picman_ui_manager_update (manager, shell->display);
        }

      context = picman_get_user_context (shell->display->picman);

      if (shell->display == picman_context_get_display (context))
        {
          picman_ui_manager_update (shell->popup_manager, shell->display);
        }
    }
}

/**
 * picman_display_shell_pause:
 * @shell: a display shell
 *
 * This function increments the pause count or the display shell.
 * If it was zero coming in, then the function pauses the active tool,
 * so that operations on the display can take place without corrupting
 * anything that the tool has drawn.  It "undraws" the current tool
 * drawing, and must be followed by picman_display_shell_resume() after
 * the operation in question is completed.
 **/
void
picman_display_shell_pause (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  shell->paused_count++;

  if (shell->paused_count == 1)
    {
      /*  pause the currently active tool  */
      tool_manager_control_active (shell->display->picman,
                                   PICMAN_TOOL_ACTION_PAUSE,
                                   shell->display);
    }
}

/**
 * picman_display_shell_resume:
 * @shell: a display shell
 *
 * This function decrements the pause count for the display shell.
 * If this brings it to zero, then the current tool is resumed.
 * It is an error to call this function without having previously
 * called picman_display_shell_pause().
 **/
void
picman_display_shell_resume (PicmanDisplayShell *shell)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (shell->paused_count > 0);

  shell->paused_count--;

  if (shell->paused_count == 0)
    {
      /* start the currently active tool */
      tool_manager_control_active (shell->display->picman,
                                   PICMAN_TOOL_ACTION_RESUME,
                                   shell->display);
    }
}

/**
 * picman_display_shell_set_highlight:
 * @shell:     a #PicmanDisplayShell
 * @highlight: a rectangle in image coordinates that should be brought out
 *
 * This function sets an area of the image that should be
 * accentuated. The actual implementation is to dim all pixels outside
 * this rectangle. Passing %NULL for @highlight unsets the rectangle.
 **/
void
picman_display_shell_set_highlight (PicmanDisplayShell   *shell,
                                  const GdkRectangle *highlight)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  if (highlight)
    {
      picman_canvas_item_begin_change (shell->passe_partout);

      picman_canvas_rectangle_set (shell->passe_partout,
                                 highlight->x,
                                 highlight->y,
                                 highlight->width,
                                 highlight->height);

      picman_canvas_item_set_visible (shell->passe_partout, TRUE);

      picman_canvas_item_end_change (shell->passe_partout);
    }
  else
    {
      picman_canvas_item_set_visible (shell->passe_partout, FALSE);
    }
}

/**
 * picman_display_shell_set_mask:
 * @shell: a #PicmanDisplayShell
 * @mask:  a #PicmanDrawable (1 byte per pixel)
 * @color: the color to use for drawing the mask
 *
 * Previews a selection (used by the foreground selection tool).
 * Pixels that are not selected (> 127) in the mask are tinted with
 * the given color.
 **/
void
picman_display_shell_set_mask (PicmanDisplayShell *shell,
                             PicmanDrawable     *mask,
                             const PicmanRGB    *color)
{
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));
  g_return_if_fail (mask == NULL ||
                    (PICMAN_IS_DRAWABLE (mask) &&
                     babl_format_get_bytes_per_pixel (picman_drawable_get_format (mask)) == 1));
  g_return_if_fail (mask == NULL || color != NULL);

  if (mask)
    g_object_ref (mask);

  if (shell->mask)
    g_object_unref (shell->mask);

  shell->mask = mask;

  if (mask)
    shell->mask_color = *color;

  picman_display_shell_expose_full (shell);
}
