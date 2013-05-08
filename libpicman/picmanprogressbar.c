/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanprogressbar.c
 * Copyright (C) 2004 Michael Natterer <mitch@picman.org>
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

#include "picmanuitypes.h"

#include "picman.h"

#include "picmanprogressbar.h"


/**
 * SECTION: picmanprogressbar
 * @title: PicmanProgressBar
 * @short_description: A widget providing a progress bar.
 *
 * A widget providing a progress bar that automatically redirects any
 * progress calls to itself.
 **/


static void     picman_progress_bar_dispose    (GObject     *object);

static void     picman_progress_bar_start      (const gchar *message,
                                              gboolean     cancelable,
                                              gpointer     user_data);
static void     picman_progress_bar_end        (gpointer     user_data);
static void     picman_progress_bar_set_text   (const gchar *message,
                                              gpointer     user_data);
static void     picman_progress_bar_set_value  (gdouble      percentage,
                                              gpointer     user_data);
static void     picman_progress_bar_pulse      (gpointer     user_data);
static guint32  picman_progress_bar_get_window (gpointer     user_data);


G_DEFINE_TYPE (PicmanProgressBar, picman_progress_bar, GTK_TYPE_PROGRESS_BAR)

#define parent_class picman_progress_bar_parent_class


static void
picman_progress_bar_class_init (PicmanProgressBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_progress_bar_dispose;
}

static void
picman_progress_bar_init (PicmanProgressBar *bar)
{
  PicmanProgressVtable vtable = { 0, };

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (bar), " ");
  gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (bar), PANGO_ELLIPSIZE_END);

  vtable.start      = picman_progress_bar_start;
  vtable.end        = picman_progress_bar_end;
  vtable.set_text   = picman_progress_bar_set_text;
  vtable.set_value  = picman_progress_bar_set_value;
  vtable.pulse      = picman_progress_bar_pulse;
  vtable.get_window = picman_progress_bar_get_window;

  bar->progress_callback = picman_progress_install_vtable (&vtable, bar);
}

static void
picman_progress_bar_dispose (GObject *object)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (object);

  if (bar->progress_callback)
    {
      picman_progress_uninstall (bar->progress_callback);
      bar->progress_callback = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_progress_bar_start (const gchar *message,
                         gboolean     cancelable,
                         gpointer     user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (bar), message ? message : " ");
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), 0.0);

  if (gtk_widget_is_drawable (GTK_WIDGET (bar)))
    while (gtk_events_pending ())
      gtk_main_iteration ();
}

static void
picman_progress_bar_end (gpointer user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (bar), " ");
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), 0.0);

  if (gtk_widget_is_drawable (GTK_WIDGET (bar)))
    while (gtk_events_pending ())
      gtk_main_iteration ();
}

static void
picman_progress_bar_set_text (const gchar *message,
                            gpointer     user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);

  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (bar), message ? message : " ");

  if (gtk_widget_is_drawable (GTK_WIDGET (bar)))
    while (gtk_events_pending ())
      gtk_main_iteration ();
}

static void
picman_progress_bar_set_value (gdouble  percentage,
                             gpointer user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);

  if (percentage >= 0.0)
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (bar), percentage);
  else
    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (bar));

  if (gtk_widget_is_drawable (GTK_WIDGET (bar)))
    while (gtk_events_pending ())
      gtk_main_iteration ();
}

static void
picman_progress_bar_pulse (gpointer user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);

  gtk_progress_bar_pulse (GTK_PROGRESS_BAR (bar));

  if (gtk_widget_is_drawable (GTK_WIDGET (bar)))
    while (gtk_events_pending ())
      gtk_main_iteration ();
}

static guint32
picman_window_get_native_id (GtkWindow *window)
{
  g_return_val_if_fail (GTK_IS_WINDOW (window), 0);

#ifdef GDK_NATIVE_WINDOW_POINTER
#ifdef __GNUC__
#warning picman_window_get_native() unimplementable for the target windowing system
#endif
#endif

#ifdef GDK_WINDOWING_WIN32
  if (window && gtk_widget_get_realized (GTK_WIDGET (window)))
    return GDK_WINDOW_HWND (gtk_widget_get_window (GTK_WIDGET (window)));
#endif

#ifdef GDK_WINDOWING_X11
  if (window && gtk_widget_get_realized (GTK_WIDGET (window)))
    return GDK_WINDOW_XID (gtk_widget_get_window (GTK_WIDGET (window)));
#endif

  return 0;
}

static guint32
picman_progress_bar_get_window (gpointer user_data)
{
  PicmanProgressBar *bar = PICMAN_PROGRESS_BAR (user_data);
  GtkWidget       *toplevel;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (bar));

  if (GTK_IS_WINDOW (toplevel))
    return picman_window_get_native_id (GTK_WINDOW (toplevel));

  return 0;
}

/**
 * picman_progress_bar_new:
 *
 * Creates a new #PicmanProgressBar widget.
 *
 * Return value: the new widget.
 *
 * Since: PICMAN 2.2
 **/
GtkWidget *
picman_progress_bar_new (void)
{
  return g_object_new (PICMAN_TYPE_PROGRESS_BAR, NULL);
}
