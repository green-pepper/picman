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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "core/picmanbrush.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"
#include "core/picmantempbuf.h"

#include "picmanheal.h"
#include "picmansourceoptions.h"

#include "picman-intl.h"



/* NOTES
 *
 * The method used here is similar to the lighting invariant correctin
 * method but slightly different: we do not divide the RGB components,
 * but subtract them I2 = I0 - I1, where I0 is the sample image to be
 * corrected, I1 is the reference pattern. Then we solve DeltaI=0
 * (Laplace) with I2 Dirichlet conditions at the borders of the
 * mask. The solver is a unoptimized red/black checker Gauss-Siedel
 * with an over-relaxation factor of 1.8. It can benefit from a
 * multi-grid evaluation of an initial solution before the main
 * iteration loop.
 *
 * I reduced the convergence criteria to 0.1% (0.001) as we are
 * dealing here with RGB integer components, more is overkill.
 *
 * Jean-Yves Couleaud cjyves@free.fr
 */

static gboolean     picman_heal_start              (PicmanPaintCore    *paint_core,
                                                  PicmanDrawable     *drawable,
                                                  PicmanPaintOptions *paint_options,
                                                  const PicmanCoords *coords,
                                                  GError          **error);

static void         picman_heal_motion             (PicmanSourceCore   *source_core,
                                                  PicmanDrawable     *drawable,
                                                  PicmanPaintOptions *paint_options,
                                                  const PicmanCoords *coords,
                                                  gdouble           opacity,
                                                  PicmanPickable     *src_pickable,
                                                  GeglBuffer       *src_buffer,
                                                  GeglRectangle    *src_rect,
                                                  gint              src_offset_x,
                                                  gint              src_offset_y,
                                                  GeglBuffer       *paint_buffer,
                                                  gint              paint_buffer_x,
                                                  gint              paint_buffer_y,
                                                  gint              paint_area_offset_x,
                                                  gint              paint_area_offset_y,
                                                  gint              paint_area_width,
                                                  gint              paint_area_height);


G_DEFINE_TYPE (PicmanHeal, picman_heal, PICMAN_TYPE_SOURCE_CORE)

#define parent_class picman_heal_parent_class


void
picman_heal_register (Picman                      *picman,
                    PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_HEAL,
                PICMAN_TYPE_SOURCE_OPTIONS,
                "picman-heal",
                _("Heal"),
                "picman-tool-heal");
}

static void
picman_heal_class_init (PicmanHealClass *klass)
{
  PicmanPaintCoreClass  *paint_core_class  = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanSourceCoreClass *source_core_class = PICMAN_SOURCE_CORE_CLASS (klass);

  paint_core_class->start   = picman_heal_start;

  source_core_class->motion = picman_heal_motion;
}

static void
picman_heal_init (PicmanHeal *heal)
{
}

static gboolean
picman_heal_start (PicmanPaintCore     *paint_core,
                 PicmanDrawable      *drawable,
                 PicmanPaintOptions  *paint_options,
                 const PicmanCoords  *coords,
                 GError           **error)
{
  PicmanSourceCore *source_core = PICMAN_SOURCE_CORE (paint_core);

  if (! PICMAN_PAINT_CORE_CLASS (parent_class)->start (paint_core, drawable,
                                                     paint_options, coords,
                                                     error))
    {
      return FALSE;
    }

  if (! source_core->set_source && picman_drawable_is_indexed (drawable))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("Healing does not operate on indexed layers."));
      return FALSE;
    }

  return TRUE;
}

/* Subtract bottom from top and store in result as a double
 */
