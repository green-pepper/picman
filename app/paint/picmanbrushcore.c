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

#include <string.h>

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "gegl/picman-babl.h"

#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmandynamicsoutput.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanmarshal.h"
#include "core/picmantempbuf.h"

#include "picmanbrushcore.h"
#include "picmanbrushcore-kernels.h"

#include "picmanpaintoptions.h"

#include "picman-intl.h"


#define EPSILON  0.00001

enum
{
  SET_BRUSH,
  SET_DYNAMICS,
  LAST_SIGNAL
};


/*  local function prototypes  */

static void      picman_brush_core_finalize           (GObject          *object);

static gboolean  picman_brush_core_start              (PicmanPaintCore    *core,
                                                    PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     const PicmanCoords *coords,
                                                     GError          **error);
static gboolean  picman_brush_core_pre_paint          (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     PicmanPaintState    paint_state,
                                                     guint32           time);
static void      picman_brush_core_post_paint         (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     PicmanPaintState    paint_state,
                                                     guint32           time);
static void      picman_brush_core_interpolate        (PicmanPaintCore    *core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     guint32           time);

static GeglBuffer * picman_brush_core_get_paint_buffer(PicmanPaintCore    *paint_core,
                                                     PicmanDrawable     *drawable,
                                                     PicmanPaintOptions *paint_options,
                                                     const PicmanCoords *coords,
                                                     gint             *paint_buffer_x,
                                                     gint             *paint_buffer_y);

static void      picman_brush_core_real_set_brush     (PicmanBrushCore    *core,
                                                     PicmanBrush        *brush);
static void      picman_brush_core_real_set_dynamics  (PicmanBrushCore    *core,
                                                     PicmanDynamics     *dynamics);

static const PicmanTempBuf *
                 picman_brush_core_subsample_mask     (PicmanBrushCore     *core,
                                                     const PicmanTempBuf *mask,
                                                     gdouble            x,
                                                     gdouble            y);
static const PicmanTempBuf *
                 picman_brush_core_pressurize_mask    (PicmanBrushCore     *core,
                                                     const PicmanTempBuf *brush_mask,
                                                     gdouble            x,
                                                     gdouble            y,
                                                     gdouble            pressure);
static const PicmanTempBuf *
                 picman_brush_core_solidify_mask      (PicmanBrushCore     *core,
                                                     const PicmanTempBuf *brush_mask,
                                                     gdouble            x,
                                                     gdouble            y);
static const PicmanTempBuf *
                 picman_brush_core_transform_mask     (PicmanBrushCore     *core,
                                                     PicmanBrush         *brush);
static const PicmanTempBuf *
                 picman_brush_core_transform_pixmap   (PicmanBrushCore     *core,
                                                     PicmanBrush         *brush);

static void      picman_brush_core_invalidate_cache   (PicmanBrush         *brush,
                                                     PicmanBrushCore     *core);

/*  brush pipe utility functions  */
static void  picman_brush_core_paint_line_pixmap_mask (PicmanDrawable      *drawable,
                                                     const PicmanTempBuf *pixmap_mask,
                                                     const PicmanTempBuf *brush_mask,
                                                     gfloat            *d,
                                                     gint               x,
                                                     gint               y,
                                                     gint               width,
                                                     PicmanBrushApplicationMode  mode);


G_DEFINE_TYPE (PicmanBrushCore, picman_brush_core, PICMAN_TYPE_PAINT_CORE)

#define parent_class picman_brush_core_parent_class

static guint core_signals[LAST_SIGNAL] = { 0, };


static void
picman_brush_core_class_init (PicmanBrushCoreClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);

  core_signals[SET_BRUSH] =
    g_signal_new ("set-brush",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanBrushCoreClass, set_brush),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_BRUSH);

  core_signals[SET_DYNAMICS] =
    g_signal_new ("set-dynamics",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanBrushCoreClass, set_dynamics),
                  NULL, NULL,
                  picman_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  PICMAN_TYPE_DYNAMICS);

  object_class->finalize                    = picman_brush_core_finalize;

  paint_core_class->start                   = picman_brush_core_start;
  paint_core_class->pre_paint               = picman_brush_core_pre_paint;
  paint_core_class->post_paint              = picman_brush_core_post_paint;
  paint_core_class->interpolate             = picman_brush_core_interpolate;
  paint_core_class->get_paint_buffer        = picman_brush_core_get_paint_buffer;

  klass->handles_changing_brush             = FALSE;
  klass->handles_transforming_brush         = TRUE;
  klass->handles_dynamic_transforming_brush = TRUE;

  klass->set_brush                          = picman_brush_core_real_set_brush;
  klass->set_dynamics                       = picman_brush_core_real_set_dynamics;
}

static void
picman_brush_core_init (PicmanBrushCore *core)
{
  gint i, j;

  core->main_brush                   = NULL;
  core->brush                        = NULL;
  core->dynamics                     = NULL;
  core->spacing                      = 1.0;
  core->scale                        = 1.0;
  core->angle                        = 1.0;
  core->hardness                     = 1.0;
  core->aspect_ratio                 = 0.0;

  core->pressure_brush               = NULL;

  core->last_solid_brush_mask        = NULL;
  core->solid_cache_invalid          = FALSE;

  core->transform_brush              = NULL;
  core->transform_pixmap             = NULL;

  core->last_subsample_brush_mask    = NULL;
  core->subsample_cache_invalid      = FALSE;

  core->rand                         = g_rand_new ();

  for (i = 0; i < BRUSH_CORE_SOLID_SUBSAMPLE; i++)
    {
      for (j = 0; j < BRUSH_CORE_SOLID_SUBSAMPLE; j++)
        {
          core->solid_brushes[i][j] = NULL;
        }
    }

  for (i = 0; i < BRUSH_CORE_JITTER_LUTSIZE - 1; ++i)
    {
      core->jitter_lut_y[i] = cos (picman_deg_to_rad (i * 360 /
                                                    BRUSH_CORE_JITTER_LUTSIZE));
      core->jitter_lut_x[i] = sin (picman_deg_to_rad (i * 360 /
                                                    BRUSH_CORE_JITTER_LUTSIZE));
    }

  g_assert (BRUSH_CORE_SUBSAMPLE == KERNEL_SUBSAMPLE);

  for (i = 0; i < KERNEL_SUBSAMPLE + 1; i++)
    {
      for (j = 0; j < KERNEL_SUBSAMPLE + 1; j++)
        {
          core->subsample_brushes[i][j] = NULL;
        }
    }
}

