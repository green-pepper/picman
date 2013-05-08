/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGrid
 * Copyright (C) 2003  Henrik Brix Andersen <brix@picman.org>
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

#include "libpicmanconfig/picmanconfig.h"

#include "core-types.h"

#include "picmangrid.h"
#include "picmanimage.h"
#include "picmanimage-grid.h"
#include "picmanimage-private.h"
#include "picmanimage-undo-push.h"

#include "picman-intl.h"


PicmanGrid *
picman_image_get_grid (PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return PICMAN_IMAGE_GET_PRIVATE (image)->grid;
}

void
picman_image_set_grid (PicmanImage *image,
                     PicmanGrid  *grid,
                     gboolean   push_undo)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (PICMAN_IS_GRID (grid));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (picman_config_is_equal_to (PICMAN_CONFIG (private->grid), PICMAN_CONFIG (grid)))
    return;

  if (push_undo)
    picman_image_undo_push_image_grid (image,
                                     C_("undo-type", "Grid"), private->grid);

  picman_config_sync (G_OBJECT (grid), G_OBJECT (private->grid), 0);
}
