/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolorselector.c
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
 *
 * based on:
 * Colour selector module
 * Copyright (C) 1999 Austin Donnelly <austin@greenend.org.uk>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "picmanwidgetstypes.h"

#include "picmancolorselector.h"
#include "picmanwidgetsmarshal.h"


/**
 * SECTION: picmancolorselector
 * @title: PicmanColorSelector
 * @short_description: Pluggable PICMAN color selector modules.
 * @see_also: #GModule, #GTypeModule, #PicmanModule
 *
 * Functions and definitions for creating pluggable PICMAN color
 * selector modules.
 **/


enum
{
  COLOR_CHANGED,
  CHANNEL_CHANGED,
  LAST_SIGNAL
};


static void   picman_color_selector_dispose (GObject *object);


G_DEFINE_TYPE (PicmanColorSelector, picman_color_selector, GTK_TYPE_BOX)

#define parent_class picman_color_selector_parent_class

static guint selector_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_selector_class_init (PicmanColorSelectorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = picman_color_selector_dispose;

  selector_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorSelectorClass, color_changed),
                  NULL, NULL,
                  _picman_widgets_marshal_VOID__POINTER_POINTER,
                  G_TYPE_NONE, 2,
                  G_TYPE_POINTER,
                  G_TYPE_POINTER);

  selector_signals[CHANNEL_CHANGED] =
    g_signal_new ("channel-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorSelectorClass, channel_changed),
                  NULL, NULL,
                  _picman_widgets_marshal_VOID__INT,
                  G_TYPE_NONE, 1,
                  G_TYPE_INT);

  klass->name                  = "Unnamed";
  klass->help_id               = NULL;
  klass->stock_id              = GTK_STOCK_SELECT_COLOR;

  klass->set_toggles_visible   = NULL;
  klass->set_toggles_sensitive = NULL;
  klass->set_show_alpha        = NULL;
  klass->set_color             = NULL;
  klass->set_channel           = NULL;
  klass->color_changed         = NULL;
  klass->channel_changed       = NULL;
  klass->set_config            = NULL;
}

static void
picman_color_selector_init (PicmanColorSelector *selector)
{
  selector->toggles_visible   = TRUE;
  selector->toggles_sensitive = TRUE;
  selector->show_alpha        = TRUE;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (selector),
                                  GTK_ORIENTATION_VERTICAL);

  picman_rgba_set (&selector->rgb, 0.0, 0.0, 0.0, 1.0);
  picman_rgb_to_hsv (&selector->rgb, &selector->hsv);

  selector->channel = PICMAN_COLOR_SELECTOR_HUE;
}

static void
picman_color_selector_dispose (GObject *object)
{
  picman_color_selector_set_config (PICMAN_COLOR_SELECTOR (object), NULL);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

GtkWidget *
picman_color_selector_new (GType                     selector_type,
                         const PicmanRGB            *rgb,
                         const PicmanHSV            *hsv,
                         PicmanColorSelectorChannel  channel)
{
  PicmanColorSelector *selector;

  g_return_val_if_fail (g_type_is_a (selector_type, PICMAN_TYPE_COLOR_SELECTOR),
                        NULL);
  g_return_val_if_fail (rgb != NULL, NULL);
  g_return_val_if_fail (hsv != NULL, NULL);

  selector = g_object_new (selector_type, NULL);

  picman_color_selector_set_color (selector, rgb, hsv);
  picman_color_selector_set_channel (selector, channel);

  return GTK_WIDGET (selector);
}

void
picman_color_selector_set_toggles_visible (PicmanColorSelector *selector,
                                         gboolean           visible)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  if (selector->toggles_visible != visible)
    {
      PicmanColorSelectorClass *selector_class;

      selector->toggles_visible = visible ? TRUE : FALSE;

      selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

      if (selector_class->set_toggles_visible)
        selector_class->set_toggles_visible (selector, visible);
    }
}

void
picman_color_selector_set_toggles_sensitive (PicmanColorSelector *selector,
                                           gboolean           sensitive)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  if (selector->toggles_sensitive != sensitive)
    {
      PicmanColorSelectorClass *selector_class;

      selector->toggles_sensitive = sensitive ? TRUE : FALSE;

      selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

      if (selector_class->set_toggles_sensitive)
        selector_class->set_toggles_sensitive (selector, sensitive);
    }
}

void
picman_color_selector_set_show_alpha (PicmanColorSelector *selector,
                                    gboolean           show_alpha)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  if (show_alpha != selector->show_alpha)
    {
      PicmanColorSelectorClass *selector_class;

      selector->show_alpha = show_alpha ? TRUE : FALSE;

      selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

      if (selector_class->set_show_alpha)
        selector_class->set_show_alpha (selector, show_alpha);
    }
}

void
picman_color_selector_set_color (PicmanColorSelector *selector,
                               const PicmanRGB     *rgb,
                               const PicmanHSV     *hsv)
{
  PicmanColorSelectorClass *selector_class;

  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));
  g_return_if_fail (rgb != NULL);
  g_return_if_fail (hsv != NULL);

  selector->rgb = *rgb;
  selector->hsv = *hsv;

  selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

  if (selector_class->set_color)
    selector_class->set_color (selector, rgb, hsv);

  picman_color_selector_color_changed (selector);
}

void
picman_color_selector_set_channel (PicmanColorSelector        *selector,
                                 PicmanColorSelectorChannel  channel)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  if (channel != selector->channel)
    {
      PicmanColorSelectorClass *selector_class;

      selector->channel = channel;

      selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

      if (selector_class->set_channel)
        selector_class->set_channel (selector, channel);

      picman_color_selector_channel_changed (selector);
    }
}

void
picman_color_selector_color_changed (PicmanColorSelector *selector)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  g_signal_emit (selector, selector_signals[COLOR_CHANGED], 0,
                 &selector->rgb, &selector->hsv);
}

void
picman_color_selector_channel_changed (PicmanColorSelector *selector)
{
  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));

  g_signal_emit (selector, selector_signals[CHANNEL_CHANGED], 0,
                 selector->channel);
}

/**
 * picman_color_selector_set_config:
 * @selector: a #PicmanColorSelector widget.
 * @config:   a #PicmanColorConfig object.
 *
 * Sets the color management configuration to use with this color selector.
 *
 * Since: PICMAN 2.4
 */
void
picman_color_selector_set_config (PicmanColorSelector *selector,
                                PicmanColorConfig   *config)
{
  PicmanColorSelectorClass *selector_class;

  g_return_if_fail (PICMAN_IS_COLOR_SELECTOR (selector));
  g_return_if_fail (config == NULL || PICMAN_IS_COLOR_CONFIG (config));

  selector_class = PICMAN_COLOR_SELECTOR_GET_CLASS (selector);

  if (selector_class->set_config)
    selector_class->set_config (selector, config);
}
