/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandockwindow.c
 * Copyright (C) 2001-2005 Michael Natterer <mitch@picman.org>
 * Copyright (C)      2009 Martin Nordholts <martinn@src.gnome.org>
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

#include "dialogs/dialogs.h" /* FIXME, we are in the widget layer */

#include "menus/menus.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontext.h"
#include "core/picmancontainer.h"
#include "core/picmancontainer.h"
#include "core/picmanlist.h"
#include "core/picmanimage.h"

#include "picmancontainercombobox.h"
#include "picmancontainerview.h"
#include "picmandialogfactory.h"
#include "picmandock.h"
#include "picmandockbook.h"
#include "picmandockcolumns.h"
#include "picmandockcontainer.h"
#include "picmandockwindow.h"
#include "picmanhelp-ids.h"
#include "picmanmenufactory.h"
#include "picmansessioninfo-aux.h"
#include "picmansessioninfo.h"
#include "picmansessionmanaged.h"
#include "picmantoolbox.h"
#include "picmanuimanager.h"
#include "picmanwidgets-utils.h"
#include "picmanwindow.h"

#include "picman-intl.h"


#define DEFAULT_DOCK_HEIGHT          300
#define DEFAULT_MENU_VIEW_SIZE       GTK_ICON_SIZE_SMALL_TOOLBAR
#define AUX_INFO_SHOW_IMAGE_MENU     "show-image-menu"
#define AUX_INFO_FOLLOW_ACTIVE_IMAGE "follow-active-image"


enum
{
  PROP_0,
  PROP_CONTEXT,
  PROP_DIALOG_FACTORY,
  PROP_UI_MANAGER_NAME,
  PROP_IMAGE_CONTAINER,
  PROP_DISPLAY_CONTAINER,
  PROP_ALLOW_DOCKBOOK_ABSENCE
};


struct _PicmanDockWindowPrivate
{
  PicmanContext       *context;

  PicmanDialogFactory *dialog_factory;

  gchar             *ui_manager_name;
  PicmanUIManager     *ui_manager;
  GQuark             image_flush_handler_id;

  PicmanDockColumns   *dock_columns;

  gboolean           allow_dockbook_absence;

  guint              update_title_idle_id;

  gint               ID;

  PicmanContainer     *image_container;
  PicmanContainer     *display_container;

  gboolean           show_image_menu;
  gboolean           auto_follow_active;

  GtkWidget         *image_combo;
  GtkWidget         *auto_button;
};


static void            picman_dock_window_dock_container_iface_init (PicmanDockContainerInterface *iface);
static void            picman_dock_window_session_managed_iface_init(PicmanSessionManagedInterface*iface);
static void            picman_dock_window_constructed               (GObject                    *object);
static void            picman_dock_window_dispose                   (GObject                    *object);
static void            picman_dock_window_finalize                  (GObject                    *object);
static void            picman_dock_window_set_property              (GObject                    *object,
                                                                   guint                       property_id,
                                                                   const GValue               *value,
                                                                   GParamSpec                 *pspec);
static void            picman_dock_window_get_property              (GObject                    *object,
                                                                   guint                       property_id,
                                                                   GValue                     *value,
                                                                   GParamSpec                 *pspec);
static void            picman_dock_window_style_set                 (GtkWidget                  *widget,
                                                                   GtkStyle                   *prev_style);
static gboolean        picman_dock_window_delete_event              (GtkWidget                  *widget,
                                                                   GdkEventAny                *event);
static GList         * picman_dock_window_get_docks                 (PicmanDockContainer          *dock_container);
static PicmanUIManager * picman_dock_window_get_ui_manager            (PicmanDockContainer          *dock_container);
static void            picman_dock_window_add_dock_from_session     (PicmanDockContainer          *dock_container,
                                                                   PicmanDock                   *dock,
                                                                   PicmanSessionInfoDock        *dock_info);
static GList         * picman_dock_window_get_aux_info              (PicmanSessionManaged         *session_managed);
static void            picman_dock_window_set_aux_info              (PicmanSessionManaged         *session_managed,
                                                                   GList                      *aux_info);
static PicmanAlignmentType
                       picman_dock_window_get_dock_side             (PicmanDockContainer          *dock_container,
                                                                   PicmanDock                   *dock);
static gboolean        picman_dock_window_should_add_to_recent      (PicmanDockWindow             *dock_window);
static void            picman_dock_window_display_changed           (PicmanDockWindow             *dock_window,
                                                                   PicmanObject                 *display,
                                                                   PicmanContext                *context);
static void            picman_dock_window_image_changed             (PicmanDockWindow             *dock_window,
                                                                   PicmanImage                  *image,
                                                                   PicmanContext                *context);
static void            picman_dock_window_image_flush               (PicmanImage                  *image,
                                                                   gboolean                    invalidate_preview,
                                                                   PicmanDockWindow             *dock_window);
