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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "paint/picmanpaintcore-stroke.h"
#include "paint/picmanpaintoptions.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-mask.h"
#include "gegl/picman-gegl-utils.h"

#include "picman.h"
#include "picman-utils.h"
#include "picmanboundary.h"
#include "picmancontainer.h"
#include "picmanerror.h"
#include "picmanimage.h"
#include "picmanimage-quick-mask.h"
#include "picmanimage-undo.h"
#include "picmanimage-undo-push.h"
#include "picmanchannel.h"
#include "picmanchannel-select.h"
#include "picmancontext.h"
#include "picmandrawable-stroke.h"
#include "picmanmarshal.h"
#include "picmanpaintinfo.h"
#include "picmanpickable.h"
#include "picmanstrokeoptions.h"

#include "picman-intl.h"


enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};


static void picman_channel_pickable_iface_init (PicmanPickableInterface *iface);

static void       picman_channel_finalize      (GObject           *object);

static gint64     picman_channel_get_memsize   (PicmanObject        *object,
                                              gint64            *gui_size);

static gchar  * picman_channel_get_description (PicmanViewable      *viewable,
                                              gchar            **tooltip);

static GeglNode * picman_channel_get_node      (PicmanFilter        *filter);

static gboolean   picman_channel_is_attached   (const PicmanItem    *item);
static PicmanItemTree * picman_channel_get_tree  (PicmanItem          *item);
static PicmanItem * picman_channel_duplicate     (PicmanItem          *item,
                                              GType              new_type);
static void       picman_channel_convert       (PicmanItem          *item,
                                              PicmanImage         *dest_image);
static void       picman_channel_translate     (PicmanItem          *item,
                                              gint               off_x,
                                              gint               off_y,
                                              gboolean           push_undo);
static void       picman_channel_scale         (PicmanItem          *item,
                                              gint               new_width,
                                              gint               new_height,
                                              gint               new_offset_x,
                                              gint               new_offset_y,
                                              PicmanInterpolationType interp_type,
                                              PicmanProgress      *progress);
static void       picman_channel_resize        (PicmanItem          *item,
                                              PicmanContext       *context,
                                              gint               new_width,
                                              gint               new_height,
                                              gint               offx,
                                              gint               offy);
static void       picman_channel_flip          (PicmanItem          *item,
                                              PicmanContext       *context,
                                              PicmanOrientationType flip_type,
                                              gdouble            axis,
                                              gboolean           flip_result);
static void       picman_channel_rotate        (PicmanItem          *item,
                                              PicmanContext       *context,
                                              PicmanRotationType   flip_type,
                                              gdouble            center_x,
                                              gdouble            center_y,
                                              gboolean           flip_result);
static void       picman_channel_transform     (PicmanItem          *item,
                                              PicmanContext       *context,
                                              const PicmanMatrix3 *matrix,
                                              PicmanTransformDirection direction,
                                              PicmanInterpolationType interpolation_type,
                                              gint               recursion_level,
                                              PicmanTransformResize clip_result,
                                              PicmanProgress      *progress);
static gboolean   picman_channel_stroke        (PicmanItem          *item,
                                              PicmanDrawable      *drawable,
                                              PicmanStrokeOptions *stroke_options,
                                              gboolean           push_undo,
                                              PicmanProgress      *progress,
                                              GError           **error);
static void       picman_channel_to_selection  (PicmanItem          *item,
                                              PicmanChannelOps     op,
                                              gboolean           antialias,
                                              gboolean           feather,
                                              gdouble            feather_radius_x,
                                              gdouble            feather_radius_y);

static void       picman_channel_convert_type  (PicmanDrawable      *drawable,
                                              PicmanImage         *dest_image,
                                              const Babl        *new_format,
                                              PicmanImageBaseType  new_base_type,
                                              PicmanPrecision      new_precision,
                                              gint               layer_dither_type,
                                              gint               mask_dither_type,
                                              gboolean           push_undo);
static void picman_channel_invalidate_boundary   (PicmanDrawable       *drawable);
static void picman_channel_get_active_components (const PicmanDrawable *drawable,
                                                gboolean           *active);
static PicmanComponentMask
                  picman_channel_get_active_mask (const PicmanDrawable *drawable);

static void      picman_channel_apply_buffer   (PicmanDrawable        *drawable,
                                              GeglBuffer          *buffer,
                                              const GeglRectangle *buffer_region,
                                              gboolean             push_undo,
                                              const gchar         *undo_desc,
                                              gdouble              opacity,
                                              PicmanLayerModeEffects  mode,
                                              GeglBuffer          *base_buffer,
                                              gint                 base_x,
                                              gint                 base_y);
static void      picman_channel_replace_buffer (PicmanDrawable        *drawable,
                                              GeglBuffer          *buffer,
                                              const GeglRectangle *buffer_region,
                                              gboolean             push_undo,
                                              const gchar         *undo_desc,
                                              gdouble              opacity,
                                              GeglBuffer          *mask,
                                              const GeglRectangle *mask_region,
                                              gint                 x,
                                              gint                 y);
static void      picman_channel_set_buffer     (PicmanDrawable        *drawable,
                                              gboolean             push_undo,
                                              const gchar         *undo_desc,
                                              GeglBuffer          *buffer,
                                              gint                 offset_x,
                                              gint                 offset_y);
static void      picman_channel_swap_pixels    (PicmanDrawable        *drawable,
                                              GeglBuffer          *buffer,
                                              gint                 x,
                                              gint                 y);

static gdouble   picman_channel_get_opacity_at (PicmanPickable        *pickable,
                                              gint                 x,
                                              gint                 y);

static gboolean   picman_channel_real_boundary (PicmanChannel         *channel,
                                              const PicmanBoundSeg **segs_in,
                                              const PicmanBoundSeg **segs_out,
                                              gint                *num_segs_in,
                                              gint                *num_segs_out,
                                              gint                 x1,
                                              gint                 y1,
                                              gint                 x2,
                                              gint                 y2);
static gboolean   picman_channel_real_bounds   (PicmanChannel         *channel,
                                              gint                *x1,
                                              gint                *y1,
                                              gint                *x2,
                                              gint                *y2);
static gboolean   picman_channel_real_is_empty (PicmanChannel         *channel);
static void       picman_channel_real_feather  (PicmanChannel         *channel,
                                              gdouble              radius_x,
                                              gdouble              radius_y,
                                              gboolean             push_undo);
static void       picman_channel_real_sharpen  (PicmanChannel         *channel,
                                              gboolean             push_undo);
static void       picman_channel_real_clear    (PicmanChannel         *channel,
                                              const gchar         *undo_desc,
                                              gboolean             push_undo);
static void       picman_channel_real_all      (PicmanChannel         *channel,
                                              gboolean             push_undo);
static void       picman_channel_real_invert   (PicmanChannel         *channel,
                                              gboolean             push_undo);
