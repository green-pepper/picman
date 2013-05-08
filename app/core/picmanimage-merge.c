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
#include "libpicmanmath/picmanmath.h"
#include "libpicmanbase/picmanbase.h"

#include "core-types.h"

#include "gegl/picmanapplicator.h"
#include "gegl/picman-babl-compat.h"
#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-utils.h"

#include "vectors/picmanvectors.h"

#include "picman.h"
#include "picmancontext.h"
#include "picmanerror.h"
#include "picmangrouplayer.h"
#include "picmanimage.h"
#include "picmanimage-merge.h"
#include "picmanimage-undo.h"
#include "picmanitemstack.h"
#include "picmanlayer-floating-sel.h"
#include "picmanlayermask.h"
#include "picmanmarshal.h"
#include "picmanparasitelist.h"
#include "picmanundostack.h"

#include "picman-intl.h"


static PicmanLayer * picman_image_merge_layers (PicmanImage     *image,
                                            PicmanContainer *container,
                                            GSList        *merge_list,
                                            PicmanContext   *context,
                                            PicmanMergeType  merge_type);


/*  public functions  */

PicmanLayer *
picman_image_merge_visible_layers (PicmanImage     *image,
                                 PicmanContext   *context,
                                 PicmanMergeType  merge_type,
                                 gboolean       merge_active_group,
                                 gboolean       discard_invisible)
{
  PicmanContainer *container;
  GList         *list;
  GSList        *merge_list     = NULL;
  GSList        *invisible_list = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  if (merge_active_group)
    {
      PicmanLayer *active_layer = picman_image_get_active_layer (image);

      /*  if the active layer is the floating selection, get the
       *  underlying drawable, but only if it is a layer
       */
      if (active_layer && picman_layer_is_floating_sel (active_layer))
        {
          PicmanDrawable *fs_drawable;

          fs_drawable = picman_layer_get_floating_sel_drawable (active_layer);

          if (PICMAN_IS_LAYER (fs_drawable))
            active_layer = PICMAN_LAYER (fs_drawable);
        }

      if (active_layer)
        container = picman_item_get_container (PICMAN_ITEM (active_layer));
      else
        container = picman_image_get_layers (image);
    }
  else
    {
      container = picman_image_get_layers (image);
    }

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (container));
       list;
       list = g_list_next (list))
    {
      PicmanLayer *layer = list->data;

      if (picman_layer_is_floating_sel (layer))
        continue;

      if (picman_item_get_visible (PICMAN_ITEM (layer)))
        {
          merge_list = g_slist_append (merge_list, layer);
        }
      else if (discard_invisible)
        {
          invisible_list = g_slist_append (invisible_list, layer);
        }
    }

  if (merge_list)
    {
      PicmanLayer *layer;

      picman_set_busy (image->picman);

      picman_image_undo_group_start (image,
                                   PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE,
                                   C_("undo-type", "Merge Visible Layers"));

      /* if there's a floating selection, anchor it */
      if (picman_image_get_floating_selection (image))
        floating_sel_anchor (picman_image_get_floating_selection (image));

      layer = picman_image_merge_layers (image,
                                       container,
                                       merge_list, context, merge_type);
      g_slist_free (merge_list);

      if (invisible_list)
        {
          GSList *list;

          for (list = invisible_list; list; list = g_slist_next (list))
            picman_image_remove_layer (image, list->data, TRUE, NULL);

          g_slist_free (invisible_list);
        }

      picman_image_undo_group_end (image);

      picman_unset_busy (image->picman);

      return layer;
    }

  return picman_image_get_active_layer (image);
}

PicmanLayer *
picman_image_flatten (PicmanImage   *image,
                    PicmanContext *context)
{
  GList     *list;
  GSList    *merge_list = NULL;
  PicmanLayer *layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  picman_set_busy (image->picman);

  picman_image_undo_group_start (image,
                               PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE,
                               C_("undo-type", "Flatten Image"));

  /* if there's a floating selection, anchor it */
  if (picman_image_get_floating_selection (image))
    floating_sel_anchor (picman_image_get_floating_selection (image));

  for (list = picman_image_get_layer_iter (image);
       list;
       list = g_list_next (list))
    {
      layer = list->data;

      if (picman_item_get_visible (PICMAN_ITEM (layer)))
        merge_list = g_slist_append (merge_list, layer);
    }

  layer = picman_image_merge_layers (image,
                                   picman_image_get_layers (image),
                                   merge_list, context,
                                   PICMAN_FLATTEN_IMAGE);
  g_slist_free (merge_list);

  picman_image_alpha_changed (image);

  picman_image_undo_group_end (image);

  picman_unset_busy (image->picman);

  return layer;
}

