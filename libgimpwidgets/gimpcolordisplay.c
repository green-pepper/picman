/* LIBPICMAN - The PICMAN Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * picmancolordisplay.c
 * Copyright (C) 2002 Michael Natterer <mitch@picman.org>
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

#include <gegl.h>
#include <gtk/gtk.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanconfig/picmanconfig.h"

#include "picmanwidgetstypes.h"

#include "picmanstock.h"

#undef PICMAN_DISABLE_DEPRECATED
#include "picmancolordisplay.h"


/**
 * SECTION: picmancolordisplay
 * @title: PicmanColorDisplay
 * @short_description: Pluggable PICMAN display color correction modules.
 * @see_also: #GModule, #GTypeModule, #PicmanModule
 *
 * Functions and definitions for creating pluggable PICMAN
 * display color correction modules.
 **/


enum
{
  PROP_0,
  PROP_ENABLED,
  PROP_COLOR_CONFIG,
  PROP_COLOR_MANAGED
};

enum
{
  CHANGED,
  LAST_SIGNAL
};


typedef struct
{
  PicmanColorConfig  *config;
  PicmanColorManaged *managed;
} PicmanColorDisplayPrivate;

#define PICMAN_COLOR_DISPLAY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), PICMAN_TYPE_COLOR_DISPLAY, PicmanColorDisplayPrivate))


static void       picman_color_display_constructed (GObject       *object);
static void       picman_color_display_dispose      (GObject      *object);
static void       picman_color_display_set_property (GObject      *object,
                                                   guint         property_id,
                                                   const GValue *value,
                                                  GParamSpec    *pspec);
static void       picman_color_display_get_property (GObject      *object,
                                                   guint         property_id,
                                                   GValue       *value,
                                                   GParamSpec   *pspec);

static void  picman_color_display_set_color_config  (PicmanColorDisplay *display,
                                                   PicmanColorConfig  *config);
static void  picman_color_display_set_color_managed (PicmanColorDisplay *display,
                                                   PicmanColorManaged *managed);


G_DEFINE_TYPE_WITH_CODE (PicmanColorDisplay, picman_color_display, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_CONFIG, NULL))

#define parent_class picman_color_display_parent_class

static guint display_signals[LAST_SIGNAL] = { 0 };