static void
picman_brush_core_finalize (GObject *object)
{
  PicmanBrushCore *core = PICMAN_BRUSH_CORE (object);
  gint           i, j;

  if (core->pressure_brush)
    {
      picman_temp_buf_unref (core->pressure_brush);
      core->pressure_brush = NULL;
    }

  for (i = 0; i < BRUSH_CORE_SOLID_SUBSAMPLE; i++)
    for (j = 0; j < BRUSH_CORE_SOLID_SUBSAMPLE; j++)
      if (core->solid_brushes[i][j])
        {
          picman_temp_buf_unref (core->solid_brushes[i][j]);
          core->solid_brushes[i][j] = NULL;
        }

  if (core->rand)
    {
      g_rand_free (core->rand);
      core->rand = NULL;
    }

  for (i = 0; i < KERNEL_SUBSAMPLE + 1; i++)
    for (j = 0; j < KERNEL_SUBSAMPLE + 1; j++)
      if (core->subsample_brushes[i][j])
        {
          picman_temp_buf_unref (core->subsample_brushes[i][j]);
          core->subsample_brushes[i][j] = NULL;
        }

  if (core->main_brush)
    {
      g_signal_handlers_disconnect_by_func (core->main_brush,
                                            picman_brush_core_invalidate_cache,
                                            core);
      picman_brush_end_use (core->main_brush);
      g_object_unref (core->main_brush);
      core->main_brush = NULL;
    }

  if (core->dynamics)
    {
      g_object_unref (core->dynamics);
      core->dynamics = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
picman_brush_core_pre_paint (PicmanPaintCore    *paint_core,
                           PicmanDrawable     *drawable,
                           PicmanPaintOptions *paint_options,
                           PicmanPaintState    paint_state,
                           guint32           time)
{
  PicmanBrushCore *core = PICMAN_BRUSH_CORE (paint_core);

  if (paint_state == PICMAN_PAINT_STATE_MOTION)
    {
      PicmanCoords last_coords;
      PicmanCoords current_coords;
      gdouble scale;

      picman_paint_core_get_last_coords (paint_core, &last_coords);
      picman_paint_core_get_current_coords (paint_core, &current_coords);

      /* If we current point == last point, check if the brush
       * wants to be painted in that case. (Direction dependent
       * pixmap brush pipes don't, as they don't know which
       * pixmap to select.)
       */
      if (last_coords.x == current_coords.x &&
          last_coords.y == current_coords.y &&
          ! picman_brush_want_null_motion (core->main_brush,
                                         &last_coords,
                                         &current_coords))
        {
          return FALSE;
        }
      /*No drawing anything if the scale is too small*/
      if (PICMAN_BRUSH_CORE_GET_CLASS (core)->handles_transforming_brush)
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (drawable));
          gdouble    fade_point;

          if (PICMAN_BRUSH_CORE_GET_CLASS (core)->handles_dynamic_transforming_brush)
            {
              fade_point = picman_paint_options_get_fade (paint_options, image,
                                                        paint_core->pixel_dist);

              scale = paint_options->brush_size /
                      MAX (picman_temp_buf_get_width  (core->main_brush->mask),
                           picman_temp_buf_get_height (core->main_brush->mask)) *
                      picman_dynamics_get_linear_value (core->dynamics,
                                                      PICMAN_DYNAMICS_OUTPUT_SIZE,
                                                      &current_coords,
                                                      paint_options,
                                                      fade_point);

              if (scale < 0.0000001)
                return FALSE;
            }
        }

      if (PICMAN_BRUSH_CORE_GET_CLASS (paint_core)->handles_changing_brush)
        {
          core->brush = picman_brush_select_brush (core->main_brush,
                                                 &last_coords,
                                                 &current_coords);
        }
    }

  return TRUE;
}

static void
picman_brush_core_post_paint (PicmanPaintCore    *paint_core,
                            PicmanDrawable     *drawable,
                            PicmanPaintOptions *paint_options,
                            PicmanPaintState    paint_state,
                            guint32           time)
{
  PicmanBrushCore *core = PICMAN_BRUSH_CORE (paint_core);

  if (paint_state == PICMAN_PAINT_STATE_MOTION)
    {
      core->brush = core->main_brush;
    }
}

static gboolean
picman_brush_core_start (PicmanPaintCore     *paint_core,
                       PicmanDrawable      *drawable,
                       PicmanPaintOptions  *paint_options,
                       const PicmanCoords  *coords,
                       GError           **error)
{
  PicmanBrushCore *core    = PICMAN_BRUSH_CORE (paint_core);
  PicmanContext   *context = PICMAN_CONTEXT (paint_options);

  picman_brush_core_set_brush (core, picman_context_get_brush (context));

  picman_brush_core_set_dynamics (core, picman_context_get_dynamics (context));

  if (! core->main_brush)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("No brushes available for use with this tool."));
      return FALSE;
    }

  if (! core->dynamics)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("No paint dynamics available for use with this tool."));
      return FALSE;
    }

  if (PICMAN_BRUSH_CORE_GET_CLASS (core)->handles_transforming_brush)
    {
      picman_brush_core_eval_transform_dynamics (core,
                                               drawable,
                                               paint_options,
                                               coords);
    }

  core->spacing = (gdouble) picman_brush_get_spacing (core->main_brush) / 100.0;

  core->brush = core->main_brush;

  core->jitter =
    picman_paint_options_get_jitter (paint_options,
                                   picman_item_get_image (PICMAN_ITEM (drawable)));

  return TRUE;
}

/**
 * picman_avoid_exact_integer
 * @x: points to a gdouble
 *
 * Adjusts *x such that it is not too close to an integer. This is used
 * for decision algorithms that would be vulnerable to rounding glitches
 * if exact integers were input.
 *
 * Side effects: Changes the value of *x
 **/
static void
picman_avoid_exact_integer (gdouble *x)
{
  const gdouble integral   = floor (*x);
  const gdouble fractional = *x - integral;

  if (fractional < EPSILON)
    {
      *x = integral + EPSILON;
    }
  else if (fractional > (1 -EPSILON))
    {
      *x = integral + (1 - EPSILON);
    }
}

