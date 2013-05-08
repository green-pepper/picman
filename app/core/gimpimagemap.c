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

/* This file contains the code necessary for generating on canvas
 * previews, either by connecting a function to process the pixels or
 * by connecting a specified GEGL operation to do the processing. It
 * keeps an undo buffer to allow direct modification of the pixel data
 * (so that it will show up in the projection) and it will restore the
 * source in case the mapping procedure was cancelled.
 *
 * To create a tool that uses this, see /tools/picmanimagemaptool.c for
 * the interface and /tools/picmancolorbalancetool.c for an example of
 * using that interface.
 *
 * Note that when talking about on canvas preview, we are speaking
 * about non destructive image editing where the operation is previewd
 * before being applied.
 */

#include "config.h"

#include <gegl.h>

#include "core-types.h"

#include "gegl/picmanapplicator.h"
#include "gegl/picman-gegl-utils.h"

#include "picmandrawable.h"
#include "picmandrawable-filter.h"
#include "picmanfilter.h"
#include "picmanimage.h"
#include "picmanimagemap.h"
#include "picmanmarshal.h"
#include "picmanpickable.h"
#include "picmanviewable.h"
#include "picmanchannel.h"
#include "picmanprogress.h"


enum
{
  FLUSH,
  LAST_SIGNAL
};


struct _PicmanImageMap
{
  PicmanObject      parent_instance;

  PicmanDrawable   *drawable;
  gchar          *undo_desc;
  GeglNode       *operation;
  gchar          *stock_id;

  PicmanFilter     *filter;
  GeglNode       *translate;
  PicmanApplicator *applicator;
};


static void   picman_image_map_pickable_iface_init (PicmanPickableInterface *iface);

static void            picman_image_map_dispose         (GObject             *object);
static void            picman_image_map_finalize        (GObject             *object);

static PicmanImage     * picman_image_map_get_image       (PicmanPickable        *pickable);
static const Babl    * picman_image_map_get_format      (PicmanPickable        *pickable);
static const Babl    * picman_image_map_get_format_with_alpha
                                                      (PicmanPickable        *pickable);
static GeglBuffer    * picman_image_map_get_buffer      (PicmanPickable        *pickable);
static gboolean        picman_image_map_get_pixel_at    (PicmanPickable        *pickable,
                                                       gint                 x,
                                                       gint                 y,
                                                       const Babl          *format,
                                                       gpointer             pixel);


G_DEFINE_TYPE_WITH_CODE (PicmanImageMap, picman_image_map, PICMAN_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_image_map_pickable_iface_init))

#define parent_class picman_image_map_parent_class

static guint image_map_signals[LAST_SIGNAL] = { 0 };


static void
picman_image_map_class_init (PicmanImageMapClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  image_map_signals[FLUSH] =
    g_signal_new ("flush",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanImageMapClass, flush),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose  = picman_image_map_dispose;
  object_class->finalize = picman_image_map_finalize;
}

static void
picman_image_map_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->get_image             = picman_image_map_get_image;
  iface->get_format            = picman_image_map_get_format;
  iface->get_format_with_alpha = picman_image_map_get_format_with_alpha;
  iface->get_buffer            = picman_image_map_get_buffer;
  iface->get_pixel_at          = picman_image_map_get_pixel_at;
}

static void
picman_image_map_init (PicmanImageMap *image_map)
{
}

static void
picman_image_map_dispose (GObject *object)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (object);

  if (image_map->drawable)
    picman_viewable_preview_thaw (PICMAN_VIEWABLE (image_map->drawable));

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_image_map_finalize (GObject *object)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (object);

  if (image_map->undo_desc)
    {
      g_free (image_map->undo_desc);
      image_map->undo_desc = NULL;
    }

  if (image_map->operation)
    {
      g_object_unref (image_map->operation);
      image_map->operation = NULL;
    }

  if (image_map->stock_id)
    {
      g_free (image_map->stock_id);
      image_map->stock_id = NULL;
    }

  if (image_map->filter)
    {
      g_object_unref (image_map->filter);
      image_map->filter = NULL;
    }

  if (image_map->applicator)
    {
      g_object_unref (image_map->applicator);
      image_map->applicator = NULL;
    }

  if (image_map->drawable)
    {
      g_object_unref (image_map->drawable);
      image_map->drawable = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static PicmanImage *
picman_image_map_get_image (PicmanPickable *pickable)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (pickable);

  return picman_pickable_get_image (PICMAN_PICKABLE (image_map->drawable));
}

