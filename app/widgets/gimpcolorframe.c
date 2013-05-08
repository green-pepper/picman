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

#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "gegl/picman-babl.h"

#include "core/picmanimage.h"

#include "picmancolorframe.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_MODE,
  PROP_HAS_NUMBER,
  PROP_NUMBER,
  PROP_HAS_COLOR_AREA
};


/*  local function prototypes  */

static void       picman_color_frame_finalize      (GObject        *object);
static void       picman_color_frame_get_property  (GObject        *object,
                                                  guint           property_id,
                                                  GValue         *value,
                                                  GParamSpec     *pspec);
static void       picman_color_frame_set_property  (GObject        *object,
                                                  guint           property_id,
                                                  const GValue   *value,
                                                  GParamSpec     *pspec);

static void       picman_color_frame_style_set     (GtkWidget      *widget,
                                                  GtkStyle       *prev_style);
static gboolean   picman_color_frame_expose        (GtkWidget      *widget,
                                                  GdkEventExpose *eevent);

static void       picman_color_frame_menu_callback (GtkWidget      *widget,
                                                  PicmanColorFrame *frame);
static void       picman_color_frame_update        (PicmanColorFrame *frame);


G_DEFINE_TYPE (PicmanColorFrame, picman_color_frame, PICMAN_TYPE_FRAME)

#define parent_class picman_color_frame_parent_class


