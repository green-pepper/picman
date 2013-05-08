/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995-2003 Spencer Kimball, Peter Mattis, and others
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
#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picman-transform-resize.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmandrawable-transform.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"
#include "picmanpickable.h"
#include "picmanprogress.h"
#include "picmanselection.h"

#include "picman-intl.h"


#if defined (HAVE_FINITE)
#define FINITE(x) finite(x)
#elif defined (HAVE_ISFINITE)
#define FINITE(x) isfinite(x)
#elif defined (G_OS_WIN32)
#define FINITE(x) _finite(x)
#else
#error "no FINITE() implementation available?!"
#endif


/*  public functions  */

GeglBuffer *
picman_drawable_transform_buffer_affine (PicmanDrawable           *drawable,
                                       PicmanContext            *context,
                                       GeglBuffer             *orig_buffer,
                                       gint                    orig_offset_x,
                                       gint                    orig_offset_y,
                                       const PicmanMatrix3      *matrix,
                                       PicmanTransformDirection  direction,
                                       PicmanInterpolationType   interpolation_type,
                                       gint                    recursion_level,
                                       PicmanTransformResize     clip_result,
                                       gint                   *new_offset_x,
                                       gint                   *new_offset_y,
                                       PicmanProgress           *progress)
{
  GeglBuffer  *new_buffer;
  PicmanMatrix3  m;
  gint         u1, v1, u2, v2;  /* source bounding box */
  gint         x1, y1, x2, y2;  /* target bounding box */
  PicmanMatrix3  gegl_matrix;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (orig_buffer), NULL);
  g_return_val_if_fail (matrix != NULL, NULL);
  g_return_val_if_fail (new_offset_x != NULL, NULL);
  g_return_val_if_fail (new_offset_y != NULL, NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);

  m = *matrix;

  if (direction == PICMAN_TRANSFORM_BACKWARD)
    {
      /*  Find the inverse of the transformation matrix  */
      picman_matrix3_invert (&m);
    }

  u1 = orig_offset_x;
  v1 = orig_offset_y;
  u2 = u1 + gegl_buffer_get_width  (orig_buffer);
  v2 = v1 + gegl_buffer_get_height (orig_buffer);

  /*  Always clip unfloated buffers since they must keep their size  */
  if (G_TYPE_FROM_INSTANCE (drawable) == PICMAN_TYPE_CHANNEL &&
      ! babl_format_has_alpha (gegl_buffer_get_format (orig_buffer)))
    clip_result = PICMAN_TRANSFORM_RESIZE_CLIP;

  /*  Find the bounding coordinates of target */
  picman_transform_resize_boundary (&m, clip_result,
                                  u1, v1, u2, v2,
                                  &x1, &y1, &x2, &y2);

  /*  Get the new temporary buffer for the transformed result  */
  new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, x2 - x1, y2 - y1),
                                gegl_buffer_get_format (orig_buffer));

  picman_matrix3_identity (&gegl_matrix);
  picman_matrix3_translate (&gegl_matrix, u1, v1);
  picman_matrix3_mult (&m, &gegl_matrix);
  picman_matrix3_translate (&gegl_matrix, -x1, -y1);

  picman_gegl_apply_transform (orig_buffer, progress, NULL,
                             new_buffer,
                             interpolation_type,
                             &gegl_matrix);

  *new_offset_x = x1;
  *new_offset_y = y1;

  return new_buffer;
}

