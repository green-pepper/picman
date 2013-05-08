/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationmaskcomponents.c
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
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

#include "picmanoperationmaskcomponents.h"


enum
{
  PROP_0,
  PROP_MASK
};


static void       picman_operation_mask_components_get_property (GObject             *object,
                                                               guint                property_id,
                                                               GValue              *value,
                                                               GParamSpec          *pspec);
static void       picman_operation_mask_components_set_property (GObject             *object,
                                                               guint                property_id,
                                                               const GValue        *value,
                                                               GParamSpec          *pspec);

static void       picman_operation_mask_components_prepare      (GeglOperation       *operation);
static gboolean picman_operation_mask_components_parent_process (GeglOperation        *operation,
                                                               GeglOperationContext *context,
                                                               const gchar          *output_prop,
                                                               const GeglRectangle  *result,
                                                               gint                  level);
static gboolean   picman_operation_mask_components_process      (GeglOperation       *operation,
                                                               void                *in_buf,
                                                               void                *aux_buf,
                                                               void                *out_buf,
                                                               glong                samples,
                                                               const GeglRectangle *roi,
                                                               gint                 level);


G_DEFINE_TYPE (PicmanOperationMaskComponents, picman_operation_mask_components,
               GEGL_TYPE_OPERATION_POINT_COMPOSER)

#define parent_class picman_operation_mask_components_parent_class


static void
picman_operation_mask_components_class_init (PicmanOperationMaskComponentsClass *klass)
{
  GObjectClass                    *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationClass              *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointComposerClass *point_class     = GEGL_OPERATION_POINT_COMPOSER_CLASS (klass);

  object_class->set_property = picman_operation_mask_components_set_property;
  object_class->get_property = picman_operation_mask_components_get_property;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:mask-components",
                                 "categories",  "picman",
                                 "description", "Selectively pick components from src or aux",
                                 NULL);

  operation_class->prepare = picman_operation_mask_components_prepare;
  operation_class->process = picman_operation_mask_components_parent_process;

  point_class->process     = picman_operation_mask_components_process;

  g_object_class_install_property (object_class, PROP_MASK,
                                   g_param_spec_flags ("mask",
                                                       "Mask",
                                                       "The component mask",
                                                       PICMAN_TYPE_COMPONENT_MASK,
                                                       PICMAN_COMPONENT_ALL,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_CONSTRUCT));
}

static void
picman_operation_mask_components_init (PicmanOperationMaskComponents *self)
{
}

static void
picman_operation_mask_components_get_property (GObject    *object,
                                             guint       property_id,
                                             GValue     *value,
                                             GParamSpec *pspec)
{
  PicmanOperationMaskComponents *self = PICMAN_OPERATION_MASK_COMPONENTS (object);

  switch (property_id)
    {
    case PROP_MASK:
      g_value_set_flags (value, self->mask);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_mask_components_set_property (GObject      *object,
                                             guint         property_id,
                                             const GValue *value,
                                             GParamSpec   *pspec)
{
  PicmanOperationMaskComponents *self = PICMAN_OPERATION_MASK_COMPONENTS (object);

  switch (property_id)
    {
    case PROP_MASK:
      self->mask = g_value_get_flags (value);
      break;

   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_operation_mask_components_prepare (GeglOperation *operation)
{
  const Babl *format = gegl_operation_get_source_format (operation, "input");

  if (format)
    {
      const Babl *model = babl_format_get_model (format);

      if (model == babl_model ("R'G'B'A"))
        format = babl_format ("R'G'B'A float");
      else
        format = babl_format ("RGBA float");
    }
  else
    {
      format = babl_format ("RGBA float");
    }

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "aux",    format);
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
picman_operation_mask_components_parent_process (GeglOperation        *operation,
                                               GeglOperationContext *context,
                                               const gchar          *output_prop,
                                               const GeglRectangle  *result,
                                               gint                  level)
{
  PicmanOperationMaskComponents *self = PICMAN_OPERATION_MASK_COMPONENTS (operation);

  if (self->mask == 0)
    {
      GObject *input = gegl_operation_context_get_object (context, "input");

      gegl_operation_context_set_object (context, "output", input);

      return TRUE;
    }
  else if (self->mask == PICMAN_COMPONENT_ALL)
    {
      GObject *aux = gegl_operation_context_get_object (context, "aux");

      gegl_operation_context_set_object (context, "output", aux);

      return TRUE;
    }

  return GEGL_OPERATION_CLASS (parent_class)->process (operation, context,
                                                       output_prop, result,
                                                       level);
}

static gboolean
picman_operation_mask_components_process (GeglOperation       *operation,
                                        void                *in_buf,
                                        void                *aux_buf,
                                        void                *out_buf,
                                        glong                samples,
                                        const GeglRectangle *roi,
                                        gint                 level)
{
  PicmanOperationMaskComponents *self = PICMAN_OPERATION_MASK_COMPONENTS (operation);
  gfloat                      *src  = in_buf;
  gfloat                      *aux  = aux_buf;
  gfloat                      *dest = out_buf;
  PicmanComponentMask            mask = self->mask;
  static const gfloat          nothing[] = { 0.0, 0.0, 0.0, 1.0 };

  if (! aux)
    aux = (gfloat *) nothing;

  while (samples--)
    {
      dest[RED]   = (mask & PICMAN_COMPONENT_RED)   ? aux[RED]   : src[RED];
      dest[GREEN] = (mask & PICMAN_COMPONENT_GREEN) ? aux[GREEN] : src[GREEN];
      dest[BLUE]  = (mask & PICMAN_COMPONENT_BLUE)  ? aux[BLUE]  : src[BLUE];
      dest[ALPHA] = (mask & PICMAN_COMPONENT_ALPHA) ? aux[ALPHA] : src[ALPHA];

      src += 4;

      if (aux_buf)
        aux  += 4;

      dest += 4;
    }

  return TRUE;
}
