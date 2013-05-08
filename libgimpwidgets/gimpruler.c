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

#include <string.h>

#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "picmanwidgetstypes.h"

#include "picmanruler.h"


/**
 * SECTION: picmanruler
 * @title: PicmanRuler
 * @short_description: A ruler widget with configurable unit and orientation.
 *
 * A ruler widget with configurable unit and orientation.
 **/


#define DEFAULT_RULER_FONT_SCALE  PANGO_SCALE_SMALL
#define MINIMUM_INCR              5


enum
{
  PROP_0,
  PROP_ORIENTATION,
  PROP_UNIT,
  PROP_LOWER,
  PROP_UPPER,
  PROP_POSITION,
  PROP_MAX_SIZE
};


/* All distances below are in 1/72nd's of an inch. (According to
 * Adobe that's a point, but points are really 1/72.27 in.)
 */
typedef struct
{
  GtkOrientation   orientation;
  PicmanUnit         unit;
  gdouble          lower;
  gdouble          upper;
  gdouble          position;
  gdouble          max_size;

  GdkWindow       *input_window;
  cairo_surface_t *backing_store;
  PangoLayout     *layout;
  gdouble          font_scale;

  gint             xsrc;
  gint             ysrc;

  GList           *track_widgets;
} PicmanRulerPrivate;

#define PICMAN_RULER_GET_PRIVATE(ruler) \
  G_TYPE_INSTANCE_GET_PRIVATE (ruler, PICMAN_TYPE_RULER, PicmanRulerPrivate)


static const struct
{
  const gdouble  ruler_scale[16];
  const gint     subdivide[5];
} ruler_metric =
{
  { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000 },
  { 1, 5, 10, 50, 100 }
};


static void          picman_ruler_dispose       (GObject        *object);
static void          picman_ruler_set_property  (GObject        *object,
                                               guint            prop_id,
                                               const GValue   *value,
                                               GParamSpec     *pspec);
static void          picman_ruler_get_property  (GObject        *object,
                                               guint           prop_id,
                                               GValue         *value,
                                               GParamSpec     *pspec);

static void          picman_ruler_realize       (GtkWidget      *widget);
static void          picman_ruler_unrealize     (GtkWidget      *widget);
static void          picman_ruler_map           (GtkWidget      *widget);
static void          picman_ruler_unmap         (GtkWidget      *widget);
static void          picman_ruler_size_allocate (GtkWidget      *widget,
                                               GtkAllocation  *allocation);
static void          picman_ruler_size_request  (GtkWidget      *widget,
                                               GtkRequisition *requisition);
static void          picman_ruler_style_set     (GtkWidget      *widget,
                                               GtkStyle       *prev_style);
static gboolean      picman_ruler_motion_notify (GtkWidget      *widget,
                                               GdkEventMotion *event);
static gboolean      picman_ruler_expose        (GtkWidget      *widget,
                                               GdkEventExpose *event);

static void          picman_ruler_draw_ticks    (PicmanRuler      *ruler);
static void          picman_ruler_draw_pos      (PicmanRuler      *ruler);
static void          picman_ruler_make_pixmap   (PicmanRuler      *ruler);

static PangoLayout * picman_ruler_get_layout    (GtkWidget      *widget,
                                               const gchar    *text);


G_DEFINE_TYPE (PicmanRuler, picman_ruler, GTK_TYPE_WIDGET)

#define parent_class picman_ruler_parent_class


