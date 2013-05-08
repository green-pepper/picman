/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmanchainbutton.c
 * Copyright (C) 1999-2000 Sven Neumann <sven@picman.org>
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

#include <gtk/gtk.h>

#include "picmanwidgetstypes.h"

#include "picmanchainbutton.h"
#include "picmanstock.h"


/**
 * SECTION: picmanchainbutton
 * @title: PicmanChainButton
 * @short_description: Widget to visually connect two entry widgets.
 * @see_also: You may want to use the convenience function
 *            picman_coordinates_new() to set up two PicmanSizeEntries
 *            (see #PicmanSizeEntry) linked with a #PicmanChainButton.
 *
 * This widget provides a button showing either a linked or a broken
 * chain that can be used to link two entries, spinbuttons, colors or
 * other GUI elements and show that they may be locked. Use it for
 * example to connect X and Y ratios to provide the possibility of a
 * constrained aspect ratio.
 *
 * The #PicmanChainButton only gives visual feedback, it does not really
 * connect widgets. You have to take care of locking the values
 * yourself by checking the state of the #PicmanChainButton whenever a
 * value changes in one of the connected widgets and adjusting the
 * other value if necessary.
 **/


enum
{
  PROP_0,
  PROP_POSITION
};

enum
{
  TOGGLED,
  LAST_SIGNAL
};

static void      picman_chain_button_constructed      (GObject         *object);
static void      picman_chain_button_set_property     (GObject         *object,
                                                     guint            property_id,
                                                     const GValue    *value,
                                                     GParamSpec      *pspec);
static void      picman_chain_button_get_property     (GObject         *object,
                                                     guint            property_id,
                                                     GValue          *value,
                                                     GParamSpec      *pspec);

static void      picman_chain_button_clicked_callback (GtkWidget       *widget,
                                                     PicmanChainButton *button);
static void      picman_chain_button_update_image     (PicmanChainButton *button);

static GtkWidget * picman_chain_line_new            (PicmanChainPosition  position,
                                                   gint               which);


G_DEFINE_TYPE (PicmanChainButton, picman_chain_button, GTK_TYPE_TABLE)

#define parent_class picman_chain_button_parent_class

static guint picman_chain_button_signals[LAST_SIGNAL] = { 0 };

static const gchar * const picman_chain_stock_items[] =
{
  PICMAN_STOCK_HCHAIN,
  PICMAN_STOCK_HCHAIN_BROKEN,
  PICMAN_STOCK_VCHAIN,
  PICMAN_STOCK_VCHAIN_BROKEN
};


static void
picman_chain_button_class_init (PicmanChainButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_chain_button_constructed;
  object_class->set_property = picman_chain_button_set_property;
  object_class->get_property = picman_chain_button_get_property;

  picman_chain_button_signals[TOGGLED] =
    g_signal_new ("toggled",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanChainButtonClass, toggled),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  klass->toggled = NULL;

  /**
   * PicmanChainButton:position:
   *
   * The position in which the chain button will be used.
   *
   * Since: PICMAN 2.4
   */
  g_object_class_install_property (object_class, PROP_POSITION,
                                   g_param_spec_enum ("position", NULL, NULL,
                                                      PICMAN_TYPE_CHAIN_POSITION,
                                                      PICMAN_CHAIN_TOP,
                                                      G_PARAM_CONSTRUCT_ONLY |
                                                      PICMAN_PARAM_READWRITE));
}

static void
picman_chain_button_init (PicmanChainButton *button)
{
  button->position = PICMAN_CHAIN_TOP;
  button->active   = FALSE;
  button->image    = gtk_image_new ();
  button->button   = gtk_button_new ();

  gtk_button_set_relief (GTK_BUTTON (button->button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button->button), button->image);
  gtk_widget_show (button->image);

  g_signal_connect (button->button, "clicked",
                    G_CALLBACK (picman_chain_button_clicked_callback),
                    button);
}

