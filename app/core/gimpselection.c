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

#include "core-types.h"

#include "gegl/picman-babl.h"
#include "gegl/picman-gegl-apply-operation.h"

#include "picman.h"
#include "picman-edit.h"
#include "picmancontext.h"
#include "picmandrawable-private.h"
#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanlayermask.h"
#include "picmanlayer-floating-sel.h"
#include "picmanpickable.h"
#include "picmanselection.h"

#include "picman-intl.h"


static gboolean   picman_selection_is_attached   (const PicmanItem      *item);
static PicmanItemTree * picman_selection_get_tree  (PicmanItem            *item);
static void       picman_selection_translate     (PicmanItem            *item,
                                                gint                 offset_x,
                                                gint                 offset_y,
                                                gboolean             push_undo);
static void       picman_selection_scale         (PicmanItem            *item,
                                                gint                 new_width,
                                                gint                 new_height,
                                                gint                 new_offset_x,
                                                gint                 new_offset_y,
                                                PicmanInterpolationType interp_type,
                                                PicmanProgress        *progress);
static void       picman_selection_resize        (PicmanItem            *item,
                                                PicmanContext         *context,
                                                gint                 new_width,
                                                gint                 new_height,
                                                gint                 offset_x,
                                                gint                 offset_y);
static void       picman_selection_flip          (PicmanItem            *item,
                                                PicmanContext         *context,
                                                PicmanOrientationType  flip_type,
                                                gdouble              axis,
                                                gboolean             clip_result);
static void       picman_selection_rotate        (PicmanItem            *item,
                                                PicmanContext         *context,
                                                PicmanRotationType     rotation_type,
                                                gdouble              center_x,
                                                gdouble              center_y,
                                                gboolean             clip_result);
static gboolean   picman_selection_stroke        (PicmanItem            *item,
                                                PicmanDrawable        *drawable,
                                                PicmanStrokeOptions   *stroke_options,
                                                gboolean             push_undo,
                                                PicmanProgress        *progress,
                                                GError             **error);
static void       picman_selection_convert_type  (PicmanDrawable        *drawable,
                                                PicmanImage           *dest_image,
                                                const Babl          *new_format,
                                                PicmanImageBaseType    new_base_type,
                                                PicmanPrecision        new_precision,
                                                gint                 layer_dither_type,
                                                gint                 mask_dither_type,
                                                gboolean             push_undo);
static void picman_selection_invalidate_boundary (PicmanDrawable        *drawable);

static gboolean   picman_selection_boundary      (PicmanChannel         *channel,
                                                const PicmanBoundSeg **segs_in,
                                                const PicmanBoundSeg **segs_out,
                                                gint                *num_segs_in,
                                                gint                *num_segs_out,
                                                gint                 x1,
                                                gint                 y1,
                                                gint                 x2,
                                                gint                 y2);
static gboolean   picman_selection_bounds        (PicmanChannel         *channel,
                                                gint                *x1,
                                                gint                *y1,
                                                gint                *x2,
                                                gint                *y2);
static gboolean   picman_selection_is_empty      (PicmanChannel         *channel);
static void       picman_selection_feather       (PicmanChannel         *channel,
                                                gdouble              radius_x,
                                                gdouble              radius_y,
                                                gboolean             push_undo);
static void       picman_selection_sharpen       (PicmanChannel         *channel,
                                                gboolean             push_undo);
static void       picman_selection_clear         (PicmanChannel         *channel,
                                                const gchar         *undo_desc,
                                                gboolean             push_undo);
static void       picman_selection_all           (PicmanChannel         *channel,
                                                gboolean             push_undo);
static void       picman_selection_invert        (PicmanChannel         *channel,
                                                gboolean             push_undo);
static void       picman_selection_border        (PicmanChannel         *channel,
                                                gint                 radius_x,
                                                gint                 radius_y,
                                                gboolean             feather,
                                                gboolean             edge_lock,
                                                gboolean             push_undo);
static void       picman_selection_grow          (PicmanChannel         *channel,
                                                gint                 radius_x,
                                                gint                 radius_y,
                                                gboolean             push_undo);
static void       picman_selection_shrink        (PicmanChannel         *channel,
                                                gint                 radius_x,
                                                gint                 radius_y,
                                                gboolean             edge_lock,
                                                gboolean             push_undo);


G_DEFINE_TYPE (PicmanSelection, picman_selection, PICMAN_TYPE_CHANNEL)

#define parent_class picman_selection_parent_class


