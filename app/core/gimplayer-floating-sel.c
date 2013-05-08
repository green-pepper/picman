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

#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "picmanboundary.h"
#include "picmandrawable-filter.h"
#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"
#include "picmanlayermask.h"

#include "picman-intl.h"


/* public functions  */

void
floating_sel_attach (PicmanLayer    *layer,
                     PicmanDrawable *drawable)
{
  PicmanImage *image;
  PicmanLayer *floating_sel;

  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (drawable != PICMAN_DRAWABLE (layer));
  g_return_if_fail (picman_item_get_image (PICMAN_ITEM (layer)) ==
                    picman_item_get_image (PICMAN_ITEM (drawable)));

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  floating_sel = picman_image_get_floating_selection (image);

  /*  If there is already a floating selection, anchor it  */
  if (floating_sel)
    {
      floating_sel_anchor (floating_sel);

      /*  if we were pasting to the old floating selection, paste now
       *  to the drawable
       */
      if (drawable == (PicmanDrawable *) floating_sel)
        drawable = picman_image_get_active_drawable (image);
    }

  picman_layer_set_lock_alpha (layer, TRUE, FALSE);

  picman_layer_set_floating_sel_drawable (layer, drawable);

  picman_image_add_layer (image, layer, NULL, 0, TRUE);
}

void
floating_sel_anchor (PicmanLayer *layer)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanFilter   *filter = NULL;
  gint          off_x, off_y;
  gint          dr_off_x, dr_off_y;

  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_layer_is_floating_sel (layer));

  image = picman_item_get_image (PICMAN_ITEM (layer));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_FS_ANCHOR,
                               C_("undo-type", "Anchor Floating Selection"));

  drawable = picman_layer_get_floating_sel_drawable (layer);

  picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);
  picman_item_get_offset (PICMAN_ITEM (drawable), &dr_off_x, &dr_off_y);

  if (picman_item_get_visible (PICMAN_ITEM (layer)) &&
      picman_rectangle_intersect (off_x, off_y,
                                picman_item_get_width  (PICMAN_ITEM (layer)),
                                picman_item_get_height (PICMAN_ITEM (layer)),
                                dr_off_x, dr_off_y,
                                picman_item_get_width  (PICMAN_ITEM (drawable)),
                                picman_item_get_height (PICMAN_ITEM (drawable)),
                                NULL, NULL, NULL, NULL))
    {
      filter = picman_drawable_get_floating_sel_filter (drawable);
      g_object_ref (filter);
    }

  /*  first remove the filter, then merge it, or we will get warnings
   *  about already connected nodes
   */
  picman_image_remove_layer (image, layer, TRUE, NULL);

  if (filter)
    {
      picman_drawable_merge_filter (drawable, filter, NULL, NULL);
      g_object_unref (filter);
    }

  picman_image_undo_group_end (image);

  /*  invalidate the boundaries  */
  picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (picman_image_get_mask (image)));
}

gboolean
floating_sel_to_layer (PicmanLayer  *layer,
                       GError    **error)
{
  PicmanItem  *item;
  PicmanImage *image;

  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (picman_layer_is_floating_sel (layer), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  item  = PICMAN_ITEM (layer);
  image = picman_item_get_image (item);

  /*  Check if the floating layer belongs to a channel  */
  if (PICMAN_IS_CHANNEL (picman_layer_get_floating_sel_drawable (layer)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot create a new layer from the floating "
			     "selection because it belongs to a layer mask "
			     "or channel."));
      return FALSE;
    }

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_FS_TO_LAYER,
                               C_("undo-type", "Floating Selection to Layer"));

  picman_image_undo_push_fs_to_layer (image, NULL, layer);

  picman_drawable_detach_floating_sel (picman_layer_get_floating_sel_drawable (layer));
  picman_layer_set_floating_sel_drawable (layer, NULL);

  picman_item_set_visible (item, TRUE, TRUE);
  picman_layer_set_lock_alpha (layer, FALSE, TRUE);

  picman_image_undo_group_end (image);

  /* When the floating selection is converted to/from a normal layer
   * it does something resembling a name change, so emit the
   * "name-changed" signal
   */
  picman_object_name_changed (PICMAN_OBJECT (layer));

  picman_drawable_update (PICMAN_DRAWABLE (layer),
                        0, 0,
                        picman_item_get_width  (item),
                        picman_item_get_height (item));

  return TRUE;
}

