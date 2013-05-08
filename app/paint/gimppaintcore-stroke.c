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

#include "core/picmanboundary.h"
#include "core/picmandrawable.h"
#include "core/picmanerror.h"
#include "core/picmancoords.h"

#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "picmanpaintcore.h"
#include "picmanpaintcore-stroke.h"
#include "picmanpaintoptions.h"

#include "picman-intl.h"


static void picman_paint_core_stroke_emulate_dynamics (PicmanCoords *coords,
                                                     gint        length);


static const PicmanCoords default_coords = PICMAN_COORDS_DEFAULT_VALUES;


gboolean
picman_paint_core_stroke (PicmanPaintCore     *core,
                        PicmanDrawable      *drawable,
                        PicmanPaintOptions  *paint_options,
                        PicmanCoords        *strokes,
                        gint               n_strokes,
                        gboolean           push_undo,
                        GError           **error)
{
  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), FALSE);
  g_return_val_if_fail (strokes != NULL, FALSE);
  g_return_val_if_fail (n_strokes > 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (picman_paint_core_start (core, drawable, paint_options, &strokes[0],
                             error))
    {
      gint i;

      core->start_coords = strokes[0];
      core->last_coords  = strokes[0];

      picman_paint_core_paint (core, drawable, paint_options,
                             PICMAN_PAINT_STATE_INIT, 0);

      picman_paint_core_paint (core, drawable, paint_options,
                             PICMAN_PAINT_STATE_MOTION, 0);

      for (i = 1; i < n_strokes; i++)
        {
          picman_paint_core_interpolate (core, drawable, paint_options,
                                       &strokes[i], 0);
        }

      picman_paint_core_paint (core, drawable, paint_options,
                             PICMAN_PAINT_STATE_FINISH, 0);

      picman_paint_core_finish (core, drawable, push_undo);

      picman_paint_core_cleanup (core);

      return TRUE;
    }

  return FALSE;
}

gboolean
picman_paint_core_stroke_boundary (PicmanPaintCore      *core,
                                 PicmanDrawable       *drawable,
                                 PicmanPaintOptions   *paint_options,
                                 gboolean            emulate_dynamics,
                                 const PicmanBoundSeg *bound_segs,
                                 gint                n_bound_segs,
                                 gint                offset_x,
                                 gint                offset_y,
                                 gboolean            push_undo,
                                 GError            **error)
{
  PicmanBoundSeg *stroke_segs;
  gint          n_stroke_segs;
  gint          off_x;
  gint          off_y;
  PicmanCoords   *coords;
  gboolean      initialized = FALSE;
  gint          n_coords;
  gint          seg;
  gint          s;

  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), FALSE);
  g_return_val_if_fail (bound_segs != NULL && n_bound_segs > 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  stroke_segs = picman_boundary_sort (bound_segs, n_bound_segs,
                                    &n_stroke_segs);

  if (n_stroke_segs == 0)
    return TRUE;

  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  off_x -= offset_x;
  off_y -= offset_y;

  coords = g_new0 (PicmanCoords, n_bound_segs + 4);

  seg      = 0;
  n_coords = 0;

  /* we offset all coordinates by 0.5 to align the brush with the path */

  coords[n_coords]   = default_coords;
  coords[n_coords].x = (gdouble) (stroke_segs[0].x1 - off_x + 0.5);
  coords[n_coords].y = (gdouble) (stroke_segs[0].y1 - off_y + 0.5);

  n_coords++;

  for (s = 0; s < n_stroke_segs; s++)
    {
      while (stroke_segs[seg].x1 != -1 ||
             stroke_segs[seg].x2 != -1 ||
             stroke_segs[seg].y1 != -1 ||
             stroke_segs[seg].y2 != -1)
        {
          coords[n_coords]   = default_coords;
          coords[n_coords].x = (gdouble) (stroke_segs[seg].x1 - off_x + 0.5);
          coords[n_coords].y = (gdouble) (stroke_segs[seg].y1 - off_y + 0.5);

          n_coords++;
          seg++;
        }

      /* Close the stroke points up */
      coords[n_coords] = coords[0];

      n_coords++;

      if (emulate_dynamics)
        picman_paint_core_stroke_emulate_dynamics (coords, n_coords);

      if (initialized ||
          picman_paint_core_start (core, drawable, paint_options, &coords[0],
                                 error))
        {
          gint i;

          initialized = TRUE;

          core->cur_coords   = coords[0];
          core->start_coords = coords[0];
          core->last_coords  = coords[0];

          picman_paint_core_paint (core, drawable, paint_options,
                                 PICMAN_PAINT_STATE_INIT, 0);

          picman_paint_core_paint (core, drawable, paint_options,
                                 PICMAN_PAINT_STATE_MOTION, 0);

          for (i = 1; i < n_coords; i++)
            {
              picman_paint_core_interpolate (core, drawable, paint_options,
                                           &coords[i], 0);
            }

          picman_paint_core_paint (core, drawable, paint_options,
                                 PICMAN_PAINT_STATE_FINISH, 0);
        }
      else
        {
          break;
        }

      n_coords = 0;
      seg++;

      coords[n_coords]   = default_coords;
      coords[n_coords].x = (gdouble) (stroke_segs[seg].x1 - off_x + 0.5);
      coords[n_coords].y = (gdouble) (stroke_segs[seg].y1 - off_y + 0.5);

      n_coords++;
    }

  if (initialized)
    {
      picman_paint_core_finish (core, drawable, push_undo);

      picman_paint_core_cleanup (core);
    }

  g_free (coords);
  g_free (stroke_segs);

  return initialized;
}