static void       picman_channel_real_border   (PicmanChannel         *channel,
                                              gint                 radius_x,
                                              gint                 radius_y,
                                              gboolean             feather,
                                              gboolean             edge_lock,
                                              gboolean             push_undo);
static void       picman_channel_real_grow     (PicmanChannel         *channel,
                                              gint                 radius_x,
                                              gint                 radius_y,
                                              gboolean             push_undo);
static void       picman_channel_real_shrink   (PicmanChannel         *channel,
                                              gint                 radius_x,
                                              gint                 radius_y,
                                              gboolean             edge_lock,
                                              gboolean             push_undo);


G_DEFINE_TYPE_WITH_CODE (PicmanChannel, picman_channel, PICMAN_TYPE_DRAWABLE,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_channel_pickable_iface_init))

#define parent_class picman_channel_parent_class

static guint channel_signals[LAST_SIGNAL] = { 0 };


static void
picman_channel_class_init (PicmanChannelClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanFilterClass   *filter_class      = PICMAN_FILTER_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class    = PICMAN_DRAWABLE_CLASS (klass);

  channel_signals[COLOR_CHANGED] =
    g_signal_new ("color-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanChannelClass, color_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize           = picman_channel_finalize;

  picman_object_class->get_memsize   = picman_channel_get_memsize;

  viewable_class->get_description  = picman_channel_get_description;
  viewable_class->default_stock_id = "picman-channel";

  filter_class->get_node           = picman_channel_get_node;

  item_class->is_attached          = picman_channel_is_attached;
  item_class->get_tree             = picman_channel_get_tree;
  item_class->duplicate            = picman_channel_duplicate;
  item_class->convert              = picman_channel_convert;
  item_class->translate            = picman_channel_translate;
  item_class->scale                = picman_channel_scale;
  item_class->resize               = picman_channel_resize;
  item_class->flip                 = picman_channel_flip;
  item_class->rotate               = picman_channel_rotate;
  item_class->transform            = picman_channel_transform;
  item_class->stroke               = picman_channel_stroke;
  item_class->to_selection         = picman_channel_to_selection;
  item_class->default_name         = _("Channel");
  item_class->rename_desc          = C_("undo-type", "Rename Channel");
  item_class->translate_desc       = C_("undo-type", "Move Channel");
  item_class->scale_desc           = C_("undo-type", "Scale Channel");
  item_class->resize_desc          = C_("undo-type", "Resize Channel");
  item_class->flip_desc            = C_("undo-type", "Flip Channel");
  item_class->rotate_desc          = C_("undo-type", "Rotate Channel");
  item_class->transform_desc       = C_("undo-type", "Transform Channel");
  item_class->stroke_desc          = C_("undo-type", "Stroke Channel");
  item_class->to_selection_desc    = C_("undo-type", "Channel to Selection");
  item_class->reorder_desc         = C_("undo-type", "Reorder Channel");
  item_class->raise_desc           = C_("undo-type", "Raise Channel");
  item_class->raise_to_top_desc    = C_("undo-type", "Raise Channel to Top");
  item_class->lower_desc           = C_("undo-type", "Lower Channel");
  item_class->lower_to_bottom_desc = C_("undo-type", "Lower Channel to Bottom");
  item_class->raise_failed         = _("Channel cannot be raised higher.");
  item_class->lower_failed         = _("Channel cannot be lowered more.");

  drawable_class->convert_type          = picman_channel_convert_type;
  drawable_class->invalidate_boundary   = picman_channel_invalidate_boundary;
  drawable_class->get_active_components = picman_channel_get_active_components;
  drawable_class->get_active_mask       = picman_channel_get_active_mask;
  drawable_class->apply_buffer          = picman_channel_apply_buffer;
  drawable_class->replace_buffer        = picman_channel_replace_buffer;
  drawable_class->set_buffer            = picman_channel_set_buffer;
  drawable_class->swap_pixels           = picman_channel_swap_pixels;

  klass->boundary       = picman_channel_real_boundary;
  klass->bounds         = picman_channel_real_bounds;
  klass->is_empty       = picman_channel_real_is_empty;
  klass->feather        = picman_channel_real_feather;
  klass->sharpen        = picman_channel_real_sharpen;
  klass->clear          = picman_channel_real_clear;
  klass->all            = picman_channel_real_all;
  klass->invert         = picman_channel_real_invert;
  klass->border         = picman_channel_real_border;
  klass->grow           = picman_channel_real_grow;
  klass->shrink         = picman_channel_real_shrink;

  klass->feather_desc   = C_("undo-type", "Feather Channel");
  klass->sharpen_desc   = C_("undo-type", "Sharpen Channel");
  klass->clear_desc     = C_("undo-type", "Clear Channel");
  klass->all_desc       = C_("undo-type", "Fill Channel");
  klass->invert_desc    = C_("undo-type", "Invert Channel");
  klass->border_desc    = C_("undo-type", "Border Channel");
  klass->grow_desc      = C_("undo-type", "Grow Channel");
  klass->shrink_desc    = C_("undo-type", "Shrink Channel");
}

static void
picman_channel_init (PicmanChannel *channel)
{
  picman_rgba_set (&channel->color, 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE);

  channel->show_masked    = FALSE;

  /*  Selection mask variables  */
  channel->boundary_known = FALSE;
  channel->segs_in        = NULL;
  channel->segs_out       = NULL;
  channel->num_segs_in    = 0;
  channel->num_segs_out   = 0;
  channel->empty          = FALSE;
  channel->bounds_known   = FALSE;
  channel->x1             = 0;
  channel->y1             = 0;
  channel->x2             = 0;
  channel->y2             = 0;
}

static void
picman_channel_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->get_opacity_at = picman_channel_get_opacity_at;
}

