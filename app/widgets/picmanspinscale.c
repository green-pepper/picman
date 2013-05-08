/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanspinscale.c
 * Copyright (C) 2010 Michael Natterer <mitch@picman.org>
 *               2012 Øyvind Kolås    <pippin@picman.org>
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

#include "libpicmanwidgets/picmanwidgets.h"
#include "libpicmanmath/picmanmath.h"

#include "widgets-types.h"

#include "picmanspinscale.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_LABEL
};

typedef enum
{
  TARGET_NUMBER,
  TARGET_UPPER,
  TARGET_LOWER
} SpinScaleTarget;


typedef struct _PicmanSpinScalePrivate PicmanSpinScalePrivate;

struct _PicmanSpinScalePrivate
{
  gchar       *label;

  gboolean     scale_limits_set;
  gdouble      scale_lower;
  gdouble      scale_upper;
  gdouble      gamma;

  PangoLayout *layout;
  gboolean     changing_value;
  gboolean     relative_change;
  gdouble      start_x;
  gdouble      start_value;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
                                                       PICMAN_TYPE_SPIN_SCALE, \
                                                       PicmanSpinScalePrivate))


static void       picman_spin_scale_dispose        (GObject          *object);
static void       picman_spin_scale_finalize       (GObject          *object);
static void       picman_spin_scale_set_property   (GObject          *object,
                                                  guint             property_id,
                                                  const GValue     *value,
                                                  GParamSpec       *pspec);
static void       picman_spin_scale_get_property   (GObject          *object,
                                                  guint             property_id,
                                                  GValue           *value,
                                                  GParamSpec       *pspec);

static void       picman_spin_scale_size_request   (GtkWidget        *widget,
                                                  GtkRequisition   *requisition);
static void       picman_spin_scale_style_set      (GtkWidget        *widget,
                                                  GtkStyle         *prev_style);
static gboolean   picman_spin_scale_expose         (GtkWidget        *widget,
                                                  GdkEventExpose   *event);
static gboolean   picman_spin_scale_button_press   (GtkWidget        *widget,
                                                  GdkEventButton   *event);
static gboolean   picman_spin_scale_button_release (GtkWidget        *widget,
                                                  GdkEventButton   *event);
static gboolean   picman_spin_scale_motion_notify  (GtkWidget        *widget,
                                                  GdkEventMotion   *event);
static gboolean   picman_spin_scale_leave_notify   (GtkWidget        *widget,
                                                  GdkEventCrossing *event);

static void       picman_spin_scale_value_changed  (GtkSpinButton    *spin_button);


G_DEFINE_TYPE (PicmanSpinScale, picman_spin_scale, GTK_TYPE_SPIN_BUTTON);

#define parent_class picman_spin_scale_parent_class


static void
picman_spin_scale_class_init (PicmanSpinScaleClass *klass)
{
  GObjectClass       *object_class      = G_OBJECT_CLASS (klass);
  GtkWidgetClass     *widget_class      = GTK_WIDGET_CLASS (klass);
  GtkSpinButtonClass *spin_button_class = GTK_SPIN_BUTTON_CLASS (klass);

  object_class->dispose              = picman_spin_scale_dispose;
  object_class->finalize             = picman_spin_scale_finalize;
  object_class->set_property         = picman_spin_scale_set_property;
  object_class->get_property         = picman_spin_scale_get_property;

  widget_class->size_request         = picman_spin_scale_size_request;
  widget_class->style_set            = picman_spin_scale_style_set;
  widget_class->expose_event         = picman_spin_scale_expose;
  widget_class->button_press_event   = picman_spin_scale_button_press;
  widget_class->button_release_event = picman_spin_scale_button_release;
  widget_class->motion_notify_event  = picman_spin_scale_motion_notify;
  widget_class->leave_notify_event   = picman_spin_scale_leave_notify;

  spin_button_class->value_changed   = picman_spin_scale_value_changed;

  g_object_class_install_property (object_class, PROP_LABEL,
                                   g_param_spec_string ("label", NULL, NULL,
                                                        NULL,
                                                        PICMAN_PARAM_READWRITE));

  g_type_class_add_private (klass, sizeof (PicmanSpinScalePrivate));
}

static void
picman_spin_scale_init (PicmanSpinScale *scale)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (scale);

  gtk_widget_add_events (GTK_WIDGET (scale),
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON1_MOTION_MASK |
                         GDK_LEAVE_NOTIFY_MASK);

  gtk_entry_set_alignment (GTK_ENTRY (scale), 1.0);
  gtk_entry_set_has_frame (GTK_ENTRY (scale), FALSE);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (scale), TRUE);

  private->gamma = 1.0;
}

