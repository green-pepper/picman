/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanNavigationView Widget
 * Copyright (C) 2001-2002 Michael Natterer <mitch@picman.org>
 *
 * partly based on app/nav_window
 * Copyright (C) 1999 Andy Thomas <alt@picman.org>
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
#include <gdk/gdkkeysyms.h>

#include "libpicmanmath/picmanmath.h"

#include "widgets-types.h"

#include "core/picmanimage.h"
#include "core/picmanmarshal.h"

#include "picmannavigationview.h"
#include "picmanviewrenderer.h"
#include "picmanwidgets-utils.h"


#define BORDER_WIDTH 2


enum
{
  MARKER_CHANGED,
  ZOOM,
  SCROLL,
  LAST_SIGNAL
};


struct _PicmanNavigationView
{
  PicmanView     parent_instance;

  /*  values in image coordinates  */
  gdouble      x;
  gdouble      y;
  gdouble      width;
  gdouble      height;

  /*  values in view coordinates  */
  gint         p_x;
  gint         p_y;
  gint         p_width;
  gint         p_height;

  gint         motion_offset_x;
  gint         motion_offset_y;
  gboolean     has_grab;
};


static void     picman_navigation_view_size_allocate  (GtkWidget      *widget,
                                                     GtkAllocation  *allocation);
static gboolean picman_navigation_view_expose         (GtkWidget      *widget,
                                                     GdkEventExpose *event);
static gboolean picman_navigation_view_button_press   (GtkWidget      *widget,
                                                     GdkEventButton *bevent);
static gboolean picman_navigation_view_button_release (GtkWidget      *widget,
                                                     GdkEventButton *bevent);
static gboolean picman_navigation_view_scroll         (GtkWidget      *widget,
                                                     GdkEventScroll *sevent);
static gboolean picman_navigation_view_motion_notify  (GtkWidget      *widget,
                                                     GdkEventMotion *mevent);
static gboolean picman_navigation_view_key_press      (GtkWidget      *widget,
                                                     GdkEventKey    *kevent);

static void     picman_navigation_view_transform      (PicmanNavigationView *nav_view);
static void     picman_navigation_view_draw_marker    (PicmanNavigationView *nav_view,
                                                     cairo_t            *cr);
static void     picman_navigation_view_move_to        (PicmanNavigationView *nav_view,
                                                     gint                tx,
                                                     gint                ty);
static void     picman_navigation_view_get_ratio      (PicmanNavigationView *nav_view,
                                                     gdouble            *ratiox,
                                                     gdouble            *ratioy);


G_DEFINE_TYPE (PicmanNavigationView, picman_navigation_view, PICMAN_TYPE_VIEW)

#define parent_class picman_navigation_view_parent_class

static guint view_signals[LAST_SIGNAL] = { 0 };


static void
picman_navigation_view_class_init (PicmanNavigationViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  view_signals[MARKER_CHANGED] =
    g_signal_new ("marker-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanNavigationViewClass, marker_changed),
                  NULL, NULL,
                  picman_marshal_VOID__DOUBLE_DOUBLE_DOUBLE_DOUBLE,
                  G_TYPE_NONE, 4,
                  G_TYPE_DOUBLE,
                  G_TYPE_DOUBLE,
                  G_TYPE_DOUBLE,
                  G_TYPE_DOUBLE);

  view_signals[ZOOM] =
    g_signal_new ("zoom",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanNavigationViewClass, zoom),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_ZOOM_TYPE);

  view_signals[SCROLL] =
    g_signal_new ("scroll",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanNavigationViewClass, scroll),
                  NULL, NULL,
                  picman_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  GDK_TYPE_SCROLL_DIRECTION);

  widget_class->size_allocate        = picman_navigation_view_size_allocate;
  widget_class->expose_event         = picman_navigation_view_expose;
  widget_class->button_press_event   = picman_navigation_view_button_press;
  widget_class->button_release_event = picman_navigation_view_button_release;
  widget_class->scroll_event         = picman_navigation_view_scroll;
  widget_class->motion_notify_event  = picman_navigation_view_motion_notify;
  widget_class->key_press_event      = picman_navigation_view_key_press;
}