static void
picman_selection_class_init (PicmanSelectionClass *klass)
{
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);
  PicmanItemClass     *item_class     = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class = PICMAN_DRAWABLE_CLASS (klass);
  PicmanChannelClass  *channel_class  = PICMAN_CHANNEL_CLASS (klass);

  viewable_class->default_stock_id    = "picman-selection";

  item_class->is_attached             = picman_selection_is_attached;
  item_class->get_tree                = picman_selection_get_tree;
  item_class->translate               = picman_selection_translate;
  item_class->scale                   = picman_selection_scale;
  item_class->resize                  = picman_selection_resize;
  item_class->flip                    = picman_selection_flip;
  item_class->rotate                  = picman_selection_rotate;
  item_class->stroke                  = picman_selection_stroke;
  item_class->default_name            = _("Selection Mask");
  item_class->translate_desc          = C_("undo-type", "Move Selection");
  item_class->stroke_desc             = C_("undo-type", "Stroke Selection");

  drawable_class->convert_type        = picman_selection_convert_type;
  drawable_class->invalidate_boundary = picman_selection_invalidate_boundary;

  channel_class->boundary             = picman_selection_boundary;
  channel_class->bounds               = picman_selection_bounds;
  channel_class->is_empty             = picman_selection_is_empty;
  channel_class->feather              = picman_selection_feather;
  channel_class->sharpen              = picman_selection_sharpen;
  channel_class->clear                = picman_selection_clear;
  channel_class->all                  = picman_selection_all;
  channel_class->invert               = picman_selection_invert;
  channel_class->border               = picman_selection_border;
  channel_class->grow                 = picman_selection_grow;
  channel_class->shrink               = picman_selection_shrink;

  channel_class->feather_desc         = C_("undo-type", "Feather Selection");
  channel_class->sharpen_desc         = C_("undo-type", "Sharpen Selection");
  channel_class->clear_desc           = C_("undo-type", "Select None");
  channel_class->all_desc             = C_("undo-type", "Select All");
  channel_class->invert_desc          = C_("undo-type", "Invert Selection");
  channel_class->border_desc          = C_("undo-type", "Border Selection");
  channel_class->grow_desc            = C_("undo-type", "Grow Selection");
  channel_class->shrink_desc          = C_("undo-type", "Shrink Selection");
}

static void
picman_selection_init (PicmanSelection *selection)
{
  selection->stroking_count = 0;
}

static gboolean
picman_selection_is_attached (const PicmanItem *item)
{
  return (PICMAN_IS_IMAGE (picman_item_get_image (item)) &&
          picman_image_get_mask (picman_item_get_image (item)) ==
          PICMAN_CHANNEL (item));
}

static PicmanItemTree *
picman_selection_get_tree (PicmanItem *item)
{
  return NULL;
}

static void
picman_selection_translate (PicmanItem *item,
                          gint      offset_x,
                          gint      offset_y,
                          gboolean  push_undo)
{
  PICMAN_ITEM_CLASS (parent_class)->translate (item, offset_x, offset_y,
                                             push_undo);
}

static void
picman_selection_scale (PicmanItem              *item,
                      gint                   new_width,
                      gint                   new_height,
                      gint                   new_offset_x,
                      gint                   new_offset_y,
                      PicmanInterpolationType  interp_type,
                      PicmanProgress          *progress)
{
  PICMAN_ITEM_CLASS (parent_class)->scale (item, new_width, new_height,
                                         new_offset_x, new_offset_y,
                                         interp_type, progress);

  picman_item_set_offset (item, 0, 0);
}

static void
picman_selection_resize (PicmanItem    *item,
                       PicmanContext *context,
                       gint         new_width,
                       gint         new_height,
                       gint         offset_x,
                       gint         offset_y)
{
  PICMAN_ITEM_CLASS (parent_class)->resize (item, context, new_width, new_height,
                                          offset_x, offset_y);

  picman_item_set_offset (item, 0, 0);
}

static void
picman_selection_flip (PicmanItem            *item,
                     PicmanContext         *context,
                     PicmanOrientationType  flip_type,
                     gdouble              axis,
                     gboolean             clip_result)
{
  PICMAN_ITEM_CLASS (parent_class)->flip (item, context, flip_type, axis, TRUE);
}

static void
picman_selection_rotate (PicmanItem         *item,
                       PicmanContext      *context,
                       PicmanRotationType  rotation_type,
                       gdouble           center_x,
                       gdouble           center_y,
                       gboolean          clip_result)
{
  PICMAN_ITEM_CLASS (parent_class)->rotate (item, context, rotation_type,
                                          center_x, center_y,
                                          clip_result);
}