static void
picman_chain_button_constructed (GObject *object)
{
  PicmanChainButton *button = PICMAN_CHAIN_BUTTON (object);

  G_OBJECT_CLASS (parent_class)->constructed (object);

  button->line1 = picman_chain_line_new (button->position, 1);
  button->line2 = picman_chain_line_new (button->position, -1);

  picman_chain_button_update_image (button);

  if (button->position & PICMAN_CHAIN_LEFT) /* are we a vertical chainbutton? */
    {
      gtk_table_resize (GTK_TABLE (button), 3, 1);
      gtk_table_attach (GTK_TABLE (button), button->button, 0, 1, 1, 2,
                        GTK_SHRINK, GTK_SHRINK, 0, 0);
      gtk_table_attach_defaults (GTK_TABLE (button),
                                 button->line1, 0, 1, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (button),
                                 button->line2, 0, 1, 2, 3);
    }
  else
    {
      gtk_table_resize (GTK_TABLE (button), 1, 3);
      gtk_table_attach (GTK_TABLE (button), button->button, 1, 2, 0, 1,
                        GTK_SHRINK, GTK_SHRINK, 0, 0);
      gtk_table_attach_defaults (GTK_TABLE (button),
                                 button->line1, 0, 1, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (button),
                                 button->line2, 2, 3, 0, 1);
    }

  gtk_widget_show (button->button);
  gtk_widget_show (button->line1);
  gtk_widget_show (button->line2);
}