GeglBuffer *
picman_drawable_transform_buffer_flip (PicmanDrawable        *drawable,
                                     PicmanContext         *context,
                                     GeglBuffer          *orig_buffer,
                                     gint                 orig_offset_x,
                                     gint                 orig_offset_y,
                                     PicmanOrientationType  flip_type,
                                     gdouble              axis,
                                     gboolean             clip_result,
                                     gint                *new_offset_x,
                                     gint                *new_offset_y)
{
  GeglBuffer    *new_buffer;
  GeglRectangle  src_rect;
  GeglRectangle  dest_rect;
  gint           orig_x, orig_y;
  gint           orig_width, orig_height;
  gint           new_x, new_y;
  gint           new_width, new_height;
  gint           i;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (orig_buffer), NULL);

  orig_x      = orig_offset_x;
  orig_y      = orig_offset_y;
  orig_width  = gegl_buffer_get_width (orig_buffer);
  orig_height = gegl_buffer_get_height (orig_buffer);

  new_x      = orig_x;
  new_y      = orig_y;
  new_width  = orig_width;
  new_height = orig_height;

  switch (flip_type)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      new_x = RINT (-((gdouble) orig_x +
                      (gdouble) orig_width - axis) + axis);
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      new_y = RINT (-((gdouble) orig_y +
                      (gdouble) orig_height - axis) + axis);
      break;

    case PICMAN_ORIENTATION_UNKNOWN:
      g_return_val_if_reached (NULL);
      break;
    }

  new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                new_width, new_height),
                                gegl_buffer_get_format (orig_buffer));

  if (clip_result && (new_x != orig_x || new_y != orig_y))
    {
      PicmanRGB    bg;
      GeglColor *color;
      gint       clip_x, clip_y;
      gint       clip_width, clip_height;

      *new_offset_x = orig_x;
      *new_offset_y = orig_y;

      /*  "Outside" a channel is transparency, not the bg color  */
      if (PICMAN_IS_CHANNEL (drawable))
        picman_rgba_set (&bg, 0.0, 0.0, 0.0, 0.0);
      else
        picman_context_get_background (context, &bg);

      color = picman_gegl_color_new (&bg);
      gegl_buffer_set_color (new_buffer, NULL, color);
      g_object_unref (color);

      if (picman_rectangle_intersect (orig_x, orig_y, orig_width, orig_height,
                                    new_x, new_y, new_width, new_height,
                                    &clip_x, &clip_y,
                                    &clip_width, &clip_height))
        {
          orig_x = new_x = clip_x - orig_x;
          orig_y = new_y = clip_y - orig_y;
        }

      orig_width  = new_width  = clip_width;
      orig_height = new_height = clip_height;
    }
  else
    {
      *new_offset_x = new_x;
      *new_offset_y = new_y;

      orig_x = 0;
      orig_y = 0;
      new_x  = 0;
      new_y  = 0;
    }

  if (new_width == 0 && new_height == 0)
    return new_buffer;

  switch (flip_type)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      src_rect.x      = orig_x;
      src_rect.y      = orig_y;
      src_rect.width  = 1;
      src_rect.height = orig_height;

      dest_rect.x      = new_x + new_width - 1;
      dest_rect.y      = new_y;
      dest_rect.width  = 1;
      dest_rect.height = new_height;

      for (i = 0; i < orig_width; i++)
        {
          src_rect.x  = i + orig_x;
          dest_rect.x = new_x + new_width - i - 1;

          gegl_buffer_copy (orig_buffer, &src_rect,
                            new_buffer, &dest_rect);
        }
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      src_rect.x      = orig_x;
      src_rect.y      = orig_y;
      src_rect.width  = orig_width;
      src_rect.height = 1;

      dest_rect.x      = new_x;
      dest_rect.y      = new_y + new_height - 1;
      dest_rect.width  = new_width;
      dest_rect.height = 1;

      for (i = 0; i < orig_height; i++)
        {
          src_rect.y  = i + orig_y;
          dest_rect.y = new_y + new_height - i - 1;

          gegl_buffer_copy (orig_buffer, &src_rect,
                            new_buffer, &dest_rect);
        }
      break;

    case PICMAN_ORIENTATION_UNKNOWN:
      break;
    }

  return new_buffer;
}

