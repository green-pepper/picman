/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationpointfilter.c
 * Copyright (C) 2007 Michael Natterer <mitch@picman.org>
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

#include "operations-types.h"

#include "picmanoperationpointfilter.h"


static void   picman_operation_point_filter_finalize (GObject       *object);
static void   picman_operation_point_filter_prepare  (GeglOperation *operation);


G_DEFINE_ABSTRACT_TYPE (PicmanOperationPointFilter, picman_operation_point_filter,
                        GEGL_TYPE_OPERATION_POINT_FILTER)

#define parent_class picman_operation_point_filter_parent_class


static void
picman_operation_point_filter_class_init (PicmanOperationPointFilterClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  GeglOperationClass  *operation_class = GEGL_OPERATION_CLASS (klass);

  object_class->finalize = picman_operation_point_filter_finalize;

  operation_class->prepare = picman_operation_point_filter_prepare;
}

static void
picman_operation_point_filter_init (PicmanOperationPointFilter *self)
{
}

static void
picman_operation_point_filter_finalize (GObject *object)
{
  PicmanOperationPointFilter *self = PICMAN_OPERATION_POINT_FILTER (object);

  if (self->config)
    {
      g_object_unref (self->config);
      self->config = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

void
picman_operation_point_filter_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  PicmanOperationPointFilter *self = PICMAN_OPERATION_POINT_FILTER (object);

  switch (property_id)
    {
    case PICMAN_OPERATION_POINT_FILTER_PROP_CONFIG:
      g_value_set_object (value, self->config);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

void
picman_operation_point_filter_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  PicmanOperationPointFilter *self = PICMAN_OPERATION_POINT_FILTER (object);

  switch (property_id)
    {
    case PICMAN_OPERATION_POINT_FILTER_PROP_CONFIG:
      if (self->config)
        g_object_unref (self->config);
      self->config = g_value_dup_object (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_point_filter_prepare (GeglOperation *operation)
{
  const Babl *format = babl_format ("R'G'B'A float");

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "output", format);
}
