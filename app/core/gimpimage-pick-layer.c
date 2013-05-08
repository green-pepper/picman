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

#include "picmangrouplayer.h"
#include "picmanimage.h"
#include "picmanimage-pick-layer.h"
#include "picmanpickable.h"

#include "text/picmantextlayer.h"


PicmanLayer *
picman_image_pick_layer (const PicmanImage *image,
                       gint             x,
                       gint             y)
{
  GList *all_layers;
  GList *list;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  all_layers = picman_image_get_layer_list (image);

  for (list = all_layers; list; list = g_list_next (list))
    {
      PicmanLayer *layer = list->data;
      gint       off_x, off_y;

      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      if (picman_pickable_get_opacity_at (PICMAN_PICKABLE (layer),
                                        x - off_x, y - off_y) > 0.25)
        {
          g_list_free (all_layers);

          return layer;
        }
    }

  g_list_free (all_layers);

  return NULL;
}

PicmanLayer *
picman_image_pick_layer_by_bounds (const PicmanImage *image,
                                 gint             x,
                                 gint             y)
{
  GList *all_layers;
  GList *list;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  all_layers = picman_image_get_layer_list (image);

  for (list = all_layers; list; list = g_list_next (list))
    {
      PicmanLayer *layer = list->data;

      if (picman_item_is_visible (PICMAN_ITEM (layer)))
        {
          gint off_x, off_y;
          gint width, height;

          picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);
          width  = picman_item_get_width  (PICMAN_ITEM (layer));
          height = picman_item_get_height (PICMAN_ITEM (layer));

          if (x >= off_x        &&
              y >= off_y        &&
              x < off_x + width &&
              y < off_y + height)
            {
              g_list_free (all_layers);

              return layer;
            }
        }
    }

  g_list_free (all_layers);

  return NULL;
}

PicmanTextLayer *
picman_image_pick_text_layer (const PicmanImage *image,
                            gint             x,
                            gint             y)
{
  GList *all_layers;
  GList *list;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  all_layers = picman_image_get_layer_list (image);

  for (list = all_layers; list; list = g_list_next (list))
    {
      PicmanLayer *layer = list->data;
      gint       off_x, off_y;

      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      if (PICMAN_IS_TEXT_LAYER (layer) &&
          x >= off_x &&
          y >= off_y &&
          x <  off_x + picman_item_get_width  (PICMAN_ITEM (layer)) &&
          y <  off_y + picman_item_get_height (PICMAN_ITEM (layer)) &&
          picman_item_is_visible (PICMAN_ITEM (layer)))
        {
          g_list_free (all_layers);

          return PICMAN_TEXT_LAYER (layer);
        }
      else if (picman_pickable_get_opacity_at (PICMAN_PICKABLE (layer),
                                             x - off_x, y - off_y) > 0.25)
        {
          /*  a normal layer covers any possible text layers below,
           *  bail out
           */

          break;
        }
    }

  g_list_free (all_layers);

  return NULL;
}