static void
picman_brush_core_interpolate (PicmanPaintCore    *paint_core,
                             PicmanDrawable     *drawable,
                             PicmanPaintOptions *paint_options,
                             guint32           time)
{
  PicmanBrushCore      *core  = PICMAN_BRUSH_CORE (paint_core);
  PicmanImage          *image = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanDynamicsOutput *spacing_output;
  PicmanCoords          last_coords;
  PicmanCoords          current_coords;
  PicmanVector2         delta_vec;
  gdouble             delta_pressure;
  gdouble             delta_xtilt, delta_ytilt;
  gdouble             delta_wheel;
  gdouble             delta_velocity;
  gdouble             temp_direction;
  PicmanVector2         temp_vec;
  gint                n, num_points;
  gdouble             t0, dt, tn;
  gdouble             st_factor, st_offset;
  gdouble             initial;
  gdouble             dist;
  gdouble             total;
  gdouble             pixel_dist;
  gdouble             pixel_initial;
  gdouble             xd, yd;
  gdouble             mag;
  gdouble             dyn_spacing = core->spacing;
  gdouble             fade_point;
  gboolean            use_dyn_spacing;

  g_return_if_fail (PICMAN_IS_BRUSH (core->brush));

  picman_paint_core_get_last_coords (paint_core, &last_coords);
  picman_paint_core_get_current_coords (paint_core, &current_coords);

  picman_avoid_exact_integer (&last_coords.x);
  picman_avoid_exact_integer (&last_coords.y);
  picman_avoid_exact_integer (&current_coords.x);
  picman_avoid_exact_integer (&current_coords.y);

  delta_vec.x    = current_coords.x        - last_coords.x;
  delta_vec.y    = current_coords.y        - last_coords.y;
  delta_pressure = current_coords.pressure - last_coords.pressure;
  delta_xtilt    = current_coords.xtilt    - last_coords.xtilt;
  delta_ytilt    = current_coords.ytilt    - last_coords.ytilt;
  delta_wheel    = current_coords.wheel    - last_coords.wheel;
  delta_velocity = current_coords.velocity - last_coords.velocity;
  temp_direction = current_coords.direction;

  /*  return if there has been no motion  */
  if (! delta_vec.x    &&
      ! delta_vec.y    &&
      ! delta_pressure &&
      ! delta_xtilt    &&
      ! delta_ytilt    &&
      ! delta_wheel    &&
      ! delta_velocity)
    return;

  pixel_dist    = picman_vector2_length (&delta_vec);
  pixel_initial = paint_core->pixel_dist;

  /*  Zero sized brushes are unfit for interpolate, so we just let
   *  paint core fail on its own
   */
  if (core->scale == 0.0)
    {
      picman_paint_core_set_last_coords (paint_core, &current_coords);

      picman_paint_core_paint (paint_core, drawable, paint_options,
                             PICMAN_PAINT_STATE_MOTION, time);

      paint_core->pixel_dist = pixel_initial + pixel_dist; /* Dont forget to update pixel distance*/

      return;
    }

  /* Handle dynamic spacing */
  spacing_output = picman_dynamics_get_output (core->dynamics,
                                             PICMAN_DYNAMICS_OUTPUT_SPACING);

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  use_dyn_spacing = picman_dynamics_output_is_enabled (spacing_output);

  if (use_dyn_spacing)
    {
      dyn_spacing = picman_dynamics_output_get_linear_value (spacing_output,
                                                           &current_coords,
                                                           paint_options,
                                                           fade_point);

      /* Dynamic spacing assumes that the value set in core is the min
       * value and the max is full 200% spacing. This approach differs
       * from the usual factor from user input approach because making
       * spacing smaller than the nominal value is unlikely and
       * spacing has a hard defined max.
       */
      dyn_spacing = (core->spacing +
                     ((2.0 - core->spacing) * ( 1.0 - dyn_spacing)));

      /*  Limiting spacing to minimum 1%  */
      dyn_spacing = MAX (core->spacing, dyn_spacing);
    }

  /* calculate the distance traveled in the coordinate space of the brush */
  temp_vec = core->brush->x_axis;
  picman_vector2_mul (&temp_vec, core->scale);
  picman_vector2_rotate (&temp_vec, core->angle * G_PI * 2);

  mag = picman_vector2_length (&temp_vec);
  xd  = picman_vector2_inner_product (&delta_vec, &temp_vec) / (mag * mag);

  temp_vec = core->brush->y_axis;
  picman_vector2_mul (&temp_vec, core->scale);
  picman_vector2_rotate (&temp_vec, core->angle * G_PI * 2);

  mag = picman_vector2_length (&temp_vec);
  yd  = picman_vector2_inner_product (&delta_vec, &temp_vec) / (mag * mag);

  dist    = 0.5 * sqrt (xd * xd + yd * yd);
  total   = dist + paint_core->distance;
  initial = paint_core->distance;


  if (delta_vec.x * delta_vec.x > delta_vec.y * delta_vec.y)
    {
      st_factor = delta_vec.x;
      st_offset = last_coords.x - 0.5;
    }
  else
    {
      st_factor = delta_vec.y;
      st_offset = last_coords.y - 0.5;
    }

  if (use_dyn_spacing)
    {
      gint s0;

      num_points = dist / dyn_spacing;

      s0 = (gint) floor (st_offset + 0.5);
      t0 = (s0 - st_offset) / st_factor;
      dt = dyn_spacing / dist;

      if (num_points == 0)
        return;
    }
  else if (fabs (st_factor) > dist / core->spacing)
    {
      /*  The stripe principle leads to brush positions that are spaced
       *  *closer* than the official brush spacing. Use the official
       *  spacing instead. This is the common case when the brush spacing
       *  is large.
       *  The net effect is then to put a lower bound on the spacing, but
       *  one that varies with the slope of the line. This is suppose to
       *  make thin lines (say, with a 1x1 brush) prettier while leaving
       *  lines with larger brush spacing as they used to look in 1.2.x.
       */

      dt = core->spacing / dist;
      n = (gint) (initial / core->spacing + 1.0 + EPSILON);
      t0 = (n * core->spacing - initial) / dist;
      num_points = 1 + (gint) floor ((1 + EPSILON - t0) / dt);

      /* if we arnt going to paint anything this time and the brush
       * has only moved on one axis return without updating the brush
       * position, distance etc. so that we can more accurately space
       * brush strokes when curves are supplied to us in single pixel
       * chunks.
       */

      if (num_points == 0 && (delta_vec.x == 0 || delta_vec.y == 0))
        return;
    }
  else if (fabs (st_factor) < EPSILON)
    {
      /* Hm, we've hardly moved at all. Don't draw anything, but reset the
       * old coordinates and hope we've gone longer the next time...
       */
      current_coords.x = last_coords.x;
      current_coords.y = last_coords.y;

      picman_paint_core_set_current_coords (paint_core, &current_coords);

      /* ... but go along with the current pressure, tilt and wheel */
      return;
    }
  else
    {
      gint direction = st_factor > 0 ? 1 : -1;
      gint x, y;
      gint s0, sn;

      /*  Choose the first and last stripe to paint.
       *    FIRST PRIORITY is to avoid gaps painting with a 1x1 aliasing
       *  brush when a horizontalish line segment follows a verticalish
       *  one or vice versa - no matter what the angle between the two
       *  lines is. This will also limit the local thinning that a 1x1
       *  subsampled brush may suffer in the same situation.
       *    SECOND PRIORITY is to avoid making free-hand drawings
       *  unpleasantly fat by plotting redundant points.
       *    These are achieved by the following rules, but it is a little
       *  tricky to see just why. Do not change this algorithm unless you
       *  are sure you know what you're doing!
       */

      /*  Basic case: round the beginning and ending point to nearest
       *  stripe center.
       */
      s0 = (gint) floor (st_offset + 0.5);
      sn = (gint) floor (st_offset + st_factor + 0.5);

      t0 = (s0 - st_offset) / st_factor;
      tn = (sn - st_offset) / st_factor;

      x = (gint) floor (last_coords.x + t0 * delta_vec.x);
      y = (gint) floor (last_coords.y + t0 * delta_vec.y);

      if (t0 < 0.0 && !( x == (gint) floor (last_coords.x) &&
                         y == (gint) floor (last_coords.y) ))
        {
          /*  Exception A: If the first stripe's brush position is
           *  EXTRApolated into a different pixel square than the
           *  ideal starting point, dont't plot it.
           */
          s0 += direction;
        }
      else if (x == (gint) floor (paint_core->last_paint.x) &&
               y == (gint) floor (paint_core->last_paint.y))
        {
          /*  Exception B: If first stripe's brush position is within the
           *  same pixel square as the last plot of the previous line,
           *  don't plot it either.
           */
          s0 += direction;
        }

      x = (gint) floor (last_coords.x + tn * delta_vec.x);
      y = (gint) floor (last_coords.y + tn * delta_vec.y);

      if (tn > 1.0 && !( x == (gint) floor (current_coords.x) &&
                         y == (gint) floor (current_coords.y)))
        {
          /*  Exception C: If the last stripe's brush position is
           *  EXTRApolated into a different pixel square than the
           *  ideal ending point, don't plot it.
           */
          sn -= direction;
        }

      t0 = (s0 - st_offset) / st_factor;
      tn = (sn - st_offset) / st_factor;
      dt         =     direction * 1.0 / st_factor;
      num_points = 1 + direction * (sn - s0);

      if (num_points >= 1)
        {
          /*  Hack the reported total distance such that it looks to the
           *  next line as if the the last pixel plotted were at an integer
           *  multiple of the brush spacing. This helps prevent artifacts
           *  for connected lines when the brush spacing is such that some
           *  slopes will use the stripe regime and other slopes will use
           *  the nominal brush spacing.
           */

          if (tn < 1)
            total = initial + tn * dist;

          total = core->spacing * (gint) (total / core->spacing + 0.5);
          total += (1.0 - tn) * dist;
        }
    }

  for (n = 0; n < num_points; n++)
    {
      gdouble t = t0 + n * dt;
      gdouble p = (gdouble) n / num_points;

      current_coords.x         = last_coords.x        + t * delta_vec.x;
      current_coords.y         = last_coords.y        + t * delta_vec.y;
      current_coords.pressure  = last_coords.pressure + p * delta_pressure;
      current_coords.xtilt     = last_coords.xtilt    + p * delta_xtilt;
      current_coords.ytilt     = last_coords.ytilt    + p * delta_ytilt;
      current_coords.wheel     = last_coords.wheel    + p * delta_wheel;
      current_coords.velocity  = last_coords.velocity + p * delta_velocity;
      current_coords.direction = temp_direction;

      if (core->jitter > 0.0)
        {
          gdouble dyn_jitter;
          gdouble jitter_dist;
          gint32  jitter_angle;

          dyn_jitter = (core->jitter *
                        picman_dynamics_get_linear_value (core->dynamics,
                                                        PICMAN_DYNAMICS_OUTPUT_JITTER,
                                                        &current_coords,
                                                        paint_options,
                                                        fade_point));

          jitter_dist  = g_rand_double_range (core->rand, 0, dyn_jitter);
          jitter_angle = g_rand_int_range (core->rand,
                                           0, BRUSH_CORE_JITTER_LUTSIZE);

          current_coords.x +=
            (core->brush->x_axis.x + core->brush->y_axis.x) *
            jitter_dist * core->jitter_lut_x[jitter_angle] * core->scale;

          current_coords.y +=
            (core->brush->y_axis.y + core->brush->x_axis.y) *
            jitter_dist * core->jitter_lut_y[jitter_angle] * core->scale;
        }

      picman_paint_core_set_current_coords (paint_core, &current_coords);

      paint_core->distance   = initial       + t * dist;
      paint_core->pixel_dist = pixel_initial + t * pixel_dist;

      picman_paint_core_paint (paint_core, drawable, paint_options,
                             PICMAN_PAINT_STATE_MOTION, time);
    }

  current_coords.x        = last_coords.x        + delta_vec.x;
  current_coords.y        = last_coords.y        + delta_vec.y;
  current_coords.pressure = last_coords.pressure + delta_pressure;
  current_coords.xtilt    = last_coords.xtilt    + delta_xtilt;
  current_coords.ytilt    = last_coords.ytilt    + delta_ytilt;
  current_coords.wheel    = last_coords.wheel    + delta_wheel;
  current_coords.velocity = last_coords.velocity + delta_velocity;

  picman_paint_core_set_current_coords (paint_core, &current_coords);
  picman_paint_core_set_last_coords (paint_core, &current_coords);

  paint_core->distance   = total;
  paint_core->pixel_dist = pixel_initial + pixel_dist;
}