static void
picman_ruler_class_init (PicmanRulerClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose             = picman_ruler_dispose;
  object_class->set_property        = picman_ruler_set_property;
  object_class->get_property        = picman_ruler_get_property;

  widget_class->realize             = picman_ruler_realize;
  widget_class->unrealize           = picman_ruler_unrealize;
  widget_class->map                 = picman_ruler_map;
  widget_class->unmap               = picman_ruler_unmap;
  widget_class->size_allocate       = picman_ruler_size_allocate;
  widget_class->size_request        = picman_ruler_size_request;
  widget_class->style_set           = picman_ruler_style_set;
  widget_class->motion_notify_event = picman_ruler_motion_notify;
  widget_class->expose_event        = picman_ruler_expose;

  g_type_class_add_private (object_class, sizeof (PicmanRulerPrivate));

  g_object_class_install_property (object_class,
                                   PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation",
                                                      "Orientation",
                                                      "The orientation of the ruler",
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_LOWER,
                                   picman_param_spec_unit ("unit",
                                                         "Unit",
                                                         "Unit of ruler",
                                                         TRUE, TRUE,
                                                         PICMAN_UNIT_PIXEL,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
                                                        "Lower",
                                                        "Lower limit of ruler",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
                                                        "Upper",
                                                        "Upper limit of ruler",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_POSITION,
                                   g_param_spec_double ("position",
                                                        "Position",
                                                        "Position of mark on the ruler",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_MAX_SIZE,
                                   g_param_spec_double ("max-size",
                                                        "Max Size",
                                                        "Maximum size of the ruler",
                                                        -G_MAXDOUBLE,
                                                        G_MAXDOUBLE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_double ("font-scale",
                                                                NULL, NULL,
                                                                0.0,
                                                                G_MAXDOUBLE,
                                                                DEFAULT_RULER_FONT_SCALE,
                                                                PICMAN_PARAM_READABLE));
}

static void
picman_ruler_init (PicmanRuler *ruler)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (ruler);

  gtk_widget_set_has_window (GTK_WIDGET (ruler), FALSE);

  priv->orientation   = GTK_ORIENTATION_HORIZONTAL;
  priv->unit          = PICMAN_PIXELS;
  priv->lower         = 0;
  priv->upper         = 0;
  priv->position      = 0;
  priv->max_size      = 0;
  priv->backing_store = NULL;
  priv->font_scale    = DEFAULT_RULER_FONT_SCALE;
}

static void
picman_ruler_dispose (GObject *object)
{
  PicmanRuler        *ruler = PICMAN_RULER (object);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);

  while (priv->track_widgets)
    picman_ruler_remove_track_widget (ruler, priv->track_widgets->data);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_ruler_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  PicmanRuler        *ruler = PICMAN_RULER (object);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      priv->orientation = g_value_get_enum (value);
      gtk_widget_queue_resize (GTK_WIDGET (ruler));
      break;

    case PROP_UNIT:
      picman_ruler_set_unit (ruler, g_value_get_int (value));
      break;

    case PROP_LOWER:
      picman_ruler_set_range (ruler,
                            g_value_get_double (value),
                            priv->upper,
                            priv->max_size);
      break;
    case PROP_UPPER:
      picman_ruler_set_range (ruler,
                            priv->lower,
                            g_value_get_double (value),
                            priv->max_size);
      break;

    case PROP_POSITION:
      picman_ruler_set_position (ruler, g_value_get_double (value));
      break;

    case PROP_MAX_SIZE:
      picman_ruler_set_range (ruler,
                            priv->lower,
                            priv->upper,
                            g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
picman_ruler_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  PicmanRuler        *ruler = PICMAN_RULER (object);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, priv->orientation);
      break;

    case PROP_UNIT:
      g_value_set_int (value, priv->unit);
      break;

    case PROP_LOWER:
      g_value_set_double (value, priv->lower);
      break;

    case PROP_UPPER:
      g_value_set_double (value, priv->upper);
      break;

    case PROP_POSITION:
      g_value_set_double (value, priv->position);
      break;

    case PROP_MAX_SIZE:
      g_value_set_double (value, priv->max_size);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * picman_ruler_new:
 * @orientation: the ruler's orientation.
 *
 * Creates a new ruler.
 *
 * Return value: a new #PicmanRuler widget.
 *
 * Since: PICMAN 2.8
 **/
GtkWidget *
picman_ruler_new (GtkOrientation orientation)
{
  return g_object_new (PICMAN_TYPE_RULER,
                       "orientation", orientation,
                       NULL);
}

static void
picman_ruler_update_position (PicmanRuler *ruler,
                            gdouble    x,
                            gdouble    y)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (ruler);
  GtkAllocation     allocation;
  gdouble           lower;
  gdouble           upper;

  gtk_widget_get_allocation (GTK_WIDGET (ruler), &allocation);
  picman_ruler_get_range (ruler, &lower, &upper, NULL);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      picman_ruler_set_position (ruler,
                               lower +
                               (upper - lower) * x / allocation.width);
    }
  else
    {
      picman_ruler_set_position (ruler,
                               lower +
                               (upper - lower) * y / allocation.height);
    }
}

