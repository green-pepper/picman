/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationpointlayermode.c
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
 * Copyright (C) 2008 Martin Nordholts <martinn@svn.gnome.org>
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

#include <cairo.h>
#include <gegl-plugin.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "operations-types.h"

#include "picmanoperationpointlayermode.h"


enum
{
  PROP_0,
  PROP_LINEAR,
  PROP_OPACITY
};


static void     picman_operation_point_layer_mode_set_property (GObject       *object,
                                                              guint          property_id,
                                                              const GValue  *value,
                                                              GParamSpec    *pspec);
static void     picman_operation_point_layer_mode_get_property (GObject       *object,
                                                              guint          property_id,
                                                              GValue        *value,
                                                              GParamSpec    *pspec);

static void     picman_operation_point_layer_mode_prepare      (GeglOperation *operation);


G_DEFINE_TYPE (PicmanOperationPointLayerMode, picman_operation_point_layer_mode,
               GEGL_TYPE_OPERATION_POINT_COMPOSER3)


static void
picman_operation_point_layer_mode_class_init (PicmanOperationPointLayerModeClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);

  object_class->set_property = picman_operation_point_layer_mode_set_property;
  object_class->get_property = picman_operation_point_layer_mode_get_property;

  operation_class->prepare   = picman_operation_point_layer_mode_prepare;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:point-layer-mode",
                                 "description", "PICMAN point layer mode operation",
                                 "categories",  "compositors",
                                 NULL);

  g_object_class_install_property (object_class, PROP_LINEAR,
                                   g_param_spec_boolean ("linear",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT));

  g_object_class_install_property (object_class, PROP_OPACITY,
                                   g_param_spec_double ("opacity",
                                                        NULL, NULL,
                                                        0.0, 1.0, 1.0,
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_operation_point_layer_mode_init (PicmanOperationPointLayerMode *self)
{
}

static void
picman_operation_point_layer_mode_set_property (GObject      *object,
                                              guint         property_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
  PicmanOperationPointLayerMode *self = PICMAN_OPERATION_POINT_LAYER_MODE (object);

  switch (property_id)
    {
    case PROP_LINEAR:
      self->linear = g_value_get_boolean (value);
      break;

    case PROP_OPACITY:
      self->opacity = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_point_layer_mode_get_property (GObject    *object,
                                              guint       property_id,
                                              GValue     *value,
                                              GParamSpec *pspec)
{
  PicmanOperationPointLayerMode *self = PICMAN_OPERATION_POINT_LAYER_MODE (object);

  switch (property_id)
    {
    case PROP_LINEAR:
      g_value_set_boolean (value, self->linear);
      break;

    case PROP_OPACITY:
      g_value_set_double (value, self->opacity);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_point_layer_mode_prepare (GeglOperation *operation)
{
  PicmanOperationPointLayerMode *self = PICMAN_OPERATION_POINT_LAYER_MODE (operation);
  const Babl                  *format;

  if (self->linear)
    format = babl_format ("RGBA float");
  else
    format = babl_format ("R'G'B'A float");

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "output", format);
  gegl_operation_set_format (operation, "aux",    format);
  gegl_operation_set_format (operation, "aux2",   babl_format ("Y float"));
}
