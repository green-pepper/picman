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

#include "picman.h"
#include "picmanchannelpropundo.h"
#include "picmanchannelundo.h"
#include "picmandrawablemodundo.h"
#include "picmandrawableundo.h"
#include "picmanfloatingselundo.h"
#include "picmangrid.h"
#include "picmangrouplayer.h"
#include "picmangrouplayerundo.h"
#include "picmanguide.h"
#include "picmanguideundo.h"
#include "picmanimage.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanimageundo.h"
#include "picmanitempropundo.h"
#include "picmanlayermask.h"
#include "picmanlayermaskpropundo.h"
#include "picmanlayermaskundo.h"
#include "picmanlayerpropundo.h"
#include "picmanlayerundo.h"
#include "picmanmaskundo.h"
#include "picmansamplepoint.h"
#include "picmansamplepointundo.h"
#include "picmanselection.h"

#include "text/picmantextlayer.h"
#include "text/picmantextundo.h"

#include "vectors/picmanvectors.h"
#include "vectors/picmanvectorsmodundo.h"
#include "vectors/picmanvectorspropundo.h"
#include "vectors/picmanvectorsundo.h"

#include "picman-intl.h"


/**************************/
/*  Image Property Undos  */
/**************************/

PicmanUndo *
picman_image_undo_push_image_type (PicmanImage   *image,
                                 const gchar *undo_desc)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_TYPE, undo_desc,
                               PICMAN_DIRTY_IMAGE,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_precision (PicmanImage   *image,
                                      const gchar *undo_desc)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_PRECISION, undo_desc,
                               PICMAN_DIRTY_IMAGE,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_size (PicmanImage   *image,
                                 const gchar *undo_desc,
                                 gint         previous_origin_x,
                                 gint         previous_origin_y,
                                 gint         previous_width,
                                 gint         previous_height)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_SIZE, undo_desc,
                               PICMAN_DIRTY_IMAGE | PICMAN_DIRTY_IMAGE_SIZE,
                               "previous-origin-x", previous_origin_x,
                               "previous-origin-y", previous_origin_y,
                               "previous-width",    previous_width,
                               "previous-height",   previous_height,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_resolution (PicmanImage   *image,
                                       const gchar *undo_desc)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_RESOLUTION, undo_desc,
                               PICMAN_DIRTY_IMAGE,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_grid (PicmanImage   *image,
                                 const gchar *undo_desc,
                                 PicmanGrid    *grid)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GRID (grid), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_GRID, undo_desc,
                               PICMAN_DIRTY_IMAGE_META,
                               "grid", grid,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_colormap (PicmanImage   *image,
                                     const gchar *undo_desc)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_IMAGE_COLORMAP, undo_desc,
                               PICMAN_DIRTY_IMAGE,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_parasite (PicmanImage          *image,
                                     const gchar        *undo_desc,
                                     const PicmanParasite *parasite)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (parasite != NULL, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_PARASITE_ATTACH, undo_desc,
                               PICMAN_DIRTY_IMAGE_META,
                               "parasite-name", picman_parasite_name (parasite),
                               NULL);
}

PicmanUndo *
picman_image_undo_push_image_parasite_remove (PicmanImage   *image,
                                            const gchar *undo_desc,
                                            const gchar *name)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_IMAGE_UNDO,
                               PICMAN_UNDO_PARASITE_REMOVE, undo_desc,
                               PICMAN_DIRTY_IMAGE_META,
                               "parasite-name", name,
                               NULL);
}


/********************************/
/*  Guide & Sample Point Undos  */
/********************************/

PicmanUndo *
picman_image_undo_push_guide (PicmanImage   *image,
                            const gchar *undo_desc,
                            PicmanGuide   *guide)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GUIDE (guide), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_GUIDE_UNDO,
                               PICMAN_UNDO_GUIDE, undo_desc,
                               PICMAN_DIRTY_IMAGE_META,
                               "guide", guide,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_sample_point (PicmanImage       *image,
                                   const gchar     *undo_desc,
                                   PicmanSamplePoint *sample_point)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (sample_point != NULL, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_SAMPLE_POINT_UNDO,
                               PICMAN_UNDO_SAMPLE_POINT, undo_desc,
                               PICMAN_DIRTY_IMAGE_META,
                               "sample-point", sample_point,
                               NULL);
}


/********************/
/*  Drawable Undos  */
/********************/

PicmanUndo *
picman_image_undo_push_drawable (PicmanImage    *image,
                               const gchar  *undo_desc,
                               PicmanDrawable *drawable,
                               GeglBuffer   *buffer,
                               gint          x,
                               gint          y)
{
  PicmanItem *item;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);

  item = PICMAN_ITEM (drawable);

  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_DRAWABLE_UNDO,
                               PICMAN_UNDO_DRAWABLE, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",   item,
                               "buffer", buffer,
                               "x",      x,
                               "y",      y,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_drawable_mod (PicmanImage    *image,
                                   const gchar  *undo_desc,
                                   PicmanDrawable *drawable,
                                   gboolean      copy_buffer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_DRAWABLE_MOD_UNDO,
                               PICMAN_UNDO_DRAWABLE_MOD, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",        drawable,
                               "copy-buffer", copy_buffer,
                               NULL);
}


