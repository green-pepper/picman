/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanhistogram module Copyright (C) 1999 Jay Cox <jaycox@picman.org>
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

#include "core-types.h"

#include "gegl/picman-gegl-nodes.h"

#include "picmanchannel.h"
#include "picmandrawable-histogram.h"
#include "picmanhistogram.h"
#include "picmanimage.h"


void
picman_drawable_calculate_histogram (PicmanDrawable  *drawable,
                                   PicmanHistogram *histogram)
{
  PicmanImage   *image;
  PicmanChannel *mask;
  gint         x, y, width, height;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (histogram != NULL);

  if (! picman_item_mask_intersect (PICMAN_ITEM (drawable), &x, &y, &width, &height))
    return;

  image = picman_item_get_image (PICMAN_ITEM (drawable));
  mask  = picman_image_get_mask (image);

  if (FALSE)
    {
      GeglNode      *node = gegl_node_new ();
      GeglNode      *buffer_source;
      GeglNode      *histogram_sink;
      GeglProcessor *processor;

      buffer_source =
        picman_gegl_add_buffer_source (node,
                                     picman_drawable_get_buffer (drawable),
                                     0, 0);

      histogram_sink =
        gegl_node_new_child (node,
                             "operation", "picman:histogram-sink",
                             "histogram", histogram,
                             NULL);

      gegl_node_connect_to (buffer_source,  "output",
                            histogram_sink, "input");

      if (! picman_channel_is_empty (mask))
        {
          GeglNode *mask_source;
          gint      off_x, off_y;

          g_printerr ("adding mask aux\n");

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

          mask_source =
            picman_gegl_add_buffer_source (node,
                                         picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)),
                                         -off_x, -off_y);

          gegl_node_connect_to (mask_source,    "output",
                                histogram_sink, "aux");
        }

      processor = gegl_node_new_processor (histogram_sink,
                                           GEGL_RECTANGLE (x, y, width, height));

      while (gegl_processor_work (processor, NULL));

      g_object_unref (processor);
      g_object_unref (node);
    }
  else
    {
      if (! picman_channel_is_empty (mask))
        {
          gint off_x, off_y;

          picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

          picman_histogram_calculate (histogram,
                                    picman_drawable_get_buffer (drawable),
                                    GEGL_RECTANGLE (x, y, width, height),
                                    picman_drawable_get_buffer (PICMAN_DRAWABLE (mask)),
                                    GEGL_RECTANGLE (x + off_x, y + off_y,
                                                    width, height));
        }
      else
        {
          picman_histogram_calculate (histogram,
                                    picman_drawable_get_buffer (drawable),
                                    GEGL_RECTANGLE (x, y, width, height),
                                    NULL, NULL);
        }
    }
}