static gboolean
picman_selection_stroke (PicmanItem           *item,
                       PicmanDrawable       *drawable,
                       PicmanStrokeOptions  *stroke_options,
                       gboolean            push_undo,
                       PicmanProgress       *progress,
                       GError            **error)
{
  PicmanSelection      *selection = PICMAN_SELECTION (item);
  const PicmanBoundSeg *dummy_in;
  const PicmanBoundSeg *dummy_out;
  gint                num_dummy_in;
  gint                num_dummy_out;
  gboolean            retval;

  if (! picman_channel_boundary (PICMAN_CHANNEL (selection),
                               &dummy_in, &dummy_out,
                               &num_dummy_in, &num_dummy_out,
                               0, 0, 0, 0))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("There is no selection to stroke."));
      return FALSE;
    }

  picman_selection_push_stroking (selection);

  retval = PICMAN_ITEM_CLASS (parent_class)->stroke (item, drawable,
                                                   stroke_options,
                                                   push_undo, progress, error);

  picman_selection_pop_stroking (selection);

  return retval;
}

static void
picman_selection_convert_type (PicmanDrawable      *drawable,
                             PicmanImage         *dest_image,
                             const Babl        *new_format,
                             PicmanImageBaseType  new_base_type,
                             PicmanPrecision      new_precision,
                             gint               layer_dither_type,
                             gint               mask_dither_type,
                             gboolean           push_undo)
{
  new_format = picman_babl_mask_format (new_precision);

  PICMAN_DRAWABLE_CLASS (parent_class)->convert_type (drawable, dest_image,
                                                    new_format,
                                                    new_base_type,
                                                    new_precision,
                                                    layer_dither_type,
                                                    mask_dither_type,
                                                    push_undo);
}

static void
picman_selection_invalidate_boundary (PicmanDrawable *drawable)
{
  PicmanImage *image = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanLayer *layer;

  /*  Turn the current selection off  */
  picman_image_selection_invalidate (image);

  PICMAN_DRAWABLE_CLASS (parent_class)->invalidate_boundary (drawable);

  /*  If there is a floating selection, update it's area...
   *  we need to do this since this selection mask can act as an additional
   *  mask in the composition of the floating selection
   */
  layer = picman_image_get_active_layer (image);

  if (layer && picman_layer_is_floating_sel (layer))
    picman_drawable_update (PICMAN_DRAWABLE (layer),
                          0, 0,
                          picman_item_get_width  (PICMAN_ITEM (layer)),
                          picman_item_get_height (PICMAN_ITEM (layer)));

#ifdef __GNUC__
#warning FIXME is this still needed?
#endif
#if 0
  /*  invalidate the preview  */
  drawable->private->preview_valid = FALSE;
#endif
}

static gboolean
picman_selection_boundary (PicmanChannel         *channel,
                         const PicmanBoundSeg **segs_in,
                         const PicmanBoundSeg **segs_out,
                         gint                *num_segs_in,
                         gint                *num_segs_out,
                         gint                 unused1,
                         gint                 unused2,
                         gint                 unused3,
                         gint                 unused4)
{
  PicmanImage    *image = picman_item_get_image (PICMAN_ITEM (channel));
  PicmanDrawable *drawable;
  PicmanLayer    *layer;

  if ((layer = picman_image_get_floating_selection (image)))
    {
      /*  If there is a floating selection, then
       *  we need to do some slightly different boundaries.
       *  Instead of inside and outside boundaries being defined
       *  by the extents of the layer, the inside boundary (the one
       *  that actually marches and is black/white) is the boundary of
       *  the floating selection.  The outside boundary (doesn't move,
       *  is black/gray) is defined as the normal selection mask
       */

      /*  Find the selection mask boundary  */
      PICMAN_CHANNEL_CLASS (parent_class)->boundary (channel,
                                                   segs_in, segs_out,
                                                   num_segs_in, num_segs_out,
                                                   0, 0, 0, 0);

      /*  Find the floating selection boundary  */
      *segs_in = floating_sel_boundary (layer, num_segs_in);

      return TRUE;
    }
  else if ((drawable = picman_image_get_active_drawable (image)) &&
           PICMAN_IS_CHANNEL (drawable))
    {
      /*  Otherwise, return the boundary...if a channel is active  */

      return PICMAN_CHANNEL_CLASS (parent_class)->boundary (channel,
                                                          segs_in, segs_out,
                                                          num_segs_in,
                                                          num_segs_out,
                                                          0, 0,
                                                          picman_image_get_width  (image),
                                                          picman_image_get_height (image));
    }
  else if ((layer = picman_image_get_active_layer (image)))
    {
      /*  If a layer is active, we return multiple boundaries based
       *  on the extents
       */

      gint x1, y1;
      gint x2, y2;
      gint offset_x;
      gint offset_y;

      picman_item_get_offset (PICMAN_ITEM (layer), &offset_x, &offset_y);

      x1 = CLAMP (offset_x, 0, picman_image_get_width  (image));
      y1 = CLAMP (offset_y, 0, picman_image_get_height (image));
      x2 = CLAMP (offset_x + picman_item_get_width (PICMAN_ITEM (layer)),
                  0, picman_image_get_width (image));
      y2 = CLAMP (offset_y + picman_item_get_height (PICMAN_ITEM (layer)),
                  0, picman_image_get_height (image));

      return PICMAN_CHANNEL_CLASS (parent_class)->boundary (channel,
                                                          segs_in, segs_out,
                                                          num_segs_in,
                                                          num_segs_out,
                                                          x1, y1, x2, y2);
    }

  *segs_in      = NULL;
  *segs_out     = NULL;
  *num_segs_in  = 0;
  *num_segs_out = 0;

  return FALSE;
}

