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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "display-types.h"

#include "config/picmandisplayconfig.h"

#include "widgets/picmanwidgets-utils.h"

#include "picmancanvas.h"

#include "picman-intl.h"


#define MAX_BATCH_SIZE 32000


enum
{
  PROP_0,
  PROP_CONFIG
};


/*  local function prototypes  */

static void       picman_canvas_set_property    (GObject         *object,
                                               guint            property_id,
                                               const GValue    *value,
                                               GParamSpec      *pspec);
static void       picman_canvas_get_property    (GObject         *object,
                                               guint            property_id,
                                               GValue          *value,
                                               GParamSpec      *pspec);

static void       picman_canvas_unrealize       (GtkWidget       *widget);
static void       picman_canvas_style_set       (GtkWidget       *widget,
                                               GtkStyle        *prev_style);
static gboolean   picman_canvas_focus_in_event  (GtkWidget       *widget,
                                               GdkEventFocus   *event);
static gboolean   picman_canvas_focus_out_event (GtkWidget       *widget,
                                               GdkEventFocus   *event);
static gboolean   picman_canvas_focus           (GtkWidget       *widget,
                                               GtkDirectionType direction);


G_DEFINE_TYPE (PicmanCanvas, picman_canvas, PICMAN_TYPE_OVERLAY_BOX)

#define parent_class picman_canvas_parent_class


static void
picman_canvas_class_init (PicmanCanvasClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property    = picman_canvas_set_property;
  object_class->get_property    = picman_canvas_get_property;

  widget_class->unrealize       = picman_canvas_unrealize;
  widget_class->style_set       = picman_canvas_style_set;
  widget_class->focus_in_event  = picman_canvas_focus_in_event;
  widget_class->focus_out_event = picman_canvas_focus_out_event;
  widget_class->focus           = picman_canvas_focus;

  g_object_class_install_property (object_class, PROP_CONFIG,
                                   g_param_spec_object ("config", NULL, NULL,
                                                        PICMAN_TYPE_DISPLAY_CONFIG,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_canvas_init (PicmanCanvas *canvas)
{
  GtkWidget *widget = GTK_WIDGET (canvas);

  gtk_widget_set_can_focus (widget, TRUE);
  gtk_widget_add_events (widget, PICMAN_CANVAS_EVENT_MASK);
  gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_ALL);
}

static void
picman_canvas_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  PicmanCanvas *canvas = PICMAN_CANVAS (object);

  switch (property_id)
    {
    case PROP_CONFIG:
      canvas->config = g_value_get_object (value); /* don't dup */
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_get_property (GObject    *object,
                          guint       property_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  PicmanCanvas *canvas = PICMAN_CANVAS (object);

  switch (property_id)
    {
    case PROP_CONFIG:
      g_value_set_object (value, canvas->config);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_canvas_unrealize (GtkWidget *widget)
{
  PicmanCanvas *canvas = PICMAN_CANVAS (widget);

  if (canvas->layout)
    {
      g_object_unref (canvas->layout);
      canvas->layout = NULL;
    }

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
picman_canvas_style_set (GtkWidget *widget,
                       GtkStyle  *prev_style)
{
  PicmanCanvas *canvas = PICMAN_CANVAS (widget);

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (canvas->layout)
    {
      g_object_unref (canvas->layout);
      canvas->layout = NULL;
    }
}

static gboolean
picman_canvas_focus_in_event (GtkWidget     *widget,
                            GdkEventFocus *event)
{
  /*  don't allow the default impl to invalidate the whole widget,
   *  we don't draw a focus indicator anyway.
   */
  return FALSE;
}

static gboolean
picman_canvas_focus_out_event (GtkWidget     *widget,
                             GdkEventFocus *event)
{
  /*  see focus-in-event
   */
  return FALSE;
}

static gboolean
picman_canvas_focus (GtkWidget        *widget,
                   GtkDirectionType  direction)
{
  GtkWidget *focus = gtk_container_get_focus_child (GTK_CONTAINER (widget));

  /* override GtkContainer's focus() implementation which would always
   * give focus to the canvas because it is focussable. Instead, try
   * navigating in the focussed overlay child first, and use
   * GtkContainer's default implementation only if that fails (which
   * happens when focus navigation leaves the overlay child).
   */

  if (focus && gtk_widget_child_focus (focus, direction))
    return TRUE;

  return GTK_WIDGET_CLASS (parent_class)->focus (widget, direction);
}


/*  public functions  */

/**
 * picman_canvas_new:
 *
 * Creates a new #PicmanCanvas widget.
 *
 * The #PicmanCanvas widget is a #GtkDrawingArea abstraction. It manages
 * a set of graphic contexts for drawing on a PICMAN display. If you
 * draw using a #PicmanCanvasStyle, #PicmanCanvas makes sure that the
 * associated #GdkGC is created. All drawing on the canvas needs to
 * happen by means of the #PicmanCanvas drawing functions. Besides from
 * not needing a #GdkGC pointer, the #PicmanCanvas drawing functions
 * look and work like their #GdkDrawable counterparts. #PicmanCanvas
 * gracefully handles attempts to draw on the unrealized widget.
 *
 * Return value: a new #PicmanCanvas widget
 **/
GtkWidget *
picman_canvas_new (PicmanDisplayConfig *config)
{
  g_return_val_if_fail (PICMAN_IS_DISPLAY_CONFIG (config), NULL);

  return g_object_new (PICMAN_TYPE_CANVAS,
                       "name",   "picman-canvas",
                       "config", config,
                       NULL);
}

/**
 * picman_canvas_get_layout:
 * @canvas:  a #PicmanCanvas widget
 * @format:  a standard printf() format string.
 * @Varargs: the parameters to insert into the format string.
 *
 * Returns a layout which can be used for
 * pango_cairo_show_layout(). The layout belongs to the canvas and
 * should not be freed, not should a pointer to it be kept around
 * after drawing.
 *
 * Returns: a #PangoLayout owned by the canvas.
 **/
PangoLayout *
picman_canvas_get_layout (PicmanCanvas  *canvas,
                        const gchar *format,
                        ...)
{
  va_list  args;
  gchar   *text;

  if (! canvas->layout)
    canvas->layout = gtk_widget_create_pango_layout (GTK_WIDGET (canvas),
                                                     NULL);

  va_start (args, format);
  text = g_strdup_vprintf (format, args);
  va_end (args);

  pango_layout_set_text (canvas->layout, text, -1);
  g_free (text);

  return canvas->layout;
}

/**
 * picman_canvas_set_bg_color:
 * @canvas:   a #PicmanCanvas widget
 * @color:    a color in #PicmanRGB format
 *
 * Sets the background color of the canvas's window.  This
 * is the color the canvas is set to if it is cleared.
 **/
void
picman_canvas_set_bg_color (PicmanCanvas *canvas,
                          PicmanRGB    *color)
{
  GtkWidget   *widget = GTK_WIDGET (canvas);
  GdkColormap *colormap;
  GdkColor     gdk_color;

  if (! gtk_widget_get_realized (widget))
    return;

  picman_rgb_get_gdk_color (color, &gdk_color);

  colormap = gdk_drawable_get_colormap (gtk_widget_get_window (widget));
  g_return_if_fail (colormap != NULL);
  gdk_colormap_alloc_color (colormap, &gdk_color, FALSE, TRUE);

  gdk_window_set_background (gtk_widget_get_window (widget), &gdk_color);

  gtk_widget_queue_draw (GTK_WIDGET (canvas));
}
