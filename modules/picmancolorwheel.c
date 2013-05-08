/* HSV color selector for GTK+
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Simon Budig <Simon.Budig@unix-ag.org> (original code)
 *          Federico Mena-Quintero <federico@picman.org> (cleanup for GTK+)
 *          Jonathan Blandford <jrb@redhat.com> (cleanup for GTK+)
 *          Michael Natterer <mitch@picman.org> (ported back to PICMAN)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <libpicmanmath/picmanmath.h>

#include "picmancolorwheel.h"


/* Default ring fraction */
#define DEFAULT_FRACTION 0.1

/* Default width/height */
#define DEFAULT_SIZE 100

/* Default ring width */
#define DEFAULT_RING_WIDTH 10


/* Dragging modes */
typedef enum
{
  DRAG_NONE,
  DRAG_H,
  DRAG_SV
} DragMode;

/* Private part of the PicmanColorWheel structure */
typedef struct
{
  /* Color value */
  gdouble h;
  gdouble s;
  gdouble v;

  /* ring_width is this fraction of size */
  gdouble ring_fraction;

  /* Size and ring width */
  gint size;
  gint ring_width;

  /* Window for capturing events */
  GdkWindow *window;

  /* Dragging mode */
  DragMode mode;

  guint focus_on_ring : 1;
} PicmanColorWheelPrivate;

enum
{
  CHANGED,
  MOVE,
  LAST_SIGNAL
};

static void     picman_color_wheel_map            (GtkWidget          *widget);
static void     picman_color_wheel_unmap          (GtkWidget          *widget);
static void     picman_color_wheel_realize        (GtkWidget          *widget);
static void     picman_color_wheel_unrealize      (GtkWidget          *widget);
static void     picman_color_wheel_size_request   (GtkWidget          *widget,
                                                 GtkRequisition     *requisition);
static void     picman_color_wheel_size_allocate  (GtkWidget          *widget,
                                                 GtkAllocation      *allocation);
static gboolean picman_color_wheel_button_press   (GtkWidget          *widget,
                                                 GdkEventButton     *event);
static gboolean picman_color_wheel_button_release (GtkWidget          *widget,
                                                 GdkEventButton     *event);
static gboolean picman_color_wheel_motion         (GtkWidget          *widget,
                                                 GdkEventMotion     *event);
static gboolean picman_color_wheel_expose         (GtkWidget          *widget,
                                                 GdkEventExpose     *event);
static gboolean picman_color_wheel_grab_broken    (GtkWidget          *widget,
                                                 GdkEventGrabBroken *event);
static gboolean picman_color_wheel_focus          (GtkWidget          *widget,
                                                 GtkDirectionType    direction);
static void     picman_color_wheel_move           (PicmanColorWheel     *wheel,
                                                 GtkDirectionType    dir);


static guint wheel_signals[LAST_SIGNAL];

G_DEFINE_TYPE (PicmanColorWheel, picman_color_wheel, GTK_TYPE_WIDGET)

#define parent_class picman_color_wheel_parent_class


static void
picman_color_wheel_class_init (PicmanColorWheelClass *class)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass      *widget_class = GTK_WIDGET_CLASS (class);
  PicmanColorWheelClass *wheel_class  = PICMAN_COLOR_WHEEL_CLASS (class);
  GtkBindingSet       *binding_set;

  widget_class->map                  = picman_color_wheel_map;
  widget_class->unmap                = picman_color_wheel_unmap;
  widget_class->realize              = picman_color_wheel_realize;
  widget_class->unrealize            = picman_color_wheel_unrealize;
  widget_class->size_request         = picman_color_wheel_size_request;
  widget_class->size_allocate        = picman_color_wheel_size_allocate;
  widget_class->button_press_event   = picman_color_wheel_button_press;
  widget_class->button_release_event = picman_color_wheel_button_release;
  widget_class->motion_notify_event  = picman_color_wheel_motion;
  widget_class->expose_event         = picman_color_wheel_expose;
  widget_class->focus                = picman_color_wheel_focus;
  widget_class->grab_broken_event    = picman_color_wheel_grab_broken;

  wheel_class->move                  = picman_color_wheel_move;

  wheel_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorWheelClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  wheel_signals[MOVE] =
    g_signal_new ("move",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (PicmanColorWheelClass, move),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  GTK_TYPE_DIRECTION_TYPE);

  binding_set = gtk_binding_set_by_class (class);

  gtk_binding_entry_add_signal (binding_set, GDK_Up, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_UP);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Up, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_UP);

  gtk_binding_entry_add_signal (binding_set, GDK_Down, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_DOWN);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Down, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_DOWN);


  gtk_binding_entry_add_signal (binding_set, GDK_Right, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_RIGHT);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Right, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_RIGHT);

  gtk_binding_entry_add_signal (binding_set, GDK_Left, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_LEFT);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Left, 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_LEFT);

  g_type_class_add_private (object_class, sizeof (PicmanColorWheelPrivate));
}

