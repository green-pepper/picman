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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanwidgets/picmanwidgets.h"
#include "libpicmanwidgets/picmanwidgets-private.h"

#include "gui-types.h"

#include "config/picmanguiconfig.h"

#include "core/picman.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmanimage.h"
#include "core/picmantoolinfo.h"

#include "plug-in/picmanenvirontable.h"
#include "plug-in/picmanpluginmanager.h"

#include "display/picmandisplay.h"
#include "display/picmandisplay-foreach.h"
#include "display/picmandisplayshell.h"
#include "display/picmanstatusbar.h"

#include "tools/picman-tools.h"

#include "widgets/picmanclipboard.h"
#include "widgets/picmancolorselectorpalette.h"
#include "widgets/picmancontrollers.h"
#include "widgets/picmandevices.h"
#include "widgets/picmandialogfactory.h"
#include "widgets/picmandnd.h"
#include "widgets/picmanrender.h"
#include "widgets/picmanhelp.h"
#include "widgets/picmanhelp-ids.h"
#include "widgets/picmanmenufactory.h"
#include "widgets/picmanmessagebox.h"
#include "widgets/picmansessioninfo.h"
#include "widgets/picmanuimanager.h"
#include "widgets/picmanwidgets-utils.h"

#include "actions/actions.h"
#include "actions/windows-commands.h"

#include "menus/menus.h"

#include "dialogs/dialogs.h"

#include "color-history.h"
#include "picmanuiconfigurer.h"
#include "gui.h"
#include "gui-unique.h"
#include "gui-vtable.h"
#include "session.h"
#include "splash.h"
#include "themes.h"
#ifdef GDK_WINDOWING_QUARTZ
#include "ige-mac-menu.h"
#endif /* GDK_WINDOWING_QUARTZ */

#include "picman-intl.h"


/*  local function prototypes  */

static gchar    * gui_sanity_check              (void);
static void       gui_help_func                 (const gchar        *help_id,
                                                 gpointer            help_data);
static gboolean   gui_get_background_func       (PicmanRGB            *color);
static gboolean   gui_get_foreground_func       (PicmanRGB            *color);

static void       gui_initialize_after_callback (Picman               *picman,
                                                 PicmanInitStatusFunc  callback);

static void       gui_restore_callback          (Picman               *picman,
                                                 PicmanInitStatusFunc  callback);
static void       gui_restore_after_callback    (Picman               *picman,
                                                 PicmanInitStatusFunc  callback);

static gboolean   gui_exit_callback             (Picman               *picman,
                                                 gboolean            force);
static gboolean   gui_exit_after_callback       (Picman               *picman,
                                                 gboolean            force);

static void       gui_show_tooltips_notify      (PicmanGuiConfig      *gui_config,
                                                 GParamSpec         *pspec,
                                                 Picman               *picman);
static void       gui_show_help_button_notify   (PicmanGuiConfig      *gui_config,
                                                 GParamSpec         *pspec,
                                                 Picman               *picman);
static void       gui_user_manual_notify        (PicmanGuiConfig      *gui_config,
                                                 GParamSpec         *pspec,
                                                 Picman               *picman);
static void       gui_single_window_mode_notify (PicmanGuiConfig      *gui_config,
                                                 GParamSpec         *pspec,
                                                 PicmanUIConfigurer   *ui_configurer);
static void       gui_tearoff_menus_notify      (PicmanGuiConfig      *gui_config,
                                                 GParamSpec         *pspec,
                                                 GtkUIManager       *manager);

static void       gui_global_buffer_changed     (Picman               *picman);

static void       gui_menu_show_tooltip         (PicmanUIManager      *manager,
                                                 const gchar        *tooltip,
                                                 Picman               *picman);
static void       gui_menu_hide_tooltip         (PicmanUIManager      *manager,
                                                 Picman               *picman);

static void       gui_display_changed           (PicmanContext        *context,
                                                 PicmanDisplay        *display,
                                                 Picman               *picman);


/*  private variables  */

static Picman             *the_gui_picman     = NULL;
static PicmanUIManager    *image_ui_manager = NULL;
static PicmanUIConfigurer *ui_configurer    = NULL;


/*  public functions  */

void
gui_libs_init (GOptionContext *context)
{
  g_return_if_fail (context != NULL);

  g_option_context_add_group (context, gtk_get_option_group (TRUE));
}

