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
#include <gegl-plugin.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"

#include "core-types.h"

#include "gegl/picmanapplicator.h"
#include "gegl/picman-babl.h"
#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-utils.h"

#include "picman-utils.h"
#include "picmanchannel.h"
#include "picmancontext.h"
#include "picmandrawable-combine.h"
#include "picmandrawable-filter.h"
#include "picmandrawable-preview.h"
#include "picmandrawable-private.h"
#include "picmandrawable-shadow.h"
#include "picmandrawable-transform.h"
#include "picmanfilterstack.h"
#include "picmanimage.h"
#include "picmanimage-colormap.h"
#include "picmanimage-undo-push.h"
#include "picmanlayer.h"
#include "picmanmarshal.h"
#include "picmanpattern.h"
#include "picmanpickable.h"
#include "picmanprogress.h"

#include "picman-log.h"

#include "picman-intl.h"


enum
{
  UPDATE,
  ALPHA_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0
};


/*  local function prototypes  */

static void  picman_drawable_pickable_iface_init (PicmanPickableInterface *iface);

static void       picman_drawable_dispose            (GObject           *object);
static void       picman_drawable_finalize           (GObject           *object);
static void       picman_drawable_set_property       (GObject           *object,
                                                    guint              property_id,
                                                    const GValue      *value,
                                                    GParamSpec        *pspec);
static void       picman_drawable_get_property       (GObject           *object,
                                                    guint              property_id,
                                                    GValue            *value,
                                                    GParamSpec        *pspec);

static gint64     picman_drawable_get_memsize        (PicmanObject        *object,
                                                    gint64            *gui_size);

static gboolean   picman_drawable_get_size           (PicmanViewable      *viewable,
                                                    gint              *width,
                                                    gint              *height);

static GeglNode * picman_drawable_get_node           (PicmanFilter        *filter);

static void       picman_drawable_removed            (PicmanItem          *item);
static void       picman_drawable_visibility_changed (PicmanItem          *item);
static PicmanItem * picman_drawable_duplicate          (PicmanItem          *item,
                                                    GType              new_type);
static void       picman_drawable_scale              (PicmanItem          *item,
                                                    gint               new_width,
                                                    gint               new_height,
                                                    gint               new_offset_x,
                                                    gint               new_offset_y,
                                                    PicmanInterpolationType interp_type,
                                                    PicmanProgress      *progress);
static void       picman_drawable_resize             (PicmanItem          *item,
                                                    PicmanContext       *context,
                                                    gint               new_width,
                                                    gint               new_height,
                                                    gint               offset_x,
                                                    gint               offset_y);
static void       picman_drawable_flip               (PicmanItem          *item,
                                                    PicmanContext       *context,
                                                    PicmanOrientationType  flip_type,
                                                    gdouble            axis,
                                                    gboolean           clip_result);
static void       picman_drawable_rotate             (PicmanItem          *item,
                                                    PicmanContext       *context,
                                                    PicmanRotationType   rotate_type,
                                                    gdouble            center_x,
                                                    gdouble            center_y,
                                                    gboolean           clip_result);
static void       picman_drawable_transform          (PicmanItem          *item,
                                                    PicmanContext       *context,
                                                    const PicmanMatrix3 *matrix,
                                                    PicmanTransformDirection direction,
                                                    PicmanInterpolationType interpolation_type,
                                                    gint               recursion_level,
                                                    PicmanTransformResize clip_result,
                                                    PicmanProgress      *progress);

static gboolean   picman_drawable_get_pixel_at       (PicmanPickable      *pickable,
                                                    gint               x,
                                                    gint               y,
                                                    const Babl        *format,
                                                    gpointer           pixel);
static void       picman_drawable_real_update        (PicmanDrawable      *drawable,
                                                    gint               x,
                                                    gint               y,
                                                    gint               width,
                                                    gint               height);

static gint64  picman_drawable_real_estimate_memsize (const PicmanDrawable *drawable,
                                                    gint               width,
                                                    gint               height);

static void       picman_drawable_real_convert_type  (PicmanDrawable      *drawable,
                                                    PicmanImage         *dest_image,
                                                    const Babl        *new_format,
                                                    PicmanImageBaseType  new_base_type,
                                                    PicmanPrecision      new_precision,
                                                    gint               layer_dither_type,
                                                    gint               mask_dither_type,
                                                    gboolean           push_undo);

static GeglBuffer * picman_drawable_real_get_buffer  (PicmanDrawable      *drawable);
static void       picman_drawable_real_set_buffer    (PicmanDrawable      *drawable,
                                                    gboolean           push_undo,
                                                    const gchar       *undo_desc,
                                                    GeglBuffer        *buffer,
                                                    gint               offset_x,
                                                    gint               offset_y);

static void       picman_drawable_real_push_undo     (PicmanDrawable      *drawable,
                                                    const gchar       *undo_desc,
                                                    GeglBuffer        *buffer,
                                                    gint               x,
                                                    gint               y,
                                                    gint               width,
                                                    gint               height);
static void       picman_drawable_real_swap_pixels   (PicmanDrawable      *drawable,
                                                    GeglBuffer        *buffer,
                                                    gint               x,
                                                    gint               y);

static void       picman_drawable_sync_fs_filter     (PicmanDrawable      *drawable,
                                                    gboolean           detach_fs);
static void       picman_drawable_fs_notify          (PicmanLayer         *fs,
                                                    const GParamSpec  *pspec,
                                                    PicmanDrawable      *drawable);
static void       picman_drawable_fs_affect_changed  (PicmanImage         *image,
                                                    PicmanChannelType    channel,
                                                    PicmanDrawable      *drawable);
static void       picman_drawable_fs_mask_changed    (PicmanImage         *image,
                                                    PicmanDrawable      *drawable);
static void       picman_drawable_fs_update          (PicmanLayer         *fs,
                                                    gint               x,
                                                    gint               y,
                                                    gint               width,
                                                    gint               height,
                                                    PicmanDrawable      *drawable);


G_DEFINE_TYPE_WITH_CODE (PicmanDrawable, picman_drawable, PICMAN_TYPE_ITEM,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_drawable_pickable_iface_init))

