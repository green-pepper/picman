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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmanprogress.h"

#include "widgets/picmanactiongroup.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandock.h"
#include "widgets/picmandockbook.h"
#include "widgets/picmandockcolumns.h"
#include "widgets/picmandockcontainer.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmansessioninfo-aux.h"
#include "widgets/picmansessionmanaged.h"
#include "widgets/picmansessioninfo-dock.h"
#include "widgets/picmantoolbox.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanview.h"

#include "picmandisplay.h"
#include "picmandisplay-foreach.h"
#include "picmandisplayshell.h"
#include "picmandisplayshell-appearance.h"
#include "picmandisplayshell-close.h"
#include "picmandisplayshell-scroll.h"
#include "picmandisplayshell-tool-events.h"
#include "picmandisplayshell-transform.h"
#include "picmanimagewindow.h"
#include "picmanstatusbar.h"

#include "picman-log.h"
#include "picman-intl.h"


#define PICMAN_EMPTY_IMAGE_WINDOW_ENTRY_ID   "picman-empty-image-window"
#define PICMAN_SINGLE_IMAGE_WINDOW_ENTRY_ID  "picman-single-image-window"

/* GtkPaned position of the image area, i.e. the width of the left
 * docks area
 */
#define PICMAN_IMAGE_WINDOW_LEFT_DOCKS_WIDTH "left-docks-width"

/* GtkPaned position of the right docks area */
#define PICMAN_IMAGE_WINDOW_RIGHT_DOCKS_POS  "right-docks-position"

/* Whether the window's maximized or not */
#define PICMAN_IMAGE_WINDOW_MAXIMIZED        "maximized"


enum
{
  PROP_0,
  PROP_PICMAN,
  PROP_MENU_FACTORY,
  PROP_DIALOG_FACTORY,
};


typedef struct _PicmanImageWindowPrivate PicmanImageWindowPrivate;

struct _PicmanImageWindowPrivate
{
  Picman              *picman;
  PicmanUIManager     *menubar_manager;
  PicmanDialogFactory *dialog_factory;

  GList             *shells;
  PicmanDisplayShell  *active_shell;

  GtkWidget         *main_vbox;
  GtkWidget         *menubar;
  GtkWidget         *hbox;
  GtkWidget         *left_hpane;
  GtkWidget         *left_docks;
  GtkWidget         *right_hpane;
  GtkWidget         *notebook;
  GtkWidget         *right_docks;

  GdkWindowState     window_state;

  const gchar       *entry_id;
};

typedef struct
{
  PicmanImageWindow *window;
  gint             x;
  gint             y;
} PosCorrectionData;


#define PICMAN_IMAGE_WINDOW_GET_PRIVATE(window) \
        G_TYPE_INSTANCE_GET_PRIVATE (window, \
                                     PICMAN_TYPE_IMAGE_WINDOW, \
                                     PicmanImageWindowPrivate)


/*  local function prototypes  */

static void      picman_image_window_dock_container_iface_init
                                                       (PicmanDockContainerInterface
                                                                            *iface);
static void      picman_image_window_session_managed_iface_init
                                                       (PicmanSessionManagedInterface
                                                                            *iface);
static void      picman_image_window_constructed         (GObject             *object);
static void      picman_image_window_dispose             (GObject             *object);
static void      picman_image_window_finalize            (GObject             *object);
static void      picman_image_window_set_property        (GObject             *object,
                                                        guint                property_id,
                                                        const GValue        *value,
                                                        GParamSpec          *pspec);
static void      picman_image_window_get_property        (GObject             *object,
                                                        guint                property_id,
                                                        GValue              *value,
                                                        GParamSpec          *pspec);

static gboolean  picman_image_window_delete_event        (GtkWidget           *widget,
                                                        GdkEventAny         *event);
static gboolean  picman_image_window_configure_event     (GtkWidget           *widget,
                                                        GdkEventConfigure   *event);
static gboolean  picman_image_window_window_state_event  (GtkWidget           *widget,
                                                        GdkEventWindowState *event);
static void      picman_image_window_style_set           (GtkWidget           *widget,
                                                        GtkStyle            *prev_style);
static GList *   picman_image_window_get_docks           (PicmanDockContainer   *dock_container);
static PicmanUIManager *
                 picman_image_window_dock_container_get_ui_manager
                                                       (PicmanDockContainer   *dock_container);
static void      picman_image_window_add_dock            (PicmanDockContainer   *dock_container,
                                                        PicmanDock            *dock,
                                                        PicmanSessionInfoDock *dock_info);
static PicmanAlignmentType
                 picman_image_window_get_dock_side       (PicmanDockContainer   *dock_container,
                                                        PicmanDock            *dock);
static GList   * picman_image_window_get_aux_info        (PicmanSessionManaged  *session_managed);
static void      picman_image_window_set_aux_info        (PicmanSessionManaged  *session_managed,
                                                        GList               *aux_info);

static void      picman_image_window_config_notify       (PicmanImageWindow     *window,
                                                        GParamSpec          *pspec,
                                                        PicmanGuiConfig       *config);
static void      picman_image_window_session_clear       (PicmanImageWindow     *window);
static void      picman_image_window_session_apply       (PicmanImageWindow     *window,
                                                        const gchar         *entry_id);
static void      picman_image_window_session_update      (PicmanImageWindow     *window,
                                                        PicmanDisplay         *new_display,
                                                        const gchar         *new_entry_id);
static const gchar *
                 picman_image_window_config_to_entry_id  (PicmanGuiConfig       *config);