void
gui_abort (const gchar *abort_message)
{
  GtkWidget *dialog;
  GtkWidget *box;

  g_return_if_fail (abort_message != NULL);

  dialog = picman_dialog_new (_("PICMAN Message"), "picman-abort",
                            NULL, GTK_DIALOG_MODAL, NULL, NULL,
                            GTK_STOCK_OK, GTK_RESPONSE_OK,
                            NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  box = g_object_new (PICMAN_TYPE_MESSAGE_BOX,
                      "stock-id",     PICMAN_STOCK_WILBER_EEK,
                      "border-width", 12,
                      NULL);

  picman_message_box_set_text (PICMAN_MESSAGE_BOX (box), "%s", abort_message);

  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                      box, TRUE, TRUE, 0);
  gtk_widget_show (box);

  picman_dialog_run (PICMAN_DIALOG (dialog));

  exit (EXIT_FAILURE);
}

PicmanInitStatusFunc
gui_init (Picman     *picman,
          gboolean  no_splash)
{
  PicmanInitStatusFunc  status_callback = NULL;
  GdkScreen          *screen;
  gchar              *abort_message;

  g_return_val_if_fail (PICMAN_IS_PICMAN (picman), NULL);
  g_return_val_if_fail (the_gui_picman == NULL, NULL);

  abort_message = gui_sanity_check ();
  if (abort_message)
    gui_abort (abort_message);

  the_gui_picman = picman;

  gui_unique_init (picman);

  picman_widgets_init (gui_help_func,
                     gui_get_foreground_func,
                     gui_get_background_func,
                     NULL);

  g_type_class_ref (PICMAN_TYPE_COLOR_SELECT);

  /*  disable automatic startup notification  */
  gtk_window_set_auto_startup_notification (FALSE);

  picman_dnd_init (picman);

  themes_init (picman);

  screen = gdk_screen_get_default ();
  gtk_widget_set_default_colormap (gdk_screen_get_rgb_colormap (screen));

  if (! no_splash)
    {
      splash_create (picman->be_verbose);
      status_callback = splash_update;
    }

  g_signal_connect_after (picman, "initialize",
                          G_CALLBACK (gui_initialize_after_callback),
                          NULL);

  g_signal_connect (picman, "restore",
                    G_CALLBACK (gui_restore_callback),
                    NULL);
  g_signal_connect_after (picman, "restore",
                          G_CALLBACK (gui_restore_after_callback),
                          NULL);

  g_signal_connect (picman, "exit",
                    G_CALLBACK (gui_exit_callback),
                    NULL);
  g_signal_connect_after (picman, "exit",
                          G_CALLBACK (gui_exit_after_callback),
                          NULL);

  return status_callback;
}


/*  private functions  */

static gchar *
gui_sanity_check (void)
{
#define GTK_REQUIRED_MAJOR 2
#define GTK_REQUIRED_MINOR 24
#define GTK_REQUIRED_MICRO 10

  const gchar *mismatch = gtk_check_version (GTK_REQUIRED_MAJOR,
                                             GTK_REQUIRED_MINOR,
                                             GTK_REQUIRED_MICRO);

  if (mismatch)
    {
      return g_strdup_printf
        ("%s\n\n"
         "PICMAN requires GTK+ version %d.%d.%d or later.\n"
         "Installed GTK+ version is %d.%d.%d.\n\n"
         "Somehow you or your software packager managed\n"
         "to install PICMAN with an older GTK+ version.\n\n"
         "Please upgrade to GTK+ version %d.%d.%d or later.",
         mismatch,
         GTK_REQUIRED_MAJOR, GTK_REQUIRED_MINOR, GTK_REQUIRED_MICRO,
         gtk_major_version, gtk_minor_version, gtk_micro_version,
         GTK_REQUIRED_MAJOR, GTK_REQUIRED_MINOR, GTK_REQUIRED_MICRO);
    }

#undef GTK_REQUIRED_MAJOR
#undef GTK_REQUIRED_MINOR
#undef GTK_REQUIRED_MICRO

  return NULL;
}


static void
gui_help_func (const gchar *help_id,
               gpointer     help_data)
{
  g_return_if_fail (PICMAN_IS_PICMAN (the_gui_picman));

  picman_help (the_gui_picman, NULL, NULL, help_id);
}

static gboolean
gui_get_foreground_func (PicmanRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);
  g_return_val_if_fail (PICMAN_IS_PICMAN (the_gui_picman), FALSE);

  picman_context_get_foreground (picman_get_user_context (the_gui_picman), color);

  return TRUE;
}

static gboolean
gui_get_background_func (PicmanRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);
  g_return_val_if_fail (PICMAN_IS_PICMAN (the_gui_picman), FALSE);

  picman_context_get_background (picman_get_user_context (the_gui_picman), color);

  return TRUE;
}

