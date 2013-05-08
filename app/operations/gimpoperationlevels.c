/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanoperationlevels.c
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

#include "libpicmanmath/picmanmath.h"

#include "operations-types.h"

#include "picmanlevelsconfig.h"
#include "picmanoperationlevels.h"


static gboolean picman_operation_levels_process (GeglOperation       *operation,
                                               void                *in_buf,
                                               void                *out_buf,
                                               glong                samples,
                                               const GeglRectangle *roi,
                                               gint                 level);


G_DEFINE_TYPE (PicmanOperationLevels, picman_operation_levels,
               PICMAN_TYPE_OPERATION_POINT_FILTER)

#define parent_class picman_operation_levels_parent_class


static void
picman_operation_levels_class_init (PicmanOperationLevelsClass *klass)
{
  GObjectClass                  *object_class    = G_OBJECT_CLASS (klass);
  GeglOperationClass            *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_class     = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  object_class->set_property   = picman_operation_point_filter_set_property;
  object_class->get_property   = picman_operation_point_filter_get_property;

  gegl_operation_class_set_keys (operation_class,
                                 "name",        "picman:levels",
                                 "categories",  "color",
                                 "description", "PICMAN Levels operation",
                                 NULL);

  point_class->process = picman_operation_levels_process;

  g_object_class_install_property (object_class,
                                   PICMAN_OPERATION_POINT_FILTER_PROP_CONFIG,
                                   g_param_spec_object ("config",
                                                        "Config",
                                                        "The config object",
                                                        PICMAN_TYPE_LEVELS_CONFIG,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT));
}

static void
picman_operation_levels_init (PicmanOperationLevels *self)
{
}

static inline gdouble
picman_operation_levels_map (gdouble value,
                           gdouble inv_gamma,
                           gdouble low_input,
                           gdouble high_input,
                           gdouble low_output,
                           gdouble high_output)
{
  /*  determine input intensity  */
  if (high_input != low_input)
    value = (value - low_input) / (high_input - low_input);
  else
    value = (value - low_input);

  if (inv_gamma != 1.0 && value > 0)
    value =  pow (value, inv_gamma);

  /*  determine the output intensity  */
  if (high_output >= low_output)
    value = value * (high_output - low_output) + low_output;
  else if (high_output < low_output)
    value = low_output - value * (low_output - high_output);

  return value;
}

static gboolean
picman_operation_levels_process (GeglOperation       *operation,
                               void                *in_buf,
                               void                *out_buf,
                               glong                samples,
                               const GeglRectangle *roi,
                               gint                 level)
{
  PicmanOperationPointFilter *point  = PICMAN_OPERATION_POINT_FILTER (operation);
  PicmanLevelsConfig         *config = PICMAN_LEVELS_CONFIG (point->config);
  gfloat                   *src    = in_buf;
  gfloat                   *dest   = out_buf;
  gfloat                    inv_gamma[5];
  gint                      channel;

  if (! config)
    return FALSE;

  for (channel = 0; channel < 5; channel++)
    {
      g_return_val_if_fail (config->gamma[channel] != 0.0, FALSE);

      inv_gamma[channel] = 1.0 / config->gamma[channel];
    }

  while (samples--)
    {
      for (channel = 0; channel < 4; channel++)
        {
          gdouble value;

          value = picman_operation_levels_map (src[channel],
                                             inv_gamma[channel + 1],
                                             config->low_input[channel + 1],
                                             config->high_input[channel + 1],
                                             config->low_output[channel + 1],
                                             config->high_output[channel + 1]);

          /* don't apply the overall curve to the alpha channel */
          if (channel != ALPHA)
            value = picman_operation_levels_map (value,
                                               inv_gamma[0],
                                               config->low_input[0],
                                               config->high_input[0],
                                               config->low_output[0],
                                               config->high_output[0]);

          dest[channel] = value;
        }

      src  += 4;
      dest += 4;
    }

  return TRUE;
}


/*  public functions  */

gdouble
picman_operation_levels_map_input (PicmanLevelsConfig     *config,
                                 PicmanHistogramChannel  channel,
                                 gdouble               value)
{
  g_return_val_if_fail (PICMAN_IS_LEVELS_CONFIG (config), 0.0);

  /*  determine input intensity  */
  if (config->high_input[channel] != config->low_input[channel])
    value = ((value - config->low_input[channel]) /
             (config->high_input[channel] - config->low_input[channel]));
  else
    value = (value - config->low_input[channel]);

  if (config->gamma[channel] != 0.0)
    value = pow (value, 1.0 / config->gamma[channel]);

  return value;
}
