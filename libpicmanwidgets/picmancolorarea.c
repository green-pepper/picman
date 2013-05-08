/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorarea.c
 * Copyright (C) 2001-2002  Sven Neumann <sven@picman.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanbase/picmanbase.h"

#include "picmanwidgetstypes.h"

#include "picmancairo-utils.h"
#include "picmancolorarea.h"


/**
 * SECTION: picmancolorarea
 * @title: PicmanColorArea
 * @short_description: Displays a #PicmanRGB color, optionally with
 *                     alpha-channel.
 *
 * Displays a #PicmanRGB color, optionally with alpha-channel.
 **/


#define DRAG_PREVIEW_SIZE   32
#define DRAG_ICON_OFFSET    -8


enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_COLOR,
  PROP_TYPE,
  PROP_DRAG_MASK,
  PROP_DRAW_BORDER
};


static void      picman_color_area_get_property  (GObject            *object,
                                                guint               property_id,
                                                GValue             *value,
                                                GParamSpec         *pspec);
static void      picman_color_area_set_property  (GObject            *object,
                                                guint               property_id,
                                                const GValue       *value,
                                                GParamSpec         *pspec);
static void      picman_color_area_finalize      (GObject            *object);

static void      picman_color_area_size_allocate (GtkWidget          *widget,
                                                GtkAllocation      *allocation);
static void      picman_color_area_state_changed (GtkWidget          *widget,
                                                GtkStateType        previous_state);
static gboolean  picman_color_area_expose        (GtkWidget          *widget,
                                                GdkEventExpose     *event);
static void      picman_color_area_render_buf    (GtkWidget          *widget,
                                                gboolean            insensitive,
                                                PicmanColorAreaType   type,
                                                guchar             *buf,
                                                guint               width,
                                                guint               height,
                                                guint               rowstride,
                                                PicmanRGB            *color);
static void      picman_color_area_render        (PicmanColorArea      *area);

static void  picman_color_area_drag_begin         (GtkWidget        *widget,
                                                 GdkDragContext   *context);
static void  picman_color_area_drag_end           (GtkWidget        *widget,
                                                 GdkDragContext   *context);
static void  picman_color_area_drag_data_received (GtkWidget        *widget,
                                                 GdkDragContext   *context,
                                                 gint              x,
                                                 gint              y,
                                                 GtkSelectionData *selection_data,
                                                 guint             info,
                                                 guint             time);
static void  picman_color_area_drag_data_get      (GtkWidget        *widget,
                                                 GdkDragContext   *context,
                                                 GtkSelectionData *selection_data,
                                                 guint             info,
                                                 guint             time);


G_DEFINE_TYPE (PicmanColorArea, picman_color_area, GTK_TYPE_DRAWING_AREA)

#define parent_class picman_color_area_parent_class

static guint picman_color_area_signals[LAST_SIGNAL] = { 0 };

static const GtkTargetEntry target = { "application/x-color", 0 };