static void      picman_image_window_show_tooltip        (PicmanUIManager       *manager,
                                                        const gchar         *tooltip,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_hide_tooltip        (PicmanUIManager       *manager,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_update_ui_manager   (PicmanImageWindow     *window);

static void      picman_image_window_shell_size_allocate (PicmanDisplayShell    *shell,
                                                        GtkAllocation       *allocation,
                                                        PosCorrectionData   *data);
static gboolean  picman_image_window_shell_events        (GtkWidget           *widget,
                                                        GdkEvent            *event,
                                                        PicmanImageWindow     *window);

static void      picman_image_window_switch_page         (GtkNotebook         *notebook,
                                                        gpointer             page,
                                                        gint                 page_num,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_page_removed        (GtkNotebook         *notebook,
                                                        GtkWidget           *widget,
                                                        gint                 page_num,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_disconnect_from_active_shell
                                                       (PicmanImageWindow *window);

static void      picman_image_window_image_notify        (PicmanDisplay         *display,
                                                        const GParamSpec    *pspec,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_shell_scaled        (PicmanDisplayShell    *shell,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_shell_rotated       (PicmanDisplayShell    *shell,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_shell_title_notify  (PicmanDisplayShell    *shell,
                                                        const GParamSpec    *pspec,
                                                        PicmanImageWindow     *window);
static void      picman_image_window_shell_icon_notify   (PicmanDisplayShell    *shell,
                                                        const GParamSpec    *pspec,
                                                        PicmanImageWindow     *window);
static GtkWidget *
                 picman_image_window_create_tab_label    (PicmanImageWindow     *window,
                                                        PicmanDisplayShell    *shell);


G_DEFINE_TYPE_WITH_CODE (PicmanImageWindow, picman_image_window, PICMAN_TYPE_WINDOW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCK_CONTAINER,
                                                picman_image_window_dock_container_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_SESSION_MANAGED,
                                                picman_image_window_session_managed_iface_init))

#define parent_class picman_image_window_parent_class


static const gchar image_window_rc_style[] =
  "style \"fullscreen-menubar-style\"\n"
  "{\n"
  "  GtkMenuBar::shadow-type      = none\n"
  "  GtkMenuBar::internal-padding = 0\n"
  "}\n"
  "widget \"*.picman-menubar-fullscreen\" style \"fullscreen-menubar-style\"\n";

static void
picman_image_window_class_init (PicmanImageWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed        = picman_image_window_constructed;
  object_class->dispose            = picman_image_window_dispose;
  object_class->finalize           = picman_image_window_finalize;
  object_class->set_property       = picman_image_window_set_property;
  object_class->get_property       = picman_image_window_get_property;

  widget_class->delete_event       = picman_image_window_delete_event;
  widget_class->configure_event    = picman_image_window_configure_event;
  widget_class->window_state_event = picman_image_window_window_state_event;
  widget_class->style_set          = picman_image_window_style_set;

  g_object_class_install_property (object_class, PROP_PICMAN,
                                   g_param_spec_object ("picman",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_PICMAN,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_MENU_FACTORY,
                                   g_param_spec_object ("menu-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_MENU_FACTORY,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DIALOG_FACTORY,
                                   g_param_spec_object ("dialog-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DIALOG_FACTORY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (PicmanImageWindowPrivate));

  gtk_rc_parse_string (image_window_rc_style);
}

static void
picman_image_window_init (PicmanImageWindow *window)
{
  static gint  role_serial = 1;
  gchar       *role;

  role = g_strdup_printf ("picman-image-window-%d", role_serial++);
  gtk_window_set_role (GTK_WINDOW (window), role);
  g_free (role);

  gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
}

static void
picman_image_window_dock_container_iface_init (PicmanDockContainerInterface *iface)
{
  iface->get_docks      = picman_image_window_get_docks;
  iface->get_ui_manager = picman_image_window_dock_container_get_ui_manager;
  iface->add_dock       = picman_image_window_add_dock;
  iface->get_dock_side  = picman_image_window_get_dock_side;
}

static void
picman_image_window_session_managed_iface_init (PicmanSessionManagedInterface *iface)
{
  iface->get_aux_info = picman_image_window_get_aux_info;
  iface->set_aux_info = picman_image_window_set_aux_info;
}

static void
picman_image_window_constructed (GObject *object)
{
  PicmanImageWindow        *window  = PICMAN_IMAGE_WINDOW (object);
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanGuiConfig          *config;

  g_assert (PICMAN_IS_UI_MANAGER (private->menubar_manager));

  g_signal_connect_object (private->dialog_factory, "dock-window-added",
                           G_CALLBACK (picman_image_window_update_ui_manager),
                           window, G_CONNECT_SWAPPED);
  g_signal_connect_object (private->dialog_factory, "dock-window-removed",
                           G_CALLBACK (picman_image_window_update_ui_manager),
                           window, G_CONNECT_SWAPPED);

  gtk_window_add_accel_group (GTK_WINDOW (window),
                              gtk_ui_manager_get_accel_group (GTK_UI_MANAGER (private->menubar_manager)));

  g_signal_connect (private->menubar_manager, "show-tooltip",
                    G_CALLBACK (picman_image_window_show_tooltip),
                    window);
  g_signal_connect (private->menubar_manager, "hide-tooltip",
                    G_CALLBACK (picman_image_window_hide_tooltip),
                    window);

  config = PICMAN_GUI_CONFIG (picman_dialog_factory_get_context (private->dialog_factory)->picman->config);

  /* Create the window toplevel container */
  private->main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (window), private->main_vbox);
  gtk_widget_show (private->main_vbox);

  /* Create the menubar */
#ifndef GDK_WINDOWING_QUARTZ
  private->menubar =
    gtk_ui_manager_get_widget (GTK_UI_MANAGER (private->menubar_manager),
                               "/image-menubar");
#endif /* !GDK_WINDOWING_QUARTZ */
  if (private->menubar)
    {
      gtk_box_pack_start (GTK_BOX (private->main_vbox),
                          private->menubar, FALSE, FALSE, 0);

      /*  make sure we can activate accels even if the menubar is invisible
       *  (see http://bugzilla.gnome.org/show_bug.cgi?id=137151)
       */
      g_signal_connect (private->menubar, "can-activate-accel",
                        G_CALLBACK (gtk_true),
                        NULL);

      /*  active display callback  */
      g_signal_connect (private->menubar, "button-press-event",
                        G_CALLBACK (picman_image_window_shell_events),
                        window);
      g_signal_connect (private->menubar, "button-release-event",
                        G_CALLBACK (picman_image_window_shell_events),
                        window);
      g_signal_connect (private->menubar, "key-press-event",
                        G_CALLBACK (picman_image_window_shell_events),
                        window);
    }

  /* Create the hbox that contains docks and images */
  private->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (private->main_vbox), private->hbox,
                      TRUE, TRUE, 0);
  gtk_widget_show (private->hbox);

  /* Create the left pane */
  private->left_hpane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (private->hbox), private->left_hpane,
                      TRUE, TRUE, 0);
  gtk_widget_show (private->left_hpane);

  /* Create the left dock columns widget */
  private->left_docks =
    picman_dock_columns_new (picman_get_user_context (private->picman),
                           private->dialog_factory,
                           private->menubar_manager);
  gtk_paned_pack1 (GTK_PANED (private->left_hpane), private->left_docks,
                   FALSE, FALSE);
  gtk_widget_set_visible (private->left_docks, config->single_window_mode);

  /* Create the right pane */
  private->right_hpane = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack2 (GTK_PANED (private->left_hpane), private->right_hpane,
                   TRUE, FALSE);
  gtk_widget_show (private->right_hpane);

  /* Create notebook that contains images */
  private->notebook = gtk_notebook_new ();
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (private->notebook), TRUE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), FALSE);
  gtk_paned_pack1 (GTK_PANED (private->right_hpane), private->notebook,
                   TRUE, TRUE);
  g_signal_connect (private->notebook, "switch-page",
                    G_CALLBACK (picman_image_window_switch_page),
                    window);
  g_signal_connect (private->notebook, "page-removed",
                    G_CALLBACK (picman_image_window_page_removed),
                    window);
  gtk_widget_show (private->notebook);

  /* Create the right dock columns widget */
  private->right_docks =
    picman_dock_columns_new (picman_get_user_context (private->picman),
                           private->dialog_factory,
                           private->menubar_manager);
  gtk_paned_pack2 (GTK_PANED (private->right_hpane), private->right_docks,
                   FALSE, FALSE);
  gtk_widget_set_visible (private->right_docks, config->single_window_mode);

  g_signal_connect_object (config, "notify::single-window-mode",
                           G_CALLBACK (picman_image_window_config_notify),
                           window, G_CONNECT_SWAPPED);
  g_signal_connect_object (config, "notify::hide-docks",
                           G_CALLBACK (picman_image_window_config_notify),
                           window, G_CONNECT_SWAPPED);

  picman_image_window_session_update (window,
                                    NULL /*new_display*/,
                                    picman_image_window_config_to_entry_id (config));
}

static void
picman_image_window_dispose (GObject *object)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (object);

  if (private->dialog_factory)
    {
      g_signal_handlers_disconnect_by_func (private->dialog_factory,
                                            picman_image_window_update_ui_manager,
                                            object);
      private->dialog_factory = NULL;
    }

  if (private->menubar_manager)
    {
      g_object_unref (private->menubar_manager);
      private->menubar_manager = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_image_window_finalize (GObject *object)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (object);

  if (private->shells)
    {
      g_list_free (private->shells);
      private->shells = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_image_window_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanImageWindow        *window  = PICMAN_IMAGE_WINDOW (object);
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  switch (property_id)
    {
    case PROP_PICMAN:
      private->picman = g_value_get_object (value);
      break;
    case PROP_MENU_FACTORY:
      {
        PicmanMenuFactory *factory = g_value_get_object (value);

        private->menubar_manager = picman_menu_factory_manager_new (factory,
                                                                  "<Image>",
                                                                  window,
                                                                  FALSE);
      }
      break;
    case PROP_DIALOG_FACTORY:
      private->dialog_factory = g_value_get_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_image_window_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanImageWindow        *window  = PICMAN_IMAGE_WINDOW (object);
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  switch (property_id)
    {
    case PROP_PICMAN:
      g_value_set_object (value, private->picman);
      break;
    case PROP_DIALOG_FACTORY:
      g_value_set_object (value, private->dialog_factory);
      break;

    case PROP_MENU_FACTORY:
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_image_window_delete_event (GtkWidget   *widget,
                                GdkEventAny *event)
{
  PicmanImageWindow  *window = PICMAN_IMAGE_WINDOW (widget);
  PicmanDisplayShell *shell  = picman_image_window_get_active_shell (window);

  /* FIXME multiple shells */
  if (shell)
    picman_display_shell_close (shell, FALSE);

  return TRUE;
}

static gboolean
picman_image_window_configure_event (GtkWidget         *widget,
                                   GdkEventConfigure *event)
{
  PicmanImageWindow *window = PICMAN_IMAGE_WINDOW (widget);
  GtkAllocation    allocation;
  gint             current_width;
  gint             current_height;

  gtk_widget_get_allocation (widget, &allocation);

  /* Grab the size before we run the parent implementation */
  current_width  = allocation.width;
  current_height = allocation.height;

  /* Run the parent implementation */
  if (GTK_WIDGET_CLASS (parent_class)->configure_event)
    GTK_WIDGET_CLASS (parent_class)->configure_event (widget, event);

  /* If the window size has changed, make sure additoinal logic is run
   * in the display shell's size-allocate
   */
  if (event->width  != current_width ||
      event->height != current_height)
    {
      /* FIXME multiple shells */
      PicmanDisplayShell *shell = picman_image_window_get_active_shell (window);

      if (shell && picman_display_get_image (shell->display))
        shell->size_allocate_from_configure_event = TRUE;
    }

  return TRUE;
}

static gboolean
picman_image_window_window_state_event (GtkWidget           *widget,
                                      GdkEventWindowState *event)
{
  PicmanImageWindow        *window  = PICMAN_IMAGE_WINDOW (widget);
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanDisplayShell       *shell   = picman_image_window_get_active_shell (window);

  if (! shell)
    return FALSE;

  private->window_state = event->new_window_state;

  if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
    {
      gboolean fullscreen = picman_image_window_get_fullscreen (window);

      PICMAN_LOG (WM, "Image window '%s' [%p] set fullscreen %s",
                gtk_window_get_title (GTK_WINDOW (widget)),
                widget,
                fullscreen ? "TRUE" : "FALSE");

      if (private->menubar)
        gtk_widget_set_name (private->menubar,
                             fullscreen ? "picman-menubar-fullscreen" : NULL);

      picman_display_shell_appearance_update (shell);
    }

  if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
    {
      PicmanStatusbar *statusbar = picman_display_shell_get_statusbar (shell);
      gboolean       iconified = picman_image_window_is_iconified (window);

      PICMAN_LOG (WM, "Image window '%s' [%p] set %s",
                gtk_window_get_title (GTK_WINDOW (widget)),
                widget,
                iconified ? "iconified" : "uniconified");

      if (iconified)
        {
          if (picman_displays_get_num_visible (shell->display->picman) == 0)
            {
              PICMAN_LOG (WM, "No displays visible any longer");

              picman_dialog_factory_hide_with_display (private->dialog_factory);
            }
        }
      else
        {
          picman_dialog_factory_show_with_display (private->dialog_factory);
        }

      if (picman_progress_is_active (PICMAN_PROGRESS (statusbar)))
        {
          if (iconified)
            picman_statusbar_override_window_title (statusbar);
          else
            gtk_window_set_title (GTK_WINDOW (window), shell->title);
        }
    }

  return FALSE;
}

static void
picman_image_window_style_set (GtkWidget *widget,
                             GtkStyle  *prev_style)
{
  PicmanImageWindow        *window        = PICMAN_IMAGE_WINDOW (widget);
  PicmanImageWindowPrivate *private       = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanDisplayShell       *shell         = picman_image_window_get_active_shell (window);
  PicmanStatusbar          *statusbar     = NULL;
  GtkRequisition          requisition   = { 0, };
  GdkGeometry             geometry      = { 0, };
  GdkWindowHints          geometry_mask = 0;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (! shell)
    return;

  statusbar = picman_display_shell_get_statusbar (shell);

  gtk_widget_size_request (GTK_WIDGET (statusbar), &requisition);

  geometry.min_height = 23;

  geometry.min_width   = requisition.width;
  geometry.min_height += requisition.height;

  if (private->menubar)
    {
      gtk_widget_size_request (private->menubar, &requisition);

      geometry.min_height += requisition.height;
    }

  geometry_mask = GDK_HINT_MIN_SIZE;

  /*  Only set user pos on the empty display because it gets a pos
   *  set by picman. All other displays should be placed by the window
   *  manager. See http://bugzilla.gnome.org/show_bug.cgi?id=559580
   */
  if (! picman_display_get_image (shell->display))
    geometry_mask |= GDK_HINT_USER_POS;

  gtk_window_set_geometry_hints (GTK_WINDOW (widget), NULL,
                                 &geometry, geometry_mask);

  picman_dialog_factory_set_has_min_size (GTK_WINDOW (widget), TRUE);
}

static GList *
picman_image_window_get_docks (PicmanDockContainer *dock_container)
{
  PicmanImageWindowPrivate *private;
  GList                  *iter;
  GList                  *all_docks = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (dock_container), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (dock_container);

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->left_docks));
       iter;
       iter = g_list_next (iter))
    {
      all_docks = g_list_append (all_docks, PICMAN_DOCK (iter->data));
    }

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->right_docks));
       iter;
       iter = g_list_next (iter))
    {
      all_docks = g_list_append (all_docks, PICMAN_DOCK (iter->data));
    }

  return all_docks;
}

static PicmanUIManager *
picman_image_window_dock_container_get_ui_manager (PicmanDockContainer *dock_container)
{
  PicmanImageWindow *window;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (dock_container), FALSE);

  window = PICMAN_IMAGE_WINDOW (dock_container);

  return picman_image_window_get_ui_manager (window);
}

void
picman_image_window_add_dock (PicmanDockContainer   *dock_container,
                            PicmanDock            *dock,
                            PicmanSessionInfoDock *dock_info)
{
  PicmanImageWindow        *window;
  PicmanDisplayShell       *active_shell;
  PicmanImageWindowPrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (dock_container));

  window  = PICMAN_IMAGE_WINDOW (dock_container);
  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  if (dock_info->side == PICMAN_ALIGN_LEFT)
    {
      picman_dock_columns_add_dock (PICMAN_DOCK_COLUMNS (private->left_docks),
                                  dock,
                                  -1 /*index*/);
    }
  else
    {
      picman_dock_columns_add_dock (PICMAN_DOCK_COLUMNS (private->right_docks),
                                  dock,
                                  -1 /*index*/);
    }

  active_shell = picman_image_window_get_active_shell (window);
  if (active_shell)
    picman_display_shell_appearance_update (active_shell);
}

static PicmanAlignmentType
picman_image_window_get_dock_side (PicmanDockContainer *dock_container,
                                 PicmanDock          *dock)
{
  PicmanAlignmentType       side = -1;
  PicmanImageWindowPrivate *private;
  GList                  *iter;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (dock_container), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (dock_container);

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->left_docks));
       iter && side == -1;
       iter = g_list_next (iter))
    {
      PicmanDock *dock_iter = PICMAN_DOCK (iter->data);

      if (dock_iter == dock)
        side = PICMAN_ALIGN_LEFT;
    }

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->right_docks));
       iter && side == -1;
       iter = g_list_next (iter))
    {
      PicmanDock *dock_iter = PICMAN_DOCK (iter->data);

      if (dock_iter == dock)
        side = PICMAN_ALIGN_RIGHT;
    }

  return side;
}