static void
picman_color_wheel_init (PicmanColorWheel *wheel)
{
  PicmanColorWheelPrivate *priv;

  priv = G_TYPE_INSTANCE_GET_PRIVATE (wheel, PICMAN_TYPE_COLOR_WHEEL,
                                      PicmanColorWheelPrivate);

  wheel->priv = priv;

  gtk_widget_set_has_window (GTK_WIDGET (wheel), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (wheel), TRUE);

  priv->ring_fraction = DEFAULT_FRACTION;
  priv->size          = DEFAULT_SIZE;
  priv->ring_width    = DEFAULT_RING_WIDTH;
}

static void
picman_color_wheel_map (GtkWidget *widget)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;

  GTK_WIDGET_CLASS (parent_class)->map (widget);

  gdk_window_show (priv->window);
}

static void
picman_color_wheel_unmap (GtkWidget *widget)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;

  gdk_window_hide (priv->window);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_color_wheel_realize (GtkWidget *widget)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  GtkAllocation          allocation;
  GdkWindowAttr          attr;
  gint                   attr_mask;
  GdkWindow             *parent_window;

  gtk_widget_get_allocation (widget, &allocation);

  gtk_widget_set_realized (widget, TRUE);

  attr.window_type = GDK_WINDOW_CHILD;
  attr.x           = allocation.x;
  attr.y           = allocation.y;
  attr.width       = allocation.width;
  attr.height      = allocation.height;
  attr.wclass      = GDK_INPUT_ONLY;
  attr.event_mask  = (gtk_widget_get_events (widget) |
                      GDK_KEY_PRESS_MASK      |
                      GDK_BUTTON_PRESS_MASK   |
                      GDK_BUTTON_RELEASE_MASK |
                      GDK_POINTER_MOTION_MASK |
                      GDK_ENTER_NOTIFY_MASK   |
                      GDK_LEAVE_NOTIFY_MASK);

  attr_mask = GDK_WA_X | GDK_WA_Y;

  parent_window = gtk_widget_get_parent_window (widget);

  gtk_widget_set_window (widget, parent_window);
  g_object_ref (parent_window);

  priv->window = gdk_window_new (parent_window, &attr, attr_mask);
  gdk_window_set_user_data (priv->window, wheel);

  gtk_widget_style_attach (widget);
}

static void
picman_color_wheel_unrealize (GtkWidget *widget)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;

  gdk_window_set_user_data (priv->window, NULL);
  gdk_window_destroy (priv->window);
  priv->window = NULL;

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_color_wheel_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  gint focus_width;
  gint focus_pad;

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding", &focus_pad,
                        NULL);

  requisition->width  = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
  requisition->height = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
}

static void
picman_color_wheel_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  gint                   focus_width;
  gint                   focus_pad;

  gtk_widget_set_allocation (widget, allocation);

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding",    &focus_pad,
                        NULL);

  priv->size = MIN (allocation->width  - 2 * (focus_width + focus_pad),
                    allocation->height - 2 * (focus_width + focus_pad));

  priv->ring_width = priv->size * priv->ring_fraction;

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (priv->window,
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);
}


/* Utility functions */

#define INTENSITY(r, g, b) ((r) * 0.30 + (g) * 0.59 + (b) * 0.11)

/* Converts from HSV to RGB */
static void
hsv_to_rgb (gdouble *h,
            gdouble *s,
            gdouble *v)
{
  gdouble hue, saturation, value;
  gdouble f, p, q, t;

  if (*s == 0.0)
    {
      *h = *v;
      *s = *v;
      *v = *v; /* heh */
    }
  else
    {
      hue = *h * 6.0;
      saturation = *s;
      value = *v;

      if (hue == 6.0)
        hue = 0.0;

      f = hue - (int) hue;
      p = value * (1.0 - saturation);
      q = value * (1.0 - saturation * f);
      t = value * (1.0 - saturation * (1.0 - f));

      switch ((int) hue)
        {
        case 0:
          *h = value;
          *s = t;
          *v = p;
          break;

        case 1:
          *h = q;
          *s = value;
          *v = p;
          break;

        case 2:
          *h = p;
          *s = value;
          *v = t;
          break;

        case 3:
          *h = p;
          *s = q;
          *v = value;
          break;

        case 4:
          *h = t;
          *s = p;
          *v = value;
          break;

        case 5:
          *h = value;
          *s = p;
          *v = q;
          break;

        default:
          g_assert_not_reached ();
        }
    }
}