static void
picman_drawable_transform_rotate_point (gint              x,
                                      gint              y,
                                      PicmanRotationType  rotate_type,
                                      gdouble           center_x,
                                      gdouble           center_y,
                                      gint             *new_x,
                                      gint             *new_y)
{
  g_return_if_fail (new_x != NULL);
  g_return_if_fail (new_y != NULL);

  switch (rotate_type)
    {
    case PICMAN_ROTATE_90:
      *new_x = RINT (center_x - (gdouble) y + center_y);
      *new_y = RINT (center_y + (gdouble) x - center_x);
      break;

    case PICMAN_ROTATE_180:
      *new_x = RINT (center_x - ((gdouble) x - center_x));
      *new_y = RINT (center_y - ((gdouble) y - center_y));
      break;

    case PICMAN_ROTATE_270:
      *new_x = RINT (center_x + (gdouble) y - center_y);
      *new_y = RINT (center_y - (gdouble) x + center_x);
      break;

    default:
      g_assert_not_reached ();
    }
}

GeglBuffer *
picman_drawable_transform_buffer_rotate (PicmanDrawable     *drawable,
                                       PicmanContext      *context,
                                       GeglBuffer       *orig_buffer,
                                       gint              orig_offset_x,
                                       gint              orig_offset_y,
                                       PicmanRotationType  rotate_type,
                                       gdouble           center_x,
                                       gdouble           center_y,
                                       gboolean          clip_result,
                                       gint             *new_offset_x,
                                       gint             *new_offset_y)
{
  GeglBuffer    *new_buffer;
  GeglRectangle  src_rect;
  GeglRectangle  dest_rect;
  gint           orig_x, orig_y;
  gint           orig_width, orig_height;
  gint           orig_bpp;
  gint           new_x, new_y;
  gint           new_width, new_height;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (orig_buffer), NULL);

  orig_x      = orig_offset_x;
  orig_y      = orig_offset_y;
  orig_width  = gegl_buffer_get_width (orig_buffer);
  orig_height = gegl_buffer_get_height (orig_buffer);
  orig_bpp    = babl_format_get_bytes_per_pixel (gegl_buffer_get_format (orig_buffer));

  switch (rotate_type)
    {
    case PICMAN_ROTATE_90:
      picman_drawable_transform_rotate_point (orig_x,
                                            orig_y + orig_height,
                                            rotate_type, center_x, center_y,
                                            &new_x, &new_y);
      new_width  = orig_height;
      new_height = orig_width;
      break;

    case PICMAN_ROTATE_180:
      picman_drawable_transform_rotate_point (orig_x + orig_width,
                                            orig_y + orig_height,
                                            rotate_type, center_x, center_y,
                                            &new_x, &new_y);
      new_width  = orig_width;
      new_height = orig_height;
      break;

    case PICMAN_ROTATE_270:
      picman_drawable_transform_rotate_point (orig_x + orig_width,
                                            orig_y,
                                            rotate_type, center_x, center_y,
                                            &new_x, &new_y);
      new_width  = orig_height;
      new_height = orig_width;
      break;

    default:
      g_return_val_if_reached (NULL);
      break;
    }

  if (clip_result && (new_x != orig_x || new_y != orig_y ||
                      new_width != orig_width || new_height != orig_height))

    {
      PicmanRGB    bg;
      GeglColor *color;
      gint       clip_x, clip_y;
      gint       clip_width, clip_height;

      new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                    orig_width, orig_height),
                                    gegl_buffer_get_format (orig_buffer));

      *new_offset_x = orig_x;
      *new_offset_y = orig_y;

      /*  "Outside" a channel is transparency, not the bg color  */
      if (PICMAN_IS_CHANNEL (drawable))
        picman_rgba_set (&bg, 0.0, 0.0, 0.0, 0.0);
      else
        picman_context_get_background (context, &bg);

      color = picman_gegl_color_new (&bg);
      gegl_buffer_set_color (new_buffer, NULL, color);
      g_object_unref (color);

      if (picman_rectangle_intersect (orig_x, orig_y, orig_width, orig_height,
                                    new_x, new_y, new_width, new_height,
                                    &clip_x, &clip_y,
                                    &clip_width, &clip_height))
        {
          gint saved_orig_x = orig_x;
          gint saved_orig_y = orig_y;

          new_x = clip_x - orig_x;
          new_y = clip_y - orig_y;

          switch (rotate_type)
            {
            case PICMAN_ROTATE_90:
              picman_drawable_transform_rotate_point (clip_x + clip_width,
                                                    clip_y,
                                                    PICMAN_ROTATE_270,
                                                    center_x,
                                                    center_y,
                                                    &orig_x,
                                                    &orig_y);
              orig_x      -= saved_orig_x;
              orig_y      -= saved_orig_y;
              orig_width   = clip_height;
              orig_height  = clip_width;
              break;

            case PICMAN_ROTATE_180:
              orig_x      = clip_x - orig_x;
              orig_y      = clip_y - orig_y;
              orig_width  = clip_width;
              orig_height = clip_height;
              break;

            case PICMAN_ROTATE_270:
              picman_drawable_transform_rotate_point (clip_x,
                                                    clip_y + clip_height,
                                                    PICMAN_ROTATE_90,
                                                    center_x,
                                                    center_y,
                                                    &orig_x,
                                                    &orig_y);
              orig_x      -= saved_orig_x;
              orig_y      -= saved_orig_y;
              orig_width   = clip_height;
              orig_height  = clip_width;
              break;
            }

          new_width  = clip_width;
          new_height = clip_height;
        }
      else
        {
          new_width  = 0;
          new_height = 0;
        }
    }
  else
    {
      new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                    new_width, new_height),
                                    gegl_buffer_get_format (orig_buffer));

      *new_offset_x = new_x;
      *new_offset_y = new_y;

      orig_x = 0;
      orig_y = 0;
      new_x  = 0;
      new_y  = 0;
    }

  if (new_width < 1 || new_height < 1)
    return new_buffer;

  src_rect.x      = orig_x;
  src_rect.y      = orig_y;
  src_rect.width  = orig_width;
  src_rect.height = orig_height;

  dest_rect.x      = new_x;
  dest_rect.y      = new_y;
  dest_rect.width  = new_width;
  dest_rect.height = new_height;

  switch (rotate_type)
    {
    case PICMAN_ROTATE_90:
      {
        guchar *buf = g_new (guchar, new_height * orig_bpp);
        gint    i;

        g_assert (new_height == orig_width);

        src_rect.y      = orig_y + orig_height - 1;
        src_rect.height = 1;

        dest_rect.x     = new_x;
        dest_rect.width = 1;

        for (i = 0; i < orig_height; i++)
          {
            src_rect.y  = orig_y + orig_height - 1 - i;
            dest_rect.x = new_x + i;

            gegl_buffer_get (orig_buffer, &src_rect, 1.0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
            gegl_buffer_set (new_buffer, &dest_rect, 0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE);
          }

        g_free (buf);
      }
      break;

    case PICMAN_ROTATE_180:
      {
        guchar *buf = g_new (guchar, new_width * orig_bpp);
        gint    i, j, k;

        g_assert (new_width == orig_width);

        src_rect.y      = orig_y + orig_height - 1;
        src_rect.height = 1;

        dest_rect.y      = new_y;
        dest_rect.height = 1;

        for (i = 0; i < orig_height; i++)
          {
            src_rect.y  = orig_y + orig_height - 1 - i;
            dest_rect.y = new_y + i;

            gegl_buffer_get (orig_buffer, &src_rect, 1.0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);

            for (j = 0; j < orig_width / 2; j++)
              {
                guchar *left  = buf + j * orig_bpp;
                guchar *right = buf + (orig_width - 1 - j) * orig_bpp;

                for (k = 0; k < orig_bpp; k++)
                  {
                    guchar tmp = left[k];
                    left[k]    = right[k];
                    right[k]   = tmp;
                  }
              }

            gegl_buffer_set (new_buffer, &dest_rect, 0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE);
          }

        g_free (buf);
      }
      break;

    case PICMAN_ROTATE_270:
      {
        guchar *buf = g_new (guchar, new_width * orig_bpp);
        gint    i;

        g_assert (new_width == orig_height);

        src_rect.x     = orig_x + orig_width - 1;
        src_rect.width = 1;

        dest_rect.y      = new_y;
        dest_rect.height = 1;

        for (i = 0; i < orig_width; i++)
          {
            src_rect.x  = orig_x + orig_width - 1 - i;
            dest_rect.y = new_y + i;

            gegl_buffer_get (orig_buffer, &src_rect, 1.0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_NONE);
            gegl_buffer_set (new_buffer, &dest_rect, 0, NULL, buf,
                             GEGL_AUTO_ROWSTRIDE);
          }

        g_free (buf);
      }
      break;
    }

  return new_buffer;
}

