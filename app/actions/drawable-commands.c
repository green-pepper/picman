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
#include <gtk/gtk.h>

#include "libpicmanwidgets/picmanwidgets.h"

#include "actions-types.h"

#include "core/picman.h"
#include "core/picmandrawable-equalize.h"
#include "core/picmandrawable-levels.h"
#include "core/picmandrawable-operation.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanitem-linked.h"
#include "core/picmanitemundo.h"
#include "core/picmanlayermask.h"
#include "core/picmanprogress.h"

#include "dialogs/offset-dialog.h"

#include "actions.h"
#include "drawable-commands.h"

#include "picman-intl.h"


/*  public functions  */

void
drawable_equalize_cmd_callback (GtkAction *action,
                                gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  return_if_no_drawable (image, drawable, data);

  picman_drawable_equalize (drawable, TRUE);
  picman_image_flush (image);
}

void
drawable_invert_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanDisplay  *display;
  return_if_no_drawable (image, drawable, data);
  return_if_no_display (display, data);

  picman_drawable_apply_operation_by_name (drawable, PICMAN_PROGRESS (display),
                                         _("Invert"), "gegl:invert",
                                         NULL);
  picman_image_flush (image);
}

void
drawable_value_invert_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanDisplay  *display;
  return_if_no_drawable (image, drawable, data);
  return_if_no_display (display, data);

  picman_drawable_apply_operation_by_name (drawable, PICMAN_PROGRESS (display),
                                         _("Invert"), "gegl:value-invert",
                                         NULL);
  picman_image_flush (image);
}

void
drawable_levels_stretch_cmd_callback (GtkAction *action,
                                      gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanDisplay  *display;
  GtkWidget    *widget;
  return_if_no_drawable (image, drawable, data);
  return_if_no_display (display, data);
  return_if_no_widget (widget, data);

  if (! picman_drawable_is_rgb (drawable))
    {
      picman_message_literal (image->picman,
			    G_OBJECT (widget), PICMAN_MESSAGE_WARNING,
			    _("White Balance operates only on RGB color layers."));
      return;
    }

  picman_drawable_levels_stretch (drawable, PICMAN_PROGRESS (display));
  picman_image_flush (image);
}

void
drawable_offset_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  GtkWidget    *widget;
  GtkWidget    *dialog;
  return_if_no_drawable (image, drawable, data);
  return_if_no_widget (widget, data);

  dialog = offset_dialog_new (drawable, action_data_get_context (data),
                              widget);
  gtk_widget_show (dialog);
}


void
drawable_linked_cmd_callback (GtkAction *action,
                              gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  gboolean      linked;
  return_if_no_drawable (image, drawable, data);

  linked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (PICMAN_IS_LAYER_MASK (drawable))
    drawable =
      PICMAN_DRAWABLE (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));

  if (linked != picman_item_get_linked (PICMAN_ITEM (drawable)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_LINKED);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (drawable))
        push_undo = FALSE;

      picman_item_set_linked (PICMAN_ITEM (drawable), linked, push_undo);
      picman_image_flush (image);
    }
}

void
drawable_visible_cmd_callback (GtkAction *action,
                               gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  gboolean      visible;
  return_if_no_drawable (image, drawable, data);

  visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (PICMAN_IS_LAYER_MASK (drawable))
    drawable =
      PICMAN_DRAWABLE (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));

  if (visible != picman_item_get_visible (PICMAN_ITEM (drawable)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_VISIBILITY);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (drawable))
        push_undo = FALSE;

      picman_item_set_visible (PICMAN_ITEM (drawable), visible, push_undo);
      picman_image_flush (image);
    }
}

void
drawable_lock_content_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  gboolean      locked;
  return_if_no_drawable (image, drawable, data);

  locked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (PICMAN_IS_LAYER_MASK (drawable))
    drawable =
      PICMAN_DRAWABLE (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));

  if (locked != picman_item_get_lock_content (PICMAN_ITEM (drawable)))
    {
#if 0
      PicmanUndo *undo;
#endif
      gboolean  push_undo = TRUE;

#if 0
      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_VISIBILITY);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (drawable))
        push_undo = FALSE;