static void
picman_channel_finalize (GObject *object)
{
  PicmanChannel *channel = PICMAN_CHANNEL (object);

  if (channel->segs_in)
    {
      g_free (channel->segs_in);
      channel->segs_in = NULL;
    }

  if (channel->segs_out)
    {
      g_free (channel->segs_out);
      channel->segs_out = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_channel_get_memsize (PicmanObject *object,
                          gint64     *gui_size)
{
  PicmanChannel *channel = PICMAN_CHANNEL (object);

  *gui_size += channel->num_segs_in  * sizeof (PicmanBoundSeg);
  *gui_size += channel->num_segs_out * sizeof (PicmanBoundSeg);

  return PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object, gui_size);
}

static gchar *
picman_channel_get_description (PicmanViewable  *viewable,
                              gchar        **tooltip)
{
  if (! strcmp (PICMAN_IMAGE_QUICK_MASK_NAME,
                picman_object_get_name (viewable)))
    {
      return g_strdup (_("Quick Mask"));
    }

  return PICMAN_VIEWABLE_CLASS (parent_class)->get_description (viewable,
                                                              tooltip);
}

static GeglNode *
picman_channel_get_node (PicmanFilter *filter)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (filter);
  PicmanChannel  *channel  = PICMAN_CHANNEL (filter);
  GeglNode     *node;
  GeglNode     *source;
  GeglNode     *mode_node;
  GeglColor    *color;

  node = PICMAN_FILTER_CLASS (parent_class)->get_node (filter);

  source = picman_drawable_get_source_node (drawable);
  gegl_node_add_child (node, source);

  color = picman_gegl_color_new (&channel->color);

  g_warn_if_fail (channel->color_node == NULL);

  channel->color_node = gegl_node_new_child (node,
                                             "operation", "gegl:color",
                                             "value",     color,
                                             NULL);

  g_object_unref (color);

  g_warn_if_fail (channel->mask_node == NULL);

  channel->mask_node = gegl_node_new_child (node,
                                            "operation", "gegl:opacity",
                                            NULL);
  gegl_node_connect_to (channel->color_node, "output",
                        channel->mask_node,  "input");

  g_warn_if_fail (channel->invert_node == NULL);

  channel->invert_node = gegl_node_new_child (node,
                                              "operation", "gegl:invert",
                                              NULL);

  if (channel->show_masked)
    {
      gegl_node_connect_to (source,               "output",
                            channel->invert_node, "input");
      gegl_node_connect_to (channel->invert_node, "output",
                            channel->mask_node,   "aux");
    }
  else
    {
      gegl_node_connect_to (source,             "output",
                            channel->mask_node, "aux");
    }

  mode_node = picman_drawable_get_mode_node (drawable);

  gegl_node_connect_to (channel->mask_node, "output",
                        mode_node,          "aux");

  return node;
}

static gboolean
picman_channel_is_attached (const PicmanItem *item)
{
  PicmanImage *image = picman_item_get_image (item);

  return (PICMAN_IS_IMAGE (image) &&
          picman_container_have (picman_image_get_channels (image),
                               PICMAN_OBJECT (item)));
}

static PicmanItemTree *
picman_channel_get_tree (PicmanItem *item)
{
  if (picman_item_is_attached (item))
    {
      PicmanImage *image = picman_item_get_image (item);

      return picman_image_get_channel_tree (image);
    }

  return NULL;
}

static PicmanItem *
picman_channel_duplicate (PicmanItem *item,
                        GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_CHANNEL (new_item))
    {
      PicmanChannel *channel     = PICMAN_CHANNEL (item);
      PicmanChannel *new_channel = PICMAN_CHANNEL (new_item);

      new_channel->color        = channel->color;
      new_channel->show_masked  = channel->show_masked;

      /*  selection mask variables  */
      new_channel->bounds_known = channel->bounds_known;
      new_channel->empty        = channel->empty;
      new_channel->x1           = channel->x1;
      new_channel->y1           = channel->y1;
      new_channel->x2           = channel->x2;
      new_channel->y2           = channel->y2;
    }

  return new_item;
}

static void
picman_channel_convert (PicmanItem  *item,
                      PicmanImage *dest_image)
{
  PicmanChannel  *channel  = PICMAN_CHANNEL (item);
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);

  if (! picman_drawable_is_gray (drawable))
    {
      picman_drawable_convert_type (drawable, dest_image, PICMAN_GRAY,
                                  picman_image_get_precision (dest_image),
                                  0, 0,
                                  FALSE);
    }

  if (picman_drawable_has_alpha (drawable))
    {
      GeglBuffer *new_buffer;
      const Babl *format;
      PicmanRGB     background;

      format = picman_drawable_get_format_without_alpha (drawable);

      new_buffer =
        gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                         picman_item_get_width (item),
                                         picman_item_get_height (item)),
                         format);

      picman_rgba_set (&background, 0.0, 0.0, 0.0, 0.0);

      picman_gegl_apply_flatten (picman_drawable_get_buffer (drawable),
                               NULL, NULL,
                               new_buffer, &background);

      picman_drawable_set_buffer_full (drawable, FALSE, NULL,
                                     new_buffer,
                                     picman_item_get_offset_x (item),
                                     picman_item_get_offset_y (item));
      g_object_unref (new_buffer);
    }

  if (G_TYPE_FROM_INSTANCE (channel) == PICMAN_TYPE_CHANNEL)
    {
      gint width  = picman_image_get_width  (dest_image);
      gint height = picman_image_get_height (dest_image);

      picman_item_set_offset (item, 0, 0);

      if (picman_item_get_width  (item) != width ||
          picman_item_get_height (item) != height)
        {
          picman_item_resize (item, picman_get_user_context (dest_image->picman),
                            width, height, 0, 0);
        }
    }

  PICMAN_ITEM_CLASS (parent_class)->convert (item, dest_image);
}

static void
picman_channel_translate (PicmanItem *item,
                        gint      off_x,
                        gint      off_y,
                        gboolean  push_undo)
{
  PicmanChannel   *channel    = PICMAN_CHANNEL (item);
  PicmanChannel   *tmp_mask   = NULL;
  GeglBuffer    *tmp_buffer = NULL;
  gint           width, height;
  gint           x1, y1, x2, y2;

  picman_channel_bounds (channel, &x1, &y1, &x2, &y2);

  /*  update the old area  */
  picman_drawable_update (PICMAN_DRAWABLE (item), x1, y1, x2 - x1, y2 - y1);

  if (push_undo)
    picman_channel_push_undo (channel, NULL);
  else
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));

  x1 = CLAMP ((x1 + off_x), 0, picman_item_get_width  (PICMAN_ITEM (channel)));
  y1 = CLAMP ((y1 + off_y), 0, picman_item_get_height (PICMAN_ITEM (channel)));
  x2 = CLAMP ((x2 + off_x), 0, picman_item_get_width  (PICMAN_ITEM (channel)));
  y2 = CLAMP ((y2 + off_y), 0, picman_item_get_height (PICMAN_ITEM (channel)));

  width  = x2 - x1;
  height = y2 - y1;

  /*  make sure width and height are non-zero  */
  if (width != 0 && height != 0)
    {
      /*  copy the portion of the mask we will keep to a
       *  temporary buffer
       */
      tmp_mask = picman_channel_new_mask (picman_item_get_image (item),
                                        width, height);
      tmp_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (tmp_mask));

      gegl_buffer_copy (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                        GEGL_RECTANGLE (x1 - off_x, y1 - off_y, width, height),
                        tmp_buffer,
                        GEGL_RECTANGLE (0,0,0,0));
    }

  /*  clear the mask  */
  gegl_buffer_clear (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                     NULL);

  if (width != 0 && height != 0)
    {
      /*  copy the temp mask back to the mask  */

      gegl_buffer_copy (tmp_buffer,
                        NULL,
                        picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                        GEGL_RECTANGLE (x1, y1, 0, 0));

      /*  free the temporary mask  */
      g_object_unref (tmp_mask);
    }

  /*  calculate new bounds  */
  if (width == 0 || height == 0)
    {
      channel->empty = TRUE;
      channel->x1    = 0;
      channel->y1    = 0;
      channel->x2    = picman_item_get_width  (PICMAN_ITEM (channel));
      channel->y2    = picman_item_get_height (PICMAN_ITEM (channel));
    }
  else
    {
      channel->x1 = x1;
      channel->y1 = y1;
      channel->x2 = x2;
      channel->y2 = y2;
    }

  /*  update the new area  */
  picman_drawable_update (PICMAN_DRAWABLE (item),
                        channel->x1, channel->y1,
                        channel->x2 - channel->x1,
                        channel->y2 - channel->y1);
}