static GList *
picman_image_window_get_aux_info (PicmanSessionManaged *session_managed)
{
  GList                  *aux_info = NULL;
  PicmanImageWindowPrivate *private;
  PicmanGuiConfig          *config;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (session_managed), NULL);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (session_managed);

  config = PICMAN_GUI_CONFIG (picman_dialog_factory_get_context (private->dialog_factory)->picman->config);

  if (config->single_window_mode)
    {
      PicmanSessionInfoAux *aux;
      GtkAllocation       allocation;
      gchar               widthbuf[128];

      g_snprintf (widthbuf, sizeof (widthbuf), "%d",
                  gtk_paned_get_position (GTK_PANED (private->left_hpane)));
      aux = picman_session_info_aux_new (PICMAN_IMAGE_WINDOW_LEFT_DOCKS_WIDTH, widthbuf);
      aux_info = g_list_append (aux_info, aux);

      gtk_widget_get_allocation (private->right_hpane, &allocation);

      /* a negative number will be interpreted as the width of the second
       * child of the pane
       */
      g_snprintf (widthbuf, sizeof (widthbuf), "%d",
                  gtk_paned_get_position (GTK_PANED (private->right_hpane)) -
                  allocation.width);
      aux = picman_session_info_aux_new (PICMAN_IMAGE_WINDOW_RIGHT_DOCKS_POS, widthbuf);
      aux_info = g_list_append (aux_info, aux);

      aux = picman_session_info_aux_new (PICMAN_IMAGE_WINDOW_MAXIMIZED,
                                       picman_image_window_is_maximized (PICMAN_IMAGE_WINDOW (session_managed)) ?
                                       "yes" : "no");
      aux_info = g_list_append (aux_info, aux);
    }

  return aux_info;
}

