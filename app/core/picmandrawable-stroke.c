/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmandrawable-stroke.c
 * Copyright (C) 2003 Simon Budig  <simon@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picmanbezierdesc.h"
#include "picmanboundary.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmandrawable-stroke.h"
#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanpattern.h"
#include "picmanscanconvert.h"
#include "picmanstrokeoptions.h"

#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "picman-intl.h"


/*  local function prototypes  */

static PicmanScanConvert * picman_drawable_render_boundary     (PicmanDrawable        *drawable,
                                                            const PicmanBoundSeg  *bound_segs,
                                                            gint                 n_bound_segs,
                                                            gint                 offset_x,
                                                            gint                 offset_y);
static PicmanScanConvert * picman_drawable_render_vectors      (PicmanDrawable        *drawable,
                                                            PicmanVectors         *vectors,
                                                            gboolean             do_stroke,
                                                            GError             **error);
static void              picman_drawable_stroke_scan_convert (PicmanDrawable        *drawable,
                                                            PicmanFillOptions     *options,
                                                            PicmanScanConvert     *scan_convert,
                                                            gboolean             do_stroke,
                                                            gboolean             push_undo);


/*  public functions  */

void
picman_drawable_fill_boundary (PicmanDrawable       *drawable,
                             PicmanFillOptions    *options,
                             const PicmanBoundSeg *bound_segs,
                             gint                n_bound_segs,
                             gint                offset_x,
                             gint                offset_y,
                             gboolean            push_undo)
{
  PicmanScanConvert *scan_convert;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (PICMAN_IS_STROKE_OPTIONS (options));
  g_return_if_fail (bound_segs == NULL || n_bound_segs != 0);
  g_return_if_fail (picman_fill_options_get_style (options) !=
                    PICMAN_FILL_STYLE_PATTERN ||
                    picman_context_get_pattern (PICMAN_CONTEXT (options)) != NULL);

  scan_convert = picman_drawable_render_boundary (drawable,
                                                bound_segs, n_bound_segs,
                                                offset_x, offset_y);

  if (scan_convert)
    {
      picman_drawable_stroke_scan_convert (drawable, options,
                                         scan_convert, FALSE, push_undo);
      picman_scan_convert_free (scan_convert);
    }
}

void
picman_drawable_stroke_boundary (PicmanDrawable       *drawable,
                               PicmanStrokeOptions  *options,
                               const PicmanBoundSeg *bound_segs,
                               gint                n_bound_segs,
                               gint                offset_x,
                               gint                offset_y,
                               gboolean            push_undo)
{
  PicmanScanConvert *scan_convert;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (PICMAN_IS_STROKE_OPTIONS (options));
  g_return_if_fail (bound_segs == NULL || n_bound_segs != 0);
  g_return_if_fail (picman_fill_options_get_style (PICMAN_FILL_OPTIONS (options)) !=
                    PICMAN_FILL_STYLE_PATTERN ||
                    picman_context_get_pattern (PICMAN_CONTEXT (options)) != NULL);

  scan_convert = picman_drawable_render_boundary (drawable,
                                                bound_segs, n_bound_segs,
                                                offset_x, offset_y);

  if (scan_convert)
    {
      picman_drawable_stroke_scan_convert (drawable, PICMAN_FILL_OPTIONS (options),
                                         scan_convert, TRUE, push_undo);
      picman_scan_convert_free (scan_convert);
    }
}

gboolean
picman_drawable_fill_vectors (PicmanDrawable     *drawable,
                            PicmanFillOptions  *options,
                            PicmanVectors      *vectors,
                            gboolean          push_undo,
                            GError          **error)
{
  PicmanScanConvert *scan_convert;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_FILL_OPTIONS (options), FALSE);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (picman_fill_options_get_style (options) !=
                        PICMAN_FILL_STYLE_PATTERN ||
                        picman_context_get_pattern (PICMAN_CONTEXT (options)) != NULL,
                        FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  scan_convert = picman_drawable_render_vectors (drawable, vectors, FALSE, error);

  if (scan_convert)
    {
      picman_drawable_stroke_scan_convert (drawable, options,
                                         scan_convert, FALSE, push_undo);
      picman_scan_convert_free (scan_convert);

      return TRUE;
    }

  return FALSE;
}

gboolean
picman_drawable_stroke_vectors (PicmanDrawable       *drawable,
                              PicmanStrokeOptions  *options,
                              PicmanVectors        *vectors,
                              gboolean            push_undo,
                              GError            **error)
{
  PicmanScanConvert *scan_convert;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_STROKE_OPTIONS (options), FALSE);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (picman_fill_options_get_style (PICMAN_FILL_OPTIONS (options)) !=
                        PICMAN_FILL_STYLE_PATTERN ||
                        picman_context_get_pattern (PICMAN_CONTEXT (options)) != NULL,
                        FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  scan_convert = picman_drawable_render_vectors (drawable, vectors, TRUE, error);

  if (scan_convert)
    {
      picman_drawable_stroke_scan_convert (drawable, PICMAN_FILL_OPTIONS (options),
                                         scan_convert, TRUE, push_undo);
      picman_scan_convert_free (scan_convert);

      return TRUE;
    }

  return FALSE;
}


/*  private functions  */