static void
picman_channel_scale (PicmanItem              *item,
                    gint                   new_width,
                    gint                   new_height,
                    gint                   new_offset_x,
                    gint                   new_offset_y,
                    PicmanInterpolationType  interpolation_type,
                    PicmanProgress          *progress)
{
  PicmanChannel *channel = PICMAN_CHANNEL (item);

  if (G_TYPE_FROM_INSTANCE (item) == PICMAN_TYPE_CHANNEL)
    {
      new_offset_x = 0;
      new_offset_y = 0;
    }

  /*  don't waste CPU cycles scaling an empty channel  */
  if (channel->bounds_known && channel->empty)
    {
      PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
      GeglBuffer   *new_buffer;

      new_buffer =
        gegl_buffer_new (GEGL_RECTANGLE (0, 0, new_width, new_height),
                         picman_drawable_get_format (drawable));

      picman_drawable_set_buffer_full (drawable,
                                     picman_item_is_attached (item), NULL,
                                     new_buffer,
                                     new_offset_x, new_offset_y);
      g_object_unref (new_buffer);

      picman_channel_clear (PICMAN_CHANNEL (item), NULL, FALSE);
    }
  else
    {
      PICMAN_ITEM_CLASS (parent_class)->scale (item, new_width, new_height,
                                             new_offset_x, new_offset_y,
                                             interpolation_type, progress);
    }
}

static void
picman_channel_resize (PicmanItem    *item,
                     PicmanContext *context,
                     gint         new_width,
                     gint         new_height,
                     gint         offset_x,
                     gint         offset_y)
{
  PICMAN_ITEM_CLASS (parent_class)->resize (item, context, new_width, new_height,
                                          offset_x, offset_y);

  if (G_TYPE_FROM_INSTANCE (item) == PICMAN_TYPE_CHANNEL)
    {
      picman_item_set_offset (item, 0, 0);
    }
}

static void
picman_channel_flip (PicmanItem            *item,
                   PicmanContext         *context,
                   PicmanOrientationType  flip_type,
                   gdouble              axis,
                   gboolean             clip_result)
{
  if (G_TYPE_FROM_INSTANCE (item) == PICMAN_TYPE_CHANNEL)
    clip_result = TRUE;

  PICMAN_ITEM_CLASS (parent_class)->flip (item, context, flip_type, axis,
                                        clip_result);
}

static void
picman_channel_rotate (PicmanItem         *item,
                     PicmanContext      *context,
                     PicmanRotationType  rotate_type,
                     gdouble           center_x,
                     gdouble           center_y,
                     gboolean          clip_result)
{
  /*  don't default to clip_result == TRUE here  */

  PICMAN_ITEM_CLASS (parent_class)->rotate (item, context,
                                          rotate_type, center_x, center_y,
                                          clip_result);
}

static void
picman_channel_transform (PicmanItem               *item,
                        PicmanContext            *context,
                        const PicmanMatrix3      *matrix,
                        PicmanTransformDirection  direction,
                        PicmanInterpolationType   interpolation_type,
                        gint                    recursion_level,
                        PicmanTransformResize     clip_result,
                        PicmanProgress           *progress)
{
  if (G_TYPE_FROM_INSTANCE (item) == PICMAN_TYPE_CHANNEL)
    clip_result = TRUE;

  PICMAN_ITEM_CLASS (parent_class)->transform (item, context, matrix, direction,
                                             interpolation_type,
                                             recursion_level,
                                             clip_result,
                                             progress);
}

static gboolean
picman_channel_stroke (PicmanItem           *item,
                     PicmanDrawable       *drawable,
                     PicmanStrokeOptions  *stroke_options,
                     gboolean            push_undo,
                     PicmanProgress       *progress,
                     GError            **error)
{
  PicmanChannel        *channel = PICMAN_CHANNEL (item);
  const PicmanBoundSeg *segs_in;
  const PicmanBoundSeg *segs_out;
  gint                n_segs_in;
  gint                n_segs_out;
  gboolean            retval = FALSE;
  gint                offset_x, offset_y;

  if (! picman_channel_boundary (channel, &segs_in, &segs_out,
                               &n_segs_in, &n_segs_out,
                               0, 0, 0, 0))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot stroke empty channel."));
      return FALSE;
    }

  picman_item_get_offset (PICMAN_ITEM (channel), &offset_x, &offset_y);

  switch (picman_stroke_options_get_method (stroke_options))
    {
    case PICMAN_STROKE_METHOD_LIBART:
      picman_drawable_stroke_boundary (drawable,
                                     stroke_options,
                                     segs_in, n_segs_in,
                                     offset_x, offset_y,
                                     push_undo);
      retval = TRUE;
      break;

    case PICMAN_STROKE_METHOD_PAINT_CORE:
      {
        PicmanPaintInfo    *paint_info;
        PicmanPaintCore    *core;
        PicmanPaintOptions *paint_options;
        gboolean          emulate_dynamics;

        paint_info = picman_context_get_paint_info (PICMAN_CONTEXT (stroke_options));

        core = g_object_new (paint_info->paint_type, NULL);

        paint_options = picman_stroke_options_get_paint_options (stroke_options);
        emulate_dynamics = picman_stroke_options_get_emulate_dynamics (stroke_options);

        retval = picman_paint_core_stroke_boundary (core, drawable,
                                                  paint_options,
                                                  emulate_dynamics,
                                                  segs_in, n_segs_in,
                                                  offset_x, offset_y,
                                                  push_undo, error);

        g_object_unref (core);
      }
      break;

    default:
      g_return_val_if_reached (FALSE);
    }

  return retval;
}

static void
picman_channel_to_selection (PicmanItem       *item,
                           PicmanChannelOps  op,
                           gboolean        antialias,
                           gboolean        feather,
                           gdouble         feather_radius_x,
                           gdouble         feather_radius_y)
{
  PicmanChannel *channel = PICMAN_CHANNEL (item);
  PicmanImage   *image   = picman_item_get_image (item);
  gint         off_x, off_y;

  picman_item_get_offset (item, &off_x, &off_y);

  picman_channel_select_channel (picman_image_get_mask (image),
                               PICMAN_ITEM_GET_CLASS (item)->to_selection_desc,
                               channel, off_x, off_y,
                               op,
                               feather, feather_radius_x, feather_radius_x);
}