/****************/
/*  Mask Undos  */
/****************/

PicmanUndo *
picman_image_undo_push_mask (PicmanImage   *image,
                           const gchar *undo_desc,
                           PicmanChannel *mask)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CHANNEL (mask), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (mask)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_MASK_UNDO,
                               PICMAN_UNDO_MASK, undo_desc,
                               PICMAN_IS_SELECTION (mask) ?
                               PICMAN_DIRTY_SELECTION :
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item", mask,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_mask_precision (PicmanImage   *image,
                                     const gchar *undo_desc,
                                     PicmanChannel *mask)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CHANNEL (mask), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (mask)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_MASK_UNDO,
                               PICMAN_UNDO_MASK, undo_desc,
                               PICMAN_IS_SELECTION (mask) ?
                               PICMAN_DIRTY_SELECTION :
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",           mask,
                               "convert-format", TRUE,
                               NULL);
}


/****************/
/*  Item Undos  */
/****************/

PicmanUndo *
picman_image_undo_push_item_reorder (PicmanImage   *image,
                                   const gchar *undo_desc,
                                   PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_REORDER, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_rename (PicmanImage   *image,
                                  const gchar *undo_desc,
                                  PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_RENAME, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_displace (PicmanImage   *image,
                                    const gchar *undo_desc,
                                    PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_DISPLACE, undo_desc,
                               PICMAN_IS_DRAWABLE (item) ?
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE :
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_VECTORS,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_visibility (PicmanImage   *image,
                                      const gchar *undo_desc,
                                      PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_VISIBILITY, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_linked (PicmanImage   *image,
                                  const gchar *undo_desc,
                                  PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_LINKED, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_lock_content (PicmanImage   *image,
                                        const gchar *undo_desc,
                                        PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_LOCK_CONTENT, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_lock_position (PicmanImage   *image,
                                         const gchar *undo_desc,
                                         PicmanItem    *item)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_ITEM_LOCK_POSITION, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", item,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_parasite (PicmanImage          *image,
                                    const gchar        *undo_desc,
                                    PicmanItem           *item,
                                    const PicmanParasite *parasite)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);
  g_return_val_if_fail (parasite != NULL, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_PARASITE_ATTACH, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item",          item,
                               "parasite-name", picman_parasite_name (parasite),
                               NULL);
}

PicmanUndo *
picman_image_undo_push_item_parasite_remove (PicmanImage   *image,
                                           const gchar *undo_desc,
                                           PicmanItem    *item,
                                           const gchar *name)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_ITEM (item), NULL);
  g_return_val_if_fail (picman_item_is_attached (item), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_ITEM_PROP_UNDO,
                               PICMAN_UNDO_PARASITE_REMOVE, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item",          item,
                               "parasite-name", name,
                               NULL);
}


/*****************/
/*  Layer Undos  */
/*****************/

PicmanUndo *
picman_image_undo_push_layer_add (PicmanImage   *image,
                                const gchar *undo_desc,
                                PicmanLayer   *layer,
                                PicmanLayer   *prev_layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (! picman_item_is_attached (PICMAN_ITEM (layer)), NULL);
  g_return_val_if_fail (prev_layer == NULL || PICMAN_IS_LAYER (prev_layer),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_UNDO,
                               PICMAN_UNDO_LAYER_ADD, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",       layer,
                               "prev-layer", prev_layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_remove (PicmanImage   *image,
                                   const gchar *undo_desc,
                                   PicmanLayer   *layer,
                                   PicmanLayer   *prev_parent,
                                   gint         prev_position,
                                   PicmanLayer   *prev_layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);
  g_return_val_if_fail (prev_parent == NULL || PICMAN_IS_LAYER (prev_parent),
                        NULL);
  g_return_val_if_fail (prev_layer == NULL || PICMAN_IS_LAYER (prev_layer),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_UNDO,
                               PICMAN_UNDO_LAYER_REMOVE, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",          layer,
                               "prev-parent",   prev_parent,
                               "prev-position", prev_position,
                               "prev-layer",    prev_layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_mode (PicmanImage   *image,
                                 const gchar *undo_desc,
                                 PicmanLayer   *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_PROP_UNDO,
                               PICMAN_UNDO_LAYER_MODE, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_opacity (PicmanImage   *image,
                                    const gchar *undo_desc,
                                    PicmanLayer   *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_PROP_UNDO,
                               PICMAN_UNDO_LAYER_OPACITY, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_lock_alpha (PicmanImage   *image,
                                       const gchar *undo_desc,
                                       PicmanLayer   *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_PROP_UNDO,
                               PICMAN_UNDO_LAYER_LOCK_ALPHA, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}


/***********************/
/*  Group Layer Undos  */
/***********************/

PicmanUndo *
picman_image_undo_push_group_layer_suspend (PicmanImage      *image,
                                          const gchar    *undo_desc,
                                          PicmanGroupLayer *group)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GROUP_LAYER (group), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (group)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_GROUP_LAYER_UNDO,
                               PICMAN_UNDO_GROUP_LAYER_SUSPEND, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",  group,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_group_layer_resume (PicmanImage      *image,
                                         const gchar    *undo_desc,
                                         PicmanGroupLayer *group)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GROUP_LAYER (group), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (group)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_GROUP_LAYER_UNDO,
                               PICMAN_UNDO_GROUP_LAYER_RESUME, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",  group,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_group_layer_convert (PicmanImage      *image,
                                          const gchar    *undo_desc,
                                          PicmanGroupLayer *group)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GROUP_LAYER (group), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (group)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_GROUP_LAYER_UNDO,
                               PICMAN_UNDO_GROUP_LAYER_CONVERT, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item", group,
                               NULL);
}


/**********************/
/*  Text Layer Undos  */
/**********************/

PicmanUndo *
picman_image_undo_push_text_layer (PicmanImage        *image,
                                 const gchar      *undo_desc,
                                 PicmanTextLayer    *layer,
                                 const GParamSpec *pspec)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_TEXT_UNDO,
                               PICMAN_UNDO_TEXT_LAYER, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item",  layer,
                               "param", pspec,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_text_layer_modified (PicmanImage     *image,
                                          const gchar   *undo_desc,
                                          PicmanTextLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_TEXT_UNDO,
                               PICMAN_UNDO_TEXT_LAYER_MODIFIED, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_text_layer_convert (PicmanImage     *image,
                                         const gchar   *undo_desc,
                                         PicmanTextLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_TEXT_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_TEXT_UNDO,
                               PICMAN_UNDO_TEXT_LAYER_CONVERT, undo_desc,
                               PICMAN_DIRTY_ITEM,
                               "item", layer,
                               NULL);
}


/**********************/
/*  Layer Mask Undos  */
/**********************/

PicmanUndo *
picman_image_undo_push_layer_mask_add (PicmanImage     *image,
                                     const gchar   *undo_desc,
                                     PicmanLayer     *layer,
                                     PicmanLayerMask *mask)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER_MASK (mask), NULL);
  g_return_val_if_fail (! picman_item_is_attached (PICMAN_ITEM (mask)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_MASK_UNDO,
                               PICMAN_UNDO_LAYER_MASK_ADD, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",       layer,
                               "layer-mask", mask,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_mask_remove (PicmanImage     *image,
                                        const gchar   *undo_desc,
                                        PicmanLayer     *layer,
                                        PicmanLayerMask *mask)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER_MASK (mask), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (mask)), NULL);
  g_return_val_if_fail (picman_layer_mask_get_layer (mask) == layer, NULL);
  g_return_val_if_fail (picman_layer_get_mask (layer) == mask, NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_MASK_UNDO,
                               PICMAN_UNDO_LAYER_MASK_REMOVE, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",       layer,
                               "layer-mask", mask,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_mask_apply (PicmanImage   *image,
                                       const gchar *undo_desc,
                                       PicmanLayer   *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_MASK_PROP_UNDO,
                               PICMAN_UNDO_LAYER_MASK_APPLY, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_layer_mask_show (PicmanImage   *image,
                                      const gchar *undo_desc,
                                      PicmanLayer   *layer)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_LAYER_MASK_PROP_UNDO,
                               PICMAN_UNDO_LAYER_MASK_SHOW, undo_desc,
                               PICMAN_DIRTY_ITEM_META,
                               "item", layer,
                               NULL);
}


/*******************/
/*  Channel Undos  */
/*******************/

PicmanUndo *
picman_image_undo_push_channel_add (PicmanImage   *image,
                                  const gchar *undo_desc,
                                  PicmanChannel *channel,
                                  PicmanChannel *prev_channel)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), NULL);
  g_return_val_if_fail (! picman_item_is_attached (PICMAN_ITEM (channel)), NULL);
  g_return_val_if_fail (prev_channel == NULL || PICMAN_IS_CHANNEL (prev_channel),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_CHANNEL_UNDO,
                               PICMAN_UNDO_CHANNEL_ADD, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",         channel,
                               "prev-channel", prev_channel,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_channel_remove (PicmanImage   *image,
                                     const gchar *undo_desc,
                                     PicmanChannel *channel,
                                     PicmanChannel *prev_parent,
                                     gint         prev_position,
                                     PicmanChannel *prev_channel)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)), NULL);
  g_return_val_if_fail (prev_parent == NULL || PICMAN_IS_CHANNEL (prev_parent),
                        NULL);
  g_return_val_if_fail (prev_channel == NULL || PICMAN_IS_CHANNEL (prev_channel),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_CHANNEL_UNDO,
                               PICMAN_UNDO_CHANNEL_REMOVE, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",          channel,
                               "prev-parent",   prev_parent,
                               "prev-position", prev_position,
                               "prev-channel",  prev_channel,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_channel_color (PicmanImage   *image,
                                    const gchar *undo_desc,
                                    PicmanChannel *channel)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_CHANNEL_PROP_UNDO,
                               PICMAN_UNDO_CHANNEL_COLOR, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_DRAWABLE,
                               "item", channel,
                               NULL);
}