#define parent_class picman_drawable_parent_class

static guint picman_drawable_signals[LAST_SIGNAL] = { 0 };


static void
picman_drawable_class_init (PicmanDrawableClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanFilterClass   *filter_class      = PICMAN_FILTER_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);

  picman_drawable_signals[UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDrawableClass, update),
                  NULL, NULL,
                  picman_marshal_VOID__INT_INT_INT_INT,
                  G_TYPE_NONE, 4,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT,
                  G_TYPE_INT);

  picman_drawable_signals[ALPHA_CHANGED] =
    g_signal_new ("alpha-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanDrawableClass, alpha_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->dispose              = picman_drawable_dispose;
  object_class->finalize             = picman_drawable_finalize;
  object_class->set_property         = picman_drawable_set_property;
  object_class->get_property         = picman_drawable_get_property;

  picman_object_class->get_memsize     = picman_drawable_get_memsize;

  viewable_class->get_size           = picman_drawable_get_size;
  viewable_class->get_new_preview    = picman_drawable_get_new_preview;

  filter_class->get_node             = picman_drawable_get_node;

  item_class->removed                = picman_drawable_removed;
  item_class->visibility_changed     = picman_drawable_visibility_changed;
  item_class->duplicate              = picman_drawable_duplicate;
  item_class->scale                  = picman_drawable_scale;
  item_class->resize                 = picman_drawable_resize;
  item_class->flip                   = picman_drawable_flip;
  item_class->rotate                 = picman_drawable_rotate;
  item_class->transform              = picman_drawable_transform;

  klass->update                      = picman_drawable_real_update;
  klass->alpha_changed               = NULL;
  klass->estimate_memsize            = picman_drawable_real_estimate_memsize;
  klass->invalidate_boundary         = NULL;
  klass->get_active_components       = NULL;
  klass->get_active_mask             = NULL;
  klass->convert_type                = picman_drawable_real_convert_type;
  klass->apply_buffer                = picman_drawable_real_apply_buffer;
  klass->replace_buffer              = picman_drawable_real_replace_buffer;
  klass->get_buffer                  = picman_drawable_real_get_buffer;
  klass->set_buffer                  = picman_drawable_real_set_buffer;
  klass->push_undo                   = picman_drawable_real_push_undo;
  klass->swap_pixels                 = picman_drawable_real_swap_pixels;

  g_type_class_add_private (klass, sizeof (PicmanDrawablePrivate));
}

static void
picman_drawable_init (PicmanDrawable *drawable)
{
  drawable->private = G_TYPE_INSTANCE_GET_PRIVATE (drawable,
                                                   PICMAN_TYPE_DRAWABLE,
                                                   PicmanDrawablePrivate);

  drawable->private->filter_stack = picman_filter_stack_new (PICMAN_TYPE_FILTER);
}

/* sorry for the evil casts */

static void
picman_drawable_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->get_image             = (PicmanImage     * (*) (PicmanPickable *pickable)) picman_item_get_image;
  iface->get_format            = (const Babl    * (*) (PicmanPickable *pickable)) picman_drawable_get_format;
  iface->get_format_with_alpha = (const Babl    * (*) (PicmanPickable *pickable)) picman_drawable_get_format_with_alpha;
  iface->get_buffer            = (GeglBuffer    * (*) (PicmanPickable *pickable)) picman_drawable_get_buffer;
  iface->get_pixel_at          = picman_drawable_get_pixel_at;
}

static void
picman_drawable_dispose (GObject *object)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (object);

  if (picman_drawable_get_floating_sel (drawable))
    picman_drawable_detach_floating_sel (drawable);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_drawable_finalize (GObject *object)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (object);

  if (drawable->private->buffer)
    {
      g_object_unref (drawable->private->buffer);
      drawable->private->buffer = NULL;
    }

  picman_drawable_free_shadow_buffer (drawable);

  if (drawable->private->source_node)
    {
      g_object_unref (drawable->private->source_node);
      drawable->private->source_node = NULL;
    }

  if (drawable->private->filter_stack)
    {
      g_object_unref (drawable->private->filter_stack);
      drawable->private->filter_stack = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_drawable_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_drawable_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  switch (property_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gint64
picman_drawable_get_memsize (PicmanObject *object,
                           gint64     *gui_size)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (object);
  gint64        memsize  = 0;

  memsize += picman_gegl_buffer_get_memsize (picman_drawable_get_buffer (drawable));
  memsize += picman_gegl_buffer_get_memsize (drawable->private->shadow);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_drawable_get_size (PicmanViewable *viewable,
                        gint         *width,
                        gint         *height)
{
  PicmanItem *item = PICMAN_ITEM (viewable);

  *width  = picman_item_get_width  (item);
  *height = picman_item_get_height (item);

  return TRUE;
}

static GeglNode *
picman_drawable_get_node (PicmanFilter *filter)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (filter);
  GeglNode     *node;
  GeglNode     *input;
  GeglNode     *output;

  node = PICMAN_FILTER_CLASS (parent_class)->get_node (filter);

  g_warn_if_fail (drawable->private->mode_node == NULL);

  drawable->private->mode_node =
    gegl_node_new_child (node,
                         "operation", "picman:normal-mode",
                         NULL);

  input  = gegl_node_get_input_proxy  (node, "input");
  output = gegl_node_get_output_proxy (node, "output");

  if (picman_item_get_visible (PICMAN_ITEM (drawable)))
    {
      gegl_node_connect_to (input,                        "output",
                            drawable->private->mode_node, "input");
      gegl_node_connect_to (drawable->private->mode_node, "output",
                            output,                       "input");
    }
  else
    {
      gegl_node_connect_to (input,  "output",
                            output, "input");
    }

  return node;
}

static void
picman_drawable_removed (PicmanItem *item)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);

  picman_drawable_free_shadow_buffer (drawable);

  if (PICMAN_ITEM_CLASS (parent_class)->removed)
    PICMAN_ITEM_CLASS (parent_class)->removed (item);
}