static void
picman_image_window_set_right_hpane_position (GtkPaned      *paned,
                                            GtkAllocation *allocation,
                                            void          *data)
{
  gint position = GPOINTER_TO_INT (data);

  g_return_if_fail (GTK_IS_PANED (paned));

  if (position > 0)
    gtk_paned_set_position (paned, position);
  else
    gtk_paned_set_position (paned, position + allocation->width);

  g_signal_handlers_disconnect_by_func (paned,
                                        picman_image_window_set_right_hpane_position,
                                        data);
}

static void
picman_image_window_set_aux_info (PicmanSessionManaged *session_managed,
                                GList              *aux_info)
{
  PicmanImageWindowPrivate *private;
  GList                  *iter;
  gint                    left_docks_width      = -1;
  gint                    right_docks_pos       = -1;
  gboolean                wait_with_right_docks = FALSE;
  gboolean                maximized             = FALSE;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (session_managed));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (session_managed);

  for (iter = aux_info; iter; iter = g_list_next (iter))
    {
      PicmanSessionInfoAux *aux   = iter->data;
      gint               *width = NULL;

      if (! strcmp (aux->name, PICMAN_IMAGE_WINDOW_LEFT_DOCKS_WIDTH))
        width = &left_docks_width;
      else if (! strcmp (aux->name, PICMAN_IMAGE_WINDOW_RIGHT_DOCKS_POS))
        width = &right_docks_pos;
      else if (! strcmp (aux->name, PICMAN_IMAGE_WINDOW_MAXIMIZED))
        if (! g_ascii_strcasecmp (aux->value, "yes"))
          maximized = TRUE;

      if (width)
        sscanf (aux->value, "%d", width);
    }

  if (left_docks_width > 0 &&
      gtk_paned_get_position (GTK_PANED (private->left_hpane)) != left_docks_width)
    {
      gtk_paned_set_position (GTK_PANED (private->left_hpane), left_docks_width);

      /* We can't set the position of the right docks, because it will
       * be undesirably adjusted when its get a new size
       * allocation. We must wait until after the size allocation.
       */
      wait_with_right_docks = TRUE;
    }

  if (right_docks_pos > 0 &&
      gtk_paned_get_position (GTK_PANED (private->right_hpane)) != right_docks_pos)
    {
      if (wait_with_right_docks || right_docks_pos < 0)
        {
          /* We must wait on a size allocation before we can set the
           * position
           */
          g_signal_connect_data (private->right_hpane, "size-allocate",
                                 G_CALLBACK (picman_image_window_set_right_hpane_position),
                                 GINT_TO_POINTER (right_docks_pos), NULL,
                                 G_CONNECT_AFTER);
        }
      else
        {
          /* We can set the position directly, because we didn't
           * change the left hpane position
           */
          gtk_paned_set_position (GTK_PANED (private->right_hpane),
                                  right_docks_pos);
        }
    }

  if (maximized)
    gtk_window_maximize (GTK_WINDOW (session_managed));
  else
    gtk_window_unmaximize (GTK_WINDOW (session_managed));
}