static void
picman_navigation_view_init (PicmanNavigationView *view)
{
  gtk_widget_set_can_focus (GTK_WIDGET (view), TRUE);
  gtk_widget_add_events (GTK_WIDGET (view),
                         GDK_POINTER_MOTION_MASK |
                         GDK_KEY_PRESS_MASK);

  view->x               = 0.0;
  view->y               = 0.0;
  view->width           = 0.0;
  view->height          = 0.0;

  view->p_x             = 0;
  view->p_y             = 0;
  view->p_width         = 0;
  view->p_height        = 0;

  view->motion_offset_x = 0;
  view->motion_offset_y = 0;
  view->has_grab        = FALSE;
}

static void
picman_navigation_view_size_allocate (GtkWidget     *widget,
                                    GtkAllocation *allocation)
{
  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  if (PICMAN_VIEW (widget)->renderer->viewable)
    picman_navigation_view_transform (PICMAN_NAVIGATION_VIEW (widget));
}

static gboolean
picman_navigation_view_expose (GtkWidget      *widget,
                             GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      cairo_t *cr;

      GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);

      cr = gdk_cairo_create (gtk_widget_get_window (widget));

      gdk_cairo_region (cr, event->region);
      cairo_clip (cr);

      picman_navigation_view_draw_marker (PICMAN_NAVIGATION_VIEW (widget), cr);

      cairo_destroy (cr);
    }

  return TRUE;
}

void
picman_navigation_view_grab_pointer (PicmanNavigationView *nav_view)
{
  GtkWidget  *widget = GTK_WIDGET (nav_view);
  GdkDisplay *display;
  GdkCursor  *cursor;
  GdkWindow  *window;

  nav_view->has_grab = TRUE;

  gtk_grab_add (widget);

  display = gtk_widget_get_display (widget);
  cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);

  window = PICMAN_VIEW (nav_view)->event_window;

  gdk_pointer_grab (window, FALSE,
                    GDK_BUTTON_RELEASE_MASK      |
                    GDK_POINTER_MOTION_HINT_MASK |
                    GDK_BUTTON_MOTION_MASK       |
                    GDK_EXTENSION_EVENTS_ALL,
                    NULL, cursor, GDK_CURRENT_TIME);

  gdk_cursor_unref (cursor);
}

static gboolean
picman_navigation_view_button_press (GtkWidget      *widget,
                                   GdkEventButton *bevent)
{
  PicmanNavigationView *nav_view = PICMAN_NAVIGATION_VIEW (widget);
  gint                tx, ty;
  GdkDisplay         *display;

  tx = bevent->x;
  ty = bevent->y;

  if (bevent->type == GDK_BUTTON_PRESS && bevent->button == 1)
    {
      if (! (tx >  nav_view->p_x &&
             tx < (nav_view->p_x + nav_view->p_width) &&
             ty >  nav_view->p_y &&
             ty < (nav_view->p_y + nav_view->p_height)))
        {
          GdkCursor *cursor;

          nav_view->motion_offset_x = nav_view->p_width  / 2;
          nav_view->motion_offset_y = nav_view->p_height / 2;

          tx -= nav_view->motion_offset_x;
          ty -= nav_view->motion_offset_y;

          picman_navigation_view_move_to (nav_view, tx, ty);

          display = gtk_widget_get_display (widget);
          cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);
          gdk_window_set_cursor (PICMAN_VIEW (widget)->event_window, cursor);
          gdk_cursor_unref (cursor);
        }
      else
        {
          nav_view->motion_offset_x = tx - nav_view->p_x;
          nav_view->motion_offset_y = ty - nav_view->p_y;
        }

      picman_navigation_view_grab_pointer (nav_view);
    }

  return TRUE;
}

static gboolean
picman_navigation_view_button_release (GtkWidget      *widget,
                                     GdkEventButton *bevent)
{
  PicmanNavigationView *nav_view = PICMAN_NAVIGATION_VIEW (widget);

  if (bevent->button == 1 && nav_view->has_grab)
    {
      nav_view->has_grab = FALSE;

      gtk_grab_remove (widget);
      gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                  GDK_CURRENT_TIME);
    }

  return TRUE;
}