static void
picman_color_area_class_init (PicmanColorAreaClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  PicmanRGB         color;

  picman_color_area_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorAreaClass, color_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->get_property       = picman_color_area_get_property;
  object_class->set_property       = picman_color_area_set_property;
  object_class->finalize           = picman_color_area_finalize;

  widget_class->size_allocate      = picman_color_area_size_allocate;
  widget_class->state_changed      = picman_color_area_state_changed;
  widget_class->expose_event       = picman_color_area_expose;

  widget_class->drag_begin         = picman_color_area_drag_begin;
  widget_class->drag_end           = picman_color_area_drag_end;
  widget_class->drag_data_received = picman_color_area_drag_data_received;
  widget_class->drag_data_get      = picman_color_area_drag_data_get;

  klass->color_changed             = NULL;

  picman_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);

  /**
   * PicmanColorArea:color:
   *
   * The color displayed in the color area.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_COLOR,
                                   picman_param_spec_rgb ("color", NULL, NULL,
                                                        TRUE, &color,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
  /**
   * PicmanColorArea:type:
   *
   * The type of the color area.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_TYPE,
                                   g_param_spec_enum ("type", NULL, NULL,
                                                      PICMAN_TYPE_COLOR_AREA_TYPE,
                                                      PICMAN_COLOR_AREA_FLAT,
                                                      PICMAN_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT));
  /**
   * PicmanColorArea:drag-type:
   *
   * The event_mask that should trigger drags.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_DRAG_MASK,
                                   g_param_spec_flags ("drag-mask", NULL, NULL,
                                                       GDK_TYPE_MODIFIER_TYPE,
                                                       0,
                                                       PICMAN_PARAM_WRITABLE |
                                                       G_PARAM_CONSTRUCT_ONLY));
  /**
   * PicmanColorArea:draw-border:
   *
   * Whether to draw a thin border in the foreground color around the area.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_DRAW_BORDER,
                                   g_param_spec_boolean ("draw-border",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));
}

static void
picman_color_area_init (PicmanColorArea *area)
{
  area->buf         = NULL;
  area->width       = 0;
  area->height      = 0;
  area->rowstride   = 0;
  area->draw_border = FALSE;

  gtk_drag_dest_set (GTK_WIDGET (area),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     &target, 1,
                     GDK_ACTION_COPY);
}

static void
picman_color_area_finalize (GObject *object)
{
  PicmanColorArea *area = PICMAN_COLOR_AREA (object);

  if (area->buf)
    {
      g_free (area->buf);
      area->buf = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_area_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanColorArea *area = PICMAN_COLOR_AREA (object);

  switch (property_id)
    {
    case PROP_COLOR:
      g_value_set_boxed (value, &area->color);
      break;

    case PROP_TYPE:
      g_value_set_enum (value, area->type);
      break;

    case PROP_DRAW_BORDER:
      g_value_set_boolean (value, area->draw_border);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_area_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanColorArea   *area = PICMAN_COLOR_AREA (object);
  GdkModifierType  drag_mask;

  switch (property_id)
    {
    case PROP_COLOR:
      picman_color_area_set_color (area, g_value_get_boxed (value));
      break;

    case PROP_TYPE:
      picman_color_area_set_type (area, g_value_get_enum (value));
      break;

    case PROP_DRAG_MASK:
      drag_mask = g_value_get_flags (value) & (GDK_BUTTON1_MASK |
                                               GDK_BUTTON2_MASK |
                                               GDK_BUTTON3_MASK);
      if (drag_mask)
        gtk_drag_source_set (GTK_WIDGET (area),
                             drag_mask,
                             &target, 1,
                             GDK_ACTION_COPY | GDK_ACTION_MOVE);
      break;

    case PROP_DRAW_BORDER:
      picman_color_area_set_draw_border (area, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_area_size_allocate (GtkWidget     *widget,
                               GtkAllocation *allocation)
{
  PicmanColorArea *area = PICMAN_COLOR_AREA (widget);

  if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  if (allocation->width  != area->width ||
      allocation->height != area->height)
    {
      area->width  = allocation->width;
      area->height = allocation->height;

      area->rowstride = area->width * 4 + 4;

      g_free (area->buf);
      area->buf = g_new (guchar, area->rowstride * area->height);

      area->needs_render = TRUE;
    }
}

static void
picman_color_area_state_changed (GtkWidget    *widget,
                               GtkStateType  previous_state)
{
  if (gtk_widget_get_state (widget) == GTK_STATE_INSENSITIVE ||
      previous_state == GTK_STATE_INSENSITIVE)
    {
      PICMAN_COLOR_AREA (widget)->needs_render = TRUE;
    }

  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, previous_state);
}

static gboolean
picman_color_area_expose (GtkWidget      *widget,
                        GdkEventExpose *event)
{
  PicmanColorArea   *area  = PICMAN_COLOR_AREA (widget);
  GtkStyle        *style = gtk_widget_get_style (widget);
  cairo_t         *cr;
  cairo_surface_t *buffer;

  if (! area->buf || ! gtk_widget_is_drawable (widget))
    return FALSE;

  if (area->needs_render)
    picman_color_area_render (area);

  cr = gdk_cairo_create (gtk_widget_get_window (widget));

  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  buffer = cairo_image_surface_create_for_data (area->buf,
                                                CAIRO_FORMAT_RGB24,
                                                area->width,
                                                area->height,
                                                area->rowstride);
  cairo_set_source_surface (cr, buffer, 0.0, 0.0);
  cairo_surface_destroy (buffer);

  cairo_paint (cr);

  if (area->draw_border)
    {
      cairo_set_line_width (cr, 1.0);
      gdk_cairo_set_source_color (cr,
                                  &style->fg[gtk_widget_get_state (widget)]);

      cairo_rectangle (cr, 0.5, 0.5, area->width - 1, area->height - 1);

      cairo_stroke (cr);
    }

  cairo_destroy (cr);

  return FALSE;
}

/**
 * picman_color_area_new:
 * @color:     A pointer to a #PicmanRGB struct.
 * @type:      The type of color area to create.
 * @drag_mask: The event_mask that should trigger drags.
 *
 * Creates a new #PicmanColorArea widget.
 *
 * This returns a preview area showing the color. It handles color
 * DND. If the color changes, the "color_changed" signal is emitted.
 *
 * Returns: Pointer to the new #PicmanColorArea widget.
 **/