static gboolean
picman_selection_bounds (PicmanChannel *channel,
                       gint        *x1,
                       gint        *y1,
                       gint        *x2,
                       gint        *y2)
{
  return PICMAN_CHANNEL_CLASS (parent_class)->bounds (channel, x1, y1, x2, y2);
}

static gboolean
picman_selection_is_empty (PicmanChannel *channel)
{
  PicmanSelection *selection = PICMAN_SELECTION (channel);

  /*  in order to allow stroking of selections, we need to pretend here
   *  that the selection mask is empty so that it doesn't mask the paint
   *  during the stroke operation.
   */
  if (selection->stroking_count > 0)
    return TRUE;

  return PICMAN_CHANNEL_CLASS (parent_class)->is_empty (channel);
}

static void
picman_selection_feather (PicmanChannel *channel,
                        gdouble      radius_x,
                        gdouble      radius_y,
                        gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->feather (channel, radius_x, radius_y,
                                              push_undo);
}

static void
picman_selection_sharpen (PicmanChannel *channel,
                        gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->sharpen (channel, push_undo);
}

static void
picman_selection_clear (PicmanChannel *channel,
                      const gchar *undo_desc,
                      gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->clear (channel, undo_desc, push_undo);
}

static void
picman_selection_all (PicmanChannel *channel,
                    gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->all (channel, push_undo);
}

static void
picman_selection_invert (PicmanChannel *channel,
                       gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->invert (channel, push_undo);
}

static void
picman_selection_border (PicmanChannel *channel,
                       gint         radius_x,
                       gint         radius_y,
                       gboolean     feather,
                       gboolean     edge_lock,
                       gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->border (channel,
                                             radius_x, radius_y,
                                             feather, edge_lock,
                                             push_undo);
}

static void
picman_selection_grow (PicmanChannel *channel,
                     gint         radius_x,
                     gint         radius_y,
                     gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->grow (channel,
					   radius_x, radius_y,
                                           push_undo);
}

static void
picman_selection_shrink (PicmanChannel *channel,
                       gint         radius_x,
                       gint         radius_y,
                       gboolean     edge_lock,
                       gboolean     push_undo)
{
  PICMAN_CHANNEL_CLASS (parent_class)->shrink (channel,
					     radius_x, radius_y, edge_lock,
					     push_undo);
}


/*  public functions  */

PicmanChannel *
picman_selection_new (PicmanImage *image,
                    gint       width,
                    gint       height)
{
  PicmanRGB      black = { 0.0, 0.0, 0.0, 0.5 };
  PicmanChannel *channel;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);

  channel = PICMAN_CHANNEL (picman_drawable_new (PICMAN_TYPE_SELECTION,
                                             image, NULL,
                                             0, 0, width, height,
                                             picman_image_get_mask_format (image)));

  picman_channel_set_color (channel, &black, FALSE);
  picman_channel_set_show_masked (channel, TRUE);

  channel->x2 = width;
  channel->y2 = height;

  return channel;
}