/* Computes the vertices of the saturation/value triangle */
static void
compute_triangle (PicmanColorWheel *wheel,
                  gint           *hx,
                  gint           *hy,
                  gint           *sx,
                  gint           *sy,
                  gint           *vx,
                  gint           *vy)
{
  PicmanColorWheelPrivate *priv = wheel->priv;
  GtkAllocation          allocation;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                inner, outer;
  gdouble                angle;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  inner = outer - priv->ring_width;
  angle = priv->h * 2.0 * G_PI;

  *hx = floor (center_x + cos (angle) * inner + 0.5);
  *hy = floor (center_y - sin (angle) * inner + 0.5);
  *sx = floor (center_x + cos (angle + 2.0 * G_PI / 3.0) * inner + 0.5);
  *sy = floor (center_y - sin (angle + 2.0 * G_PI / 3.0) * inner + 0.5);
  *vx = floor (center_x + cos (angle + 4.0 * G_PI / 3.0) * inner + 0.5);
  *vy = floor (center_y - sin (angle + 4.0 * G_PI / 3.0) * inner + 0.5);
}

/* Computes whether a point is inside the hue ring */
static gboolean
is_in_ring (PicmanColorWheel *wheel,
            gdouble         x,
            gdouble         y)
{
  PicmanColorWheelPrivate *priv = wheel->priv;
  GtkAllocation          allocation;
  gdouble                dx, dy, dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                inner, outer;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  inner = outer - priv->ring_width;

  dx = x - center_x;
  dy = center_y - y;
  dist = dx * dx + dy * dy;

  return (dist >= inner * inner && dist <= outer * outer);
}

/* Computes a saturation/value pair based on the mouse coordinates */
static void
compute_sv (PicmanColorWheel *wheel,
            gdouble         x,
            gdouble         y,
            gdouble        *s,
            gdouble        *v)
{
  GtkAllocation allocation;
  gint          ihx, ihy, isx, isy, ivx, ivy;
  gdouble       hx, hy, sx, sy, vx, vy;
  gdouble       center_x;
  gdouble       center_y;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  compute_triangle (wheel, &ihx, &ihy, &isx, &isy, &ivx, &ivy);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  hx = ihx - center_x;
  hy = center_y - ihy;
  sx = isx - center_x;
  sy = center_y - isy;
  vx = ivx - center_x;
  vy = center_y - ivy;
  x -= center_x;
  y = center_y - y;

  if (vx * (x - sx) + vy * (y - sy) < 0.0)
    {
      *s = 1.0;
      *v = (((x - sx) * (hx - sx) + (y - sy) * (hy-sy))
            / ((hx - sx) * (hx - sx) + (hy - sy) * (hy - sy)));

      if (*v < 0.0)
        *v = 0.0;
      else if (*v > 1.0)
        *v = 1.0;
    }
  else if (hx * (x - sx) + hy * (y - sy) < 0.0)
    {
      *s = 0.0;
      *v = (((x - sx) * (vx - sx) + (y - sy) * (vy - sy))
            / ((vx - sx) * (vx - sx) + (vy - sy) * (vy - sy)));

      if (*v < 0.0)
        *v = 0.0;
      else if (*v > 1.0)
        *v = 1.0;
    }
  else if (sx * (x - hx) + sy * (y - hy) < 0.0)
    {
      *v = 1.0;
      *s = (((x - vx) * (hx - vx) + (y - vy) * (hy - vy)) /
            ((hx - vx) * (hx - vx) + (hy - vy) * (hy - vy)));

      if (*s < 0.0)
        *s = 0.0;
      else if (*s > 1.0)
        *s = 1.0;
    }
  else
    {
      *v = (((x - sx) * (hy - vy) - (y - sy) * (hx - vx))
            / ((vx - sx) * (hy - vy) - (vy - sy) * (hx - vx)));

      if (*v<= 0.0)
        {
          *v = 0.0;
          *s = 0.0;
        }
      else
        {
          if (*v > 1.0)
            *v = 1.0;

          if (fabs (hy - vy) < fabs (hx - vx))
            *s = (x - sx - *v * (vx - sx)) / (*v * (hx - vx));
          else
            *s = (y - sy - *v * (vy - sy)) / (*v * (hy - vy));

          if (*s < 0.0)
            *s = 0.0;
          else if (*s > 1.0)
            *s = 1.0;
        }
    }
}

/* Computes whether a point is inside the saturation/value triangle */
static gboolean
is_in_triangle (PicmanColorWheel *wheel,
                gdouble         x,
                gdouble         y)
{
  gint    hx, hy, sx, sy, vx, vy;
  gdouble det, s, v;

  compute_triangle (wheel, &hx, &hy, &sx, &sy, &vx, &vy);

  det = (vx - sx) * (hy - sy) - (vy - sy) * (hx - sx);

  s = ((x - sx) * (hy - sy) - (y - sy) * (hx - sx)) / det;
  v = ((vx - sx) * (y - sy) - (vy - sy) * (x - sx)) / det;

  return (s >= 0.0 && v >= 0.0 && s + v <= 1.0);
}

