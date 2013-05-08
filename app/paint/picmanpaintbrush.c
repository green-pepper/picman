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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"

#include "paint-types.h"

#include "gegl/picman-gegl-utils.h"

#include "core/picman.h"
#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmangradient.h"
#include "core/picmanimage.h"
#include "core/picmantempbuf.h"

#include "picmanpaintbrush.h"
#include "picmanpaintoptions.h"

#include "picman-intl.h"


static void   picman_paintbrush_paint (PicmanPaintCore    *paint_core,
                                     PicmanDrawable     *drawable,
                                     PicmanPaintOptions *paint_options,
                                     const PicmanCoords *coords,
                                     PicmanPaintState    paint_state,
                                     guint32           time);


G_DEFINE_TYPE (PicmanPaintbrush, picman_paintbrush, PICMAN_TYPE_BRUSH_CORE)


void
picman_paintbrush_register (Picman                      *picman,
                          PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_PAINTBRUSH,
                PICMAN_TYPE_PAINT_OPTIONS,
                "picman-paintbrush",
                _("Paintbrush"),
                "picman-tool-paintbrush");
}

static void
picman_paintbrush_class_init (PicmanPaintbrushClass *klass)
{
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanBrushCoreClass *brush_core_class = PICMAN_BRUSH_CORE_CLASS (klass);

  paint_core_class->paint                  = picman_paintbrush_paint;

  brush_core_class->handles_changing_brush = TRUE;
}

static void
picman_paintbrush_init (PicmanPaintbrush *paintbrush)
{
}

static void
picman_paintbrush_paint (PicmanPaintCore    *paint_core,
                       PicmanDrawable     *drawable,
                       PicmanPaintOptions *paint_options,
                       const PicmanCoords *coords,
                       PicmanPaintState    paint_state,
                       guint32           time)
{
  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_MOTION:
      _picman_paintbrush_motion (paint_core, drawable, paint_options, coords,
                               PICMAN_OPACITY_OPAQUE);
      break;

    default:
      break;
    }
}

void
_picman_paintbrush_motion (PicmanPaintCore    *paint_core,
                         PicmanDrawable     *drawable,
                         PicmanPaintOptions *paint_options,
                         const PicmanCoords *coords,
                         gdouble           opacity)
{
  PicmanBrushCore            *brush_core = PICMAN_BRUSH_CORE (paint_core);
  PicmanContext              *context    = PICMAN_CONTEXT (paint_options);
  PicmanDynamics             *dynamics   = brush_core->dynamics;
  PicmanImage                *image;
  PicmanRGB                   gradient_color;
  GeglBuffer               *paint_buffer;
  gint                      paint_buffer_x;
  gint                      paint_buffer_y;
  PicmanPaintApplicationMode  paint_appl_mode;
  gdouble                   fade_point;
  gdouble                   grad_point;
  gdouble                   force;

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  opacity *= picman_dynamics_get_linear_value (dynamics,
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

  paint_appl_mode = paint_options->application_mode;

  grad_point = picman_dynamics_get_linear_value (dynamics,
                                               PICMAN_DYNAMICS_OUTPUT_COLOR,
                                               coords,
                                               paint_options,
                                               fade_point);

  if (picman_paint_options_get_gradient_color (paint_options, image,
                                             grad_point,
                                             paint_core->pixel_dist,
                                             &gradient_color))
    {
      /* optionally take the color from the current gradient */

      GeglColor *color;

      opacity *= gradient_color.a;
      picman_rgb_set_alpha (&gradient_color, PICMAN_OPACITY_OPAQUE);

      color = picman_gegl_color_new (&gradient_color);

      gegl_buffer_set_color (paint_buffer, NULL, color);
      g_object_unref (color);

      paint_appl_mode = PICMAN_PAINT_INCREMENTAL;
    }
  else if (brush_core->brush && brush_core->brush->pixmap)
    {
      /* otherwise check if the brush has a pixmap and use that to
       * color the area
       */
      picman_brush_core_color_area_with_pixmap (brush_core, drawable,
                                              coords,
                                              paint_buffer,
                                              paint_buffer_x,
                                              paint_buffer_y,
                                              picman_paint_options_get_brush_mode (paint_options));

      paint_appl_mode = PICMAN_PAINT_INCREMENTAL;
    }
  else
    {
      /* otherwise fill the area with the foreground color */

      PicmanRGB    foreground;
      GeglColor *color;

      picman_context_get_foreground (context, &foreground);
      color = picman_gegl_color_new (&foreground);

      gegl_buffer_set_color (paint_buffer, NULL, color);
      g_object_unref (color);
    }

  force = picman_dynamics_get_linear_value (dynamics,
                                          PICMAN_DYNAMICS_OUTPUT_FORCE,
                                          coords,
                                          paint_options,
                                          fade_point);

  /* finally, let the brush core paste the colored area on the canvas */
  picman_brush_core_paste_canvas (brush_core, drawable,
                                coords,
                                MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                picman_context_get_opacity (context),
                                picman_context_get_paint_mode (context),
                                picman_paint_options_get_brush_mode (paint_options),
                                force,
                                paint_appl_mode);
}