/* Returns TRUE if a translation should be done */
static gboolean
gtk_widget_get_translation_to_window (GtkWidget *widget,
                                      GdkWindow *window,
                                      int       *x,
                                      int       *y)
{
  GdkWindow *w, *widget_window;

  if (! gtk_widget_get_has_window (widget))
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation (widget, &allocation);

      *x = -allocation.x;
      *y = -allocation.y;
    }
  else
    {
      *x = 0;
      *y = 0;
    }

  widget_window = gtk_widget_get_window (widget);

  for (w = window;
       w && w != widget_window;
       w = gdk_window_get_effective_parent (w))
    {
      gdouble px, py;

      gdk_window_coords_to_parent (w, *x, *y, &px, &py);

      *x += px;
      *y += py;
    }

  if (w == NULL)
    {
      *x = 0;
      *y = 0;
      return FALSE;
    }

  return TRUE;
}

static void
picman_ruler_event_to_widget_coords (GtkWidget *widget,
                                   GdkWindow *window,
                                   gdouble    event_x,
                                   gdouble    event_y,
                                   gint      *widget_x,
                                   gint      *widget_y)
{
  gint tx, ty;

  if (gtk_widget_get_translation_to_window (widget, window, &tx, &ty))
    {
      event_x += tx;
      event_y += ty;
    }

  *widget_x = event_x;
  *widget_y = event_y;
}

static gboolean
picman_ruler_track_widget_motion_notify (GtkWidget      *widget,
                                       GdkEventMotion *mevent,
                                       PicmanRuler      *ruler)
{
  gint widget_x;
  gint widget_y;
  gint ruler_x;
  gint ruler_y;

  widget = gtk_get_event_widget ((GdkEvent *) mevent);

  picman_ruler_event_to_widget_coords (widget, mevent->window,
                                     mevent->x, mevent->y,
                                     &widget_x, &widget_y);

  if (gtk_widget_translate_coordinates (widget, GTK_WIDGET (ruler),
                                        widget_x, widget_y,
                                        &ruler_x, &ruler_y))
    {
      picman_ruler_update_position (ruler, ruler_x, ruler_y);
    }

  return FALSE;
}

/**
 * picman_ruler_add_track_widget:
 * @ruler: a #PicmanRuler
 * @widget: the track widget to add
 *
 * Adds a "track widget" to the ruler. The ruler will connect to
 * GtkWidget:motion-notify-event: on the track widget and update its
 * position marker accordingly. The marker is correctly updated also
 * for the track widget's children, regardless of whether they are
 * ordinary children of off-screen children.
 *
 * Since: PICMAN 2.8
 */
void
picman_ruler_add_track_widget (PicmanRuler *ruler,
                             GtkWidget *widget)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));
  g_return_if_fail (GTK_IS_WIDGET (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  g_return_if_fail (g_list_find (priv->track_widgets, widget) == NULL);

  priv->track_widgets = g_list_prepend (priv->track_widgets, widget);

  g_signal_connect (widget, "motion-notify-event",
                    G_CALLBACK (picman_ruler_track_widget_motion_notify),
                    ruler);
  g_signal_connect_swapped (widget, "destroy",
                            G_CALLBACK (picman_ruler_remove_track_widget),
                            ruler);
}

/**
 * picman_ruler_remove_track_widget:
 * @ruler: a #PicmanRuler
 * @widget: the track widget to remove
 *
 * Removes a previously added track widget from the ruler. See
 * picman_ruler_add_track_widget().
 *
 * Since: PICMAN 2.8
 */
void
picman_ruler_remove_track_widget (PicmanRuler *ruler,
                                GtkWidget *widget)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));
  g_return_if_fail (GTK_IS_WIDGET (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  g_return_if_fail (g_list_find (priv->track_widgets, widget) != NULL);

  priv->track_widgets = g_list_remove (priv->track_widgets, widget);

  g_signal_handlers_disconnect_by_func (widget,
                                        picman_ruler_track_widget_motion_notify,
                                        ruler);
  g_signal_handlers_disconnect_by_func (widget,
                                        picman_ruler_remove_track_widget,
                                        ruler);
}

/**
 * picman_ruler_set_unit:
 * @ruler: a #PicmanRuler
 * @unit:  the #PicmanUnit to set the ruler to
 *
 * This sets the unit of the ruler.
 *
 * Since: PICMAN 2.8
 */
void
picman_ruler_set_unit (PicmanRuler *ruler,
                     PicmanUnit   unit)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  if (priv->unit != unit)
    {
      priv->unit = unit;
      g_object_notify (G_OBJECT (ruler), "unit");

      gtk_widget_queue_draw (GTK_WIDGET (ruler));
    }
}