/* Computes a value based on the mouse coordinates */
static double
compute_v (PicmanColorWheel *wheel,
           gdouble         x,
           gdouble         y)
{
  GtkAllocation allocation;
  gdouble       center_x;
  gdouble       center_y;
  gdouble       dx, dy;
  gdouble       angle;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  dx = x - center_x;
  dy = center_y - y;

  angle = atan2 (dy, dx);
  if (angle < 0.0)
    angle += 2.0 * G_PI;

  return angle / (2.0 * G_PI);
}

static void
set_cross_grab (PicmanColorWheel *wheel,
                guint32         time)
{
  PicmanColorWheelPrivate *priv = wheel->priv;
  GdkCursor             *cursor;

  cursor =
    gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (wheel)),
                                GDK_CROSSHAIR);

  gdk_pointer_grab (priv->window, FALSE,
                    GDK_POINTER_MOTION_MASK      |
                    GDK_POINTER_MOTION_HINT_MASK |
                    GDK_BUTTON_RELEASE_MASK,
                    NULL, cursor, time);
  gdk_cursor_unref (cursor);
}

static gboolean
picman_color_wheel_grab_broken (GtkWidget          *widget,
                              GdkEventGrabBroken *event)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;

  priv->mode = DRAG_NONE;

  return TRUE;
}

static gboolean
picman_color_wheel_button_press (GtkWidget      *widget,
                               GdkEventButton *event)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  gdouble                x, y;

  if (priv->mode != DRAG_NONE || event->button != 1)
    return FALSE;

  x = event->x;
  y = event->y;

  if (is_in_ring (wheel, x, y))
    {
      priv->mode = DRAG_H;
      set_cross_grab (wheel, event->time);

      picman_color_wheel_set_color (wheel,
                                  compute_v (wheel, x, y),
                                  priv->s,
                                  priv->v);

      gtk_widget_grab_focus (widget);
      priv->focus_on_ring = TRUE;

      return TRUE;
    }

  if (is_in_triangle (wheel, x, y))
    {
      gdouble s, v;

      priv->mode = DRAG_SV;
      set_cross_grab (wheel, event->time);

      compute_sv (wheel, x, y, &s, &v);
      picman_color_wheel_set_color (wheel, priv->h, s, v);

      gtk_widget_grab_focus (widget);
      priv->focus_on_ring = FALSE;

      return TRUE;
    }

  return FALSE;
}

static gboolean
picman_color_wheel_button_release (GtkWidget      *widget,
                                 GdkEventButton *event)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  DragMode               mode;
  gdouble                x, y;

  if (priv->mode == DRAG_NONE || event->button != 1)
    return FALSE;

  /* Set the drag mode to DRAG_NONE so that signal handlers for "catched"
   * can see that this is the final color state.
   */

  mode = priv->mode;
  priv->mode = DRAG_NONE;

  x = event->x;
  y = event->y;

  if (mode == DRAG_H)
    {
      picman_color_wheel_set_color (wheel,
                                  compute_v (wheel, x, y), priv->s, priv->v);
    }
  else if (mode == DRAG_SV)
    {
      gdouble s, v;

      compute_sv (wheel, x, y, &s, &v);
      picman_color_wheel_set_color (wheel, priv->h, s, v);
    }
  else
    g_assert_not_reached ();

  gdk_display_pointer_ungrab (gdk_window_get_display (event->window),
                              event->time);

  return TRUE;
}

static gboolean
picman_color_wheel_motion (GtkWidget      *widget,
                         GdkEventMotion *event)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  gdouble                x, y;

  if (priv->mode == DRAG_NONE)
    return FALSE;

  gdk_event_request_motions (event);
  x = event->x;
  y = event->y;

  if (priv->mode == DRAG_H)
    {
      picman_color_wheel_set_color (wheel,
                                  compute_v (wheel, x, y), priv->s, priv->v);
      return TRUE;
    }
  else if (priv->mode == DRAG_SV)
    {
      gdouble s, v;

      compute_sv (wheel, x, y, &s, &v);
      picman_color_wheel_set_color (wheel, priv->h, s, v);
      return TRUE;
    }

  g_assert_not_reached ();

  return FALSE;
}


/* Redrawing */