/*  public functions  */

PicmanImageWindow *
picman_image_window_new (Picman              *picman,
                       PicmanImage         *image,
                       PicmanMenuFactory   *menu_factory,
                       PicmanDialogFactory *dialog_factory)
{
  PicmanImageWindow *window;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (image) || image == NULL, NULL);
  g_return_val_if_fail (PICMAN_IS_MENU_FACTORY (menu_factory), NULL);
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (dialog_factory), NULL);

  window = g_object_new (PICMAN_TYPE_IMAGE_WINDOW,
                         "picman",            picman,
                         "menu-factory",    menu_factory,
                         "dialog-factory",  dialog_factory,
                         /* The window position will be overridden by the
                          * dialog factory, it is only really used on first
                          * startup.
                          */
                         image ? NULL : "window-position",
                         GTK_WIN_POS_CENTER,
                         NULL);

  picman->image_windows = g_list_prepend (picman->image_windows, window);

  return window;
}

void
picman_image_window_destroy (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  private->picman->image_windows = g_list_remove (private->picman->image_windows,
                                                window);

  gtk_widget_destroy (GTK_WIDGET (window));
}

PicmanUIManager *
picman_image_window_get_ui_manager (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return private->menubar_manager;
}

PicmanDockColumns  *
picman_image_window_get_left_docks (PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return PICMAN_DOCK_COLUMNS (private->left_docks);
}

PicmanDockColumns  *
picman_image_window_get_right_docks (PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return PICMAN_DOCK_COLUMNS (private->right_docks);
}

void
picman_image_window_add_shell (PicmanImageWindow  *window,
                             PicmanDisplayShell *shell)
{
  PicmanImageWindowPrivate *private;
  GtkWidget              *tab_label;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  g_return_if_fail (g_list_find (private->shells, shell) == NULL);

  private->shells = g_list_append (private->shells, shell);

  if (g_list_length (private->shells) > 1)
    {
      picman_image_window_keep_canvas_pos (window);
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), TRUE);
    }

  tab_label = picman_image_window_create_tab_label (window, shell);

  gtk_notebook_append_page (GTK_NOTEBOOK (private->notebook),
                            GTK_WIDGET (shell), tab_label);

  gtk_widget_show (GTK_WIDGET (shell));
}

PicmanDisplayShell *
picman_image_window_get_shell (PicmanImageWindow *window,
                             gint             index)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), NULL);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return g_list_nth_data (private->shells, index);
}

void
picman_image_window_remove_shell (PicmanImageWindow  *window,
                                PicmanDisplayShell *shell)
{
  PicmanImageWindowPrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  g_return_if_fail (g_list_find (private->shells, shell) != NULL);

  private->shells = g_list_remove (private->shells, shell);

  gtk_container_remove (GTK_CONTAINER (private->notebook),
                        GTK_WIDGET (shell));

  if (g_list_length (private->shells) == 1)
    {
      picman_image_window_keep_canvas_pos (window);
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (private->notebook), FALSE);
    }
}

gint
picman_image_window_get_n_shells (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), 0);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return g_list_length (private->shells);
}

void
picman_image_window_set_active_shell (PicmanImageWindow  *window,
                                    PicmanDisplayShell *shell)
{
  PicmanImageWindowPrivate *private;
  gint                    page_num;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));
  g_return_if_fail (PICMAN_IS_DISPLAY_SHELL (shell));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  g_return_if_fail (g_list_find (private->shells, shell));

  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (private->notebook),
                                    GTK_WIDGET (shell));

  gtk_notebook_set_current_page (GTK_NOTEBOOK (private->notebook), page_num);
}

PicmanDisplayShell *
picman_image_window_get_active_shell (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), NULL);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return private->active_shell;
}

void
picman_image_window_set_fullscreen (PicmanImageWindow *window,
                                  gboolean         fullscreen)
{
  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));

  if (fullscreen != picman_image_window_get_fullscreen (window))
    {
      if (fullscreen)
        gtk_window_fullscreen (GTK_WINDOW (window));
      else
        gtk_window_unfullscreen (GTK_WINDOW (window));
    }
}

gboolean
picman_image_window_get_fullscreen (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return (private->window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;
}

void
picman_image_window_set_show_menubar (PicmanImageWindow *window,
                                    gboolean         show)
{
  PicmanImageWindowPrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  if (private->menubar)
    gtk_widget_set_visible (private->menubar, show);
}

gboolean
picman_image_window_get_show_menubar (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  if (private->menubar)
    return gtk_widget_get_visible (private->menubar);

  return FALSE;
}

gboolean
picman_image_window_is_iconified (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return (private->window_state & GDK_WINDOW_STATE_ICONIFIED) != 0;
}

gboolean
picman_image_window_is_maximized (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  return (private->window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
}

/**
 * picman_image_window_has_toolbox:
 * @window:
 *
 * Returns: %TRUE if the image window contains a PicmanToolbox.
 **/
gboolean
picman_image_window_has_toolbox (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private;
  GList                  *iter = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGE_WINDOW (window), FALSE);

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->left_docks));
       iter;
       iter = g_list_next (iter))
    {
      if (PICMAN_IS_TOOLBOX (iter->data))
        return TRUE;
    }

  for (iter = picman_dock_columns_get_docks (PICMAN_DOCK_COLUMNS (private->right_docks));
       iter;
       iter = g_list_next (iter))
    {
      if (PICMAN_IS_TOOLBOX (iter->data))
        return TRUE;
    }

  return FALSE;
}