PicmanDrawable *
picman_drawable_transform_affine (PicmanDrawable           *drawable,
                                PicmanContext            *context,
                                const PicmanMatrix3      *matrix,
                                PicmanTransformDirection  direction,
                                PicmanInterpolationType   interpolation_type,
                                gint                    recursion_level,
                                PicmanTransformResize     clip_result,
                                PicmanProgress           *progress)
{
  PicmanImage    *image;
  GeglBuffer   *orig_buffer;
  gint          orig_offset_x;
  gint          orig_offset_y;
  gboolean      new_layer;
  PicmanDrawable *result = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (matrix != NULL, NULL);
  g_return_val_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress), NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  /* Start a transform undo group */
  picman_image_undo_group_start (image,
                               PICMAN_UNDO_GROUP_TRANSFORM,
                               C_("undo-type", "Transform"));

  /* Cut/Copy from the specified drawable */
  orig_buffer = picman_drawable_transform_cut (drawable, context,
                                             &orig_offset_x, &orig_offset_y,
                                             &new_layer);

  if (orig_buffer)
    {
      GeglBuffer *new_buffer;
      gint        new_offset_x;
      gint        new_offset_y;

      /*  always clip unfloated buffers so they keep their size  */
      if (PICMAN_IS_CHANNEL (drawable) &&
          ! babl_format_has_alpha (gegl_buffer_get_format (orig_buffer)))
        clip_result = PICMAN_TRANSFORM_RESIZE_CLIP;

      /*  also transform the mask if we are transforming an entire layer  */
      if (PICMAN_IS_LAYER (drawable) &&
          picman_layer_get_mask (PICMAN_LAYER (drawable)) &&
          picman_channel_is_empty (picman_image_get_mask (image)))
        {
          PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (drawable));

          picman_item_transform (PICMAN_ITEM (mask), context,
                               matrix,
                               direction,
                               interpolation_type,
                               recursion_level,
                               clip_result,
                               progress);
        }

      /* transform the buffer */
      new_buffer = picman_drawable_transform_buffer_affine (drawable, context,
                                                          orig_buffer,
                                                          orig_offset_x,
                                                          orig_offset_y,
                                                          matrix,
                                                          direction,
                                                          interpolation_type,
                                                          recursion_level,
                                                          clip_result,
                                                          &new_offset_x,
                                                          &new_offset_y,
                                                          progress);

      /* Free the cut/copied buffer */
      g_object_unref (orig_buffer);

      if (new_buffer)
        {
          result = picman_drawable_transform_paste (drawable, new_buffer,
                                                  new_offset_x, new_offset_y,
                                                  new_layer);
          g_object_unref (new_buffer);
        }
    }

  /*  push the undo group end  */
  picman_image_undo_group_end (image);

  return result;
}