/*******************/
/*  Vectors Undos  */
/*******************/

PicmanUndo *
picman_image_undo_push_vectors_add (PicmanImage   *image,
                                  const gchar *undo_desc,
                                  PicmanVectors *vectors,
                                  PicmanVectors *prev_vectors)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);
  g_return_val_if_fail (! picman_item_is_attached (PICMAN_ITEM (vectors)), NULL);
  g_return_val_if_fail (prev_vectors == NULL || PICMAN_IS_VECTORS (prev_vectors),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_VECTORS_UNDO,
                               PICMAN_UNDO_VECTORS_ADD, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",         vectors,
                               "prev-vectors", prev_vectors,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_vectors_remove (PicmanImage   *image,
                                     const gchar *undo_desc,
                                     PicmanVectors *vectors,
                                     PicmanVectors *prev_parent,
                                     gint         prev_position,
                                     PicmanVectors *prev_vectors)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (vectors)), NULL);
  g_return_val_if_fail (prev_parent == NULL || PICMAN_IS_VECTORS (prev_parent),
                        NULL);
  g_return_val_if_fail (prev_vectors == NULL || PICMAN_IS_VECTORS (prev_vectors),
                        NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_VECTORS_UNDO,
                               PICMAN_UNDO_VECTORS_REMOVE, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item",          vectors,
                               "prev-parent",   prev_parent,
                               "prev-position", prev_position,
                               "prev-vectors",  prev_vectors,
                               NULL);
}