static void
picman_channel_convert_type (PicmanDrawable      *drawable,
                           PicmanImage         *dest_image,
                           const Babl        *new_format,
                           PicmanImageBaseType  new_base_type,
                           PicmanPrecision      new_precision,
                           gint               layer_dither_type,
                           gint               mask_dither_type,
                           gboolean           push_undo)
{
  GeglBuffer *dest_buffer;

  dest_buffer =
    gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                     picman_item_get_width  (PICMAN_ITEM (drawable)),
                                     picman_item_get_height (PICMAN_ITEM (drawable))),
                     new_format);

  if (mask_dither_type == 0)
    {
      gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                        dest_buffer, NULL);
    }
  else
    {
      gint bits;

      bits = (babl_format_get_bytes_per_pixel (new_format) * 8 /
              babl_format_get_n_components (new_format));

      picman_gegl_apply_color_reduction (picman_drawable_get_buffer (drawable),
                                       NULL, NULL,
                                       dest_buffer, bits, mask_dither_type);
    }

  picman_drawable_set_buffer (drawable, push_undo, NULL, dest_buffer);
  g_object_unref (dest_buffer);
}

static void
picman_channel_invalidate_boundary (PicmanDrawable *drawable)
{
  PICMAN_CHANNEL (drawable)->boundary_known = FALSE;
}

static void
picman_channel_get_active_components (const PicmanDrawable *drawable,
                                    gboolean           *active)
{
  /*  Make sure that the alpha channel is not valid.  */
  active[GRAY]    = TRUE;
  active[ALPHA_G] = FALSE;
}

static PicmanComponentMask
picman_channel_get_active_mask (const PicmanDrawable *drawable)
{
  /*  Return all, because that skips the component mask op when painting  */
  return PICMAN_COMPONENT_ALL;
}

static void
picman_channel_apply_buffer (PicmanDrawable         *drawable,
                           GeglBuffer           *buffer,
                           const GeglRectangle  *buffer_region,
                           gboolean              push_undo,
                           const gchar          *undo_desc,
                           gdouble               opacity,
                           PicmanLayerModeEffects  mode,
                           GeglBuffer           *base_buffer,
                           gint                  base_x,
                           gint                  base_y)
{
  picman_drawable_invalidate_boundary (drawable);

  PICMAN_DRAWABLE_CLASS (parent_class)->apply_buffer (drawable, buffer,
                                                    buffer_region,
                                                    push_undo, undo_desc,
                                                    opacity, mode,
                                                    base_buffer,
                                                    base_x, base_y);

  PICMAN_CHANNEL (drawable)->bounds_known = FALSE;
}

static void
picman_channel_replace_buffer (PicmanDrawable        *drawable,
                             GeglBuffer          *buffer,
                             const GeglRectangle *buffer_region,
                             gboolean             push_undo,
                             const gchar         *undo_desc,
                             gdouble              opacity,
                             GeglBuffer          *mask,
                             const GeglRectangle *mask_region,
                             gint                 x,
                             gint                 y)
{
  picman_drawable_invalidate_boundary (drawable);

  PICMAN_DRAWABLE_CLASS (parent_class)->replace_buffer (drawable, buffer,
                                                      buffer_region,
                                                      push_undo, undo_desc,
                                                      opacity,
                                                      mask, mask_region,
                                                      x, y);

  PICMAN_CHANNEL (drawable)->bounds_known = FALSE;
}

static void
picman_channel_set_buffer (PicmanDrawable *drawable,
                         gboolean      push_undo,
                         const gchar  *undo_desc,
                         GeglBuffer   *buffer,
                         gint          offset_x,
                         gint          offset_y)
{
  PICMAN_DRAWABLE_CLASS (parent_class)->set_buffer (drawable,
                                                  push_undo, undo_desc,
                                                  buffer,
                                                  offset_x, offset_y);

  PICMAN_CHANNEL (drawable)->bounds_known = FALSE;
}

static void
picman_channel_swap_pixels (PicmanDrawable *drawable,
                          GeglBuffer   *buffer,
                          gint          x,
                          gint          y)
{
  picman_drawable_invalidate_boundary (drawable);

  PICMAN_DRAWABLE_CLASS (parent_class)->swap_pixels (drawable, buffer, x, y);

  PICMAN_CHANNEL (drawable)->bounds_known = FALSE;
}

static gdouble
picman_channel_get_opacity_at (PicmanPickable *pickable,
                             gint          x,
                             gint          y)
{
  PicmanChannel *channel = PICMAN_CHANNEL (pickable);
  gdouble      value   = PICMAN_OPACITY_TRANSPARENT;

  if (x >= 0 && x < picman_item_get_width  (PICMAN_ITEM (channel)) &&
      y >= 0 && y < picman_item_get_height (PICMAN_ITEM (channel)))
    {
      if (! channel->bounds_known ||
          (! channel->empty &&
           x >= channel->x1 &&
           x <  channel->x2 &&
           y >= channel->y1 &&
           y <  channel->y2))
        {
          gegl_buffer_sample (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                              x, y, NULL, &value, babl_format ("Y double"),
                              GEGL_SAMPLER_NEAREST, GEGL_ABYSS_NONE);
        }
    }

  return value;
}

static gboolean
picman_channel_real_boundary (PicmanChannel         *channel,
                            const PicmanBoundSeg **segs_in,
                            const PicmanBoundSeg **segs_out,
                            gint                *num_segs_in,
                            gint                *num_segs_out,
                            gint                 x1,
                            gint                 y1,
                            gint                 x2,
                            gint                 y2)
{
  if (! channel->boundary_known)
    {
      gint x3, y3, x4, y4;

      /* free the out of date boundary segments */
      g_free (channel->segs_in);
      g_free (channel->segs_out);

      if (picman_channel_bounds (channel, &x3, &y3, &x4, &y4))
        {
          GeglBuffer *buffer;
          GeglRectangle  rect = { x3, y3, x4 - x3, y4 - y3 };

          buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));

          channel->segs_out = picman_boundary_find (buffer, &rect,
                                                  babl_format ("Y float"),
                                                  PICMAN_BOUNDARY_IGNORE_BOUNDS,
                                                  x1, y1, x2, y2,
                                                  PICMAN_BOUNDARY_HALF_WAY,
                                                  &channel->num_segs_out);
          x1 = MAX (x1, x3);
          y1 = MAX (y1, y3);
          x2 = MIN (x2, x4);
          y2 = MIN (y2, y4);

          if (x2 > x1 && y2 > y1)
            {
              channel->segs_in = picman_boundary_find (buffer, NULL,
                                                     babl_format ("Y float"),
                                                     PICMAN_BOUNDARY_WITHIN_BOUNDS,
                                                     x1, y1, x2, y2,
                                                     PICMAN_BOUNDARY_HALF_WAY,
                                                     &channel->num_segs_in);
            }
          else
            {
              channel->segs_in     = NULL;
              channel->num_segs_in = 0;
            }
        }
      else
        {
          channel->segs_in      = NULL;
          channel->segs_out     = NULL;
          channel->num_segs_in  = 0;
          channel->num_segs_out = 0;
        }

      channel->boundary_known = TRUE;
    }

  *segs_in      = channel->segs_in;
  *segs_out     = channel->segs_out;
  *num_segs_in  = channel->num_segs_in;
  *num_segs_out = channel->num_segs_out;

  return (! channel->empty);
}

