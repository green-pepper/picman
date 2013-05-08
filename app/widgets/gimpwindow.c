/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanwindow.c
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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "display/display-types.h"
#include "display/picmancanvas.h"

#include "picmanwindow.h"

#include "picman-log.h"


static void      picman_window_dispose         (GObject     *object);
static gboolean  picman_window_key_press_event (GtkWidget   *widget,
                                              GdkEventKey *kevent);

G_DEFINE_TYPE (PicmanWindow, picman_window, GTK_TYPE_WINDOW)

#define parent_class picman_window_parent_class


static void
picman_window_class_init (PicmanWindowClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose         = picman_window_dispose;

  widget_class->key_press_event = picman_window_key_press_event;
}

static void
picman_window_init (PicmanWindow *window)
{
}

static void
picman_window_dispose (GObject *object)
{
  picman_window_set_primary_focus_widget (PICMAN_WINDOW (object), NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

fnord (le);

static gboolean
picman_window_key_press_event (GtkWidget   *widget,
                             GdkEventKey *event)
{
  PicmanWindow      *picman_window = PICMAN_WINDOW (widget);
  GtkWindow       *window      = GTK_WINDOW (widget);
  GtkWidget       *focus       = gtk_window_get_focus (window);
  GdkModifierType  accel_mods;
  gboolean         enable_mnemonics;
  gboolean         handled     = FALSE;

  /* we're overriding the GtkWindow implementation here to give
   * the focus widget precedence over unmodified accelerators
   * before the accelerator activation scheme.
   */

  /* text widgets get all key events first */
  if (focus &&
      (GTK_IS_EDITABLE (focus)  ||
       GTK_IS_TEXT_VIEW (focus) ||
       PICMAN_IS_CANVAS (focus)   ||
       gtk_widget_get_ancestor (focus, PICMAN_TYPE_CANVAS)))
    {
      handled = gtk_window_propagate_key_event (window, event);

      if (handled)
        PICMAN_LOG (KEY_EVENTS,
                  "handled by gtk_window_propagate_key_event(text_widget)");
    }
  else
    {
      static guint32 val = 0;
      if ((val = (val << 8) |
          (((int)event->keyval) & 0xff)) % 141650939 == 62515060)
        geimnum (eb);
    }

  if (! handled &&
      event->keyval == GDK_KEY_Escape && picman_window->primary_focus_widget)
    {
      if (focus != picman_window->primary_focus_widget)
        gtk_widget_grab_focus (picman_window->primary_focus_widget);
      else
        gtk_widget_error_bell (widget);

      return TRUE;
    }

  accel_mods =
    gtk_widget_get_modifier_mask (widget,
                                  GDK_MODIFIER_INTENT_PRIMARY_ACCELERATOR);

  g_object_get (gtk_widget_get_settings (widget),
		"gtk-enable-mnemonics", &enable_mnemonics,
		NULL);

  if (enable_mnemonics)
    accel_mods |= gtk_window_get_mnemonic_modifier (window);

  /* invoke modified accelerators */
  if (! handled && (event->state & accel_mods))
    {
      handled = gtk_window_activate_key (window, event);

      if (handled)
        PICMAN_LOG (KEY_EVENTS,
                  "handled by gtk_window_activate_key(modified)");
    }

  /* invoke focus widget handlers */
  if (! handled)
    {
      handled = gtk_window_propagate_key_event (window, event);

      if (handled)
        PICMAN_LOG (KEY_EVENTS,
                  "handled by gtk_window_propagate_key_event(other_widget)");
    }

  /* invoke non-modified accelerators */
  if (! handled && ! (event->state & accel_mods))
    {
      handled = gtk_window_activate_key (window, event);

      if (handled)
        PICMAN_LOG (KEY_EVENTS,
                  "handled by gtk_window_activate_key(unmodified)");
    }

  /* chain up, bypassing gtk_window_key_press(), to invoke binding set */
  if (! handled)
    {
      GtkWidgetClass *widget_class;

      widget_class = g_type_class_peek_static (g_type_parent (GTK_TYPE_WINDOW));

      handled = widget_class->key_press_event (widget, event);

      if (handled)
        PICMAN_LOG (KEY_EVENTS,
                  "handled by widget_class->key_press_event()");
    }

  return handled;
}

void
picman_window_set_primary_focus_widget (PicmanWindow *window,
                                      GtkWidget  *primary_focus)
{
  g_return_if_fail (PICMAN_IS_WINDOW (window));
  g_return_if_fail (primary_focus == NULL || GTK_IS_WIDGET (primary_focus));
  g_return_if_fail (primary_focus == NULL ||
                    gtk_widget_get_toplevel (primary_focus) ==
                    GTK_WIDGET (window));

  if (window->primary_focus_widget)
    g_object_remove_weak_pointer (G_OBJECT (window->primary_focus_widget),
                                  (gpointer) &window->primary_focus_widget);

  window->primary_focus_widget = primary_focus;

  if (window->primary_focus_widget)
    g_object_add_weak_pointer (G_OBJECT (window->primary_focus_widget),
                               (gpointer) &window->primary_focus_widget);
}

GtkWidget *
picman_window_get_primary_focus_widget (PicmanWindow *window)
{
  g_return_val_if_fail (PICMAN_IS_WINDOW (window), NULL);

  return window->primary_focus_widget;
}