/**
 * picman_ruler_get_unit:
 * @ruler: a #PicmanRuler
 *
 * Return value: the unit currently used in the @ruler widget.
 *
 * Since: PICMAN 2.8
 **/
PicmanUnit
picman_ruler_get_unit (PicmanRuler *ruler)
{
  g_return_val_if_fail (PICMAN_IS_RULER (ruler), 0);

  return PICMAN_RULER_GET_PRIVATE (ruler)->unit;
}

/**
 * picman_ruler_set_position:
 * @ruler: a #PicmanRuler
 * @position: the position to set the ruler to
 *
 * This sets the position of the ruler.
 *
 * Since: PICMAN 2.8
 */
void
picman_ruler_set_position (PicmanRuler *ruler,
                         gdouble    position)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  if (priv->position != position)
    {
      priv->position = position;
      g_object_notify (G_OBJECT (ruler), "position");

      picman_ruler_draw_pos (ruler);
    }
}

/**
 * picman_ruler_get_position:
 * @ruler: a #PicmanRuler
 *
 * Return value: the current position of the @ruler widget.
 *
 * Since: PICMAN 2.8
 **/
gdouble
picman_ruler_get_position (PicmanRuler *ruler)
{
  g_return_val_if_fail (PICMAN_IS_RULER (ruler), 0.0);

  return PICMAN_RULER_GET_PRIVATE (ruler)->position;
}

/**
 * picman_ruler_set_range:
 * @ruler: a #PicmanRuler
 * @lower: the lower limit of the ruler
 * @upper: the upper limit of the ruler
 * @max_size: the maximum size of the ruler used when calculating the space to
 * leave for the text
 *
 * This sets the range of the ruler.
 *
 * Since: PICMAN 2.8
 */
void
picman_ruler_set_range (PicmanRuler *ruler,
                      gdouble    lower,
                      gdouble    upper,
                      gdouble    max_size)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  g_object_freeze_notify (G_OBJECT (ruler));
  if (priv->lower != lower)
    {
      priv->lower = lower;
      g_object_notify (G_OBJECT (ruler), "lower");
    }
  if (priv->upper != upper)
    {
      priv->upper = upper;
      g_object_notify (G_OBJECT (ruler), "upper");
    }
  if (priv->max_size != max_size)
    {
      priv->max_size = max_size;
      g_object_notify (G_OBJECT (ruler), "max-size");
    }
  g_object_thaw_notify (G_OBJECT (ruler));

  gtk_widget_queue_draw (GTK_WIDGET (ruler));
}

/**
 * picman_ruler_get_range:
 * @ruler: a #PicmanRuler
 * @lower: location to store lower limit of the ruler, or %NULL
 * @upper: location to store upper limit of the ruler, or %NULL
 * @max_size: location to store the maximum size of the ruler used when
 *            calculating the space to leave for the text, or %NULL.
 *
 * Retrieves values indicating the range and current position of a #PicmanRuler.
 * See picman_ruler_set_range().
 *
 * Since: PICMAN 2.8
 **/