static void
picman_color_frame_class_init (PicmanColorFrameClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize     = picman_color_frame_finalize;
  object_class->get_property = picman_color_frame_get_property;
  object_class->set_property = picman_color_frame_set_property;

  widget_class->style_set    = picman_color_frame_style_set;
  widget_class->expose_event = picman_color_frame_expose;

  g_object_class_install_property (object_class, PROP_MODE,
                                   g_param_spec_enum ("mode",
                                                      NULL, NULL,
                                                      PICMAN_TYPE_COLOR_FRAME_MODE,
                                                      PICMAN_COLOR_FRAME_MODE_PIXEL,
                                                      PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_HAS_NUMBER,
                                   g_param_spec_boolean ("has-number",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_NUMBER,
                                   g_param_spec_int ("number",
                                                     NULL, NULL,
                                                     0, 256, 0,
                                                     PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_HAS_COLOR_AREA,
                                   g_param_spec_boolean ("has-color-area",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE));
}

static void
picman_color_frame_init (PicmanColorFrame *frame)
{
  GtkWidget *vbox;
  GtkWidget *vbox2;
  gint       i;

  frame->sample_valid  = FALSE;
  frame->sample_format = babl_format ("R'G'B' u8");

  picman_rgba_set (&frame->color, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);

  frame->menu = picman_enum_combo_box_new (PICMAN_TYPE_COLOR_FRAME_MODE);
  gtk_frame_set_label_widget (GTK_FRAME (frame), frame->menu);
  gtk_widget_show (frame->menu);

  g_signal_connect (frame->menu, "changed",
                    G_CALLBACK (picman_color_frame_menu_callback),
                    frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  frame->color_area =
    g_object_new (PICMAN_TYPE_COLOR_AREA,
                  "color",          &frame->color,
                  "type",           PICMAN_COLOR_AREA_SMALL_CHECKS,
                  "drag-mask",      GDK_BUTTON1_MASK,
                  "draw-border",    TRUE,
                  "height-request", 20,
                  NULL);
  gtk_box_pack_start (GTK_BOX (vbox), frame->color_area, FALSE, FALSE, 0);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_box_set_homogeneous (GTK_BOX (vbox2), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), vbox2, FALSE, FALSE, 0);
  gtk_widget_show (vbox2);

  for (i = 0; i < PICMAN_COLOR_FRAME_ROWS; i++)
    {
      GtkWidget *hbox;

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
      gtk_widget_show (hbox);

      frame->name_labels[i] = gtk_label_new (" ");
      gtk_misc_set_alignment (GTK_MISC (frame->name_labels[i]), 0.0, 0.5);
      gtk_box_pack_start (GTK_BOX (hbox), frame->name_labels[i],
                          FALSE, FALSE, 0);
      gtk_widget_show (frame->name_labels[i]);

      frame->value_labels[i] = gtk_label_new (" ");
      gtk_label_set_selectable (GTK_LABEL (frame->value_labels[i]), TRUE);
      gtk_misc_set_alignment (GTK_MISC (frame->value_labels[i]), 1.0, 0.5);
      gtk_box_pack_end (GTK_BOX (hbox), frame->value_labels[i],
                        FALSE, FALSE, 0);
      gtk_widget_show (frame->value_labels[i]);
    }
}

static void
picman_color_frame_finalize (GObject *object)
{
  PicmanColorFrame *frame = PICMAN_COLOR_FRAME (object);

  if (frame->number_layout)
    {
      g_object_unref (frame->number_layout);
      frame->number_layout = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_color_frame_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanColorFrame *frame = PICMAN_COLOR_FRAME (object);

  switch (property_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, frame->frame_mode);
      break;

    case PROP_HAS_NUMBER:
      g_value_set_boolean (value, frame->has_number);
      break;

    case PROP_NUMBER:
      g_value_set_int (value, frame->number);
      break;

    case PROP_HAS_COLOR_AREA:
      g_value_set_boolean (value, frame->has_color_area);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_frame_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanColorFrame *frame = PICMAN_COLOR_FRAME (object);

  switch (property_id)
    {
    case PROP_MODE:
      picman_color_frame_set_mode (frame, g_value_get_enum (value));
      break;

    case PROP_HAS_NUMBER:
      picman_color_frame_set_has_number (frame, g_value_get_boolean (value));
      break;

    case PROP_NUMBER:
      picman_color_frame_set_number (frame, g_value_get_int (value));
      break;

    case PROP_HAS_COLOR_AREA:
      picman_color_frame_set_has_color_area (frame, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_frame_style_set (GtkWidget *widget,
                            GtkStyle  *prev_style)
{
  PicmanColorFrame *frame = PICMAN_COLOR_FRAME (widget);

  GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);

  if (frame->number_layout)
    {
      g_object_unref (frame->number_layout);
      frame->number_layout = NULL;
    }
}

static gboolean
picman_color_frame_expose (GtkWidget      *widget,
                         GdkEventExpose *eevent)
{
  PicmanColorFrame *frame = PICMAN_COLOR_FRAME (widget);

  if (frame->has_number)
    {
      GtkStyle      *style = gtk_widget_get_style (widget);
      GtkAllocation  allocation;
      GtkAllocation  menu_allocation;
      GtkAllocation  color_area_allocation;
      cairo_t       *cr;
      gchar          buf[8];
      gint           w, h;
      gdouble        scale;

      gtk_widget_get_allocation (widget, &allocation);
      gtk_widget_get_allocation (frame->menu, &menu_allocation);
      gtk_widget_get_allocation (frame->color_area, &color_area_allocation);

      cr = gdk_cairo_create (gtk_widget_get_window (widget));
      gdk_cairo_region (cr, eevent->region);
      cairo_clip (cr);

      cairo_translate (cr, allocation.x, allocation.y);

      gdk_cairo_set_source_color (cr, &style->light[GTK_STATE_NORMAL]);

      g_snprintf (buf, sizeof (buf), "%d", frame->number);

      if (! frame->number_layout)
        frame->number_layout = gtk_widget_create_pango_layout (widget, NULL);

      pango_layout_set_text (frame->number_layout, buf, -1);
      pango_layout_get_pixel_size (frame->number_layout, &w, &h);

      scale = ((gdouble) (allocation.height -
                          menu_allocation.height -
                          color_area_allocation.height) /
               (gdouble) h);

      cairo_scale (cr, scale, scale);

      cairo_move_to (cr,
                     (allocation.width / 2.0) / scale - w / 2.0,
                     (allocation.height / 2.0 +
                      menu_allocation.height / 2.0 +
                      color_area_allocation.height / 2.0) / scale - h / 2.0);
      pango_cairo_show_layout (cr, frame->number_layout);

      cairo_destroy (cr);
    }

  return GTK_WIDGET_CLASS (parent_class)->expose_event (widget, eevent);
}


/*  public functions  */

/**
 * picman_color_frame_new:
 *
 * Creates a new #PicmanColorFrame widget.
 *
 * Return value: The new #PicmanColorFrame widget.
 **/
GtkWidget *
picman_color_frame_new (void)
{
  return g_object_new (PICMAN_TYPE_COLOR_FRAME, NULL);
}


/**
 * picman_color_frame_set_mode:
 * @frame: The #PicmanColorFrame.
 * @mode:  The new @mode.
 *
 * Sets the #PicmanColorFrame's color @mode. Calling this function does
 * the same as selecting the @mode from the frame's #GtkOptionMenu.
 **/
void
picman_color_frame_set_mode (PicmanColorFrame     *frame,
                           PicmanColorFrameMode  mode)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));

  picman_int_combo_box_set_active (PICMAN_INT_COMBO_BOX (frame->menu), mode);

  g_object_notify (G_OBJECT (frame), "mode");
}

void
picman_color_frame_set_has_number (PicmanColorFrame *frame,
                                 gboolean        has_number)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));

  if (has_number != frame->has_number)
    {
      frame->has_number = has_number ? TRUE : FALSE;

      gtk_widget_queue_draw (GTK_WIDGET (frame));

      g_object_notify (G_OBJECT (frame), "has-number");
    }
}