static gboolean
picman_channel_real_bounds (PicmanChannel *channel,
                          gint        *x1,
                          gint        *y1,
                          gint        *x2,
                          gint        *y2)
{
  GeglBuffer *buffer;

  /*  if the channel's bounds have already been reliably calculated...  */
  if (channel->bounds_known)
    {
      *x1 = channel->x1;
      *y1 = channel->y1;
      *x2 = channel->x2;
      *y2 = channel->y2;

      return ! channel->empty;
    }

  buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));

  channel->empty = ! picman_gegl_mask_bounds (buffer, x1, y1, x2, y2);

  channel->x1 = *x1;
  channel->y1 = *y1;
  channel->x2 = *x2;
  channel->y2 = *y2;

  channel->bounds_known = TRUE;

  return ! channel->empty;
}

static gboolean
picman_channel_real_is_empty (PicmanChannel *channel)
{
  GeglBuffer *buffer;

  if (channel->bounds_known)
    return channel->empty;

  buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));

  if (! picman_gegl_mask_is_empty (buffer))
    return FALSE;

  /*  The mask is empty, meaning we can set the bounds as known  */
  if (channel->segs_in)
    g_free (channel->segs_in);
  if (channel->segs_out)
    g_free (channel->segs_out);

  channel->empty          = TRUE;
  channel->segs_in        = NULL;
  channel->segs_out       = NULL;
  channel->num_segs_in    = 0;
  channel->num_segs_out   = 0;
  channel->bounds_known   = TRUE;
  channel->boundary_known = TRUE;
  channel->x1             = 0;
  channel->y1             = 0;
  channel->x2             = picman_item_get_width  (PICMAN_ITEM (channel));
  channel->y2             = picman_item_get_height (PICMAN_ITEM (channel));

  return TRUE;
}

static void
picman_channel_real_feather (PicmanChannel *channel,
                           gdouble      radius_x,
                           gdouble      radius_y,
                           gboolean     push_undo)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (channel);

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->feather_desc);
  else
    picman_drawable_invalidate_boundary (drawable);

  picman_gegl_apply_feather (picman_drawable_get_buffer (drawable),
                           NULL, NULL,
                           picman_drawable_get_buffer (drawable),
                           radius_x,
                           radius_y);

  channel->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_sharpen (PicmanChannel *channel,
                           gboolean     push_undo)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (channel);

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->sharpen_desc);
  else
    picman_drawable_invalidate_boundary (drawable);

  picman_gegl_apply_threshold (picman_drawable_get_buffer (drawable),
                             NULL, NULL,
                             picman_drawable_get_buffer (drawable),
                             0.5);

  channel->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_clear (PicmanChannel *channel,
                         const gchar *undo_desc,
                         gboolean     push_undo)
{
  if (push_undo)
    {
      if (! undo_desc)
        undo_desc = PICMAN_CHANNEL_GET_CLASS (channel)->clear_desc;

      picman_channel_push_undo (channel, undo_desc);
    }
  else
    {
      picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));
    }

  if (channel->bounds_known && ! channel->empty)
    {
      gegl_buffer_clear (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                         GEGL_RECTANGLE (channel->x1, channel->y1,
                                         channel->x2 - channel->x1,
                                         channel->y2 - channel->y1));
    }
  else
    {
      gegl_buffer_clear (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                         NULL);
    }

  /*  we know the bounds  */
  channel->bounds_known = TRUE;
  channel->empty        = TRUE;
  channel->x1           = 0;
  channel->y1           = 0;
  channel->x2           = picman_item_get_width  (PICMAN_ITEM (channel));
  channel->y2           = picman_item_get_height (PICMAN_ITEM (channel));

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_all (PicmanChannel *channel,
                       gboolean     push_undo)
{
  GeglColor *color;

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->all_desc);
  else
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));

  /*  clear the channel  */
  color = gegl_color_new ("#fff");
  gegl_buffer_set_color (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                         NULL, color);
  g_object_unref (color);

  /*  we know the bounds  */
  channel->bounds_known = TRUE;
  channel->empty        = FALSE;
  channel->x1           = 0;
  channel->y1           = 0;
  channel->x2           = picman_item_get_width  (PICMAN_ITEM (channel));
  channel->y2           = picman_item_get_height (PICMAN_ITEM (channel));

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_invert (PicmanChannel *channel,
                          gboolean     push_undo)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (channel);

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->invert_desc);
  else
    picman_drawable_invalidate_boundary (drawable);

  if (channel->bounds_known && channel->empty)
    {
      picman_channel_all (channel, FALSE);
    }
  else
    {
      picman_gegl_apply_invert (picman_drawable_get_buffer (drawable),
                              NULL, NULL,
                              picman_drawable_get_buffer (drawable));

      channel->bounds_known = FALSE;

      picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                            picman_item_get_width  (PICMAN_ITEM (channel)),
                            picman_item_get_height (PICMAN_ITEM (channel)));
    }
}

static void
picman_channel_real_border (PicmanChannel *channel,
                          gint         radius_x,
                          gint         radius_y,
                          gboolean     feather,
                          gboolean     edge_lock,
                          gboolean     push_undo)
{
  GeglNode *border;
  gint      x1, y1, x2, y2;

  if (radius_x < 0 || radius_y < 0)
    return;

  if (! picman_channel_bounds (channel, &x1, &y1, &x2, &y2))
    return;

  if (picman_channel_is_empty (channel))
    return;

  if (x1 - radius_x < 0)
    x1 = 0;
  else
    x1 -= radius_x;

  if (x2 + radius_x > picman_item_get_width (PICMAN_ITEM (channel)))
    x2 = picman_item_get_width (PICMAN_ITEM (channel));
  else
    x2 += radius_x;

  if (y1 - radius_y < 0)
    y1 = 0;
  else
    y1 -= radius_y;

  if (y2 + radius_y > picman_item_get_height (PICMAN_ITEM (channel)))
    y2 = picman_item_get_height (PICMAN_ITEM (channel));
  else
    y2 += radius_y;

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->border_desc);
  else
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));

  border = gegl_node_new_child (NULL,
                                "operation", "picman:border",
                                "radius-x",  radius_x,
                                "radius-y",  radius_y,
                                "feather",   feather,
                                "edge-lock", edge_lock,
                                NULL);

  picman_gegl_apply_operation (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             NULL, NULL,
                             border,
                             picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1));

  g_object_unref (border);

  channel->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_grow (PicmanChannel *channel,
                        gint         radius_x,
                        gint         radius_y,
                        gboolean     push_undo)
{
  GeglNode *grow;
  gint      x1, y1, x2, y2;

  if (radius_x == 0 && radius_y == 0)
    return;

  if (radius_x <= 0 && radius_y <= 0)
    {
      picman_channel_shrink (channel, -radius_x, -radius_y, FALSE, push_undo);
      return;
    }

  if (radius_x < 0 || radius_y < 0)
    return;

  if (! picman_channel_bounds (channel, &x1, &y1, &x2, &y2))
    return;

  if (picman_channel_is_empty (channel))
    return;

  if (x1 - radius_x > 0)
    x1 = x1 - radius_x;
  else
    x1 = 0;

  if (y1 - radius_y > 0)
    y1 = y1 - radius_y;
  else
    y1 = 0;

  if (x2 + radius_x < picman_item_get_width (PICMAN_ITEM (channel)))
    x2 = x2 + radius_x;
  else
    x2 = picman_item_get_width (PICMAN_ITEM (channel));

  if (y2 + radius_y < picman_item_get_height (PICMAN_ITEM (channel)))
    y2 = y2 + radius_y;
  else
    y2 = picman_item_get_height (PICMAN_ITEM (channel));

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->grow_desc);
  else
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));

  grow = gegl_node_new_child (NULL,
                              "operation", "picman:grow",
                              "radius-x",  radius_x,
                              "radius-y",  radius_y,
                              NULL);

  picman_gegl_apply_operation (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             NULL, NULL,
                             grow,
                             picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1));

  g_object_unref (grow);

  channel->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}

