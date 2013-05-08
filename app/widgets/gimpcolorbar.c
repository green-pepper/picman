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

#include <string.h>

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "widgets-types.h"

#include "picmancolorbar.h"


enum
{
  PROP_0,
  PROP_ORIENTATION,
  PROP_COLOR,
  PROP_CHANNEL
};


/*  local function prototypes  */

static void      picman_color_bar_set_property (GObject        *object,
                                              guint           property_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);
static void      picman_color_bar_get_property (GObject        *object,
                                              guint           property_id,
                                              GValue         *value,
                                              GParamSpec     *pspec);

static gboolean  picman_color_bar_expose       (GtkWidget      *widget,
                                              GdkEventExpose *event);


G_DEFINE_TYPE (PicmanColorBar, picman_color_bar, GTK_TYPE_EVENT_BOX)

#define parent_class picman_color_bar_parent_class


static void
picman_color_bar_class_init (PicmanColorBarClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanRGB         white        = { 1.0, 1.0, 1.0, 1.0 };

  object_class->set_property = picman_color_bar_set_property;
  object_class->get_property = picman_color_bar_get_property;

  widget_class->expose_event = picman_color_bar_expose;

  g_object_class_install_property (object_class, PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation",
                                                      NULL, NULL,
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (object_class, PROP_COLOR,
                                   picman_param_spec_rgb ("color",
                                                        NULL, NULL,
                                                        FALSE, &white,
                                                        PICMAN_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_CHANNEL,
                                   g_param_spec_enum ("histogram-channel",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_HISTOGRAM_CHANNEL,
                                                      PICMAN_HISTOGRAM_VALUE,
                                                      PICMAN_PARAM_WRITABLE));
}

static void
picman_color_bar_init (PicmanColorBar *bar)
{
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (bar), FALSE);

  bar->orientation = GTK_ORIENTATION_HORIZONTAL;
}


static void
picman_color_bar_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  PicmanColorBar *bar = PICMAN_COLOR_BAR (object);

  switch (property_id)
    {
    case PROP_ORIENTATION:
      bar->orientation = g_value_get_enum (value);
      break;
    case PROP_COLOR:
      picman_color_bar_set_color (bar, g_value_get_boxed (value));
      break;
    case PROP_CHANNEL:
      picman_color_bar_set_channel (bar, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_bar_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  PicmanColorBar *bar = PICMAN_COLOR_BAR (object);

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
picman_color_bar_expose (GtkWidget      *widget,
                       GdkEventExpose *event)
{
  PicmanColorBar    *bar = PICMAN_COLOR_BAR (widget);
  cairo_t         *cr;
  GtkAllocation    allocation;
  cairo_surface_t *surface;
  cairo_pattern_t *pattern;
  guchar          *src;
  guchar          *dest;
  gint             x, y;
  gint             width, height;
  gint             i;

  cr = gdk_cairo_create (event->window);

  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  gtk_widget_get_allocation (widget, &allocation);

  x = y = gtk_container_get_border_width (GTK_CONTAINER (bar));

  width  = allocation.width  - 2 * x;
  height = allocation.height - 2 * y;

  if (width < 1 || height < 1)
    return TRUE;

  cairo_translate (cr, allocation.x + x, allocation.y + y);
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_clip (cr);

  surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 256, 1);

  for (i = 0, src = bar->buf, dest = cairo_image_surface_get_data (surface);
       i < 256;
       i++, src += 3, dest += 4)
    {
      PICMAN_CAIRO_RGB24_SET_PIXEL(dest, src[0], src[1], src[2]);
    }

  cairo_surface_mark_dirty (surface);

  pattern = cairo_pattern_create_for_surface (surface);
  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REFLECT);
  cairo_surface_destroy (surface);

  if (bar->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      cairo_scale (cr, (gdouble) width / 256.0, 1.0);
    }
  else
    {
      cairo_translate (cr, 0, height);
      cairo_scale (cr, 1.0, (gdouble) height / 256.0);
      cairo_rotate (cr, - G_PI / 2);
    }

  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);

  cairo_paint (cr);

  cairo_destroy (cr);

  return TRUE;
}