void
picman_color_frame_set_number (PicmanColorFrame *frame,
                             gint            number)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));

  if (number != frame->number)
    {
      frame->number = number;

      gtk_widget_queue_draw (GTK_WIDGET (frame));

      g_object_notify (G_OBJECT (frame), "number");
    }
}

void
picman_color_frame_set_has_color_area (PicmanColorFrame *frame,
                                     gboolean        has_color_area)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));

  if (has_color_area != frame->has_color_area)
    {
      frame->has_color_area = has_color_area ? TRUE : FALSE;

      g_object_set (frame->color_area, "visible", frame->has_color_area, NULL);

      g_object_notify (G_OBJECT (frame), "has-color-area");
    }
}

/**
 * picman_color_frame_set_color:
 * @frame:         The #PicmanColorFrame.
 * @sample_format: The format of the #PicmanDrawable or #PicmanImage the @color
 *                 was picked from.
 * @color:         The @color to set.
 * @color_index:   The @color's index. This value is ignored unless
 *                 @sample_format is an indexed format.
 *
 * Sets the color sample to display in the #PicmanColorFrame.
 **/
void
picman_color_frame_set_color (PicmanColorFrame *frame,
                            const Babl     *sample_format,
                            const PicmanRGB  *color,
                            gint            color_index)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));
  g_return_if_fail (color != NULL);

  if (frame->sample_valid                   &&
      frame->sample_format == sample_format &&
      frame->color_index == color_index     &&
      picman_rgba_distance (&frame->color, color) < 0.0001)
    {
      frame->color = *color;
      return;
    }

  frame->sample_valid  = TRUE;
  frame->sample_format = sample_format;
  frame->color         = *color;
  frame->color_index   = color_index;

  picman_color_frame_update (frame);
}

/**
 * picman_color_frame_set_invalid:
 * @frame: The #PicmanColorFrame.
 *
 * Tells the #PicmanColorFrame that the current sample is invalid. All labels
 * visible for the current color space will show "n/a" (not available).
 *
 * There is no special API for setting the frame to "valid" again because
 * this happens automatically when calling picman_color_frame_set_color().
 **/
void
picman_color_frame_set_invalid (PicmanColorFrame *frame)
{
  g_return_if_fail (PICMAN_IS_COLOR_FRAME (frame));

  if (! frame->sample_valid)
    return;

  frame->sample_valid = FALSE;

  picman_color_frame_update (frame);
}


/*  private functions  */

static void
picman_color_frame_menu_callback (GtkWidget      *widget,
                                PicmanColorFrame *frame)
{
  gint value;

  if (picman_int_combo_box_get_active (PICMAN_INT_COMBO_BOX (widget), &value))
    {
      frame->frame_mode = value;
      picman_color_frame_update (frame);
    }
}