static void
picman_channel_real_shrink (PicmanChannel *channel,
                          gint         radius_x,
                          gint         radius_y,
                          gboolean     edge_lock,
                          gboolean     push_undo)
{
  GeglNode *shrink;
  gint      x1, y1, x2, y2;

  if (radius_x == 0 && radius_y == 0)
    return;

  if (radius_x <= 0 && radius_y <= 0)
    {
      picman_channel_grow (channel, -radius_x, -radius_y, push_undo);
      return;
    }

  if (radius_x < 0 || radius_y < 0)
    return;

  if (! picman_channel_bounds (channel, &x1, &y1, &x2, &y2))
    return;

  if (picman_channel_is_empty (channel))
    return;

  if (x1 > 0)
    x1--;
  if (y1 > 0)
    y1--;
  if (x2 < picman_item_get_width (PICMAN_ITEM (channel)))
    x2++;
  if (y2 < picman_item_get_height (PICMAN_ITEM (channel)))
    y2++;

  if (push_undo)
    picman_channel_push_undo (channel,
                            PICMAN_CHANNEL_GET_CLASS (channel)->shrink_desc);
  else
    picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));

  shrink = gegl_node_new_child (NULL,
                                "operation", "picman:shrink",
                                "radius-x",  radius_x,
                                "radius-y",  radius_y,
                                "edge-lock", edge_lock,
                                NULL);

  picman_gegl_apply_operation (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             NULL, NULL,
                             shrink,
                             picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                             GEGL_RECTANGLE (x1, y1, x2 - x1, y2 - y1));

  g_object_unref (shrink);

  channel->bounds_known = FALSE;

  picman_drawable_update (PICMAN_DRAWABLE (channel), 0, 0,
                        picman_item_get_width  (PICMAN_ITEM (channel)),
                        picman_item_get_height (PICMAN_ITEM (channel)));
}


/*  public functions  */

PicmanChannel *
picman_channel_new (PicmanImage     *image,
                  gint           width,
                  gint           height,
                  const gchar   *name,
                  const PicmanRGB *color)
{
  PicmanChannel *channel;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  channel =
    PICMAN_CHANNEL (picman_drawable_new (PICMAN_TYPE_CHANNEL,
                                     image, name,
                                     0, 0, width, height,
                                     picman_image_get_channel_format (image)));

  if (color)
    channel->color = *color;

  channel->show_masked = TRUE;

  /*  selection mask variables  */
  channel->x2          = width;
  channel->y2          = height;

  return channel;
}

PicmanChannel *
picman_channel_new_from_alpha (PicmanImage     *image,
                             PicmanDrawable  *drawable,
                             const gchar   *name,
                             const PicmanRGB *color)
{
  PicmanChannel *channel;
  GeglBuffer  *dest_buffer;
  gint         width;
  gint         height;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_drawable_has_alpha (drawable), NULL);

  width  = picman_item_get_width  (PICMAN_ITEM (drawable));
  height = picman_item_get_height (PICMAN_ITEM (drawable));

  channel = picman_channel_new (image, width, height, name, color);

  picman_channel_clear (channel, NULL, FALSE);

  dest_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));

  gegl_buffer_set_format (dest_buffer,
                          picman_image_get_component_format (image,
                                                           PICMAN_ALPHA_CHANNEL));
  gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                    dest_buffer, NULL);
  gegl_buffer_set_format (dest_buffer, NULL);

  channel->bounds_known = FALSE;

  return channel;
}

PicmanChannel *
picman_channel_new_from_component (PicmanImage       *image,
                                 PicmanChannelType  type,
                                 const gchar     *name,
                                 const PicmanRGB   *color)
{
  PicmanProjection *projection;
  PicmanChannel    *channel;
  GeglBuffer     *src_buffer;
  GeglBuffer     *dest_buffer;
  gint            width;
  gint            height;
  const Babl     *format;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  format = picman_image_get_component_format (image, type);

  g_return_val_if_fail (format != NULL, NULL);

  projection = picman_image_get_projection (image);

  picman_pickable_flush (PICMAN_PICKABLE (projection));

  src_buffer = picman_pickable_get_buffer (PICMAN_PICKABLE (projection));
  width  = gegl_buffer_get_width  (src_buffer);
  height = gegl_buffer_get_height (src_buffer);

  channel = picman_channel_new (image, width, height, name, color);

  dest_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));

  gegl_buffer_set_format (dest_buffer, format);
  gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);
  gegl_buffer_set_format (dest_buffer, NULL);

  return channel;
}

PicmanChannel *
picman_channel_get_parent (PicmanChannel *channel)
{
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), NULL);

  return PICMAN_CHANNEL (picman_viewable_get_parent (PICMAN_VIEWABLE (channel)));
}

void
picman_channel_set_color (PicmanChannel   *channel,
                        const PicmanRGB *color,
                        gboolean       push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (color != NULL);

  if (picman_rgba_distance (&channel->color, color) > 0.0001)
    {
      if (push_undo && picman_item_is_attached (PICMAN_ITEM (channel)))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (channel));

          picman_image_undo_push_channel_color (image, C_("undo-type", "Set Channel Color"),
                                              channel);
        }

      channel->color = *color;

      if (picman_filter_peek_node (PICMAN_FILTER (channel)))
        {
          GeglColor *gegl_color = picman_gegl_color_new (&channel->color);

          gegl_node_set (channel->color_node,
                         "value", gegl_color,
                         NULL);

          g_object_unref (gegl_color);
        }

      picman_drawable_update (PICMAN_DRAWABLE (channel),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (channel)),
                            picman_item_get_height (PICMAN_ITEM (channel)));

      g_signal_emit (channel, channel_signals[COLOR_CHANGED], 0);
    }
}

