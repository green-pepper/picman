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

#include <gegl.h>

#include "core-types.h"

#include "picmandrawable.h"
#include "picmandrawable-equalize.h"
#include "picmandrawable-histogram.h"
#include "picmandrawable-operation.h"
#include "picmanhistogram.h"

#include "picman-intl.h"


void
picman_drawable_equalize (PicmanDrawable *drawable,
                        gboolean      mask_only)
{
  PicmanHistogram *hist;
  GeglNode      *equalize;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));

  hist = picman_histogram_new ();
  picman_drawable_calculate_histogram (drawable, hist);

  equalize = gegl_node_new_child (NULL,
                                  "operation", "picman:equalize",
                                  "histogram", hist,
                                  NULL);

  picman_drawable_apply_operation (drawable, NULL,
                                 C_("undo-type", "Equalize"),
                                 equalize);

  g_object_unref (equalize);

  picman_histogram_unref (hist);
}