/* Paints the hue ring */
static void
paint_ring (PicmanColorWheel *wheel,
            cairo_t        *cr,
            gint            x,
            gint            y,
            gint            width,
            gint            height)
{
  GtkWidget             *widget = GTK_WIDGET (wheel);
  PicmanColorWheelPrivate *priv   = wheel->priv;
  GtkAllocation          allocation;
  gint                   xx, yy;
  gdouble                dx, dy, dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                inner, outer;
  guint32               *buf, *p;
  gdouble                angle;
  gdouble                hue;
  gdouble                r, g, b;
  cairo_surface_t       *source;
  cairo_t               *source_cr;
  gint                   stride;
  gint                   focus_width;
  gint                   focus_pad;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding", &focus_pad,
                        NULL);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  inner = outer - priv->ring_width;

  /* Create an image initialized with the ring colors */

  stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width);
  buf = g_new (guint32, height * stride / 4);

  for (yy = 0; yy < height; yy++)
    {
      p = buf + yy * width;

      dy = -(yy + y - center_y);

      for (xx = 0; xx < width; xx++)
        {
          dx = xx + x - center_x;

          dist = dx * dx + dy * dy;
          if (dist < ((inner-1) * (inner-1)) || dist > ((outer+1) * (outer+1)))
            {
              *p++ = 0;
              continue;
            }

          angle = atan2 (dy, dx);
          if (angle < 0.0)
            angle += 2.0 * G_PI;

          hue = angle / (2.0 * G_PI);

          r = hue;
          g = 1.0;
          b = 1.0;
          hsv_to_rgb (&r, &g, &b);

          *p++ = (((int)floor (r * 255 + 0.5) << 16) |
                  ((int)floor (g * 255 + 0.5) << 8) |
                  (int)floor (b * 255 + 0.5));
        }
    }

  source = cairo_image_surface_create_for_data ((unsigned char *)buf,
                                                CAIRO_FORMAT_RGB24,
                                                width, height, stride);

  /* Now draw the value marker onto the source image, so that it
   * will get properly clipped at the edges of the ring
   */
  source_cr = cairo_create (source);

  r = priv->h;
  g = 1.0;
  b = 1.0;
  hsv_to_rgb (&r, &g, &b);

  if (INTENSITY (r, g, b) > 0.5)
    cairo_set_source_rgb (source_cr, 0., 0., 0.);
  else
    cairo_set_source_rgb (source_cr, 1., 1., 1.);

  cairo_move_to (source_cr, -x + center_x, - y + center_y);
  cairo_line_to (source_cr,
                 -x + center_x + cos (priv->h * 2.0 * G_PI) * priv->size / 2,
                 -y + center_y - sin (priv->h * 2.0 * G_PI) * priv->size / 2);
  cairo_stroke (source_cr);
  cairo_destroy (source_cr);

  /* Draw the ring using the source image */

  cairo_save (cr);

  cairo_set_source_surface (cr, source, x, y);
  cairo_surface_destroy (source);

  cairo_set_line_width (cr, priv->ring_width);
  cairo_new_path (cr);
  cairo_arc (cr,
             center_x, center_y,
             priv->size / 2. - priv->ring_width / 2.,
             0, 2 * G_PI);
  cairo_stroke (cr);

  cairo_restore (cr);

  g_free (buf);
}

/* Converts an HSV triplet to an integer RGB triplet */
static void
get_color (gdouble  h,
           gdouble  s,
           gdouble  v,
           gint    *r,
           gint    *g,
           gint    *b)
{
  hsv_to_rgb (&h, &s, &v);

  *r = floor (h * 255 + 0.5);
  *g = floor (s * 255 + 0.5);
  *b = floor (v * 255 + 0.5);
}

#define SWAP(a, b, t) ((t) = (a), (a) = (b), (b) = (t))

#define LERP(a, b, v1, v2, i) (((v2) - (v1) != 0)                                       \
                               ? ((a) + ((b) - (a)) * ((i) - (v1)) / ((v2) - (v1)))     \
                               : (a))

/* Number of pixels we extend out from the edges when creating
 * color source to avoid artifacts
 */
#define PAD 3

