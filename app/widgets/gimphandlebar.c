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

#include <gtk/gtk.h>

#include "libpicmanmath/picmanmath.h"

#include "widgets-types.h"

#include "picmanhandlebar.h"


enum
{
  PROP_0,
  PROP_ORIENTATION
};


/*  local function prototypes  */

static void      picman_handle_bar_set_property       (GObject        *object,
                                                     guint           property_id,
                                                     const GValue   *value,
                                                     GParamSpec     *pspec);
static void      picman_handle_bar_get_property       (GObject        *object,
                                                     guint           property_id,
                                                     GValue         *value,
                                                     GParamSpec     *pspec);

static gboolean  picman_handle_bar_expose             (GtkWidget      *widget,
                                                     GdkEventExpose *eevent);
static gboolean  picman_handle_bar_button_press       (GtkWidget      *widget,
                                                     GdkEventButton *bevent);
static gboolean  picman_handle_bar_button_release     (GtkWidget      *widget,
                                                     GdkEventButton *bevent);
static gboolean  picman_handle_bar_motion_notify      (GtkWidget      *widget,
                                                     GdkEventMotion *mevent);

static void      picman_handle_bar_adjustment_changed (GtkAdjustment  *adjustment,
                                                     PicmanHandleBar  *bar);


G_DEFINE_TYPE (PicmanHandleBar, picman_handle_bar, GTK_TYPE_EVENT_BOX)

#define parent_class picman_handle_bar_parent_class