static gboolean
picman_navigation_view_scroll (GtkWidget      *widget,
                             GdkEventScroll *sevent)
{
  if (sevent->state & picman_get_toggle_behavior_mask ())
    {
      switch (sevent->direction)
        {
        case GDK_SCROLL_UP:
          g_signal_emit (widget, view_signals[ZOOM], 0, PICMAN_ZOOM_IN);
          break;

        case GDK_SCROLL_DOWN:
          g_signal_emit (widget, view_signals[ZOOM], 0, PICMAN_ZOOM_OUT);
          break;

        default:
          break;
        }
    }
  else
    {
      GdkScrollDirection direction = sevent->direction;

      if (sevent->state & GDK_SHIFT_MASK)
        switch (direction)
          {
          case GDK_SCROLL_UP:    direction = GDK_SCROLL_LEFT;  break;
          case GDK_SCROLL_DOWN:  direction = GDK_SCROLL_RIGHT; break;
          case GDK_SCROLL_LEFT:  direction = GDK_SCROLL_UP;    break;
          case GDK_SCROLL_RIGHT: direction = GDK_SCROLL_DOWN;  break;
          }

      g_signal_emit (widget, view_signals[SCROLL], 0, direction);
    }

  return TRUE;
}

static gboolean
picman_navigation_view_motion_notify (GtkWidget      *widget,
                                    GdkEventMotion *mevent)
{
  PicmanNavigationView *nav_view = PICMAN_NAVIGATION_VIEW (widget);
  PicmanView           *view     = PICMAN_VIEW (widget);

  if (! nav_view->has_grab)
    {
      GdkDisplay *display = gtk_widget_get_display (widget);
      GdkCursor  *cursor;

      if (nav_view->p_x == 0 &&
          nav_view->p_y == 0 &&
          nav_view->p_width  == view->renderer->width &&
          nav_view->p_height == view->renderer->height)
        {
          gdk_window_set_cursor (view->event_window, NULL);
          return FALSE;
        }
      else if (mevent->x >= nav_view->p_x &&
               mevent->y >= nav_view->p_y &&
               mevent->x <  nav_view->p_x + nav_view->p_width &&
               mevent->y <  nav_view->p_y + nav_view->p_height)
        {
          cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);
        }
      else
        {
          cursor = gdk_cursor_new_for_display (display, GDK_HAND2);
        }

      gdk_window_set_cursor (view->event_window, cursor);
      gdk_cursor_unref (cursor);

      return FALSE;
    }

  picman_navigation_view_move_to (nav_view,
                                mevent->x - nav_view->motion_offset_x,
                                mevent->y - nav_view->motion_offset_y);

  gdk_event_request_motions (mevent);

  return TRUE;
}

static gboolean
picman_navigation_view_key_press (GtkWidget   *widget,
                                GdkEventKey *kevent)
{
  PicmanNavigationView *nav_view = PICMAN_NAVIGATION_VIEW (widget);
  gint                scroll_x = 0;
  gint                scroll_y = 0;

  switch (kevent->keyval)
    {
    case GDK_KEY_Up:
      scroll_y = -1;
      break;

    case GDK_KEY_Left:
      scroll_x = -1;
      break;

    case GDK_KEY_Right:
      scroll_x = 1;
      break;

    case GDK_KEY_Down:
      scroll_y = 1;
      break;

    default:
      break;
    }

  if (scroll_x || scroll_y)
    {
      picman_navigation_view_move_to (nav_view,
                                    nav_view->p_x + scroll_x,
                                    nav_view->p_y + scroll_y);
      return TRUE;
    }

  return FALSE;
}


/*  public functions  */