/* Paints the HSV triangle */
static void
paint_triangle (PicmanColorWheel *wheel,
                cairo_t        *cr,
                gint            x,
                gint            y,
                gint            width,
                gint            height)
{
  GtkWidget             *widget = GTK_WIDGET (wheel);
  PicmanColorWheelPrivate *priv   = wheel->priv;
  gint                   hx, hy, sx, sy, vx, vy; /* HSV vertices */
  gint                   x1, y1, r1, g1, b1; /* First vertex in scanline order */
  gint                   x2, y2, r2, g2, b2; /* Second vertex */
  gint                   x3, y3, r3, g3, b3; /* Third vertex */
  gint                   t;
  guint32               *buf, *p, c;
  gint                   xl, xr, rl, rr, gl, gr, bl, br; /* Scanline data */
  gint                   xx, yy;
  gint                   x_interp, y_interp;
  gint                   x_start, x_end;
  cairo_surface_t       *source;
  gdouble                r, g, b;
  gchar                 *detail;
  gint                   stride;

  /* Compute triangle's vertices */

  compute_triangle (wheel, &hx, &hy, &sx, &sy, &vx, &vy);

  x1 = hx;
  y1 = hy;
  get_color (priv->h, 1.0, 1.0, &r1, &g1, &b1);

  x2 = sx;
  y2 = sy;
  get_color (priv->h, 1.0, 0.0, &r2, &g2, &b2);

  x3 = vx;
  y3 = vy;
  get_color (priv->h, 0.0, 1.0, &r3, &g3, &b3);

  if (y2 > y3)
    {
      SWAP (x2, x3, t);
      SWAP (y2, y3, t);
      SWAP (r2, r3, t);
      SWAP (g2, g3, t);
      SWAP (b2, b3, t);
    }

  if (y1 > y3)
    {
      SWAP (x1, x3, t);
      SWAP (y1, y3, t);
      SWAP (r1, r3, t);
      SWAP (g1, g3, t);
      SWAP (b1, b3, t);
    }

  if (y1 > y2)
    {
      SWAP (x1, x2, t);
      SWAP (y1, y2, t);
      SWAP (r1, r2, t);
      SWAP (g1, g2, t);
      SWAP (b1, b2, t);
    }

  /* Shade the triangle */

  stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width);
  buf = g_new (guint32, height * stride / 4);

  for (yy = 0; yy < height; yy++)
    {
      p = buf + yy * width;

      if (yy + y >= y1 - PAD && yy + y < y3 + PAD)
        {
          y_interp = CLAMP (yy + y, y1, y3);

          if (y_interp < y2)
            {
              xl = LERP (x1, x2, y1, y2, y_interp);

              rl = LERP (r1, r2, y1, y2, y_interp);
              gl = LERP (g1, g2, y1, y2, y_interp);
              bl = LERP (b1, b2, y1, y2, y_interp);
            }
          else
            {
              xl = LERP (x2, x3, y2, y3, y_interp);

              rl = LERP (r2, r3, y2, y3, y_interp);
              gl = LERP (g2, g3, y2, y3, y_interp);
              bl = LERP (b2, b3, y2, y3, y_interp);
            }

          xr = LERP (x1, x3, y1, y3, y_interp);

          rr = LERP (r1, r3, y1, y3, y_interp);
          gr = LERP (g1, g3, y1, y3, y_interp);
          br = LERP (b1, b3, y1, y3, y_interp);

          if (xl > xr)
            {
              SWAP (xl, xr, t);
              SWAP (rl, rr, t);
              SWAP (gl, gr, t);
              SWAP (bl, br, t);
            }

          x_start = MAX (xl - PAD, x);
          x_end = MIN (xr + PAD, x + width);
          x_start = MIN (x_start, x_end);

          c = (rl << 16) | (gl << 8) | bl;

          for (xx = x; xx < x_start; xx++)
            *p++ = c;

          for (; xx < x_end; xx++)
            {
              x_interp = CLAMP (xx, xl, xr);

              *p++ = ((LERP (rl, rr, xl, xr, x_interp) << 16) |
                      (LERP (gl, gr, xl, xr, x_interp) << 8) |
                      LERP (bl, br, xl, xr, x_interp));
            }

          c = (rr << 16) | (gr << 8) | br;

          for (; xx < x + width; xx++)
            *p++ = c;
        }
    }

  source = cairo_image_surface_create_for_data ((unsigned char *)buf,
                                                CAIRO_FORMAT_RGB24,
                                                width, height, stride);

  /* Draw a triangle with the image as a source */

  cairo_set_source_surface (cr, source, x, y);
  cairo_surface_destroy (source);

  cairo_move_to (cr, x1, y1);
  cairo_line_to (cr, x2, y2);
  cairo_line_to (cr, x3, y3);
  cairo_close_path (cr);
  cairo_fill (cr);

  g_free (buf);

  /* Draw value marker */

  xx = floor (sx + (vx - sx) * priv->v + (hx - vx) * priv->s * priv->v + 0.5);
  yy = floor (sy + (vy - sy) * priv->v + (hy - vy) * priv->s * priv->v + 0.5);

  r = priv->h;
  g = priv->s;
  b = priv->v;
  hsv_to_rgb (&r, &g, &b);

  if (INTENSITY (r, g, b) > 0.5)
    {
      detail = "colorwheel_light";
      cairo_set_source_rgb (cr, 0., 0., 0.);
    }
  else
    {
      detail = "colorwheel_dark";
      cairo_set_source_rgb (cr, 1., 1., 1.);
    }

