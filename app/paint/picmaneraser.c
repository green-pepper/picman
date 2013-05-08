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

#include "gegl/picman-gegl-utils.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanimage.h"

#include "picmaneraser.h"
#include "picmaneraseroptions.h"

#include "picman-intl.h"


static void   picman_eraser_paint  (PicmanPaintCore    *paint_core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  const PicmanCoords *coords,
                                  PicmanPaintState    paint_state,
                                  guint32           time);
static void   picman_eraser_motion (PicmanPaintCore    *paint_core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  const PicmanCoords *coords);


G_DEFINE_TYPE (PicmanEraser, picman_eraser, PICMAN_TYPE_BRUSH_CORE)


void
picman_eraser_register (Picman                      *picman,
                      PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_ERASER,
                PICMAN_TYPE_ERASER_OPTIONS,
                "picman-eraser",
                _("Eraser"),
                "picman-tool-eraser");
}

static void
picman_eraser_class_init (PicmanEraserClass *klass)
{
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanBrushCoreClass *brush_core_class = PICMAN_BRUSH_CORE_CLASS (klass);

  paint_core_class->paint = picman_eraser_paint;

  brush_core_class->handles_changing_brush = TRUE;
}

static void
picman_eraser_init (PicmanEraser *eraser)
{
}

static void
picman_eraser_paint (PicmanPaintCore    *paint_core,
                   PicmanDrawable     *drawable,
                   PicmanPaintOptions *paint_options,
                   const PicmanCoords *coords,
                   PicmanPaintState    paint_state,
                   guint32           time)
{
  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_MOTION:
      picman_eraser_motion (paint_core, drawable, paint_options, coords);
      break;

    default:
      break;
    }
}

static void
picman_eraser_motion (PicmanPaintCore    *paint_core,
                    PicmanDrawable     *drawable,
                    PicmanPaintOptions *paint_options,
                    const PicmanCoords *coords)
{
  PicmanEraserOptions    *options  = PICMAN_ERASER_OPTIONS (paint_options);
  PicmanContext          *context  = PICMAN_CONTEXT (paint_options);
  PicmanDynamics         *dynamics = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage            *image    = picman_item_get_image (PICMAN_ITEM (drawable));
  gdouble               fade_point;
  gdouble               opacity;
  PicmanLayerModeEffects  paint_mode;
  GeglBuffer           *paint_buffer;
  gint                  paint_buffer_x;
  gint                  paint_buffer_y;
  PicmanRGB               background;
  GeglColor            *color;
  gdouble               force;

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

  picman_context_get_background (context, &background);
  color = picman_gegl_color_new (&background);

  gegl_buffer_set_color (paint_buffer, NULL, color);
  g_object_unref (color);

  if (options->anti_erase)
    paint_mode = PICMAN_ANTI_ERASE_MODE;
  else if (picman_drawable_has_alpha (drawable))
    paint_mode = PICMAN_ERASE_MODE;
  else
    paint_mode = PICMAN_NORMAL_MODE;

  force = picman_dynamics_get_linear_value (dynamics,
                                          PICMAN_DYNAMICS_OUTPUT_FORCE,
                                          coords,
                                          paint_options,
                                          fade_point);

  picman_brush_core_paste_canvas (PICMAN_BRUSH_CORE (paint_core), drawable,
                                coords,
                                MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                picman_context_get_opacity (context),
                                paint_mode,
                                picman_paint_options_get_brush_mode (paint_options),
                                force,
                                paint_options->application_mode);
}
