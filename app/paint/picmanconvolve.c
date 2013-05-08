/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
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

#include "paint-types.h"

#include "gegl/picman-gegl-loops.h"

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"
#include "core/picmantempbuf.h"

#include "picmanconvolve.h"
#include "picmanconvolveoptions.h"

#include "picman-intl.h"


#define FIELD_COLS    4
#define MIN_BLUR      64         /*  (8/9 original pixel)   */
#define MAX_BLUR      0.25       /*  (1/33 original pixel)  */
#define MIN_SHARPEN   -512
#define MAX_SHARPEN   -64


static void    picman_convolve_paint            (PicmanPaintCore    *paint_core,
                                               PicmanDrawable     *drawable,
                                               PicmanPaintOptions *paint_options,
                                               const PicmanCoords *coords,
                                               PicmanPaintState    paint_state,
                                               guint32           time);
static void    picman_convolve_motion           (PicmanPaintCore    *paint_core,
                                               PicmanDrawable     *drawable,
                                               PicmanPaintOptions *paint_options,
                                               const PicmanCoords *coords);

static void    picman_convolve_calculate_matrix (PicmanConvolve     *convolve,
                                               PicmanConvolveType  type,
                                               gint              radius_x,
                                               gint              radius_y,
                                               gdouble           rate);
static gdouble picman_convolve_sum_matrix       (const gfloat     *matrix);


G_DEFINE_TYPE (PicmanConvolve, picman_convolve, PICMAN_TYPE_BRUSH_CORE)


void
picman_convolve_register (Picman                      *picman,
                        PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_CONVOLVE,
                PICMAN_TYPE_CONVOLVE_OPTIONS,
                "picman-convolve",
                _("Convolve"),
                "picman-tool-blur");
}

static void
picman_convolve_class_init (PicmanConvolveClass *klass)
{
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);

  paint_core_class->paint = picman_convolve_paint;
}

static void
picman_convolve_init (PicmanConvolve *convolve)
{
  gint i;

  for (i = 0; i < 9; i++)
    convolve->matrix[i] = 1.0;

  convolve->matrix_divisor = 9.0;
}

static void
picman_convolve_paint (PicmanPaintCore    *paint_core,
                     PicmanDrawable     *drawable,
                     PicmanPaintOptions *paint_options,
                     const PicmanCoords *coords,
                     PicmanPaintState    paint_state,
                     guint32           time)
{
  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_MOTION:
      picman_convolve_motion (paint_core, drawable, paint_options, coords);
      break;

    default:
      break;
    }
}