static void            picman_dock_window_update_title              (PicmanDockWindow             *dock_window);
static gboolean        picman_dock_window_update_title_idle         (PicmanDockWindow             *dock_window);
static gchar         * picman_dock_window_get_description           (PicmanDockWindow             *dock_window,
                                                                   gboolean                    complete);
static void            picman_dock_window_dock_removed              (PicmanDockWindow             *dock_window,
                                                                   PicmanDock                   *dock,
                                                                   PicmanDockColumns            *dock_columns);
static void            picman_dock_window_factory_display_changed   (PicmanContext                *context,
                                                                   PicmanObject                 *display,
                                                                   PicmanDock                   *dock);
static void            picman_dock_window_factory_image_changed     (PicmanContext                *context,
                                                                   PicmanImage                  *image,
                                                                   PicmanDock                   *dock);
static void            picman_dock_window_auto_clicked              (GtkWidget                  *widget,
                                                                   PicmanDock                   *dock);


G_DEFINE_TYPE_WITH_CODE (PicmanDockWindow, picman_dock_window, PICMAN_TYPE_WINDOW,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_DOCK_CONTAINER,
                                                picman_dock_window_dock_container_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_SESSION_MANAGED,
                                                picman_dock_window_session_managed_iface_init))

#define parent_class picman_dock_window_parent_class