gint
picman_selection_push_stroking (PicmanSelection *selection)
{
  g_return_val_if_fail (PICMAN_IS_SELECTION (selection), 0);

  selection->stroking_count++;

  return selection->stroking_count;
}

gint
picman_selection_pop_stroking (PicmanSelection *selection)
{
  g_return_val_if_fail (PICMAN_IS_SELECTION (selection), 0);
  g_return_val_if_fail (selection->stroking_count > 0, 0);

  selection->stroking_count--;

  return selection->stroking_count;
}

void
picman_selection_load (PicmanSelection *selection,
                     PicmanChannel   *channel)
{
  gint width;
  gint height;

  g_return_if_fail (PICMAN_IS_SELECTION (selection));
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  width  = picman_item_get_width  (PICMAN_ITEM (selection));
  height = picman_item_get_height (PICMAN_ITEM (selection));

  g_return_if_fail (width  == picman_item_get_width  (PICMAN_ITEM (channel)));
  g_return_if_fail (height == picman_item_get_height (PICMAN_ITEM (channel)));

  picman_channel_push_undo (PICMAN_CHANNEL (selection),
                          C_("undo-type", "Channel to Selection"));

  /*  copy the channel to the mask  */
  gegl_buffer_copy (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                    NULL,
                    picman_drawable_get_buffer (PICMAN_DRAWABLE (selection)),
                    NULL);

  PICMAN_CHANNEL (selection)->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (selection),
                        0, 0, width, height);
}

PicmanChannel *
picman_selection_save (PicmanSelection *selection)
{
  PicmanImage   *image;
  PicmanChannel *new_channel;

  g_return_val_if_fail (PICMAN_IS_SELECTION (selection), NULL);

  image = picman_item_get_image (PICMAN_ITEM (selection));

  new_channel = PICMAN_CHANNEL (picman_item_duplicate (PICMAN_ITEM (selection),
                                                   PICMAN_TYPE_CHANNEL));

  /*  saved selections are not visible by default  */
  picman_item_set_visible (PICMAN_ITEM (new_channel), FALSE, FALSE);

  picman_image_add_channel (image, new_channel,
                          PICMAN_IMAGE_ACTIVE_PARENT, -1, TRUE);

  return new_channel;
}