void
picman_ruler_get_range (PicmanRuler *ruler,
                      gdouble   *lower,
                      gdouble   *upper,
                      gdouble   *max_size)
{
  PicmanRulerPrivate *priv;

  g_return_if_fail (PICMAN_IS_RULER (ruler));

  priv = PICMAN_RULER_GET_PRIVATE (ruler);

  if (lower)
    *lower = priv->lower;
  if (upper)
    *upper = priv->upper;
  if (max_size)
    *max_size = priv->max_size;
}

static void
picman_ruler_realize (GtkWidget *widget)
{
  PicmanRuler        *ruler = PICMAN_RULER (widget);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);
  GtkAllocation     allocation;
  GdkWindowAttr     attributes;
  gint              attributes_mask;

  GTK_WIDGET_CLASS (picman_ruler_parent_class)->realize (widget);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.wclass      = GDK_INPUT_ONLY;
  attributes.event_mask  = (gtk_widget_get_events (widget) |
                            GDK_EXPOSURE_MASK              |
                            GDK_POINTER_MOTION_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  priv->input_window = gdk_window_new (gtk_widget_get_window (widget),
                                       &attributes, attributes_mask);
  gdk_window_set_user_data (priv->input_window, ruler);

  picman_ruler_make_pixmap (ruler);
}

static void
picman_ruler_unrealize (GtkWidget *widget)
{
  PicmanRuler        *ruler = PICMAN_RULER (widget);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);

  if (priv->backing_store)
    {
      cairo_surface_destroy (priv->backing_store);
      priv->backing_store = NULL;
    }

  if (priv->layout)
    {
      g_object_unref (priv->layout);
      priv->layout = NULL;
    }

  if (priv->input_window)
    {
      gdk_window_destroy (priv->input_window);
      priv->input_window = NULL;
    }

  GTK_WIDGET_CLASS (picman_ruler_parent_class)->unrealize (widget);
}

static void
picman_ruler_map (GtkWidget *widget)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (parent_class)->map (widget);

  if (priv->input_window)
    gdk_window_show (priv->input_window);
}

static void
picman_ruler_unmap (GtkWidget *widget)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (widget);

  if (priv->input_window)
    gdk_window_hide (priv->input_window);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
picman_ruler_size_allocate (GtkWidget     *widget,
                          GtkAllocation *allocation)
{
  PicmanRuler        *ruler = PICMAN_RULER (widget);
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize (priv->input_window,
                              allocation->x, allocation->y,
                              allocation->width, allocation->height);

      picman_ruler_make_pixmap (ruler);
    }
}

static void
picman_ruler_size_request (GtkWidget      *widget,
                         GtkRequisition *requisition)
{
  PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (widget);
  GtkStyle         *style = gtk_widget_get_style (widget);
  PangoLayout      *layout;
  PangoRectangle    ink_rect;
  gint              size;

  layout = picman_ruler_get_layout (widget, "0123456789");
  pango_layout_get_pixel_extents (layout, &ink_rect, NULL);

  size = 2 + ink_rect.height * 1.7;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      requisition->width  = style->xthickness * 2 + 1;
      requisition->height = style->ythickness * 2 + size;
    }
  else
    {
      requisition->width  = style->xthickness * 2 + size;
      requisition->height = style->ythickness * 2 + 1;
    }
}

static void
picman_ruler_style_set (GtkWidget *widget,
                      GtkStyle  *prev_style)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (widget);

  GTK_WIDGET_CLASS (picman_ruler_parent_class)->style_set (widget, prev_style);

  gtk_widget_style_get (widget,
                        "font-scale", &priv->font_scale,
                        NULL);

  if (priv->layout)
    {
      g_object_unref (priv->layout);
      priv->layout = NULL;
    }
}

static gboolean
picman_ruler_motion_notify (GtkWidget      *widget,
                          GdkEventMotion *event)
{
  PicmanRuler *ruler = PICMAN_RULER (widget);

  picman_ruler_update_position (ruler, event->x, event->y);

  return FALSE;
}