GtkWidget *
picman_color_area_new (const PicmanRGB     *color,
                     PicmanColorAreaType  type,
                     GdkModifierType    drag_mask)
{
  return g_object_new (PICMAN_TYPE_COLOR_AREA,
                       "color",     color,
                       "type",      type,
                       "drag-mask", drag_mask,
                       NULL);
}

/**
 * picman_color_area_set_color:
 * @area: Pointer to a #PicmanColorArea.
 * @color: Pointer to a #PicmanRGB struct that defines the new color.
 *
 * Sets @area to a different @color.
 **/
void
picman_color_area_set_color (PicmanColorArea *area,
                           const PicmanRGB *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_AREA (area));
  g_return_if_fail (color != NULL);

  if (picman_rgba_distance (&area->color, color) < 0.000001)
    return;

  area->color = *color;

  area->needs_render = TRUE;
  gtk_widget_queue_draw (GTK_WIDGET (area));

  g_object_notify (G_OBJECT (area), "color");

  g_signal_emit (area, picman_color_area_signals[COLOR_CHANGED], 0);
}

/**
 * picman_color_area_get_color:
 * @area: Pointer to a #PicmanColorArea.
 * @color: Pointer to a #PicmanRGB struct that is used to return the color.
 *
 * Retrieves the current color of the @area.
 **/
void
picman_color_area_get_color (PicmanColorArea *area,
                           PicmanRGB       *color)
{
  g_return_if_fail (PICMAN_IS_COLOR_AREA (area));
  g_return_if_fail (color != NULL);

  *color = area->color;
}

/**
 * picman_color_area_has_alpha:
 * @area: Pointer to a #PicmanColorArea.
 *
 * Checks whether the @area shows transparency information. This is determined
 * via the @area's #PicmanColorAreaType.
 *
 * Returns: %TRUE if @area shows transparency information, %FALSE otherwise.
 **/
gboolean
picman_color_area_has_alpha (PicmanColorArea *area)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_AREA (area), FALSE);

  return area->type != PICMAN_COLOR_AREA_FLAT;
}

/**
 * picman_color_area_set_type:
 * @area: Pointer to a #PicmanColorArea.
 * @type: A #PicmanColorAreaType.
 *
 * Changes the type of @area. The #PicmanColorAreaType determines
 * whether the widget shows transparency information and chooses the
 * size of the checkerboard used to do that.
 **/
void
picman_color_area_set_type (PicmanColorArea     *area,
                          PicmanColorAreaType  type)
{
  g_return_if_fail (PICMAN_IS_COLOR_AREA (area));

  if (area->type != type)
    {
      area->type = type;

      area->needs_render = TRUE;
      gtk_widget_queue_draw (GTK_WIDGET (area));

      g_object_notify (G_OBJECT (area), "type");
    }
}

/**
 * picman_color_area_set_draw_border:
 * @area: Pointer to a #PicmanColorArea.
 * @draw_border: whether to draw a border or not
 *
 * The @area can draw a thin border in the foreground color around
 * itself.  This function toggles this behaviour on and off. The
 * default is not draw a border.
 **/