static PicmanScanConvert *
picman_drawable_render_boundary (PicmanDrawable       *drawable,
                               const PicmanBoundSeg *bound_segs,
                               gint                n_bound_segs,
                               gint                offset_x,
                               gint                offset_y)
{
  if (bound_segs)
    {
      PicmanBoundSeg *stroke_segs;
      gint          n_stroke_segs;

      stroke_segs = picman_boundary_sort (bound_segs, n_bound_segs,
                                        &n_stroke_segs);

      if (stroke_segs)
        {
          PicmanBezierDesc *bezier;

          bezier = picman_bezier_desc_new_from_bound_segs (stroke_segs,
                                                         n_bound_segs,
                                                         n_stroke_segs);

          g_free (stroke_segs);

          if (bezier)
            {
              PicmanScanConvert *scan_convert;

              scan_convert = picman_scan_convert_new ();

              picman_bezier_desc_translate (bezier, offset_x, offset_y);
              picman_scan_convert_add_bezier (scan_convert, bezier);

              picman_bezier_desc_free (bezier);

              return scan_convert;
            }
        }
    }

  return NULL;
}

static PicmanScanConvert *
picman_drawable_render_vectors (PicmanDrawable  *drawable,
                              PicmanVectors   *vectors,
                              gboolean       do_stroke,
                              GError       **error)
{
  const PicmanBezierDesc *bezier;

  bezier = picman_vectors_get_bezier (vectors);

  if (bezier && (do_stroke ? bezier->num_data >= 2 : bezier->num_data > 4))
    {
      PicmanScanConvert *scan_convert;

      scan_convert = picman_scan_convert_new ();
      picman_scan_convert_add_bezier (scan_convert, bezier);

      return scan_convert;
    }

  g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                       do_stroke ?
                       _("Not enough points to stroke") :
                       _("Not enough points to fill"));

  return NULL;
}

static void
picman_drawable_stroke_scan_convert (PicmanDrawable    *drawable,
                                   PicmanFillOptions *options,
                                   PicmanScanConvert *scan_convert,
                                   gboolean         do_stroke,
                                   gboolean         push_undo)
{
  PicmanContext *context = PICMAN_CONTEXT (options);
  PicmanImage   *image   = picman_item_get_image (PICMAN_ITEM (drawable));
  GeglBuffer  *base_buffer;
  GeglBuffer  *mask_buffer;
  gint         x, y, w, h;
  gint         off_x;
  gint         off_y;

  /*  must call picman_channel_is_empty() instead of relying on
   *  picman_item_mask_intersect() because the selection pretends to
   *  be empty while it is being stroked, to prevent masking itself.
   */
  if (picman_channel_is_empty (picman_image_get_mask (image)))
    {
      x = 0;
      y = 0;
      w = picman_item_get_width  (PICMAN_ITEM (drawable));
      h = picman_item_get_height (PICMAN_ITEM (drawable));
    }
  else if (! picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &w, &h))
    {
      return;
    }

  if (do_stroke)
    {
      PicmanStrokeOptions *stroke_options = PICMAN_STROKE_OPTIONS (options);
      gdouble            width;
      PicmanUnit           unit;

      width = picman_stroke_options_get_width (stroke_options);
      unit  = picman_stroke_options_get_unit (stroke_options);

      if (unit != PICMAN_UNIT_PIXEL)
        {
          gdouble xres;
          gdouble yres;

          picman_image_get_resolution (image, &xres, &yres);

          picman_scan_convert_set_pixel_ratio (scan_convert, yres / xres);

          width = picman_units_to_pixels (width, unit, yres);
        }

      picman_scan_convert_stroke (scan_convert, width,
                                picman_stroke_options_get_join_style (stroke_options),
                                picman_stroke_options_get_cap_style (stroke_options),
                                picman_stroke_options_get_miter_limit (stroke_options),
                                picman_stroke_options_get_dash_offset (stroke_options),
                                picman_stroke_options_get_dash_info (stroke_options));
    }

  /* fill a 1-bpp GeglBuffer with black, this will describe the shape
   * of the stroke.
   */
  mask_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, w, h),
                                 babl_format ("Y u8"));

  /* render the stroke into it */
  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  picman_scan_convert_render (scan_convert, mask_buffer,
                            x + off_x, y + off_y,
                            picman_fill_options_get_antialias (options));

  base_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, w, h),
                                 picman_drawable_get_format_with_alpha (drawable));

  switch (picman_fill_options_get_style (options))
    {
    case PICMAN_FILL_STYLE_SOLID:
      {
        PicmanRGB    fg;
        GeglColor *color;

        picman_context_get_foreground (context, &fg);

        color = picman_gegl_color_new (&fg);
        gegl_buffer_set_color (base_buffer, NULL, color);
        g_object_unref (color);
      }
      break;

    case PICMAN_FILL_STYLE_PATTERN:
      {
        PicmanPattern *pattern = picman_context_get_pattern (context);
        GeglBuffer  *pattern_buffer;

        pattern_buffer = picman_pattern_create_buffer (pattern);
        gegl_buffer_set_pattern (base_buffer, NULL, pattern_buffer, 0, 0);
        g_object_unref (pattern_buffer);
      }
      break;
    }

  picman_gegl_apply_opacity (base_buffer, NULL, NULL, base_buffer,
                           mask_buffer, 0, 0, 1.0);
  g_object_unref (mask_buffer);

  /* Apply to drawable */
  picman_drawable_apply_buffer (drawable, base_buffer,
                              GEGL_RECTANGLE (0, 0, w, h),
                              push_undo, C_("undo-type", "Render Stroke"),
                              picman_context_get_opacity (context),
                              picman_context_get_paint_mode (context),
                              NULL, x, y);

  g_object_unref (base_buffer);

  picman_drawable_update (drawable, x, y, w, h);
}