#define RADIUS 4
#define FOCUS_RADIUS 6

  cairo_new_path (cr);
  cairo_arc (cr, xx, yy, RADIUS, 0, 2 * G_PI);
  cairo_stroke (cr);

  /* Draw focus outline */

  if (gtk_widget_has_focus (widget) &&
      ! priv->focus_on_ring)
    {
      GtkAllocation allocation;
      gint          focus_width;
      gint          focus_pad;

      gtk_widget_get_allocation (widget, &allocation);

      gtk_widget_style_get (widget,
                            "focus-line-width", &focus_width,
                            "focus-padding", &focus_pad,
                            NULL);

      gtk_paint_focus (gtk_widget_get_style (widget),
                       gtk_widget_get_window (widget),
                       gtk_widget_get_state (widget),
                       NULL, widget, detail,
                       allocation.x + xx - FOCUS_RADIUS - focus_width - focus_pad,
                       allocation.y + yy - FOCUS_RADIUS - focus_width - focus_pad,
                       2 * (FOCUS_RADIUS + focus_width + focus_pad),
                       2 * (FOCUS_RADIUS + focus_width + focus_pad));
    }
}

/* Paints the contents of the HSV color selector */
static void
paint (PicmanColorWheel *hsv,
       cairo_t        *cr,
       gint            x,
       gint            y,
       gint            width,
       gint            height)
{
  paint_ring (hsv, cr, x, y, width, height);
  paint_triangle (hsv, cr, x, y, width, height);
}

static gint
picman_color_wheel_expose (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;
  GtkAllocation          allocation;
  GdkRectangle           dest;
  cairo_t               *cr;

  if (! (event->window == gtk_widget_get_window (widget) &&
         gtk_widget_is_drawable (widget)))
    return FALSE;

  gtk_widget_get_allocation (widget, &allocation);

  if (!gdk_rectangle_intersect (&event->area, &allocation, &dest))
    return FALSE;

  cr = gdk_cairo_create (gtk_widget_get_window (widget));

  cairo_translate (cr, allocation.x, allocation.y);
  paint (wheel, cr,
         dest.x - allocation.x,
         dest.y - allocation.y,
         dest.width, dest.height);
  cairo_destroy (cr);

  if (gtk_widget_has_focus (widget) && priv->focus_on_ring)
    gtk_paint_focus (gtk_widget_get_style (widget),
                     gtk_widget_get_window (widget),
                     gtk_widget_get_state (widget),
                     &event->area, widget, NULL,
                     allocation.x,
                     allocation.y,
                     allocation.width,
                     allocation.height);

  return FALSE;
}

static gboolean
picman_color_wheel_focus (GtkWidget        *widget,
                        GtkDirectionType  dir)
{
  PicmanColorWheel        *wheel = PICMAN_COLOR_WHEEL (widget);
  PicmanColorWheelPrivate *priv  = wheel->priv;

  if (!gtk_widget_has_focus (widget))
    {
      if (dir == GTK_DIR_TAB_BACKWARD)
        priv->focus_on_ring = FALSE;
      else
        priv->focus_on_ring = TRUE;

      gtk_widget_grab_focus (widget);
      return TRUE;
    }

  switch (dir)
    {
    case GTK_DIR_UP:
      if (priv->focus_on_ring)
        return FALSE;
      else
        priv->focus_on_ring = TRUE;
      break;

    case GTK_DIR_DOWN:
      if (priv->focus_on_ring)
        priv->focus_on_ring = FALSE;
      else
        return FALSE;
      break;

    case GTK_DIR_LEFT:
    case GTK_DIR_TAB_BACKWARD:
      if (priv->focus_on_ring)
        return FALSE;
      else
        priv->focus_on_ring = TRUE;
      break;

    case GTK_DIR_RIGHT:
    case GTK_DIR_TAB_FORWARD:
      if (priv->focus_on_ring)
        priv->focus_on_ring = FALSE;
      else
        return FALSE;
      break;
    }

  gtk_widget_queue_draw (widget);

  return TRUE;
}

/**
 * picman_color_wheel_new:
 *
 * Creates a new HSV color selector.
 *
 * Return value: A newly-created HSV color selector.
 *
 * Since: 2.14
 */
GtkWidget*
picman_color_wheel_new (void)
{
  return g_object_new (PICMAN_TYPE_COLOR_WHEEL, NULL);
}

/**
 * picman_color_wheel_set_color:
 * @hsv: An HSV color selector
 * @h: Hue
 * @s: Saturation
 * @v: Value
 *
 * Sets the current color in an HSV color selector.
 * Color component values must be in the [0.0, 1.0] range.
 *
 * Since: 2.14
 */
void
picman_color_wheel_set_color (PicmanColorWheel *wheel,
                            gdouble         h,
                            gdouble         s,
                            gdouble         v)
{
  PicmanColorWheelPrivate *priv;

  g_return_if_fail (PICMAN_IS_COLOR_WHEEL (wheel));
  g_return_if_fail (h >= 0.0 && h <= 1.0);
  g_return_if_fail (s >= 0.0 && s <= 1.0);
  g_return_if_fail (v >= 0.0 && v <= 1.0);

  priv = wheel->priv;

  priv->h = h;
  priv->s = s;
  priv->v = v;

  g_signal_emit (wheel, wheel_signals[CHANGED], 0);

  gtk_widget_queue_draw (GTK_WIDGET (wheel));
}

