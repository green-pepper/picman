/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_WIN32
#include <gdk/gdkwin32.h>
#endif

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <Cocoa/Cocoa.h>
#endif

#include "picman.h"
#include "picmanui.h"

#include "libpicmanmodule/picmanmodule.h"

#include "libpicmanwidgets/picmanwidgets.h"
#include "libpicmanwidgets/picmanwidgets-private.h"


/**
 * SECTION: picmanui
 * @title: picmanui
 * @short_description: Common user interface functions. This header includes
 *                     all other PICMAN User Interface Library headers.
 * @see_also: gtk_init(), gdk_set_use_xshm(), gdk_rgb_get_visual(),
 *            gdk_rgb_get_cmap(), gtk_widget_set_default_visual(),
 *            gtk_widget_set_default_colormap(), gtk_preview_set_gamma().
 *
 * Common user interface functions. This header includes all other
 * PICMAN User Interface Library headers.
 **/


/*  local function prototypes  */

static void      picman_ui_help_func              (const gchar *help_id,
                                                 gpointer     help_data);
static void      picman_ensure_modules            (void);
static void      picman_window_transient_realized (GtkWidget   *window,
                                                 GdkWindow   *parent);
static gboolean  picman_window_set_transient_for  (GtkWindow   *window,
                                                 GdkWindow   *parent);


static gboolean picman_ui_initialized = FALSE;


/*  public functions  */

/**
 * picman_ui_init:
 * @prog_name: The name of the plug-in which will be passed as argv[0] to
 *             gtk_init(). It's a convention to use the name of the
 *             executable and _not_ the PDB procedure name.
 * @preview:   This parameter is unused and exists for historical
 *             reasons only.
 *
 * This function initializes GTK+ with gtk_init() and initializes GDK's
 * image rendering subsystem (GdkRGB) to follow the PICMAN main program's
 * colormap allocation/installation policy.
 *
 * It also sets up various other things so that the plug-in user looks
 * and behaves like the PICMAN core. This includes selecting the GTK+
 * theme and setting up the help system as chosen in the PICMAN
 * preferences. Any plug-in that provides a user interface should call
 * this function.
 **/
void
picman_ui_init (const gchar *prog_name,
              gboolean     preview)
{
  GdkScreen   *screen;
  const gchar *display_name;
  gchar       *themerc;

  g_return_if_fail (prog_name != NULL);

  if (picman_ui_initialized)
    return;

  g_set_prgname (prog_name);

  display_name = picman_display_name ();

  if (display_name)
    {
#if defined (GDK_WINDOWING_X11)
      g_setenv ("DISPLAY", display_name, TRUE);
#else
      g_setenv ("GDK_DISPLAY", display_name, TRUE);
#endif
    }

  if (picman_user_time ())
    {
      /* Construct a fake startup ID as we only want to pass the
       * interaction timestamp, see _gdk_windowing_set_default_display().
       */
      gchar *startup_id = g_strdup_printf ("_TIME%u", picman_user_time ());

      g_setenv ("DESKTOP_STARTUP_ID", startup_id, TRUE);
      g_free (startup_id);
    }

  gtk_init (NULL, NULL);

  themerc = picman_personal_rc_file ("themerc");
  gtk_rc_add_default_file (themerc);
  g_free (themerc);

  gdk_set_program_class (picman_wm_class ());

  screen = gdk_screen_get_default ();
  gtk_widget_set_default_colormap (gdk_screen_get_rgb_colormap (screen));

  picman_widgets_init (picman_ui_help_func,
                     picman_context_get_foreground,
                     picman_context_get_background,
                     picman_ensure_modules);

  if (! picman_show_tool_tips ())
    picman_help_disable_tooltips ();

  picman_dialogs_show_help_button (picman_show_help_button ());

#ifdef GDK_WINDOWING_QUARTZ
  [NSApp activateIgnoringOtherApps:YES];
#endif

  picman_ui_initialized = TRUE;
}

static GdkWindow *
picman_ui_get_foreign_window (guint32 window)
{
#ifdef GDK_WINDOWING_X11
  return gdk_x11_window_foreign_new_for_display (gdk_display_get_default (),
                                                 window);
#endif

#ifdef GDK_WINDOWING_WIN32
  return gdk_win32_window_foreign_new_for_display (gdk_display_get_default (),
                                                   window);
#endif

  return NULL;
}

/**
 * picman_ui_get_display_window:
 * @gdisp_ID: a #PicmanDisplay ID.
 *
 * Returns the #GdkWindow of a display window. The purpose is to allow
 * to make plug-in dialogs transient to the image display as explained
 * with gdk_window_set_transient_for().
 *
 * You shouldn't have to call this function directly. Use
 * picman_window_set_transient_for_display() instead.
 *
 * Return value: A reference to a #GdkWindow or %NULL. You should
 *               unref the window using g_object_unref() as soon as
 *               you don't need it any longer.
 *
 * Since: PICMAN 2.4
 */
GdkWindow *
picman_ui_get_display_window (guint32 gdisp_ID)
{
  guint32 window;

  g_return_val_if_fail (picman_ui_initialized, NULL);

  window = picman_display_get_window_handle (gdisp_ID);
  if (window)
    return picman_ui_get_foreign_window (window);

  return NULL;
}