GeglBuffer *
picman_selection_extract (PicmanSelection *selection,
                        PicmanPickable  *pickable,
                        PicmanContext   *context,
                        gboolean       cut_image,
                        gboolean       keep_indexed,
                        gboolean       add_alpha,
                        gint          *offset_x,
                        gint          *offset_y,
                        GError       **error)
{
  PicmanImage  *image;
  GeglBuffer *src_buffer;
  GeglBuffer *dest_buffer;
  const Babl *src_format;
  const Babl *dest_format;
  gint        x1, y1, x2, y2;
  gboolean    non_empty;
  gint        off_x, off_y;

  g_return_val_if_fail (PICMAN_IS_SELECTION (selection), NULL);
  g_return_val_if_fail (PICMAN_IS_PICKABLE (pickable), NULL);
  if (PICMAN_IS_ITEM (pickable))
    g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (pickable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  image = picman_pickable_get_image (pickable);

  /*  If there are no bounds, then just extract the entire image
   *  This may not be the correct behavior, but after getting rid
   *  of floating selections, it's still tempting to "cut" or "copy"
   *  a small layer and expect it to work, even though there is no
   *  actual selection mask
   */
  if (PICMAN_IS_DRAWABLE (pickable))
    non_empty = picman_item_mask_bounds (PICMAN_ITEM (pickable),
                                       &x1, &y1, &x2, &y2);
  else
    non_empty = picman_channel_bounds (PICMAN_CHANNEL (selection),
                                     &x1, &y1, &x2, &y2);

  if (non_empty && ((x1 == x2) || (y1 == y2)))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Unable to cut or copy because the "
			     "selected region is empty."));
      return NULL;
    }

  /*  If there is a selection, we must add alpha because the selection
   *  could have any shape.
   */
  if (non_empty)
    add_alpha = TRUE;

  src_format = picman_pickable_get_format (pickable);

  /*  How many bytes in the temp buffer?  */
  if (babl_format_is_palette (src_format) && ! keep_indexed)
    {
      dest_format = picman_image_get_format (image, PICMAN_RGB,
                                           picman_image_get_precision (image),
                                           add_alpha ||
                                           babl_format_has_alpha (src_format));
    }
  else
    {
      if (add_alpha)
        dest_format = picman_pickable_get_format_with_alpha (pickable);
      else
        dest_format = src_format;
    }

  if (PICMAN_IS_DRAWABLE (pickable))
    {
      picman_item_get_offset (PICMAN_ITEM (pickable), &off_x, &off_y);
    }
  else
    {
      off_x = off_y = 0;
    }

  picman_pickable_flush (pickable);

  src_buffer = picman_pickable_get_buffer (pickable);

  /*  Allocate the temp buffer  */
  dest_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, x2 - x1, y2 - y1),
                                 dest_format);

  /*  First, copy the pixels, possibly doing INDEXED->RGB and adding alpha  */
  gegl_buffer_copy (src_buffer, GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1),
                    dest_buffer, GEGL_RECTANGLE (0, 0,0 ,0 ));

  if (non_empty)
    {
      /*  If there is a selection, mask the dest_buffer with it  */

      GeglBuffer *mask_buffer;

      mask_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (selection));

      picman_gegl_apply_opacity (dest_buffer, NULL, NULL, dest_buffer,
                               mask_buffer,
                               - (off_x + x1),
                               - (off_y + y1),
                               1.0);

      if (cut_image && PICMAN_IS_DRAWABLE (pickable))
        {
          picman_edit_clear (image, PICMAN_DRAWABLE (pickable), context);
        }
    }
  else if (cut_image && PICMAN_IS_DRAWABLE (pickable))
    {
      /*  If we're cutting without selection, remove either the layer
       *  (or floating selection), the layer mask, or the channel
       */
      if (cut_image && PICMAN_IS_DRAWABLE (pickable))
        {
          if (PICMAN_IS_LAYER (pickable))
            {
              picman_image_remove_layer (image, PICMAN_LAYER (pickable),
                                       TRUE, NULL);
            }
          else if (PICMAN_IS_LAYER_MASK (pickable))
            {
              picman_layer_apply_mask (picman_layer_mask_get_layer (PICMAN_LAYER_MASK (pickable)),
                                     PICMAN_MASK_DISCARD, TRUE);
            }
          else if (PICMAN_IS_CHANNEL (pickable))
            {
              picman_image_remove_channel (image, PICMAN_CHANNEL (pickable),
                                         TRUE, NULL);
            }
        }
    }

  *offset_x = x1 + off_x;
  *offset_y = y1 + off_y;

  return dest_buffer;
}

PicmanLayer *
picman_selection_float (PicmanSelection *selection,
                      PicmanDrawable  *drawable,
                      PicmanContext   *context,
                      gboolean       cut_image,
                      gint           off_x,
                      gint           off_y,
                      GError       **error)
{
  PicmanImage  *image;
  PicmanLayer  *layer;
  GeglBuffer *buffer;
  gint        x1, y1;
  gint        x2, y2;

  g_return_val_if_fail (PICMAN_IS_SELECTION (selection), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  image = picman_item_get_image (PICMAN_ITEM (selection));

  /*  Make sure there is a region to float...  */
  if (! picman_item_mask_bounds (PICMAN_ITEM (drawable), &x1, &y1, &x2, &y2) ||
      (x1 == x2 || y1 == y2))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot float selection because the selected "
			     "region is empty."));
      return NULL;
    }

  /*  Start an undo group  */
  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_FS_FLOAT,
                               C_("undo-type", "Float Selection"));

  /*  Cut or copy the selected region  */
  buffer = picman_selection_extract (selection, PICMAN_PICKABLE (drawable), context,
                                   cut_image, FALSE, TRUE,
                                   &x1, &y1, NULL);

  /*  Clear the selection  */
  picman_channel_clear (PICMAN_CHANNEL (selection), NULL, TRUE);

  /* Create a new layer from the buffer, using the drawable's type
   *  because it may be different from the image's type if we cut from
   *  a channel or layer mask
   */
  layer = picman_layer_new_from_buffer (buffer, image,
                                      picman_drawable_get_format_with_alpha (drawable),
                                      _("Floated Layer"),
                                      PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

  /*  Set the offsets  */
  picman_item_set_offset (PICMAN_ITEM (layer), x1 + off_x, y1 + off_y);

  /*  Free the temp buffer  */
  g_object_unref (buffer);

  /*  Add the floating layer to the image  */
  floating_sel_attach (layer, drawable);

  /*  End an undo group  */
  picman_image_undo_group_end (image);

  /*  invalidate the image's boundary variables  */
  PICMAN_CHANNEL (selection)->boundary_known = FALSE;

  return layer;
}
