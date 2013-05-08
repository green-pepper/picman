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

#if 0

#include "config.h"

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "base/siox.h"
#include "base/tile-manager.h"

#include "gegl/picman-gegl-utils.h"

#include "picmanchannel.h"
#include "picmandrawable.h"
#include "picmandrawable-foreground-extract.h"
#include "picmanimage.h"
#include "picmanprogress.h"

#include "picman-intl.h"


/*  public functions  */

void
picman_drawable_foreground_extract (PicmanDrawable              *drawable,
                                  PicmanForegroundExtractMode  mode,
                                  PicmanDrawable              *mask,
                                  PicmanProgress              *progress)
{
  SioxState    *state;
  const gdouble sensitivity[3] = { SIOX_DEFAULT_SENSITIVITY_L,
                                   SIOX_DEFAULT_SENSITIVITY_A,
                                   SIOX_DEFAULT_SENSITIVITY_B };

  g_return_if_fail (PICMAN_IS_DRAWABLE (mask));
  g_return_if_fail (mode == PICMAN_FOREGROUND_EXTRACT_SIOX);

  state =
    picman_drawable_foreground_extract_siox_init (drawable,
                                                0, 0,
                                                picman_item_get_width  (PICMAN_ITEM (mask)),
                                                picman_item_get_height (PICMAN_ITEM (mask)));

  if (state)
    {
      picman_drawable_foreground_extract_siox (mask, state,
                                             SIOX_REFINEMENT_RECALCULATE,
                                             SIOX_DEFAULT_SMOOTHNESS,
                                             sensitivity,
                                             FALSE,
                                             progress);

      picman_drawable_foreground_extract_siox_done (state);
    }
}

SioxState *
picman_drawable_foreground_extract_siox_init (PicmanDrawable *drawable,
                                            gint          x,
                                            gint          y,
                                            gint          width,
                                            gint          height)
{
  GeglBuffer   *buffer;
  const guchar *colormap = NULL;
  gboolean      intersect;
  gint          offset_x;
  gint          offset_y;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);

  if (picman_drawable_is_indexed (drawable))
    colormap = picman_drawable_get_colormap (drawable);

  picman_item_get_offset (PICMAN_ITEM (drawable), &offset_x, &offset_y);

  intersect = picman_rectangle_intersect (offset_x, offset_y,
                                        picman_item_get_width  (PICMAN_ITEM (drawable)),
                                        picman_item_get_height (PICMAN_ITEM (drawable)),
                                        x, y, width, height,
                                        &x, &y, &width, &height);


  /* FIXME:
   * Clear the mask outside the rectangle that we are working on?
   */

  if (! intersect)
    return NULL;

  buffer = picman_drawable_get_buffer (drawable);

  return siox_init (picman_gegl_buffer_get_tiles (buffer), colormap,
                    offset_x, offset_y,
                    x, y, width, height);
}

void
picman_drawable_foreground_extract_siox (PicmanDrawable       *mask,
                                       SioxState          *state,
                                       SioxRefinementType  refinement,
                                       gint                smoothness,
                                       const gdouble       sensitivity[3],
                                       gboolean            multiblob,
                                       PicmanProgress       *progress)
{
  GeglBuffer *buffer;
  gint        x1, y1;
  gint        x2, y2;

  g_return_if_fail (PICMAN_IS_DRAWABLE (mask));
  g_return_if_fail (babl_format_get_bytes_per_pixel (picman_drawable_get_format (mask)) == 1);

  g_return_if_fail (state != NULL);

  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (progress)
    picman_progress_start (progress, _("Foreground Extraction"), FALSE);

  if (PICMAN_IS_CHANNEL (mask))
    {
      picman_channel_bounds (PICMAN_CHANNEL (mask), &x1, &y1, &x2, &y2);
    }
  else
    {
      x1 = 0;
      y1 = 0;
      x2 = picman_item_get_width  (PICMAN_ITEM (mask));
      y2 = picman_item_get_height (PICMAN_ITEM (mask));
    }

  buffer = picman_drawable_get_buffer (mask);

  siox_foreground_extract (state, refinement,
                           picman_gegl_buffer_get_tiles (buffer),
                           x1, y1, x2, y2,
                           smoothness, sensitivity, multiblob,
                           (SioxProgressFunc) picman_progress_set_value,
                           progress);

  if (progress)
    picman_progress_end (progress);

  picman_drawable_update (mask, x1, y1, x2, y2);
}

void
picman_drawable_foreground_extract_siox_done (SioxState *state)
{
  g_return_if_fail (state != NULL);

  siox_done (state);
}

#endif