void
picman_channel_get_color (const PicmanChannel *channel,
                        PicmanRGB           *color)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (color != NULL);

  *color = channel->color;
}

gdouble
picman_channel_get_opacity (const PicmanChannel *channel)
{
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), PICMAN_OPACITY_TRANSPARENT);

  return channel->color.a;
}

void
picman_channel_set_opacity (PicmanChannel *channel,
                          gdouble      opacity,
                          gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  opacity = CLAMP (opacity, PICMAN_OPACITY_TRANSPARENT, PICMAN_OPACITY_OPAQUE);

  if (channel->color.a != opacity)
    {
      if (push_undo && picman_item_is_attached (PICMAN_ITEM (channel)))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (channel));

          picman_image_undo_push_channel_color (image, C_("undo-type", "Set Channel Opacity"),
                                              channel);
        }

      channel->color.a = opacity;

      if (picman_filter_peek_node (PICMAN_FILTER (channel)))
        {
          GeglColor *gegl_color = picman_gegl_color_new (&channel->color);

          gegl_node_set (channel->color_node,
                         "value", gegl_color,
                         NULL);

          g_object_unref (gegl_color);
        }

      picman_drawable_update (PICMAN_DRAWABLE (channel),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (channel)),
                            picman_item_get_height (PICMAN_ITEM (channel)));

      g_signal_emit (channel, channel_signals[COLOR_CHANGED], 0);
    }
}

gboolean
picman_channel_get_show_masked (PicmanChannel *channel)
{
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), FALSE);

  return channel->show_masked;
}

void
picman_channel_set_show_masked (PicmanChannel *channel,
                              gboolean     show_masked)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (show_masked != channel->show_masked)
    {
      channel->show_masked = show_masked ? TRUE : FALSE;

      if (channel->invert_node)
        {
          GeglNode *source;

          source = picman_drawable_get_source_node (PICMAN_DRAWABLE (channel));

          if (channel->show_masked)
            {
              gegl_node_connect_to (source,               "output",
                                    channel->invert_node, "input");
              gegl_node_connect_to (channel->invert_node, "output",
                                    channel->mask_node,   "aux");
            }
          else
            {
              gegl_node_disconnect (channel->invert_node, "input");

              gegl_node_connect_to (source,             "output",
                                    channel->mask_node, "aux");
            }
        }

      picman_drawable_update (PICMAN_DRAWABLE (channel),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (channel)),
                            picman_item_get_height (PICMAN_ITEM (channel)));
    }
}

void
picman_channel_push_undo (PicmanChannel *channel,
                        const gchar *undo_desc)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (channel)));

  picman_image_undo_push_mask (picman_item_get_image (PICMAN_ITEM (channel)),
                             undo_desc, channel);

  picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (channel));
}


/******************************/
/*  selection mask functions  */
/******************************/

PicmanChannel *
picman_channel_new_mask (PicmanImage *image,
                       gint       width,
                       gint       height)
{
  PicmanChannel *channel;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  channel =
    PICMAN_CHANNEL (picman_drawable_new (PICMAN_TYPE_CHANNEL,
                                     image, _("Selection Mask"),
                                     0, 0, width, height,
                                     picman_image_get_mask_format (image)));

  channel->show_masked = TRUE;
  channel->x2          = width;
  channel->y2          = height;

  gegl_buffer_clear (picman_drawable_get_buffer (PICMAN_DRAWABLE (channel)),
                     NULL);

  return channel;
}

gboolean
picman_channel_boundary (PicmanChannel         *channel,
                       const PicmanBoundSeg **segs_in,
                       const PicmanBoundSeg **segs_out,
                       gint                *num_segs_in,
                       gint                *num_segs_out,
                       gint                 x1,
                       gint                 y1,
                       gint                 x2,
                       gint                 y2)
{
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), FALSE);
  g_return_val_if_fail (segs_in != NULL, FALSE);
  g_return_val_if_fail (segs_out != NULL, FALSE);
  g_return_val_if_fail (num_segs_in != NULL, FALSE);
  g_return_val_if_fail (num_segs_out != NULL, FALSE);

  return PICMAN_CHANNEL_GET_CLASS (channel)->boundary (channel,
                                                     segs_in, segs_out,
                                                     num_segs_in, num_segs_out,
                                                     x1, y1,
                                                     x2, y2);
}

gboolean
picman_channel_bounds (PicmanChannel *channel,
                     gint        *x1,
                     gint        *y1,
                     gint        *x2,
                     gint        *y2)
{
  gint     tmp_x1, tmp_y1, tmp_x2, tmp_y2;
  gboolean retval;

  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), FALSE);

  retval = PICMAN_CHANNEL_GET_CLASS (channel)->bounds (channel,
                                                     &tmp_x1, &tmp_y1,
                                                     &tmp_x2, &tmp_y2);

  if (x1) *x1 = tmp_x1;
  if (y1) *y1 = tmp_y1;
  if (x2) *x2 = tmp_x2;
  if (y2) *y2 = tmp_y2;

  return retval;
}

gboolean
picman_channel_is_empty (PicmanChannel *channel)
{
  g_return_val_if_fail (PICMAN_IS_CHANNEL (channel), FALSE);

  return PICMAN_CHANNEL_GET_CLASS (channel)->is_empty (channel);
}

void
picman_channel_feather (PicmanChannel *channel,
                      gdouble      radius_x,
                      gdouble      radius_y,
                      gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->feather (channel, radius_x, radius_y,
                                             push_undo);
}

void
picman_channel_sharpen (PicmanChannel *channel,
                      gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->sharpen (channel, push_undo);
}

void
picman_channel_clear (PicmanChannel *channel,
                    const gchar *undo_desc,
                    gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->clear (channel, undo_desc, push_undo);
}

void
picman_channel_all (PicmanChannel *channel,
                  gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->all (channel, push_undo);
}

void
picman_channel_invert (PicmanChannel *channel,
                     gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->invert (channel, push_undo);
}

void
picman_channel_border (PicmanChannel *channel,
                     gint         radius_x,
                     gint         radius_y,
                     gboolean     feather,
                     gboolean     edge_lock,
                     gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->border (channel,
                                            radius_x, radius_y, feather, edge_lock,
                                            push_undo);
}

void
picman_channel_grow (PicmanChannel *channel,
                   gint         radius_x,
                   gint         radius_y,
                   gboolean     push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->grow (channel, radius_x, radius_y,
                                          push_undo);
}

void
picman_channel_shrink (PicmanChannel  *channel,
                     gint          radius_x,
                     gint          radius_y,
                     gboolean      edge_lock,
                     gboolean      push_undo)
{
  g_return_if_fail (PICMAN_IS_CHANNEL (channel));

  if (! picman_item_is_attached (PICMAN_ITEM (channel)))
    push_undo = FALSE;

  PICMAN_CHANNEL_GET_CLASS (channel)->shrink (channel, radius_x, radius_y,
                                            edge_lock, push_undo);
}