static void
picman_spin_scale_dispose (GObject *object)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (object);

  if (private->layout)
    {
      g_object_unref (private->layout);
      private->layout = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_spin_scale_finalize (GObject *object)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (object);

  if (private->label)
    {
      g_free (private->label);
      private->label = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_spin_scale_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_LABEL:
      g_free (private->label);
      private->label = g_value_dup_string (value);
      if (private->layout)
        {
          g_object_unref (private->layout);
          private->layout = NULL;
        }
      gtk_widget_queue_resize (GTK_WIDGET (object));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_spin_scale_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (object);

  switch (property_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, private->label);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_spin_scale_size_request (GtkWidget      *widget,
                              GtkRequisition *requisition)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);
  GtkStyle             *style   = gtk_widget_get_style (widget);
  PangoContext         *context = gtk_widget_get_pango_context (widget);
  PangoFontMetrics     *metrics;
  gint                  height;

  GTK_WIDGET_CLASS (parent_class)->size_request (widget, requisition);

  metrics = pango_context_get_metrics (context, style->font_desc,
                                       pango_context_get_language (context));

  height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
                         pango_font_metrics_get_descent (metrics));

  requisition->height += height;

  if (private->label)
    {
      gint char_width;
      gint digit_width;
      gint char_pixels;

      char_width = pango_font_metrics_get_approximate_char_width (metrics);
      digit_width = pango_font_metrics_get_approximate_digit_width (metrics);
      char_pixels = PANGO_PIXELS (MAX (char_width, digit_width));

      /* ~3 chars for the ellipses */
      requisition->width += char_pixels * 3;
    }

  pango_font_metrics_unref (metrics);
}

static void
picman_spin_scale_style_set (GtkWidget *widget,
                           GtkStyle  *prev_style)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (private->layout)
    {
      g_object_unref (private->layout);
      private->layout = NULL;
    }
}

static gboolean
picman_spin_scale_expose (GtkWidget      *widget,
                        GdkEventExpose *event)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);
  GtkStyle             *style   = gtk_widget_get_style (widget);
  cairo_t              *cr;
  gboolean              rtl;
  gint                  w, h;

  GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);

  cr = gdk_cairo_create (event->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  w = gdk_window_get_width (event->window);
  h = gdk_window_get_height (event->window);

  cairo_set_line_width (cr, 1.0);

  if (event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      /* let spinbutton-side line of rectangle disappear */
      if (rtl)
        cairo_rectangle (cr, -0.5, 0.5, w, h - 1.0);
      else
        cairo_rectangle (cr, 0.5, 0.5, w, h - 1.0);

      gdk_cairo_set_source_color (cr,
                                  &style->text[gtk_widget_get_state (widget)]);
      cairo_stroke (cr);
    }
  else
    {
      /* let text-box-side line of rectangle disappear */
      if (rtl)
        cairo_rectangle (cr, 0.5, 0.5, w, h - 1.0);
      else
        cairo_rectangle (cr, -0.5, 0.5, w, h - 1.0);

      gdk_cairo_set_source_color (cr,
                                  &style->text[gtk_widget_get_state (widget)]);
      cairo_stroke (cr);

      if (rtl)
        cairo_rectangle (cr, 1.5, 1.5, w - 2.0, h - 3.0);
      else
        cairo_rectangle (cr, 0.5, 1.5, w - 2.0, h - 3.0);

      gdk_cairo_set_source_color (cr,
                                  &style->base[gtk_widget_get_state (widget)]);
      cairo_stroke (cr);
    }

  if (private->label &&
      gtk_widget_is_drawable (widget) &&
      event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      GtkRequisition requisition;
      GtkAllocation  allocation;
      PangoRectangle logical;
      gint           layout_offset_x;
      gint           layout_offset_y;

      GTK_WIDGET_CLASS (parent_class)->size_request (widget, &requisition);
      gtk_widget_get_allocation (widget, &allocation);

      if (! private->layout)
        {
          private->layout = gtk_widget_create_pango_layout (widget,
                                                            private->label);
          pango_layout_set_ellipsize (private->layout, PANGO_ELLIPSIZE_END);
        }

      pango_layout_set_width (private->layout,
                              PANGO_SCALE *
                              (allocation.width - requisition.width));
      pango_layout_get_pixel_extents (private->layout, NULL, &logical);

      gtk_entry_get_layout_offsets (GTK_ENTRY (widget), NULL, &layout_offset_y);

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        layout_offset_x = w - logical.width - 2;
      else
        layout_offset_x = 2;

      layout_offset_x -= logical.x;

      cairo_move_to (cr, layout_offset_x, layout_offset_y);

      gdk_cairo_set_source_color (cr,
                                  &style->text[gtk_widget_get_state (widget)]);

      pango_cairo_show_layout (cr, private->layout);
    }

  cairo_destroy (cr);

  return FALSE;
}