static gboolean
picman_ruler_expose (GtkWidget      *widget,
                   GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      PicmanRuler        *ruler = PICMAN_RULER (widget);
      PicmanRulerPrivate *priv  = PICMAN_RULER_GET_PRIVATE (ruler);
      GtkAllocation     allocation;
      cairo_t          *cr;

      picman_ruler_draw_ticks (ruler);

      cr = gdk_cairo_create (gtk_widget_get_window (widget));
      gdk_cairo_region (cr, event->region);
      cairo_clip (cr);

      gtk_widget_get_allocation (widget, &allocation);
      cairo_translate (cr, allocation.x, allocation.y);

      cairo_set_source_surface (cr, priv->backing_store, 0, 0);
      cairo_paint (cr);

      picman_ruler_draw_pos (ruler);

      cairo_destroy (cr);
    }

  return FALSE;
}

static void
picman_ruler_draw_ticks (PicmanRuler *ruler)
{
  GtkWidget        *widget = GTK_WIDGET (ruler);
  GtkStyle         *style  = gtk_widget_get_style (widget);
  PicmanRulerPrivate *priv   = PICMAN_RULER_GET_PRIVATE (ruler);
  GtkStateType      state  = gtk_widget_get_state (widget);
  GtkAllocation     allocation;
  cairo_t          *cr;
  gint              i;
  gint              width, height;
  gint              xthickness;
  gint              ythickness;
  gint              length, ideal_length;
  gdouble           lower, upper;  /* Upper and lower limits, in ruler units */
  gdouble           increment;     /* Number of pixels per unit */
  gint              scale;         /* Number of units per major unit */
  gdouble           start, end, cur;
  gchar             unit_str[32];
  gint              digit_height;
  gint              digit_offset;
  gint              text_size;
  gint              pos;
  gdouble           max_size;
  PicmanUnit          unit;
  PangoLayout      *layout;
  PangoRectangle    logical_rect, ink_rect;

  if (! gtk_widget_is_drawable (widget))
    return;

  gtk_widget_get_allocation (widget, &allocation);

  xthickness = style->xthickness;
  ythickness = style->ythickness;

  layout = picman_ruler_get_layout (widget, "0123456789");
  pango_layout_get_extents (layout, &ink_rect, &logical_rect);

  digit_height = PANGO_PIXELS (ink_rect.height) + 2;
  digit_offset = ink_rect.y;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      width  = allocation.width;
      height = allocation.height - ythickness * 2;
    }
  else
    {
      width  = allocation.height;
      height = allocation.width - ythickness * 2;
    }

  cr = cairo_create (priv->backing_store);
  gdk_cairo_set_source_color (cr, &style->bg[state]);

#if 0
  gtk_paint_box (style, priv->backing_store,
                 GTK_STATE_NORMAL, GTK_SHADOW_OUT,
                 NULL, widget,
                 priv->orientation == GTK_ORIENTATION_HORIZONTAL ?
                 "hruler" : "vruler",
                 0, 0,
                 allocation.width, allocation.height);
#else
  cairo_paint (cr);