static const Babl *
picman_image_map_get_format (PicmanPickable *pickable)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (pickable);

  return picman_pickable_get_format (PICMAN_PICKABLE (image_map->drawable));
}

static const Babl *
picman_image_map_get_format_with_alpha (PicmanPickable *pickable)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (pickable);

  return picman_pickable_get_format_with_alpha (PICMAN_PICKABLE (image_map->drawable));
}

static GeglBuffer *
picman_image_map_get_buffer (PicmanPickable *pickable)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (pickable);

  return picman_pickable_get_buffer (PICMAN_PICKABLE (image_map->drawable));
}

static gboolean
picman_image_map_get_pixel_at (PicmanPickable *pickable,
                             gint          x,
                             gint          y,
                             const Babl   *format,
                             gpointer      pixel)
{
  PicmanImageMap *image_map = PICMAN_IMAGE_MAP (pickable);

  return picman_pickable_get_pixel_at (PICMAN_PICKABLE (image_map->drawable),
                                     x, y, format, pixel);
}

PicmanImageMap *
picman_image_map_new (PicmanDrawable *drawable,
                    const gchar  *undo_desc,
                    GeglNode     *operation,
                    const gchar  *stock_id)
{
  PicmanImageMap *image_map;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (GEGL_IS_NODE (operation), NULL);

  image_map = g_object_new (PICMAN_TYPE_IMAGE_MAP, NULL);

  image_map->drawable  = g_object_ref (drawable);
  image_map->undo_desc = g_strdup (undo_desc);

  image_map->operation = g_object_ref (operation);
  image_map->stock_id  = g_strdup (stock_id);

  picman_viewable_preview_freeze (PICMAN_VIEWABLE (drawable));

  return image_map;
}