static void
picman_drawable_visibility_changed (PicmanItem *item)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglNode     *node;

  /*  don't use picman_filter_get_node() because that would create
   *  the node.
   */
  node = picman_filter_peek_node (PICMAN_FILTER (item));

  if (node)
    {
      GeglNode *input  = gegl_node_get_input_proxy  (node, "input");
      GeglNode *output = gegl_node_get_output_proxy (node, "output");

      if (picman_item_get_visible (item))
        {
          gegl_node_connect_to (input,                        "output",
                                drawable->private->mode_node, "input");
          gegl_node_connect_to (drawable->private->mode_node, "output",
                                output,                       "input");
        }
      else
        {
          gegl_node_disconnect (drawable->private->mode_node, "input");

          gegl_node_connect_to (input,  "output",
                                output, "input");
        }

      /* FIXME: chain up again when above floating sel special case is gone */
      return;
    }

  PICMAN_ITEM_CLASS (parent_class)->visibility_changed (item);
}

static PicmanItem *
picman_drawable_duplicate (PicmanItem *item,
                         GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_DRAWABLE (new_item))
    {
      PicmanDrawable  *drawable     = PICMAN_DRAWABLE (item);
      PicmanDrawable  *new_drawable = PICMAN_DRAWABLE (new_item);

      if (new_drawable->private->buffer)
        g_object_unref (new_drawable->private->buffer);

      new_drawable->private->buffer =
        gegl_buffer_dup (picman_drawable_get_buffer (drawable));
    }

  return new_item;
}

static void
picman_drawable_scale (PicmanItem              *item,
                     gint                   new_width,
                     gint                   new_height,
                     gint                   new_offset_x,
                     gint                   new_offset_y,
                     PicmanInterpolationType  interpolation_type,
                     PicmanProgress          *progress)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglBuffer   *new_buffer;

  new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                new_width, new_height),
                                picman_drawable_get_format (drawable));

  picman_gegl_apply_scale (picman_drawable_get_buffer (drawable),
                         progress, C_("undo-type", "Scale"),
                         new_buffer,
                         interpolation_type,
                         ((gdouble) new_width /
                          picman_item_get_width  (item)),
                         ((gdouble) new_height /
                          picman_item_get_height (item)));

  picman_drawable_set_buffer_full (drawable, picman_item_is_attached (item), NULL,
                                 new_buffer,
                                 new_offset_x, new_offset_y);
  g_object_unref (new_buffer);
}

static void
picman_drawable_resize (PicmanItem    *item,
                      PicmanContext *context,
                      gint         new_width,
                      gint         new_height,
                      gint         offset_x,
                      gint         offset_y)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglBuffer   *new_buffer;
  gint          new_offset_x;
  gint          new_offset_y;
  gint          copy_x, copy_y;
  gint          copy_width, copy_height;

  /*  if the size doesn't change, this is a nop  */
  if (new_width  == picman_item_get_width  (item) &&
      new_height == picman_item_get_height (item) &&
      offset_x   == 0                       &&
      offset_y   == 0)
    return;

  new_offset_x = picman_item_get_offset_x (item) - offset_x;
  new_offset_y = picman_item_get_offset_y (item) - offset_y;

  picman_rectangle_intersect (picman_item_get_offset_x (item),
                            picman_item_get_offset_y (item),
                            picman_item_get_width (item),
                            picman_item_get_height (item),
                            new_offset_x,
                            new_offset_y,
                            new_width,
                            new_height,
                            &copy_x,
                            &copy_y,
                            &copy_width,
                            &copy_height);

  new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                new_width, new_height),
                                picman_drawable_get_format (drawable));

  if (copy_width  != new_width ||
      copy_height != new_height)
    {
      /*  Clear the new tiles if needed  */

      PicmanRGB    bg;
      GeglColor *col;

      if (! picman_drawable_has_alpha (drawable) && ! PICMAN_IS_CHANNEL (drawable))
        picman_context_get_background (context, &bg);
      else
        picman_rgba_set (&bg, 0.0, 0.0, 0.0, 0.0);

      col = picman_gegl_color_new (&bg);

      gegl_buffer_set_color (new_buffer, NULL, col);
      g_object_unref (col);
    }

  if (copy_width && copy_height)
    {
      /*  Copy the pixels in the intersection  */
      gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (copy_x - picman_item_get_offset_x (item),
                                        copy_y - picman_item_get_offset_y (item),
                                        copy_width,
                                        copy_height),
                        new_buffer,
                        GEGL_RECTANGLE (copy_x - new_offset_x,
                                        copy_y - new_offset_y, 0, 0));
    }

  picman_drawable_set_buffer_full (drawable, picman_item_is_attached (item), NULL,
                                 new_buffer,
                                 new_offset_x, new_offset_y);
  g_object_unref (new_buffer);
}

static void
picman_drawable_flip (PicmanItem            *item,
                    PicmanContext         *context,
                    PicmanOrientationType  flip_type,
                    gdouble              axis,
                    gboolean             clip_result)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglBuffer   *buffer;
  gint          off_x, off_y;
  gint          new_off_x, new_off_y;

  picman_item_get_offset (item, &off_x, &off_y);

  buffer = picman_drawable_transform_buffer_flip (drawable, context,
                                                picman_drawable_get_buffer (drawable),
                                                off_x, off_y,
                                                flip_type, axis,
                                                clip_result,
                                                &new_off_x, &new_off_y);

  if (buffer)
    {
      picman_drawable_transform_paste (drawable, buffer,
                                     new_off_x, new_off_y, FALSE);
      g_object_unref (buffer);
    }
}

static void
picman_drawable_rotate (PicmanItem         *item,
                      PicmanContext      *context,
                      PicmanRotationType  rotate_type,
                      gdouble           center_x,
                      gdouble           center_y,
                      gboolean          clip_result)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglBuffer   *buffer;
  gint          off_x, off_y;
  gint          new_off_x, new_off_y;

  picman_item_get_offset (item, &off_x, &off_y);

  buffer = picman_drawable_transform_buffer_rotate (drawable, context,
                                                  picman_drawable_get_buffer (drawable),
                                                  off_x, off_y,
                                                  rotate_type, center_x, center_y,
                                                  clip_result,
                                                  &new_off_x, &new_off_y);

  if (buffer)
    {
      picman_drawable_transform_paste (drawable, buffer,
                                     new_off_x, new_off_y, FALSE);
      g_object_unref (buffer);
    }
}