void
picman_color_area_set_draw_border (PicmanColorArea *area,
                                 gboolean       draw_border)
{
  g_return_if_fail (PICMAN_IS_COLOR_AREA (area));

  draw_border = draw_border ? TRUE : FALSE;

  if (area->draw_border != draw_border)
    {
      area->draw_border = draw_border;

      gtk_widget_queue_draw (GTK_WIDGET (area));

      g_object_notify (G_OBJECT (area), "draw-border");
    }
}

static void
picman_color_area_render_buf (GtkWidget         *widget,
                            gboolean           insensitive,
                            PicmanColorAreaType  type,
                            guchar            *buf,
                            guint              width,
                            guint              height,
                            guint              rowstride,
                            PicmanRGB           *color)
{
  GtkStyle *style = gtk_widget_get_style (widget);
  guint     x, y;
  guint     check_size = 0;
  guchar    light[3];
  guchar    dark[3];
  guchar    opaque[3];
  guchar    insens[3];
  guchar   *p;
  gdouble   frac;

  switch (type)
    {
    case PICMAN_COLOR_AREA_FLAT:
      check_size = 0;
      break;

    case PICMAN_COLOR_AREA_SMALL_CHECKS:
      check_size = PICMAN_CHECK_SIZE_SM;
      break;

    case PICMAN_COLOR_AREA_LARGE_CHECKS:
      check_size = PICMAN_CHECK_SIZE;
      break;
    }

  picman_rgb_get_uchar (color, opaque, opaque + 1, opaque + 2);

  insens[0] = style->bg[GTK_STATE_INSENSITIVE].red   >> 8;
  insens[1] = style->bg[GTK_STATE_INSENSITIVE].green >> 8;
  insens[2] = style->bg[GTK_STATE_INSENSITIVE].blue  >> 8;

  if (insensitive || check_size == 0 || color->a == 1.0)
    {
      for (y = 0; y < height; y++)
        {
          p = buf + y * rowstride;

          for (x = 0; x < width; x++)
            {
              if (insensitive && ((x + y) % 2))
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              insens[0],
                                              insens[1],
                                              insens[2]);
                }
              else
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              opaque[0],
                                              opaque[1],
                                              opaque[2]);
                }

              p += 4;
            }
        }

      return;
    }

  light[0] = (PICMAN_CHECK_LIGHT +
              (color->r - PICMAN_CHECK_LIGHT) * color->a) * 255.999;
  light[1] = (PICMAN_CHECK_LIGHT +
              (color->g - PICMAN_CHECK_LIGHT) * color->a) * 255.999;
  light[2] = (PICMAN_CHECK_LIGHT +
              (color->b - PICMAN_CHECK_LIGHT) * color->a) * 255.999;

  dark[0] = (PICMAN_CHECK_DARK +
             (color->r - PICMAN_CHECK_DARK)  * color->a) * 255.999;
  dark[1] = (PICMAN_CHECK_DARK +
             (color->g - PICMAN_CHECK_DARK)  * color->a) * 255.999;
  dark[2] = (PICMAN_CHECK_DARK +
             (color->b - PICMAN_CHECK_DARK)  * color->a) * 255.999;

  for (y = 0; y < height; y++)
    {
      p = buf + y * rowstride;

      for (x = 0; x < width; x++)
        {
          if ((width - x) * height > y * width)
            {
              PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                          opaque[0],
                                          opaque[1],
                                          opaque[2]);
              p += 4;

              continue;
            }

          frac = y - (gdouble) ((width - x) * height) / (gdouble) width;

          if (((x / check_size) ^ (y / check_size)) & 1)
            {
              if ((gint) frac)
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              light[0],
                                              light[1],
                                              light[2]);
                }
              else
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              ((gdouble) light[0]  * frac +
                                               (gdouble) opaque[0] * (1.0 - frac)),
                                              ((gdouble) light[1]  * frac +
                                               (gdouble) opaque[1] * (1.0 - frac)),
                                              ((gdouble) light[2]  * frac +
                                               (gdouble) opaque[2] * (1.0 - frac)));
                }
            }
          else
            {
              if ((gint) frac)
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              dark[0],
                                              dark[1],
                                              dark[2]);
                }
              else
                {
                  PICMAN_CAIRO_RGB24_SET_PIXEL (p,
                                              ((gdouble) dark[0] * frac +
                                               (gdouble) opaque[0] * (1.0 - frac)),
                                              ((gdouble) dark[1] * frac +
                                               (gdouble) opaque[1] * (1.0 - frac)),
                                              ((gdouble) dark[2] * frac +
                                               (gdouble) opaque[2] * (1.0 - frac)));
                }
            }

          p += 4;
        }
    }
}

