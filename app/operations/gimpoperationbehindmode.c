/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationbehindmode.c
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

#include "operations-types.h"

#include "picmanoperationbehindmode.h"


static gboolean picman_operation_behind_mode_process (GeglOperation       *operation,
                                                    void                *in_buf,
                                                    void                *aux_buf,
                                                    void                *aux2_buf,
                                                    void                *out_buf,
                                                    glong                samples,
                                                    const GeglRectangle *roi,
                                                    gint                 level);


G_DEFINE_TYPE (PicmanOperationBehindMode, picman_operation_behind_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)


static void
picman_operation_behind_mode_class_init (PicmanOperationBehindModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:behind-mode",
                                 "description", "PICMAN behind mode operation",
                                 NULL);

  point_class->process = picman_operation_behind_mode_process;
}

static void
picman_operation_behind_mode_init (PicmanOperationBehindMode *self)
{
}

static gboolean
picman_operation_behind_mode_process (GeglOperation       *operation,
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
  gfloat                      *layer    = aux_buf;
  gfloat                      *mask     = aux2_buf;
  gfloat                      *out      = out_buf;
  const gboolean               has_mask = mask != NULL;

  while (samples--)
    {
      gint    b;
      gdouble value = opacity;

      if (has_mask)
        value *= *mask;

      out[ALPHA] = in[ALPHA] + (1.0 - in[ALPHA]) * layer[ALPHA] * value;

      if (out[ALPHA])
        {
          for (b = RED; b < ALPHA; b++)
            {
              out[b] = (in[b] * in[ALPHA] + layer[b] * value * layer[ALPHA] * value * (1.0 - in[ALPHA])) / out[ALPHA];
            }
        }
      else
        {
          for (b = RED; b <= ALPHA; b++)
            {
              out[b] = in[b];
            }
        }

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}