static void
picman_drawable_transform (PicmanItem               *item,
                         PicmanContext            *context,
                         const PicmanMatrix3      *matrix,
                         PicmanTransformDirection  direction,
                         PicmanInterpolationType   interpolation_type,
                         gint                    recursion_level,
                         PicmanTransformResize     clip_result,
                         PicmanProgress           *progress)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (item);
  GeglBuffer   *buffer;
  gint          off_x, off_y;
  gint          new_off_x, new_off_y;

  picman_item_get_offset (item, &off_x, &off_y);

  buffer = picman_drawable_transform_buffer_affine (drawable, context,
                                                  picman_drawable_get_buffer (drawable),
                                                  off_x, off_y,
                                                  matrix, direction,
                                                  interpolation_type,
                                                  recursion_level,
                                                  clip_result,
                                                  &new_off_x, &new_off_y,
                                                  progress);

  if (buffer)
    {
      picman_drawable_transform_paste (drawable, buffer,
                                     new_off_x, new_off_y, FALSE);
      g_object_unref (buffer);
    }
}

static gboolean
picman_drawable_get_pixel_at (PicmanPickable *pickable,
                            gint          x,
                            gint          y,
                            const Babl   *format,
                            gpointer      pixel)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (pickable);

  /* do not make this a g_return_if_fail() */
  if (x < 0 || x >= picman_item_get_width  (PICMAN_ITEM (drawable)) ||
      y < 0 || y >= picman_item_get_height (PICMAN_ITEM (drawable)))
    return FALSE;

  gegl_buffer_sample (picman_drawable_get_buffer (drawable),
                      x, y, NULL, pixel, format,
                      GEGL_SAMPLER_NEAREST, GEGL_ABYSS_NONE);

  return TRUE;
}

static void
picman_drawable_real_update (PicmanDrawable *drawable,
                           gint          x,
                           gint          y,
                           gint          width,
                           gint          height)
{
  if (drawable->private->buffer_source_node)
    {
      GObject *operation = NULL;

      g_object_get (drawable->private->buffer_source_node,
                    "gegl-operation", &operation,
                    NULL);

      if (operation)
        {
          gegl_operation_invalidate (GEGL_OPERATION (operation),
                                     GEGL_RECTANGLE (x,y,width,height), FALSE);
          g_object_unref (operation);
        }
    }

  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (drawable));
}

static gint64
picman_drawable_real_estimate_memsize (const PicmanDrawable *drawable,
                                     gint                width,
                                     gint                height)
{
  const Babl *format = picman_drawable_get_format (drawable);

  return (gint64) babl_format_get_bytes_per_pixel (format) * width * height;
}

/* FIXME: this default impl is currently unused because no subclass
 * chins up. the goal is to handle the almost identical subclass code
 * here again.
 */
static void
picman_drawable_real_convert_type (PicmanDrawable      *drawable,
                                 PicmanImage         *dest_image,
                                 const Babl        *new_format,
                                 PicmanImageBaseType  new_base_type,
                                 PicmanPrecision      new_precision,
                                 gint               layer_dither_type,
                                 gint               mask_dither_type,
                                 gboolean           push_undo)
{
  GeglBuffer *dest_buffer;
  const Babl *format;

  format = picman_image_get_format (dest_image,
                                  new_base_type,
                                  new_precision,
                                  picman_drawable_has_alpha (drawable));

  dest_buffer =
    gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                     picman_item_get_width  (PICMAN_ITEM (drawable)),
                                     picman_item_get_height (PICMAN_ITEM (drawable))),
                     format);

  gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                    dest_buffer, NULL);

  picman_drawable_set_buffer (drawable, push_undo, NULL, dest_buffer);
  g_object_unref (dest_buffer);
}

static GeglBuffer *
picman_drawable_real_get_buffer (PicmanDrawable *drawable)
{
#if 0
  gegl_buffer_flush (drawable->private->buffer);
  picman_gegl_buffer_refetch_tiles (drawable->private->buffer);
#endif

  return drawable->private->buffer;
}

static void
picman_drawable_real_set_buffer (PicmanDrawable *drawable,
                               gboolean      push_undo,
                               const gchar  *undo_desc,
                               GeglBuffer   *buffer,
                               gint          offset_x,
                               gint          offset_y)
{
  PicmanItem *item = PICMAN_ITEM (drawable);
  gboolean  old_has_alpha;

  old_has_alpha = picman_drawable_has_alpha (drawable);

  picman_drawable_invalidate_boundary (drawable);

  if (push_undo)
    picman_image_undo_push_drawable_mod (picman_item_get_image (item), undo_desc,
                                       drawable, FALSE);

  /*  ref new before unrefing old, they might be the same  */
  g_object_ref (buffer);

  if (drawable->private->buffer)
    g_object_unref (drawable->private->buffer);

  drawable->private->buffer = buffer;

  picman_item_set_offset (item, offset_x, offset_y);
  picman_item_set_size (item,
                      gegl_buffer_get_width  (buffer),
                      gegl_buffer_get_height (buffer));

  if (old_has_alpha != picman_drawable_has_alpha (drawable))
    picman_drawable_alpha_changed (drawable);

  if (drawable->private->buffer_source_node)
    gegl_node_set (drawable->private->buffer_source_node,
                   "buffer", picman_drawable_get_buffer (drawable),
                   NULL);
}

static void
picman_drawable_real_push_undo (PicmanDrawable *drawable,
                              const gchar  *undo_desc,
                              GeglBuffer   *buffer,
                              gint          x,
                              gint          y,
                              gint          width,
                              gint          height)
{
  if (! buffer)
    {
      buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                picman_drawable_get_format (drawable));

      gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (x, y, width, height),
                        buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));
    }
  else
    {
      g_object_ref (buffer);
    }

  picman_image_undo_push_drawable (picman_item_get_image (PICMAN_ITEM (drawable)),
                                 undo_desc, drawable,
                                 buffer, x, y);

  g_object_unref (buffer);
}