static void
picman_color_display_class_init (PicmanColorDisplayClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = picman_color_display_constructed;
  object_class->dispose      = picman_color_display_dispose;
  object_class->set_property = picman_color_display_set_property;
  object_class->get_property = picman_color_display_get_property;

  g_type_class_add_private (object_class, sizeof (PicmanColorDisplayPrivate));

  g_object_class_install_property (object_class, PROP_ENABLED,
                                   g_param_spec_boolean ("enabled", NULL, NULL,
                                                         TRUE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));
  g_object_class_install_property (object_class, PROP_COLOR_CONFIG,
                                   g_param_spec_object ("color-config",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_COLOR_CONFIG,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_COLOR_MANAGED,
                                   g_param_spec_object ("color-managed",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_COLOR_MANAGED,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
  display_signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanColorDisplayClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  klass->name            = "Unnamed";
  klass->help_id         = NULL;
  klass->stock_id        = PICMAN_STOCK_DISPLAY_FILTER;

  klass->clone           = NULL;
  klass->convert_surface = NULL;
  klass->convert         = NULL;
  klass->load_state      = NULL;
  klass->save_state      = NULL;
  klass->configure       = NULL;
  klass->configure_reset = NULL;
  klass->changed         = NULL;
}

static void
picman_color_display_init (PicmanColorDisplay *display)
{
  display->enabled = FALSE;
}

static void
picman_color_display_constructed (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->constructed (object);

  /* emit an initial "changed" signal after all construct properties are set */
  picman_color_display_changed (PICMAN_COLOR_DISPLAY (object));
}

static void
picman_color_display_dispose (GObject *object)
{
  PicmanColorDisplayPrivate *private = PICMAN_COLOR_DISPLAY_GET_PRIVATE (object);

  if (private->config)
    {
      g_signal_handlers_disconnect_by_func (private->config,
                                            picman_color_display_changed,
                                            object);
      g_object_unref (private->config);
      private->config = NULL;
    }

  if (private->managed)
    {
      g_signal_handlers_disconnect_by_func (private->managed,
                                            picman_color_display_changed,
                                            object);
      g_object_unref (private->managed);
      private->managed = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_color_display_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  PicmanColorDisplay *display = PICMAN_COLOR_DISPLAY (object);

  switch (property_id)
    {
    case PROP_ENABLED:
      display->enabled = g_value_get_boolean (value);
      break;

    case PROP_COLOR_CONFIG:
      picman_color_display_set_color_config (display,
                                           g_value_get_object (value));
      break;

    case PROP_COLOR_MANAGED:
      picman_color_display_set_color_managed (display,
                                            g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_display_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  PicmanColorDisplay *display = PICMAN_COLOR_DISPLAY (object);

  switch (property_id)
    {
    case PROP_ENABLED:
      g_value_set_boolean (value, display->enabled);
      break;

    case PROP_COLOR_CONFIG:
      g_value_set_object (value,
                          PICMAN_COLOR_DISPLAY_GET_PRIVATE (display)->config);
      break;

    case PROP_COLOR_MANAGED:
      g_value_set_object (value,
                          PICMAN_COLOR_DISPLAY_GET_PRIVATE (display)->managed);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_color_display_set_color_config (PicmanColorDisplay *display,
                                     PicmanColorConfig  *config)
{
  PicmanColorDisplayPrivate *private = PICMAN_COLOR_DISPLAY_GET_PRIVATE (display);

  g_return_if_fail (private->config == NULL);

  if (config)
    {
      private->config = g_object_ref (config);

      g_signal_connect_swapped (private->config, "notify",
                                G_CALLBACK (picman_color_display_changed),
                                display);
    }
}

static void
picman_color_display_set_color_managed (PicmanColorDisplay *display,
                                      PicmanColorManaged *managed)
{
  PicmanColorDisplayPrivate *private = PICMAN_COLOR_DISPLAY_GET_PRIVATE (display);

  g_return_if_fail (private->managed == NULL);

  if (managed)
    {
      private->managed = g_object_ref (managed);

      g_signal_connect_swapped (private->managed, "profile-changed",
                                G_CALLBACK (picman_color_display_changed),
                                display);
    }
}

/**
 * picman_color_display_new:
 * @display_type: the GType of the PicmanColorDisplay to instantiate.
 *
 * This function is deprecated. Please use g_object_new() directly.
 *
 * Return value: a new %PicmanColorDisplay object.
 **/
PicmanColorDisplay *
picman_color_display_new (GType display_type)
{
  g_return_val_if_fail (g_type_is_a (display_type, PICMAN_TYPE_COLOR_DISPLAY),
                        NULL);

  return g_object_new (display_type, NULL);
}

PicmanColorDisplay *
picman_color_display_clone (PicmanColorDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), NULL);

  /*  implementing the clone method is deprecated
   */
  if (PICMAN_COLOR_DISPLAY_GET_CLASS (display)->clone)
    {
      PicmanColorDisplay *clone;

      clone = PICMAN_COLOR_DISPLAY_GET_CLASS (display)->clone (display);

      if (clone)
        {
          PicmanColorDisplayPrivate *private;

          private = PICMAN_COLOR_DISPLAY_GET_PRIVATE (display);

          g_object_set (clone,
                        "enabled",       display->enabled,
                        "color-managed", private->managed,
                        NULL);
        }

      return clone;
    }

  return PICMAN_COLOR_DISPLAY (picman_config_duplicate (PICMAN_CONFIG (display)));
}

/**
 * picman_color_display_convert_surface:
 * @display: a #PicmanColorDisplay
 * @surface: a #cairo_image_surface_t of type ARGB32
 *
 * Converts all pixels in @surface.
 *
 * Since: PICMAN 2.8
 **/
void
picman_color_display_convert_surface (PicmanColorDisplay *display,
                                    cairo_surface_t  *surface)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));
  g_return_if_fail (surface != NULL);
  g_return_if_fail (cairo_surface_get_type (surface) ==
                    CAIRO_SURFACE_TYPE_IMAGE);

  if (display->enabled &&
      PICMAN_COLOR_DISPLAY_GET_CLASS (display)->convert_surface)
    {
      cairo_surface_flush (surface);
      PICMAN_COLOR_DISPLAY_GET_CLASS (display)->convert_surface (display, surface);
      cairo_surface_mark_dirty (surface);
    }
}

/**
 * picman_color_display_convert:
 * @display: a #PicmanColorDisplay
 * @buf: the pixel buffer to convert
 * @width: the width of the buffer
 * @height: the height of the buffer
 * @bpp: the number of bytes per pixel
 * @bpl: the buffer's rowstride
 *
 * Converts all pixels in @buf.
 *
 * Deprecated: PICMAN 2.8: Use picman_color_display_convert_surface() instead.
 **/
void
picman_color_display_convert (PicmanColorDisplay *display,
                            guchar            *buf,
                            gint               width,
                            gint               height,
                            gint               bpp,
                            gint               bpl)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  /*  implementing the convert method is deprecated
   */
  if (display->enabled && PICMAN_COLOR_DISPLAY_GET_CLASS (display)->convert)
    PICMAN_COLOR_DISPLAY_GET_CLASS (display)->convert (display, buf,
                                                     width, height,
                                                     bpp, bpl);
}

void
picman_color_display_load_state (PicmanColorDisplay *display,
                               PicmanParasite     *state)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));
  g_return_if_fail (state != NULL);

  /*  implementing the load_state method is deprecated
   */
  if (PICMAN_COLOR_DISPLAY_GET_CLASS (display)->load_state)
    {
      PICMAN_COLOR_DISPLAY_GET_CLASS (display)->load_state (display, state);
    }
  else
    {
      picman_config_deserialize_string (PICMAN_CONFIG (display),
                                      picman_parasite_data (state),
                                      picman_parasite_data_size (state),
                                      NULL, NULL);
    }
}