static void
gui_initialize_after_callback (Picman               *picman,
                               PicmanInitStatusFunc  status_callback)
{
  const gchar *name = NULL;

  g_return_if_fail (PICMAN_IS_PICMAN (picman));

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

#if defined (GDK_WINDOWING_X11)
  name = "DISPLAY";
#elif defined (GDK_WINDOWING_DIRECTFB) || defined (GDK_WINDOWING_FB)
  name = "GDK_DISPLAY";
#endif

  /* TODO: Need to care about display migration with GTK+ 2.2 at some point */

  if (name)
    {
      gchar *display = gdk_get_display ();

      picman_environ_table_add (picman->plug_in_manager->environ_table,
                              name, display, NULL);
      g_free (display);
    }

  picman_tools_init (picman);

  picman_context_set_tool (picman_get_user_context (picman),
                         picman_tool_info_get_standard (picman));
}

static void
gui_restore_callback (Picman               *picman,
                      PicmanInitStatusFunc  status_callback)
{
  PicmanDisplayConfig *display_config = PICMAN_DISPLAY_CONFIG (picman->config);
  PicmanGuiConfig     *gui_config     = PICMAN_GUI_CONFIG (picman->config);

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  gui_vtable_init (picman);

  if (! gui_config->show_tooltips)
    picman_help_disable_tooltips ();

  g_signal_connect (gui_config, "notify::show-tooltips",
                    G_CALLBACK (gui_show_tooltips_notify),
                    picman);

  picman_dialogs_show_help_button (gui_config->use_help &&
                                 gui_config->show_help_button);

  g_signal_connect (gui_config, "notify::use-help",
                    G_CALLBACK (gui_show_help_button_notify),
                    picman);
  g_signal_connect (gui_config, "notify::user-manual-online",
                    G_CALLBACK (gui_user_manual_notify),
                    picman);
  g_signal_connect (gui_config, "notify::show-help-button",
                    G_CALLBACK (gui_show_help_button_notify),
                    picman);

  g_signal_connect (picman_get_user_context (picman), "display-changed",
                    G_CALLBACK (gui_display_changed),
                    picman);

  /* make sure the monitor resolution is valid */
  if (display_config->monitor_res_from_gdk               ||
      display_config->monitor_xres < PICMAN_MIN_RESOLUTION ||
      display_config->monitor_yres < PICMAN_MIN_RESOLUTION)
    {
      gdouble xres, yres;

      picman_get_screen_resolution (NULL, &xres, &yres);

      g_object_set (picman->config,
                    "monitor-xresolution",                      xres,
                    "monitor-yresolution",                      yres,
                    "monitor-resolution-from-windowing-system", TRUE,
                    NULL);
    }

  actions_init (picman);
  menus_init (picman, global_action_factory);
  picman_render_init (picman);

  dialogs_init (picman, global_menu_factory);

  picman_clipboard_init (picman);
  picman_clipboard_set_buffer (picman, picman->global_buffer);

  g_signal_connect (picman, "buffer-changed",
                    G_CALLBACK (gui_global_buffer_changed),
                    NULL);

  picman_devices_init (picman);
  picman_controllers_init (picman);
  session_init (picman);

  g_type_class_unref (g_type_class_ref (PICMAN_TYPE_COLOR_SELECTOR_PALETTE));

  /*  initialize the document history  */
  status_callback (NULL, _("Documents"), 0.9);
  picman_recent_list_load (picman);

  status_callback (NULL, _("Tool Options"), 1.0);
  picman_tools_restore (picman);
}

#ifdef GDK_WINDOWING_QUARTZ
static void
gui_add_to_app_menu (PicmanUIManager   *ui_manager,
                     IgeMacMenuGroup *group,
                     const gchar     *action_path,
                     const gchar     *label)
{
  GtkWidget *item;

  item = gtk_ui_manager_get_widget (GTK_UI_MANAGER (ui_manager), action_path);

  if (GTK_IS_MENU_ITEM (item))
    ige_mac_menu_add_app_menu_item (group, GTK_MENU_ITEM (item), label);
}
#endif