static void
picman_drawable_real_swap_pixels (PicmanDrawable *drawable,
                                GeglBuffer   *buffer,
                                gint          x,
                                gint          y)
{
  GeglBuffer *tmp;
  gint        width  = gegl_buffer_get_width (buffer);
  gint        height = gegl_buffer_get_height (buffer);

  tmp = gegl_buffer_dup (buffer);

  gegl_buffer_copy (picman_drawable_get_buffer (drawable),
                    GEGL_RECTANGLE (x, y, width, height),
                    buffer,
                    GEGL_RECTANGLE (0, 0, 0, 0));
  gegl_buffer_copy (tmp,
                    GEGL_RECTANGLE (0, 0, width, height),
                    picman_drawable_get_buffer (drawable),
                    GEGL_RECTANGLE (x, y, 0, 0));

  g_object_unref (tmp);

  picman_drawable_update (drawable, x, y, width, height);
}

static void
picman_drawable_sync_fs_filter (PicmanDrawable *drawable,
                              gboolean      detach_fs)
{
  PicmanDrawablePrivate *private = drawable->private;
  PicmanImage           *image   = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanLayer           *fs      = picman_drawable_get_floating_sel (drawable);

  if (! private->source_node)
    return;

  if (fs && ! detach_fs)
    {
      PicmanImage   *image = picman_item_get_image (PICMAN_ITEM (drawable));
      PicmanChannel *mask  = picman_image_get_mask (image);
      gint         off_x, off_y;
      gint         fs_off_x, fs_off_y;

      if (! private->fs_filter)
        {
          GeglNode *node;
          GeglNode *fs_source;
          gboolean  linear;

          private->fs_filter = picman_filter_new ("Floating Selection");
          picman_viewable_set_stock_id (PICMAN_VIEWABLE (private->fs_filter),
                                      "picman-floating-selection");

          node = picman_filter_get_node (private->fs_filter);

          fs_source = picman_drawable_get_source_node (PICMAN_DRAWABLE (fs));
          linear    = picman_drawable_get_linear (PICMAN_DRAWABLE (fs));

          /* rip the fs' source node out of its graph */
          if (fs->layer_offset_node)
            {
              gegl_node_disconnect (fs->layer_offset_node, "input");
              gegl_node_remove_child (picman_filter_get_node (PICMAN_FILTER (fs)),
                                      fs_source);
            }

          gegl_node_add_child (node, fs_source);

          private->fs_applicator = picman_applicator_new (node, linear);

          private->fs_crop_node =
            gegl_node_new_child (node,
                                 "operation", "gegl:crop",
                                 NULL);

          gegl_node_connect_to (fs_source,             "output",
                                private->fs_crop_node, "input");
          gegl_node_connect_to (private->fs_crop_node, "output",
                                node,                  "aux");

          picman_drawable_add_filter (drawable, private->fs_filter);

          g_signal_connect (fs, "notify",
                            G_CALLBACK (picman_drawable_fs_notify),
                            drawable);
          g_signal_connect (image, "component-active-changed",
                            G_CALLBACK (picman_drawable_fs_affect_changed),
                            drawable);
          g_signal_connect (image, "mask-changed",
                            G_CALLBACK (picman_drawable_fs_mask_changed),
                            drawable);
        }

      picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);
      picman_item_get_offset (PICMAN_ITEM (fs), &fs_off_x, &fs_off_y);

      gegl_node_set (private->fs_crop_node,
                     "x",      (gdouble) (off_x - fs_off_x),
                     "y",      (gdouble) (off_y - fs_off_y),
                     "width",  (gdouble) picman_item_get_width  (PICMAN_ITEM (drawable)),
                     "height", (gdouble) picman_item_get_height (PICMAN_ITEM (drawable)),
                     NULL);

      picman_applicator_set_apply_offset (private->fs_applicator,
                                        fs_off_x - off_x,
                                        fs_off_y - off_y);

      if (picman_channel_is_empty (mask))
        {
          picman_applicator_set_mask_buffer (private->fs_applicator, NULL);
        }
      else
        {
          GeglBuffer *buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

          picman_applicator_set_mask_buffer (private->fs_applicator, buffer);
          picman_applicator_set_mask_offset (private->fs_applicator,
                                           -off_x, -off_y);
        }

      picman_applicator_set_mode (private->fs_applicator,
                                picman_layer_get_opacity (fs),
                                picman_layer_get_mode (fs));
      picman_applicator_set_affect (private->fs_applicator,
                                  picman_drawable_get_active_mask (drawable));
    }
  else
    {
      if (private->fs_filter)
        {
          GeglNode *node;
          GeglNode *fs_source;

          g_signal_handlers_disconnect_by_func (fs,
                                                picman_drawable_fs_notify,
                                                drawable);
          g_signal_handlers_disconnect_by_func (image,
                                                picman_drawable_fs_affect_changed,
                                                drawable);
          g_signal_handlers_disconnect_by_func (image,
                                                picman_drawable_fs_mask_changed,
                                                drawable);

          picman_drawable_remove_filter (drawable, private->fs_filter);

          node = picman_filter_get_node (private->fs_filter);

          fs_source = picman_drawable_get_source_node (PICMAN_DRAWABLE (fs));

          gegl_node_remove_child (node, fs_source);

          /* plug the fs' source node back into its graph */
          if (fs->layer_offset_node)
            {
              gegl_node_add_child (picman_filter_get_node (PICMAN_FILTER (fs)),
                                   fs_source);
              gegl_node_connect_to (fs_source,             "output",
                                    fs->layer_offset_node, "input");
            }

          g_object_unref (private->fs_filter);
          private->fs_filter = NULL;

          g_object_unref (private->fs_applicator);
          private->fs_applicator = NULL;

          private->fs_crop_node = NULL;
        }
    }
}

