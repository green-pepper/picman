/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationcolorerasemode.c
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

#include <cairo.h>
#include <gegl-plugin.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "operations-types.h"

#include "picmanoperationcolorerasemode.h"


static gboolean picman_operation_color_erase_mode_process (GeglOperation       *operation,
                                                         void                *in_buf,
                                                         void                *aux_buf,
                                                         void                *aux2_buf,
                                                         void                *out_buf,
                                                         glong                samples,
                                                         const GeglRectangle *roi,
                                                         gint                 level);


G_DEFINE_TYPE (PicmanOperationColorEraseMode, picman_operation_color_erase_mode,
               PICMAN_TYPE_OPERATION_POINT_LAYER_MODE)


static void
picman_operation_color_erase_mode_class_init (PicmanOperationColorEraseModeClass *klass)
{
  GeglOperationClass               *operation_class;
  GeglOperationPointComposer3Class *point_class;

  operation_class = GEGL_OPERATION_CLASS (klass);
  point_class     = GEGL_OPERATION_POINT_COMPOSER3_CLASS (klass);

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:color-erase-mode",
                                 "description", "PICMAN color erase mode operation",
                                 NULL);

  point_class->process = picman_operation_color_erase_mode_process;
}

static void
picman_operation_color_erase_mode_init (PicmanOperationColorEraseMode *self)
{
}

static gboolean
picman_operation_color_erase_mode_process (GeglOperation       *operation,
                                         void                *in_buf,
                                         void                *aux_buf,
                                         void                *aux2_buf,
                                         void                *out_buf,
                                         glong                samples,
                                         const GeglRectangle *roi,
                                         gint                 level)
{
  gdouble         opacity  = PICMAN_OPERATION_POINT_LAYER_MODE (operation)->opacity;
  gfloat         *in       = in_buf;
  gfloat         *layer    = aux_buf;
  gfloat         *mask     = aux2_buf;
  gfloat         *out      = out_buf;
  const gboolean  has_mask = mask != NULL;

  while (samples--)
    {
      gfloat  layer_alpha;
      PicmanRGB bgcolor, color, alpha;

      layer_alpha = layer[ALPHA] * opacity;
      if (has_mask)
        layer_alpha *= *mask;

      picman_rgba_set (&color, in[0], in[1], in[2], in[3]);
      picman_rgba_set (&bgcolor, layer[0], layer[1], layer[2], layer_alpha);

      /* start of helper function copied from legacy 8-bit blending code */
      alpha.a = color.a;

      if (bgcolor.r < 0.0001)
        alpha.r = color.r;
      else if ( color.r > bgcolor.r )
        alpha.r = (color.r - bgcolor.r) / (1.0 - bgcolor.r);
      else if (color.r < bgcolor.r)
        alpha.r = (bgcolor.r - color.r) / bgcolor.r;
      else alpha.r = 0.0;

      if (bgcolor.g < 0.0001)
        alpha.g = color.g;
      else if ( color.g > bgcolor.g )
        alpha.g = (color.g - bgcolor.g) / (1.0 - bgcolor.g);
      else if ( color.g < bgcolor.g )
        alpha.g = (bgcolor.g - color.g) / (bgcolor.g);
      else alpha.g = 0.0;

      if (bgcolor.b < 0.0001)
        alpha.b = color.b;
      else if ( color.b > bgcolor.b )
        alpha.b = (color.b - bgcolor.b) / (1.0 - bgcolor.b);
      else if ( color.b < bgcolor.b )
        alpha.b = (bgcolor.b - color.b) / (bgcolor.b);
      else alpha.b = 0.0;

      if ( alpha.r > alpha.g )
        {
          if ( alpha.r > alpha.b )
            {
              color.a = alpha.r;
            }
          else
            {
              color.a = alpha.b;
            }
        }
      else if ( alpha.g > alpha.b )
        {
          color.a = alpha.g;
        }
      else
        {
          color.a = alpha.b;
        }

      color.a = (1.0 - bgcolor.a) + (color.a * bgcolor.a);

      if (color.a > 0.0001)
        {
          color.r = (color.r - bgcolor.r) / color.a + bgcolor.r;
          color.g = (color.g - bgcolor.g) / color.a + bgcolor.g;
          color.b = (color.b - bgcolor.b) / color.a + bgcolor.b;

          color.a *= alpha.a;
        }
      /* end of helper function */

      out[0] = color.r;
      out[1] = color.g;
      out[2] = color.b;
      out[3] = color.a;

      in    += 4;
      layer += 4;
      out   += 4;

      if (has_mask)
        mask++;
    }

  return TRUE;
}

