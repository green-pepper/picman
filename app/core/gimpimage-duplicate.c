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

#include "picman.h"
#include "picmanchannel.h"
#include "picmanguide.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-duplicate.h"
#include "picmanimage-grid.h"
#include "picmanimage-guides.h"
#include "picmanimage-private.h"
#include "picmanimage-undo.h"
#include "picmanimage-sample-points.h"
#include "picmanitemstack.h"
#include "picmanlayer.h"
#include "picmanlayermask.h"
#include "picmanlayer-floating-sel.h"
#include "picmanparasitelist.h"
#include "picmansamplepoint.h"

#include "vectors/picmanvectors.h"


static void          picman_image_duplicate_resolution      (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_save_source_uri (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_colormap        (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static PicmanItem    * picman_image_duplicate_item            (PicmanItem      *item,
                                                           PicmanImage     *new_image);
static PicmanLayer   * picman_image_duplicate_layers          (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static PicmanChannel * picman_image_duplicate_channels        (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static PicmanVectors * picman_image_duplicate_vectors         (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_floating_sel    (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_mask            (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_components      (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_guides          (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_sample_points   (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_grid            (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_quick_mask      (PicmanImage     *image,
                                                           PicmanImage     *new_image);
static void          picman_image_duplicate_parasites       (PicmanImage     *image,
                                                           PicmanImage     *new_image);


PicmanImage *
picman_image_duplicate (PicmanImage *image)
{
  PicmanImage    *new_image;
  PicmanLayer    *active_layer;
  PicmanChannel  *active_channel;
  PicmanVectors  *active_vectors;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  picman_set_busy_until_idle (image->picman);

  /*  Create a new image  */
  new_image = picman_create_image (image->picman,
                                 picman_image_get_width  (image),
                                 picman_image_get_height (image),
                                 picman_image_get_base_type (image),
                                 picman_image_get_precision (image),
                                 FALSE);
  picman_image_undo_disable (new_image);

  /*  Store the source uri to be used by the save dialog  */
  picman_image_duplicate_save_source_uri (image, new_image);


  /*  Copy the colormap if necessary  */
  picman_image_duplicate_colormap (image, new_image);

  /*  Copy resolution information  */
  picman_image_duplicate_resolution (image, new_image);

  /*  Copy the layers  */
  active_layer = picman_image_duplicate_layers (image, new_image);

  /*  Copy the channels  */
  active_channel = picman_image_duplicate_channels (image, new_image);

  /*  Copy any vectors  */
  active_vectors = picman_image_duplicate_vectors (image, new_image);

  /*  Copy floating layer  */
  picman_image_duplicate_floating_sel (image, new_image);

  /*  Copy the selection mask  */
  picman_image_duplicate_mask (image, new_image);

  /*  Set active layer, active channel, active vectors  */
  if (active_layer)
    picman_image_set_active_layer (new_image, active_layer);

  if (active_channel)
    picman_image_set_active_channel (new_image, active_channel);

  if (active_vectors)
    picman_image_set_active_vectors (new_image, active_vectors);

  /*  Copy state of all color components  */
  picman_image_duplicate_components (image, new_image);

  /*  Copy any guides  */
  picman_image_duplicate_guides (image, new_image);

  /*  Copy any sample points  */
  picman_image_duplicate_sample_points (image, new_image);

  /*  Copy the grid  */
  picman_image_duplicate_grid (image, new_image);

  /*  Copy the quick mask info  */
  picman_image_duplicate_quick_mask (image, new_image);

  /*  Copy parasites  */
  picman_image_duplicate_parasites (image, new_image);

  picman_image_undo_enable (new_image);

  return new_image;
}

static void
picman_image_duplicate_resolution (PicmanImage *image,
                                 PicmanImage *new_image)
{
  gdouble xres;
  gdouble yres;

  picman_image_get_resolution (image, &xres, &yres);
  picman_image_set_resolution (new_image, xres, yres);
  picman_image_set_unit (new_image, picman_image_get_unit (image));
}

static void
picman_image_duplicate_save_source_uri (PicmanImage *image,
                                      PicmanImage *new_image)
{
  g_object_set_data_full (G_OBJECT (new_image), "picman-image-source-uri",
                          g_strdup (picman_image_get_uri (image)),
                          (GDestroyNotify) g_free);
}

static void
picman_image_duplicate_colormap (PicmanImage *image,
                               PicmanImage *new_image)
{
  if (picman_image_get_base_type (new_image) == PICMAN_INDEXED)
    picman_image_set_colormap (new_image,
                             picman_image_get_colormap (image),
                             picman_image_get_colormap_size (image),
                             FALSE);
}

static PicmanItem *
picman_image_duplicate_item (PicmanItem  *item,
                           PicmanImage *new_image)
{
  PicmanItem *new_item;

  new_item = picman_item_convert (item, new_image,
                                G_TYPE_FROM_INSTANCE (item));

  /*  Make sure the copied item doesn't say: "<old item> copy"  */
  picman_object_set_name (PICMAN_OBJECT (new_item),
                        picman_object_get_name (item));

  return new_item;
}

static PicmanLayer *
picman_image_duplicate_layers (PicmanImage *image,
                             PicmanImage *new_image)
{
  PicmanLayer *active_layer = NULL;
  GList     *list;
  gint       count;

  for (list = picman_image_get_layer_iter (image), count = 0;
       list;
       list = g_list_next (list))
    {
      PicmanLayer *layer = list->data;
      PicmanLayer *new_layer;

      if (picman_layer_is_floating_sel (layer))
        continue;

      new_layer = PICMAN_LAYER (picman_image_duplicate_item (PICMAN_ITEM (layer),
                                                         new_image));

      /*  Make sure that if the layer has a layer mask,
       *  its name isn't screwed up
       */
      if (new_layer->mask)
        picman_object_set_name (PICMAN_OBJECT (new_layer->mask),
                              picman_object_get_name (layer->mask));

      if (picman_image_get_active_layer (image) == layer)
        active_layer = new_layer;

      picman_image_add_layer (new_image, new_layer,
                            NULL, count++, FALSE);
    }

  return active_layer;
}

static PicmanChannel *
picman_image_duplicate_channels (PicmanImage *image,
                               PicmanImage *new_image)
{
  PicmanChannel *active_channel = NULL;
  GList       *list;
  gint         count;

  for (list = picman_image_get_channel_iter (image), count = 0;
       list;
       list = g_list_next (list))
    {
      PicmanChannel  *channel = list->data;
      PicmanChannel  *new_channel;

      new_channel = PICMAN_CHANNEL (picman_image_duplicate_item (PICMAN_ITEM (channel),
                                                             new_image));

      if (picman_image_get_active_channel (image) == channel)
        active_channel = new_channel;

      picman_image_add_channel (new_image, new_channel,
                              NULL, count++, FALSE);
    }

  return active_channel;
}

static PicmanVectors *
picman_image_duplicate_vectors (PicmanImage *image,
                              PicmanImage *new_image)
{
  PicmanVectors *active_vectors = NULL;
  GList       *list;
  gint         count;

  for (list = picman_image_get_vectors_iter (image), count = 0;
       list;
       list = g_list_next (list))
    {
      PicmanVectors  *vectors = list->data;
      PicmanVectors  *new_vectors;

      new_vectors = PICMAN_VECTORS (picman_image_duplicate_item (PICMAN_ITEM (vectors),
                                                             new_image));

      if (picman_image_get_active_vectors (image) == vectors)
        active_vectors = new_vectors;

      picman_image_add_vectors (new_image, new_vectors,
                              NULL, count++, FALSE);
    }

  return active_vectors;
}

static void
picman_image_duplicate_floating_sel (PicmanImage *image,
                                   PicmanImage *new_image)
{
  PicmanLayer     *floating_sel;
  PicmanDrawable  *floating_sel_drawable;
  GList         *floating_sel_path;
  PicmanItemStack *new_item_stack;
  PicmanLayer     *new_floating_sel;
  PicmanDrawable  *new_floating_sel_drawable;

  floating_sel = picman_image_get_floating_selection (image);

  if (! floating_sel)
    return;

  floating_sel_drawable = picman_layer_get_floating_sel_drawable (floating_sel);

  if (PICMAN_IS_LAYER_MASK (floating_sel_drawable))
    {
      PicmanLayer *layer;

      layer = picman_layer_mask_get_layer (PICMAN_LAYER_MASK (floating_sel_drawable));

      floating_sel_path = picman_item_get_path (PICMAN_ITEM (layer));

      new_item_stack = PICMAN_ITEM_STACK (picman_image_get_layers (new_image));
    }
  else
    {
      floating_sel_path = picman_item_get_path (PICMAN_ITEM (floating_sel_drawable));

      if (PICMAN_IS_LAYER (floating_sel_drawable))
        new_item_stack = PICMAN_ITEM_STACK (picman_image_get_layers (new_image));
      else
        new_item_stack = PICMAN_ITEM_STACK (picman_image_get_channels (new_image));
    }

  /*  adjust path[0] for the floating layer missing in new_image  */
  floating_sel_path->data =
    GUINT_TO_POINTER (GPOINTER_TO_UINT (floating_sel_path->data) - 1);

  if (PICMAN_IS_LAYER (floating_sel_drawable))
    {
      new_floating_sel =
        PICMAN_LAYER (picman_image_duplicate_item (PICMAN_ITEM (floating_sel),
                                               new_image));
    }
  else
    {
      /*  can't use picman_item_convert() for floating selections of channels
       *  or layer masks because they maybe don't have a normal layer's type
       */
      new_floating_sel =
        PICMAN_LAYER (picman_item_duplicate (PICMAN_ITEM (floating_sel),
                                         G_TYPE_FROM_INSTANCE (floating_sel)));
      picman_item_set_image (PICMAN_ITEM (new_floating_sel), new_image);

      picman_object_set_name (PICMAN_OBJECT (new_floating_sel),
                            picman_object_get_name (floating_sel));
    }

  /*  Make sure the copied layer doesn't say: "<old layer> copy"  */
  picman_object_set_name (PICMAN_OBJECT (new_floating_sel),
                        picman_object_get_name (floating_sel));

  new_floating_sel_drawable =
    PICMAN_DRAWABLE (picman_item_stack_get_item_by_path (new_item_stack,
                                                     floating_sel_path));

  if (PICMAN_IS_LAYER_MASK (floating_sel_drawable))
    new_floating_sel_drawable =
      PICMAN_DRAWABLE (picman_layer_get_mask (PICMAN_LAYER (new_floating_sel_drawable)));

  floating_sel_attach (new_floating_sel, new_floating_sel_drawable);

  g_list_free (floating_sel_path);
}

static void
picman_image_duplicate_mask (PicmanImage *image,
                           PicmanImage *new_image)
{
  PicmanDrawable *mask;
  PicmanDrawable *new_mask;

  mask     = PICMAN_DRAWABLE (picman_image_get_mask (image));
  new_mask = PICMAN_DRAWABLE (picman_image_get_mask (new_image));

  gegl_buffer_copy (picman_drawable_get_buffer (mask), NULL,
                    picman_drawable_get_buffer (new_mask), NULL);

  PICMAN_CHANNEL (new_mask)->bounds_known   = FALSE;
  PICMAN_CHANNEL (new_mask)->boundary_known = FALSE;
}

static void
picman_image_duplicate_components (PicmanImage *image,
                                 PicmanImage *new_image)
{
  PicmanImagePrivate *private     = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanImagePrivate *new_private = PICMAN_IMAGE_GET_PRIVATE (new_image);
  gint              count;

  for (count = 0; count < MAX_CHANNELS; count++)
    {
      new_private->visible[count] = private->visible[count];
      new_private->active[count]  = private->active[count];
    }
}

static void
picman_image_duplicate_guides (PicmanImage *image,
                             PicmanImage *new_image)
{
  GList *list;

  for (list = picman_image_get_guides (image);
       list;
       list = g_list_next (list))
    {
      PicmanGuide *guide    = list->data;
      gint       position = picman_guide_get_position (guide);

      switch (picman_guide_get_orientation (guide))
        {
        case PICMAN_ORIENTATION_HORIZONTAL:
          picman_image_add_hguide (new_image, position, FALSE);
          break;

        case PICMAN_ORIENTATION_VERTICAL:
          picman_image_add_vguide (new_image, position, FALSE);
          break;

        default:
          g_error ("Unknown guide orientation.\n");
        }
    }
}

static void
picman_image_duplicate_sample_points (PicmanImage *image,
                                    PicmanImage *new_image)
{
  GList *list;

  for (list = picman_image_get_sample_points (image);
       list;
       list = g_list_next (list))
    {
      PicmanSamplePoint *sample_point = list->data;

      picman_image_add_sample_point_at_pos (new_image,
                                          sample_point->x,
                                          sample_point->y,
                                          FALSE);
    }
}

static void
picman_image_duplicate_grid (PicmanImage *image,
                           PicmanImage *new_image)
{
  if (picman_image_get_grid (image))
    picman_image_set_grid (new_image, picman_image_get_grid (image), FALSE);
}

static void
picman_image_duplicate_quick_mask (PicmanImage *image,
                                 PicmanImage *new_image)
{
  PicmanImagePrivate *private     = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanImagePrivate *new_private = PICMAN_IMAGE_GET_PRIVATE (new_image);

  new_private->quick_mask_state    = private->quick_mask_state;
  new_private->quick_mask_inverted = private->quick_mask_inverted;
  new_private->quick_mask_color    = private->quick_mask_color;
}

static void
picman_image_duplicate_parasites (PicmanImage *image,
                                PicmanImage *new_image)
{
  PicmanImagePrivate *private     = PICMAN_IMAGE_GET_PRIVATE (image);
  PicmanImagePrivate *new_private = PICMAN_IMAGE_GET_PRIVATE (new_image);

  if (private->parasites)
    {
      g_object_unref (new_private->parasites);
      new_private->parasites = picman_parasite_list_copy (private->parasites);
    }
}
