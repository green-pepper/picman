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

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"

#include "picmanairbrush.h"
#include "picmanairbrushoptions.h"

#include "picman-intl.h"


static void       picman_airbrush_finalize (GObject          *object);

static void       picman_airbrush_paint    (PicmanPaintCore    *paint_core,
                                          PicmanDrawable     *drawable,
                                          PicmanPaintOptions *paint_options,
                                          const PicmanCoords *coords,
                                          PicmanPaintState    paint_state,
                                          guint32           time);
static void       picman_airbrush_motion   (PicmanPaintCore    *paint_core,
                                          PicmanDrawable     *drawable,
                                          PicmanPaintOptions *paint_options,
                                          const PicmanCoords *coords);
static gboolean   picman_airbrush_timeout  (gpointer          data);


G_DEFINE_TYPE (PicmanAirbrush, picman_airbrush, PICMAN_TYPE_PAINTBRUSH)

#define parent_class picman_airbrush_parent_class


void
picman_airbrush_register (Picman                      *picman,
                        PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_AIRBRUSH,
                PICMAN_TYPE_AIRBRUSH_OPTIONS,
                "picman-airbrush",
                _("Airbrush"),
                "picman-tool-airbrush");
}

static void
picman_airbrush_class_init (PicmanAirbrushClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);

  object_class->finalize  = picman_airbrush_finalize;

  paint_core_class->paint = picman_airbrush_paint;
}

static void
picman_airbrush_init (PicmanAirbrush *airbrush)
{
  airbrush->timeout_id = 0;
}

static void
picman_airbrush_finalize (GObject *object)
{
  PicmanAirbrush *airbrush = PICMAN_AIRBRUSH (object);

  if (airbrush->timeout_id)
    {
      g_source_remove (airbrush->timeout_id);
      airbrush->timeout_id = 0;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_airbrush_paint (PicmanPaintCore    *paint_core,
                     PicmanDrawable     *drawable,
                     PicmanPaintOptions *paint_options,
                     const PicmanCoords *coords,
                     PicmanPaintState    paint_state,
                     guint32           time)
{
  PicmanAirbrush        *airbrush = PICMAN_AIRBRUSH (paint_core);
  PicmanAirbrushOptions *options  = PICMAN_AIRBRUSH_OPTIONS (paint_options);
  PicmanDynamics        *dynamics = PICMAN_BRUSH_CORE (paint_core)->dynamics;

  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_INIT:
      if (airbrush->timeout_id)
        {
          g_source_remove (airbrush->timeout_id);
          airbrush->timeout_id = 0;
        }

      PICMAN_PAINT_CORE_CLASS (parent_class)->paint (paint_core, drawable,
                                                   paint_options,
                                                   coords,
                                                   paint_state, time);
      break;

    case PICMAN_PAINT_STATE_MOTION:
      if (airbrush->timeout_id)
        {
          g_source_remove (airbrush->timeout_id);
          airbrush->timeout_id = 0;
        }

      picman_airbrush_motion (paint_core, drawable, paint_options, coords);

      if ((options->rate != 0.0) && (!options->motion_only))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (drawable));
          gdouble    fade_point;
          gdouble    dynamic_rate;
          gint       timeout;

          fade_point = picman_paint_options_get_fade (paint_options, image,
                                                    paint_core->pixel_dist);

          airbrush->drawable      = drawable;
          airbrush->paint_options = paint_options;

          dynamic_rate = picman_dynamics_get_linear_value (dynamics,
                                                         PICMAN_DYNAMICS_OUTPUT_RATE,
                                                         coords,
                                                         paint_options,
                                                         fade_point);

          timeout = 10000 / (options->rate * dynamic_rate);

          airbrush->timeout_id = g_timeout_add (timeout,
                                                picman_airbrush_timeout,
                                                airbrush);
        }
      break;

    case PICMAN_PAINT_STATE_FINISH:
      if (airbrush->timeout_id)
        {
          g_source_remove (airbrush->timeout_id);
          airbrush->timeout_id = 0;
        }

      PICMAN_PAINT_CORE_CLASS (parent_class)->paint (paint_core, drawable,
                                                   paint_options,
                                                   coords,
                                                   paint_state, time);
      break;
    }
}

static void
picman_airbrush_motion (PicmanPaintCore    *paint_core,
                      PicmanDrawable     *drawable,
                      PicmanPaintOptions *paint_options,
                      const PicmanCoords *coords)

{
  PicmanAirbrushOptions *options  = PICMAN_AIRBRUSH_OPTIONS (paint_options);
  PicmanDynamics        *dynamics = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage           *image    = picman_item_get_image (PICMAN_ITEM (drawable));
  gdouble              opacity;
  gdouble              fade_point;

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  opacity = (options->flow / 100.0 *
             picman_dynamics_get_linear_value (dynamics,
                                             PICMAN_DYNAMICS_OUTPUT_FLOW,
                                             coords,
                                             paint_options,
                                             fade_point));

  _picman_paintbrush_motion (paint_core, drawable, paint_options, coords, opacity);
}

static gboolean
picman_airbrush_timeout (gpointer data)
{
  PicmanAirbrush *airbrush = PICMAN_AIRBRUSH (data);
  PicmanCoords    coords;

  picman_paint_core_get_current_coords (PICMAN_PAINT_CORE (airbrush), &coords);

  picman_airbrush_paint (PICMAN_PAINT_CORE (airbrush),
                       airbrush->drawable,
                       airbrush->paint_options,
                       &coords,
                       PICMAN_PAINT_STATE_MOTION, 0);

  picman_image_flush (picman_item_get_image (PICMAN_ITEM (airbrush->drawable)));

  return FALSE;
}