static void
picman_dock_window_class_init (PicmanDockWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed  = picman_dock_window_constructed;
  object_class->dispose      = picman_dock_window_dispose;
  object_class->finalize     = picman_dock_window_finalize;
  object_class->set_property = picman_dock_window_set_property;
  object_class->get_property = picman_dock_window_get_property;

  widget_class->style_set    = picman_dock_window_style_set;
  widget_class->delete_event = picman_dock_window_delete_event;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context", NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DIALOG_FACTORY,
                                   g_param_spec_object ("dialog-factory",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DIALOG_FACTORY,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_UI_MANAGER_NAME,
                                   g_param_spec_string ("ui-manager-name",
                                                        NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_IMAGE_CONTAINER,
                                   g_param_spec_object ("image-container",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_DISPLAY_CONTAINER,
                                   g_param_spec_object ("display-container",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTAINER,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_ALLOW_DOCKBOOK_ABSENCE,
                                   g_param_spec_boolean ("allow-dockbook-absence",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT_ONLY));


  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("default-height",
                                                             NULL, NULL,
                                                             -1, G_MAXINT,
                                                             DEFAULT_DOCK_HEIGHT,
                                                             PICMAN_PARAM_READABLE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_enum ("menu-preview-size",
                                                              NULL, NULL,
                                                              GTK_TYPE_ICON_SIZE,
                                                              DEFAULT_MENU_VIEW_SIZE,
                                                              PICMAN_PARAM_READABLE));

  g_type_class_add_private (klass, sizeof (PicmanDockWindowPrivate));
}

static void
picman_dock_window_init (PicmanDockWindow *dock_window)
{
  static gint  dock_window_ID = 1;
  gchar       *name           = NULL;

  /* Initialize members */
  dock_window->p = G_TYPE_INSTANCE_GET_PRIVATE (dock_window,
                                                PICMAN_TYPE_DOCK_WINDOW,
                                                PicmanDockWindowPrivate);
  dock_window->p->ID                 = dock_window_ID++;
  dock_window->p->auto_follow_active = TRUE;

  /* Initialize theming and style-setting stuff */
  name = g_strdup_printf ("picman-dock-%d", dock_window->p->ID);
  gtk_widget_set_name (GTK_WIDGET (dock_window), name);
  g_free (name);

  /* Misc */
  gtk_window_set_resizable (GTK_WINDOW (dock_window), TRUE);
  gtk_window_set_focus_on_map (GTK_WINDOW (dock_window), FALSE);
}

static void
picman_dock_window_dock_container_iface_init (PicmanDockContainerInterface *iface)
{
  iface->get_docks      = picman_dock_window_get_docks;
  iface->get_ui_manager = picman_dock_window_get_ui_manager;
  iface->add_dock       = picman_dock_window_add_dock_from_session;
  iface->get_dock_side  = picman_dock_window_get_dock_side;
}

static void
picman_dock_window_session_managed_iface_init (PicmanSessionManagedInterface *iface)
{
  iface->get_aux_info = picman_dock_window_get_aux_info;
  iface->set_aux_info = picman_dock_window_set_aux_info;
}

static void
picman_dock_window_constructed (GObject *object)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (object);
  PicmanGuiConfig  *config;
  PicmanContext    *factory_context;
  GtkAccelGroup  *accel_group;
  Picman           *picman;
  GtkSettings    *settings;
  gint            menu_view_width  = -1;
  gint            menu_view_height = -1;

  G_OBJECT_CLASS (parent_class)->constructed (object);

  picman   = PICMAN (dock_window->p->context->picman);
  config = PICMAN_GUI_CONFIG (picman->config);

  /* Create a separate context per dock so that docks can be bound to
   * a specific image and does not necessarily have to follow the
   * active image in the user context
   */
  g_object_unref (dock_window->p->context);
  dock_window->p->context           = picman_context_new (picman, "Dock Context", NULL);
  dock_window->p->image_container   = picman->images;
  dock_window->p->display_container = picman->displays;

  factory_context =
    picman_dialog_factory_get_context (dock_window->p->dialog_factory);

  /* Setup hints */
  picman_window_set_hint (GTK_WINDOW (dock_window), config->dock_window_hint);

  /* Make image window related keyboard shortcuts work also when a
   * dock window is the focused window
   */
  dock_window->p->ui_manager =
    picman_menu_factory_manager_new (global_menu_factory,
                                   dock_window->p->ui_manager_name,
                                   dock_window,
                                   config->tearoff_menus);
  accel_group =
    gtk_ui_manager_get_accel_group (GTK_UI_MANAGER (dock_window->p->ui_manager));
  gtk_window_add_accel_group (GTK_WINDOW (dock_window), accel_group);

  g_signal_connect_object (dock_window->p->context, "display-changed",
                           G_CALLBACK (picman_dock_window_display_changed),
                           dock_window,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (dock_window->p->context, "image-changed",
                           G_CALLBACK (picman_dock_window_image_changed),
                           dock_window,
                           G_CONNECT_SWAPPED);

  dock_window->p->image_flush_handler_id =
    picman_container_add_handler (picman->images, "flush",
                                G_CALLBACK (picman_dock_window_image_flush),
                                dock_window);

  picman_context_define_properties (dock_window->p->context,
                                  PICMAN_CONTEXT_ALL_PROPS_MASK &
                                  ~(PICMAN_CONTEXT_IMAGE_MASK |
                                    PICMAN_CONTEXT_DISPLAY_MASK),
                                  FALSE);
  picman_context_set_parent (dock_window->p->context,
                           factory_context);

  /* Setup widget hierarchy */
  {
    GtkWidget *vbox = NULL;

    /* Top-level GtkVBox */
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (dock_window), vbox);
    gtk_widget_show (vbox);

    /* Image selection menu */
    {
      GtkWidget *hbox = NULL;

      /* GtkHBox */
      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
      if (dock_window->p->show_image_menu)
        gtk_widget_show (hbox);

      /* Image combo */
      dock_window->p->image_combo = picman_container_combo_box_new (NULL, NULL, 16, 1);
      gtk_box_pack_start (GTK_BOX (hbox), dock_window->p->image_combo, TRUE, TRUE, 0);
      g_signal_connect (dock_window->p->image_combo, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &dock_window->p->image_combo);
      picman_help_set_help_data (dock_window->p->image_combo,
                               NULL, PICMAN_HELP_DOCK_IMAGE_MENU);
      gtk_widget_show (dock_window->p->image_combo);

      /* Auto button */
      dock_window->p->auto_button = gtk_toggle_button_new_with_label (_("Auto"));
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dock_window->p->auto_button),
                                    dock_window->p->auto_follow_active);
      gtk_box_pack_start (GTK_BOX (hbox), dock_window->p->auto_button, FALSE, FALSE, 0);
      gtk_widget_show (dock_window->p->auto_button);

      g_signal_connect (dock_window->p->auto_button, "clicked",
                        G_CALLBACK (picman_dock_window_auto_clicked),
                        dock_window);

      picman_help_set_help_data (dock_window->p->auto_button,
                               _("When enabled the dialog automatically "
                                 "follows the image you are working on."),
                               PICMAN_HELP_DOCK_AUTO_BUTTON);
    }

    /* PicmanDockColumns */
    /* Let the PicmanDockColumns mirror the context so that a PicmanDock can
     * get it when inside a dock window. We do the same thing in the
     * PicmanImageWindow so docks can get the PicmanContext there as well
     */
    dock_window->p->dock_columns =
      PICMAN_DOCK_COLUMNS (picman_dock_columns_new (dock_window->p->context,
                                                dock_window->p->dialog_factory,
                                                dock_window->p->ui_manager));
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (dock_window->p->dock_columns),
                        TRUE, TRUE, 0);
    gtk_widget_show (GTK_WIDGET (dock_window->p->dock_columns));
    g_signal_connect_object (dock_window->p->dock_columns, "dock-removed",
                             G_CALLBACK (picman_dock_window_dock_removed),
                             dock_window,
                             G_CONNECT_SWAPPED);

    g_signal_connect_object (dock_window->p->dock_columns, "dock-added",
                             G_CALLBACK (picman_dock_window_update_title),
                             dock_window,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (dock_window->p->dock_columns, "dock-removed",
                             G_CALLBACK (picman_dock_window_update_title),
                             dock_window,
                             G_CONNECT_SWAPPED);
  }

  if (dock_window->p->auto_follow_active)
    {
      if (picman_context_get_display (factory_context))
        picman_context_copy_property (factory_context,
                                    dock_window->p->context,
                                    PICMAN_CONTEXT_PROP_DISPLAY);
      else
        picman_context_copy_property (factory_context,
                                    dock_window->p->context,
                                    PICMAN_CONTEXT_PROP_IMAGE);
    }

  g_signal_connect_object (factory_context, "display-changed",
                           G_CALLBACK (picman_dock_window_factory_display_changed),
                           dock_window,
                           0);
  g_signal_connect_object (factory_context, "image-changed",
                           G_CALLBACK (picman_dock_window_factory_image_changed),
                           dock_window,
                           0);

  settings = gtk_widget_get_settings (GTK_WIDGET (dock_window));
  gtk_icon_size_lookup_for_settings (settings,
                                     DEFAULT_MENU_VIEW_SIZE,
                                     &menu_view_width,
                                     &menu_view_height);

  g_object_set (dock_window->p->image_combo,
                "container", dock_window->p->image_container,
                "context",   dock_window->p->context,
                NULL);

  picman_help_connect (GTK_WIDGET (dock_window), picman_standard_help_func,
                     PICMAN_HELP_DOCK, NULL);

  if (dock_window->p->auto_follow_active)
    {
      if (picman_context_get_display (factory_context))
        picman_context_copy_property (factory_context,
                                    dock_window->p->context,
                                    PICMAN_CONTEXT_PROP_DISPLAY);
      else
        picman_context_copy_property (factory_context,
                                    dock_window->p->context,
                                    PICMAN_CONTEXT_PROP_IMAGE);
    }
}

