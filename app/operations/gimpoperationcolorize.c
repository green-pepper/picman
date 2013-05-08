/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationcolorize.c
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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "operations-types.h"

#include "picmancolorizeconfig.h"
#include "picmanoperationcolorize.h"


static gboolean picman_operation_colorize_process (GeglOperation       *operation,
                                                 void                *in_buf,
                                                 void                *out_buf,
                                                 glong                samples,
                                                 const GeglRectangle *roi,
                                                 gint                 level);


G_DEFINE_TYPE (PicmanOperationColorize, picman_operation_colorize,
               PICMAN_TYPE_OPERATION_POINT_FILTER)

#define parent_class picman_operation_colorize_parent_class


static void
picman_operation_colorize_class_init (PicmanOperationColorizeClass *klass)
{
  GObjectClass                  *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationClass            *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_class     = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  object_class->set_property   = picman_operation_point_filter_set_property;
  object_class->get_property   = picman_operation_point_filter_get_property;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:colorize",
                                 "categories",  "color",
                                 "description", "PICMAN Colorize operation",
                                 NULL);

  point_class->process = picman_operation_colorize_process;

  g_object_class_install_property (object_class,
                                   PICMAN_OPERATION_POINT_FILTER_PROP_CONFIG,
                                   g_param_spec_object ("config",
                                                        "Config",
                                                        "The config object",
                                                        PICMAN_TYPE_COLORIZE_CONFIG,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_operation_colorize_init (PicmanOperationColorize *self)
{
}

static gboolean
picman_operation_colorize_process (GeglOperation       *operation,
                                 void                *in_buf,
                                 void                *out_buf,
                                 glong                samples,
                                 const GeglRectangle *roi,
                                 gint                 level)
{
  PicmanOperationPointFilter *point  = PICMAN_OPERATION_POINT_FILTER (operation);
  PicmanColorizeConfig       *config = PICMAN_COLORIZE_CONFIG (point->config);
  gfloat                   *src    = in_buf;
  gfloat                   *dest   = out_buf;
  PicmanHSL                   hsl;

  if (! config)
    return FALSE;

  hsl.h = config->hue;
  hsl.s = config->saturation;

  while (samples--)
    {
      PicmanRGB rgb;
      gfloat  lum = PICMAN_RGB_LUMINANCE (src[RED],
                                        src[GREEN],
                                        src[BLUE]);

      if (config->lightness > 0)
        {
          lum = lum * (1.0 - config->lightness);

          lum += 1.0 - (1.0 - config->lightness);
        }
      else if (config->lightness < 0)
        {
          lum = lum * (config->lightness + 1.0);
        }

      hsl.l = lum;

      picman_hsl_to_rgb (&hsl, &rgb);

      /*  the code in base/colorize.c would multiply r,b,g with lum,
       *  but this is a bug since it should multiply with 255. We
       *  don't repeat this bug here (this is the reason why the gegl
       *  colorize is brighter than the legacy one).
       */
      dest[RED]   = rgb.r; /* * lum; */
      dest[GREEN] = rgb.g; /* * lum; */
      dest[BLUE]  = rgb.b; /* * lum */;
      dest[ALPHA] = src[ALPHA];

      src  += 4;
      dest += 4;
    }

  return TRUE;
}
