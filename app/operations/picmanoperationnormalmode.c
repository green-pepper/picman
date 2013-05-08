/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationnormalmode.c
 * Copyright (C) 2012 Michael Natterer <mitch@picman.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gegl-plugin.h>

#include "operations-types.h"

#include "picmanoperationnormalmode.h"


static gboolean picman_operation_normal_parent_process (GeglOperation        *operation,
                                                      GeglOperationContext *context,
                                                      const gchar          *output_prop,
                                                      const GeglRectangle  *result,
                                                      gint                  level);
static gboolean picman_operation_normal_mode_process   (GeglOperation        *operation,
                                                      void                 *in_buf,
                                                      void                 *aux_buf,
                                                      void                 *aux2_buf,
                                                      void                 *out_buf,
                                                      glong                 samples,
                                                      const GeglRectangle  *roi,
                                                      gint                  level);


G_DEFINE_TYPE (PicmanOperationNormalMode, picman_operation_normal_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)

#define parent_class picman_operation_normal_mode_parent_class

static const gchar* reference_xml = "<?xml version='1.0' encoding='UTF-8'?>"
"<gegl>"
"<node operation='picman:normal-mode'>"
"  <node operation='gegl:load'>"
"    <params>"
"      <param name='path'>blending-test-B.png</param>"
"    </params>"
"  </node>"
"</node>"
"<node operation='gegl:load'>"
"  <params>"
"    <param name='path'>blending-test-A.png</param>"
"  </params>"
"</node>"
"</gegl>";


static void
picman_operation_normal_mode_class_init (PicmanOperationNormalModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",                  "picman:normal-mode",
                                 "description",           "PICMAN normal mode operation",
                                 "reference-image",       "normal-mode.png",
                                 "reference-composition", reference_xml,
                                 NULL);

  operation_class->process     = picman_operation_normal_parent_process;

  point_class->process         = picman_operation_normal_mode_process;
}

static void
picman_operation_normal_mode_init (PicmanOperationNormalMode *self)
{
}

static gboolean
picman_operation_normal_parent_process (GeglOperation        *operation,
                                      GeglOperationContext *context,
                                      const gchar          *output_prop,
                                      const GeglRectangle  *result,
                                      gint                  level)
{
  PicmanOperationPointLayerMode *point;

  point = PICMAN_OPERATION_POINT_LAYER_MODE (operation);

  if (point->opacity == 1.0 &&
      ! gegl_operation_context_get_object (context, "aux2"))
    {
      const GeglRectangle *in_extent  = NULL;
      const GeglRectangle *aux_extent = NULL;
      GObject             *input;
      GObject             *aux;

      /* get the raw values this does not increase the reference count */
      input = gegl_operation_context_get_object (context, "input");
      aux   = gegl_operation_context_get_object (context, "aux");

      /* pass the input/aux buffers directly through if they are not
       * overlapping
       */
      if (input)
        in_extent = gegl_buffer_get_abyss (GEGL_BUFFER (input));

      if (! input ||
          (aux && ! gegl_rectangle_intersect (NULL, in_extent, result)))
        {
          gegl_operation_context_set_object (context, "output", aux);
          return TRUE;
        }

      if (aux)
        aux_extent = gegl_buffer_get_abyss (GEGL_BUFFER (aux));

      if (! aux ||
          (input && ! gegl_rectangle_intersect (NULL, aux_extent, result)))
        {
          gegl_operation_context_set_object (context, "output", input);
          return TRUE;
        }
    }

  /* chain up, which will create the needed buffers for our actual
   * process function
   */
  return GEGL_OPERATION_CLASS (parent_class)->process (operation, context,
                                                       output_prop, result,
                                                       level);
}

static gboolean
picman_operation_normal_mode_process (GeglOperation       *operation,
                                    void                *in_buf,
                                    void                *aux_buf,
                                    void                *aux2_buf,
                                    void                *out_buf,
                                    glong                samples,
                                    const GeglRectangle *roi,
                                    gint                 level)
{
  PicmanOperationPointLayerMode *point    = PICMAN_OPERATION_POINT_LAYER_MODE (operation);
  gdouble                      opacity  = point->opacity;
  gfloat                      *in       = in_buf;
  gfloat                      *aux      = aux_buf;
  gfloat                      *mask     = aux2_buf;
  gfloat                      *out      = out_buf;
  const gboolean               has_mask = mask != NULL;

  while (samples--)
    {
      gfloat aux_alpha;

      aux_alpha = aux[ALPHA] * opacity;
      if (has_mask)
        aux_alpha *= *mask;

      out[ALPHA] = aux_alpha + in[ALPHA] - aux_alpha * in[ALPHA];

      if (out[ALPHA])
        {
          gfloat in_weight       = in[ALPHA] * (1.0f - aux_alpha);
          gfloat recip_out_alpha = 1.0f / out[ALPHA];
          gint   b;

          for (b = RED; b < ALPHA; b++)
            {
              out[b] = (aux[b] * aux_alpha + in[b] * in_weight) * recip_out_alpha;
            }
        }
      else
        {
          gint b;

          for (b = RED; b < ALPHA; b++)
            {
              out[b] = in[b];
            }
        }

      in   += 4;
      aux  += 4;
      out  += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}