void
picman_image_map_apply (PicmanImageMap *image_map)
{
  PicmanImage         *image;
  PicmanChannel       *mask;
  GeglRectangle      rect;
  PicmanComponentMask  active_mask;

  g_return_if_fail (PICMAN_IS_IMAGE_MAP (image_map));

  /*  Make sure the drawable is still valid  */
  if (! picman_item_is_attached (PICMAN_ITEM (image_map->drawable)))
    return;

  /*  The application should occur only within selection bounds  */
  if (! picman_item_mask_intersect (PICMAN_ITEM (image_map->drawable),
                                  &rect.x, &rect.y,
                                  &rect.width, &rect.height))
    return;

  if (! image_map->filter)
    {
      GeglNode *filter_node;
      GeglNode *filter_output;
      GeglNode *input;

      image_map->filter = picman_filter_new (image_map->undo_desc);
      picman_viewable_set_stock_id (PICMAN_VIEWABLE (image_map->filter),
                                  image_map->stock_id);

      filter_node = picman_filter_get_node (image_map->filter);

      gegl_node_add_child (filter_node, image_map->operation);

      image_map->applicator =
        picman_applicator_new (filter_node,
                             picman_drawable_get_linear (image_map->drawable));

      picman_filter_set_applicator (image_map->filter,
                                  image_map->applicator);

      image_map->translate = gegl_node_new_child (filter_node,
                                                  "operation", "gegl:translate",
                                                  NULL);

      input  = gegl_node_get_input_proxy  (filter_node, "input");

      if (gegl_node_has_pad (image_map->operation, "input") &&
          gegl_node_has_pad (image_map->operation, "output"))
        {
          /*  if there are input and output pads we probably have a
           *  filter OP, connect it on both ends.
           */
          gegl_node_link_many (input,
                               image_map->translate,
                               image_map->operation,
                               NULL);

          filter_output = image_map->operation;
        }
      else if (gegl_node_has_pad (image_map->operation, "output"))
        {
          /*  if there is only an output pad we probably have a
           *  source OP, blend its result on top of the original
           *  pixels.
           */
          GeglNode *over = gegl_node_new_child (filter_node,
                                                "operation", "gegl:over",
                                                NULL);

          gegl_node_link_many (input,
                               image_map->translate,
                               over,
                               NULL);

          gegl_node_connect_to (image_map->operation, "output",
                                over,                 "aux");

          filter_output = over;
        }
      else
        {
          /* otherwise we just construct a silly nop pipleline
           */
          gegl_node_link_many (input,
                               image_map->translate,
                               NULL);

          filter_output = image_map->translate;
        }

      gegl_node_connect_to (filter_output, "output",
                            filter_node,   "aux");

      picman_applicator_set_mode (image_map->applicator,
                                PICMAN_OPACITY_OPAQUE,
                                PICMAN_REPLACE_MODE);
    }

  if (! picman_drawable_has_filter (image_map->drawable, image_map->filter))
    picman_drawable_add_filter (image_map->drawable, image_map->filter);

  gegl_node_set (image_map->translate,
                 "x", (gdouble) -rect.x,
                 "y", (gdouble) -rect.y,
                 NULL);

  picman_applicator_set_apply_offset (image_map->applicator,
                                    rect.x, rect.y);

  active_mask = picman_drawable_get_active_mask (image_map->drawable);

  /*  don't let the filter affect the drawable projection's alpha,
   *  because it can't affect the drawable buffer's alpha either
   *  when finally merged (see bug #699279)
   */
  if (! picman_drawable_has_alpha (image_map->drawable))
    active_mask &= ~PICMAN_COMPONENT_ALPHA;

  picman_applicator_set_affect (image_map->applicator, active_mask);

  image = picman_item_get_image (PICMAN_ITEM (image_map->drawable));
  mask  = picman_image_get_mask (image);

  if (picman_channel_is_empty (mask))
    {
      picman_applicator_set_mask_buffer (image_map->applicator, NULL);
    }
  else
    {
      GeglBuffer *mask_buffer;
      gint        offset_x, offset_y;

      mask_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));
      picman_item_get_offset (PICMAN_ITEM (image_map->drawable),
                            &offset_x, &offset_y);

      picman_applicator_set_mask_buffer (image_map->applicator, mask_buffer);
      picman_applicator_set_mask_offset (image_map->applicator,
                                       offset_x, offset_y);
    }

  picman_drawable_update (image_map->drawable,
                        rect.x, rect.y,
                        rect.width, rect.height);

  g_signal_emit (image_map, image_map_signals[FLUSH], 0);
}

void
picman_image_map_commit (PicmanImageMap *image_map,
                       PicmanProgress *progress)
{
  g_return_if_fail (PICMAN_IS_IMAGE_MAP (image_map));
  g_return_if_fail (progress == NULL || PICMAN_IS_PROGRESS (progress));

  if (picman_drawable_has_filter (image_map->drawable, image_map->filter))
    {
      picman_drawable_remove_filter (image_map->drawable, image_map->filter);

      picman_drawable_merge_filter (image_map->drawable, image_map->filter,
                                  progress,
                                  image_map->undo_desc);

      g_signal_emit (image_map, image_map_signals[FLUSH], 0);
    }
}

void
picman_image_map_abort (PicmanImageMap *image_map)
{
  g_return_if_fail (PICMAN_IS_IMAGE_MAP (image_map));

  if (picman_drawable_has_filter (image_map->drawable, image_map->filter))
    {
      GeglRectangle rect;

      picman_drawable_remove_filter (image_map->drawable, image_map->filter);

      if (picman_item_is_attached (PICMAN_ITEM (image_map->drawable)) &&
          picman_item_mask_intersect (PICMAN_ITEM (image_map->drawable),
                                    &rect.x, &rect.y,
                                    &rect.width, &rect.height))
        {
          picman_drawable_update (image_map->drawable,
                                rect.x, rect.y,
                                rect.width, rect.height);

          g_signal_emit (image_map, image_map_signals[FLUSH], 0);
        }
    }
}