static GeglBuffer *
picman_brush_core_get_paint_buffer (PicmanPaintCore    *paint_core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  const PicmanCoords *coords,
                                  gint             *paint_buffer_x,
                                  gint             *paint_buffer_y)
{
  PicmanBrushCore *core = PICMAN_BRUSH_CORE (paint_core);
  gint           x, y;
  gint           x1, y1, x2, y2;
  gint           drawable_width, drawable_height;
  gint           brush_width, brush_height;

  if (PICMAN_BRUSH_CORE_GET_CLASS (core)->handles_transforming_brush)
    {
      picman_brush_core_eval_transform_dynamics (core,
                                               drawable,
                                               paint_options,
                                               coords);
    }

  picman_brush_transform_size (core->brush,
                             core->scale, core->aspect_ratio, core->angle,
                             &brush_width, &brush_height);

  /*  adjust the x and y coordinates to the upper left corner of the brush  */
  x = (gint) floor (coords->x) - (brush_width  / 2);
  y = (gint) floor (coords->y) - (brush_height / 2);

  drawable_width  = picman_item_get_width  (PICMAN_ITEM (drawable));
  drawable_height = picman_item_get_height (PICMAN_ITEM (drawable));

  x1 = CLAMP (x - 1, 0, drawable_width);
  y1 = CLAMP (y - 1, 0, drawable_height);
  x2 = CLAMP (x + brush_width  + 1, 0, drawable_width);
  y2 = CLAMP (y + brush_height + 1, 0, drawable_height);

  /*  configure the canvas buffer  */
  if ((x2 - x1) && (y2 - y1))
    {
      PicmanTempBuf *temp_buf;

      if (paint_core->paint_buffer                                       &&
          gegl_buffer_get_width  (paint_core->paint_buffer) == (x2 - x1) &&
          gegl_buffer_get_height (paint_core->paint_buffer) == (y2 - y1))
        {
          *paint_buffer_x = x1;
          *paint_buffer_y = y1;

          return paint_core->paint_buffer;
        }

      temp_buf = picman_temp_buf_new ((x2 - x1), (y2 - y1),
                                    babl_format ("RGBA float"));

      *paint_buffer_x = x1;
      *paint_buffer_y = y1;

      if (paint_core->paint_buffer)
        g_object_unref (paint_core->paint_buffer);

      paint_core->paint_buffer = picman_temp_buf_create_buffer (temp_buf);

      picman_temp_buf_unref (temp_buf);

      return paint_core->paint_buffer;
    }

  return NULL;
}