static void
picman_dock_window_dispose (GObject *object)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (object);

  if (dock_window->p->update_title_idle_id)
    {
      g_source_remove (dock_window->p->update_title_idle_id);
      dock_window->p->update_title_idle_id = 0;
    }

  if (dock_window->p->image_flush_handler_id)
    {
      picman_container_remove_handler (dock_window->p->context->picman->images,
                                     dock_window->p->image_flush_handler_id);
      dock_window->p->image_flush_handler_id = 0;
    }

  if (dock_window->p->ui_manager)
    {
      g_object_unref (dock_window->p->ui_manager);
      dock_window->p->ui_manager = NULL;
    }

  if (dock_window->p->dialog_factory)
    {
      g_object_unref (dock_window->p->dialog_factory);
      dock_window->p->dialog_factory = NULL;
    }

  if (dock_window->p->context)
    {
      g_object_unref (dock_window->p->context);
      dock_window->p->context = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_dock_window_finalize (GObject *object)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (object);

  if (dock_window->p->ui_manager_name)
    {
      g_free (dock_window->p->ui_manager_name);
      dock_window->p->ui_manager_name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_dock_window_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      dock_window->p->context = g_value_dup_object (value);
      break;

    case PROP_DIALOG_FACTORY:
      dock_window->p->dialog_factory = g_value_dup_object (value);
      break;

    case PROP_UI_MANAGER_NAME:
      g_free (dock_window->p->ui_manager_name);
      dock_window->p->ui_manager_name = g_value_dup_string (value);
      break;

    case PROP_IMAGE_CONTAINER:
      dock_window->p->image_container = g_value_dup_object (value);
      break;

    case PROP_DISPLAY_CONTAINER:
      dock_window->p->display_container = g_value_dup_object (value);
      break;

    case PROP_ALLOW_DOCKBOOK_ABSENCE:
      dock_window->p->allow_dockbook_absence = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dock_window_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, dock_window->p->context);
      break;

    case PROP_DIALOG_FACTORY:
      g_value_set_object (value, dock_window->p->dialog_factory);
      break;

    case PROP_UI_MANAGER_NAME:
      g_value_set_string (value, dock_window->p->ui_manager_name);
      break;

    case PROP_IMAGE_CONTAINER:
      g_value_set_object (value, dock_window->p->image_container);
      break;

    case PROP_DISPLAY_CONTAINER:
      g_value_set_object (value, dock_window->p->display_container);
      break;

    case PROP_ALLOW_DOCKBOOK_ABSENCE:
      g_value_set_boolean (value, dock_window->p->allow_dockbook_absence);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_dock_window_style_set (GtkWidget *widget,
                            GtkStyle  *prev_style)
{
  PicmanDockWindow *dock_window      = PICMAN_DOCK_WINDOW (widget);
  GtkStyle       *button_style;
  GtkIconSize     menu_view_size;
  GtkSettings    *settings;
  gint            menu_view_width  = 18;
  gint            menu_view_height = 18;
  gint            focus_line_width;
  gint            focus_padding;
  gint            ythickness;

  gint default_height = DEFAULT_DOCK_HEIGHT;

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "default-height", &default_height,
                        "menu-preview-size", &menu_view_size,
                        NULL);

  gtk_window_set_default_size (GTK_WINDOW (widget), -1, default_height);

  settings = gtk_widget_get_settings (dock_window->p->image_combo);
  gtk_icon_size_lookup_for_settings (settings,
                                     menu_view_size,
                                     &menu_view_width,
                                     &menu_view_height);

  gtk_widget_style_get (dock_window->p->auto_button,
                        "focus-line-width", &focus_line_width,
                        "focus-padding",    &focus_padding,
                        NULL);

  button_style = gtk_widget_get_style (widget);
  ythickness = button_style->ythickness;

  picman_container_view_set_view_size (PICMAN_CONTAINER_VIEW (dock_window->p->image_combo),
                                     menu_view_height, 1);

  gtk_widget_set_size_request (dock_window->p->auto_button, -1,
                               menu_view_height +
                               2 * (1 /* CHILD_SPACING */ +
                                    ythickness            +
                                    focus_padding         +
                                    focus_line_width));
}