static void
picman_drawable_fs_notify (PicmanLayer        *fs,
                         const GParamSpec *pspec,
                         PicmanDrawable     *drawable)
{
  if (! strcmp (pspec->name, "offset-x") ||
      ! strcmp (pspec->name, "offset-y") ||
      ! strcmp (pspec->name, "visible")  ||
      ! strcmp (pspec->name, "mode")     ||
      ! strcmp (pspec->name, "opacity"))
    {
      picman_drawable_sync_fs_filter (drawable, FALSE);
    }
}

static void
picman_drawable_fs_affect_changed (PicmanImage       *image,
                                 PicmanChannelType  channel,
                                 PicmanDrawable    *drawable)
{
  PicmanLayer *fs = picman_drawable_get_floating_sel (drawable);

  picman_drawable_sync_fs_filter (drawable, FALSE);

  picman_drawable_update (PICMAN_DRAWABLE (fs),
                        0, 0,
                        picman_item_get_width  (PICMAN_ITEM (fs)),
                        picman_item_get_height (PICMAN_ITEM (fs)));
}

static void
picman_drawable_fs_mask_changed (PicmanImage       *image,
                               PicmanDrawable    *drawable)
{
  PicmanLayer *fs = picman_drawable_get_floating_sel (drawable);

  picman_drawable_sync_fs_filter (drawable, FALSE);

  picman_drawable_update (PICMAN_DRAWABLE (fs),
                        0, 0,
                        picman_item_get_width  (PICMAN_ITEM (fs)),
                        picman_item_get_height (PICMAN_ITEM (fs)));
}

static void
picman_drawable_fs_update (PicmanLayer    *fs,
                         gint          x,
                         gint          y,
                         gint          width,
                         gint          height,
                         PicmanDrawable *drawable)
{
  gint fs_off_x, fs_off_y;
  gint off_x, off_y;
  gint dr_x, dr_y, dr_width, dr_height;

  picman_item_get_offset (PICMAN_ITEM (fs), &fs_off_x, &fs_off_y);
  picman_item_get_offset (PICMAN_ITEM (drawable), &off_x, &off_y);

  if (picman_rectangle_intersect (x + fs_off_x,
                                y + fs_off_y,
                                width,
                                height,
                                off_x,
                                off_y,
                                picman_item_get_width  (PICMAN_ITEM (drawable)),
                                picman_item_get_height (PICMAN_ITEM (drawable)),
                                &dr_x,
                                &dr_y,
                                &dr_width,
                                &dr_height))
    {
      picman_drawable_update (drawable,
                            dr_x - off_x, dr_y - off_y,
                            dr_width, dr_height);
    }
}


/*  public functions  */

PicmanDrawable *
picman_drawable_new (GType          type,
                   PicmanImage     *image,
                   const gchar   *name,
                   gint           offset_x,
                   gint           offset_y,
                   gint           width,
                   gint           height,
                   const Babl    *format)
{
  PicmanDrawable *drawable;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (g_type_is_a (type, PICMAN_TYPE_DRAWABLE), NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);
  g_return_val_if_fail (format != NULL, NULL);

  drawable = PICMAN_DRAWABLE (picman_item_new (type,
                                           image, name,
                                           offset_x, offset_y,
                                           width, height));

  drawable->private->buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                               width, height),
                                               format);

  return drawable;
}

gint64
picman_drawable_estimate_memsize (const PicmanDrawable *drawable,
                                gint                width,
                                gint                height)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), 0);

  return PICMAN_DRAWABLE_GET_CLASS (drawable)->estimate_memsize (drawable,
                                                               width, height);
}

void
picman_drawable_update (PicmanDrawable *drawable,
                      gint          x,
                      gint          y,
                      gint          width,
                      gint          height)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  g_signal_emit (drawable, picman_drawable_signals[UPDATE], 0,
                 x, y, width, height);
}

void
picman_drawable_alpha_changed (PicmanDrawable *drawable)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  g_signal_emit (drawable, picman_drawable_signals[ALPHA_CHANGED], 0);
}

void
picman_drawable_invalidate_boundary (PicmanDrawable *drawable)
{
  PicmanDrawableClass *drawable_class;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  drawable_class = PICMAN_DRAWABLE_GET_CLASS (drawable);

  if (drawable_class->invalidate_boundary)
    drawable_class->invalidate_boundary (drawable);
}

void
picman_drawable_get_active_components (const PicmanDrawable *drawable,
                                     gboolean           *active)
{
  PicmanDrawableClass *drawable_class;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (active != NULL);

  drawable_class = PICMAN_DRAWABLE_GET_CLASS (drawable);

  if (drawable_class->get_active_components)
    drawable_class->get_active_components (drawable, active);
}

PicmanComponentMask
picman_drawable_get_active_mask (const PicmanDrawable *drawable)
{
  PicmanDrawableClass *drawable_class;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), 0);

  drawable_class = PICMAN_DRAWABLE_GET_CLASS (drawable);

  if (drawable_class->get_active_mask)
    return drawable_class->get_active_mask (drawable);

  return 0;
}

void
picman_drawable_convert_type (PicmanDrawable      *drawable,
                            PicmanImage         *dest_image,
                            PicmanImageBaseType  new_base_type,
                            PicmanPrecision      new_precision,
                            gint               layer_dither_type,
                            gint               mask_dither_type,
                            gboolean           push_undo)
{
  const Babl *new_format;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (PICMAN_IS_IMAGE (dest_image));
  g_return_if_fail (new_base_type != picman_drawable_get_base_type (drawable) ||
                    new_precision != picman_drawable_get_precision (drawable));

  if (! picman_item_is_attached (PICMAN_ITEM (drawable)))
    push_undo = FALSE;

  new_format = picman_image_get_format (dest_image,
                                      new_base_type,
                                      new_precision,
                                      picman_drawable_has_alpha (drawable));

  PICMAN_DRAWABLE_GET_CLASS (drawable)->convert_type (drawable, dest_image,
                                                    new_format,
                                                    new_base_type,
                                                    new_precision,
                                                    layer_dither_type,
                                                    mask_dither_type,
                                                    push_undo);
}