void
floating_sel_activate_drawable (PicmanLayer *layer)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;

  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_layer_is_floating_sel (layer));

  image = picman_item_get_image (PICMAN_ITEM (layer));

  drawable = picman_layer_get_floating_sel_drawable (layer);

  /*  set the underlying drawable to active  */
  if (PICMAN_IS_LAYER_MASK (drawable))
    {
      PicmanLayerMask *mask = PICMAN_LAYER_MASK (drawable);

      picman_image_set_active_layer (image, picman_layer_mask_get_layer (mask));
    }
  else if (PICMAN_IS_CHANNEL (drawable))
    {
      picman_image_set_active_channel (image, PICMAN_CHANNEL (drawable));
    }
  else
    {
      picman_image_set_active_layer (image, PICMAN_LAYER (drawable));
    }
}

const PicmanBoundSeg *
floating_sel_boundary (PicmanLayer *layer,
                       gint      *n_segs)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_layer_is_floating_sel (layer), NULL);
  g_return_val_if_fail (n_segs != NULL, NULL);

  if (layer->fs.boundary_known == FALSE)
    {
      gint width, height;
      gint off_x, off_y;

      width  = picman_item_get_width  (PICMAN_ITEM (layer));
      height = picman_item_get_height (PICMAN_ITEM (layer));
      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      if (layer->fs.segs)
        g_free (layer->fs.segs);

      if (picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
        {
          GeglBuffer *buffer;
          gint        i;

          /*  find the segments  */
          buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer));

          layer->fs.segs = picman_boundary_find (buffer, NULL,
                                               babl_format ("A float"),
                                               PICMAN_BOUNDARY_WITHIN_BOUNDS,
                                               0, 0, width, height,
                                               PICMAN_BOUNDARY_HALF_WAY,
                                               &layer->fs.num_segs);

          /*  offset the segments  */
          for (i = 0; i < layer->fs.num_segs; i++)
            {
              layer->fs.segs[i].x1 += off_x;
              layer->fs.segs[i].y1 += off_y;
              layer->fs.segs[i].x2 += off_x;
              layer->fs.segs[i].y2 += off_y;
            }
        }
      else
        {
          layer->fs.num_segs = 4;
          layer->fs.segs     = g_new0 (PicmanBoundSeg, 4);

          /* top */
          layer->fs.segs[0].x1 = off_x;
          layer->fs.segs[0].y1 = off_y;
          layer->fs.segs[0].x2 = off_x + width;
          layer->fs.segs[0].y2 = off_y;

          /* left */
          layer->fs.segs[1].x1 = off_x;
          layer->fs.segs[1].y1 = off_y;
          layer->fs.segs[1].x2 = off_x;
          layer->fs.segs[1].y2 = off_y + height;

          /* right */
          layer->fs.segs[2].x1 = off_x + width;
          layer->fs.segs[2].y1 = off_y;
          layer->fs.segs[2].x2 = off_x + width;
          layer->fs.segs[2].y2 = off_y + height;

          /* bottom */
          layer->fs.segs[3].x1 = off_x;
          layer->fs.segs[3].y1 = off_y + height;
          layer->fs.segs[3].x2 = off_x + width;
          layer->fs.segs[3].y2 = off_y + height;
        }

      layer->fs.boundary_known = TRUE;
    }

  *n_segs = layer->fs.num_segs;

  return layer->fs.segs;
}

void
floating_sel_invalidate (PicmanLayer *layer)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_layer_is_floating_sel (layer));

  /*  Invalidate the attached-to drawable's preview  */
  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (picman_layer_get_floating_sel_drawable (layer)));

  /*  Invalidate the boundary  */
  layer->fs.boundary_known = FALSE;
}