static void
picman_brush_core_real_set_brush (PicmanBrushCore *core,
                                PicmanBrush     *brush)
{
  if (brush == core->main_brush)
    return;

  if (core->main_brush)
    {
      g_signal_handlers_disconnect_by_func (core->main_brush,
                                            picman_brush_core_invalidate_cache,
                                            core);
      picman_brush_end_use (core->main_brush);
      g_object_unref (core->main_brush);
      core->main_brush = NULL;
    }

  core->main_brush = brush;

  if (core->main_brush)
    {
      g_object_ref (core->main_brush);
      picman_brush_begin_use (core->main_brush);
      g_signal_connect (core->main_brush, "invalidate-preview",
                        G_CALLBACK (picman_brush_core_invalidate_cache),
                        core);
    }
}

static void
picman_brush_core_real_set_dynamics (PicmanBrushCore *core,
                                   PicmanDynamics  *dynamics)
{
  if (dynamics == core->dynamics)
    return;

  if (core->dynamics)
    g_object_unref (core->dynamics);

  core->dynamics = dynamics;

  if (core->dynamics)
    g_object_ref (core->dynamics);
}

void
picman_brush_core_set_brush (PicmanBrushCore *core,
                           PicmanBrush     *brush)
{
  g_return_if_fail (PICMAN_IS_BRUSH_CORE (core));
  g_return_if_fail (brush == NULL || PICMAN_IS_BRUSH (brush));

  if (brush != core->main_brush)
    g_signal_emit (core, core_signals[SET_BRUSH], 0, brush);
}

void
picman_brush_core_set_dynamics (PicmanBrushCore *core,
                              PicmanDynamics  *dynamics)
{
  g_return_if_fail (PICMAN_IS_BRUSH_CORE (core));
  g_return_if_fail (dynamics == NULL || PICMAN_IS_DYNAMICS (dynamics));

  if (dynamics != core->dynamics)
    g_signal_emit (core, core_signals[SET_DYNAMICS], 0, dynamics);
}

void
picman_brush_core_paste_canvas (PicmanBrushCore            *core,
                              PicmanDrawable             *drawable,
                              const PicmanCoords         *coords,
                              gdouble                   brush_opacity,
                              gdouble                   image_opacity,
                              PicmanLayerModeEffects      paint_mode,
                              PicmanBrushApplicationMode  brush_hardness,
                              gdouble                   dynamic_force,
                              PicmanPaintApplicationMode  mode)
{
  const PicmanTempBuf *brush_mask;

  brush_mask = picman_brush_core_get_brush_mask (core, coords,
                                               brush_hardness,
                                               dynamic_force);

  if (brush_mask)
    {
      PicmanPaintCore *paint_core = PICMAN_PAINT_CORE (core);
      GeglBuffer    *paint_mask;
      gint           x;
      gint           y;
      gint           off_x;
      gint           off_y;

      x = (gint) floor (coords->x) - (picman_temp_buf_get_width  (brush_mask) >> 1);
      y = (gint) floor (coords->y) - (picman_temp_buf_get_height (brush_mask) >> 1);

      off_x = (x < 0) ? -x : 0;
      off_y = (y < 0) ? -y : 0;

      paint_mask = picman_temp_buf_create_buffer ((PicmanTempBuf *) brush_mask);

      picman_paint_core_paste (paint_core, paint_mask,
                             GEGL_RECTANGLE (off_x, off_y,
                                             gegl_buffer_get_width  (paint_core->paint_buffer),
                                             gegl_buffer_get_height (paint_core->paint_buffer)),
                             drawable,
                             brush_opacity,
                             image_opacity, paint_mode,
                             mode);

      g_object_unref (paint_mask);
    }
}

/* Similar to picman_brush_core_paste_canvas, but replaces the alpha channel
 * rather than using it to composite (i.e. transparent over opaque
 * becomes transparent rather than opauqe.
 */
void
picman_brush_core_replace_canvas (PicmanBrushCore            *core,
                                PicmanDrawable             *drawable,
                                const PicmanCoords         *coords,
                                gdouble                   brush_opacity,
                                gdouble                   image_opacity,
                                PicmanBrushApplicationMode  brush_hardness,
                                gdouble                   dynamic_force,
                                PicmanPaintApplicationMode  mode)
{
  const PicmanTempBuf *brush_mask;

  brush_mask = picman_brush_core_get_brush_mask (core, coords,
                                               brush_hardness,
                                               dynamic_force);

  if (brush_mask)
    {
      PicmanPaintCore *paint_core = PICMAN_PAINT_CORE (core);
      GeglBuffer    *paint_mask;
      gint           x;
      gint           y;
      gint           off_x;
      gint           off_y;

      x = (gint) floor (coords->x) - (picman_temp_buf_get_width  (brush_mask) >> 1);
      y = (gint) floor (coords->y) - (picman_temp_buf_get_height (brush_mask) >> 1);

      off_x = (x < 0) ? -x : 0;
      off_y = (y < 0) ? -y : 0;

      paint_mask = picman_temp_buf_create_buffer ((PicmanTempBuf *) brush_mask);

      picman_paint_core_replace (paint_core, paint_mask,
                               GEGL_RECTANGLE (off_x, off_y,
                                               gegl_buffer_get_width  (paint_core->paint_buffer),
                                               gegl_buffer_get_height (paint_core->paint_buffer)),
                               drawable,
                               brush_opacity,
                               image_opacity,
                               mode);

      g_object_unref (paint_mask);
    }
}


static void
picman_brush_core_invalidate_cache (PicmanBrush     *brush,
                                  PicmanBrushCore *core)
{
  /* Make sure we don't cache data for a brush that has changed */

  core->subsample_cache_invalid = TRUE;
  core->solid_cache_invalid     = TRUE;

  /* Notify of the brush change */

  g_signal_emit (core, core_signals[SET_BRUSH], 0, brush);
}


/************************************************************
 *             LOCAL FUNCTION DEFINITIONS                   *
 ************************************************************/

static inline void
rotate_pointers (gulong  **p,
                 guint32   n)
{
  guint32  i;
  gulong  *tmp;

  tmp = p[0];

  for (i = 0; i < n-1; i++)
    p[i] = p[i+1];

  p[i] = tmp;
}