static void
gui_restore_after_callback (Picman               *picman,
                            PicmanInitStatusFunc  status_callback)
{
  PicmanGuiConfig *gui_config = PICMAN_GUI_CONFIG (picman->config);
  PicmanDisplay   *display;

  if (picman->be_verbose)
    g_print ("INIT: %s\n", G_STRFUNC);

  picman->message_handler = PICMAN_MESSAGE_BOX;

  if (gui_config->restore_accels)
    menus_restore (picman);

  ui_configurer = g_object_new (PICMAN_TYPE_UI_CONFIGURER,
                                "picman", picman,
                                NULL);

  image_ui_manager = picman_menu_factory_manager_new (global_menu_factory,
                                                    "<Image>",
                                                    picman,
                                                    gui_config->tearoff_menus);
  picman_ui_manager_update (image_ui_manager, picman);

#ifdef GDK_WINDOWING_QUARTZ
  {
    IgeMacMenuGroup *group;
    GtkWidget       *menu;
    GtkWidget       *item;

    menu = gtk_ui_manager_get_widget (GTK_UI_MANAGER (image_ui_manager),
				      "/dummy-menubar/image-popup");

    if (GTK_IS_MENU_ITEM (menu))
      menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu));

    ige_mac_menu_set_menu_bar (GTK_MENU_SHELL (menu));

    item = gtk_ui_manager_get_widget (GTK_UI_MANAGER (image_ui_manager),
                                      "/dummy-menubar/image-popup/File/file-quit");
    if (GTK_IS_MENU_ITEM (item))
      ige_mac_menu_set_quit_menu_item (GTK_MENU_ITEM (item));

    /*  the about group  */
    group = ige_mac_menu_add_app_menu_group ();

    gui_add_to_app_menu (image_ui_manager, group,
                         "/dummy-menubar/image-popup/Help/dialogs-about",
                         _("About PICMAN"));

    /*  the preferences group  */
    group = ige_mac_menu_add_app_menu_group ();

#define PREFERENCES "/dummy-menubar/image-popup/Edit/Preferences/"

    gui_add_to_app_menu (image_ui_manager, group,
                         PREFERENCES "dialogs-preferences", NULL);
    gui_add_to_app_menu (image_ui_manager, group,
                         PREFERENCES "dialogs-input-devices", NULL);
    gui_add_to_app_menu (image_ui_manager, group,
                         PREFERENCES "dialogs-keyboard-shortcuts", NULL);
    gui_add_to_app_menu (image_ui_manager, group,
                         PREFERENCES "dialogs-module-dialog", NULL);
    gui_add_to_app_menu (image_ui_manager, group,
                         PREFERENCES "plug-in-unit-editor", NULL);

#undef PREFERENCES
  }
#endif /* GDK_WINDOWING_QUARTZ */

  g_signal_connect_object (gui_config, "notify::single-window-mode",
                           G_CALLBACK (gui_single_window_mode_notify),
                           ui_configurer, 0);
  g_signal_connect_object (gui_config, "notify::tearoff-menus",
                           G_CALLBACK (gui_tearoff_menus_notify),
                           image_ui_manager, 0);
  g_signal_connect (image_ui_manager, "show-tooltip",
                    G_CALLBACK (gui_menu_show_tooltip),
                    picman);
  g_signal_connect (image_ui_manager, "hide-tooltip",
                    G_CALLBACK (gui_menu_hide_tooltip),
                    picman);

  picman_devices_restore (picman);
  picman_controllers_restore (picman, image_ui_manager);

  if (status_callback == splash_update)
    splash_destroy ();

  color_history_restore (picman);

  if (picman_get_show_gui (picman))
    {
      PicmanDisplayShell *shell;

      /*  create the empty display  */
      display = PICMAN_DISPLAY (picman_create_display (picman,
                                                   NULL,
                                                   PICMAN_UNIT_PIXEL,
                                                   1.0));

      shell = picman_display_get_shell (display);

      if (gui_config->restore_session)
        session_restore (picman);

      /*  move keyboard focus to the display  */
      gtk_window_present (GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (shell))));
    }

  /*  indicate that the application has finished loading  */
  gdk_notify_startup_complete ();
}

static gboolean
gui_exit_callback (Picman     *picman,
                   gboolean  force)
{
  PicmanGuiConfig  *gui_config = PICMAN_GUI_CONFIG (picman->config);

  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  if (! force && picman_displays_dirty (picman))
    {
      picman_dialog_factory_dialog_raise (picman_dialog_factory_get_singleton (),
                                        gdk_screen_get_default (),
                                        "picman-quit-dialog", -1);

      return TRUE; /* stop exit for now */
    }

  picman->message_handler = PICMAN_CONSOLE;

  gui_unique_exit ();

  if (gui_config->save_session_info)
    session_save (picman, FALSE);

  color_history_save (picman);

  if (gui_config->save_accels)
    menus_save (picman, FALSE);

  if (gui_config->save_device_status)
    picman_devices_save (picman, FALSE);

  if (TRUE /* gui_config->save_controllers */)
    picman_controllers_save (picman);

  g_signal_handlers_disconnect_by_func (picman_get_user_context (picman),
                                        gui_display_changed,
                                        picman);

  picman_displays_delete (picman);

  picman_tools_save (picman, gui_config->save_tool_options, FALSE);
  picman_tools_exit (picman);

  return FALSE; /* continue exiting */
}