gboolean
picman_paint_core_stroke_vectors (PicmanPaintCore     *core,
                                PicmanDrawable      *drawable,
                                PicmanPaintOptions  *paint_options,
                                gboolean           emulate_dynamics,
                                PicmanVectors       *vectors,
                                gboolean           push_undo,
                                GError           **error)
{
  GList    *stroke;
  gboolean  initialized = FALSE;
  gboolean  due_to_lack_of_points = FALSE;
  gint      off_x, off_y;
  gint      vectors_off_x, vectors_off_y;

  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), FALSE);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  picman_item_get_offset (PICMAN_ITEM (vectors),  &vectors_off_x, &vectors_off_y);
  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  off_x -= vectors_off_x;
  off_y -= vectors_off_y;

  for (stroke = vectors->strokes; stroke; stroke = stroke->next)
    {
      GArray   *coords;
      gboolean  closed;

      coords = picman_stroke_interpolate (PICMAN_STROKE (stroke->data),
                                        1.0, &closed);

      if (coords && coords->len)
        {
          gint i;

          for (i = 0; i < coords->len; i++)
            {
              g_array_index (coords, PicmanCoords, i).x -= off_x;
              g_array_index (coords, PicmanCoords, i).y -= off_y;
            }

          if (emulate_dynamics)
            picman_paint_core_stroke_emulate_dynamics ((PicmanCoords *) coords->data,
                                                     coords->len);

          if (initialized ||
              picman_paint_core_start (core, drawable, paint_options,
                                     &g_array_index (coords, PicmanCoords, 0),
                                     error))
            {
              initialized = TRUE;

              core->cur_coords   = g_array_index (coords, PicmanCoords, 0);
              core->start_coords = g_array_index (coords, PicmanCoords, 0);
              core->last_coords  = g_array_index (coords, PicmanCoords, 0);

              picman_paint_core_paint (core, drawable, paint_options,
                                     PICMAN_PAINT_STATE_INIT, 0);

              picman_paint_core_paint (core, drawable, paint_options,
                                     PICMAN_PAINT_STATE_MOTION, 0);

              for (i = 1; i < coords->len; i++)
                {
                  picman_paint_core_interpolate (core, drawable, paint_options,
                                               &g_array_index (coords, PicmanCoords, i),
                                               0);
                }

              picman_paint_core_paint (core, drawable, paint_options,
                                     PICMAN_PAINT_STATE_FINISH, 0);
            }
          else
            {
              if (coords)
                g_array_free (coords, TRUE);

              break;
            }
        }
      else
        {
          due_to_lack_of_points = TRUE;
        }

      if (coords)
        g_array_free (coords, TRUE);
    }

  if (initialized)
    {
      picman_paint_core_finish (core, drawable, push_undo);

      picman_paint_core_cleanup (core);
    }

  if (! initialized && due_to_lack_of_points && *error == NULL)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("Not enough points to stroke"));
    }

  return initialized;
}

static void
picman_paint_core_stroke_emulate_dynamics (PicmanCoords *coords,
                                         gint        length)
{
  const gint ramp_length = length / 3;

  /* Calculate and create pressure ramp parameters */
  if (ramp_length > 0)
    {
      gdouble slope = 1.0 / (gdouble) (ramp_length);
      gint    i;

      /* Calculate pressure start ramp */
      for (i = 0; i < ramp_length; i++)
        {
          coords[i].pressure =  i * slope;
        }

      /* Calculate pressure end ramp */
      for (i = length - ramp_length; i < length; i++)
        {
          coords[i].pressure = 1.0 - (i - (length - ramp_length)) * slope;
        }
    }

  /* Calculate and create velocity ramp parameters */
  if (length > 0)
    {
      gdouble slope = 1.0 / length;
      gint    i;

      /* Calculate velocity end ramp */
      for (i = 0; i < length; i++)
        {
          coords[i].velocity = i * slope;
        }
    }

  if (length > 1)
    {
      gint i;
      /* Fill in direction */
      for (i = 1; i < length; i++)
        {
           coords[i].direction = picman_coords_direction (&coords[i-1], &coords[i]);

        }

      coords[0].direction = coords[1].direction;
    }
}
