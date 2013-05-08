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

#include "libpicmanmath/picmanmath.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmanhistogram.h"

#include "picmancolorbar.h"
#include "picmanhandlebar.h"
#include "picmanhistogrambox.h"
#include "picmanhistogramview.h"

#include "picman-intl.h"


/*  #define DEBUG_VIEW  */

#define GRADIENT_HEIGHT  12
#define CONTROL_HEIGHT   10


/*  local function prototypes  */

static void   picman_histogram_box_low_adj_update  (GtkAdjustment     *adj,
                                                  PicmanHistogramBox  *box);
static void   picman_histogram_box_high_adj_update (GtkAdjustment     *adj,
                                                  PicmanHistogramBox  *box);
static void   picman_histogram_box_histogram_range (PicmanHistogramView *view,
                                                  gint               start,
                                                  gint               end,
                                                  PicmanHistogramBox  *box);
static void   picman_histogram_box_channel_notify  (PicmanHistogramView *view,
                                                  GParamSpec        *pspec,
                                                  PicmanHistogramBox  *box);
static void   picman_histogram_box_border_notify   (PicmanHistogramView *view,
                                                  GParamSpec        *pspec,
                                                  PicmanHistogramBox  *box);


G_DEFINE_TYPE (PicmanHistogramBox, picman_histogram_box, GTK_TYPE_BOX)


static void
picman_histogram_box_class_init (PicmanHistogramBoxClass *klass)
{
}

static void
picman_histogram_box_init (PicmanHistogramBox *box)
{
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *spinbutton;
  GtkObject *adjustment;
  GtkWidget *frame;
  GtkWidget *view;
  GtkWidget *bar;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (box),
                                  GTK_ORIENTATION_VERTICAL);

  gtk_box_set_spacing (GTK_BOX (box), 2);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  /*  The histogram  */
  view = picman_histogram_view_new (TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), view, TRUE, TRUE, 0);
  gtk_widget_show (view);

  g_signal_connect (view, "range-changed",
                    G_CALLBACK (picman_histogram_box_histogram_range),
                    box);

  box->view = PICMAN_HISTOGRAM_VIEW (view);

  /*  The gradient below the histogram */
  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2),
                                  PICMAN_HISTOGRAM_VIEW (view)->border_width);
  gtk_box_pack_start (GTK_BOX (vbox), vbox2, FALSE, FALSE, 0);
  gtk_widget_show (vbox2);

  box->color_bar = bar = g_object_new (PICMAN_TYPE_COLOR_BAR,
                                       "histogram-channel", box->view->channel,
                                       NULL);
  gtk_widget_set_size_request (bar, -1, GRADIENT_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox2), bar, FALSE, FALSE, 0);
  gtk_widget_show (bar);

  g_signal_connect (view, "notify::histogram-channel",
                    G_CALLBACK (picman_histogram_box_channel_notify),
                    box);
  g_signal_connect (view, "notify::border-width",
                    G_CALLBACK (picman_histogram_box_border_notify),
                    box);

  box->slider_bar = bar = g_object_new (PICMAN_TYPE_HANDLE_BAR, NULL);
  gtk_widget_set_size_request (bar, -1, CONTROL_HEIGHT);
  gtk_box_pack_start (GTK_BOX (vbox2), bar, FALSE, FALSE, 0);
  gtk_widget_show (bar);

  g_signal_connect_swapped (box->color_bar, "button-press-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (box->slider_bar)->button_press_event),
                            box->slider_bar);

  g_signal_connect_swapped (box->color_bar, "button-release-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (box->slider_bar)->button_release_event),
                            box->slider_bar);

  g_signal_connect_swapped (box->color_bar, "motion-notify-event",
                            G_CALLBACK (GTK_WIDGET_GET_CLASS (box->slider_bar)->motion_notify_event),
                            box->slider_bar);

  /*  The range selection */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  /*  low spinbutton  */
  spinbutton = picman_spin_button_new (&adjustment,
                                     0.0, 0.0, 255.0, 1.0, 16.0, 0.0,
                                     1.0, 0);
  box->low_adj = GTK_ADJUSTMENT (adjustment);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_histogram_box_low_adj_update),
                    box);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (bar), 0,
                                  GTK_ADJUSTMENT (adjustment));

  /*  high spinbutton  */
  spinbutton = picman_spin_button_new (&adjustment,
                                     255.0, 0.0, 255.0, 1.0, 16.0, 0.0,
                                     1.0, 0);
  box->high_adj = GTK_ADJUSTMENT (adjustment);
  gtk_box_pack_end (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  g_signal_connect (adjustment, "value-changed",
                    G_CALLBACK (picman_histogram_box_high_adj_update),
                    box);

  picman_handle_bar_set_adjustment (PICMAN_HANDLE_BAR (bar), 2,
                                  GTK_ADJUSTMENT (adjustment));

#ifdef DEBUG_VIEW
  spinbutton = picman_prop_spin_button_new (G_OBJECT (box->view), "border-width",
                                          1, 5, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);

  spinbutton = picman_prop_spin_button_new (G_OBJECT (box->view), "subdivisions",
                                          1, 5, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, FALSE, FALSE, 0);
  gtk_widget_show (spinbutton);
#endif
}

static void
picman_histogram_box_low_adj_update (GtkAdjustment    *adjustment,
                                   PicmanHistogramBox *box)
{
  gint value = ROUND (gtk_adjustment_get_value (adjustment));

  if (box->view->start != value)
    {
      gtk_adjustment_set_lower (box->high_adj, value);

      picman_histogram_view_set_range (box->view, value, box->view->end);
    }
}

static void
picman_histogram_box_high_adj_update (GtkAdjustment    *adjustment,
                                    PicmanHistogramBox *box)
{
  gint value = ROUND (gtk_adjustment_get_value (adjustment));

  if (box->view->end != value)
    {
      gtk_adjustment_set_upper (box->low_adj, value);

      picman_histogram_view_set_range (box->view, box->view->start, value);
    }
}

static void
picman_histogram_box_histogram_range (PicmanHistogramView *view,
                                    gint               start,
                                    gint               end,
                                    PicmanHistogramBox  *box)
{
  gtk_adjustment_set_lower (box->high_adj, start);
  gtk_adjustment_set_upper (box->low_adj,  end);

  gtk_adjustment_set_value (box->low_adj,  start);
  gtk_adjustment_set_value (box->high_adj, end);
}

static void
picman_histogram_box_channel_notify (PicmanHistogramView *view,
                                   GParamSpec        *pspec,
                                   PicmanHistogramBox  *box)
{
  picman_color_bar_set_channel (PICMAN_COLOR_BAR (box->color_bar), view->channel);
}

static void
picman_histogram_box_border_notify (PicmanHistogramView *view,
                                  GParamSpec        *pspec,
                                  PicmanHistogramBox  *box)
{
  GtkWidget *vbox = gtk_widget_get_parent (box->color_bar);

  gtk_container_set_border_width (GTK_CONTAINER (vbox), view->border_width);
}


/*  public functions  */

GtkWidget *
picman_histogram_box_new (void)
{
  return g_object_new (PICMAN_TYPE_HISTOGRAM_BOX, NULL);
}

void
picman_histogram_box_set_channel (PicmanHistogramBox     *box,
                                PicmanHistogramChannel  channel)
{
  g_return_if_fail (PICMAN_IS_HISTOGRAM_BOX (box));

  if (box->view)
    picman_histogram_view_set_channel (box->view, channel);
}