static void
picman_chain_button_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  PicmanChainButton *button = PICMAN_CHAIN_BUTTON (object);

  switch (property_id)
    {
    case PROP_POSITION:
      button->position = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_chain_button_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  PicmanChainButton *button = PICMAN_CHAIN_BUTTON (object);

  switch (property_id)
    {
    case PROP_POSITION:
      g_value_set_enum (value, button->position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * picman_chain_button_new:
 * @position: The position you are going to use for the button
 *            with respect to the widgets you want to chain.
 *
 * Creates a new #PicmanChainButton widget.
 *
 * This returns a button showing either a broken or a linked chain and
 * small clamps attached to both sides that visually group the two widgets
 * you want to connect. This widget looks best when attached
 * to a table taking up two columns (or rows respectively) next
 * to the widgets that it is supposed to connect. It may work
 * for more than two widgets, but the look is optimized for two.
 *
 * Returns: Pointer to the new #PicmanChainButton, which is inactive
 *          by default. Use picman_chain_button_set_active() to
 *          change its state.
 */
GtkWidget *
picman_chain_button_new (PicmanChainPosition position)
{
  return g_object_new (PICMAN_TYPE_CHAIN_BUTTON,
                       "position", position,
                       NULL);
}

/**
 * picman_chain_button_set_active:
 * @button: Pointer to a #PicmanChainButton.
 * @active: The new state.
 *
 * Sets the state of the #PicmanChainButton to be either locked (%TRUE) or
 * unlocked (%FALSE) and changes the showed pixmap to reflect the new state.
 */
void
picman_chain_button_set_active (PicmanChainButton  *button,
                              gboolean          active)
{
  g_return_if_fail (PICMAN_IS_CHAIN_BUTTON (button));

  if (button->active != active)
    {
      button->active = active ? TRUE : FALSE;

      picman_chain_button_update_image (button);
    }
}

/**
 * picman_chain_button_get_active
 * @button: Pointer to a #PicmanChainButton.
 *
 * Checks the state of the #PicmanChainButton.
 *
 * Returns: %TRUE if the #PicmanChainButton is active (locked).
 */
gboolean
picman_chain_button_get_active (PicmanChainButton *button)
{
  g_return_val_if_fail (PICMAN_IS_CHAIN_BUTTON (button), FALSE);

  return button->active;
}

static void
picman_chain_button_clicked_callback (GtkWidget       *widget,
                                    PicmanChainButton *button)
{
  g_return_if_fail (PICMAN_IS_CHAIN_BUTTON (button));

  picman_chain_button_set_active (button, ! button->active);

  g_signal_emit (button, picman_chain_button_signals[TOGGLED], 0);
}

static void
picman_chain_button_update_image (PicmanChainButton *button)
{
  guint i;

  i = ((button->position & PICMAN_CHAIN_LEFT) << 1) + (button->active ? 0 : 1);

  gtk_image_set_from_stock (GTK_IMAGE (button->image),
                            picman_chain_stock_items[i],
                            GTK_ICON_SIZE_BUTTON);
}


/* PicmanChainLine is a simple no-window widget for drawing the lines.
 *
 * Originally this used to be a GtkDrawingArea but this turned out to
 * be a bad idea. We don't need an extra window to draw on and we also
 * don't need any input events.
 */

static GType     picman_chain_line_get_type     (void) G_GNUC_CONST;
static gboolean  picman_chain_line_expose_event (GtkWidget       *widget,
                                               GdkEventExpose  *event);

struct _PicmanChainLine
{
  GtkWidget          parent_instance;
  PicmanChainPosition  position;
  gint               which;
};

typedef struct _PicmanChainLine  PicmanChainLine;
typedef GtkWidgetClass         PicmanChainLineClass;

G_DEFINE_TYPE (PicmanChainLine, picman_chain_line, GTK_TYPE_WIDGET)

static void
picman_chain_line_class_init (PicmanChainLineClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->expose_event = picman_chain_line_expose_event;
}

static void
picman_chain_line_init (PicmanChainLine *line)
{
  gtk_widget_set_has_window (GTK_WIDGET (line), FALSE);
}

static GtkWidget *
picman_chain_line_new (PicmanChainPosition  position,
                     gint               which)
{
  PicmanChainLine *line = g_object_new (picman_chain_line_get_type (), NULL);

  line->position = position;
  line->which    = which;

  return GTK_WIDGET (line);
}

static gboolean
picman_chain_line_expose_event (GtkWidget      *widget,
                              GdkEventExpose *event)
{
  GtkStyle          *style = gtk_widget_get_style (widget);
  PicmanChainLine     *line  = ((PicmanChainLine *) widget);
  GtkAllocation      allocation;
  GdkPoint           points[3];
  PicmanChainPosition  position;
  cairo_t           *cr;

  gtk_widget_get_allocation (widget, &allocation);

  cr = gdk_cairo_create (gtk_widget_get_window (widget));
  gdk_cairo_region (cr, event->region);
  cairo_translate (cr, allocation.x, allocation.y);
  cairo_clip (cr);

#define SHORT_LINE 4
  points[0].x = allocation.width  / 2;
  points[0].y = allocation.height / 2;

  position = line->position;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    {
      switch (position)
        {
        case PICMAN_CHAIN_TOP:
        case PICMAN_CHAIN_BOTTOM:
          break;

        case PICMAN_CHAIN_LEFT:
          position = PICMAN_CHAIN_RIGHT;
          break;

        case PICMAN_CHAIN_RIGHT:
          position = PICMAN_CHAIN_LEFT;
          break;
        }
    }

  switch (position)
    {
    case PICMAN_CHAIN_LEFT:
      points[0].x += SHORT_LINE;
      points[1].x = points[0].x - SHORT_LINE;
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = (line->which == 1 ? allocation.height - 1 : 0);
      break;

    case PICMAN_CHAIN_RIGHT:
      points[0].x -= SHORT_LINE;
      points[1].x = points[0].x + SHORT_LINE;
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = (line->which == 1 ? allocation.height - 1 : 0);
      break;

    case PICMAN_CHAIN_TOP:
      points[0].y += SHORT_LINE;
      points[1].x = points[0].x;
      points[1].y = points[0].y - SHORT_LINE;
      points[2].x = (line->which == 1 ? allocation.width - 1 : 0);
      points[2].y = points[1].y;
      break;

    case PICMAN_CHAIN_BOTTOM:
      points[0].y -= SHORT_LINE;
      points[1].x = points[0].x;
      points[1].y = points[0].y + SHORT_LINE;
      points[2].x = (line->which == 1 ? allocation.width - 1 : 0);
      points[2].y = points[1].y;
      break;

    default:
      return FALSE;
    }

  cairo_move_to (cr, points[0].x, points[0].y);
  cairo_line_to (cr, points[1].x, points[1].y);
  cairo_line_to (cr, points[2].x, points[2].y);

  cairo_set_line_width (cr, 2.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
  gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_NORMAL]);

  cairo_stroke (cr);

  cairo_destroy (cr);

  return TRUE;
}