static const PicmanTempBuf *
picman_brush_core_subsample_mask (PicmanBrushCore     *core,
                                const PicmanTempBuf *mask,
                                gdouble            x,
                                gdouble            y)
{
  PicmanTempBuf  *dest;
  gdouble       left;
  const guchar *m;
  guchar       *d;
  const gint   *k;
  gint          index1;
  gint          index2;
  gint          dest_offset_x = 0;
  gint          dest_offset_y = 0;
  const gint   *kernel;
  gint          i, j;
  gint          r, s;
  gulong       *accum[KERNEL_HEIGHT];
  gint          offs;
  gint          mask_width  = picman_temp_buf_get_width  (mask);
  gint          mask_height = picman_temp_buf_get_height (mask);
  gint          dest_width;
  gint          dest_height;

  while (x < 0)
    x += mask_width;

  left = x - floor (x);
  index1 = (gint) (left * (gdouble) (KERNEL_SUBSAMPLE + 1));

  while (y < 0)
    y += mask_height;

  left = y - floor (y);
  index2 = (gint) (left * (gdouble) (KERNEL_SUBSAMPLE + 1));


  if ((mask_width % 2) == 0)
    {
      index1 += KERNEL_SUBSAMPLE >> 1;

      if (index1 > KERNEL_SUBSAMPLE)
        {
          index1 -= KERNEL_SUBSAMPLE + 1;
          dest_offset_x = 1;
        }
    }

  if ((mask_height % 2) == 0)
    {
      index2 += KERNEL_SUBSAMPLE >> 1;

      if (index2 > KERNEL_SUBSAMPLE)
        {
          index2 -= KERNEL_SUBSAMPLE + 1;
          dest_offset_y = 1;
        }
    }


  kernel = subsample[index2][index1];

  if (mask == core->last_subsample_brush_mask &&
      ! core->subsample_cache_invalid)
    {
      if (core->subsample_brushes[index2][index1])
        return core->subsample_brushes[index2][index1];
    }
  else
    {
      for (i = 0; i < KERNEL_SUBSAMPLE + 1; i++)
        for (j = 0; j < KERNEL_SUBSAMPLE + 1; j++)
          if (core->subsample_brushes[i][j])
            {
              picman_temp_buf_unref (core->subsample_brushes[i][j]);
              core->subsample_brushes[i][j] = NULL;
            }

      core->last_subsample_brush_mask = mask;
      core->subsample_cache_invalid   = FALSE;
    }

  dest = picman_temp_buf_new (mask_width  + 2,
                            mask_height + 2,
                            picman_temp_buf_get_format (mask));
  picman_temp_buf_data_clear (dest);

  dest_width  = picman_temp_buf_get_width  (dest);
  dest_height = picman_temp_buf_get_height (dest);

  /* Allocate and initialize the accum buffer */
  for (i = 0; i < KERNEL_HEIGHT ; i++)
    accum[i] = g_new0 (gulong, dest_width + 1);

  core->subsample_brushes[index2][index1] = dest;

  m = picman_temp_buf_get_data (mask);
  for (i = 0; i < mask_height; i++)
    {
      for (j = 0; j < mask_width; j++)
        {
          k = kernel;
          for (r = 0; r < KERNEL_HEIGHT; r++)
            {
              offs = j + dest_offset_x;
              s = KERNEL_WIDTH;
              while (s--)
                accum[r][offs++] += *m * *k++;
            }
          m++;
        }

      /* store the accum buffer into the destination mask */
      d = picman_temp_buf_get_data (dest) + (i + dest_offset_y) * dest_width;
      for (j = 0; j < dest_width; j++)
        *d++ = (accum[0][j] + 127) / KERNEL_SUM;

      rotate_pointers (accum, KERNEL_HEIGHT);

      memset (accum[KERNEL_HEIGHT - 1], 0, sizeof (gulong) * dest_width);
    }

  /* store the rest of the accum buffer into the dest mask */
  while (i + dest_offset_y < dest_height)
    {
      d = picman_temp_buf_get_data (dest) + (i + dest_offset_y) * dest_width;
      for (j = 0; j < dest_width; j++)
        *d++ = (accum[0][j] + (KERNEL_SUM / 2)) / KERNEL_SUM;

      rotate_pointers (accum, KERNEL_HEIGHT);
      i++;
    }

  for (i = 0; i < KERNEL_HEIGHT ; i++)
    g_free (accum[i]);

  return dest;
}

/* #define FANCY_PRESSURE */

static const PicmanTempBuf *
picman_brush_core_pressurize_mask (PicmanBrushCore     *core,
                                 const PicmanTempBuf *brush_mask,
                                 gdouble            x,
                                 gdouble            y,
                                 gdouble            pressure)
{
  static guchar      mapi[256];
  const guchar      *source;
  guchar            *dest;
  const PicmanTempBuf *subsample_mask;
  gint               i;

  /* Get the raw subsampled mask */
  subsample_mask = picman_brush_core_subsample_mask (core,
                                                   brush_mask,
                                                   x, y);

  /* Special case pressure = 0.5 */
  if ((gint) (pressure * 100 + 0.5) == 50)
    return subsample_mask;

  if (core->pressure_brush)
    picman_temp_buf_unref (core->pressure_brush);

  core->pressure_brush =
    picman_temp_buf_new (picman_temp_buf_get_width  (brush_mask) + 2,
                       picman_temp_buf_get_height (brush_mask) + 2,
                       picman_temp_buf_get_format (brush_mask));
  picman_temp_buf_data_clear (core->pressure_brush);

#ifdef FANCY_PRESSURE

  /* Create the pressure profile
   *
   * It is: I'(I) = tanh (20 * (pressure - 0.5) * I)           : pressure > 0.5
   *        I'(I) = 1 - tanh (20 * (0.5 - pressure) * (1 - I)) : pressure < 0.5
   *
   * It looks like:
   *
   *    low pressure      medium pressure     high pressure
   *
   *         |                   /                  --
   *         |                  /                  /
   *        /                  /                  |
   *      --                  /                   |
   */
  {
    gdouble  map[256];
    gdouble  ds, s, c;

    ds = (pressure - 0.5) * (20.0 / 256.0);
    s  = 0;
    c  = 1.0;

    if (ds > 0)
      {
        for (i = 0; i < 256; i++)
          {
            map[i] = s / c;
            s += c * ds;
            c += s * ds;
          }

        for (i = 0; i < 256; i++)
          mapi[i] = (gint) (255 * map[i] / map[255]);
      }
    else
      {
        ds = -ds;

        for (i = 255; i >= 0; i--)
          {
            map[i] = s / c;
            s += c * ds;
            c += s * ds;
          }

        for (i = 0; i < 256; i++)
          mapi[i] = (gint) (255 * (1 - map[i] / map[0]));
      }
  }

#else /* ! FANCY_PRESSURE */

  {
    gdouble j, k;

    j = pressure + pressure;
    k = 0;

    for (i = 0; i < 256; i++)
      {
        if (k > 255)
          mapi[i] = 255;
        else
          mapi[i] = (guchar) k;

        k += j;
      }
  }

#endif /* FANCY_PRESSURE */

  /* Now convert the brush */

  source = picman_temp_buf_get_data (subsample_mask);
  dest   = picman_temp_buf_get_data (core->pressure_brush);

  i = picman_temp_buf_get_width  (subsample_mask) *
      picman_temp_buf_get_height (subsample_mask);

  while (i--)
    *dest++ = mapi[(*source++)];

  return core->pressure_brush;
}

