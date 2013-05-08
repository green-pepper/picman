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

#include "core-types.h"

#include "gegl/picman-gegl-mask-combine.h"

#include "picmanchannel.h"
#include "picmanchannel-combine.h"


void
picman_channel_combine_rect (PicmanChannel    *mask,
                           PicmanChannelOps  op,
                           gint            x,
                           gint            y,
                           gint            w,
                           gint            h)
{
  GeglBuffer *buffer;

  g_return_if_fail (PICMAN_IS_CHANNEL (mask));

  buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

  if (! picman_gegl_mask_combine_rect (buffer, op, x, y, w, h))
    return;

  picman_rectangle_intersect (x, y, w, h,
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (mask)),
                            picman_item_get_height (PICMAN_ITEM (mask)),
                            &x, &y, &w, &h);

  /*  Determine new boundary  */
  if (mask->bounds_known && (op == PICMAN_CHANNEL_OP_ADD) && ! mask->empty)
    {
      if (x < mask->x1)
        mask->x1 = x;
      if (y < mask->y1)
        mask->y1 = y;
      if ((x + w) > mask->x2)
        mask->x2 = (x + w);
      if ((y + h) > mask->y2)
        mask->y2 = (y + h);
    }
  else if (op == PICMAN_CHANNEL_OP_REPLACE || mask->empty)
    {
      mask->empty = FALSE;
      mask->x1    = x;
      mask->y1    = y;
      mask->x2    = x + w;
      mask->y2    = y + h;
    }
  else
    {
      mask->bounds_known = FALSE;
    }

  mask->x1 = CLAMP (mask->x1, 0, picman_item_get_width  (PICMAN_ITEM (mask)));
  mask->y1 = CLAMP (mask->y1, 0, picman_item_get_height (PICMAN_ITEM (mask)));
  mask->x2 = CLAMP (mask->x2, 0, picman_item_get_width  (PICMAN_ITEM (mask)));
  mask->y2 = CLAMP (mask->y2, 0, picman_item_get_height (PICMAN_ITEM (mask)));

  picman_drawable_update (PICMAN_DRAWABLE (mask), x, y, w, h);
}

/**
 * picman_channel_combine_ellipse:
 * @mask:      the channel with which to combine the ellipse
 * @op:        whether to replace, add to, or subtract from the current
 *             contents
 * @x:         x coordinate of upper left corner of ellipse
 * @y:         y coordinate of upper left corner of ellipse
 * @w:         width of ellipse bounding box
 * @h:         height of ellipse bounding box
 * @antialias: if %TRUE, antialias the ellipse
 *
 * Mainly used for elliptical selections.  If @op is
 * %PICMAN_CHANNEL_OP_REPLACE or %PICMAN_CHANNEL_OP_ADD, sets pixels
 * within the ellipse to 255.  If @op is %PICMAN_CHANNEL_OP_SUBTRACT,
 * sets pixels within to zero.  If @antialias is %TRUE, pixels that
 * impinge on the edge of the ellipse are set to intermediate values,
 * depending on how much they overlap.
 **/
void
picman_channel_combine_ellipse (PicmanChannel    *mask,
                              PicmanChannelOps  op,
                              gint            x,
                              gint            y,
                              gint            w,
                              gint            h,
                              gboolean        antialias)
{
  picman_channel_combine_ellipse_rect (mask, op, x, y, w, h,
                                     w / 2.0, h / 2.0, antialias);
}

/**
 * picman_channel_combine_ellipse_rect:
 * @mask:      the channel with which to combine the elliptic rect
 * @op:        whether to replace, add to, or subtract from the current
 *             contents
 * @x:         x coordinate of upper left corner of bounding rect
 * @y:         y coordinate of upper left corner of bounding rect
 * @w:         width of bounding rect
 * @h:         height of bounding rect
 * @a:         elliptic a-constant applied to corners
 * @b:         elliptic b-constant applied to corners
 * @antialias: if %TRUE, antialias the elliptic corners
 *
 * Used for rounded cornered rectangles and ellipses.  If @op is
 * %PICMAN_CHANNEL_OP_REPLACE or %PICMAN_CHANNEL_OP_ADD, sets pixels
 * within the ellipse to 255.  If @op is %PICMAN_CHANNEL_OP_SUBTRACT,
 * sets pixels within to zero.  If @antialias is %TRUE, pixels that
 * impinge on the edge of the ellipse are set to intermediate values,
 * depending on how much they overlap.
 **/
void
picman_channel_combine_ellipse_rect (PicmanChannel    *mask,
                                   PicmanChannelOps  op,
                                   gint            x,
                                   gint            y,
                                   gint            w,
                                   gint            h,
                                   gdouble         a,
                                   gdouble         b,
                                   gboolean        antialias)
{
  GeglBuffer *buffer;

  g_return_if_fail (PICMAN_IS_CHANNEL (mask));
  g_return_if_fail (a >= 0.0 && b >= 0.0);
  g_return_if_fail (op != PICMAN_CHANNEL_OP_INTERSECT);

  buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

  if (! picman_gegl_mask_combine_ellipse_rect (buffer, op, x, y, w, h,
                                             a, b, antialias))
    return;

  picman_rectangle_intersect (x, y, w, h,
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (mask)),
                            picman_item_get_height (PICMAN_ITEM (mask)),
                            &x, &y, &w, &h);

  /*  determine new boundary  */
  if (mask->bounds_known && (op == PICMAN_CHANNEL_OP_ADD) && ! mask->empty)
    {
      if (x < mask->x1) mask->x1 = x;
      if (y < mask->y1) mask->y1 = y;

      if ((x + w) > mask->x2) mask->x2 = (x + w);
      if ((y + h) > mask->y2) mask->y2 = (y + h);
    }
  else if (op == PICMAN_CHANNEL_OP_REPLACE || mask->empty)
    {
      mask->empty = FALSE;
      mask->x1    = x;
      mask->y1    = y;
      mask->x2    = x + w;
      mask->y2    = y + h;
    }
  else
    {
      mask->bounds_known = FALSE;
    }

  picman_drawable_update (PICMAN_DRAWABLE (mask), x, y, w, h);
}

void
picman_channel_combine_mask (PicmanChannel    *mask,
                           PicmanChannel    *add_on,
                           PicmanChannelOps  op,
                           gint            off_x,
                           gint            off_y)
{
  GeglBuffer *add_on_buffer;

  g_return_if_fail (PICMAN_IS_CHANNEL (mask));
  g_return_if_fail (PICMAN_IS_CHANNEL (add_on));

  add_on_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (add_on));

  picman_channel_combine_buffer (mask, add_on_buffer,
                               op, off_x, off_y);
}

void
picman_channel_combine_buffer (PicmanChannel    *mask,
                             GeglBuffer     *add_on_buffer,
                             PicmanChannelOps  op,
                             gint            off_x,
                             gint            off_y)
{
  GeglBuffer *buffer;
  gint        x, y, w, h;

  g_return_if_fail (PICMAN_IS_CHANNEL (mask));
  g_return_if_fail (GEGL_IS_BUFFER (add_on_buffer));

  buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

  if (! picman_gegl_mask_combine_buffer (buffer, add_on_buffer,
                                       op, off_x, off_y))
    return;

  picman_rectangle_intersect (off_x, off_y,
                            gegl_buffer_get_width  (add_on_buffer),
                            gegl_buffer_get_height (add_on_buffer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (mask)),
                            picman_item_get_height (PICMAN_ITEM (mask)),
                            &x, &y, &w, &h);

  mask->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (mask), x, y, w, h);
}