/**
 * picman_dock_window_delete_event:
 * @widget:
 * @event:
 *
 * Makes sure that when dock windows are closed they are added to the
 * list of recently closed docks so that they are easy to bring back.
 **/
static gboolean
picman_dock_window_delete_event (GtkWidget   *widget,
                               GdkEventAny *event)
{
  PicmanDockWindow         *dock_window = PICMAN_DOCK_WINDOW (widget);
  PicmanSessionInfo        *info        = NULL;
  const gchar            *entry_name  = NULL;
  PicmanDialogFactoryEntry *entry       = NULL;
  gchar                  *name        = NULL;

  /* Don't add docks with just a singe dockable to the list of
   * recently closed dock since those can be brought back through the
   * normal Windows->Dockable Dialogs menu
   */
  if (! picman_dock_window_should_add_to_recent (dock_window))
    return FALSE;

  info = picman_session_info_new ();

  name = picman_dock_window_get_description (dock_window, TRUE /*complete*/);
  picman_object_set_name (PICMAN_OBJECT (info), name);
  g_free (name);

  picman_session_info_get_info_with_widget (info, GTK_WIDGET (dock_window));

  entry_name = (picman_dock_window_has_toolbox (dock_window) ?
                "picman-toolbox-window" :
                "picman-dock-window");
  entry = picman_dialog_factory_find_entry (picman_dialog_factory_get_singleton (), entry_name);
  picman_session_info_set_factory_entry (info, entry);

  picman_container_add (global_recent_docks, PICMAN_OBJECT (info));
  g_object_unref (info);

  return FALSE;
}

static GList *
picman_dock_window_get_docks (PicmanDockContainer *dock_container)
{
  PicmanDockWindow *dock_window;

  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_container), NULL);

  dock_window = PICMAN_DOCK_WINDOW (dock_container);

  return g_list_copy (picman_dock_columns_get_docks (dock_window->p->dock_columns));
}

static PicmanUIManager *
picman_dock_window_get_ui_manager (PicmanDockContainer *dock_container)
{
  PicmanDockWindow *dock_window;

  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_container), NULL);

  dock_window = PICMAN_DOCK_WINDOW (dock_container);

  return dock_window->p->ui_manager;
}

static void
picman_dock_window_add_dock_from_session (PicmanDockContainer   *dock_container,
                                        PicmanDock            *dock,
                                        PicmanSessionInfoDock *dock_info)
{
  PicmanDockWindow *dock_window;

  g_return_if_fail (PICMAN_IS_DOCK_WINDOW (dock_container));

  dock_window = PICMAN_DOCK_WINDOW (dock_container);

  picman_dock_window_add_dock (dock_window,
                             dock,
                             -1 /*index*/);
}

static GList *
picman_dock_window_get_aux_info (PicmanSessionManaged *session_managed)
{
  PicmanDockWindow     *dock_window;
  GList              *aux_info = NULL;
  PicmanSessionInfoAux *aux;

  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (session_managed), NULL);

  dock_window = PICMAN_DOCK_WINDOW (session_managed);

  if (dock_window->p->allow_dockbook_absence)
    {
      /* Assume it is the toolbox; it does not have aux info */
      return NULL;
    }

  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), NULL);

  aux = picman_session_info_aux_new (AUX_INFO_SHOW_IMAGE_MENU,
                                   dock_window->p->show_image_menu ?
                                   "true" : "false");
  aux_info = g_list_append (aux_info, aux);

  aux = picman_session_info_aux_new (AUX_INFO_FOLLOW_ACTIVE_IMAGE,
                                   dock_window->p->auto_follow_active ?
                                   "true" : "false");
  aux_info = g_list_append (aux_info, aux);

  return aux_info;
}