#endif

      picman_item_set_lock_content (PICMAN_ITEM (drawable), locked, push_undo);
      picman_image_flush (image);
    }
}

void
drawable_lock_position_cmd_callback (GtkAction *action,
                                    gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  gboolean      locked;
  return_if_no_drawable (image, drawable, data);

  locked = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));

  if (PICMAN_IS_LAYER_MASK (drawable))
    drawable =
      PICMAN_DRAWABLE (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (drawable)));

  if (locked != picman_item_get_lock_position (PICMAN_ITEM (drawable)))
    {
      PicmanUndo *undo;
      gboolean  push_undo = TRUE;

      undo = picman_image_undo_can_compress (image, PICMAN_TYPE_ITEM_UNDO,
                                           PICMAN_UNDO_ITEM_LOCK_POSITION);

      if (undo && PICMAN_ITEM_UNDO (undo)->item == PICMAN_ITEM (drawable))
        push_undo = FALSE;

      picman_item_set_lock_position (PICMAN_ITEM (drawable), locked, push_undo);
      picman_image_flush (image);
    }
}

void
drawable_flip_cmd_callback (GtkAction *action,
                            gint       value,
                            gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanItem     *item;
  PicmanContext  *context;
  gint          off_x, off_y;
  gdouble       axis = 0.0;
  return_if_no_drawable (image, drawable, data);
  return_if_no_context (context, data);

  item = PICMAN_ITEM (drawable);

  picman_item_get_offset (item, &off_x, &off_y);

  switch ((PicmanOrientationType) value)
    {
    case PICMAN_ORIENTATION_HORIZONTAL:
      axis = ((gdouble) off_x + (gdouble) picman_item_get_width (item) / 2.0);
      break;

    case PICMAN_ORIENTATION_VERTICAL:
      axis = ((gdouble) off_y + (gdouble) picman_item_get_height (item) / 2.0);
      break;

    default:
      break;
    }

  if (picman_item_get_linked (item))
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                                 PICMAN_ITEM_GET_CLASS (item)->flip_desc);

  picman_item_flip (item, context,
                  (PicmanOrientationType) value, axis, FALSE);

  if (picman_item_get_linked (item))
    {
      picman_item_linked_flip (item, context,
                             (PicmanOrientationType) value, axis, FALSE);
      picman_image_undo_group_end (image);
    }

  picman_image_flush (image);
}

void
drawable_rotate_cmd_callback (GtkAction *action,
                              gint       value,
                              gpointer   data)
{
  PicmanImage    *image;
  PicmanDrawable *drawable;
  PicmanContext  *context;
  PicmanItem     *item;
  gint          off_x, off_y;
  gdouble       center_x, center_y;
  gboolean      clip_result = FALSE;
  return_if_no_drawable (image, drawable, data);
  return_if_no_context (context, data);

  item = PICMAN_ITEM (drawable);

  picman_item_get_offset (item, &off_x, &off_y);

  center_x = ((gdouble) off_x + (gdouble) picman_item_get_width  (item) / 2.0);
  center_y = ((gdouble) off_y + (gdouble) picman_item_get_height (item) / 2.0);

  if (picman_item_get_linked (item))
    picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_TRANSFORM,
                                 PICMAN_ITEM_GET_CLASS (item)->rotate_desc);

  if (PICMAN_IS_CHANNEL (item))
    clip_result = TRUE;

  picman_item_rotate (item, context, (PicmanRotationType) value,
                    center_x, center_y, clip_result);

  if (picman_item_get_linked (item))
    {
      picman_item_linked_rotate (item, context, (PicmanRotationType) value,
                               center_x, center_y, FALSE);
      picman_image_undo_group_end (image);
    }

  picman_image_flush (image);
}