PicmanDrawable *
picman_drawable_transform_flip (PicmanDrawable        *drawable,
                              PicmanContext         *context,
                              PicmanOrientationType  flip_type,
                              gdouble              axis,
                              gboolean             clip_result)
{
  PicmanImage    *image;
  GeglBuffer   *orig_buffer;
  gint          orig_offset_x;
  gint          orig_offset_y;
  gboolean      new_layer;
  PicmanDrawable *result = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  /* Start a transform undo group */
  picman_image_undo_group_start (image,
                               PICMAN_UNDO_GROUP_TRANSFORM,
                               C_("undo-type", "Flip"));

  /* Cut/Copy from the specified drawable */
  orig_buffer = picman_drawable_transform_cut (drawable, context,
                                             &orig_offset_x, &orig_offset_y,
                                             &new_layer);

  if (orig_buffer)
    {
      GeglBuffer *new_buffer = NULL;
      gint        new_offset_x;
      gint        new_offset_y;

      /*  always clip unfloated buffers so they keep their size  */
      if (PICMAN_IS_CHANNEL (drawable) &&
          ! babl_format_has_alpha (gegl_buffer_get_format (orig_buffer)))
        clip_result = TRUE;

      /*  also transform the mask if we are transforming an entire layer  */
      if (PICMAN_IS_LAYER (drawable) &&
          picman_layer_get_mask (PICMAN_LAYER (drawable)) &&
          picman_channel_is_empty (picman_image_get_mask (image)))
        {
          PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (drawable));

          picman_item_flip (PICMAN_ITEM (mask), context,
                          flip_type,
                          axis,
                          clip_result);
        }

      /* transform the buffer */
      if (orig_buffer)
        {
          new_buffer = picman_drawable_transform_buffer_flip (drawable, context,
                                                            orig_buffer,
                                                            orig_offset_x,
                                                            orig_offset_y,
                                                            flip_type, axis,
                                                            clip_result,
                                                            &new_offset_x,
                                                            &new_offset_y);

          /* Free the cut/copied buffer */
          g_object_unref (orig_buffer);
        }

      if (new_buffer)
        {
          result = picman_drawable_transform_paste (drawable, new_buffer,
                                                  new_offset_x, new_offset_y,
                                                  new_layer);
          g_object_unref (new_buffer);
        }
    }

  /*  push the undo group end  */
  picman_image_undo_group_end (image);

  return result;
}