static SpinScaleTarget
picman_spin_scale_get_target (GtkWidget *widget,
                            gdouble    x,
                            gdouble    y)
{
  GtkAllocation   allocation;
  PangoRectangle  logical;
  gint            layout_x;
  gint            layout_y;

  gtk_widget_get_allocation (widget, &allocation);
  gtk_entry_get_layout_offsets (GTK_ENTRY (widget), &layout_x, &layout_y);
  pango_layout_get_pixel_extents (gtk_entry_get_layout (GTK_ENTRY (widget)),
                                  NULL, &logical);

  if (x > layout_x && x < layout_x + logical.width &&
      y > layout_y && y < layout_y + logical.height)
    {
      return TARGET_NUMBER;
    }
  else if (y > allocation.height / 2)
    {
      return TARGET_LOWER;
    }

  return TARGET_UPPER;
}

static void
picman_spin_scale_get_limits (PicmanSpinScale *scale,
                            gdouble       *lower,
                            gdouble       *upper)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (scale);

  if (private->scale_limits_set)
    {
      *lower = private->scale_lower;
      *upper = private->scale_upper;
    }
  else
    {
      GtkSpinButton *spin_button = GTK_SPIN_BUTTON (scale);
      GtkAdjustment *adjustment  = gtk_spin_button_get_adjustment (spin_button);

      *lower = gtk_adjustment_get_lower (adjustment);
      *upper = gtk_adjustment_get_upper (adjustment);
    }
}

static void
picman_spin_scale_change_value (GtkWidget *widget,
                              gdouble    x)
{
  PicmanSpinScalePrivate *private     = GET_PRIVATE (widget);
  GtkSpinButton        *spin_button = GTK_SPIN_BUTTON (widget);
  GtkAdjustment        *adjustment  = gtk_spin_button_get_adjustment (spin_button);
  GdkWindow            *text_window = gtk_entry_get_text_window (GTK_ENTRY (widget));
  gdouble               lower;
  gdouble               upper;
  gint                  width;
  gdouble               value;

  picman_spin_scale_get_limits (PICMAN_SPIN_SCALE (widget), &lower, &upper);

  width = gdk_window_get_width (text_window);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    x = width - x;

  if (private->relative_change)
    {
      gdouble diff;
      gdouble step;

      step = (upper - lower) / width / 10.0;

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        diff = x - (width - private->start_x);
      else
        diff = x - private->start_x;

      value = (private->start_value + diff * step);
    }
  else
    {
      gdouble fraction;

      fraction = x / (gdouble) width;
      if (fraction > 0.0)
        fraction = pow (fraction, private->gamma);

      value = fraction * (upper - lower) + lower;
    }

  gtk_adjustment_set_value (adjustment, value);
}

static gboolean
picman_spin_scale_button_press (GtkWidget      *widget,
                              GdkEventButton *event)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);

  private->changing_value  = FALSE;
  private->relative_change = FALSE;

  if (event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      switch (picman_spin_scale_get_target (widget, event->x, event->y))
        {
        case TARGET_UPPER:
          private->changing_value = TRUE;

          gtk_widget_grab_focus (widget);

          picman_spin_scale_change_value (widget, event->x);

          return TRUE;

        case TARGET_LOWER:
          private->changing_value = TRUE;

          gtk_widget_grab_focus (widget);

          private->relative_change = TRUE;
          private->start_x = event->x;
          private->start_value = gtk_adjustment_get_value (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));

          return TRUE;

        default:
          break;
        }
    }

  return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, event);
}

static gboolean
picman_spin_scale_button_release (GtkWidget      *widget,
                                GdkEventButton *event)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);

  if (private->changing_value)
    {
      private->changing_value = FALSE;

      picman_spin_scale_change_value (widget, event->x);

      return TRUE;
    }

  return GTK_WIDGET_CLASS (parent_class)->button_release_event (widget, event);
}

static gboolean
picman_spin_scale_motion_notify (GtkWidget      *widget,
                               GdkEventMotion *event)
{
  PicmanSpinScalePrivate *private = GET_PRIVATE (widget);

  gdk_event_request_motions (event);

  if (private->changing_value)
    {
      picman_spin_scale_change_value (widget, event->x);

      return TRUE;
    }

  GTK_WIDGET_CLASS (parent_class)->motion_notify_event (widget, event);

  if (! (event->state &
         (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK)) &&
      event->window == gtk_entry_get_text_window (GTK_ENTRY (widget)))
    {
      GdkDisplay *display = gtk_widget_get_display (widget);
      GdkCursor  *cursor  = NULL;

      switch (picman_spin_scale_get_target (widget, event->x, event->y))
        {
        case TARGET_NUMBER:
          cursor = gdk_cursor_new_for_display (display, GDK_XTERM);
          break;

        case TARGET_UPPER:
          cursor = gdk_cursor_new_for_display (display, GDK_SB_UP_ARROW);
          break;

        case TARGET_LOWER:
          cursor = gdk_cursor_new_for_display (display, GDK_SB_H_DOUBLE_ARROW);
          break;
        }

      gdk_window_set_cursor (event->window, cursor);
      gdk_cursor_unref (cursor);
    }

  return FALSE;
}

