/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanview-popup.c
 * Copyright (C) 2003-2006 Michael Natterer <mitch@picman.org>
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

#include "core/picmancontext.h"
#include "core/picmanviewable.h"

#include "picmanview.h"
#include "picmanviewrenderer.h"
#include "picmanview-popup.h"


#define VIEW_POPUP_DELAY 150


typedef struct _PicmanViewPopup PicmanViewPopup;

struct _PicmanViewPopup
{
  GtkWidget    *widget;
  PicmanContext  *context;
  PicmanViewable *viewable;

  gint          popup_width;
  gint          popup_height;
  gboolean      dot_for_dot;
  gint          button;
  gint          button_x;
  gint          button_y;

  guint         timeout_id;
  GtkWidget    *popup;
};


/*  local function prototypes  */

static void       picman_view_popup_hide           (PicmanViewPopup  *popup);
static gboolean   picman_view_popup_button_release (GtkWidget      *widget,
                                                  GdkEventButton *bevent,
                                                  PicmanViewPopup  *popup);
static void       picman_view_popup_unmap          (GtkWidget      *widget,
                                                  PicmanViewPopup  *popup);
static void       picman_view_popup_drag_begin     (GtkWidget      *widget,
                                                  GdkDragContext *context,
                                                  PicmanViewPopup  *popup);
static gboolean   picman_view_popup_timeout        (PicmanViewPopup  *popup);


/*  public functions  */

gboolean
picman_view_popup_show (GtkWidget      *widget,
                      GdkEventButton *bevent,
                      PicmanContext    *context,
                      PicmanViewable   *viewable,
                      gint            view_width,
                      gint            view_height,
                      gboolean        dot_for_dot)
{
  PicmanViewPopup *popup;
  gint           popup_width;
  gint           popup_height;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (bevent != NULL, FALSE);
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (PICMAN_IS_VIEWABLE (viewable), FALSE);

  if (! picman_viewable_get_popup_size (viewable,
                                      view_width,
                                      view_height,
                                      dot_for_dot,
                                      &popup_width,
                                      &popup_height))
    return FALSE;

  popup = g_slice_new0 (PicmanViewPopup);

  popup->widget       = widget;
  popup->context      = context;
  popup->viewable     = viewable;
  popup->popup_width  = popup_width;
  popup->popup_height = popup_height;
  popup->dot_for_dot  = dot_for_dot;
  popup->button       = bevent->button;
  popup->button_x     = bevent->x_root;
  popup->button_y     = bevent->y_root;

  g_signal_connect (widget, "button-release-event",
                    G_CALLBACK (picman_view_popup_button_release),
                    popup);
  g_signal_connect (widget, "unmap",
                    G_CALLBACK (picman_view_popup_unmap),
                    popup);
  g_signal_connect (widget, "drag-begin",
                    G_CALLBACK (picman_view_popup_drag_begin),
                    popup);

  popup->timeout_id = g_timeout_add (VIEW_POPUP_DELAY,
                                     (GSourceFunc) picman_view_popup_timeout,
                                     popup);

  g_object_set_data_full (G_OBJECT (widget), "picman-view-popup", popup,
                          (GDestroyNotify) picman_view_popup_hide);

  gtk_grab_add (widget);

  return TRUE;
}


/*  private functions  */

static void
picman_view_popup_hide (PicmanViewPopup *popup)
{
  if (popup->timeout_id)
    g_source_remove (popup->timeout_id);

  if (popup->popup)
    gtk_widget_destroy (popup->popup);

  g_signal_handlers_disconnect_by_func (popup->widget,
                                        picman_view_popup_button_release,
                                        popup);
  g_signal_handlers_disconnect_by_func (popup->widget,
                                        picman_view_popup_unmap,
                                        popup);
  g_signal_handlers_disconnect_by_func (popup->widget,
                                        picman_view_popup_drag_begin,
                                        popup);

  gtk_grab_remove (popup->widget);

  g_slice_free (PicmanViewPopup, popup);
}

static gboolean
picman_view_popup_button_release (GtkWidget      *widget,
                                GdkEventButton *bevent,
                                PicmanViewPopup  *popup)
{
  if (bevent->button == popup->button)
    g_object_set_data (G_OBJECT (popup->widget), "picman-view-popup", NULL);

  return FALSE;
}

static void
picman_view_popup_unmap (GtkWidget     *widget,
                       PicmanViewPopup *popup)
{
  g_object_set_data (G_OBJECT (popup->widget), "picman-view-popup", NULL);
}

static void
picman_view_popup_drag_begin (GtkWidget      *widget,
                            GdkDragContext *context,
                            PicmanViewPopup  *popup)
{
  g_object_set_data (G_OBJECT (popup->widget), "picman-view-popup", NULL);
}

static gboolean
picman_view_popup_timeout (PicmanViewPopup *popup)
{
  GtkWidget    *window;
  GtkWidget    *frame;
  GtkWidget    *view;
  GdkScreen    *screen;
  GdkRectangle  rect;
  gint          monitor;
  gint          x;
  gint          y;

  popup->timeout_id = 0;

  screen = gtk_widget_get_screen (popup->widget);

  window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

  gtk_window_set_screen (GTK_WINDOW (window), screen);
  gtk_window_set_transient_for (GTK_WINDOW (window),
                                GTK_WINDOW (gtk_widget_get_toplevel (popup->widget)));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (window), frame);
  gtk_widget_show (frame);

  view = picman_view_new_full (popup->context,
                             popup->viewable,
                             popup->popup_width,
                             popup->popup_height,
                             0, TRUE, FALSE, FALSE);
  picman_view_renderer_set_dot_for_dot (PICMAN_VIEW (view)->renderer,
                                      popup->dot_for_dot);
  gtk_container_add (GTK_CONTAINER (frame), view);
  gtk_widget_show (view);

  x = popup->button_x - (popup->popup_width  / 2);
  y = popup->button_y - (popup->popup_height / 2);

  monitor = gdk_screen_get_monitor_at_point (screen, x, y);
  gdk_screen_get_monitor_geometry (screen, monitor, &rect);

  x = CLAMP (x, rect.x, rect.x + rect.width  - popup->popup_width);
  y = CLAMP (y, rect.y, rect.y + rect.height - popup->popup_height);

  gtk_window_move (GTK_WINDOW (window), x, y);
  gtk_widget_show (window);

  popup->popup = window;

  return FALSE;
}
