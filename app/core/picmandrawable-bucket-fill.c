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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-mask.h"
#include "gegl/picman-gegl-mask-combine.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picmanchannel-combine.h"
#include "picmancontext.h"
#include "picmandrawable.h"
#include "picmandrawable-bucket-fill.h"
#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanimage-contiguous-region.h"
#include "picmanpattern.h"

#include "picman-intl.h"


static void   picman_drawable_bucket_fill_internal (PicmanDrawable        *drawable,
                                                  PicmanBucketFillMode   fill_mode,
                                                  gint                 paint_mode,
                                                  gdouble              opacity,
                                                  gboolean             fill_transparent,
                                                  PicmanSelectCriterion  fill_criterion,
                                                  gdouble              threshold,
                                                  gboolean             sample_merged,
                                                  gdouble              x,
                                                  gdouble              y,
                                                  const PicmanRGB       *color,
                                                  PicmanPattern         *pattern);


/*  public functions  */

gboolean
picman_drawable_bucket_fill (PicmanDrawable         *drawable,
                           PicmanContext          *context,
                           PicmanBucketFillMode    fill_mode,
                           gint                  paint_mode,
                           gdouble               opacity,
                           gboolean              fill_transparent,
                           PicmanSelectCriterion   fill_criterion,
                           gdouble               threshold,
                           gboolean              sample_merged,
                           gdouble               x,
                           gdouble               y,
                           GError              **error)
{
  PicmanRGB      color;
  PicmanPattern *pattern = NULL;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (fill_mode == PICMAN_FG_BUCKET_FILL)
    {
      picman_context_get_foreground (context, &color);
    }
  else if (fill_mode == PICMAN_BG_BUCKET_FILL)
    {
      picman_context_get_background (context, &color);
    }
  else if (fill_mode == PICMAN_PATTERN_BUCKET_FILL)
    {
      pattern = picman_context_get_pattern (context);

      if (! pattern)
        {
          g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			       _("No patterns available for this operation."));
          return FALSE;
        }
    }
  else
    {
      g_warning ("%s: invalid fill_mode passed", G_STRFUNC);
      return FALSE;
    }

  picman_drawable_bucket_fill_internal (drawable,
                                      fill_mode,
                                      paint_mode, opacity,
                                      fill_transparent, fill_criterion,
                                      threshold, sample_merged,
                                      x, y,
                                      &color, pattern);

  return TRUE;
}


/*  private functions  */

static void
picman_drawable_bucket_fill_internal (PicmanDrawable        *drawable,
                                    PicmanBucketFillMode   fill_mode,
                                    gint                 paint_mode,
                                    gdouble              opacity,
                                    gboolean             fill_transparent,
                                    PicmanSelectCriterion  fill_criterion,
                                    gdouble              threshold,
                                    gboolean             sample_merged,
                                    gdouble              x,
                                    gdouble              y,
                                    const PicmanRGB       *color,
                                    PicmanPattern         *pattern)
{
  PicmanImage   *image;
  GeglBuffer  *buffer;
  GeglBuffer  *mask_buffer;
  gint         x1, y1, x2, y2;
  gint         mask_offset_x = 0;
  gint         mask_offset_y = 0;
  gboolean     selection;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (fill_mode != PICMAN_PATTERN_BUCKET_FILL ||
                    PICMAN_IS_PATTERN (pattern));
  g_return_if_fail (fill_mode == PICMAN_PATTERN_BUCKET_FILL ||
                    color != NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  selection = picman_item_mask_bounds (PICMAN_ITEM (drawable), &x1, &y1, &x2, &y2);

  if ((x1 == x2) || (y1 == y2))
    return;

  picman_set_busy (image->picman);

  /*  Do a seed bucket fill...To do this, calculate a new
   *  contiguous region. If there is a selection, calculate the
   *  intersection of this region with the existing selection.
   */
  mask_buffer = picman_image_contiguous_region_by_seed (image, drawable,
                                                      sample_merged,
                                                      TRUE,
                                                      threshold,
                                                      fill_transparent,
                                                      fill_criterion,
                                                      (gint) x,
                                                      (gint) y);

  if (selection)
    {
      PicmanDrawable *sel;
      gint          off_x = 0;
      gint          off_y = 0;

      if (! sample_merged)
        picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

      sel = PICMAN_DRAWABLE (picman_image_get_mask (image));

      picman_gegl_mask_combine_buffer (mask_buffer,
                                     picman_drawable_get_buffer (sel),
                                     PICMAN_CHANNEL_OP_INTERSECT,
                                     -off_x, -off_y);
    }

  picman_gegl_mask_bounds (mask_buffer, &x1, &y1, &x2, &y2);

  /*  make sure we handle the mask correctly if it was sample-merged  */
  if (sample_merged)
    {
      PicmanItem *item = PICMAN_ITEM (drawable);
      gint      off_x, off_y;

      /*  Limit the channel bounds to the drawable's extents  */
      picman_item_get_offset (item, &off_x, &off_y);

      x1 = CLAMP (x1, off_x, (off_x + picman_item_get_width (item)));
      y1 = CLAMP (y1, off_y, (off_y + picman_item_get_height (item)));
      x2 = CLAMP (x2, off_x, (off_x + picman_item_get_width (item)));
      y2 = CLAMP (y2, off_y, (off_y + picman_item_get_height (item)));

      mask_offset_x = x1;
      mask_offset_y = y1;

     /*  translate mask bounds to drawable coords  */
      x1 -= off_x;
      y1 -= off_y;
      x2 -= off_x;
      y2 -= off_y;
    }
  else
    {
      mask_offset_x = x1;
      mask_offset_y = y1;
    }

  buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, x2 - x1, y2 - y1),
                            picman_drawable_get_format_with_alpha (drawable));

  switch (fill_mode)
    {
    case PICMAN_FG_BUCKET_FILL:
    case PICMAN_BG_BUCKET_FILL:
      {
        GeglColor *gegl_color = picman_gegl_color_new (color);

        gegl_buffer_set_color (buffer, NULL, gegl_color);
        g_object_unref (gegl_color);
      }
      break;

    case PICMAN_PATTERN_BUCKET_FILL:
      {
        GeglBuffer *pattern_buffer = picman_pattern_create_buffer (pattern);

        gegl_buffer_set_pattern (buffer, NULL, pattern_buffer, -x1, -y1);
        g_object_unref (pattern_buffer);
      }
      break;
    }

  picman_gegl_apply_opacity (buffer, NULL, NULL, buffer,
                           mask_buffer,
                           -mask_offset_x,
                           -mask_offset_y,
                           1.0);
  g_object_unref (mask_buffer);

  /*  Apply it to the image  */
  picman_drawable_apply_buffer (drawable, buffer,
                              GEGL_RECTANGLE (0, 0, x2 - x1, y2 - y1),
                              TRUE, C_("undo-type", "Bucket Fill"),
                              opacity, paint_mode,
                              NULL, x1, y1);

  g_object_unref (buffer);

  picman_drawable_update (drawable, x1, y1, x2 - x1, y2 - y1);

  picman_unset_busy (image->picman);
}