static void
picman_handle_bar_class_init (PicmanHandleBarClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property         = picman_handle_bar_set_property;
  object_class->get_property         = picman_handle_bar_get_property;

  widget_class->expose_event         = picman_handle_bar_expose;
  widget_class->button_press_event   = picman_handle_bar_button_press;
  widget_class->button_release_event = picman_handle_bar_button_release;
  widget_class->motion_notify_event  = picman_handle_bar_motion_notify;

  g_object_class_install_property (object_class, PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation",
                                                      NULL, NULL,
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_handle_bar_init (PicmanHandleBar *bar)
{
  gtk_widget_add_events (GTK_WIDGET (bar),
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_BUTTON_MOTION_MASK);

  gtk_event_box_set_visible_window (GTK_EVENT_BOX (bar), FALSE);

  bar->orientation = GTK_ORIENTATION_HORIZONTAL;
  bar->lower       = 0.0;
  bar->upper       = 1.0;
}

static void
picman_handle_bar_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanHandleBar *bar = PICMAN_HANDLE_BAR (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      bar->orientation = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_handle_bar_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanHandleBar *bar = PICMAN_HANDLE_BAR (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, bar->orientation);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_handle_bar_expose (GtkWidget      *widget,
                        GdkEventExpose *eevent)
{
  PicmanHandleBar *bar = PICMAN_HANDLE_BAR (widget);
  GtkAllocation  allocation;
  cairo_t       *cr;
  gint           x, y;
  gint           width, height;
  gint           i;

  gtk_widget_get_allocation (widget, &allocation);

  x = y = gtk_container_get_border_width (GTK_CONTAINER (widget));

  width  = allocation.width  - 2 * x;
  height = allocation.height - 2 * y;

  if (! gtk_widget_get_has_window (widget))
    {
      x += allocation.x;
      y += allocation.y;
    }

  cr = gdk_cairo_create (gtk_widget_get_window (widget));

  gdk_cairo_region (cr, eevent->region);
  cairo_clip (cr);

  cairo_set_line_width (cr, 1.0);
  cairo_translate (cr, 0.5, 0.5);

  for (i = 0; i < 3; i++)
    {
      bar->slider_pos[i] = -1;

      if (bar->slider_adj[i])
        {
          bar->slider_pos[i] = ROUND ((gdouble) width *
                                      (gtk_adjustment_get_value (bar->slider_adj[i]) - bar->lower) /
                                      (bar->upper - bar->lower + 1));

          cairo_set_source_rgb (cr, 0.5 * i, 0.5 * i, 0.5 * i);

          cairo_move_to (cr,
                         x + bar->slider_pos[i],
                         y);
          cairo_line_to (cr,
                         x + bar->slider_pos[i] - (height - 1) / 2,
                         y + height - 1);
          cairo_line_to (cr,
                         x + bar->slider_pos[i] + (height - 1) / 2,
                         y + height - 1);
          cairo_line_to (cr,
                         x + bar->slider_pos[i],
                         y);

          cairo_fill_preserve (cr);

          cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);

          cairo_stroke (cr);
        }
    }

  cairo_destroy (cr);

  return FALSE;
}

static gboolean
picman_handle_bar_button_press (GtkWidget      *widget,
                              GdkEventButton *bevent)
{
  PicmanHandleBar *bar= PICMAN_HANDLE_BAR (widget);
  GtkAllocation  allocation;
  gint           border;
  gint           width;
  gdouble        value;
  gint           min_dist;
  gint           i;

  gtk_widget_get_allocation (widget, &allocation);

  border = gtk_container_get_border_width (GTK_CONTAINER (widget));
  width  = allocation.width - 2 * border;

  if (width < 1)
    return FALSE;

  min_dist = G_MAXINT;
  for (i = 0; i < 3; i++)
    if (bar->slider_pos[i] != -1)
      {
        gdouble dist = bevent->x - bar->slider_pos[i] + border;

        if (fabs (dist) < min_dist ||
            (fabs (dist) == min_dist && dist > 0))
          {
            bar->active_slider = i;
            min_dist = fabs (dist);
          }
      }

  value = ((gdouble) (bevent->x - border) /
           (gdouble) width *
           (bar->upper - bar->lower + 1));

  gtk_adjustment_set_value (bar->slider_adj[bar->active_slider], value);

  return TRUE;
}

static gboolean
picman_handle_bar_button_release (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  return TRUE;
}

static gboolean
picman_handle_bar_motion_notify (GtkWidget      *widget,
                               GdkEventMotion *mevent)
{
  PicmanHandleBar *bar    = PICMAN_HANDLE_BAR (widget);
  GtkAllocation  allocation;
  gint           border;
  gint           width;
  gdouble        value;

  gtk_widget_get_allocation (widget, &allocation);

  border = gtk_container_get_border_width (GTK_CONTAINER (widget));
  width  = allocation.width - 2 * border;

  if (width < 1)
    return FALSE;

  value = ((gdouble) (mevent->x - border) /
           (gdouble) width *
           (bar->upper - bar->lower + 1));

  gtk_adjustment_set_value (bar->slider_adj[bar->active_slider], value);

  return TRUE;
}


/*  public functions  */

/**
 * picman_handle_bar_new:
 * @orientation: whether the bar should be oriented horizontally or
 *               vertically
 *
 * Creates a new #PicmanHandleBar widget.
 *
 * Return value: The new #PicmanHandleBar widget.
 **/
GtkWidget *
picman_handle_bar_new (GtkOrientation  orientation)
{
  return g_object_new (PICMAN_TYPE_HANDLE_BAR,
                       "orientation", orientation,
                       NULL);
}

void
picman_handle_bar_set_adjustment (PicmanHandleBar  *bar,
                                gint            handle_no,
                                GtkAdjustment  *adjustment)
{
  g_return_if_fail (PICMAN_IS_HANDLE_BAR (bar));
  g_return_if_fail (handle_no >= 0 && handle_no <= 2);
  g_return_if_fail (adjustment == NULL || GTK_IS_ADJUSTMENT (adjustment));

  if (adjustment == bar->slider_adj[handle_no])
    return;

  if (bar->slider_adj[handle_no])
    {
      g_signal_handlers_disconnect_by_func (bar->slider_adj[handle_no],
                                            picman_handle_bar_adjustment_changed,
                                            bar);
      g_object_unref (bar->slider_adj[handle_no]);
      bar->slider_adj[handle_no] = NULL;
    }

  bar->slider_adj[handle_no] = adjustment;

  if (bar->slider_adj[handle_no])
    {
      g_object_ref (bar->slider_adj[handle_no]);

      g_signal_connect (bar->slider_adj[handle_no], "value-changed",
                        G_CALLBACK (picman_handle_bar_adjustment_changed),
                        bar);
      g_signal_connect (bar->slider_adj[handle_no], "changed",
                        G_CALLBACK (picman_handle_bar_adjustment_changed),
                        bar);
    }

  picman_handle_bar_adjustment_changed (bar->slider_adj[handle_no], bar);
}


/*  private functions  */

static void
picman_handle_bar_adjustment_changed (GtkAdjustment *adjustment,
                                    PicmanHandleBar *bar)
{
  if (bar->slider_adj[0])
    bar->lower = gtk_adjustment_get_lower (bar->slider_adj[0]);

  if (bar->slider_adj[2])
    bar->upper = gtk_adjustment_get_upper (bar->slider_adj[2]);

  gtk_widget_queue_draw (GTK_WIDGET (bar));
}