static void
picman_heal_sub (GeglBuffer          *top_buffer,
               const GeglRectangle *top_rect,
               GeglBuffer          *bottom_buffer,
               const GeglRectangle *bottom_rect,
               GeglBuffer          *result_buffer,
               const GeglRectangle *result_rect)
{
  GeglBufferIterator *iter;
  const Babl         *format       = gegl_buffer_get_format (top_buffer);
  gint                n_components = babl_format_get_n_components (format);

  if (n_components == 2)
    format = babl_format ("Y'A float");
  else if (n_components == 4)
    format = babl_format ("R'G'B'A float");
  else
    g_return_if_reached ();

  iter = gegl_buffer_iterator_new (top_buffer, top_rect, 0, format,
                                   GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

  gegl_buffer_iterator_add (iter, bottom_buffer, bottom_rect, 0, format,
                            GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

  gegl_buffer_iterator_add (iter, result_buffer, result_rect, 0,
                            babl_format_n (babl_type ("double"), n_components),
                            GEGL_BUFFER_WRITE, GEGL_ABYSS_NONE);

  while (gegl_buffer_iterator_next (iter))
    {
      gfloat  *t      = iter->data[0];
      gfloat  *b      = iter->data[1];
      gdouble *r      = iter->data[2];
      gint     length = iter->length * n_components;

      while (length--)
        *r++ = *t++ - *b++;
    }
}

/* Add first to second and store in result
 */
static void
picman_heal_add (GeglBuffer          *first_buffer,
               const GeglRectangle *first_rect,
               GeglBuffer          *second_buffer,
               const GeglRectangle *second_rect,
               GeglBuffer          *result_buffer,
               const GeglRectangle *result_rect)
{
  GeglBufferIterator *iter;
  const Babl         *format       = gegl_buffer_get_format (result_buffer);
  gint                n_components = babl_format_get_n_components (format);

  if (n_components == 2)
    format = babl_format ("Y'A float");
  else if (n_components == 4)
    format = babl_format ("R'G'B'A float");
  else
    g_return_if_reached ();

  iter = gegl_buffer_iterator_new (first_buffer, first_rect, 0,
                                   babl_format_n (babl_type ("double"),
                                                  n_components),
                                   GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

  gegl_buffer_iterator_add (iter, second_buffer, second_rect, 0, format,
                            GEGL_BUFFER_READ, GEGL_ABYSS_NONE);

  gegl_buffer_iterator_add (iter, result_buffer, result_rect, 0, format,
                            GEGL_BUFFER_WRITE, GEGL_ABYSS_NONE);

  while (gegl_buffer_iterator_next (iter))
    {
      gdouble *f      = iter->data[0];
      gfloat  *s      = iter->data[1];
      gfloat  *r      = iter->data[2];
      gint     length = iter->length * n_components;

      while (length--)
        *r++ = *f++ + *s++;
    }
}

/* Perform one iteration of the laplace solver for matrix.  Store the
 * result in solution and return the square of the cummulative error
 * of the solution.
 */
static gdouble
picman_heal_laplace_iteration (gdouble *matrix,
                             gint     height,
                             gint     depth,
                             gint     width,
                             gdouble *solution,
                             guchar  *mask)
{
  const gint    rowstride = width * depth;
  gint          i, j, k, off, offm, offm0, off0;
  gdouble       tmp, diff;
  gdouble       err       = 0.0;
  const gdouble w         = 1.80 * 0.25; /* Over-relaxation = 1.8 */

  /* we use a red/black checker model of the discretization grid */

  /* do reds */
  for (i = 0; i < height; i++)
    {
      off0  = i * rowstride;
      offm0 = i * width;

      for (j = i % 2; j < width; j += 2)
        {
          off  = off0 + j * depth;
          offm = offm0 + j;

          if ((0 == mask[offm]) ||
              (i == 0) || (i == (height - 1)) ||
              (j == 0) || (j == (width - 1)))
            {
              /* do nothing at the boundary or outside mask */
              for (k = 0; k < depth; k++)
                solution[off + k] = matrix[off + k];
            }
          else
            {
              /* Use Gauss Siedel to get the correction factor then
               * over-relax it
               */
              for (k = 0; k < depth; k++)
                {
                  tmp = solution[off + k];
                  solution[off + k] = (matrix[off + k] +
                                       w *
                                       (matrix[off - depth + k] +     /* west */
                                        matrix[off + depth + k] +     /* east */
                                        matrix[off - rowstride + k] + /* north */
                                        matrix[off + rowstride + k] - 4.0 *
                                        matrix[off+k]));              /* south */

                  diff = solution[off + k] - tmp;
                  err += diff * diff;
                }
            }
        }
    }


  /* Do blacks
   *
   * As we've done the reds earlier, we can use them right now to
   * accelerate the convergence. So we have "solution" in the solver
   * instead of "matrix" above
   */
  for (i = 0; i < height; i++)
    {
      off0 =  i * rowstride;
      offm0 = i * width;

      for (j = (i % 2) ? 0 : 1; j < width; j += 2)
        {
          off = off0 + j * depth;
          offm = offm0 + j;

          if ((0 == mask[offm]) ||
              (i == 0) || (i == (height - 1)) ||
              (j == 0) || (j == (width - 1)))
            {
              /* do nothing at the boundary or outside mask */
              for (k = 0; k < depth; k++)
                solution[off + k] = matrix[off + k];
            }
          else
            {
              /* Use Gauss Siedel to get the correction factor then
               * over-relax it
               */
              for (k = 0; k < depth; k++)
                {
                  tmp = solution[off + k];
                  solution[off + k] = (matrix[off + k] +
                                       w *
                                       (solution[off - depth + k] +     /* west */
                                        solution[off + depth + k] +     /* east */
                                        solution[off - rowstride + k] + /* north */
                                        solution[off + rowstride + k] - 4.0 *
                                        matrix[off+k]));                /* south */

                  diff = solution[off + k] - tmp;
                  err += diff*diff;
                }
            }
        }
    }

  return err;
}

/* Solve the laplace equation for matrix and store the result in solution.
 */
static void
picman_heal_laplace_loop (gdouble *matrix,
                        gint     height,
                        gint     depth,
                        gint     width,
                        gdouble *solution,
                        guchar  *mask)
{
#define EPSILON   1e-8
#define MAX_ITER  500
  gint i;

  /* repeat until convergence or max iterations */
  for (i = 0; i < MAX_ITER; i++)
    {
      gdouble sqr_err;

      /* do one iteration and store the amount of error */
      sqr_err = picman_heal_laplace_iteration (matrix, height, depth, width,
                                             solution, mask);

      /* copy solution to matrix */
      memcpy (matrix, solution, width * height * depth * sizeof (double));

      if (sqr_err < EPSILON)
        break;
    }
}

/* Original Algorithm Design:
 *
 * T. Georgiev, "Photoshop Healing Brush: a Tool for Seamless Cloning
 * http://www.tgeorgiev.net/Photoshop_Healing.pdf
 */
static void
picman_heal (GeglBuffer          *src_buffer,
           const GeglRectangle *src_rect,
           GeglBuffer          *dest_buffer,
           const GeglRectangle *dest_rect,
           GeglBuffer          *mask_buffer,
           const GeglRectangle *mask_rect)
{
  const Babl *src_format;
  const Babl *dest_format;
  gint        src_components;
  gint        dest_components;
  gint        width;
  gint        height;
  gdouble    *i_1;
  gdouble    *i_2;
  GeglBuffer *i_1_buffer;
  GeglBuffer *i_2_buffer;
  guchar     *mask;

  src_format  = gegl_buffer_get_format (src_buffer);
  dest_format = gegl_buffer_get_format (dest_buffer);

  src_components  = babl_format_get_n_components (src_format);
  dest_components = babl_format_get_n_components (dest_format);

  width  = gegl_buffer_get_width  (src_buffer);
  height = gegl_buffer_get_height (src_buffer);

  g_return_if_fail (src_components == dest_components);

  i_1  = g_new (gdouble, width * height * src_components);
  i_2  = g_new (gdouble, width * height * src_components);

  i_1_buffer =
    gegl_buffer_linear_new_from_data (i_1,
                                      babl_format_n (babl_type ("double"),
                                                     src_components),
                                      GEGL_RECTANGLE (0, 0, width, height),
                                      GEGL_AUTO_ROWSTRIDE,
                                      (GDestroyNotify) g_free, i_1);
  i_2_buffer =
    gegl_buffer_linear_new_from_data (i_2,
                                      babl_format_n (babl_type ("double"),
                                                     src_components),
                                      GEGL_RECTANGLE (0, 0, width, height),
                                      GEGL_AUTO_ROWSTRIDE,
                                      (GDestroyNotify) g_free, i_2);

  /* subtract pattern from image and store the result as a double in i_1 */
  picman_heal_sub (dest_buffer, dest_rect,
                 src_buffer, src_rect,
                 i_1_buffer, GEGL_RECTANGLE (0, 0, width, height));

  mask = g_new (guchar, mask_rect->width * mask_rect->height);

  gegl_buffer_get (mask_buffer, mask_rect, 1.0, babl_format ("Y u8"),
                   mask, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  /* FIXME: is a faster implementation needed? */
  picman_heal_laplace_loop (i_1, height, src_components, width, i_2, mask);

  g_free (mask);

  /* add solution to original image and store in dest */
  picman_heal_add (i_2_buffer, GEGL_RECTANGLE (0, 0, width, height),
                 src_buffer, src_rect,
                 dest_buffer, dest_rect);

  g_object_unref (i_1_buffer);
  g_object_unref (i_2_buffer);
}

static void
picman_heal_motion (PicmanSourceCore   *source_core,
                  PicmanDrawable     *drawable,
                  PicmanPaintOptions *paint_options,
                  const PicmanCoords *coords,
                  gdouble           opacity,
                  PicmanPickable     *src_pickable,
                  GeglBuffer       *src_buffer,
                  GeglRectangle    *src_rect,
                  gint              src_offset_x,
                  gint              src_offset_y,
                  GeglBuffer       *paint_buffer,
                  gint              paint_buffer_x,
                  gint              paint_buffer_y,
                  gint              paint_area_offset_x,
                  gint              paint_area_offset_y,
                  gint              paint_area_width,
                  gint              paint_area_height)
{
  PicmanPaintCore     *paint_core = PICMAN_PAINT_CORE (source_core);
  PicmanContext       *context    = PICMAN_CONTEXT (paint_options);
  PicmanDynamics      *dynamics   = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage         *image      = picman_item_get_image (PICMAN_ITEM (drawable));
  GeglBuffer        *src_copy;
  GeglBuffer        *mask_buffer;
  const PicmanTempBuf *mask_buf;
  gdouble            fade_point;
  gdouble            hardness;
  gint               mask_off_x;
  gint               mask_off_y;

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  hardness = picman_dynamics_get_linear_value (dynamics,
                                             PICMAN_DYNAMICS_OUTPUT_HARDNESS,
                                             coords,
                                             paint_options,
                                             fade_point);

  mask_buf = picman_brush_core_get_brush_mask (PICMAN_BRUSH_CORE (source_core),
                                             coords,
                                             PICMAN_BRUSH_HARD,
                                             hardness);

  /* check that all buffers are of the same size */
  if (src_rect->width  != gegl_buffer_get_width  (paint_buffer) ||
      src_rect->height != gegl_buffer_get_height (paint_buffer))
    {
      /* this generally means that the source point has hit the edge
       * of the layer, so it is not an error and we should not
       * complain, just don't do anything
       */
      return;
    }

  /*  heal should work in perceptual space, use R'G'B' instead of RGB  */
  src_copy = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                              src_rect->width,
                                              src_rect->height),
                              babl_format ("R'G'B'A float"));

  gegl_buffer_copy (src_buffer,
                    src_rect,
                    src_copy,
                    GEGL_RECTANGLE (0, 0,
                                    src_rect->width,
                                    src_rect->height));

  gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                    GEGL_RECTANGLE (paint_buffer_x, paint_buffer_y,
                                    gegl_buffer_get_width  (paint_buffer),
                                    gegl_buffer_get_height (paint_buffer)),
                    paint_buffer,
                    GEGL_RECTANGLE (paint_area_offset_x,
                                    paint_area_offset_y,
                                    paint_area_width,
                                    paint_area_height));

  mask_buffer = picman_temp_buf_create_buffer ((PicmanTempBuf *) mask_buf);

  /* find the offset of the brush mask's rect */
  {
    gint x = (gint) floor (coords->x) - (gegl_buffer_get_width  (mask_buffer) >> 1);
    gint y = (gint) floor (coords->y) - (gegl_buffer_get_height (mask_buffer) >> 1);

    mask_off_x = (x < 0) ? -x : 0;
    mask_off_y = (y < 0) ? -y : 0;
  }

  picman_heal (src_copy,
             GEGL_RECTANGLE (0, 0,
                             gegl_buffer_get_width  (src_copy),
                             gegl_buffer_get_height (src_copy)),
             paint_buffer,
             GEGL_RECTANGLE (paint_area_offset_x,
                             paint_area_offset_y,
                             paint_area_width,
                             paint_area_height),
             mask_buffer,
             GEGL_RECTANGLE (mask_off_x, mask_off_y,
                             paint_area_width,
                             paint_area_height));

  g_object_unref (src_copy);
  g_object_unref (mask_buffer);

  /* replace the canvas with our healed data */
  picman_brush_core_replace_canvas (PICMAN_BRUSH_CORE (paint_core), drawable,
                                  coords,
                                  MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                  picman_context_get_opacity (context),
                                  picman_paint_options_get_brush_mode (paint_options),
                                  hardness,
                                  PICMAN_PAINT_INCREMENTAL);
}