#endif

  gdk_cairo_set_source_color (cr, &style->fg[state]);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      cairo_rectangle (cr,
                       xthickness,
                       height + ythickness,
                       allocation.width - 2 * xthickness,
                       1);
    }
  else
    {
      cairo_rectangle (cr,
                       height + xthickness,
                       ythickness,
                       1,
                       allocation.height - 2 * ythickness);
    }

  picman_ruler_get_range (ruler, &lower, &upper, &max_size);

  if ((upper - lower) == 0)
    goto out;

  increment = (gdouble) width / (upper - lower);

  /* determine the scale
   *   use the maximum extents of the ruler to determine the largest
   *   possible number to be displayed.  Calculate the height in pixels
   *   of this displayed text. Use this height to find a scale which
   *   leaves sufficient room for drawing the ruler.
   *
   *   We calculate the text size as for the vruler instead of
   *   actually measuring the text width, so that the result for the
   *   scale looks consistent with an accompanying vruler.
   */
  scale = ceil (max_size);
  g_snprintf (unit_str, sizeof (unit_str), "%d", scale);
  text_size = strlen (unit_str) * digit_height + 1;

  for (scale = 0; scale < G_N_ELEMENTS (ruler_metric.ruler_scale); scale++)
    if (ruler_metric.ruler_scale[scale] * fabs (increment) > 2 * text_size)
      break;

  if (scale == G_N_ELEMENTS (ruler_metric.ruler_scale))
    scale = G_N_ELEMENTS (ruler_metric.ruler_scale) - 1;

  unit = picman_ruler_get_unit (ruler);

  /* drawing starts here */
  length = 0;
  for (i = G_N_ELEMENTS (ruler_metric.subdivide) - 1; i >= 0; i--)
    {
      gdouble subd_incr;

      /* hack to get proper subdivisions at full pixels */
      if (unit == PICMAN_UNIT_PIXEL && scale == 1 && i == 1)
        subd_incr = 1.0;
      else
        subd_incr = ((gdouble) ruler_metric.ruler_scale[scale] /
                     (gdouble) ruler_metric.subdivide[i]);

      if (subd_incr * fabs (increment) <= MINIMUM_INCR)
        continue;

      /* don't subdivide pixels */
      if (unit == PICMAN_UNIT_PIXEL && subd_incr < 1.0)
        continue;

      /* Calculate the length of the tickmarks. Make sure that
       * this length increases for each set of ticks
       */
      ideal_length = height / (i + 1) - 1;
      if (ideal_length > ++length)
        length = ideal_length;

      if (lower < upper)
        {
          start = floor (lower / subd_incr) * subd_incr;
          end   = ceil  (upper / subd_incr) * subd_incr;
        }
      else
        {
          start = floor (upper / subd_incr) * subd_incr;
          end   = ceil  (lower / subd_incr) * subd_incr;
        }

      for (cur = start; cur <= end; cur += subd_incr)
        {
          pos = ROUND ((cur - lower) * increment);

          if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              cairo_rectangle (cr,
                               pos, height + ythickness - length,
                               1,   length);
            }
          else
            {
              cairo_rectangle (cr,
                               height + xthickness - length, pos,
                               length,                       1);
            }

          /* draw label */
          if (i == 0)
            {
              g_snprintf (unit_str, sizeof (unit_str), "%d", (int) cur);

              if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
                {
                  pango_layout_set_text (layout, unit_str, -1);
                  pango_layout_get_extents (layout, &logical_rect, NULL);

#if 0
                  gtk_paint_layout (style,
                                    priv->backing_store,
                                    state,
                                    FALSE,
                                    NULL,
                                    widget,
                                    "hruler",
                                    pos + 2,
                                    ythickness + PANGO_PIXELS (logical_rect.y - digit_offset),
                                    layout);
#else
                  cairo_move_to (cr,
                                 pos + 2,
                                 ythickness + PANGO_PIXELS (logical_rect.y - digit_offset));
                  pango_cairo_show_layout (cr, layout);
#endif
                }
              else
                {
                  gint j;

                  for (j = 0; j < (int) strlen (unit_str); j++)
                    {
                      pango_layout_set_text (layout, unit_str + j, 1);
                      pango_layout_get_extents (layout, NULL, &logical_rect);

#if 0
                      gtk_paint_layout (style,
                                        priv->backing_store,
                                        state,
                                        FALSE,
                                        NULL,
                                        widget,
                                        "vruler",
                                        xthickness + 1,
                                        pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset),
                                        layout);
#else
                      cairo_move_to (cr,
                                     xthickness + 1,
                                     pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset));
                      pango_cairo_show_layout (cr, layout);
#endif

                    }
                }
            }
        }
    }

  cairo_fill (cr);
out:
  cairo_destroy (cr);
}