PicmanUndo *
picman_image_undo_push_vectors_mod (PicmanImage   *image,
                                  const gchar *undo_desc,
                                  PicmanVectors *vectors)
{
  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (vectors)), NULL);

  return picman_image_undo_push (image, PICMAN_TYPE_VECTORS_MOD_UNDO,
                               PICMAN_UNDO_VECTORS_MOD, undo_desc,
                               PICMAN_DIRTY_ITEM | PICMAN_DIRTY_VECTORS,
                               "item", vectors,
                               NULL);
}


/******************************/
/*  Floating Selection Undos  */
/******************************/

PicmanUndo *
picman_image_undo_push_fs_to_layer (PicmanImage    *image,
                                  const gchar  *undo_desc,
                                  PicmanLayer    *floating_layer)
{
  PicmanUndo *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (floating_layer), NULL);

  undo = picman_image_undo_push (image, PICMAN_TYPE_FLOATING_SEL_UNDO,
                               PICMAN_UNDO_FS_TO_LAYER, undo_desc,
                               PICMAN_DIRTY_IMAGE_STRUCTURE,
                               "item", floating_layer,
                               NULL);

  return undo;
}


/******************************************************************************/
/*  Something for which programmer is too lazy to write an undo function for  */
/******************************************************************************/

static void
undo_pop_cantundo (PicmanUndo            *undo,
                   PicmanUndoMode         undo_mode,
                   PicmanUndoAccumulator *accum)
{
  switch (undo_mode)
    {
    case PICMAN_UNDO_MODE_UNDO:
      picman_message (undo->image->picman, NULL, PICMAN_MESSAGE_WARNING,
                    _("Can't undo %s"), picman_object_get_name (undo));
      break;

    case PICMAN_UNDO_MODE_REDO:
      break;
    }
}

PicmanUndo *
picman_image_undo_push_cantundo (PicmanImage   *image,
                               const gchar *undo_desc)
{
  PicmanUndo *undo;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  /* This is the sole purpose of this type of undo: the ability to
   * mark an image as having been mutated, without really providing
   * any adequate undo facility.
   */

  undo = picman_image_undo_push (image, PICMAN_TYPE_UNDO,
                               PICMAN_UNDO_CANT, undo_desc,
                               PICMAN_DIRTY_ALL,
                               NULL);

  if (undo)
    g_signal_connect (undo, "pop",
                      G_CALLBACK (undo_pop_cantundo),
                      NULL);

  return undo;
}