PicmanLayer *
picman_image_merge_down (PicmanImage      *image,
                       PicmanLayer      *current_layer,
                       PicmanContext    *context,
                       PicmanMergeType   merge_type,
                       GError        **error)
{
  PicmanLayer *layer;
  GList     *list;
  GList     *layer_list = NULL;
  GSList    *merge_list = NULL;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER (current_layer), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (current_layer)), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  for (list = picman_item_get_container_iter (PICMAN_ITEM (current_layer));
       list;
       list = g_list_next (list))
    {
      layer = list->data;

      if (layer == current_layer)
        break;
    }

  for (layer_list = g_list_next (list);
       layer_list;
       layer_list = g_list_next (layer_list))
    {
      layer = layer_list->data;

      if (picman_item_get_visible (PICMAN_ITEM (layer)))
        {
          if (picman_viewable_get_children (PICMAN_VIEWABLE (layer)))
            {
              g_set_error_literal (error, 0, 0,
                                   _("Cannot merge down to a layer group."));
              return NULL;
            }

          if (picman_item_is_content_locked (PICMAN_ITEM (layer)))
            {
              g_set_error_literal (error, 0, 0,
                                   _("The layer to merge down to is locked."));
              return NULL;
            }

          merge_list = g_slist_append (NULL, layer);
          break;
        }
    }

  if (! merge_list)
    {
      g_set_error_literal (error, 0, 0,
                           _("There is no visible layer to merge down to."));
      return NULL;
    }

  merge_list = g_slist_prepend (merge_list, current_layer);

  picman_set_busy (image->picman);

  picman_image_undo_group_start (image,
                               PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE,
                               C_("undo-type", "Merge Down"));

  layer = picman_image_merge_layers (image,
                                   picman_item_get_container (PICMAN_ITEM (current_layer)),
                                   merge_list, context, merge_type);
  g_slist_free (merge_list);

  picman_image_undo_group_end (image);

  picman_unset_busy (image->picman);

  return layer;
}

PicmanLayer *
picman_image_merge_group_layer (PicmanImage      *image,
                              PicmanGroupLayer *group)
{
  PicmanLayer *parent;
  PicmanLayer *layer;
  gint       index;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_GROUP_LAYER (group), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (group)), NULL);
  g_return_val_if_fail (picman_item_get_image (PICMAN_ITEM (group)) == image, NULL);

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_LAYERS_MERGE,
                               C_("undo-type", "Merge Layer Group"));

  parent = picman_layer_get_parent (PICMAN_LAYER (group));
  index  = picman_item_get_index (PICMAN_ITEM (group));

  layer = PICMAN_LAYER (picman_item_duplicate (PICMAN_ITEM (group),
                                           PICMAN_TYPE_LAYER));

  picman_object_set_name (PICMAN_OBJECT (layer), picman_object_get_name (group));

  picman_image_remove_layer (image, PICMAN_LAYER (group), TRUE, NULL);
  picman_image_add_layer (image, layer, parent, index, TRUE);

  picman_image_undo_group_end (image);

  return layer;
}


/* merging vectors */

PicmanVectors *
picman_image_merge_visible_vectors (PicmanImage  *image,
                                  GError    **error)
{
  GList       *list;
  GList       *merge_list = NULL;
  PicmanVectors *vectors;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  for (list = picman_image_get_vectors_iter (image);
       list;
       list = g_list_next (list))
    {
      vectors = list->data;

      if (picman_item_get_visible (PICMAN_ITEM (vectors)))
        merge_list = g_list_prepend (merge_list, vectors);
    }

  merge_list = g_list_reverse (merge_list);

  if (merge_list && merge_list->next)
    {
      PicmanVectors *target_vectors;
      gchar       *name;
      gint         pos;

      picman_set_busy (image->picman);

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_IMAGE_VECTORS_MERGE,
                                   C_("undo-type", "Merge Visible Paths"));

      vectors = PICMAN_VECTORS (merge_list->data);

      name = g_strdup (picman_object_get_name (vectors));
      pos = picman_item_get_index (PICMAN_ITEM (vectors));

      target_vectors = PICMAN_VECTORS (picman_item_duplicate (PICMAN_ITEM (vectors),
                                                          PICMAN_TYPE_VECTORS));
      picman_image_remove_vectors (image, vectors, TRUE, NULL);

      for (list = g_list_next (merge_list);
           list;
           list = g_list_next (list))
        {
          vectors = list->data;

          picman_vectors_add_strokes (vectors, target_vectors);
          picman_image_remove_vectors (image, vectors, TRUE, NULL);
        }

      picman_object_take_name (PICMAN_OBJECT (target_vectors), name);

      g_list_free (merge_list);

      /* FIXME tree */
      picman_image_add_vectors (image, target_vectors, NULL, pos, TRUE);
      picman_unset_busy (image->picman);

      picman_image_undo_group_end (image);

      return target_vectors;
    }
  else
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Not enough visible paths for a merge. "
			     "There must be at least two."));
      return NULL;
    }
}