static void
picman_dock_window_set_aux_info (PicmanSessionManaged *session_managed,
                               GList              *aux_info)
{
  PicmanDockWindow *dock_window;
  GList          *list;
  gboolean        menu_shown;
  gboolean        auto_follow;

  g_return_if_fail (PICMAN_IS_DOCK_WINDOW (session_managed));

  dock_window = PICMAN_DOCK_WINDOW (session_managed);
  menu_shown  = dock_window->p->show_image_menu;
  auto_follow = dock_window->p->auto_follow_active;

  for (list = aux_info; list; list = g_list_next (list))
    {
      PicmanSessionInfoAux *aux = list->data;

      if (! strcmp (aux->name, AUX_INFO_SHOW_IMAGE_MENU))
        {
          menu_shown = ! g_ascii_strcasecmp (aux->value, "true");
        }
      else if (! strcmp (aux->name, AUX_INFO_FOLLOW_ACTIVE_IMAGE))
        {
          auto_follow = ! g_ascii_strcasecmp (aux->value, "true");
        }
    }

  if (menu_shown != dock_window->p->show_image_menu)
    picman_dock_window_set_show_image_menu (dock_window, menu_shown);

  if (auto_follow != dock_window->p->auto_follow_active)
    picman_dock_window_set_auto_follow_active (dock_window, auto_follow);
}

static PicmanAlignmentType
picman_dock_window_get_dock_side (PicmanDockContainer *dock_container,
                                PicmanDock          *dock)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_container), -1);
  g_return_val_if_fail (PICMAN_IS_DOCK (dock), -1);

  /* A PicmanDockWindow don't have docks on different sides, it's just
   * one set of columns
   */
  return -1;
}

/**
 * picman_dock_window_should_add_to_recent:
 * @dock_window:
 *
 * Returns: %FALSE if the dock window can be recreated with one
 *          Windows menu item such as Windows->Toolbox or
 *          Windows->Dockable Dialogs->Layers, %TRUE if not. It should
 *          then be added to the list of recently closed docks.
 **/
static gboolean
picman_dock_window_should_add_to_recent (PicmanDockWindow *dock_window)
{
  GList    *docks;
  gboolean  should_add = TRUE;

  docks = picman_dock_container_get_docks (PICMAN_DOCK_CONTAINER (dock_window));

  if (! docks)
    {
      should_add = FALSE;
    }
  else if (g_list_length (docks) == 1)
    {
      PicmanDock *dock = PICMAN_DOCK (g_list_nth_data (docks, 0));

      if (PICMAN_IS_TOOLBOX (dock) &&
          picman_dock_get_n_dockables (dock) == 0)
        {
          should_add = FALSE;
        }
      else if (! PICMAN_IS_TOOLBOX (dock) &&
               picman_dock_get_n_dockables (dock) == 1)
        {
          should_add = FALSE;
        }
    }

  g_list_free (docks);

  return should_add;
}

static void
picman_dock_window_image_flush (PicmanImage      *image,
                              gboolean        invalidate_preview,
                              PicmanDockWindow *dock_window)
{
  if (image == picman_context_get_image (dock_window->p->context))
    {
      PicmanObject *display = picman_context_get_display (dock_window->p->context);

      if (display)
        picman_ui_manager_update (dock_window->p->ui_manager, display);
    }
}

static void
picman_dock_window_update_title (PicmanDockWindow *dock_window)
{
  if (dock_window->p->update_title_idle_id)
    g_source_remove (dock_window->p->update_title_idle_id);

  dock_window->p->update_title_idle_id =
    g_idle_add ((GSourceFunc) picman_dock_window_update_title_idle,
                dock_window);
}

static gboolean
picman_dock_window_update_title_idle (PicmanDockWindow *dock_window)
{
  gchar *desc = picman_dock_window_get_description (dock_window,
                                                  FALSE /*complete*/);
  if (desc)
    {
      gtk_window_set_title (GTK_WINDOW (dock_window), desc);
      g_free (desc);
    }

  dock_window->p->update_title_idle_id = 0;

  return FALSE;
}

static gchar *
picman_dock_window_get_description (PicmanDockWindow *dock_window,
                                  gboolean        complete)
{
  GString *complete_desc = g_string_new (NULL);
  GList   *docks         = NULL;
  GList   *iter          = NULL;

  docks = picman_dock_container_get_docks (PICMAN_DOCK_CONTAINER (dock_window));

  for (iter = docks;
       iter;
       iter = g_list_next (iter))
    {
      gchar *desc = picman_dock_get_description (PICMAN_DOCK (iter->data), complete);
      g_string_append (complete_desc, desc);
      g_free (desc);

      if (g_list_next (iter))
        g_string_append (complete_desc, PICMAN_DOCK_COLUMN_SEPARATOR);
    }

  g_list_free (docks);

  return g_string_free (complete_desc, FALSE /*free_segment*/);
}

