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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-mask-combine.h"

#include "picmanchannel.h"
#include "picmanchannel-select.h"
#include "picmanchannel-combine.h"
#include "picmanimage-contiguous-region.h"
#include "picmanscanconvert.h"

#include "vectors/picmanstroke.h"
#include "vectors/picmanvectors.h"

#include "picman-intl.h"


/*  basic selection functions  */

void
picman_channel_select_rectangle (PicmanChannel    *channel,
                               gint            x,
                               gint            y,
                               gint            w,
                               gint            h,
                               PicmanChannelOps  op,
                               gboolean        feather,
                               gdouble         feather_radius_x,
                               gdouble         feather_radius_y,
                               gboolean        push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));

  if (push_undo)
    picman_channel_push_undo (channel, C_("undo-type", "Rectangle Select"));

  /*  if applicable, replace the current selection  */
  if (op == PICMAN_CHANNEL_OP_REPLACE)
    picman_channel_clear (channel, NULL, FALSE);

  /*  if feathering for rect, make a new mask with the
   *  rectangle and feather that with the old mask
   */
  if (feather || op == PICMAN_CHANNEL_OP_INTERSECT)
    {
      PicmanItem   *item = PICMAN_ITEM (channel);
      GeglBuffer *add_on;

      add_on = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                picman_item_get_width  (item),
                                                picman_item_get_height (item)),
                                babl_format ("Y float"));

      picman_gegl_mask_combine_rect (add_on, PICMAN_CHANNEL_OP_ADD, x, y, w, h);

      if (feather)
        picman_gegl_apply_feather (add_on, NULL, NULL, add_on,
                                 feather_radius_x,
                                 feather_radius_y);

      picman_channel_combine_buffer (channel, add_on, op, 0, 0);
      g_object_unref (add_on);
    }
  else
    {
      picman_channel_combine_rect (channel, op, x, y, w, h);
    }
}

void
picman_channel_select_ellipse (PicmanChannel    *channel,
                             gint            x,
                             gint            y,
                             gint            w,
                             gint            h,
                             PicmanChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y,
                             gboolean        push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));

  if (push_undo)
    picman_channel_push_undo (channel, C_("undo-type", "Ellipse Select"));

  /*  if applicable, replace the current selection  */
  if (op == PICMAN_CHANNEL_OP_REPLACE)
    picman_channel_clear (channel, NULL, FALSE);

  /*  if feathering for rect, make a new mask with the
   *  rectangle and feather that with the old mask
   */
  if (feather || op == PICMAN_CHANNEL_OP_INTERSECT)
    {
      PicmanItem   *item = PICMAN_ITEM (channel);
      GeglBuffer *add_on;

      add_on = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                picman_item_get_width  (item),
                                                picman_item_get_height (item)),
                                babl_format ("Y float"));

      picman_gegl_mask_combine_ellipse (add_on, PICMAN_CHANNEL_OP_ADD,
                                      x, y, w, h, antialias);

      if (feather)
        picman_gegl_apply_feather (add_on, NULL, NULL, add_on,
                                 feather_radius_x,
                                 feather_radius_y);

      picman_channel_combine_buffer (channel, add_on, op, 0, 0);
      g_object_unref (add_on);
    }
  else
    {
      picman_channel_combine_ellipse (channel, op, x, y, w, h, antialias);
    }
}

void
picman_channel_select_round_rect (PicmanChannel         *channel,
                                gint                 x,
                                gint                 y,
                                gint                 w,
                                gint                 h,
                                gdouble              corner_radius_x,
                                gdouble              corner_radius_y,
                                PicmanChannelOps       op,
                                gboolean             antialias,
                                gboolean             feather,
                                gdouble              feather_radius_x,
                                gdouble              feather_radius_y,
                                gboolean             push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));

  if (push_undo)
    picman_channel_push_undo (channel, C_("undo-type", "Rounded Rectangle Select"));

  /*  if applicable, replace the current selection  */
  if (op == PICMAN_CHANNEL_OP_REPLACE)
    picman_channel_clear (channel, NULL, FALSE);

  /*  if feathering for rect, make a new mask with the
   *  rectangle and feather that with the old mask
   */
  if (feather || op == PICMAN_CHANNEL_OP_INTERSECT)
    {
      PicmanItem   *item = PICMAN_ITEM (channel);
      GeglBuffer *add_on;

      add_on = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                picman_item_get_width  (item),
                                                picman_item_get_height (item)),
                                babl_format ("Y float"));

      picman_gegl_mask_combine_ellipse_rect (add_on, PICMAN_CHANNEL_OP_ADD,
                                           x, y, w, h,
                                           corner_radius_x, corner_radius_y,
                                           antialias);

      if (feather)
        picman_gegl_apply_feather (add_on, NULL, NULL, add_on,
                                 feather_radius_x,
                                 feather_radius_y);

      picman_channel_combine_buffer (channel, add_on, op, 0, 0);
      g_object_unref (add_on);
    }
  else
    {
      picman_channel_combine_ellipse_rect (channel, op, x, y, w, h,
                                         corner_radius_x, corner_radius_y,
                                         antialias);
    }
}

