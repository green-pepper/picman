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

#include <stdlib.h>
#include <string.h>

#include <gegl.h>

#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "gegl/picman-babl.h"

#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanlayermask.h"

#include "picman-intl.h"


static gboolean        picman_layer_mask_is_attached        (const PicmanItem    *item);
static gboolean        picman_layer_mask_is_content_locked  (const PicmanItem    *item);
static gboolean        picman_layer_mask_is_position_locked (const PicmanItem    *item);
static PicmanItemTree  * picman_layer_mask_get_tree           (PicmanItem          *item);
static PicmanItem      * picman_layer_mask_duplicate          (PicmanItem          *item,
                                                           GType              new_type);
static gboolean        picman_layer_mask_rename             (PicmanItem          *item,
                                                           const gchar       *new_name,
                                                           const gchar       *undo_desc,
                                                           GError           **error);

static void            picman_layer_mask_convert_type       (PicmanDrawable      *drawable,
                                                           PicmanImage         *dest_image,
                                                           const Babl        *new_format,
                                                           PicmanImageBaseType  new_base_type,
                                                           PicmanPrecision      new_precision,
                                                           gint               layer_dither_type,
                                                           gint               mask_dither_type,
                                                           gboolean           push_undo);


G_DEFINE_TYPE (PicmanLayerMask, picman_layer_mask, PICMAN_TYPE_CHANNEL)

#define parent_class picman_layer_mask_parent_class


static void
picman_layer_mask_class_init (PicmanLayerMaskClass *klass)
{
  PicmanViewableClass *viewable_class = PICMAN_VIEWABLE_CLASS (klass);
  PicmanItemClass     *item_class     = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class = PICMAN_DRAWABLE_CLASS (klass);

  viewable_class->default_stock_id = "picman-layer-mask";

  item_class->is_attached        = picman_layer_mask_is_attached;
  item_class->is_content_locked  = picman_layer_mask_is_content_locked;
  item_class->is_position_locked = picman_layer_mask_is_position_locked;
  item_class->get_tree           = picman_layer_mask_get_tree;
  item_class->duplicate          = picman_layer_mask_duplicate;
  item_class->rename             = picman_layer_mask_rename;
  item_class->translate_desc     = C_("undo-type", "Move Layer Mask");
  item_class->to_selection_desc  = C_("undo-type", "Layer Mask to Selection");

  drawable_class->convert_type  = picman_layer_mask_convert_type;
}

static void
picman_layer_mask_init (PicmanLayerMask *layer_mask)
{
  layer_mask->layer = NULL;
}

static gboolean
picman_layer_mask_is_content_locked (const PicmanItem *item)
{
  PicmanLayerMask *mask  = PICMAN_LAYER_MASK (item);
  PicmanLayer     *layer = picman_layer_mask_get_layer (mask);

  if (layer)
    return picman_item_is_content_locked (PICMAN_ITEM (layer));

  return FALSE;
}

static gboolean
picman_layer_mask_is_position_locked (const PicmanItem *item)
{
  PicmanLayerMask *mask  = PICMAN_LAYER_MASK (item);
  PicmanLayer     *layer = picman_layer_mask_get_layer (mask);

  if (layer)
    return picman_item_is_position_locked (PICMAN_ITEM (layer));

  return FALSE;
}

static gboolean
picman_layer_mask_is_attached (const PicmanItem *item)
{
  PicmanLayerMask *mask  = PICMAN_LAYER_MASK (item);
  PicmanLayer     *layer = picman_layer_mask_get_layer (mask);

  return (PICMAN_IS_IMAGE (picman_item_get_image (item)) &&
          PICMAN_IS_LAYER (layer)                      &&
          picman_layer_get_mask (layer) == mask        &&
          picman_item_is_attached (PICMAN_ITEM (layer)));
}

static PicmanItemTree *
picman_layer_mask_get_tree (PicmanItem *item)
{
  return NULL;
}

static PicmanItem *
picman_layer_mask_duplicate (PicmanItem *item,
                           GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  return new_item;
}

static gboolean
picman_layer_mask_rename (PicmanItem     *item,
                        const gchar  *new_name,
                        const gchar  *undo_desc,
                        GError      **error)
{
  /* reject renaming, layer masks are always named "<layer name> mask"  */

  g_set_error (error, PICMAN_ERROR, PICMAN_FAILED,
	       _("Cannot rename layer masks."));

  return FALSE;
}

static void
picman_layer_mask_convert_type (PicmanDrawable      *drawable,
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

PicmanLayerMask *
picman_layer_mask_new (PicmanImage     *image,
                     gint           width,
                     gint           height,
                     const gchar   *name,
                     const PicmanRGB *color)
{
  PicmanLayerMask *layer_mask;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);
  g_return_val_if_fail (color != NULL, NULL);

  layer_mask =
    PICMAN_LAYER_MASK (picman_drawable_new (PICMAN_TYPE_LAYER_MASK,
                                        image, name,
                                        0, 0, width, height,
                                        picman_image_get_mask_format (image)));

  /*  set the layer_mask color and opacity  */
  picman_channel_set_color (PICMAN_CHANNEL (layer_mask), color, FALSE);
  picman_channel_set_show_masked (PICMAN_CHANNEL (layer_mask), TRUE);

  /*  selection mask variables  */
  PICMAN_CHANNEL (layer_mask)->x2 = width;
  PICMAN_CHANNEL (layer_mask)->y2 = height;

  return layer_mask;
}

void
picman_layer_mask_set_layer (PicmanLayerMask *layer_mask,
                           PicmanLayer     *layer)
{
  g_return_if_fail (PICMAN_IS_LAYER_MASK (layer_mask));
  g_return_if_fail (layer == NULL || PICMAN_IS_LAYER (layer));

  layer_mask->layer = layer;

  if (layer)
    {
      gchar *mask_name;
      gint   offset_x;
      gint   offset_y;

      picman_item_get_offset (PICMAN_ITEM (layer), &offset_x, &offset_y);
      picman_item_set_offset (PICMAN_ITEM (layer_mask), offset_x, offset_y);

      mask_name = g_strdup_printf (_("%s mask"), picman_object_get_name (layer));

      picman_object_take_name (PICMAN_OBJECT (layer_mask), mask_name);
    }
}

PicmanLayer *
picman_layer_mask_get_layer (const PicmanLayerMask *layer_mask)
{
  g_return_val_if_fail (PICMAN_IS_LAYER_MASK (layer_mask), NULL);

  return layer_mask->layer;
}