static const PicmanTempBuf *
picman_brush_core_solidify_mask (PicmanBrushCore     *core,
                               const PicmanTempBuf *brush_mask,
                               gdouble            x,
                               gdouble            y)
{
  PicmanTempBuf  *dest;
  const guchar *m;
  gfloat       *d;
  gint          dest_offset_x     = 0;
  gint          dest_offset_y     = 0;
  gint          brush_mask_width  = picman_temp_buf_get_width  (brush_mask);
  gint          brush_mask_height = picman_temp_buf_get_height (brush_mask);
  gint          i, j;

  if ((brush_mask_width % 2) == 0)
    {
      while (x < 0)
        x += brush_mask_width;

      if ((x - floor (x)) >= 0.5)
        dest_offset_x++;
    }

  if ((brush_mask_height % 2) == 0)
    {
      while (y < 0)
        y += brush_mask_height;

      if ((y - floor (y)) >= 0.5)
        dest_offset_y++;
    }

  if (! core->solid_cache_invalid &&
      brush_mask == core->last_solid_brush_mask)
    {
      if (core->solid_brushes[dest_offset_y][dest_offset_x])
        return core->solid_brushes[dest_offset_y][dest_offset_x];
    }
  else
    {
      for (i = 0; i < BRUSH_CORE_SOLID_SUBSAMPLE; i++)
        for (j = 0; j < BRUSH_CORE_SOLID_SUBSAMPLE; j++)
          if (core->solid_brushes[i][j])
            {
              picman_temp_buf_unref (core->solid_brushes[i][j]);
              core->solid_brushes[i][j] = NULL;
            }

      core->last_solid_brush_mask = brush_mask;
      core->solid_cache_invalid   = FALSE;
    }

  dest = picman_temp_buf_new (brush_mask_width  + 2,
                            brush_mask_height + 2,
                            babl_format ("Y float"));
  picman_temp_buf_data_clear (dest);

  core->solid_brushes[dest_offset_y][dest_offset_x] = dest;

  m = picman_temp_buf_get_data (brush_mask);
  d = ((gfloat *) picman_temp_buf_get_data (dest) +
       ((dest_offset_y + 1) * picman_temp_buf_get_width (dest) +
        (dest_offset_x + 1)));

  for (i = 0; i < brush_mask_height; i++)
    {
      for (j = 0; j < brush_mask_width; j++)
        *d++ = (*m++) ? 1.0 : 0.0;

      d += 2;
    }

  return dest;
}

static const PicmanTempBuf *
picman_brush_core_transform_mask (PicmanBrushCore *core,
                                PicmanBrush     *brush)
{
  const PicmanTempBuf *mask;

  if (core->scale <= 0.0)
    return NULL;

  mask = picman_brush_transform_mask (brush,
                                    core->scale,
                                    core->aspect_ratio,
                                    core->angle,
                                    core->hardness);

  if (mask == core->transform_brush)
    return mask;

  core->transform_brush         = mask;
  core->subsample_cache_invalid = TRUE;
  core->solid_cache_invalid     = TRUE;

  return core->transform_brush;
}

static const PicmanTempBuf *
picman_brush_core_transform_pixmap (PicmanBrushCore *core,
                                  PicmanBrush     *brush)
{
  const PicmanTempBuf *pixmap;

  if (core->scale <= 0.0)
    return NULL;

  pixmap = picman_brush_transform_pixmap (brush,
                                        core->scale,
                                        core->aspect_ratio,
                                        core->angle,
                                        core->hardness);

  if (pixmap == core->transform_pixmap)
    return pixmap;

  core->transform_pixmap        = pixmap;
  core->subsample_cache_invalid = TRUE;

  return core->transform_pixmap;
}

const PicmanTempBuf *
picman_brush_core_get_brush_mask (PicmanBrushCore            *core,
                                const PicmanCoords         *coords,
                                PicmanBrushApplicationMode  brush_hardness,
                                gdouble                   dynamic_force)
{
  const PicmanTempBuf *mask;

  mask = picman_brush_core_transform_mask (core, core->brush);

  if (! mask)
    return NULL;

  switch (brush_hardness)
    {
    case PICMAN_BRUSH_SOFT:
      return picman_brush_core_subsample_mask (core, mask,
                                             coords->x,
                                             coords->y);
      break;

    case PICMAN_BRUSH_HARD:
      return picman_brush_core_solidify_mask (core, mask,
                                            coords->x,
                                            coords->y);
      break;

    case PICMAN_BRUSH_PRESSURE:
      return picman_brush_core_pressurize_mask (core, mask,
                                              coords->x,
                                              coords->y,
                                              dynamic_force);
      break;
    }

  g_return_val_if_reached (NULL);
}

void
picman_brush_core_eval_transform_dynamics (PicmanBrushCore     *core,
                                         PicmanDrawable      *drawable,
                                         PicmanPaintOptions  *paint_options,
                                         const PicmanCoords  *coords)
{
  if (core->main_brush)
    core->scale = paint_options->brush_size /
                  MAX (picman_temp_buf_get_width  (core->main_brush->mask),
                       picman_temp_buf_get_height (core->main_brush->mask));
  else
    core->scale = -1;

  core->angle        = paint_options->brush_angle;
  core->aspect_ratio = paint_options->brush_aspect_ratio;

  if (! PICMAN_IS_DYNAMICS (core->dynamics))
    return;

  if (PICMAN_BRUSH_CORE_GET_CLASS (core)->handles_dynamic_transforming_brush)
    {
      PicmanDynamicsOutput *output;
      gdouble             dyn_aspect_ratio = 0.0;
      gdouble             fade_point       = 1.0;

      if (drawable)
        {
          PicmanImage     *image      = picman_item_get_image (PICMAN_ITEM (drawable));
          PicmanPaintCore *paint_core = PICMAN_PAINT_CORE (core);

          fade_point = picman_paint_options_get_fade (paint_options, image,
                                                    paint_core->pixel_dist);
        }

      core->scale *= picman_dynamics_get_linear_value (core->dynamics,
                                                     PICMAN_DYNAMICS_OUTPUT_SIZE,
                                                     coords,
                                                     paint_options,
                                                     fade_point);

      core->angle += picman_dynamics_get_angular_value (core->dynamics,
                                                      PICMAN_DYNAMICS_OUTPUT_ANGLE,
                                                      coords,
                                                      paint_options,
                                                      fade_point);

      core->hardness = picman_dynamics_get_linear_value (core->dynamics,
                                                       PICMAN_DYNAMICS_OUTPUT_HARDNESS,
                                                       coords,
                                                       paint_options,
                                                       fade_point);

      output = picman_dynamics_get_output (core->dynamics,
                                         PICMAN_DYNAMICS_OUTPUT_ASPECT_RATIO);
      dyn_aspect_ratio = picman_dynamics_output_get_aspect_value (output,
                                                                coords,
                                                                paint_options,
                                                                fade_point);

      /* Zero aspect ratio is special cased to half of all ar range,
       * to force dynamics to have any effect. Forcing to full results
       * in disapearing stamp if applied to maximum.
       */
      if (picman_dynamics_output_is_enabled (output))
        {
          if (core->aspect_ratio == 0.0)
            core->aspect_ratio = 10.0 * dyn_aspect_ratio;
          else
            core->aspect_ratio *= dyn_aspect_ratio;
        }
    }
}


