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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "gegl/picman-gegl-loops.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanimage.h"

#include "picmandodgeburn.h"
#include "picmandodgeburnoptions.h"

#include "picman-intl.h"


static void   picman_dodge_burn_paint  (PicmanPaintCore    *paint_core,
                                      PicmanDrawable     *drawable,
                                      PicmanPaintOptions *paint_options,
                                      const PicmanCoords *coords,
                                      PicmanPaintState    paint_state,
                                      guint32           time);
static void   picman_dodge_burn_motion (PicmanPaintCore    *paint_core,
                                      PicmanDrawable     *drawable,
                                      PicmanPaintOptions *paint_options,
                                      const PicmanCoords *coords);


G_DEFINE_TYPE (PicmanDodgeBurn, picman_dodge_burn, PICMAN_TYPE_BRUSH_CORE)

#define parent_class picman_dodge_burn_parent_class


void
picman_dodge_burn_register (Picman                      *picman,
                          PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_DODGE_BURN,
                PICMAN_TYPE_DODGE_BURN_OPTIONS,
                "picman-dodge-burn",
                _("Dodge/Burn"),
                "picman-tool-dodge");
}

static void
picman_dodge_burn_class_init (PicmanDodgeBurnClass *klass)
{
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanBrushCoreClass *brush_core_class = PICMAN_BRUSH_CORE_CLASS (klass);

  paint_core_class->paint = picman_dodge_burn_paint;

  brush_core_class->handles_changing_brush = TRUE;
}

static void
picman_dodge_burn_init (PicmanDodgeBurn *dodgeburn)
{
}

static void
picman_dodge_burn_paint (PicmanPaintCore    *paint_core,
                       PicmanDrawable     *drawable,
                       PicmanPaintOptions *paint_options,
                       const PicmanCoords *coords,
                       PicmanPaintState    paint_state,
                       guint32           time)
{
  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_INIT:
      break;

    case PICMAN_PAINT_STATE_MOTION:
      picman_dodge_burn_motion (paint_core, drawable, paint_options, coords);
      break;

    case PICMAN_PAINT_STATE_FINISH:
      break;
    }
}

static void
picman_dodge_burn_motion (PicmanPaintCore    *paint_core,
                        PicmanDrawable     *drawable,
                        PicmanPaintOptions *paint_options,
                        const PicmanCoords *coords)
{
  PicmanDodgeBurnOptions *options   = PICMAN_DODGE_BURN_OPTIONS (paint_options);
  PicmanContext          *context   = PICMAN_CONTEXT (paint_options);
  PicmanDynamics         *dynamics  = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage            *image     = picman_item_get_image (PICMAN_ITEM (drawable));
  GeglBuffer           *paint_buffer;
  gint                  paint_buffer_x;
  gint                  paint_buffer_y;
  gdouble               fade_point;
  gdouble               opacity;
  gdouble               hardness;

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

  /*  DodgeBurn the region  */
  picman_gegl_dodgeburn (picman_paint_core_get_orig_image (paint_core),
                       GEGL_RECTANGLE (paint_buffer_x,
                                       paint_buffer_y,
                                       gegl_buffer_get_width  (paint_buffer),
                                       gegl_buffer_get_height (paint_buffer)),
                       paint_buffer,
                       GEGL_RECTANGLE (0, 0, 0, 0),
                       options->exposure / 100.0,
                       options->type,
                       options->mode);

  hardness = picman_dynamics_get_linear_value (dynamics,
                                             PICMAN_DYNAMICS_OUTPUT_HARDNESS,
                                             coords,
                                             paint_options,
                                             fade_point);

  /* Replace the newly dodgedburned area (paint_area) to the image */
  picman_brush_core_replace_canvas (PICMAN_BRUSH_CORE (paint_core), drawable,
                                  coords,
                                  MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                  picman_context_get_opacity (context),
                                  picman_paint_options_get_brush_mode (paint_options),
                                  hardness,
                                  PICMAN_PAINT_CONSTANT);
}