static gboolean
gui_exit_after_callback (Picman     *picman,
                         gboolean  force)
{
  if (picman->be_verbose)
    g_print ("EXIT: %s\n", G_STRFUNC);

  g_signal_handlers_disconnect_by_func (picman->config,
                                        gui_show_help_button_notify,
                                        picman);
  g_signal_handlers_disconnect_by_func (picman->config,
                                        gui_user_manual_notify,
                                        picman);
  g_signal_handlers_disconnect_by_func (picman->config,
                                        gui_show_tooltips_notify,
                                        picman);

  g_object_unref (image_ui_manager);
  image_ui_manager = NULL;

  g_object_unref (ui_configurer);
  ui_configurer = NULL;

  session_exit (picman);
  menus_exit (picman);
  actions_exit (picman);
  picman_render_exit (picman);

  picman_controllers_exit (picman);
  picman_devices_exit (picman);
  dialogs_exit (picman);

  g_signal_handlers_disconnect_by_func (picman,
                                        G_CALLBACK (gui_global_buffer_changed),
                                        NULL);
  picman_clipboard_exit (picman);

  themes_exit (picman);

  g_type_class_unref (g_type_class_peek (PICMAN_TYPE_COLOR_SELECT));

  return FALSE; /* continue exiting */
}

static void
gui_show_tooltips_notify (PicmanGuiConfig *gui_config,
                          GParamSpec    *param_spec,
                          Picman          *picman)
{
  if (gui_config->show_tooltips)
    picman_help_enable_tooltips ();
  else
    picman_help_disable_tooltips ();
}

static void
gui_show_help_button_notify (PicmanGuiConfig *gui_config,
                             GParamSpec    *param_spec,
                             Picman          *picman)
{
  picman_dialogs_show_help_button (gui_config->use_help &&
                                 gui_config->show_help_button);
}

static void
gui_user_manual_notify (PicmanGuiConfig *gui_config,
                        GParamSpec    *param_spec,
                        Picman          *picman)
{
  picman_help_user_manual_changed (picman);
}

static void
gui_single_window_mode_notify (PicmanGuiConfig      *gui_config,
                               GParamSpec         *pspec,
                               PicmanUIConfigurer   *ui_configurer)
{
  picman_ui_configurer_configure (ui_configurer,
                                gui_config->single_window_mode);
}
static void
gui_tearoff_menus_notify (PicmanGuiConfig *gui_config,
                          GParamSpec    *pspec,
                          GtkUIManager  *manager)
{
  gtk_ui_manager_set_add_tearoffs (manager, gui_config->tearoff_menus);
}

static void
gui_global_buffer_changed (Picman *picman)
{
  picman_clipboard_set_buffer (picman, picman->global_buffer);
}

static void
gui_menu_show_tooltip (PicmanUIManager *manager,
                       const gchar   *tooltip,
                       Picman          *picman)
{
  PicmanContext *context = picman_get_user_context (picman);
  PicmanDisplay *display = picman_context_get_display (context);

  if (display)
    {
      PicmanDisplayShell *shell     = picman_display_get_shell (display);
      PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

      picman_statusbar_push (statusbar, "menu-tooltip",
                           NULL, "%s", tooltip);
    }
}

static void
gui_menu_hide_tooltip (PicmanUIManager *manager,
                       Picman          *picman)
{
  PicmanContext *context = picman_get_user_context (picman);
  PicmanDisplay *display = picman_context_get_display (context);

  if (display)
    {
      PicmanDisplayShell *shell     = picman_display_get_shell (display);
      PicmanStatusbar    *statusbar = picman_display_shell_get_statusbar (shell);

      picman_statusbar_pop (statusbar, "menu-tooltip");
    }
}

static void
gui_display_changed (PicmanContext *context,
                     PicmanDisplay *display,
                     Picman        *picman)
{
  if (! display)
    {
      PicmanImage *image = picman_context_get_image (context);

      if (image)
        {
          GList *list;

          for (list = picman_get_display_iter (picman);
               list;
               list = g_list_next (list))
            {
              PicmanDisplay *display2 = list->data;

              if (picman_display_get_image (display2) == image)
                {
                  picman_context_set_display (context, display2);

                  /* stop the emission of the original signal
                   * (the emission of the recursive signal is finished)
                   */
                  g_signal_stop_emission_by_name (context, "display-changed");
                  return;
                }
            }

          picman_context_set_image (context, NULL);
        }
    }

  picman_ui_manager_update (image_ui_manager, display);
}
