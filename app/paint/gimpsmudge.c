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

#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "gegl/picman-gegl-loops.h"
#include "gegl/picman-gegl-utils.h"

#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"
#include "core/picmantempbuf.h"

#include "picmansmudge.h"
#include "picmansmudgeoptions.h"

#include "picman-intl.h"


static void       picman_smudge_finalize     (GObject          *object);

static void       picman_smudge_paint        (PicmanPaintCore    *paint_core,
                                            PicmanDrawable     *drawable,
                                            PicmanPaintOptions *paint_options,
                                            const PicmanCoords *coords,
                                            PicmanPaintState    paint_state,
                                            guint32           time);
static gboolean   picman_smudge_start        (PicmanPaintCore    *paint_core,
                                            PicmanDrawable     *drawable,
                                            PicmanPaintOptions *paint_options,
                                            const PicmanCoords *coords);
static void       picman_smudge_motion       (PicmanPaintCore    *paint_core,
                                            PicmanDrawable     *drawable,
                                            PicmanPaintOptions *paint_options,
                                            const PicmanCoords *coords);

static void       picman_smudge_accumulator_coords (PicmanPaintCore    *paint_core,
                                                  const PicmanCoords *coords,
                                                  gint             *x,
                                                  gint             *y);

static void       picman_smudge_accumulator_size   (PicmanPaintOptions *paint_options,
                                                  gint             *accumulator_size);


G_DEFINE_TYPE (PicmanSmudge, picman_smudge, PICMAN_TYPE_BRUSH_CORE)

#define parent_class picman_smudge_parent_class


void
picman_smudge_register (Picman                      *picman,
                      PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_SMUDGE,
                PICMAN_TYPE_SMUDGE_OPTIONS,
                "picman-smudge",
                _("Smudge"),
                "picman-tool-smudge");
}

static void
picman_smudge_class_init (PicmanSmudgeClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanBrushCoreClass *brush_core_class = PICMAN_BRUSH_CORE_CLASS (klass);

  object_class->finalize  = picman_smudge_finalize;

  paint_core_class->paint = picman_smudge_paint;

  brush_core_class->handles_changing_brush = TRUE;
  brush_core_class->handles_transforming_brush = TRUE;
  brush_core_class->handles_dynamic_transforming_brush = TRUE;
}

static void
picman_smudge_init (PicmanSmudge *smudge)
{
}

static void
picman_smudge_finalize (GObject *object)
{
  PicmanSmudge *smudge = PICMAN_SMUDGE (object);

  if (smudge->accum_buffer)
    {
      g_object_unref (smudge->accum_buffer);
      smudge->accum_buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_smudge_paint (PicmanPaintCore    *paint_core,
                   PicmanDrawable     *drawable,
                   PicmanPaintOptions *paint_options,
                   const PicmanCoords *coords,
                   PicmanPaintState    paint_state,
                   guint32           time)
{
  PicmanSmudge *smudge = PICMAN_SMUDGE (paint_core);

  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_MOTION:
      /* initialization fails if the user starts outside the drawable */
      if (! smudge->initialized)
        smudge->initialized = picman_smudge_start (paint_core, drawable,
                                                 paint_options, coords);

      if (smudge->initialized)
        picman_smudge_motion (paint_core, drawable, paint_options, coords);
      break;

    case PICMAN_PAINT_STATE_FINISH:
      if (smudge->accum_buffer)
        {
          g_object_unref (smudge->accum_buffer);
          smudge->accum_buffer = NULL;
        }
      smudge->initialized = FALSE;
      break;

    default:
      break;
    }
}

static gboolean
picman_smudge_start (PicmanPaintCore    *paint_core,
                   PicmanDrawable     *drawable,
                   PicmanPaintOptions *paint_options,
                   const PicmanCoords *coords)
{
  PicmanSmudge *smudge = PICMAN_SMUDGE (paint_core);
  GeglBuffer *paint_buffer;
  gint        paint_buffer_x;
  gint        paint_buffer_y;
  gint        accum_size;
  gint        x, y;

  paint_buffer = picman_paint_core_get_paint_buffer (paint_core, drawable,
                                                   paint_options, coords,
                                                   &paint_buffer_x,
                                                   &paint_buffer_y);
  if (! paint_buffer)
    return FALSE;

  picman_smudge_accumulator_size (paint_options, &accum_size);

  /*  Allocate the accumulation buffer */
  smudge->accum_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                          accum_size,
                                                          accum_size),
                                          babl_format ("RGBA float"));

  /*  adjust the x and y coordinates to the upper left corner of the
   *  accumulator
   */
  picman_smudge_accumulator_coords (paint_core, coords, &x, &y);

  /*  If clipped, prefill the smudge buffer with the color at the
   *  brush position.
   */
  if (x != paint_buffer_x ||
      y != paint_buffer_y ||
      accum_size != gegl_buffer_get_width  (paint_buffer) ||
      accum_size != gegl_buffer_get_height (paint_buffer))
    {
      PicmanRGB    pixel;
      GeglColor *color;

      picman_pickable_get_color_at (PICMAN_PICKABLE (drawable),
                                  CLAMP ((gint) coords->x,
                                         0,
                                         picman_item_get_width (PICMAN_ITEM (drawable)) - 1),
                                  CLAMP ((gint) coords->y,
                                         0,
                                         picman_item_get_height (PICMAN_ITEM (drawable)) - 1),
                                  &pixel);

      color = picman_gegl_color_new (&pixel);
      gegl_buffer_set_color (smudge->accum_buffer, NULL, color);
      g_object_unref (color);
    }

  /* copy the region under the original painthit. */
  gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                    GEGL_RECTANGLE (paint_buffer_x,
                                    paint_buffer_y,
                                    gegl_buffer_get_width  (paint_buffer),
                                    gegl_buffer_get_height (paint_buffer)),
                    smudge->accum_buffer,
                    GEGL_RECTANGLE (paint_buffer_x - x,
                                    paint_buffer_y - y,
                                    0, 0));

  return TRUE;
}