static gboolean
picman_spin_scale_leave_notify (GtkWidget        *widget,
                              GdkEventCrossing *event)
{
  gdk_window_set_cursor (event->window, NULL);

  return GTK_WIDGET_CLASS (parent_class)->leave_notify_event (widget, event);
}

static void
picman_spin_scale_value_changed (GtkSpinButton *spin_button)
{
  GtkAdjustment *adjustment = gtk_spin_button_get_adjustment (spin_button);
  PicmanSpinScalePrivate *private = GET_PRIVATE (spin_button);
  gdouble        lower;
  gdouble        upper;
  gdouble        value;

  picman_spin_scale_get_limits (PICMAN_SPIN_SCALE (spin_button), &lower, &upper);

  value = CLAMP (gtk_adjustment_get_value (adjustment), lower, upper);


  gtk_entry_set_progress_fraction (GTK_ENTRY (spin_button),
                                   pow ((value - lower) / (upper - lower),
                                        1.0 / private->gamma));
}


/*  public functions  */

GtkWidget *
picman_spin_scale_new (GtkAdjustment *adjustment,
                     const gchar   *label,
                     gint           digits)
{
  g_return_val_if_fail (GTK_IS_ADJUSTMENT (adjustment), NULL);

  return g_object_new (PICMAN_TYPE_SPIN_SCALE,
                       "adjustment", adjustment,
                       "label",      label,
                       "digits",     digits,
                       NULL);
}

void
picman_spin_scale_set_scale_limits (PicmanSpinScale *scale,
                                  gdouble        lower,
                                  gdouble        upper)
{
  PicmanSpinScalePrivate *private;
  GtkSpinButton        *spin_button;
  GtkAdjustment        *adjustment;

  g_return_if_fail (PICMAN_IS_SPIN_SCALE (scale));

  private     = GET_PRIVATE (scale);
  spin_button = GTK_SPIN_BUTTON (scale);
  adjustment  = gtk_spin_button_get_adjustment (spin_button);

  g_return_if_fail (lower >= gtk_adjustment_get_lower (adjustment));
  g_return_if_fail (upper <= gtk_adjustment_get_upper (adjustment));

  private->scale_limits_set = TRUE;
  private->scale_lower      = lower;
  private->scale_upper      = upper;
  private->gamma            = 1.0;

  picman_spin_scale_value_changed (spin_button);
}

void
picman_spin_scale_set_gamma (PicmanSpinScale *scale,
                           gdouble        gamma)
{
  PicmanSpinScalePrivate *private;

  g_return_if_fail (PICMAN_IS_SPIN_SCALE (scale));

  private = GET_PRIVATE (scale);

  private->gamma = gamma;

  picman_spin_scale_value_changed (GTK_SPIN_BUTTON (scale));
}

gdouble
picman_spin_scale_get_gamma (PicmanSpinScale *scale)
{
  PicmanSpinScalePrivate *private;

  g_return_val_if_fail (PICMAN_IS_SPIN_SCALE (scale), 1.0);

  private = GET_PRIVATE (scale);

  return private->gamma;
}

void
picman_spin_scale_unset_scale_limits (PicmanSpinScale *scale)
{
  PicmanSpinScalePrivate *private;

  g_return_if_fail (PICMAN_IS_SPIN_SCALE (scale));

  private = GET_PRIVATE (scale);

  private->scale_limits_set = FALSE;
  private->scale_lower      = 0.0;
  private->scale_upper      = 0.0;

  picman_spin_scale_value_changed (GTK_SPIN_BUTTON (scale));
}

gboolean
picman_spin_scale_get_scale_limits (PicmanSpinScale *scale,
                                  gdouble       *lower,
                                  gdouble       *upper)
{
  PicmanSpinScalePrivate *private;

  g_return_val_if_fail (PICMAN_IS_SPIN_SCALE (scale), FALSE);

  private = GET_PRIVATE (scale);

  if (lower)
    *lower = private->scale_lower;

  if (upper)
    *upper = private->scale_upper;

  return private->scale_limits_set;
}