/**
 * picman_ui_get_progress_window:
 *
 * Returns the #GdkWindow of the window this plug-in's progress bar is
 * shown in. Use it to make plug-in dialogs transient to this window
 * as explained with gdk_window_set_transient_for().
 *
 * You shouldn't have to call this function directly. Use
 * picman_window_set_transient() instead.
 *
 * Return value: A reference to a #GdkWindow or %NULL. You should
 *               unref the window using g_object_unref() as soon as
 *               you don't need it any longer.
 *
 * Since: PICMAN 2.4
 */
GdkWindow *
picman_ui_get_progress_window (void)
{
  guint32  window;

  g_return_val_if_fail (picman_ui_initialized, NULL);

  window = picman_progress_get_window_handle ();
  if (window)
     return picman_ui_get_foreign_window (window);

  return NULL;
}

#ifdef GDK_WINDOWING_QUARTZ
static void
picman_window_transient_show (GtkWidget *window)
{
  g_signal_handlers_disconnect_by_func (window,
                                        picman_window_transient_show,
                                        NULL);
  [NSApp arrangeInFront: nil];
}
#endif

/**
 * picman_window_set_transient_for_display:
 * @window:   the #GtkWindow that should become transient
 * @gdisp_ID: display ID of the image window that should become the parent
 *
 * Indicates to the window manager that @window is a transient dialog
 * associated with the PICMAN image window that is identified by it's
 * display ID.  See gdk_window_set_transient_for () for more information.
 *
 * Most of the time you will want to use the convenience function
 * picman_window_set_transient().
 *
 * Since: PICMAN 2.4
 */
void
picman_window_set_transient_for_display (GtkWindow *window,
                                       guint32    gdisp_ID)
{
  g_return_if_fail (picman_ui_initialized);
  g_return_if_fail (GTK_IS_WINDOW (window));

  if (! picman_window_set_transient_for (window,
                                       picman_ui_get_display_window (gdisp_ID)))
    {
      /*  if setting the window transient failed, at least set
       *  WIN_POS_CENTER, which will center the window on the screen
       *  where the mouse is (see bug #684003).
       */
      gtk_window_set_position (window, GTK_WIN_POS_CENTER);

#ifdef GDK_WINDOWING_QUARTZ
      g_signal_connect (window, "show",
                        G_CALLBACK (picman_window_transient_show),
                        NULL);
#endif
    }
}

/**
 * picman_window_set_transient:
 * @window: the #GtkWindow that should become transient
 *
 * Indicates to the window manager that @window is a transient dialog
 * associated with the PICMAN window that the plug-in has been
 * started from. See also picman_window_set_transient_for_display().
 *
 * Since: PICMAN 2.4
 */
void
picman_window_set_transient (GtkWindow *window)
{
  g_return_if_fail (picman_ui_initialized);
  g_return_if_fail (GTK_IS_WINDOW (window));

  if (! picman_window_set_transient_for (window, picman_ui_get_progress_window ()))
    {
      /*  see above  */
      gtk_window_set_position (window, GTK_WIN_POS_CENTER);

#ifdef GDK_WINDOWING_QUARTZ
      g_signal_connect (window, "show",
                        G_CALLBACK (picman_window_transient_show),
                        NULL);
#endif
    }
}


/*  private functions  */

static void
picman_ui_help_func (const gchar *help_id,
                   gpointer     help_data)
{
  picman_help (NULL, help_id);
}

static void
picman_ensure_modules (void)
{
  static PicmanModuleDB *module_db = NULL;

  if (! module_db)
    {
      gchar *load_inhibit = picman_get_module_load_inhibit ();
      gchar *module_path  = picman_picmanrc_query ("module-path");

      module_db = picman_module_db_new (FALSE);

      picman_module_db_set_load_inhibit (module_db, load_inhibit);
      picman_module_db_load (module_db, module_path);

      g_free (module_path);
      g_free (load_inhibit);
    }
}

static void
picman_window_transient_realized (GtkWidget *window,
                                GdkWindow *parent)
{
  if (gtk_widget_get_realized (window))
    gdk_window_set_transient_for (gtk_widget_get_window (window), parent);
}

static gboolean
picman_window_set_transient_for (GtkWindow *window,
                               GdkWindow *parent)
{
  gtk_window_set_transient_for (window, NULL);

#ifndef GDK_WINDOWING_WIN32
  g_signal_handlers_disconnect_matched (window, G_SIGNAL_MATCH_FUNC,
                                        0, 0, NULL,
                                        picman_window_transient_realized,
                                        NULL);

  if (! parent)
    return FALSE;

  if (gtk_widget_get_realized (GTK_WIDGET (window)))
    gdk_window_set_transient_for (gtk_widget_get_window (GTK_WIDGET (window)),
                                  parent);

  g_signal_connect_object (window, "realize",
                           G_CALLBACK (picman_window_transient_realized),
                           parent, 0);
  g_object_unref (parent);

  return TRUE;
#endif

  return FALSE;
}