static void
picman_smudge_motion (PicmanPaintCore    *paint_core,
                    PicmanDrawable     *drawable,
                    PicmanPaintOptions *paint_options,
                    const PicmanCoords *coords)
{
  PicmanSmudge        *smudge   = PICMAN_SMUDGE (paint_core);
  PicmanSmudgeOptions *options  = PICMAN_SMUDGE_OPTIONS (paint_options);
  PicmanContext       *context  = PICMAN_CONTEXT (paint_options);
  PicmanDynamics      *dynamics = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage         *image    = picman_item_get_image (PICMAN_ITEM (drawable));
  GeglBuffer        *paint_buffer;
  gint               paint_buffer_x;
  gint               paint_buffer_y;
  gint               paint_buffer_width;
  gint               paint_buffer_height;
  gdouble            fade_point;
  gdouble            opacity;
  gdouble            rate;
  gdouble            dynamic_rate;
  gint               x, y;
  gdouble            hardness;

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

  paint_buffer_width  = gegl_buffer_get_width  (paint_buffer);
  paint_buffer_height = gegl_buffer_get_height (paint_buffer);

  /*  Get the unclipped acumulator coordinates  */
  picman_smudge_accumulator_coords (paint_core, coords, &x, &y);

  /* Enable dynamic rate */
  dynamic_rate = picman_dynamics_get_linear_value (dynamics,
                                                 PICMAN_DYNAMICS_OUTPUT_RATE,
                                                 coords,
                                                 paint_options,
                                                 fade_point);

  rate = (options->rate / 100.0) * dynamic_rate;

  /*  Smudge uses the buffer Accum.
   *  For each successive painthit Accum is built like this
   *    Accum =  rate*Accum  + (1-rate)*I.
   *  where I is the pixels under the current painthit.
   *  Then the paint area (paint_area) is built as
   *    (Accum,1) (if no alpha),
   */

  picman_gegl_smudge_blend (smudge->accum_buffer,
                          GEGL_RECTANGLE (paint_buffer_x - x,
                                          paint_buffer_y - y,
                                          paint_buffer_width,
                                          paint_buffer_height),
                          picman_drawable_get_buffer (drawable),
                          GEGL_RECTANGLE (paint_buffer_x,
                                          paint_buffer_y,
                                          paint_buffer_width,
                                          paint_buffer_height),
                          smudge->accum_buffer,
                          GEGL_RECTANGLE (paint_buffer_x - x,
                                          paint_buffer_y - y,
                                          paint_buffer_width,
                                          paint_buffer_height),
                          rate);

  gegl_buffer_copy (smudge->accum_buffer,
                    GEGL_RECTANGLE (paint_buffer_x - x,
                                    paint_buffer_y - y,
                                    paint_buffer_width,
                                    paint_buffer_height),
                    paint_buffer,
                    GEGL_RECTANGLE (0, 0, 0, 0));

  hardness = picman_dynamics_get_linear_value (dynamics,
                                             PICMAN_DYNAMICS_OUTPUT_HARDNESS,
                                             coords,
                                             paint_options,
                                             fade_point);

  picman_brush_core_replace_canvas (PICMAN_BRUSH_CORE (paint_core), drawable,
                                  coords,
                                  MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                  picman_context_get_opacity (context),
                                  picman_paint_options_get_brush_mode (paint_options),
                                  hardness,
                                  PICMAN_PAINT_INCREMENTAL);
}

static void
picman_smudge_accumulator_coords (PicmanPaintCore    *paint_core,
                                const PicmanCoords *coords,
                                gint             *x,
                                gint             *y)
{
  PicmanSmudge *smudge = PICMAN_SMUDGE (paint_core);

  *x = (gint) coords->x - gegl_buffer_get_width  (smudge->accum_buffer) / 2;
  *y = (gint) coords->y - gegl_buffer_get_height (smudge->accum_buffer) / 2;
}

static void
picman_smudge_accumulator_size (PicmanPaintOptions *paint_options,
                              gint             *accumulator_size)
{
  /* Note: the max brush mask size plus a border of 1 pixel and a
   * little headroom
   */
  *accumulator_size = ceil (sqrt (2 * SQR (paint_options->brush_size + 1)) + 2);
}