static void
picman_ruler_draw_pos (PicmanRuler *ruler)
{
  GtkWidget        *widget = GTK_WIDGET (ruler);
  GtkStyle         *style  = gtk_widget_get_style (widget);
  PicmanRulerPrivate *priv   = PICMAN_RULER_GET_PRIVATE (ruler);
  GtkStateType      state  = gtk_widget_get_state (widget);
  GtkAllocation     allocation;
  gint              x, y;
  gint              width, height;
  gint              bs_width, bs_height;
  gint              xthickness;
  gint              ythickness;

  if (! gtk_widget_is_drawable (widget))
    return;

  gtk_widget_get_allocation (widget, &allocation);

  xthickness = style->xthickness;
  ythickness = style->ythickness;

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      width  = allocation.width;
      height = allocation.height - ythickness * 2;

      bs_width = height / 2 + 2;
      bs_width |= 1;  /* make sure it's odd */
      bs_height = bs_width / 2 + 1;
    }
  else
    {
      width  = allocation.width - xthickness * 2;
      height = allocation.height;

      bs_height = width / 2 + 2;
      bs_height |= 1;  /* make sure it's odd */
      bs_width = bs_height / 2 + 1;
    }

  if ((bs_width > 0) && (bs_height > 0))
    {
      cairo_t *cr = gdk_cairo_create (gtk_widget_get_window (widget));
      gdouble  lower;
      gdouble  upper;
      gdouble  position;
      gdouble  increment;

      cairo_rectangle (cr,
                       allocation.x, allocation.y,
                       allocation.width, allocation.height);
      cairo_clip (cr);

      cairo_translate (cr, allocation.x, allocation.y);

      /*  If a backing store exists, restore the ruler  */
      if (priv->backing_store)
        {
          cairo_set_source_surface (cr, priv->backing_store, 0, 0);
          cairo_rectangle (cr, priv->xsrc, priv->ysrc, bs_width, bs_height);
          cairo_fill (cr);
        }

      position = picman_ruler_get_position (ruler);

      picman_ruler_get_range (ruler, &lower, &upper, NULL);

      if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          increment = (gdouble) width / (upper - lower);

          x = ROUND ((position - lower) * increment) + (xthickness - bs_width) / 2 - 1;
          y = (height + bs_height) / 2 + ythickness;
        }
      else
        {
          increment = (gdouble) height / (upper - lower);

          x = (width + bs_width) / 2 + xthickness;
          y = ROUND ((position - lower) * increment) + (ythickness - bs_height) / 2 - 1;
        }

      gdk_cairo_set_source_color (cr, &style->fg[state]);

      cairo_move_to (cr, x, y);

      if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          cairo_line_to (cr, x + bs_width / 2.0, y + bs_height);
          cairo_line_to (cr, x + bs_width,       y);
        }
      else
        {
          cairo_line_to (cr, x + bs_width, y + bs_height / 2.0);
          cairo_line_to (cr, x,            y + bs_height);
        }

      cairo_fill (cr);

      cairo_destroy (cr);

      priv->xsrc = x;
      priv->ysrc = y;
    }
}

static void
picman_ruler_make_pixmap (PicmanRuler *ruler)
{
  GtkWidget        *widget = GTK_WIDGET (ruler);
  PicmanRulerPrivate *priv   = PICMAN_RULER_GET_PRIVATE (ruler);
  GtkAllocation     allocation;

  gtk_widget_get_allocation (widget, &allocation);

  if (priv->backing_store)
    cairo_surface_destroy (priv->backing_store);

  priv->backing_store =
    gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                       CAIRO_CONTENT_COLOR,
                                       allocation.width,
                                       allocation.height);
}


static PangoLayout *
picman_ruler_create_layout (GtkWidget   *widget,
                          const gchar *text)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (widget);
  PangoLayout      *layout;
  PangoAttrList    *attrs;
  PangoAttribute   *attr;

  layout = gtk_widget_create_pango_layout (widget, text);

  attrs = pango_attr_list_new ();

  attr = pango_attr_scale_new (priv->font_scale);
  attr->start_index = 0;
  attr->end_index   = -1;
  pango_attr_list_insert (attrs, attr);

  pango_layout_set_attributes (layout, attrs);
  pango_attr_list_unref (attrs);

  return layout;
}

static PangoLayout *
picman_ruler_get_layout (GtkWidget   *widget,
                       const gchar *text)
{
  PicmanRulerPrivate *priv = PICMAN_RULER_GET_PRIVATE (widget);

  if (priv->layout)
    {
      pango_layout_set_text (priv->layout, text, -1);
      return priv->layout;
    }

  priv->layout = picman_ruler_create_layout (widget, text);

  return priv->layout;
}