void
picman_drawable_apply_buffer (PicmanDrawable         *drawable,
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
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (GEGL_IS_BUFFER (buffer));
  g_return_if_fail (buffer_region != NULL);
  g_return_if_fail (base_buffer == NULL || GEGL_IS_BUFFER (base_buffer));

  PICMAN_DRAWABLE_GET_CLASS (drawable)->apply_buffer (drawable, buffer,
                                                    buffer_region,
                                                    push_undo, undo_desc,
                                                    opacity, mode,
                                                    base_buffer,
                                                    base_x, base_y);
}

void
picman_drawable_replace_buffer (PicmanDrawable        *drawable,
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
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (GEGL_IS_BUFFER (buffer));
  g_return_if_fail (GEGL_IS_BUFFER (mask));

  PICMAN_DRAWABLE_GET_CLASS (drawable)->replace_buffer (drawable, buffer,
                                                      buffer_region,
                                                      push_undo, undo_desc,
                                                      opacity,
                                                      mask, mask_region,
                                                      x, y);
}

GeglBuffer *
picman_drawable_get_buffer (PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return PICMAN_DRAWABLE_GET_CLASS (drawable)->get_buffer (drawable);
}

void
picman_drawable_set_buffer (PicmanDrawable *drawable,
                          gboolean      push_undo,
                          const gchar  *undo_desc,
                          GeglBuffer   *buffer)
{
  gint offset_x, offset_y;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (GEGL_IS_BUFFER (buffer));

  if (! picman_item_is_attached (PICMAN_ITEM (drawable)))
    push_undo = FALSE;

  picman_item_get_offset (PICMAN_ITEM (drawable), &offset_x, &offset_y);

  picman_drawable_set_buffer_full (drawable, push_undo, undo_desc, buffer,
                                 offset_x, offset_y);
}

void
picman_drawable_set_buffer_full (PicmanDrawable *drawable,
                               gboolean      push_undo,
                               const gchar  *undo_desc,
                               GeglBuffer   *buffer,
                               gint          offset_x,
                               gint          offset_y)
{
  PicmanItem *item;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (GEGL_IS_BUFFER (buffer));

  item = PICMAN_ITEM (drawable);

  if (! picman_item_is_attached (PICMAN_ITEM (drawable)))
    push_undo = FALSE;

  if (picman_item_get_width  (item)   != gegl_buffer_get_width (buffer)  ||
      picman_item_get_height (item)   != gegl_buffer_get_height (buffer) ||
      picman_item_get_offset_x (item) != offset_x                        ||
      picman_item_get_offset_y (item) != offset_y)
    {
      picman_drawable_update (drawable,
                            0, 0,
                            picman_item_get_width  (item),
                            picman_item_get_height (item));
    }

  g_object_freeze_notify (G_OBJECT (drawable));

  PICMAN_DRAWABLE_GET_CLASS (drawable)->set_buffer (drawable,
                                                  push_undo, undo_desc,
                                                  buffer,
                                                  offset_x, offset_y);

  g_object_thaw_notify (G_OBJECT (drawable));

  picman_drawable_update (drawable,
                        0, 0,
                        picman_item_get_width  (item),
                        picman_item_get_height (item));
}

GeglNode *
picman_drawable_get_source_node (PicmanDrawable *drawable)
{
  GeglNode *filter;
  GeglNode *output;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  if (drawable->private->source_node)
    return drawable->private->source_node;

  drawable->private->source_node = gegl_node_new ();

  drawable->private->buffer_source_node =
    gegl_node_new_child (drawable->private->source_node,
                         "operation", "gegl:buffer-source",
                         "buffer",    picman_drawable_get_buffer (drawable),
                         NULL);

  filter = picman_filter_stack_get_graph (PICMAN_FILTER_STACK (drawable->private->filter_stack));

  gegl_node_add_child (drawable->private->source_node, filter);

  gegl_node_connect_to (drawable->private->buffer_source_node, "output",
                        filter,                                "input");

  output = gegl_node_get_output_proxy (drawable->private->source_node, "output");

  gegl_node_connect_to (filter, "output",
                        output, "input");

  picman_drawable_sync_fs_filter (drawable, FALSE);

  return drawable->private->source_node;
}

GeglNode *
picman_drawable_get_mode_node (PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  if (! drawable->private->mode_node)
    picman_filter_get_node (PICMAN_FILTER (drawable));

  return drawable->private->mode_node;
}

void
picman_drawable_swap_pixels (PicmanDrawable *drawable,
                           GeglBuffer   *buffer,
                           gint          x,
                           gint          y)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (GEGL_IS_BUFFER (buffer));

  PICMAN_DRAWABLE_GET_CLASS (drawable)->swap_pixels (drawable, buffer, x, y);
}

void
picman_drawable_push_undo (PicmanDrawable *drawable,
                         const gchar  *undo_desc,
                         GeglBuffer   *buffer,
                         gint          x,
                         gint          y,
                         gint          width,
                         gint          height)
{
  PicmanItem *item;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (buffer == NULL || GEGL_IS_BUFFER (buffer));

  item = PICMAN_ITEM (drawable);

  g_return_if_fail (picman_item_is_attached (item));

  if (! buffer &&
      ! picman_rectangle_intersect (x, y,
                                  width, height,
                                  0, 0,
                                  picman_item_get_width (item),
                                  picman_item_get_height (item),
                                  &x, &y, &width, &height))
    {
      g_warning ("%s: tried to push empty region", G_STRFUNC);
      return;
    }

  PICMAN_DRAWABLE_GET_CLASS (drawable)->push_undo (drawable, undo_desc,
                                                 buffer,
                                                 x, y, width, height);
}

void
picman_drawable_fill (PicmanDrawable      *drawable,
                    const PicmanRGB     *color,
                    const PicmanPattern *pattern)
{
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (color != NULL || pattern != NULL);
  g_return_if_fail (pattern == NULL || PICMAN_IS_PATTERN (pattern));

  if (color)
    {
      PicmanRGB    c = *color;
      GeglColor *col;

      if (! picman_drawable_has_alpha (drawable))
        picman_rgb_set_alpha (&c, 1.0);

      col = picman_gegl_color_new (&c);
      gegl_buffer_set_color (picman_drawable_get_buffer (drawable),
                             NULL, col);
      g_object_unref (col);
    }
  else
    {
      GeglBuffer *src_buffer = picman_pattern_create_buffer (pattern);

      gegl_buffer_set_pattern (picman_drawable_get_buffer (drawable),
                               NULL, src_buffer, 0, 0);
      g_object_unref (src_buffer);
    }

  picman_drawable_update (drawable,
                        0, 0,
                        picman_item_get_width  (PICMAN_ITEM (drawable)),
                        picman_item_get_height (PICMAN_ITEM (drawable)));
}