void
picman_image_window_shrink_wrap (PicmanImageWindow *window,
                               gboolean         grow_only)
{
  PicmanDisplayShell *active_shell;
  PicmanImage        *image;
  GtkWidget        *widget;
  GtkAllocation     allocation;
  GdkScreen        *screen;
  GdkRectangle      rect;
  gint              monitor;
  gint              disp_width, disp_height;
  gint              width, height;
  gint              max_auto_width, max_auto_height;
  gint              border_width, border_height;
  gboolean          resize = FALSE;

  g_return_if_fail (PICMAN_IS_IMAGE_WINDOW (window));

  if (! gtk_widget_get_realized (GTK_WIDGET (window)))
    return;

  /* FIXME this so needs cleanup and shell/window separation */

  active_shell = picman_image_window_get_active_shell (window);

  if (!active_shell)
    return;

  image = picman_display_get_image (active_shell->display);

  widget = GTK_WIDGET (window);
  screen = gtk_widget_get_screen (widget);

  gtk_widget_get_allocation (widget, &allocation);

  monitor = gdk_screen_get_monitor_at_window (screen,
                                              gtk_widget_get_window (widget));
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);

  width  = SCALEX (active_shell, picman_image_get_width  (image));
  height = SCALEY (active_shell, picman_image_get_height (image));

  disp_width  = active_shell->disp_width;
  disp_height = active_shell->disp_height;


  /* As long as the disp_width/disp_height is larger than 1 we
   * can reliably depend on it to calculate the
   * border_width/border_height because that means there is enough
   * room in the top-level for the canvas as well as the rulers and
   * scrollbars. If it is 1 or smaller it is likely that the rulers
   * and scrollbars are overlapping each other and thus we cannot use
   * the normal approach to border size, so special case that.
   */
  if (disp_width > 1 || !active_shell->vsb)
    {
      border_width = allocation.width - disp_width;
    }
  else
    {
      GtkAllocation vsb_allocation;

      gtk_widget_get_allocation (active_shell->vsb, &vsb_allocation);

      border_width = allocation.width - disp_width + vsb_allocation.width;
    }

  if (disp_height > 1 || !active_shell->hsb)
    {
      border_height = allocation.height - disp_height;
    }
  else
    {
      GtkAllocation hsb_allocation;

      gtk_widget_get_allocation (active_shell->hsb, &hsb_allocation);

      border_height = allocation.height - disp_height + hsb_allocation.height;
    }


  max_auto_width  = (rect.width  - border_width)  * 0.75;
  max_auto_height = (rect.height - border_height) * 0.75;

  /* If one of the display dimensions has changed and one of the
   * dimensions fits inside the screen
   */
  if (((width  + border_width)  < rect.width ||
       (height + border_height) < rect.height) &&
      (width  != disp_width ||
       height != disp_height))
    {
      width  = ((width  + border_width)  < rect.width)  ? width  : max_auto_width;
      height = ((height + border_height) < rect.height) ? height : max_auto_height;

      resize = TRUE;
    }

  /*  If the projected dimension is greater than current, but less than
   *  3/4 of the screen size, expand automagically
   */
  else if ((width  > disp_width ||
            height > disp_height) &&
           (disp_width  < max_auto_width ||
            disp_height < max_auto_height))
    {
      width  = MIN (max_auto_width,  width);
      height = MIN (max_auto_height, height);

      resize = TRUE;
    }

  if (resize)
    {
      PicmanStatusbar *statusbar = picman_display_shell_get_statusbar (active_shell);
      gint           statusbar_width;

      gtk_widget_get_size_request (GTK_WIDGET (statusbar),
                                   &statusbar_width, NULL);

      if (width < statusbar_width)
        width = statusbar_width;

      width  = width  + border_width;
      height = height + border_height;

      if (grow_only)
        {
          if (width < allocation.width)
            width = allocation.width;

          if (height < allocation.height)
            height = allocation.height;
        }

      gtk_window_resize (GTK_WINDOW (window), width, height);
    }

  /* A wrap always means that we should center the image too. If the
   * window changes size another center will be done in
   * PicmanDisplayShell::configure_event().
   */
  /* FIXME multiple shells */
  picman_display_shell_scroll_center_image (active_shell, TRUE, TRUE);
}

static GtkWidget *
picman_image_window_get_first_dockbook (PicmanDockColumns *columns)
{
  GList *dock_iter;
  
  for (dock_iter = picman_dock_columns_get_docks (columns);
       dock_iter;
       dock_iter = g_list_next (dock_iter))
    {
      PicmanDock *dock      = PICMAN_DOCK (dock_iter->data);
      GList    *dockbooks = picman_dock_get_dockbooks (dock);

      if (dockbooks)
        return GTK_WIDGET (dockbooks->data);
    }

  return NULL;
}

/**
 * picman_image_window_get_default_dockbook:
 * @window:
 *
 * Gets the default dockbook, which is the dockbook in which new
 * dockables should be put in single-window mode.
 *
 * Returns: The default dockbook for new dockables, or NULL if no
 *          dockbook were avaialble.
 **/
GtkWidget *
picman_image_window_get_default_dockbook (PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private;
  PicmanDockColumns        *dock_columns;
  GtkWidget              *dockbook = NULL;

  private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  /* First try the first dockbook in the right docks */
  dock_columns = PICMAN_DOCK_COLUMNS (private->right_docks);
  dockbook     = picman_image_window_get_first_dockbook (dock_columns);

  /* Then the left docks */
  if (! dockbook)
    {
      dock_columns = PICMAN_DOCK_COLUMNS (private->left_docks);
      dockbook     = picman_image_window_get_first_dockbook (dock_columns);
    }

  return dockbook;
}

/**
 * picman_image_window_keep_canvas_pos:
 * @window:
 *
 * Stores the coordinate of the current shell image origin in
 * GtkWindow coordinates and on the first size-allocate sets the
 * offsets in the shell so the image origin remains the same in
 * GtkWindow coordinates.
 *
 * Exampe use case: The user hides docks attached to the side of image
 * windows. You want the image to remain fixed on the screen though,
 * so you use this function to keep the image fixed after the docks
 * have been hidden.
 **/
void
picman_image_window_keep_canvas_pos (PicmanImageWindow *window)
{
  PicmanDisplayShell  *shell                 = picman_image_window_get_active_shell (window);
  gint               image_origin_shell_x  = -1;
  gint               image_origin_shell_y  = -1;
  gint               image_origin_window_x = -1;
  gint               image_origin_window_y = -1;

  picman_display_shell_transform_xy (shell,
                                   0.0, 0.0,
                                   &image_origin_shell_x,
                                   &image_origin_shell_y);

  if (gtk_widget_translate_coordinates (GTK_WIDGET (shell->canvas),
                                        GTK_WIDGET (window),
                                        image_origin_shell_x,
                                        image_origin_shell_y,
                                        &image_origin_window_x,
                                        &image_origin_window_y))
    {
      PosCorrectionData *data = g_new0 (PosCorrectionData, 1);

      data->window = window;
      data->x      = image_origin_window_x;
      data->y      = image_origin_window_y;

      g_signal_connect_data (shell, "size-allocate",
                             G_CALLBACK (picman_image_window_shell_size_allocate),
                             data, (GClosureNotify) g_free,
                             G_CONNECT_AFTER);
    }
}


/*  private functions  */