/*  select by PicmanScanConvert functions  */

void
picman_channel_select_scan_convert (PicmanChannel     *channel,
                                  const gchar     *undo_desc,
                                  PicmanScanConvert *scan_convert,
                                  gint             offset_x,
                                  gint             offset_y,
                                  PicmanChannelOps   op,
                                  gboolean         antialias,
                                  gboolean         feather,
                                  gdouble          feather_radius_x,
                                  gdouble          feather_radius_y,
                                  gboolean         push_undo)
{
  PicmanItem   *item;
  GeglBuffer *add_on;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (scan_convert != NULL);

  if (push_undo)
    picman_channel_push_undo (channel, undo_desc);

  /*  if applicable, replace the current selection  */
  if (op == PICMAN_CHANNEL_OP_REPLACE)
    picman_channel_clear (channel, NULL, FALSE);

  item = PICMAN_ITEM (channel);

  add_on = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                            picman_item_get_width  (item),
                                            picman_item_get_height (item)),
                            babl_format ("Y float"));

  picman_scan_convert_render (scan_convert, add_on,
                            offset_x, offset_y, antialias);

  if (feather)
    picman_gegl_apply_feather (add_on, NULL, NULL, add_on,
                             feather_radius_x,
                             feather_radius_y);

  picman_channel_combine_buffer (channel, add_on, op, 0, 0);
  g_object_unref (add_on);
}

void
picman_channel_select_polygon (PicmanChannel    *channel,
                             const gchar    *undo_desc,
                             gint            n_points,
                             PicmanVector2    *points,
                             PicmanChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y,
                             gboolean        push_undo)
{
  PicmanScanConvert *scan_convert;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (undo_desc != NULL);

  scan_convert = picman_scan_convert_new ();

  picman_scan_convert_add_polyline (scan_convert, n_points, points, TRUE);

  picman_channel_select_scan_convert (channel, undo_desc, scan_convert, 0, 0,
                                    op, antialias, feather,
                                    feather_radius_x, feather_radius_y,
                                    push_undo);

  picman_scan_convert_free (scan_convert);
}

void
picman_channel_select_vectors (PicmanChannel    *channel,
                             const gchar    *undo_desc,
                             PicmanVectors    *vectors,
                             PicmanChannelOps  op,
                             gboolean        antialias,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y,
                             gboolean        push_undo)
{
  const PicmanBezierDesc *bezier;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));

  bezier = picman_vectors_get_bezier (vectors);

  if (bezier && bezier->num_data > 4)
    {
      PicmanScanConvert *scan_convert;

      scan_convert = picman_scan_convert_new ();
      picman_scan_convert_add_bezier (scan_convert, bezier);

      picman_channel_select_scan_convert (channel, undo_desc, scan_convert, 0, 0,
                                        op, antialias, feather,
                                        feather_radius_x, feather_radius_y,
                                        push_undo);

      picman_scan_convert_free (scan_convert);
    }
}


/*  select by PicmanChannel functions  */

void
picman_channel_select_buffer (PicmanChannel    *channel,
                            const gchar    *undo_desc,
                            GeglBuffer     *add_on,
                            gint            offset_x,
                            gint            offset_y,
                            PicmanChannelOps  op,
                            gboolean        feather,
                            gdouble         feather_radius_x,
                            gdouble         feather_radius_y)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (GEGL_IS_BUFFER (add_on));

  picman_channel_push_undo (channel, undo_desc);

  /*  if applicable, replace the current selection  */
  if (op == PICMAN_CHANNEL_OP_REPLACE)
    picman_channel_clear (channel, NULL, FALSE);

  if (feather || op == PICMAN_CHANNEL_OP_INTERSECT)
    {
      PicmanItem   *item = PICMAN_ITEM (channel);
      GeglBuffer *add_on2;

      add_on2 = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                 picman_item_get_width  (item),
                                                 picman_item_get_height (item)),
                                 babl_format ("Y float"));

      picman_gegl_mask_combine_buffer (add_on2, add_on,
                                     PICMAN_CHANNEL_OP_ADD,
                                     offset_x, offset_y);

      if (feather)
        picman_gegl_apply_feather (add_on2, NULL, NULL, add_on2,
                                 feather_radius_x,
                                 feather_radius_y);

      picman_channel_combine_buffer (channel, add_on2, op, 0, 0);
      g_object_unref (add_on2);
    }
  else
    {
      picman_channel_combine_buffer (channel, add_on, op, offset_x, offset_y);
    }
}

void
picman_channel_select_channel (PicmanChannel    *channel,
                             const gchar    *undo_desc,
                             PicmanChannel    *add_on,
                             gint            offset_x,
                             gint            offset_y,
                             PicmanChannelOps  op,
                             gboolean        feather,
                             gdouble         feather_radius_x,
                             gdouble         feather_radius_y)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (undo_desc != NULL);
  g_return_if_fail (PICMAN_IS_CHANNEL (add_on));

  picman_channel_select_buffer (channel, undo_desc,
                              picman_drawable_get_buffer (PICMAN_DRAWABLE (add_on)),
                              offset_x, offset_y, op,
                              feather,
                              feather_radius_x, feather_radius_y);
}