/*  private functions  */

static PicmanLayer *
picman_image_merge_layers (PicmanImage     *image,
                         PicmanContainer *container,
                         GSList        *merge_list,
                         PicmanContext   *context,
                         PicmanMergeType  merge_type)
{
  GList            *list;
  GSList           *reverse_list = NULL;
  GSList           *layers;
  PicmanLayer        *merge_layer;
  PicmanLayer        *layer;
  PicmanLayer        *bottom_layer;
  PicmanParasiteList *parasites;
  gint              count;
  gint              x1, y1, x2, y2;
  gint              off_x, off_y;
  gint              position;
  gchar            *name;
  PicmanLayer        *parent;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_CONTEXT (context), NULL);

  layer        = NULL;
  x1 = y1      = 0;
  x2 = y2      = 0;
  bottom_layer = NULL;

  parent = picman_layer_get_parent (merge_list->data);

  /*  Get the layer extents  */
  count = 0;
  while (merge_list)
    {
      layer = merge_list->data;

      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      switch (merge_type)
        {
        case PICMAN_EXPAND_AS_NECESSARY:
        case PICMAN_CLIP_TO_IMAGE:
          if (! count)
            {
              x1 = off_x;
              y1 = off_y;
              x2 = off_x + picman_item_get_width  (PICMAN_ITEM (layer));
              y2 = off_y + picman_item_get_height (PICMAN_ITEM (layer));
            }
          else
            {
              if (off_x < x1)
                x1 = off_x;
              if (off_y < y1)
                y1 = off_y;
              if ((off_x + picman_item_get_width (PICMAN_ITEM (layer))) > x2)
                x2 = (off_x + picman_item_get_width (PICMAN_ITEM (layer)));
              if ((off_y + picman_item_get_height (PICMAN_ITEM (layer))) > y2)
                y2 = (off_y + picman_item_get_height (PICMAN_ITEM (layer)));
            }

          if (merge_type == PICMAN_CLIP_TO_IMAGE)
            {
              x1 = CLAMP (x1, 0, picman_image_get_width  (image));
              y1 = CLAMP (y1, 0, picman_image_get_height (image));
              x2 = CLAMP (x2, 0, picman_image_get_width  (image));
              y2 = CLAMP (y2, 0, picman_image_get_height (image));
            }
          break;

        case PICMAN_CLIP_TO_BOTTOM_LAYER:
          if (merge_list->next == NULL)
            {
              x1 = off_x;
              y1 = off_y;
              x2 = off_x + picman_item_get_width  (PICMAN_ITEM (layer));
              y2 = off_y + picman_item_get_height (PICMAN_ITEM (layer));
            }
          break;

        case PICMAN_FLATTEN_IMAGE:
          if (merge_list->next == NULL)
            {
              x1 = 0;
              y1 = 0;
              x2 = picman_image_get_width  (image);
              y2 = picman_image_get_height (image);
            }
          break;
        }

      count ++;
      reverse_list = g_slist_prepend (reverse_list, layer);
      merge_list = g_slist_next (merge_list);
    }

  if ((x2 - x1) == 0 || (y2 - y1) == 0)
    return NULL;

  /*  Start a merge undo group. */

  name = g_strdup (picman_object_get_name (layer));

  if (merge_type == PICMAN_FLATTEN_IMAGE ||
      (picman_drawable_is_indexed (PICMAN_DRAWABLE (layer)) &&
       ! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer))))
    {
      GeglColor *color;
      PicmanRGB    bg;

      merge_layer = picman_layer_new (image, (x2 - x1), (y2 - y1),
                                    picman_image_get_layer_format (image, FALSE),
                                    picman_object_get_name (layer),
                                    PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);
      if (! merge_layer)
        {
          g_warning ("%s: could not allocate merge layer.", G_STRFUNC);
          return NULL;
        }

      picman_item_set_offset (PICMAN_ITEM (merge_layer), x1, y1);

      /*  get the background for compositing  */
      picman_context_get_background (context, &bg);

      color = picman_gegl_color_new (&bg);
      gegl_buffer_set_color (picman_drawable_get_buffer (PICMAN_DRAWABLE (merge_layer)),
                             GEGL_RECTANGLE(0,0,x2-x1,y2-y1), color);
      g_object_unref (color);

      position = 0;
    }
  else
    {
      /*  The final merged layer inherits the name of the bottom most layer
       *  and the resulting layer has an alpha channel whether or not the
       *  original did. Opacity is set to 100% and the MODE is set to normal.
       */

      merge_layer =
        picman_layer_new (image, (x2 - x1), (y2 - y1),
                        picman_drawable_get_format_with_alpha (PICMAN_DRAWABLE (layer)),
                        "merged layer",
                        PICMAN_OPACITY_OPAQUE, PICMAN_NORMAL_MODE);

      if (!merge_layer)
        {
          g_warning ("%s: could not allocate merge layer", G_STRFUNC);
          return NULL;
        }

      picman_item_set_offset (PICMAN_ITEM (merge_layer), x1, y1);

      /*  clear the layer  */
      gegl_buffer_clear (picman_drawable_get_buffer (PICMAN_DRAWABLE (merge_layer)),
                         NULL);

      /*  Find the index in the layer list of the bottom layer--we need this
       *  in order to add the final, merged layer to the layer list correctly
       */
      layer = reverse_list->data;
      position =
        picman_container_get_n_children (container) -
        picman_container_get_child_index (container, PICMAN_OBJECT (layer));
    }

  bottom_layer = layer;

  /* Copy the tattoo and parasites of the bottom layer to the new layer */
  picman_item_set_tattoo (PICMAN_ITEM (merge_layer),
                        picman_item_get_tattoo (PICMAN_ITEM (bottom_layer)));

  parasites = picman_item_get_parasites (PICMAN_ITEM (bottom_layer));
  parasites = picman_parasite_list_copy (parasites);
  picman_item_set_parasites (PICMAN_ITEM (merge_layer), parasites);
  g_object_unref (parasites);

  for (layers = reverse_list; layers; layers = g_slist_next (layers))
    {
      GeglBuffer           *merge_buffer;
      GeglBuffer           *layer_buffer;
      PicmanApplicator       *applicator;
      PicmanLayerModeEffects  mode;

      layer = layers->data;

      picman_item_get_offset (PICMAN_ITEM (layer), &off_x, &off_y);

      /* DISSOLVE_MODE is special since it is the only mode that does not
       *  work on the projection with the lower layer, but only locally on
       *  the layers alpha channel.
       */
      mode = picman_layer_get_mode (layer);
      if (layer == bottom_layer && mode != PICMAN_DISSOLVE_MODE)
        mode = PICMAN_NORMAL_MODE;

      merge_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (merge_layer));
      layer_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer));

      applicator =
        picman_applicator_new (NULL,
                             picman_drawable_get_linear (PICMAN_DRAWABLE (layer)));

      if (picman_layer_get_mask (layer) &&
          picman_layer_get_apply_mask (layer))
        {
          GeglBuffer *mask_buffer;

          mask_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer->mask));

          picman_applicator_set_mask_buffer (applicator, mask_buffer);
          picman_applicator_set_mask_offset (applicator,
                                           - (x1 - off_x),
                                           - (y1 - off_y));
        }

      picman_applicator_set_src_buffer (applicator, merge_buffer);
      picman_applicator_set_dest_buffer (applicator, merge_buffer);

      picman_applicator_set_apply_buffer (applicator, layer_buffer);
      picman_applicator_set_apply_offset (applicator,
                                        - (x1 - off_x),
                                        - (y1 - off_y));

      picman_applicator_set_mode (applicator,
                                picman_layer_get_opacity (layer),
                                mode);

      picman_applicator_blit (applicator,
                            GEGL_RECTANGLE (0, 0,
                                            gegl_buffer_get_width  (merge_buffer),
                                            gegl_buffer_get_height (merge_buffer)));

      g_object_unref (applicator);

      picman_image_remove_layer (image, layer, TRUE, NULL);
    }

  g_slist_free (reverse_list);

  picman_object_take_name (PICMAN_OBJECT (merge_layer), name);
  picman_item_set_visible (PICMAN_ITEM (merge_layer), TRUE, FALSE);

  /*  if the type is flatten, remove all the remaining layers  */
  if (merge_type == PICMAN_FLATTEN_IMAGE)
    {
      list = picman_image_get_layer_iter (image);
      while (list)
        {
          layer = list->data;

          list = g_list_next (list);
          picman_image_remove_layer (image, layer, TRUE, NULL);
        }

      picman_image_add_layer (image, merge_layer, parent,
                            position, TRUE);
    }
  else
    {
      /*  Add the layer to the image  */

      picman_image_add_layer (image, merge_layer, parent,
                            picman_container_get_n_children (container) -
                            position + 1,
                            TRUE);
    }

  picman_drawable_update (PICMAN_DRAWABLE (merge_layer),
                        0, 0,
                        picman_item_get_width  (PICMAN_ITEM (merge_layer)),
                        picman_item_get_height (PICMAN_ITEM (merge_layer)));

  return merge_layer;
}
