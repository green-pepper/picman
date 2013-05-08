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

#include <stdlib.h>
#include <string.h>

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmandrawable-blend.h"
#include "picmangradient.h"
#include "picmanimage.h"
#include "picmanprogress.h"

#include "picman-intl.h"


//#define USE_GRADIENT_CACHE 1


typedef struct
{
  PicmanGradient     *gradient;
  PicmanContext      *context;
  gboolean          reverse;
#ifdef USE_GRADIENT_CACHE
  PicmanRGB          *gradient_cache;
  gint              gradient_cache_size;
#endif
  gdouble           offset;
  gdouble           sx, sy;
  PicmanBlendMode     blend_mode;
  PicmanGradientType  gradient_type;
  PicmanRGB           fg, bg;
  gdouble           dist;
  gdouble           vec[2];
  PicmanRepeatMode    repeat;
  GRand            *seed;
  GeglBuffer       *dist_buffer;
} RenderBlendData;

typedef struct
{
  GeglBuffer    *buffer;
  gfloat        *row_data;
  gint           width;
  GRand         *dither_rand;
} PutPixelData;


/*  local function prototypes  */

static gdouble  gradient_calc_conical_sym_factor  (gdouble   dist,
                                                   gdouble  *axis,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_conical_asym_factor (gdouble   dist,
                                                   gdouble  *axis,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_square_factor       (gdouble   dist,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_radial_factor       (gdouble   dist,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_linear_factor       (gdouble   dist,
                                                   gdouble  *vec,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_bilinear_factor     (gdouble   dist,
                                                   gdouble  *vec,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y);
static gdouble  gradient_calc_spiral_factor       (gdouble   dist,
                                                   gdouble  *axis,
                                                   gdouble   offset,
                                                   gdouble   x,
                                                   gdouble   y,
                                                   gboolean  clockwise);

static gdouble  gradient_calc_shapeburst_angular_factor   (GeglBuffer *dist_buffer,
                                                           gdouble     x,
                                                           gdouble     y);
static gdouble  gradient_calc_shapeburst_spherical_factor (GeglBuffer *dist_buffer,
                                                           gdouble     x,
                                                           gdouble     y);
static gdouble  gradient_calc_shapeburst_dimpled_factor   (GeglBuffer *dist_buffer,
                                                           gdouble     x,
                                                           gdouble     y);

static GeglBuffer * gradient_precalc_shapeburst (PicmanImage           *image,
                                                 PicmanDrawable        *drawable,
                                                 const GeglRectangle *region,
                                                 gdouble              dist,
                                                 PicmanProgress        *progress);

static void     gradient_render_pixel       (gdouble              x,
                                             gdouble              y,
                                             PicmanRGB             *color,
                                             gpointer             render_data);
static void     gradient_put_pixel          (gint                 x,
                                             gint                 y,
                                             PicmanRGB             *color,
                                             gpointer             put_pixel_data);

static void     gradient_fill_region        (PicmanImage           *image,
                                             PicmanDrawable        *drawable,
                                             PicmanContext         *context,
                                             GeglBuffer          *buffer,
                                             const GeglRectangle *buffer_region,
                                             PicmanBlendMode        blend_mode,
                                             PicmanGradientType     gradient_type,
                                             gdouble              offset,
                                             PicmanRepeatMode       repeat,
                                             gboolean             reverse,
                                             gboolean             supersample,
                                             gint                 max_depth,
                                             gdouble              threshold,
                                             gboolean             dither,
                                             gdouble              sx,
                                             gdouble              sy,
                                             gdouble              ex,
                                             gdouble              ey,
                                             PicmanProgress        *progress);


/*  public functions  */

void
picman_drawable_blend (PicmanDrawable         *drawable,
                     PicmanContext          *context,
                     PicmanBlendMode         blend_mode,
                     PicmanLayerModeEffects  paint_mode,
                     PicmanGradientType      gradient_type,
                     gdouble               opacity,
                     gdouble               offset,
                     PicmanRepeatMode        repeat,
                     gboolean              reverse,
                     gboolean              supersample,
                     gint                  max_depth,
                     gdouble               threshold,
                     gboolean              dither,
                     gdouble               startx,
                     gdouble               starty,
                     gdouble               endx,
                     gdouble               endy,
                     PicmanProgress         *progress)
{
  PicmanImage   *image;
  GeglBuffer  *buffer;
  gint         x, y, width, height;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  if (! picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &width, &height))
    return;

  picman_set_busy (image->picman);

  /*  Always create an alpha temp buf (for generality) */
  buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                            picman_drawable_get_format_with_alpha (drawable));

  gradient_fill_region (image, drawable, context,
                        buffer, GEGL_RECTANGLE (0, 0, width, height),
                        blend_mode, gradient_type, offset, repeat, reverse,
                        supersample, max_depth, threshold, dither,
                        (startx - x), (starty - y),
                        (endx - x), (endy - y),
                        progress);

  picman_drawable_apply_buffer (drawable, buffer,
                              GEGL_RECTANGLE (0, 0, width, height),
                              TRUE, C_("undo-type", "Blend"),
                              opacity, paint_mode,
                              NULL, x, y);

  /*  update the image  */
  picman_drawable_update (drawable, x, y, width, height);

  /*  free the temporary buffer  */
  g_object_unref (buffer);

  picman_unset_busy (image->picman);
}

static gdouble
gradient_calc_conical_sym_factor (gdouble  dist,
                                  gdouble *axis,
                                  gdouble  offset,
                                  gdouble  x,
                                  gdouble  y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else if ((x != 0) || (y != 0))
    {
      gdouble vec[2];
      gdouble r;
      gdouble rat;

      /* Calculate offset from the start in pixels */

      r = sqrt (SQR (x) + SQR (y));

      vec[0] = x / r;
      vec[1] = y / r;

      rat = axis[0] * vec[0] + axis[1] * vec[1]; /* Dot product */

      if (rat > 1.0)
        rat = 1.0;
      else if (rat < -1.0)
        rat = -1.0;

      /* This cool idea is courtesy Josh MacDonald,
       * Ali Rahimi --- two more XCF losers.  */

      rat = acos (rat) / G_PI;
      rat = pow (rat, (offset / 10.0) + 1.0);

      return CLAMP (rat, 0.0, 1.0);
    }
  else
    {
      return 0.5;
    }
}

static gdouble
gradient_calc_conical_asym_factor (gdouble  dist,
                                   gdouble *axis,
                                   gdouble  offset,
                                   gdouble  x,
                                   gdouble  y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else if (x != 0 || y != 0)
    {
      gdouble ang0, ang1;
      gdouble ang;
      gdouble rat;

      ang0 = atan2 (axis[0], axis[1]) + G_PI;

      ang1 = atan2 (x, y) + G_PI;

      ang = ang1 - ang0;

      if (ang < 0.0)
        ang += (2.0 * G_PI);

      rat = ang / (2.0 * G_PI);
      rat = pow (rat, (offset / 10.0) + 1.0);

      return CLAMP (rat, 0.0, 1.0);
    }
  else
    {
      return 0.5; /* We are on middle point */
    }
}

static gdouble
gradient_calc_square_factor (gdouble dist,
                             gdouble offset,
                             gdouble x,
                             gdouble y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else
    {
      gdouble r;
      gdouble rat;

      /* Calculate offset from start as a value in [0, 1] */

      offset = offset / 100.0;

      r   = MAX (abs (x), abs (y));
      rat = r / dist;

      if (rat < offset)
        return 0.0;
      else if (offset == 1.0)
        return (rat >= 1.0) ? 1.0 : 0.0;
      else
        return (rat - offset) / (1.0 - offset);
    }
}

static gdouble
gradient_calc_radial_factor (gdouble dist,
                             gdouble offset,
                             gdouble x,
                             gdouble y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else
    {
      gdouble r;
      gdouble rat;

      /* Calculate radial offset from start as a value in [0, 1] */

      offset = offset / 100.0;

      r   = sqrt (SQR (x) + SQR (y));
      rat = r / dist;

      if (rat < offset)
        return 0.0;
      else if (offset == 1.0)
        return (rat >= 1.0) ? 1.0 : 0.0;
      else
        return (rat - offset) / (1.0 - offset);
    }
}

static gdouble
gradient_calc_linear_factor (gdouble  dist,
                             gdouble *vec,
                             gdouble  offset,
                             gdouble  x,
                             gdouble  y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else
    {
      gdouble r;
      gdouble rat;

      offset = offset / 100.0;

      r   = vec[0] * x + vec[1] * y;
      rat = r / dist;

      if (rat >= 0.0 && rat < offset)
        return 0.0;
      else if (offset == 1.0)
        return (rat >= 1.0) ? 1.0 : 0.0;
      else if (rat < 0.0)
        return rat / (1.0 - offset);
      else
        return (rat - offset) / (1.0 - offset);
    }
}

static gdouble
gradient_calc_bilinear_factor (gdouble  dist,
                               gdouble *vec,
                               gdouble  offset,
                               gdouble  x,
                               gdouble  y)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else
    {
      gdouble r;
      gdouble rat;

      /* Calculate linear offset from the start line outward */

      offset = offset / 100.0;

      r   = vec[0] * x + vec[1] * y;
      rat = r / dist;

      if (fabs (rat) < offset)
        return 0.0;
      else if (offset == 1.0)
        return (rat == 1.0) ? 1.0 : 0.0;
      else
        return (fabs (rat) - offset) / (1.0 - offset);
    }
}

static gdouble
gradient_calc_spiral_factor (gdouble   dist,
                             gdouble  *axis,
                             gdouble   offset,
                             gdouble   x,
                             gdouble   y,
                             gboolean  clockwise)
{
  if (dist == 0.0)
    {
      return 0.0;
    }
  else if (x != 0.0 || y != 0.0)
    {
      gdouble ang0, ang1;
      gdouble ang;
      double  r;

      ang0 = atan2 (axis[0], axis[1]) + G_PI;
      ang1 = atan2 (x, y) + G_PI;

      if (clockwise)
        ang = ang1 - ang0;
      else
        ang = ang0 - ang1;

      if (ang < 0.0)
        ang += (2.0 * G_PI);

      r = sqrt (SQR (x) + SQR (y)) / dist;

      return fmod (ang / (2.0 * G_PI) + r + offset, 1.0);
    }
  else
    {
      return 0.5 ; /* We are on the middle point */
    }
}

static gdouble
gradient_calc_shapeburst_angular_factor (GeglBuffer *dist_buffer,
                                         gdouble     x,
                                         gdouble     y)
{
  gint   ix = CLAMP (x, 0.0, gegl_buffer_get_width  (dist_buffer) - 0.7);
  gint   iy = CLAMP (y, 0.0, gegl_buffer_get_height (dist_buffer) - 0.7);
  gfloat value;

  gegl_buffer_get (dist_buffer, GEGL_RECTANGLE (ix, iy, 1, 1), 1.0,
                   NULL, &value,
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  value = 1.0 - value;

  return value;
}


static gdouble
gradient_calc_shapeburst_spherical_factor (GeglBuffer *dist_buffer,
                                           gdouble     x,
                                           gdouble     y)
{
  gint   ix = CLAMP (x, 0.0, gegl_buffer_get_width  (dist_buffer) - 0.7);
  gint   iy = CLAMP (y, 0.0, gegl_buffer_get_height (dist_buffer) - 0.7);
  gfloat value;

  gegl_buffer_get (dist_buffer, GEGL_RECTANGLE (ix, iy, 1, 1), 1.0,
                   NULL, &value,
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  value = 1.0 - sin (0.5 * G_PI * value);

  return value;
}


static gdouble
gradient_calc_shapeburst_dimpled_factor (GeglBuffer *dist_buffer,
                                         gdouble     x,
                                         gdouble     y)
{
  gint   ix = CLAMP (x, 0.0, gegl_buffer_get_width  (dist_buffer) - 0.7);
  gint   iy = CLAMP (y, 0.0, gegl_buffer_get_height (dist_buffer) - 0.7);
  gfloat value;

  gegl_buffer_get (dist_buffer, GEGL_RECTANGLE (ix, iy, 1, 1), 1.0,
                   NULL, &value,
                   GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

  value = cos (0.5 * G_PI * value);

  return value;
}

static GeglBuffer *
gradient_precalc_shapeburst (PicmanImage           *image,
                             PicmanDrawable        *drawable,
                             const GeglRectangle *region,
                             gdouble              dist,
                             PicmanProgress        *progress)
{
  PicmanChannel *mask;
  GeglBuffer  *dist_buffer;
  GeglBuffer  *temp_buffer;
  GeglNode    *shapeburst;
  gdouble      max;
  gfloat       max_iteration;

  picman_progress_set_text (progress, _("Calculating distance map"));

  /*  allocate the distance map  */
  dist_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                 region->width, region->height),
                                 babl_format ("Y float"));

  /*  allocate the selection mask copy
   *  XXX: its format should be the same of picman:shapeburst input buffer
   *       porting the op to 'float' should be reflected here as well
   */
  temp_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                 region->width, region->height),
                                 babl_format ("Y u8"));

  mask = picman_image_get_mask (image);

  /*  If the image mask is not empty, use it as the shape burst source  */
  if (! picman_channel_is_empty (mask))
    {
      gint x, y, width, height;
      gint off_x, off_y;

      picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &width, &height);
      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

      /*  copy the mask to the temp mask  */
      gegl_buffer_copy (picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)),
                        GEGL_RECTANGLE (x + off_x, y + off_y, width, height),
                        temp_buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));
    }
  else
    {
      /*  If the intended drawable has an alpha channel, use that  */
      if (picman_drawable_has_alpha (drawable))
        {
          const Babl *component_format;

          component_format = babl_format ("A u8");

          /*  extract the aplha into the temp mask  */
          gegl_buffer_set_format (temp_buffer, component_format);
          gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                            GEGL_RECTANGLE (region->x, region->y,
                                            region->width, region->height),
                            temp_buffer,
                            GEGL_RECTANGLE (0, 0, 0, 0));
          gegl_buffer_set_format (temp_buffer, NULL);
        }
      else
        {
          GeglColor *white = gegl_color_new ("white");

          /*  Otherwise, just fill the shapeburst to white  */
          gegl_buffer_set_color (temp_buffer, NULL, white);
          g_object_unref (white);
        }
    }

  shapeburst = gegl_node_new_child (NULL,
                                    "operation", "picman:shapeburst",
                                    NULL);

  picman_gegl_progress_connect (shapeburst, progress, NULL);

  picman_gegl_apply_operation (temp_buffer, NULL, NULL,
                             shapeburst,
                             dist_buffer, NULL);

  gegl_node_get (shapeburst, "max-iterations", &max, NULL);

  g_object_unref (shapeburst);

  max_iteration = max;

  g_object_unref (temp_buffer);

  /*  normalize the shapeburst with the max iteration  */
  if (max_iteration > 0)
    {
      GeglBufferIterator *iter;

      iter = gegl_buffer_iterator_new (dist_buffer, NULL, 0, NULL,
                                       GEGL_BUFFER_READWRITE, GEGL_ABYSS_NONE);

      while (gegl_buffer_iterator_next (iter))
        {
          gfloat *data = iter->data[0];

          while (iter->length--)
            *data++ /= max_iteration;
        }
    }

  return dist_buffer;
}