static void
picman_dock_window_dock_removed (PicmanDockWindow  *dock_window,
                               PicmanDock        *dock,
                               PicmanDockColumns *dock_columns)
{
  g_return_if_fail (PICMAN_IS_DOCK (dock));

  if (picman_dock_columns_get_docks (dock_columns) == NULL &&
      ! dock_window->p->allow_dockbook_absence)
    gtk_widget_destroy (GTK_WIDGET (dock_window));
}

static void
picman_dock_window_factory_display_changed (PicmanContext *context,
                                          PicmanObject  *display,
                                          PicmanDock    *dock)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (dock);

  if (display && dock_window->p->auto_follow_active)
    picman_context_set_display (dock_window->p->context, display);
}

static void
picman_dock_window_factory_image_changed (PicmanContext *context,
                                        PicmanImage   *image,
                                        PicmanDock    *dock)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (dock);

  /*  won't do anything if we already set the display above  */
  if (image && dock_window->p->auto_follow_active)
    picman_context_set_image (dock_window->p->context, image);
}

static void
picman_dock_window_display_changed (PicmanDockWindow *dock_window,
                                  PicmanObject     *display,
                                  PicmanContext    *context)
{
  /*  make sure auto-follow-active works both ways  */
  if (display && dock_window->p->auto_follow_active)
    {
      PicmanContext *factory_context =
        picman_dialog_factory_get_context (dock_window->p->dialog_factory);

      picman_context_set_display (factory_context, display);
    }

  picman_ui_manager_update (dock_window->p->ui_manager,
                          display);
}

static void
picman_dock_window_image_changed (PicmanDockWindow *dock_window,
                                PicmanImage      *image,
                                PicmanContext    *context)
{
  PicmanContainer *image_container   = dock_window->p->image_container;
  PicmanContainer *display_container = dock_window->p->display_container;

  /*  make sure auto-follow-active works both ways  */
  if (image && dock_window->p->auto_follow_active)
    {
      PicmanContext *factory_context =
        picman_dialog_factory_get_context (dock_window->p->dialog_factory);

      picman_context_set_image (factory_context, image);
    }

  if (image == NULL && ! picman_container_is_empty (image_container))
    {
      image = PICMAN_IMAGE (picman_container_get_first_child (image_container));

      /*  this invokes this function recursively but we don't enter
       *  the if() branch the second time
       */
      picman_context_set_image (context, image);

      /*  stop the emission of the original signal (the emission of
       *  the recursive signal is finished)
       */
      g_signal_stop_emission_by_name (context, "image-changed");
    }
  else if (image != NULL && ! picman_container_is_empty (display_container))
    {
      PicmanObject *display;
      PicmanImage  *display_image;
      gboolean    find_display = TRUE;

      display = picman_context_get_display (context);

      if (display)
        {
          g_object_get (display, "image", &display_image, NULL);

          if (display_image)
            {
              g_object_unref (display_image);

              if (display_image == image)
                find_display = FALSE;
            }
        }

      if (find_display)
        {
          GList *list;

          for (list = PICMAN_LIST (display_container)->list;
               list;
               list = g_list_next (list))
            {
              display = PICMAN_OBJECT (list->data);

              g_object_get (display, "image", &display_image, NULL);

              if (display_image)
                {
                  g_object_unref (display_image);

                  if (display_image == image)
                    {
                      /*  this invokes this function recursively but we
                       *  don't enter the if(find_display) branch the
                       *  second time
                       */
                      picman_context_set_display (context, display);

                      /*  don't stop signal emission here because the
                       *  context's image was not changed by the
                       *  recursive call
                       */
                      break;
                    }
                }
            }
        }
    }

  picman_ui_manager_update (dock_window->p->ui_manager,
                          picman_context_get_display (context));
}

static void
picman_dock_window_auto_clicked (GtkWidget *widget,
                               PicmanDock  *dock)
{
  PicmanDockWindow *dock_window = PICMAN_DOCK_WINDOW (dock);

  picman_toggle_button_update (widget, &dock_window->p->auto_follow_active);

  if (dock_window->p->auto_follow_active)
    {
      picman_context_copy_properties (picman_dialog_factory_get_context (dock_window->p->dialog_factory),
                                    dock_window->p->context,
                                    PICMAN_CONTEXT_DISPLAY_MASK |
                                    PICMAN_CONTEXT_IMAGE_MASK);
    }
}