void
picman_channel_select_alpha (PicmanChannel    *channel,
                           PicmanDrawable   *drawable,
                           PicmanChannelOps  op,
                           gboolean        feather,
                           gdouble         feather_radius_x,
                           gdouble         feather_radius_y)
{
  PicmanItem    *item;
  PicmanChannel *add_on;
  gint         off_x, off_y;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  item = PICMAN_ITEM (channel);

  if (picman_drawable_has_alpha (drawable))
    {
      add_on = picman_channel_new_from_alpha (picman_item_get_image (item),
                                            drawable, NULL, NULL);
    }
  else
    {
      /*  no alpha is equivalent to completely opaque alpha,
       *  so simply select the whole layer's extents.  --mitch
       */
      add_on = picman_channel_new_mask (picman_item_get_image (item),
                                      picman_item_get_width  (PICMAN_ITEM (drawable)),
                                      picman_item_get_height (PICMAN_ITEM (drawable)));
      picman_channel_all (add_on, FALSE);
    }

  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  picman_channel_select_channel (channel, C_("undo-type", "Alpha to Selection"), add_on,
                               off_x, off_y,
                               op, feather,
                               feather_radius_x,
                               feather_radius_y);
  g_object_unref (add_on);
}

void
picman_channel_select_component (PicmanChannel     *channel,
                               PicmanChannelType  component,
                               PicmanChannelOps   op,
                               gboolean         feather,
                               gdouble          feather_radius_x,
                               gdouble          feather_radius_y)
{
  PicmanItem    *item;
  PicmanChannel *add_on;
  const gchar *desc;
  gchar       *undo_desc;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));

  item = PICMAN_ITEM (channel);

  add_on = picman_channel_new_from_component (picman_item_get_image (item),
                                            component, NULL, NULL);

  if (feather)
    picman_channel_feather (add_on,
                          feather_radius_x,
                          feather_radius_y,
                          FALSE /* no undo */);

  picman_enum_get_value (PICMAN_TYPE_CHANNEL_TYPE, component,
                       NULL, NULL, &desc, NULL);

  undo_desc = g_strdup_printf (C_("undo-type", "%s Channel to Selection"), desc);

  picman_channel_select_channel (channel, undo_desc, add_on,
                               0, 0, op,
                               FALSE, 0.0, 0.0);

  g_free (undo_desc);
  g_object_unref (add_on);
}

void
picman_channel_select_fuzzy (PicmanChannel         *channel,
                           PicmanDrawable        *drawable,
                           gboolean             sample_merged,
                           gint                 x,
                           gint                 y,
                           gfloat               threshold,
                           gboolean             select_transparent,
                           PicmanSelectCriterion  select_criterion,
                           PicmanChannelOps       op,
                           gboolean             antialias,
                           gboolean             feather,
                           gdouble              feather_radius_x,
                           gdouble              feather_radius_y)
{
  PicmanItem   *item;
  GeglBuffer *add_on;
  gint        add_on_x = 0;
  gint        add_on_y = 0;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  item = PICMAN_ITEM (channel);

  add_on = picman_image_contiguous_region_by_seed (picman_item_get_image (item),
                                                 drawable,
                                                 sample_merged,
                                                 antialias,
                                                 threshold,
                                                 select_transparent,
                                                 select_criterion,
                                                 x, y);

  if (! sample_merged)
    picman_item_get_offset (PICMAN_ITEM (drawable), &add_on_x, &add_on_y);

  picman_channel_select_buffer (channel, C_("undo-type", "Fuzzy Select"),
                              add_on, add_on_x, add_on_y,
                              op,
                              feather,
                              feather_radius_x,
                              feather_radius_y);
  g_object_unref (add_on);
}

void
picman_channel_select_by_color (PicmanChannel         *channel,
                              PicmanDrawable        *drawable,
                              gboolean             sample_merged,
                              const PicmanRGB       *color,
                              gfloat               threshold,
                              gboolean             select_transparent,
                              PicmanSelectCriterion  select_criterion,
                              PicmanChannelOps       op,
                              gboolean             antialias,
                              gboolean             feather,
                              gdouble              feather_radius_x,
                              gdouble              feather_radius_y)
{
  PicmanItem   *item;
  GeglBuffer *add_on;
  gint        add_on_x = 0;
  gint        add_on_y = 0;

  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (color != NULL);

  item = PICMAN_ITEM (channel);

  add_on = picman_image_contiguous_region_by_color (picman_item_get_image (item),
                                                  drawable,
                                                  sample_merged,
                                                  antialias,
                                                  threshold,
                                                  select_transparent,
                                                  select_criterion,
                                                  color);

  if (! sample_merged)
    picman_item_get_offset (PICMAN_ITEM (drawable), &add_on_x, &add_on_y);

  picman_channel_select_buffer (channel, C_("undo-type", "Select by Color"),
                              add_on, add_on_x, add_on_y,
                              op,
                              feather,
                              feather_radius_x,
                              feather_radius_y);
  g_object_unref (add_on);
}