/**************************************************/
/*  Brush pipe utility functions                  */
/**************************************************/

void
picman_brush_core_color_area_with_pixmap (PicmanBrushCore            *core,
                                        PicmanDrawable             *drawable,
                                        const PicmanCoords         *coords,
                                        GeglBuffer               *area,
                                        gint                      area_x,
                                        gint                      area_y,
                                        PicmanBrushApplicationMode  mode)
{
  GeglBufferIterator *iter;
  GeglRectangle      *roi;
  gint                ulx;
  gint                uly;
  gint                offsetx;
  gint                offsety;
  const PicmanTempBuf  *pixmap_mask;
  const PicmanTempBuf  *brush_mask;

  g_return_if_fail (PICMAN_IS_BRUSH (core->brush));
  g_return_if_fail (core->brush->pixmap != NULL);

  /*  scale the brushes  */
  pixmap_mask = picman_brush_core_transform_pixmap (core, core->brush);

  if (! pixmap_mask)
    return;

  if (mode != PICMAN_BRUSH_HARD)
    brush_mask = picman_brush_core_transform_mask (core, core->brush);
  else
    brush_mask = NULL;

  /*  Calculate upper left corner of brush as in
   *  picman_paint_core_get_paint_area.  Ugly to have to do this here, too.
   */
  ulx = (gint) floor (coords->x) - (picman_temp_buf_get_width  (pixmap_mask) >> 1);
  uly = (gint) floor (coords->y) - (picman_temp_buf_get_height (pixmap_mask) >> 1);

  /*  Not sure why this is necessary, but empirically the code does
   *  not work without it for even-sided brushes.  See bug #166622.
   */
  if (picman_temp_buf_get_width (pixmap_mask) % 2 == 0)
    ulx += ROUND (coords->x) - floor (coords->x);
  if (picman_temp_buf_get_height (pixmap_mask) % 2 == 0)
    uly += ROUND (coords->y) - floor (coords->y);

  offsetx = area_x - ulx;
  offsety = area_y - uly;

  iter = gegl_buffer_iterator_new (area, NULL, 0,
                                   babl_format ("RGBA float"),
                                   GEGL_BUFFER_WRITE, GEGL_ABYSS_NONE);
  roi = &iter->roi[0];

  while (gegl_buffer_iterator_next (iter))
    {
      gfloat *d = iter->data[0];
      gint    y;

      for (y = 0; y < roi->height; y++)
        {
          picman_brush_core_paint_line_pixmap_mask (drawable,
                                                  pixmap_mask, brush_mask,
                                                  d, offsetx, y + offsety,
                                                  roi->width, mode);
          d += roi->width * 4;
        }
    }
}

static void
picman_brush_core_paint_line_pixmap_mask (PicmanDrawable             *drawable,
                                        const PicmanTempBuf        *pixmap_mask,
                                        const PicmanTempBuf        *brush_mask,
                                        gfloat                   *d,
                                        gint                      x,
                                        gint                      y,
                                        gint                      width,
                                        PicmanBrushApplicationMode  mode)
{
  const Babl        *pixmap_format;
  PicmanImageBaseType  pixmap_base_type;
  PicmanPrecision      pixmap_precision;
  gint               pixmap_bytes;
  gint               pixmap_width;
  gint               pixmap_height;
  guchar            *b;

  pixmap_width  = picman_temp_buf_get_width  (pixmap_mask);
  pixmap_height = picman_temp_buf_get_height (pixmap_mask);

  /*  Make sure x, y are positive  */
  while (x < 0) x += pixmap_width;
  while (y < 0) y += pixmap_height;

  pixmap_format    = picman_temp_buf_get_format (pixmap_mask);
  pixmap_base_type = picman_babl_format_get_base_type (pixmap_format);
  pixmap_precision = picman_babl_format_get_precision (pixmap_format);
  pixmap_bytes     = babl_format_get_bytes_per_pixel (pixmap_format);

  /* Point to the approriate scanline */
  b = (picman_temp_buf_get_data (pixmap_mask) +
       (y % pixmap_height) * pixmap_width * pixmap_bytes);

  if (mode == PICMAN_BRUSH_SOFT && brush_mask)
    {
      const Babl   *fish;
      const guchar *mask     = (picman_temp_buf_get_data (brush_mask) +
                                (y % picman_temp_buf_get_height (brush_mask)) *
                                picman_temp_buf_get_width (brush_mask));
      guchar       *line_buf = g_alloca (width * (pixmap_bytes + 1));
      guchar       *l        = line_buf;
      gint          i;

      fish = babl_fish (picman_babl_format (pixmap_base_type, pixmap_precision,
                                          TRUE),
                        babl_format ("RGBA float"));

      /* put the source pixmap's pixels, plus the mask's alpha, into
       * one line, so we can use one single call to babl_process() to
       * convert the entire line
       */
      for (i = 0; i < width; i++)
        {
          gint    x_index = ((i + x) % pixmap_width);
          gint    p_bytes = pixmap_bytes;
          guchar *p       = b + x_index * p_bytes;

          while (p_bytes--)
            *l++ = *p++;

          *l++ = mask[x_index];
        }

      babl_process (fish, line_buf, d, width);
    }
  else
    {
      const Babl *fish;
      guchar     *line_buf = g_alloca (width * (pixmap_bytes));
      guchar     *l        = line_buf;
      gint        i;

      fish = babl_fish (pixmap_format, babl_format ("RGBA float"));

      /* put the source pixmap's pixels into one line, so we can use
       * one single call to babl_process() to convert the entire line
       */
      for (i = 0; i < width; i++)
        {
          gint    x_index = ((i + x) % pixmap_width);
          gint    p_bytes = pixmap_bytes;
          guchar *p       = b + x_index * p_bytes;

          while (p_bytes--)
            *l++ = *p++;
        }

      babl_process (fish, line_buf, d, width);
    }
}