void
picman_navigation_view_set_marker (PicmanNavigationView *nav_view,
                                 gdouble             x,
                                 gdouble             y,
                                 gdouble             width,
                                 gdouble             height)
{
  PicmanView *view;

  g_return_if_fail (PICMAN_IS_NAVIGATION_VIEW (nav_view));

  view = PICMAN_VIEW (nav_view);

  g_return_if_fail (view->renderer->viewable);

  nav_view->x      = x;
  nav_view->y      = y;
  nav_view->width  = MAX (1.0, width);
  nav_view->height = MAX (1.0, height);

  picman_navigation_view_transform (nav_view);

  /* Marker changed, redraw */
  gtk_widget_queue_draw (GTK_WIDGET (view));
}

void
picman_navigation_view_set_motion_offset (PicmanNavigationView *view,
                                        gint                motion_offset_x,
                                        gint                motion_offset_y)
{
  g_return_if_fail (PICMAN_IS_NAVIGATION_VIEW (view));

  view->motion_offset_x = motion_offset_x;
  view->motion_offset_y = motion_offset_y;
}

void
picman_navigation_view_get_local_marker (PicmanNavigationView *view,
                                       gint               *x,
                                       gint               *y,
                                       gint               *width,
                                       gint               *height)
{
  g_return_if_fail (PICMAN_IS_NAVIGATION_VIEW (view));

  if (x)      *x      = view->p_x;
  if (y)      *y      = view->p_y;
  if (width)  *width  = view->p_width;
  if (height) *height = view->p_height;
}


/*  private functions  */

static void
picman_navigation_view_transform (PicmanNavigationView *nav_view)
{
  gdouble ratiox, ratioy;

  picman_navigation_view_get_ratio (nav_view, &ratiox, &ratioy);

  nav_view->p_x = RINT (nav_view->x * ratiox);
  nav_view->p_y = RINT (nav_view->y * ratioy);

  nav_view->p_width  = ceil (nav_view->width  * ratiox);
  nav_view->p_height = ceil (nav_view->height * ratioy);
}

static void
picman_navigation_view_draw_marker (PicmanNavigationView *nav_view,
                                  cairo_t            *cr)
{
  PicmanView *view = PICMAN_VIEW (nav_view);

  if (view->renderer->viewable && nav_view->width && nav_view->height)
    {
      GtkWidget     *widget = GTK_WIDGET (view);
      GtkAllocation  allocation;

      gtk_widget_get_allocation (widget, &allocation);

      cairo_translate (cr, allocation.x, allocation.y);
      cairo_rectangle (cr,
                       0, 0,
                       allocation.width, allocation.height);
      cairo_rectangle (cr,
                       nav_view->p_x, nav_view->p_y,
                       nav_view->p_width, nav_view->p_height);

      cairo_set_source_rgba (cr, 0, 0, 0, 0.5);
      cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
      cairo_fill (cr);

      cairo_rectangle (cr,
                       nav_view->p_x, nav_view->p_y,
                       nav_view->p_width, nav_view->p_height);

      cairo_set_source_rgb (cr, 1, 1, 1);
      cairo_set_line_width (cr, BORDER_WIDTH);
      cairo_stroke (cr);
    }
}

static void
picman_navigation_view_move_to (PicmanNavigationView *nav_view,
                              gint                tx,
                              gint                ty)
{
  PicmanView  *view = PICMAN_VIEW (nav_view);
  gdouble    ratiox, ratioy;
  gdouble    x, y;

  if (! view->renderer->viewable)
    return;

  picman_navigation_view_get_ratio (nav_view, &ratiox, &ratioy);

  x = tx / ratiox;
  y = ty / ratioy;

  g_signal_emit (view, view_signals[MARKER_CHANGED], 0,
                 x, y, nav_view->width, nav_view->height);
}

static void
picman_navigation_view_get_ratio (PicmanNavigationView *nav_view,
                                gdouble            *ratiox,
                                gdouble            *ratioy)
{
  PicmanView  *view = PICMAN_VIEW (nav_view);
  PicmanImage *image;

  image = PICMAN_IMAGE (view->renderer->viewable);

  *ratiox = (gdouble) view->renderer->width  /
            (gdouble) picman_image_get_width  (image);
  *ratioy = (gdouble) view->renderer->height /
            (gdouble) picman_image_get_height (image);
}