PicmanParasite *
picman_color_display_save_state (PicmanColorDisplay *display)
{
  PicmanParasite *parasite;
  gchar        *str;

  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), NULL);

  /*  implementing the save_state method is deprecated
   */
  if (PICMAN_COLOR_DISPLAY_GET_CLASS (display)->save_state)
    {
      return PICMAN_COLOR_DISPLAY_GET_CLASS (display)->save_state (display);
    }

  str = picman_config_serialize_to_string (PICMAN_CONFIG (display), NULL);

  parasite = picman_parasite_new ("Display/Proof",
                                PICMAN_PARASITE_PERSISTENT,
                                strlen (str) + 1, str);
  g_free (str);

  return parasite;
}

GtkWidget *
picman_color_display_configure (PicmanColorDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), NULL);

  if (PICMAN_COLOR_DISPLAY_GET_CLASS (display)->configure)
    return PICMAN_COLOR_DISPLAY_GET_CLASS (display)->configure (display);

  return NULL;
}

void
picman_color_display_configure_reset (PicmanColorDisplay *display)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  /*  implementing the configure_reset method is deprecated
   */
  if (PICMAN_COLOR_DISPLAY_GET_CLASS (display)->configure_reset)
    {
      PICMAN_COLOR_DISPLAY_GET_CLASS (display)->configure_reset (display);
    }
  else
    {
      picman_config_reset (PICMAN_CONFIG (display));
    }
}

void
picman_color_display_changed (PicmanColorDisplay *display)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  g_signal_emit (display, display_signals[CHANGED], 0);
}

void
picman_color_display_set_enabled (PicmanColorDisplay *display,
                                gboolean          enabled)
{
  g_return_if_fail (PICMAN_IS_COLOR_DISPLAY (display));

  if (enabled != display->enabled)
    {
      g_object_set (display,
                    "enabled", enabled,
                    NULL);
    }
}

gboolean
picman_color_display_get_enabled (PicmanColorDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), FALSE);

  return display->enabled;
}

/**
 * picman_color_display_get_config:
 * @display:
 *
 * Return value: a pointer to the #PicmanColorConfig object or %NULL.
 *
 * Since: PICMAN 2.4
 **/
PicmanColorConfig *
picman_color_display_get_config (PicmanColorDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), NULL);

  return PICMAN_COLOR_DISPLAY_GET_PRIVATE (display)->config;
}

/**
 * picman_color_display_get_managed:
 * @display:
 *
 * Return value: a pointer to the #PicmanColorManaged object or %NULL.
 *
 * Since: PICMAN 2.4
 **/
PicmanColorManaged *
picman_color_display_get_managed (PicmanColorDisplay *display)
{
  g_return_val_if_fail (PICMAN_IS_COLOR_DISPLAY (display), NULL);

  return PICMAN_COLOR_DISPLAY_GET_PRIVATE (display)->managed;
}