/*  public functions  */

/**
 * picman_color_bar_new:
 * @orientation: whether the bar should be oriented horizontally or
 *               vertically
 *
 * Creates a new #PicmanColorBar widget.
 *
 * Return value: The new #PicmanColorBar widget.
 **/
GtkWidget *
picman_color_bar_new (GtkOrientation  orientation)
{
  return g_object_new (PICMAN_TYPE_COLOR_BAR,
                       "orientation", orientation,
                       NULL);
}

/**
 * picman_color_bar_set_color:
 * @bar:   a #PicmanColorBar widget
 * @color: a #PicmanRGB color
 *
 * Makes the @bar display a gradient from black (on the left or the
 * bottom), to the given @color (on the right or at the top).
 **/
void
picman_color_bar_set_color (PicmanColorBar  *bar,
                          const PicmanRGB *color)
{
  guchar *buf;
  gint    i;

  g_return_if_fail (PICMAN_IS_COLOR_BAR (bar));
  g_return_if_fail (color != NULL);

  for (i = 0, buf = bar->buf; i < 256; i++, buf += 3)
    {
      buf[0] = ROUND (color->r * (gdouble) i);
      buf[1] = ROUND (color->g * (gdouble) i);
      buf[2] = ROUND (color->b * (gdouble) i);
    }

  gtk_widget_queue_draw (GTK_WIDGET (bar));
}

/**
 * picman_color_bar_set_channel:
 * @bar:     a #PicmanColorBar widget
 * @channel: a #PicmanHistogramChannel
 *
 * Convenience function that calls picman_color_bar_set_color() with the
 * color that matches the @channel.
 **/
void
picman_color_bar_set_channel (PicmanColorBar         *bar,
                            PicmanHistogramChannel  channel)
{
  PicmanRGB  color = { 1.0, 1.0, 1.0, 1.0 };

  g_return_if_fail (PICMAN_IS_COLOR_BAR (bar));

  switch (channel)
    {
    case PICMAN_HISTOGRAM_VALUE:
    case PICMAN_HISTOGRAM_ALPHA:
    case PICMAN_HISTOGRAM_RGB:
      picman_rgb_set (&color, 1.0, 1.0, 1.0);
      break;
    case PICMAN_HISTOGRAM_RED:
      picman_rgb_set (&color, 1.0, 0.0, 0.0);
      break;
    case PICMAN_HISTOGRAM_GREEN:
      picman_rgb_set (&color, 0.0, 1.0, 0.0);
      break;
    case PICMAN_HISTOGRAM_BLUE:
      picman_rgb_set (&color, 0.0, 0.0, 1.0);
      break;
    }

  picman_color_bar_set_color (bar, &color);
}

/**
 * picman_color_bar_set_buffers:
 * @bar:   a #PicmanColorBar widget
 * @red:   an array of 256 values
 * @green: an array of 256 values
 * @blue:  an array of 256 values
 *
 * This function gives full control over the colors displayed by the
 * @bar widget. The 3 arrays can for example be taken from a #Levels
 * or a #Curves struct.
 **/
void
picman_color_bar_set_buffers (PicmanColorBar *bar,
                            const guchar *red,
                            const guchar *green,
                            const guchar *blue)
{
  guchar *buf;
  gint    i;

  g_return_if_fail (PICMAN_IS_COLOR_BAR (bar));
  g_return_if_fail (red != NULL);
  g_return_if_fail (green != NULL);
  g_return_if_fail (blue != NULL);

  for (i = 0, buf = bar->buf; i < 256; i++, buf += 3)
    {
      buf[0] = red[i];
      buf[1] = green[i];
      buf[2] = blue[i];
    }

  gtk_widget_queue_draw (GTK_WIDGET (bar));
}