static void
picman_image_window_show_tooltip (PicmanUIManager   *manager,
                                const gchar     *tooltip,
                                PicmanImageWindow *window)
{
  PicmanDisplayShell *shell     = picman_image_window_get_active_shell (window);
  PicmanStatusbar    *statusbar = NULL;

  if (! shell)
    return;

  statusbar = picman_display_shell_get_statusbar (shell);

  picman_statusbar_push (statusbar, "menu-tooltip",
                       NULL, "%s", tooltip);
}

static void
picman_image_window_config_notify (PicmanImageWindow *window,
                                 GParamSpec      *pspec,
                                 PicmanGuiConfig   *config)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  /* Dock column visibility */
  if (strcmp (pspec->name, "single-window-mode") == 0 ||
      strcmp (pspec->name, "hide-docks")         == 0)
    {
      gboolean show_docks = (config->single_window_mode &&
                             ! config->hide_docks);

      picman_image_window_keep_canvas_pos (window);
      gtk_widget_set_visible (private->left_docks, show_docks);
      gtk_widget_set_visible (private->right_docks, show_docks);
    }

  /* Session management */
  if (strcmp (pspec->name, "single-window-mode") == 0)
    {
      picman_image_window_session_update (window,
                                        NULL /*new_display*/,
                                        picman_image_window_config_to_entry_id (config));
    }
}

static void
picman_image_window_hide_tooltip (PicmanUIManager   *manager,
                                PicmanImageWindow *window)
{
  PicmanDisplayShell *shell     = picman_image_window_get_active_shell (window);
  PicmanStatusbar    *statusbar = NULL;

  if (! shell)
    return;

  statusbar = picman_display_shell_get_statusbar (shell);

  picman_statusbar_pop (statusbar, "menu-tooltip");
}

static void
picman_image_window_update_ui_manager (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  picman_ui_manager_update (private->menubar_manager,
                          private->active_shell->display);
}

static void
picman_image_window_shell_size_allocate (PicmanDisplayShell  *shell,
                                       GtkAllocation     *allocation,
                                       PosCorrectionData *data)
{
  PicmanImageWindow *window               = data->window;
  gint             image_origin_shell_x = -1;
  gint             image_origin_shell_y = -1;

  gtk_widget_translate_coordinates (GTK_WIDGET (window),
                                    GTK_WIDGET (shell->canvas),
                                    data->x, data->y,
                                    &image_origin_shell_x,
                                    &image_origin_shell_y);

  /* Note that the shell offset isn't the offset of the image into the
   * shell, but the offset of the shell relative to the image,
   * therefore we need to negate
   */
  picman_display_shell_scroll_set_offset (shell,
                                        -image_origin_shell_x,
                                        -image_origin_shell_y);

  g_signal_handlers_disconnect_by_func (shell,
                                        picman_image_window_shell_size_allocate,
                                        data);
}

static gboolean
picman_image_window_shell_events (GtkWidget       *widget,
                                GdkEvent        *event,
                                PicmanImageWindow *window)
{
  PicmanDisplayShell *shell = picman_image_window_get_active_shell (window);

  if (! shell)
    return FALSE;

  return picman_display_shell_events (widget, event, shell);
}

static void
picman_image_window_switch_page (GtkNotebook     *notebook,
                               gpointer         page,
                               gint             page_num,
                               PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanDisplayShell       *shell;
  PicmanDisplay            *active_display;

  shell = PICMAN_DISPLAY_SHELL (gtk_notebook_get_nth_page (notebook, page_num));

  if (shell == private->active_shell)
    return;

  picman_image_window_disconnect_from_active_shell (window);

  PICMAN_LOG (WM, "PicmanImageWindow %p, private->active_shell = %p; \n",
            window, shell);
  private->active_shell = shell;

  picman_window_set_primary_focus_widget (PICMAN_WINDOW (window),
                                        shell->canvas);

  active_display = private->active_shell->display;

  g_signal_connect (active_display, "notify::image",
                    G_CALLBACK (picman_image_window_image_notify),
                    window);

  g_signal_connect (private->active_shell, "scaled",
                    G_CALLBACK (picman_image_window_shell_scaled),
                    window);
  g_signal_connect (private->active_shell, "rotated",
                    G_CALLBACK (picman_image_window_shell_rotated),
                    window);
  g_signal_connect (private->active_shell, "notify::title",
                    G_CALLBACK (picman_image_window_shell_title_notify),
                    window);
  g_signal_connect (private->active_shell, "notify::icon",
                    G_CALLBACK (picman_image_window_shell_icon_notify),
                    window);

  gtk_window_set_title (GTK_WINDOW (window), shell->title);
  gtk_window_set_icon (GTK_WINDOW (window), shell->icon);

  picman_display_shell_appearance_update (private->active_shell);

  picman_image_window_session_update (window,
                                    active_display,
                                    NULL /*new_entry_id*/);

  picman_context_set_display (picman_get_user_context (private->picman),
                            active_display);

  picman_ui_manager_update (private->menubar_manager, active_display);
}

static void
picman_image_window_page_removed (GtkNotebook     *notebook,
                                GtkWidget       *widget,
                                gint             page_num,
                                PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  if (GTK_WIDGET (private->active_shell) == widget)
    {
      PICMAN_LOG (WM, "PicmanImageWindow %p, private->active_shell = %p; \n",
                window, NULL);
      picman_image_window_disconnect_from_active_shell (window);
      private->active_shell = NULL;
    }
}

static void
picman_image_window_disconnect_from_active_shell (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private        = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanDisplay            *active_display = NULL;

  if (! private->active_shell)
    return;

  active_display = private->active_shell->display;

  if (active_display)
    g_signal_handlers_disconnect_by_func (active_display,
                                          picman_image_window_image_notify,
                                          window);

  g_signal_handlers_disconnect_by_func (private->active_shell,
                                        picman_image_window_shell_scaled,
                                        window);
  g_signal_handlers_disconnect_by_func (private->active_shell,
                                        picman_image_window_shell_rotated,
                                        window);
  g_signal_handlers_disconnect_by_func (private->active_shell,
                                        picman_image_window_shell_title_notify,
                                        window);
  g_signal_handlers_disconnect_by_func (private->active_shell,
                                        picman_image_window_shell_icon_notify,
                                        window);

  if (private->menubar_manager)
    picman_image_window_hide_tooltip (private->menubar_manager, window);
}

static void
picman_image_window_image_notify (PicmanDisplay      *display,
                                const GParamSpec *pspec,
                                PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  GtkWidget              *tab_label;
  GList                  *children;
  GtkWidget              *view;

  picman_image_window_session_update (window,
                                    display,
                                    NULL /*new_entry_id*/);

  tab_label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (private->notebook),
                                          GTK_WIDGET (picman_display_get_shell (display)));
  children  = gtk_container_get_children (GTK_CONTAINER (tab_label));
  view      = GTK_WIDGET (children->data);
  g_list_free (children);

  picman_view_set_viewable (PICMAN_VIEW (view),
                          PICMAN_VIEWABLE (picman_display_get_image (display)));

  picman_ui_manager_update (private->menubar_manager, display);
}