static void
picman_color_area_render (PicmanColorArea *area)
{
  if (! area->buf)
    return;

  picman_color_area_render_buf (GTK_WIDGET (area),
                              ! gtk_widget_is_sensitive (GTK_WIDGET (area)),
                              area->type,
                              area->buf,
                              area->width, area->height, area->rowstride,
                              &area->color);

  area->needs_render = FALSE;
}

static void
picman_color_area_drag_begin (GtkWidget      *widget,
                            GdkDragContext *context)
{
  PicmanRGB    color;
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *color_area;

  window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DND);
  gtk_window_set_screen (GTK_WINDOW (window), gtk_widget_get_screen (widget));

  gtk_widget_realize (window);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_container_add (GTK_CONTAINER (window), frame);

  picman_color_area_get_color (PICMAN_COLOR_AREA (widget), &color);

  color_area = picman_color_area_new (&color,
                                    PICMAN_COLOR_AREA (widget)->type,
                                    0);

  gtk_widget_set_size_request (color_area,
                               DRAG_PREVIEW_SIZE, DRAG_PREVIEW_SIZE);
  gtk_container_add (GTK_CONTAINER (frame), color_area);
  gtk_widget_show (color_area);
  gtk_widget_show (frame);

  g_object_set_data_full (G_OBJECT (widget),
                          "picman-color-area-drag-window",
                          window,
                          (GDestroyNotify) gtk_widget_destroy);

  gtk_drag_set_icon_widget (context, window,
                            DRAG_ICON_OFFSET, DRAG_ICON_OFFSET);
}

static void
picman_color_area_drag_end (GtkWidget      *widget,
                          GdkDragContext *context)
{
  g_object_set_data (G_OBJECT (widget),
                     "picman-color-area-drag-window", NULL);
}

static void
picman_color_area_drag_data_received (GtkWidget        *widget,
                                    GdkDragContext   *context,
                                    gint              x,
                                    gint              y,
                                    GtkSelectionData *selection_data,
                                    guint             info,
                                    guint             time)
{
  PicmanColorArea *area = PICMAN_COLOR_AREA (widget);
  const guint16 *vals;
  PicmanRGB        color;

  if (gtk_selection_data_get_length (selection_data) != 8 ||
      gtk_selection_data_get_format (selection_data) != 16)
    {
      g_warning ("%s: received invalid color data", G_STRFUNC);
      return;
    }

  vals = (const guint16 *) gtk_selection_data_get_data (selection_data);

  picman_rgba_set (&color,
                 (gdouble) vals[0] / 0xffff,
                 (gdouble) vals[1] / 0xffff,
                 (gdouble) vals[2] / 0xffff,
                 (gdouble) vals[3] / 0xffff);

  picman_color_area_set_color (area, &color);
}

static void
picman_color_area_drag_data_get (GtkWidget        *widget,
                               GdkDragContext   *context,
                               GtkSelectionData *selection_data,
                               guint             info,
                               guint             time)
{
  PicmanColorArea *area = PICMAN_COLOR_AREA (widget);
  guint16        vals[4];

  vals[0] = area->color.r * 0xffff;
  vals[1] = area->color.g * 0xffff;
  vals[2] = area->color.b * 0xffff;

  if (area->type == PICMAN_COLOR_AREA_FLAT)
    vals[3] = 0xffff;
  else
    vals[3] = area->color.a * 0xffff;

  gtk_selection_data_set (selection_data,
                          gdk_atom_intern ("application/x-color", FALSE),
                          16, (guchar *) vals, 8);
}
