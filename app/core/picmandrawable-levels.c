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

#include "operations/picmanlevelsconfig.h"

#include "picmandrawable.h"
#include "picmandrawable-histogram.h"
#include "picmandrawable-levels.h"
#include "picmandrawable-operation.h"
#include "picmanhistogram.h"
#include "picmanprogress.h"

#include "picman-intl.h"


/*  public functions  */

void
picman_drawable_levels_stretch (PicmanDrawable *drawable,
                              PicmanProgress *progress)
{
  PicmanLevelsConfig *config;
  PicmanHistogram    *histogram;
  GeglNode         *levels;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (! picman_item_mask_intersect (PICMAN_ITEM (drawable), NULL, NULL, NULL, NULL))
    return;

  config = g_object_new (PICMAN_TYPE_LEVELS_CONFIG, NULL);

  histogram = picman_histogram_new ();
  picman_drawable_calculate_histogram (drawable, histogram);

  picman_levels_config_stretch (config, histogram,
                              picman_drawable_is_rgb (drawable));

  picman_histogram_unref (histogram);

  levels = g_object_new (GEGL_TYPE_NODE,
                         "operation", "picman:levels",
                         NULL);

  gegl_node_set (levels,
                 "config", config,
                 NULL);

  picman_drawable_apply_operation (drawable, progress, _("Levels"),
                                 levels);

  g_object_unref (levels);
  g_object_unref (config);
}