static void
picman_convolve_motion (PicmanPaintCore    *paint_core,
                      PicmanDrawable     *drawable,
                      PicmanPaintOptions *paint_options,
                      const PicmanCoords *coords)
{
  PicmanConvolve        *convolve   = PICMAN_CONVOLVE (paint_core);
  PicmanBrushCore       *brush_core = PICMAN_BRUSH_CORE (paint_core);
  PicmanConvolveOptions *options    = PICMAN_CONVOLVE_OPTIONS (paint_options);
  PicmanContext         *context    = PICMAN_CONTEXT (paint_options);
  PicmanDynamics        *dynamics   = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage           *image      = picman_item_get_image (PICMAN_ITEM (drawable));
  GeglBuffer          *paint_buffer;
  gint                 paint_buffer_x;
  gint                 paint_buffer_y;
  PicmanTempBuf         *temp_buf;
  GeglBuffer          *convolve_buffer;
  gdouble              fade_point;
  gdouble              opacity;
  gdouble              rate;

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  opacity = picman_dynamics_get_linear_value (dynamics,
                                            PICMAN_DYNAMICS_OUTPUT_OPACITY,
                                            coords,
                                            paint_options,
                                            fade_point);
  if (opacity == 0.0)
    return;

  paint_buffer = picman_paint_core_get_paint_buffer (paint_core, drawable,
                                                   paint_options, coords,
                                                   &paint_buffer_x,
                                                   &paint_buffer_y);
  if (! paint_buffer)
    return;

  rate = (options->rate *
          picman_dynamics_get_linear_value (dynamics,
                                          PICMAN_DYNAMICS_OUTPUT_RATE,
                                          coords,
                                          paint_options,
                                          fade_point));

  picman_convolve_calculate_matrix (convolve, options->type,
                                  picman_temp_buf_get_width  (brush_core->brush->mask) / 2,
                                  picman_temp_buf_get_height (brush_core->brush->mask) / 2,
                                  rate);

  /*  need a linear buffer for picman_gegl_convolve()  */
  temp_buf = picman_temp_buf_new (gegl_buffer_get_width  (paint_buffer),
                                gegl_buffer_get_height (paint_buffer),
                                gegl_buffer_get_format (paint_buffer));
  convolve_buffer = picman_temp_buf_create_buffer (temp_buf);
  picman_temp_buf_unref (temp_buf);

  gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                    GEGL_RECTANGLE (paint_buffer_x,
                                    paint_buffer_y,
                                    gegl_buffer_get_width  (paint_buffer),
                                    gegl_buffer_get_height (paint_buffer)),
                    convolve_buffer,
                    GEGL_RECTANGLE (0, 0, 0, 0));

  picman_gegl_convolve (convolve_buffer,
                      GEGL_RECTANGLE (0, 0,
                                      gegl_buffer_get_width  (convolve_buffer),
                                      gegl_buffer_get_height (convolve_buffer)),
                      paint_buffer,
                      GEGL_RECTANGLE (0, 0,
                                      gegl_buffer_get_width  (paint_buffer),
                                      gegl_buffer_get_height (paint_buffer)),
                      convolve->matrix, 3, convolve->matrix_divisor,
                      PICMAN_NORMAL_CONVOL, TRUE);

  g_object_unref (convolve_buffer);

  picman_brush_core_replace_canvas (brush_core, drawable,
                                  coords,
                                  MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                  picman_context_get_opacity (context),
                                  picman_paint_options_get_brush_mode (paint_options),
                                  1.0,
                                  PICMAN_PAINT_INCREMENTAL);
}

static void
picman_convolve_calculate_matrix (PicmanConvolve    *convolve,
                                PicmanConvolveType type,
                                gint             radius_x,
                                gint             radius_y,
                                gdouble          rate)
{
  /*  find percent of tool pressure  */
  const gdouble percent = MIN (rate / 100.0, 1.0);

  convolve->matrix[0] = (radius_x && radius_y) ? 1.0 : 0.0;
  convolve->matrix[1] = (radius_y)             ? 1.0 : 0.0;
  convolve->matrix[2] = (radius_x && radius_y) ? 1.0 : 0.0;
  convolve->matrix[3] = (radius_x)             ? 1.0 : 0.0;

  /*  get the appropriate convolution matrix and size and divisor  */
  switch (type)
    {
    case PICMAN_BLUR_CONVOLVE:
      convolve->matrix[4] = MIN_BLUR + percent * (MAX_BLUR - MIN_BLUR);
      break;

    case PICMAN_SHARPEN_CONVOLVE:
      convolve->matrix[4] = MIN_SHARPEN + percent * (MAX_SHARPEN - MIN_SHARPEN);
      break;

    case PICMAN_CUSTOM_CONVOLVE:
      break;
    }

  convolve->matrix[5] = (radius_x)             ? 1.0 : 0.0;
  convolve->matrix[6] = (radius_x && radius_y) ? 1.0 : 0.0;
  convolve->matrix[7] = (radius_y)             ? 1.0 : 0.0;
  convolve->matrix[8] = (radius_x && radius_y) ? 1.0 : 0.0;

  convolve->matrix_divisor = picman_convolve_sum_matrix (convolve->matrix);
}

static gdouble
picman_convolve_sum_matrix (const gfloat *matrix)
{
  gdouble sum = 0.0;
  gint    i;

  for (i = 0; i < 9; i++)
    sum += matrix[i];

  return sum;
}