static void
gradient_render_pixel (gdouble   x,
                       gdouble   y,
                       PicmanRGB  *color,
                       gpointer  render_data)
{
  RenderBlendData *rbd = render_data;
  gdouble          factor;

  /* Calculate blending factor */

  switch (rbd->gradient_type)
    {
    case PICMAN_GRADIENT_LINEAR:
      factor = gradient_calc_linear_factor (rbd->dist,
                                            rbd->vec, rbd->offset,
                                            x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_BILINEAR:
      factor = gradient_calc_bilinear_factor (rbd->dist,
                                              rbd->vec, rbd->offset,
                                              x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_RADIAL:
      factor = gradient_calc_radial_factor (rbd->dist,
                                            rbd->offset,
                                            x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_SQUARE:
      factor = gradient_calc_square_factor (rbd->dist, rbd->offset,
                                            x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_CONICAL_SYMMETRIC:
      factor = gradient_calc_conical_sym_factor (rbd->dist,
                                                 rbd->vec, rbd->offset,
                                                 x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_CONICAL_ASYMMETRIC:
      factor = gradient_calc_conical_asym_factor (rbd->dist,
                                                  rbd->vec, rbd->offset,
                                                  x - rbd->sx, y - rbd->sy);
      break;

    case PICMAN_GRADIENT_SHAPEBURST_ANGULAR:
      factor = gradient_calc_shapeburst_angular_factor (rbd->dist_buffer, x, y);
      break;

    case PICMAN_GRADIENT_SHAPEBURST_SPHERICAL:
      factor = gradient_calc_shapeburst_spherical_factor (rbd->dist_buffer, x, y);
      break;

    case PICMAN_GRADIENT_SHAPEBURST_DIMPLED:
      factor = gradient_calc_shapeburst_dimpled_factor (rbd->dist_buffer, x, y);
      break;

    case PICMAN_GRADIENT_SPIRAL_CLOCKWISE:
      factor = gradient_calc_spiral_factor (rbd->dist,
                                            rbd->vec, rbd->offset,
                                            x - rbd->sx, y - rbd->sy, TRUE);
      break;

    case PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE:
      factor = gradient_calc_spiral_factor (rbd->dist,
                                            rbd->vec, rbd->offset,
                                            x - rbd->sx, y - rbd->sy, FALSE);
      break;

    default:
      g_assert_not_reached ();
      return;
    }

  /* Adjust for repeat */

  switch (rbd->repeat)
    {
    case PICMAN_REPEAT_NONE:
      factor = CLAMP (factor, 0.0, 1.0);
      break;

    case PICMAN_REPEAT_SAWTOOTH:
      factor = factor - floor (factor);
      break;

    case PICMAN_REPEAT_TRIANGULAR:
      {
        guint ifactor;

        if (factor < 0.0)
          factor = -factor;

        ifactor = (guint) factor;
        factor = factor - floor (factor);

        if (ifactor & 1)
          factor = 1.0 - factor;
      }
      break;
    }

  /* Blend the colors */

  if (rbd->blend_mode == PICMAN_CUSTOM_MODE)
    {
#ifdef USE_GRADIENT_CACHE
      *color = rbd->gradient_cache[(gint) (factor * (rbd->gradient_cache_size - 1))];
#else
      picman_gradient_get_color_at (rbd->gradient, rbd->context, NULL,
                                  factor, rbd->reverse, color);
#endif
    }
  else
    {
      /* Blend values */

      if (rbd->reverse)
        factor = 1.0 - factor;

      color->r = rbd->fg.r + (rbd->bg.r - rbd->fg.r) * factor;
      color->g = rbd->fg.g + (rbd->bg.g - rbd->fg.g) * factor;
      color->b = rbd->fg.b + (rbd->bg.b - rbd->fg.b) * factor;
      color->a = rbd->fg.a + (rbd->bg.a - rbd->fg.a) * factor;

      if (rbd->blend_mode == PICMAN_FG_BG_HSV_MODE)
        {
          PicmanHSV hsv = *((PicmanHSV *) color);

          picman_hsv_to_rgb (&hsv, color);
        }
    }
}

static void
gradient_put_pixel (gint      x,
                    gint      y,
                    PicmanRGB  *color,
                    gpointer  put_pixel_data)
{
  PutPixelData *ppd  = put_pixel_data;
  gfloat       *dest = ppd->row_data + 4 * x;

  if (ppd->dither_rand)
    {
      gint i = g_rand_int (ppd->dither_rand);

      *dest++ = color->r + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
      *dest++ = color->g + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
      *dest++ = color->b + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
      *dest++ = color->a + (gdouble) (i & 0xff) / 256.0 / 256.0;
    }
  else
    {
      *dest++ = color->r;
      *dest++ = color->g;
      *dest++ = color->b;
      *dest++ = color->a;
    }

  /* Paint whole row if we are on the rightmost pixel */

  if (x == (ppd->width - 1))
    gegl_buffer_set (ppd->buffer, GEGL_RECTANGLE (0, y, ppd->width, 1),
                     0, babl_format ("R'G'B'A float"), ppd->row_data,
                     GEGL_AUTO_ROWSTRIDE);
}

static void
gradient_fill_region (PicmanImage           *image,
                      PicmanDrawable        *drawable,
                      PicmanContext         *context,
                      GeglBuffer          *buffer,
                      const GeglRectangle *buffer_region,
                      PicmanBlendMode        blend_mode,
                      PicmanGradientType     gradient_type,
                      gdouble              offset,
                      PicmanRepeatMode       repeat,
                      gboolean             reverse,
                      gboolean             supersample,
                      gint                 max_depth,
                      gdouble              threshold,
                      gboolean             dither,
                      gdouble              sx,
                      gdouble              sy,
                      gdouble              ex,
                      gdouble              ey,
                      PicmanProgress        *progress)
{
  RenderBlendData rbd = { 0, };

  PICMAN_TIMER_START();

  rbd.gradient = picman_context_get_gradient (context);
  rbd.context  = context;
  rbd.reverse  = reverse;

#ifdef USE_GRADIENT_CACHE
  {
    gint i;

    rbd.gradient_cache_size = ceil (sqrt (SQR (sx - ex) + SQR (sy - ey)));
    rbd.gradient_cache      = g_new0 (PicmanRGB, rbd.gradient_cache_size);

    for (i = 0; i < rbd.gradient_cache_size; i++)
      {
        gdouble factor = (gdouble) i / (gdouble) (rbd.gradient_cache_size - 1);

        picman_gradient_get_color_at (rbd.gradient, rbd.context, NULL,
                                    factor, rbd.reverse,
                                    rbd.gradient_cache + i);
      }
  }
#endif

  if (picman_gradient_has_fg_bg_segments (rbd.gradient))
    rbd.gradient = picman_gradient_flatten (rbd.gradient, context);
  else
    rbd.gradient = g_object_ref (rbd.gradient);

  picman_context_get_foreground (context, &rbd.fg);
  picman_context_get_background (context, &rbd.bg);

  switch (blend_mode)
    {
    case PICMAN_FG_BG_RGB_MODE:
      break;

    case PICMAN_FG_BG_HSV_MODE:
      /* Convert to HSV */
      {
        PicmanHSV fg_hsv;
        PicmanHSV bg_hsv;

        picman_rgb_to_hsv (&rbd.fg, &fg_hsv);
        picman_rgb_to_hsv (&rbd.bg, &bg_hsv);

        memcpy (&rbd.fg, &fg_hsv, sizeof (PicmanRGB));
        memcpy (&rbd.bg, &bg_hsv, sizeof (PicmanRGB));
      }
      break;

    case PICMAN_FG_TRANSPARENT_MODE:
      /* Color does not change, just the opacity */

      rbd.bg   = rbd.fg;
      rbd.bg.a = PICMAN_OPACITY_TRANSPARENT;
      break;

    case PICMAN_CUSTOM_MODE:
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  /* Calculate type-specific parameters */

  switch (gradient_type)
    {
    case PICMAN_GRADIENT_RADIAL:
      rbd.dist = sqrt (SQR (ex - sx) + SQR (ey - sy));
      break;

    case PICMAN_GRADIENT_SQUARE:
      rbd.dist = MAX (fabs (ex - sx), fabs (ey - sy));
      break;

    case PICMAN_GRADIENT_CONICAL_SYMMETRIC:
    case PICMAN_GRADIENT_CONICAL_ASYMMETRIC:
    case PICMAN_GRADIENT_SPIRAL_CLOCKWISE:
    case PICMAN_GRADIENT_SPIRAL_ANTICLOCKWISE:
    case PICMAN_GRADIENT_LINEAR:
    case PICMAN_GRADIENT_BILINEAR:
      rbd.dist = sqrt (SQR (ex - sx) + SQR (ey - sy));

      if (rbd.dist > 0.0)
        {
          rbd.vec[0] = (ex - sx) / rbd.dist;
          rbd.vec[1] = (ey - sy) / rbd.dist;
        }

      break;

    case PICMAN_GRADIENT_SHAPEBURST_ANGULAR:
    case PICMAN_GRADIENT_SHAPEBURST_SPHERICAL:
    case PICMAN_GRADIENT_SHAPEBURST_DIMPLED:
      rbd.dist = sqrt (SQR (ex - sx) + SQR (ey - sy));
      rbd.dist_buffer = gradient_precalc_shapeburst (image, drawable,
                                                     buffer_region,
                                                     rbd.dist, progress);
      picman_progress_set_text (progress, _("Blending"));
      break;

    default:
      g_assert_not_reached ();
      break;
    }

  /* Initialize render data */

  rbd.offset        = offset;
  rbd.sx            = sx;
  rbd.sy            = sy;
  rbd.blend_mode    = blend_mode;
  rbd.gradient_type = gradient_type;
  rbd.repeat        = repeat;

  /* Render the gradient! */

  if (supersample)
    {
      PutPixelData  ppd;

      ppd.buffer      = buffer;
      ppd.row_data    = g_malloc (sizeof (float) * 4 * buffer_region->width);
      ppd.width       = buffer_region->width;
      ppd.dither_rand = g_rand_new ();

      picman_adaptive_supersample_area (0, 0,
                                      (buffer_region->width  - 1),
                                      (buffer_region->height - 1),
                                      max_depth, threshold,
                                      gradient_render_pixel, &rbd,
                                      gradient_put_pixel, &ppd,
                                      progress ?
                                      picman_progress_update_and_flush : NULL,
                                      progress);

      g_rand_free (ppd.dither_rand);
      g_free (ppd.row_data);
    }
  else
    {
      GeglBufferIterator *iter;
      GeglRectangle      *roi;
      gint                total = buffer_region->width * buffer_region->height;
      gint                done  = 0;

      iter = gegl_buffer_iterator_new (buffer, buffer_region, 0,
                                       babl_format ("R'G'B'A float"),
                                       GEGL_BUFFER_WRITE, GEGL_ABYSS_NONE);
      roi = &iter->roi[0];

      if (dither)
        rbd.seed = g_rand_new ();

      while (gegl_buffer_iterator_next (iter))
        {
          gfloat *dest = iter->data[0];
          gint    endx  = roi->x + roi->width;
          gint    endy  = roi->y + roi->height;
          gint    x, y;

          if (rbd.seed)
            {
              GRand *dither_rand = g_rand_new_with_seed (g_rand_int (rbd.seed));

              for (y = roi->y; y < endy; y++)
                for (x = roi->x; x < endx; x++)
                  {
                    PicmanRGB  color;
                    gint     i = g_rand_int (dither_rand);

                    gradient_render_pixel (x, y, &color, &rbd);

                    *dest++ = color.r + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
                    *dest++ = color.g + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
                    *dest++ = color.b + (gdouble) (i & 0xff) / 256.0 / 256.0; i >>= 8;
                    *dest++ = color.a + (gdouble) (i & 0xff) / 256.0 / 256.0;
                  }

              g_rand_free (dither_rand);
            }
          else
            {
              for (y = roi->y; y < endy; y++)
                for (x = roi->x; x < endx; x++)
                  {
                    PicmanRGB  color;

                    gradient_render_pixel (x, y, &color, &rbd);

                    *dest++ = color.r;
                    *dest++ = color.g;
                    *dest++ = color.b;
                    *dest++ = color.a;
                  }
            }

          done += roi->width * roi->height;

          if (progress)
            picman_progress_set_value (progress,
                                     (gdouble) done / (gdouble) total);
        }

      if (dither)
        g_rand_free (rbd.seed);
    }

#ifdef USE_GRADIENT_CACHE
  g_free (rbd.gradient_cache);
#endif

  g_object_unref (rbd.gradient);

  if (rbd.dist_buffer)
    g_object_unref (rbd.dist_buffer);

  PICMAN_TIMER_END("gradient_fill_region");
}