/**
 * picman_color_wheel_get_color:
 * @hsv: An HSV color selector
 * @h: (out): Return value for the hue
 * @s: (out): Return value for the saturation
 * @v: (out): Return value for the value
 *
 * Queries the current color in an HSV color selector.
 * Returned values will be in the [0.0, 1.0] range.
 *
 * Since: 2.14
 */
void
picman_color_wheel_get_color (PicmanColorWheel *wheel,
                            gdouble        *h,
                            gdouble        *s,
                            gdouble        *v)
{
  PicmanColorWheelPrivate *priv;

  g_return_if_fail (PICMAN_IS_COLOR_WHEEL (wheel));

  priv = wheel->priv;

  if (h) *h = priv->h;
  if (s) *s = priv->s;
  if (v) *v = priv->v;
}

/**
 * picman_color_wheel_set_ring_fraction:
 * @ring: A wheel color selector
 * @fraction: Ring fraction
 *
 * Sets the ring fraction of a wheel color selector.
 *
 * Since: PICMAN 2.10
 */
void
picman_color_wheel_set_ring_fraction (PicmanColorWheel *hsv,
                                    gdouble         fraction)
{
  PicmanColorWheelPrivate *priv;

  g_return_if_fail (PICMAN_IS_COLOR_WHEEL (hsv));

  priv = hsv->priv;

  priv->ring_fraction = CLAMP (fraction, 0.01, 0.99);

  gtk_widget_queue_draw (GTK_WIDGET (hsv));
}

/**
 * picman_color_wheel_get_ring_fraction:
 * @ring: A wheel color selector
 *
 * Returns value: The ring fraction of the wheel color selector.
 *
 * Since: PICMAN 2.10
 */
gdouble
picman_color_wheel_get_ring_fraction (PicmanColorWheel *wheel)
{
  PicmanColorWheelPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_COLOR_WHEEL (wheel), DEFAULT_FRACTION);

  priv = wheel->priv;

  return priv->ring_fraction;
}

/**
 * picman_color_wheel_is_adjusting:
 * @hsv: A #PicmanColorWheel
 *
 * An HSV color selector can be said to be adjusting if multiple rapid
 * changes are being made to its value, for example, when the user is
 * adjusting the value with the mouse. This function queries whether
 * the HSV color selector is being adjusted or not.
 *
 * Return value: %TRUE if clients can ignore changes to the color value,
 *     since they may be transitory, or %FALSE if they should consider
 *     the color value status to be final.
 *
 * Since: 2.14
 */
gboolean
picman_color_wheel_is_adjusting (PicmanColorWheel *wheel)
{
  PicmanColorWheelPrivate *priv;

  g_return_val_if_fail (PICMAN_IS_COLOR_WHEEL (wheel), FALSE);

  priv = wheel->priv;

  return priv->mode != DRAG_NONE;
}

static void
picman_color_wheel_move (PicmanColorWheel   *wheel,
                       GtkDirectionType  dir)
{
  PicmanColorWheelPrivate *priv = wheel->priv;
  gdouble                hue, sat, val;
  gint                   hx, hy, sx, sy, vx, vy; /* HSV vertices */
  gint                   x, y; /* position in triangle */

  hue = priv->h;
  sat = priv->s;
  val = priv->v;

  compute_triangle (wheel, &hx, &hy, &sx, &sy, &vx, &vy);

  x = floor (sx + (vx - sx) * priv->v + (hx - vx) * priv->s * priv->v + 0.5);
  y = floor (sy + (vy - sy) * priv->v + (hy - vy) * priv->s * priv->v + 0.5);

#define HUE_DELTA 0.002
  switch (dir)
    {
    case GTK_DIR_UP:
      if (priv->focus_on_ring)
        hue += HUE_DELTA;
      else
        {
          y -= 1;
          compute_sv (wheel, x, y, &sat, &val);
        }
      break;

    case GTK_DIR_DOWN:
      if (priv->focus_on_ring)
        hue -= HUE_DELTA;
      else
        {
          y += 1;
          compute_sv (wheel, x, y, &sat, &val);
        }
      break;

    case GTK_DIR_LEFT:
      if (priv->focus_on_ring)
        hue += HUE_DELTA;
      else
        {
          x -= 1;
          compute_sv (wheel, x, y, &sat, &val);
        }
      break;

    case GTK_DIR_RIGHT:
      if (priv->focus_on_ring)
        hue -= HUE_DELTA
          ;
      else
        {
          x += 1;
          compute_sv (wheel, x, y, &sat, &val);
        }
      break;

    default:
      /* we don't care about the tab directions */
      break;
    }

  /* Wrap */
  if (hue < 0.0)
    hue = 1.0;
  else if (hue > 1.0)
    hue = 0.0;

  picman_color_wheel_set_color (wheel, hue, sat, val);
}