PicmanDrawable *
picman_drawable_transform_rotate (PicmanDrawable     *drawable,
                                PicmanContext      *context,
                                PicmanRotationType  rotate_type,
                                gdouble           center_x,
                                gdouble           center_y,
                                gboolean          clip_result)
{
  PicmanImage    *image;
  GeglBuffer   *orig_buffer;
  gint          orig_offset_x;
  gint          orig_offset_y;
  gboolean      new_layer;
  PicmanDrawable *result = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  /* Start a transform undo group */
  picman_image_undo_group_start (image,
                               PICMAN_UNDO_GROUP_TRANSFORM,
                               C_("undo-type", "Rotate"));

  /* Cut/Copy from the specified drawable */
  orig_buffer = picman_drawable_transform_cut (drawable, context,
                                             &orig_offset_x, &orig_offset_y,
                                             &new_layer);

  if (orig_buffer)
    {
      GeglBuffer *new_buffer;
      gint        new_offset_x;
      gint        new_offset_y;

      /*  always clip unfloated buffers so they keep their size  */
      if (PICMAN_IS_CHANNEL (drawable) &&
          ! babl_format_has_alpha (gegl_buffer_get_format (orig_buffer)))
        clip_result = TRUE;

      /*  also transform the mask if we are transforming an entire layer  */
      if (PICMAN_IS_LAYER (drawable) &&
          picman_layer_get_mask (PICMAN_LAYER (drawable)) &&
          picman_channel_is_empty (picman_image_get_mask (image)))
        {
          PicmanLayerMask *mask = picman_layer_get_mask (PICMAN_LAYER (drawable));

          picman_item_rotate (PICMAN_ITEM (mask), context,
                            rotate_type,
                            center_x,
                            center_y,
                            clip_result);
        }

      /* transform the buffer */
      new_buffer = picman_drawable_transform_buffer_rotate (drawable, context,
                                                          orig_buffer,
                                                          orig_offset_x,
                                                          orig_offset_y,
                                                          rotate_type,
                                                          center_x, center_y,
                                                          clip_result,
                                                          &new_offset_x,
                                                          &new_offset_y);

      /* Free the cut/copied buffer */
      g_object_unref (orig_buffer);

      if (new_buffer)
        {
          result = picman_drawable_transform_paste (drawable, new_buffer,
                                                  new_offset_x, new_offset_y,
                                                  new_layer);
          g_object_unref (new_buffer);
        }
    }

  /*  push the undo group end  */
  picman_image_undo_group_end (image);

  return result;
}