void
picman_drawable_fill_by_type (PicmanDrawable *drawable,
                            PicmanContext  *context,
                            PicmanFillType  fill_type)
{
  PicmanRGB      color;
  PicmanPattern *pattern = NULL;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));

  switch (fill_type)
    {
    case PICMAN_FOREGROUND_FILL:
      picman_context_get_foreground (context, &color);
      break;

    case PICMAN_BACKGROUND_FILL:
      picman_context_get_background (context, &color);
      break;

    case PICMAN_WHITE_FILL:
      picman_rgba_set (&color, 1.0, 1.0, 1.0, PICMAN_OPACITY_OPAQUE);
      break;

    case PICMAN_TRANSPARENT_FILL:
      picman_rgba_set (&color, 0.0, 0.0, 0.0, PICMAN_OPACITY_TRANSPARENT);
      break;

    case PICMAN_PATTERN_FILL:
      pattern = picman_context_get_pattern (context);
      break;

    case PICMAN_NO_FILL:
      return;

    default:
      g_warning ("%s: unknown fill type %d", G_STRFUNC, fill_type);
      return;
    }

  picman_drawable_fill (drawable, pattern ? NULL : &color, pattern);
}

const Babl *
picman_drawable_get_format (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return gegl_buffer_get_format (drawable->private->buffer);
}

const Babl *
picman_drawable_get_format_with_alpha (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return picman_image_get_format (picman_item_get_image (PICMAN_ITEM (drawable)),
                                picman_drawable_get_base_type (drawable),
                                picman_drawable_get_precision (drawable),
                                TRUE);
}

const Babl *
picman_drawable_get_format_without_alpha (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return picman_image_get_format (picman_item_get_image (PICMAN_ITEM (drawable)),
                                picman_drawable_get_base_type (drawable),
                                picman_drawable_get_precision (drawable),
                                FALSE);
}

gboolean
picman_drawable_get_linear (const PicmanDrawable *drawable)
{
  const Babl *format;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  format = gegl_buffer_get_format (drawable->private->buffer);

  return picman_babl_format_get_linear (format);
}

gboolean
picman_drawable_has_alpha (const PicmanDrawable *drawable)
{
  const Babl *format;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  format = gegl_buffer_get_format (drawable->private->buffer);

  return babl_format_has_alpha (format);
}

PicmanImageBaseType
picman_drawable_get_base_type (const PicmanDrawable *drawable)
{
  const Babl *format;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), -1);

  format = gegl_buffer_get_format (drawable->private->buffer);

  return picman_babl_format_get_base_type (format);
}

PicmanPrecision
picman_drawable_get_precision (const PicmanDrawable *drawable)
{
  const Babl *format;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), -1);

  format = gegl_buffer_get_format (drawable->private->buffer);

  return picman_babl_format_get_precision (format);
}

gboolean
picman_drawable_is_rgb (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  return (picman_drawable_get_base_type (drawable) == PICMAN_RGB);
}

gboolean
picman_drawable_is_gray (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  return (picman_drawable_get_base_type (drawable) == PICMAN_GRAY);
}

gboolean
picman_drawable_is_indexed (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);

  return (picman_drawable_get_base_type (drawable) == PICMAN_INDEXED);
}

const guchar *
picman_drawable_get_colormap (const PicmanDrawable *drawable)
{
  PicmanImage *image;

  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  return image ? picman_image_get_colormap (image) : NULL;
}

PicmanLayer *
picman_drawable_get_floating_sel (const PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);

  return drawable->private->floating_selection;
}

void
picman_drawable_attach_floating_sel (PicmanDrawable *drawable,
                                   PicmanLayer    *fs)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (picman_drawable_get_floating_sel (drawable) == NULL);
  g_return_if_fail (PICMAN_IS_LAYER (fs));

  PICMAN_LOG (FLOATING_SELECTION, "%s", G_STRFUNC);

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  drawable->private->floating_selection = fs;
  picman_image_set_floating_selection (image, fs);

  /*  clear the selection  */
  picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (fs));

  picman_drawable_sync_fs_filter (drawable, FALSE);

  g_signal_connect (fs, "update",
                    G_CALLBACK (picman_drawable_fs_update),
                    drawable);

  picman_drawable_fs_update (fs,
                           0, 0,
                           picman_item_get_width  (PICMAN_ITEM (fs)),
                           picman_item_get_height (PICMAN_ITEM (fs)),
                           drawable);
}

void
picman_drawable_detach_floating_sel (PicmanDrawable *drawable)
{
  PicmanImage *image;
  PicmanLayer *fs;

  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_drawable_get_floating_sel (drawable) != NULL);

  PICMAN_LOG (FLOATING_SELECTION, "%s", G_STRFUNC);

  image = picman_item_get_image (PICMAN_ITEM (drawable));
  fs    = drawable->private->floating_selection;

  picman_drawable_sync_fs_filter (drawable, TRUE);

  g_signal_handlers_disconnect_by_func (fs,
                                        picman_drawable_fs_update,
                                        drawable);

  picman_drawable_fs_update (fs,
                           0, 0,
                           picman_item_get_width  (PICMAN_ITEM (fs)),
                           picman_item_get_height (PICMAN_ITEM (fs)),
                           drawable);

  /*  clear the selection  */
  picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (fs));

  picman_image_set_floating_selection (image, NULL);
  drawable->private->floating_selection = NULL;
}

PicmanFilter *
picman_drawable_get_floating_sel_filter (PicmanDrawable *drawable)
{
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_drawable_get_floating_sel (drawable) != NULL, NULL);

  return drawable->private->fs_filter;
}
