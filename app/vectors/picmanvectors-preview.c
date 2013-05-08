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

#include "vectors-types.h"

#include "libpicmanmath/picmanmath.h"

#include "core/picmanimage.h"
#include "core/picmantempbuf.h"

#include "picmanstroke.h"
#include "picmanvectors.h"
#include "picmanvectors-preview.h"


/*  public functions  */

PicmanTempBuf *
picman_vectors_get_new_preview (PicmanViewable *viewable,
                              PicmanContext  *context,
                              gint          width,
                              gint          height)
{
  PicmanVectors *vectors;
  PicmanItem    *item;
  PicmanStroke  *cur_stroke;
  gdouble      xscale, yscale;
  guchar      *data;
  PicmanTempBuf *temp_buf;

  vectors = PICMAN_VECTORS (viewable);
  item    = PICMAN_ITEM (viewable);

  xscale = ((gdouble) width)  / picman_image_get_width  (picman_item_get_image (item));
  yscale = ((gdouble) height) / picman_image_get_height (picman_item_get_image (item));

  temp_buf = picman_temp_buf_new (width, height, babl_format ("Y' u8"));
  data = picman_temp_buf_get_data (temp_buf);
  memset (data, 255, width * height);

  for (cur_stroke = picman_vectors_stroke_get_next (vectors, NULL);
       cur_stroke;
       cur_stroke = picman_vectors_stroke_get_next (vectors, cur_stroke))
    {
      GArray   *coords;
      gboolean  closed;
      gint      i;

      coords = picman_stroke_interpolate (cur_stroke, 0.5, &closed);

      if (coords)
        {
          for (i = 0; i < coords->len; i++)
            {
              PicmanCoords point;
              gint       x, y;

              point = g_array_index (coords, PicmanCoords, i);

              x = ROUND (point.x * xscale);
              y = ROUND (point.y * yscale);

              if (x >= 0 && y >= 0 && x < width && y < height)
                data[y * width + x] = 0;
            }

          g_array_free (coords, TRUE);
        }
    }

  return temp_buf;
}