static void
picman_image_window_session_clear (PicmanImageWindow *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  GtkWidget              *widget  = GTK_WIDGET (window);

  if (picman_dialog_factory_from_widget (widget, NULL))
    picman_dialog_factory_remove_dialog (private->dialog_factory,
                                       widget);
}

static void
picman_image_window_session_apply (PicmanImageWindow *window,
                                 const gchar     *entry_id)
{
  PicmanImageWindowPrivate *private      = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);
  PicmanSessionInfo        *session_info = NULL;
  gint                    width        = -1;
  gint                    height       = -1;

  gtk_window_unfullscreen (GTK_WINDOW (window));

  /*  get the NIW size before adding the display to the dialog
   *  factory so the window's current size doesn't affect the
   *  stored session info entry.
   */
  session_info =
    picman_dialog_factory_find_session_info (private->dialog_factory, entry_id);

  if (session_info)
    {
      width  = picman_session_info_get_width  (session_info);
      height = picman_session_info_get_height (session_info);
    }
  else
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation (GTK_WIDGET (window), &allocation);

      width  = allocation.width;
      height = allocation.height;
    }

  picman_dialog_factory_add_foreign (private->dialog_factory,
                                   entry_id,
                                   GTK_WIDGET (window));

  gtk_window_unmaximize (GTK_WINDOW (window));
  gtk_window_resize (GTK_WINDOW (window), width, height);
}

static void
picman_image_window_session_update (PicmanImageWindow *window,
                                  PicmanDisplay     *new_display,
                                  const gchar     *new_entry_id)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  /* Handle changes to the entry id */
  if (new_entry_id)
    {
      if (! private->entry_id)
        {
          /* We're initializing. If we're in single-window mode, this
           * will be the only window, so start to session manage
           * it. If we're in multi-window mode, we will find out if we
           * should session manage ourselves when we get a display
           */
          if (strcmp (new_entry_id, PICMAN_SINGLE_IMAGE_WINDOW_ENTRY_ID) == 0)
            {
              picman_image_window_session_apply (window, new_entry_id);
            }
        }
      else if (strcmp (private->entry_id, new_entry_id) != 0)
        {
          /* The entry id changed, immediately and always stop session
           * managing the old entry
           */
          picman_image_window_session_clear (window);

          if (strcmp (new_entry_id, PICMAN_EMPTY_IMAGE_WINDOW_ENTRY_ID) == 0)
            {
              /* If there is only one imageless display, we shall
               * become the empty image window
               */
              if (private->active_shell &&
                  private->active_shell->display &&
                  ! picman_display_get_image (private->active_shell->display) &&
                  g_list_length (private->shells) <= 1)
                {
                  picman_image_window_session_apply (window, new_entry_id);
                }
            }
          else if (strcmp (new_entry_id, PICMAN_SINGLE_IMAGE_WINDOW_ENTRY_ID) == 0)
            {
              /* As soon as we become the single image window, we
               * shall session manage ourself until single-window mode
               * is exited
               */
              picman_image_window_session_apply (window, new_entry_id);
            }
        }

      private->entry_id = new_entry_id;
    }

  /* Handle changes to the displays. When in single-window mode, we
   * just keep session managing the single image window. We only need
   * to care about the multi-window mode case here
   */
  if (new_display &&
      strcmp (private->entry_id, PICMAN_EMPTY_IMAGE_WINDOW_ENTRY_ID) == 0)
    {
      if (picman_display_get_image (new_display))
        {
          /* As soon as we have an image we should not affect the size of the
           * empty image window
           */
          picman_image_window_session_clear (window);
        }
      else if (! picman_display_get_image (new_display) &&
               g_list_length (private->shells) <= 1)
        {
          /* As soon as we have no image (and no other shells that may
           * contain images) we should become the empty image window
           */
          picman_image_window_session_apply (window, private->entry_id);
        }
    }
}

static const gchar *
picman_image_window_config_to_entry_id (PicmanGuiConfig *config)
{
  return (config->single_window_mode ?
          PICMAN_SINGLE_IMAGE_WINDOW_ENTRY_ID :
          PICMAN_EMPTY_IMAGE_WINDOW_ENTRY_ID);
}

static void
picman_image_window_shell_scaled (PicmanDisplayShell *shell,
                                PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  /* update the <Image>/View/Zoom menu */
  picman_ui_manager_update (private->menubar_manager,
                          shell->display);
}

static void
picman_image_window_shell_rotated (PicmanDisplayShell *shell,
                                 PicmanImageWindow  *window)
{
  PicmanImageWindowPrivate *private = PICMAN_IMAGE_WINDOW_GET_PRIVATE (window);

  /* update the <Image>/View/Rotate menu */
  picman_ui_manager_update (private->menubar_manager,
                          shell->display);
}

static void
picman_image_window_shell_title_notify (PicmanDisplayShell *shell,
                                      const GParamSpec *pspec,
                                      PicmanImageWindow  *window)
{
  gtk_window_set_title (GTK_WINDOW (window), shell->title);
}

static void
picman_image_window_shell_icon_notify (PicmanDisplayShell *shell,
                                     const GParamSpec *pspec,
                                     PicmanImageWindow  *window)
{
  gtk_window_set_icon (GTK_WINDOW (window), shell->icon);
}

static void
picman_image_window_shell_close_button_callback (PicmanDisplayShell *shell)
{
  if (shell)
    picman_display_shell_close (shell, FALSE);
}

static GtkWidget *
picman_image_window_create_tab_label (PicmanImageWindow  *window,
                                    PicmanDisplayShell *shell)
{
  GtkWidget *hbox;
  GtkWidget *view;
  PicmanImage *image;
  GtkWidget *button;
  GtkWidget *gtk_image;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_show (hbox);

  view = picman_view_new_by_types (picman_get_user_context (shell->display->picman),
                                 PICMAN_TYPE_VIEW, PICMAN_TYPE_IMAGE,
                                 PICMAN_VIEW_SIZE_LARGE, 0, FALSE);
  gtk_widget_set_size_request (view, PICMAN_VIEW_SIZE_LARGE, -1);
  gtk_box_pack_start (GTK_BOX (hbox), view, FALSE, FALSE, 0);
  gtk_widget_show (view);

  image = picman_display_get_image (shell->display);
  if (image)
    picman_view_set_viewable (PICMAN_VIEW (view), PICMAN_VIEWABLE (image));

  button = gtk_button_new ();
  gtk_widget_set_can_focus (button, FALSE);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_image = gtk_image_new_from_stock (PICMAN_STOCK_CLOSE,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), gtk_image);
  gtk_widget_show (gtk_image);

  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (picman_image_window_shell_close_button_callback),
                            shell);

  return hbox;
}
