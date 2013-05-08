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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "picman.h"
#include "picmanchannel.h"
#include "picmanimage.h"
#include "picmanimage-private.h"
#include "picmanimage-quick-mask.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanlayer-floating-sel.h"
#include "picmanselection.h"

#include "picman-intl.h"


#define CHANNEL_WAS_ACTIVE (0x2)


/*  public functions  */

void
picman_image_set_quick_mask_state (PicmanImage *image,
                                 gboolean   active)
{
  PicmanImagePrivate *private;
  PicmanChannel      *selection;
  PicmanChannel      *mask;
  gboolean          channel_was_active;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  if (active == picman_image_get_quick_mask_state (image))
    return;

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  /*  Keep track of the state so that we can make the right drawable
   *  active again when deactiviting quick mask (see bug #134371).
   */
  if (private->quick_mask_state)
    channel_was_active = (private->quick_mask_state & CHANNEL_WAS_ACTIVE) != 0;
  else
    channel_was_active = picman_image_get_active_channel (image) != NULL;

  /*  Set private->quick_mask_state early so we can return early when
   *  being called recursively.
   */
  private->quick_mask_state = (active
                               ? TRUE | (channel_was_active ?
                                         CHANNEL_WAS_ACTIVE : 0)
                               : FALSE);

  selection = picman_image_get_mask (image);
  mask      = picman_image_get_quick_mask (image);

  if (active)
    {
      if (! mask)
        {
          picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK,
                                       C_("undo-type", "Enable Quick Mask"));

          if (picman_channel_is_empty (selection))
            {
              /* if no selection */

              PicmanLayer *floating_sel = picman_image_get_floating_selection (image);

              if (floating_sel)
                floating_sel_to_layer (floating_sel, NULL);

              mask = picman_channel_new (image,
                                       picman_image_get_width  (image),
                                       picman_image_get_height (image),
                                       PICMAN_IMAGE_QUICK_MASK_NAME,
                                       &private->quick_mask_color);

              /* Clear the mask */
              picman_channel_clear (mask, NULL, FALSE);
            }
          else
            {
              /* if selection */

              mask = PICMAN_CHANNEL (picman_item_duplicate (PICMAN_ITEM (selection),
                                                        PICMAN_TYPE_CHANNEL));

              /* Clear the selection */
              picman_channel_clear (selection, NULL, TRUE);

              picman_channel_set_color (mask, &private->quick_mask_color, FALSE);
              picman_item_rename (PICMAN_ITEM (mask), PICMAN_IMAGE_QUICK_MASK_NAME,
                                NULL);
            }

          if (private->quick_mask_inverted)
            picman_channel_invert (mask, FALSE);

          picman_image_add_channel (image, mask, NULL, 0, TRUE);

          picman_image_undo_group_end (image);
        }
    }
  else
    {
      if (mask)
        {
          PicmanLayer *floating_sel = picman_image_get_floating_selection (image);

          picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_QUICK_MASK,
                                       C_("undo-type", "Disable Quick Mask"));

          if (private->quick_mask_inverted)
            picman_channel_invert (mask, TRUE);

          if (floating_sel &&
              picman_layer_get_floating_sel_drawable (floating_sel) == PICMAN_DRAWABLE (mask))
            floating_sel_anchor (floating_sel);

          picman_selection_load (PICMAN_SELECTION (picman_image_get_mask (image)),
                               mask);
          picman_image_remove_channel (image, mask, TRUE, NULL);

          if (! channel_was_active)
            picman_image_unset_active_channel (image);

          picman_image_undo_group_end (image);
        }
    }

  picman_image_quick_mask_changed (image);
}

gboolean
picman_image_get_quick_mask_state (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return PICMAN_IMAGE_GET_PRIVATE (image)->quick_mask_state;
}

void
picman_image_set_quick_mask_color (PicmanImage     *image,
                                 const PicmanRGB *color)
{
  PicmanChannel *quick_mask;

  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (color != NULL);

  PICMAN_IMAGE_GET_PRIVATE (image)->quick_mask_color = *color;

  quick_mask = picman_image_get_quick_mask (image);
  if (quick_mask)
    picman_channel_set_color (quick_mask, color, TRUE);
}

void
picman_image_get_quick_mask_color (const PicmanImage *image,
                                 PicmanRGB         *color)
{
  g_return_if_fail (PICMAN_IS_IMAGE (image));
  g_return_if_fail (color != NULL);

  *color = PICMAN_IMAGE_GET_PRIVATE (image)->quick_mask_color;
}

PicmanChannel *
picman_image_get_quick_mask (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_get_channel_by_name (image, PICMAN_IMAGE_QUICK_MASK_NAME);
}

void
picman_image_quick_mask_invert (PicmanImage *image)
{
  PicmanImagePrivate *private;

  g_return_if_fail (PICMAN_IS_IMAGE (image));

  private = PICMAN_IMAGE_GET_PRIVATE (image);

  if (private->quick_mask_state)
    {
      PicmanChannel *quick_mask = picman_image_get_quick_mask (image);

      if (quick_mask)
        picman_channel_invert (quick_mask, TRUE);
    }

  private->quick_mask_inverted = ! private->quick_mask_inverted;
}

gboolean
picman_image_get_quick_mask_inverted (const PicmanImage *image)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), FALSE);

  return PICMAN_IMAGE_GET_PRIVATE (image)->quick_mask_inverted;
}