void
picman_dock_window_add_dock (PicmanDockWindow *dock_window,
                           PicmanDock       *dock,
                           gint            index)
{
  g_return_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window));
  g_return_if_fail (PICMAN_IS_DOCK (dock));

  picman_dock_columns_add_dock (dock_window->p->dock_columns,
                              PICMAN_DOCK (dock),
                              index);

  g_signal_connect_object (dock, "description-invalidated",
                           G_CALLBACK (picman_dock_window_update_title),
                           dock_window,
                           G_CONNECT_SWAPPED);

  /* Some docks like the toolbox dock needs to maintain special hints
   * on its container GtkWindow, allow those to do so
   */
  picman_dock_set_host_geometry_hints (dock, GTK_WINDOW (dock_window));
  g_signal_connect_object (dock, "geometry-invalidated",
                           G_CALLBACK (picman_dock_set_host_geometry_hints),
                           dock_window, 0);
}

void
picman_dock_window_remove_dock (PicmanDockWindow *dock_window,
                              PicmanDock       *dock)
{
  picman_dock_columns_remove_dock (dock_window->p->dock_columns,
                                 PICMAN_DOCK (dock));

  g_signal_handlers_disconnect_by_func (dock,
                                        picman_dock_window_update_title,
                                        dock_window);
  g_signal_handlers_disconnect_by_func (dock,
                                        picman_dock_set_host_geometry_hints,
                                        dock_window);
}

GtkWidget *
picman_dock_window_new (const gchar       *role,
                      const gchar       *ui_manager_name,
                      gboolean           allow_dockbook_absence,
                      PicmanDialogFactory *factory,
                      PicmanContext       *context)
{
  g_return_val_if_fail (PICMAN_IS_DIALOG_FACTORY (factory), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_DOCK_WINDOW,
                       "role",                   role,
                       "ui-manager-name",        ui_manager_name,
                       "allow-dockbook-absence", allow_dockbook_absence,
                       "dialog-factory",         factory,
                       "context",                context,
                       NULL);
}

gint
picman_dock_window_get_id (PicmanDockWindow *dock_window)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), 0);

  return dock_window->p->ID;
}

PicmanContext *
picman_dock_window_get_context (PicmanDockWindow *dock_window)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), NULL);

  return dock_window->p->context;
}

PicmanDialogFactory *
picman_dock_window_get_dialog_factory (PicmanDockWindow *dock_window)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), NULL);

  return dock_window->p->dialog_factory;
}

gboolean
picman_dock_window_get_auto_follow_active (PicmanDockWindow *dock_window)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), FALSE);

  return dock_window->p->auto_follow_active;
}

void
picman_dock_window_set_auto_follow_active (PicmanDockWindow *dock_window,
                                         gboolean        auto_follow_active)
{
  g_return_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dock_window->p->auto_button),
                                auto_follow_active ? TRUE : FALSE);
}

gboolean
picman_dock_window_get_show_image_menu (PicmanDockWindow *dock_window)
{
  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), FALSE);

  return dock_window->p->show_image_menu;
}

void
picman_dock_window_set_show_image_menu (PicmanDockWindow *dock_window,
                                      gboolean        show)
{
  GtkWidget *parent;

  g_return_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window));

  parent = gtk_widget_get_parent (dock_window->p->image_combo);

  gtk_widget_set_visible (parent, show);

  dock_window->p->show_image_menu = show ? TRUE : FALSE;
}

void
picman_dock_window_setup (PicmanDockWindow *dock_window,
                        PicmanDockWindow *template)
{
  picman_dock_window_set_auto_follow_active (PICMAN_DOCK_WINDOW (dock_window),
                                           template->p->auto_follow_active);
  picman_dock_window_set_show_image_menu (PICMAN_DOCK_WINDOW (dock_window),
                                        template->p->show_image_menu);
}

/**
 * picman_dock_window_has_toolbox:
 * @dock_window:
 *
 * Returns: %TRUE if the dock window has a PicmanToolbox dock, %FALSE
 * otherwise.
 **/
gboolean
picman_dock_window_has_toolbox (PicmanDockWindow *dock_window)
{
  GList *iter = NULL;

  g_return_val_if_fail (PICMAN_IS_DOCK_WINDOW (dock_window), FALSE);

  for (iter = picman_dock_columns_get_docks (dock_window->p->dock_columns);
       iter;
       iter = g_list_next (iter))
    {
      if (PICMAN_IS_TOOLBOX (iter->data))
        return TRUE;
    }

  return FALSE;
}


/**
 * picman_dock_window_from_dock:
 * @dock:
 *
 * For convenience.
 *
 * Returns: If the toplevel widget for the dock is a PicmanDockWindow,
 * return that. Otherwise return %NULL.
 **/
PicmanDockWindow *
picman_dock_window_from_dock (PicmanDock *dock)
{
  GtkWidget *toplevel = NULL;

  g_return_val_if_fail (PICMAN_IS_DOCK (dock), NULL);

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (dock));

  if (PICMAN_IS_DOCK_WINDOW (toplevel))
    return PICMAN_DOCK_WINDOW (toplevel);
  else
    return NULL;
}
