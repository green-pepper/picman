/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationsaturationmode.c
 * Copyright (C) 2008 Michael Natterer <mitch@picman.org>
 *               2012 Ville Sokk <ville.sokk@gmail.com>
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
#include <cairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "operations-types.h"

#include "picmanoperationsaturationmode.h"


static gboolean picman_operation_saturation_mode_process (GeglOperation       *operation,
                                                        void                *in_buf,
                                                        void                *aux_buf,
                                                        void                *aux2_buf,
                                                        void                *out_buf,
                                                        glong                samples,
                                                        const GeglRectangle *roi,
                                                        gint                 level);


G_DEFINE_TYPE (PicmanOperationSaturationMode, picman_operation_saturation_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)


static void
picman_operation_saturation_mode_class_init (PicmanOperationSaturationModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:saturation-mode",
                                 "description", "PICMAN saturation mode operation",
                                 NULL);

  point_class->process = picman_operation_saturation_mode_process;
}

static void
picman_operation_saturation_mode_init (PicmanOperationSaturationMode *self)
{
}

static gboolean
picman_operation_saturation_mode_process (GeglOperation       *operation,
                                        void                *in_buf,
                                        void                *aux_buf,
                                        void                *aux2_buf,
                                        void                *out_buf,
                                        glong                samples,
                                        const GeglRectangle *roi,
                                        gint                 level)
{
  gdouble        opacity  = PICMAN_OPERATION_POINT_LAYER_MODE (operation)->opacity;
  gfloat        *in       = in_buf;
  gfloat        *layer    = aux_buf;
  gfloat        *mask     = aux2_buf;
  gfloat        *out      = out_buf;
  const gboolean has_mask = mask != NULL;

  while (samples--)
    {
      PicmanHSV layer_hsv, out_hsv;
      PicmanRGB layer_rgb = {layer[0], layer[1], layer[2]};
      PicmanRGB out_rgb   = {in[0], in[1], in[2]};
      gfloat  comp_alpha, new_alpha;

      comp_alpha = MIN (in[ALPHA], layer[ALPHA]) * opacity;
      if (has_mask)
        comp_alpha *= *mask;

      new_alpha = in[ALPHA] + (1.0 - in[ALPHA]) * comp_alpha;

      if (comp_alpha && new_alpha)
        {
          gint   b;
          gfloat ratio = comp_alpha / new_alpha;

          picman_rgb_to_hsv (&layer_rgb, &layer_hsv);
          picman_rgb_to_hsv (&out_rgb, &out_hsv);

          out_hsv.s = layer_hsv.s;
          picman_hsv_to_rgb (&out_hsv, &out_rgb);

          out[0] = out_rgb.r;
          out[1] = out_rgb.g;
          out[2] = out_rgb.b;

          for (b = RED; b < ALPHA; b++)
            {
              out[b] = out[b] * ratio + in[b] * (1.0 - ratio);
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

      out[ALPHA] = in[ALPHA];

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}
