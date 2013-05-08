/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanfgbgview.c
 * Copyright (C) 2005  Sven Neumann <sven@picman.org>
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

#include "libpicmancolor/picmancolor.h"
#include "libpicmanwidgets/picmanwidgets.h"

#include "widgets-types.h"

#include "core/picmancontext.h"
#include "core/picmanmarshal.h"

#include "picmandnd.h"
#include "picmanfgbgview.h"


enum
{
  PROP_0,
  PROP_CONTEXT
};


static void     picman_fg_bg_view_dispose      (GObject        *object);
static void     picman_fg_bg_view_set_property (GObject        *object,
                                              guint           property_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec);
static void     picman_fg_bg_view_get_property (GObject        *object,
                                              guint           property_id,
                                              GValue         *value,
                                              GParamSpec     *pspec);

static gboolean picman_fg_bg_view_expose       (GtkWidget      *widget,
                                              GdkEventExpose *eevent);


G_DEFINE_TYPE (PicmanFgBgView, picman_fg_bg_view, GTK_TYPE_WIDGET)

#define parent_class picman_fg_bg_view_parent_class


static void
picman_fg_bg_view_class_init (PicmanFgBgViewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose      = picman_fg_bg_view_dispose;
  object_class->set_property = picman_fg_bg_view_set_property;
  object_class->get_property = picman_fg_bg_view_get_property;

  widget_class->expose_event = picman_fg_bg_view_expose;

  g_object_class_install_property (object_class, PROP_CONTEXT,
                                   g_param_spec_object ("context",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_CONTEXT,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_fg_bg_view_init (PicmanFgBgView *view)
{
  gtk_widget_set_has_window (GTK_WIDGET (view), FALSE);

  view->context = NULL;
}

static void
picman_fg_bg_view_dispose (GObject *object)
{
  PicmanFgBgView *view = PICMAN_FG_BG_VIEW (object);

  if (view->context)
    picman_fg_bg_view_set_context (view, NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_fg_bg_view_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanFgBgView *view = PICMAN_FG_BG_VIEW (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      picman_fg_bg_view_set_context (view, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_fg_bg_view_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanFgBgView *view = PICMAN_FG_BG_VIEW (object);

  switch (property_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, view->context);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_fg_bg_view_expose (GtkWidget      *widget,
                        GdkEventExpose *eevent)
{
  PicmanFgBgView *view   = PICMAN_FG_BG_VIEW (widget);
  GtkStyle     *style  = gtk_widget_get_style (widget);
  GdkWindow    *window = gtk_widget_get_window (widget);
  cairo_t      *cr;
  GtkAllocation allocation;
  gint          rect_w, rect_h;
  PicmanRGB       color;

  if (! gtk_widget_is_drawable (widget))
    return FALSE;

  cr = gdk_cairo_create (eevent->window);

  gdk_cairo_region (cr, eevent->region);
  cairo_clip (cr);

  gtk_widget_get_allocation (widget, &allocation);

  cairo_translate (cr, allocation.x, allocation.y);

  rect_w = allocation.width  * 3 / 4;
  rect_h = allocation.height * 3 / 4;

  /*  draw the background area  */

  if (view->context)
    {
      picman_context_get_background (view->context, &color);
      picman_cairo_set_source_rgb (cr, &color);

      cairo_rectangle (cr,
                       allocation.width  - rect_w + 1,
                       allocation.height - rect_h + 1,
                       rect_w - 2,
                       rect_h - 2);
      cairo_fill (cr);
    }

  gtk_paint_shadow (style, window, GTK_STATE_NORMAL,
                    GTK_SHADOW_IN,
                    NULL, widget, NULL,
                    allocation.x + allocation.width  - rect_w,
                    allocation.y + allocation.height - rect_h,
                    rect_w, rect_h);

  /*  draw the foreground area  */

  if (view->context)
    {
      picman_context_get_foreground (view->context, &color);
      picman_cairo_set_source_rgb (cr, &color);

      cairo_rectangle (cr, 1, 1, rect_w - 2, rect_h - 2);
      cairo_fill (cr);
    }

  gtk_paint_shadow (style, window, GTK_STATE_NORMAL,
                    GTK_SHADOW_OUT,
                    NULL, widget, NULL,
                    allocation.x, allocation.y, rect_w, rect_h);

  cairo_destroy (cr);

  return TRUE;
}

/*  public functions  */

GtkWidget *
picman_fg_bg_view_new (PicmanContext *context)
{
  g_return_val_if_fail (context == NULL || PICMAN_IS_CONTEXT (context), NULL);

  return g_object_new (PICMAN_TYPE_FG_BG_VIEW,
                       "context", context,
                       NULL);
}

void
picman_fg_bg_view_set_context (PicmanFgBgView *view,
                             PicmanContext  *context)
{
  g_return_if_fail (PICMAN_IS_FG_BG_VIEW (view));
  g_return_if_fail (context == NULL || PICMAN_IS_CONTEXT (context));

  if (context == view->context)
    return;

  if (view->context)
    {
      g_signal_handlers_disconnect_by_func (view->context,
                                            gtk_widget_queue_draw,
                                            view);
      g_object_unref (view->context);
      view->context = NULL;
    }

  view->context = context;

  if (context)
    {
      g_object_ref (context);

      g_signal_connect_swapped (context, "foreground-changed",
                                G_CALLBACK (gtk_widget_queue_draw),
                                view);
      g_signal_connect_swapped (context, "background-changed",
                                G_CALLBACK (gtk_widget_queue_draw),
                                view);
    }

  g_object_notify (G_OBJECT (view), "context");
}