GeglBuffer *
picman_drawable_transform_cut (PicmanDrawable *drawable,
                             PicmanContext  *context,
                             gint         *offset_x,
                             gint         *offset_y,
                             gboolean     *new_layer)
{
  PicmanImage  *image;
  GeglBuffer *buffer;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (offset_x != NULL, NULL);
  g_return_val_if_fail (offset_y != NULL, NULL);
  g_return_val_if_fail (new_layer != NULL, NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  /*  extract the selected mask if there is a selection  */
  if (! picman_channel_is_empty (picman_image_get_mask (image)))
    {
      gint x, y, w, h;

      /* set the keep_indexed flag to FALSE here, since we use
       * picman_layer_new_from_buffer() later which assumes that the buffer
       * are either RGB or GRAY.  Eeek!!!              (Sven)
       */
      if (picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &w, &h))
        {
          buffer = picman_selection_extract (PICMAN_SELECTION (picman_image_get_mask (image)),
                                           PICMAN_PICKABLE (drawable),
                                           context,
                                           TRUE, FALSE, TRUE,
                                           offset_x, offset_y,
                                           NULL);
          /*  clear the selection  */
          picman_channel_clear (picman_image_get_mask (image), NULL, TRUE);

          *new_layer = TRUE;
        }
      else
        {
          buffer = NULL;
          *new_layer = FALSE;
        }
    }
  else  /*  otherwise, just copy the layer  */
    {
      buffer = picman_selection_extract (PICMAN_SELECTION (picman_image_get_mask (image)),
                                       PICMAN_PICKABLE (drawable),
                                       context,
                                       FALSE, TRUE, PICMAN_IS_LAYER (drawable),
                                       offset_x, offset_y,
                                       NULL);

      *new_layer = FALSE;
    }

  return buffer;
}

PicmanDrawable *
picman_drawable_transform_paste (PicmanDrawable *drawable,
                               GeglBuffer   *buffer,
                               gint          offset_x,
                               gint          offset_y,
                               gboolean      new_layer)
{
  PicmanImage   *image;
  PicmanLayer   *layer     = NULL;
  const gchar *undo_desc = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  if (PICMAN_IS_LAYER (drawable))
    undo_desc = C_("undo-type", "Transform Layer");
  else if (PICMAN_IS_CHANNEL (drawable))
    undo_desc = C_("undo-type", "Transform Channel");
  else
    return NULL;

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_EDIT_PASTE, undo_desc);

  if (new_layer)
    {
      layer =
        picman_layer_new_from_buffer (buffer, image,
                                    picman_drawable_get_format_with_alpha (drawable),
                                    _("Transformation"),
                                    PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

      picman_item_set_offset (PICMAN_ITEM (layer), offset_x, offset_y);

      floating_sel_attach (layer, drawable);

      drawable = PICMAN_DRAWABLE (layer);
    }
  else
    {
      picman_drawable_set_buffer_full (drawable, TRUE, NULL,
                                     buffer,
                                     offset_x, offset_y);
    }

  picman_image_undo_group_end (image);

  return drawable;
}