static void
picman_color_frame_update (PicmanColorFrame *frame)
{
  const gchar *names[PICMAN_COLOR_FRAME_ROWS]  = { NULL, };
  gchar       *values[PICMAN_COLOR_FRAME_ROWS] = { NULL, };
  gboolean     has_alpha;
  gint         alpha_row = 3;
  guchar       r, g, b, a;
  gint         i;

  has_alpha = babl_format_has_alpha (frame->sample_format);

  if (frame->sample_valid)
    {
      picman_rgba_get_uchar (&frame->color, &r, &g, &b, &a);

      picman_color_area_set_color (PICMAN_COLOR_AREA (frame->color_area),
                                 &frame->color);
    }

  switch (frame->frame_mode)
    {
    case PICMAN_COLOR_FRAME_MODE_PIXEL:
      if (picman_babl_format_get_base_type (frame->sample_format) == PICMAN_GRAY)
        {
          names[0]  = _("Value:");

          if (frame->sample_valid)
            values[0] = g_strdup_printf ("%d", r);

          alpha_row = 1;
        }
      else
        {
          names[0] = _("Red:");
          names[1] = _("Green:");
          names[2] = _("Blue:");

          if (frame->sample_valid)
            {
              values[0] = g_strdup_printf ("%d", r);
              values[1] = g_strdup_printf ("%d", g);
              values[2] = g_strdup_printf ("%d", b);
            }

          alpha_row = 3;

          if (babl_format_is_palette (frame->sample_format))
            {
              names[4] = _("Index:");

              if (frame->sample_valid)
                {
                  /* color_index will be -1 for an averaged sample */
                  if (frame->color_index < 0)
                    names[4] = NULL;
                  else
                    values[4] = g_strdup_printf ("%d", frame->color_index);
                }
            }
        }
      break;

    case PICMAN_COLOR_FRAME_MODE_RGB:
      names[0] = _("Red:");
      names[1] = _("Green:");
      names[2] = _("Blue:");

      if (frame->sample_valid)
        {
          values[0] = g_strdup_printf ("%d %%", ROUND (frame->color.r * 100.0));
          values[1] = g_strdup_printf ("%d %%", ROUND (frame->color.g * 100.0));
          values[2] = g_strdup_printf ("%d %%", ROUND (frame->color.b * 100.0));
        }

      alpha_row = 3;

      names[4]  = _("Hex:");

      if (frame->sample_valid)
        values[4] = g_strdup_printf ("%.2x%.2x%.2x", r, g, b);
      break;

    case PICMAN_COLOR_FRAME_MODE_HSV:
      names[0] = _("Hue:");
      names[1] = _("Sat.:");
      names[2] = _("Value:");

      if (frame->sample_valid)
        {
          PicmanHSV hsv;

          picman_rgb_to_hsv (&frame->color, &hsv);

          values[0] = g_strdup_printf ("%d \302\260", ROUND (hsv.h * 360.0));
          values[1] = g_strdup_printf ("%d %%",       ROUND (hsv.s * 100.0));
          values[2] = g_strdup_printf ("%d %%",       ROUND (hsv.v * 100.0));
        }

      alpha_row = 3;
      break;

    case PICMAN_COLOR_FRAME_MODE_CMYK:
      names[0] = _("Cyan:");
      names[1] = _("Magenta:");
      names[2] = _("Yellow:");
      names[3] = _("Black:");

      if (frame->sample_valid)
        {
          PicmanCMYK cmyk;

          picman_rgb_to_cmyk (&frame->color, 1.0, &cmyk);

          values[0] = g_strdup_printf ("%d %%", ROUND (cmyk.c * 100.0));
          values[1] = g_strdup_printf ("%d %%", ROUND (cmyk.m * 100.0));
          values[2] = g_strdup_printf ("%d %%", ROUND (cmyk.y * 100.0));
          values[3] = g_strdup_printf ("%d %%", ROUND (cmyk.k * 100.0));
        }

      alpha_row = 4;
      break;
    }

  if (has_alpha)
    {
      names[alpha_row] = _("Alpha:");

      if (frame->sample_valid)
        {
          if (frame->frame_mode == PICMAN_COLOR_FRAME_MODE_PIXEL)
            values[alpha_row] = g_strdup_printf ("%d", a);
          else
            values[alpha_row] = g_strdup_printf ("%d %%",
                                                 (gint) (frame->color.a * 100.0));
        }
    }

  for (i = 0; i < PICMAN_COLOR_FRAME_ROWS; i++)
    {
      if (names[i])
        {
          gtk_label_set_text (GTK_LABEL (frame->name_labels[i]), names[i]);

          if (frame->sample_valid)
            gtk_label_set_text (GTK_LABEL (frame->value_labels[i]), values[i]);
          else
            gtk_label_set_text (GTK_LABEL (frame->value_labels[i]), _("n/a"));
        }
      else
        {
          gtk_label_set_text (GTK_LABEL (frame->name_labels[i]),  " ");
          gtk_label_set_text (GTK_LABEL (frame->value_labels[i]), " ");
        }

      g_free (values[i]);
    }
}
