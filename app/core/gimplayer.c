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

#include <cairo.h>
#include <gegl.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmancolor/picmancolor.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "gegl/picman-gegl-apply-operation.h"
#include "gegl/picman-gegl-nodes.h"

#include "picmanboundary.h"
#include "picmanchannel-select.h"
#include "picmancontext.h"
#include "picmancontainer.h"
#include "picmanerror.h"
#include "picmanimage-undo-push.h"
#include "picmanimage-undo.h"
#include "picmanimage.h"
#include "picmanlayer-floating-sel.h"
#include "picmanlayer.h"
#include "picmanlayermask.h"
#include "picmanmarshal.h"
#include "picmanpickable.h"

#include "picman-intl.h"


enum
{
  OPACITY_CHANGED,
  MODE_CHANGED,
  LOCK_ALPHA_CHANGED,
  MASK_CHANGED,
  APPLY_MASK_CHANGED,
  EDIT_MASK_CHANGED,
  SHOW_MASK_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_OPACITY,
  PROP_MODE,
  PROP_LOCK_ALPHA,
  PROP_MASK,
  PROP_FLOATING_SELECTION
};


static void   picman_layer_pickable_iface_init (PicmanPickableInterface *iface);

static void       picman_layer_set_property       (GObject            *object,
                                                 guint               property_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void       picman_layer_get_property       (GObject            *object,
                                                 guint               property_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);
static void       picman_layer_dispose            (GObject            *object);
static void       picman_layer_finalize           (GObject            *object);
static void       picman_layer_notify             (GObject            *object,
                                                 GParamSpec         *pspec);

static void       picman_layer_name_changed       (PicmanObject         *object);
static gint64     picman_layer_get_memsize        (PicmanObject         *object,
                                                 gint64             *gui_size);

static void       picman_layer_invalidate_preview (PicmanViewable       *viewable);
static gchar    * picman_layer_get_description    (PicmanViewable       *viewable,
                                                 gchar             **tooltip);

static GeglNode * picman_layer_get_node           (PicmanFilter         *filter);

static void       picman_layer_removed            (PicmanItem           *item);
static void       picman_layer_unset_removed      (PicmanItem           *item);
static gboolean   picman_layer_is_attached        (const PicmanItem     *item);
static PicmanItemTree * picman_layer_get_tree       (PicmanItem           *item);
static PicmanItem * picman_layer_duplicate          (PicmanItem           *item,
                                                 GType               new_type);
static void       picman_layer_convert            (PicmanItem           *item,
                                                 PicmanImage          *dest_image);
static gboolean   picman_layer_rename             (PicmanItem           *item,
                                                 const gchar        *new_name,
                                                 const gchar        *undo_desc,
                                                 GError            **error);
static void       picman_layer_translate          (PicmanItem           *item,
                                                 gint                offset_x,
                                                 gint                offset_y,
                                                 gboolean            push_undo);
static void       picman_layer_scale              (PicmanItem           *item,
                                                 gint                new_width,
                                                 gint                new_height,
                                                 gint                new_offset_x,
                                                 gint                new_offset_y,
                                                 PicmanInterpolationType  interp_type,
                                                 PicmanProgress       *progress);
static void       picman_layer_resize             (PicmanItem           *item,
                                                 PicmanContext        *context,
                                                 gint                new_width,
                                                 gint                new_height,
                                                 gint                offset_x,
                                                 gint                offset_y);
static void       picman_layer_flip               (PicmanItem           *item,
                                                 PicmanContext        *context,
                                                 PicmanOrientationType flip_type,
                                                 gdouble             axis,
                                                 gboolean            clip_result);
static void       picman_layer_rotate             (PicmanItem           *item,
                                                 PicmanContext        *context,
                                                 PicmanRotationType    rotate_type,
                                                 gdouble             center_x,
                                                 gdouble             center_y,
                                                 gboolean            clip_result);
static void       picman_layer_transform          (PicmanItem           *item,
                                                 PicmanContext        *context,
                                                 const PicmanMatrix3  *matrix,
                                                 PicmanTransformDirection direction,
                                                 PicmanInterpolationType  interpolation_type,
                                                 gint                recursion_level,
                                                 PicmanTransformResize clip_result,
                                                 PicmanProgress       *progress);
static void       picman_layer_to_selection       (PicmanItem           *item,
                                                 PicmanChannelOps      op,
                                                 gboolean            antialias,
                                                 gboolean            feather,
                                                 gdouble             feather_radius_x,
                                                 gdouble             feather_radius_y);

static gint64  picman_layer_estimate_memsize      (const PicmanDrawable *drawable,
                                                 gint                width,
                                                 gint                height);
static void    picman_layer_convert_type          (PicmanDrawable       *drawable,
                                                 PicmanImage          *dest_image,
                                                 const Babl         *new_format,
                                                 PicmanImageBaseType   new_base_type,
                                                 PicmanPrecision       new_precision,
                                                 gint                layer_dither_type,
                                                 gint                mask_dither_type,
                                                 gboolean            push_undo);
static void    picman_layer_invalidate_boundary   (PicmanDrawable       *drawable);
static void    picman_layer_get_active_components (const PicmanDrawable *drawable,
                                                 gboolean           *active);
static PicmanComponentMask
               picman_layer_get_active_mask       (const PicmanDrawable *drawable);

static gdouble picman_layer_get_opacity_at        (PicmanPickable       *pickable,
                                                 gint                x,
                                                 gint                y);

static void       picman_layer_layer_mask_update  (PicmanDrawable       *layer_mask,
                                                 gint                x,
                                                 gint                y,
                                                 gint                width,
                                                 gint                height,
                                                 PicmanLayer          *layer);


G_DEFINE_TYPE_WITH_CODE (PicmanLayer, picman_layer, PICMAN_TYPE_DRAWABLE,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_layer_pickable_iface_init))

#define parent_class picman_layer_parent_class

static guint layer_signals[LAST_SIGNAL] = { 0 };


static void
picman_layer_class_init (PicmanLayerClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanFilterClass   *filter_class      = PICMAN_FILTER_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class    = PICMAN_DRAWABLE_CLASS (klass);

  layer_signals[OPACITY_CHANGED] =
    g_signal_new ("opacity-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, opacity_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[MODE_CHANGED] =
    g_signal_new ("mode-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, mode_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[LOCK_ALPHA_CHANGED] =
    g_signal_new ("lock-alpha-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, lock_alpha_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[MASK_CHANGED] =
    g_signal_new ("mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[APPLY_MASK_CHANGED] =
    g_signal_new ("apply-mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, apply_mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[EDIT_MASK_CHANGED] =
    g_signal_new ("edit-mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, edit_mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  layer_signals[SHOW_MASK_CHANGED] =
    g_signal_new ("show-mask-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanLayerClass, show_mask_changed),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->set_property          = picman_layer_set_property;
  object_class->get_property          = picman_layer_get_property;
  object_class->dispose               = picman_layer_dispose;
  object_class->finalize              = picman_layer_finalize;
  object_class->notify                = picman_layer_notify;

  picman_object_class->name_changed     = picman_layer_name_changed;
  picman_object_class->get_memsize      = picman_layer_get_memsize;

  viewable_class->default_stock_id    = "picman-layer";
  viewable_class->invalidate_preview  = picman_layer_invalidate_preview;
  viewable_class->get_description     = picman_layer_get_description;

  filter_class->get_node              = picman_layer_get_node;

  item_class->removed                 = picman_layer_removed;
  item_class->unset_removed           = picman_layer_unset_removed;
  item_class->is_attached             = picman_layer_is_attached;
  item_class->get_tree                = picman_layer_get_tree;
  item_class->duplicate               = picman_layer_duplicate;
  item_class->convert                 = picman_layer_convert;
  item_class->rename                  = picman_layer_rename;
  item_class->translate               = picman_layer_translate;
  item_class->scale                   = picman_layer_scale;
  item_class->resize                  = picman_layer_resize;
  item_class->flip                    = picman_layer_flip;
  item_class->rotate                  = picman_layer_rotate;
  item_class->transform               = picman_layer_transform;
  item_class->to_selection            = picman_layer_to_selection;
  item_class->default_name            = _("Layer");
  item_class->rename_desc             = C_("undo-type", "Rename Layer");
  item_class->translate_desc          = C_("undo-type", "Move Layer");
  item_class->scale_desc              = C_("undo-type", "Scale Layer");
  item_class->resize_desc             = C_("undo-type", "Resize Layer");
  item_class->flip_desc               = C_("undo-type", "Flip Layer");
  item_class->rotate_desc             = C_("undo-type", "Rotate Layer");
  item_class->transform_desc          = C_("undo-type", "Transform Layer");
  item_class->to_selection_desc       = C_("undo-type", "Alpha to Selection");
  item_class->reorder_desc            = C_("undo-type", "Reorder Layer");
  item_class->raise_desc              = C_("undo-type", "Raise Layer");
  item_class->raise_to_top_desc       = C_("undo-type", "Raise Layer to Top");
  item_class->lower_desc              = C_("undo-type", "Lower Layer");
  item_class->lower_to_bottom_desc    = C_("undo-type", "Lower Layer to Bottom");
  item_class->raise_failed            = _("Layer cannot be raised higher.");
  item_class->lower_failed            = _("Layer cannot be lowered more.");

  drawable_class->estimate_memsize      = picman_layer_estimate_memsize;
  drawable_class->convert_type          = picman_layer_convert_type;
  drawable_class->invalidate_boundary   = picman_layer_invalidate_boundary;
  drawable_class->get_active_components = picman_layer_get_active_components;
  drawable_class->get_active_mask       = picman_layer_get_active_mask;

  klass->opacity_changed              = NULL;
  klass->mode_changed                 = NULL;
  klass->lock_alpha_changed           = NULL;
  klass->mask_changed                 = NULL;
  klass->apply_mask_changed           = NULL;
  klass->edit_mask_changed            = NULL;
  klass->show_mask_changed            = NULL;

  g_object_class_install_property (object_class, PROP_OPACITY,
                                   g_param_spec_double ("opacity", NULL, NULL,
                                                        PICMAN_OPACITY_TRANSPARENT,
                                                        PICMAN_OPACITY_OPAQUE,
                                                        PICMAN_OPACITY_OPAQUE,
                                                        PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_MODE,
                                   g_param_spec_enum ("mode", NULL, NULL,
                                                      PICMAN_TYPE_LAYER_MODE_EFFECTS,
                                                      PICMAN_NORMAL_MODE,
                                                      PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_LOCK_ALPHA,
                                   g_param_spec_boolean ("lock-alpha",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_MASK,
                                   g_param_spec_object ("mask",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_LAYER_MASK,
                                                        PICMAN_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_FLOATING_SELECTION,
                                   g_param_spec_boolean ("floating-selection",
                                                         NULL, NULL,
                                                         FALSE,
                                                         PICMAN_PARAM_READABLE));
}

static void
picman_layer_init (PicmanLayer *layer)
{
  layer->opacity    = PICMAN_OPACITY_OPAQUE;
  layer->mode       = PICMAN_NORMAL_MODE;
  layer->lock_alpha = FALSE;

  layer->mask       = NULL;
  layer->apply_mask = TRUE;
  layer->edit_mask  = TRUE;
  layer->show_mask  = FALSE;

  /*  floating selection  */
  layer->fs.drawable       = NULL;
  layer->fs.boundary_known = FALSE;
  layer->fs.segs           = NULL;
  layer->fs.num_segs       = 0;
}

static void
picman_layer_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->get_opacity_at = picman_layer_get_opacity_at;
}

static void
picman_layer_set_property (GObject      *object,
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
picman_layer_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  PicmanLayer *layer = PICMAN_LAYER (object);

  switch (property_id)
    {
    case PROP_OPACITY:
      g_value_set_double (value, picman_layer_get_opacity (layer));
      break;
    case PROP_MODE:
      g_value_set_enum (value, picman_layer_get_mode (layer));
      break;
    case PROP_LOCK_ALPHA:
      g_value_set_boolean (value, picman_layer_get_lock_alpha (layer));
      break;
    case PROP_MASK:
      g_value_set_object (value, picman_layer_get_mask (layer));
      break;
    case PROP_FLOATING_SELECTION:
      g_value_set_boolean (value, picman_layer_is_floating_sel (layer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_layer_dispose (GObject *object)
{
  PicmanLayer *layer = PICMAN_LAYER (object);

  if (layer->mask)
    g_signal_handlers_disconnect_by_func (layer->mask,
                                          picman_layer_layer_mask_update,
                                          layer);

  if (picman_layer_is_floating_sel (layer))
    {
      PicmanDrawable *fs_drawable = picman_layer_get_floating_sel_drawable (layer);

      /* only detach if this is actually the drawable's fs because the
       * layer might be on the undo stack and not attached to anyhing
       */
      if (picman_drawable_get_floating_sel (fs_drawable) == layer)
        picman_drawable_detach_floating_sel (fs_drawable);
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
picman_layer_finalize (GObject *object)
{
  PicmanLayer *layer = PICMAN_LAYER (object);

  if (layer->mask)
    {
      g_object_unref (layer->mask);
      layer->mask = NULL;
    }

  if (layer->fs.segs)
    {
      g_free (layer->fs.segs);
      layer->fs.segs     = NULL;
      layer->fs.num_segs = 0;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static PicmanLayerModeEffects
picman_layer_get_visible_mode (PicmanLayer *layer)
{
  if (layer->mode != PICMAN_DISSOLVE_MODE &&
      picman_filter_get_is_last_node (PICMAN_FILTER (layer)))
    return PICMAN_NORMAL_MODE;

  return layer->mode;
}

static void
picman_layer_notify (GObject    *object,
                   GParamSpec *pspec)
{
  if (! strcmp (pspec->name, "is-last-node") &&
      picman_filter_peek_node (PICMAN_FILTER (object)))
    {
      PicmanLayer *layer = PICMAN_LAYER (object);
      GeglNode  *mode_node;
      gboolean   linear;

      mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));
      linear    = picman_drawable_get_linear (PICMAN_DRAWABLE (layer));

      picman_gegl_mode_node_set_mode (mode_node,
                                    picman_layer_get_visible_mode (layer),
                                    linear);

      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));
    }
}

static void
picman_layer_name_changed (PicmanObject *object)
{
  PicmanLayer *layer = PICMAN_LAYER (object);

  if (PICMAN_OBJECT_CLASS (parent_class)->name_changed)
    PICMAN_OBJECT_CLASS (parent_class)->name_changed (object);

  if (layer->mask)
    {
      gchar *mask_name = g_strdup_printf (_("%s mask"),
                                          picman_object_get_name (object));

      picman_object_take_name (PICMAN_OBJECT (layer->mask), mask_name);
    }
}

static gint64
picman_layer_get_memsize (PicmanObject *object,
                        gint64     *gui_size)
{
  PicmanLayer *layer   = PICMAN_LAYER (object);
  gint64     memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (layer->mask), gui_size);

  *gui_size += layer->fs.num_segs * sizeof (PicmanBoundSeg);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static void
picman_layer_invalidate_preview (PicmanViewable *viewable)
{
  PicmanLayer *layer = PICMAN_LAYER (viewable);

  PICMAN_VIEWABLE_CLASS (parent_class)->invalidate_preview (viewable);

  if (picman_layer_is_floating_sel (layer))
    floating_sel_invalidate (layer);
}

static gchar *
picman_layer_get_description (PicmanViewable  *viewable,
                            gchar        **tooltip)
{
  if (picman_layer_is_floating_sel (PICMAN_LAYER (viewable)))
    {
      return g_strdup_printf (_("Floating Selection\n(%s)"),
                              picman_object_get_name (viewable));
    }

  return PICMAN_VIEWABLE_CLASS (parent_class)->get_description (viewable,
                                                              tooltip);
}

static GeglNode *
picman_layer_get_node (PicmanFilter *filter)
{
  PicmanDrawable *drawable = PICMAN_DRAWABLE (filter);
  PicmanLayer    *layer    = PICMAN_LAYER (filter);
  GeglNode     *node;
  GeglNode     *source;
  GeglNode     *mode_node;
  gboolean      linear;
  gboolean      source_node_hijacked = FALSE;

  node = PICMAN_FILTER_CLASS (parent_class)->get_node (filter);

  source = picman_drawable_get_source_node (drawable);

  /* if the source node already has a parent, we are a floating
   * selection and the source node has been hijacked by the fs'
   * drawable
   */
  if (gegl_node_get_parent (source))
    source_node_hijacked = TRUE;

  if (! source_node_hijacked)
    gegl_node_add_child (node, source);

  g_warn_if_fail (layer->layer_offset_node == NULL);
  g_warn_if_fail (layer->mask_offset_node == NULL);

  /* the mode node connects it all, and has aux and aux2 inputs for
   * the layer and its mask
   */
  mode_node = picman_drawable_get_mode_node (drawable);
  linear    = picman_drawable_get_linear (drawable);

  picman_gegl_mode_node_set_mode (mode_node,
                                picman_layer_get_visible_mode (layer),
                                linear);
  picman_gegl_mode_node_set_opacity (mode_node,
                                   layer->opacity);

  /* the layer's offset node */
  layer->layer_offset_node = gegl_node_new_child (node,
                                                  "operation", "gegl:translate",
                                                  NULL);
  picman_item_add_offset_node (PICMAN_ITEM (layer), layer->layer_offset_node);

  /* the layer mask's offset node */
  layer->mask_offset_node = gegl_node_new_child (node,
                                                 "operation", "gegl:translate",
                                                  NULL);
  picman_item_add_offset_node (PICMAN_ITEM (layer), layer->mask_offset_node);

  if (! source_node_hijacked)
    {
      gegl_node_connect_to (source,                   "output",
                            layer->layer_offset_node, "input");
    }

  if (! (layer->mask && picman_layer_get_show_mask (layer)))
    {
      gegl_node_connect_to (layer->layer_offset_node, "output",
                            mode_node,                "aux");
    }

  if (layer->mask)
    {
      GeglNode *mask;

      mask = picman_drawable_get_source_node (PICMAN_DRAWABLE (layer->mask));

      gegl_node_connect_to (mask,                    "output",
                            layer->mask_offset_node, "input");

      if (picman_layer_get_show_mask (layer))
        {
          gegl_node_connect_to (layer->mask_offset_node, "output",
                                mode_node,               "aux");
        }
      else if (picman_layer_get_apply_mask (layer))
        {
          gegl_node_connect_to (layer->mask_offset_node, "output",
                                mode_node,               "aux2");
        }
    }

  return node;
}

static void
picman_layer_removed (PicmanItem *item)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  if (layer->mask)
    picman_item_removed (PICMAN_ITEM (layer->mask));

  if (PICMAN_ITEM_CLASS (parent_class)->removed)
    PICMAN_ITEM_CLASS (parent_class)->removed (item);
}

static void
picman_layer_unset_removed (PicmanItem *item)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  if (layer->mask)
    picman_item_unset_removed (PICMAN_ITEM (layer->mask));

  if (PICMAN_ITEM_CLASS (parent_class)->unset_removed)
    PICMAN_ITEM_CLASS (parent_class)->unset_removed (item);
}

static gboolean
picman_layer_is_attached (const PicmanItem *item)
{
  PicmanImage *image = picman_item_get_image (item);

  return (PICMAN_IS_IMAGE (image) &&
          picman_container_have (picman_image_get_layers (image),
                               PICMAN_OBJECT (item)));
}

static PicmanItemTree *
picman_layer_get_tree (PicmanItem *item)
{
  if (picman_item_is_attached (item))
    {
      PicmanImage *image = picman_item_get_image (item);

      return picman_image_get_layer_tree (image);
    }

  return NULL;
}

static PicmanItem *
picman_layer_duplicate (PicmanItem *item,
                      GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_LAYER (new_item))
    {
      PicmanLayer *layer     = PICMAN_LAYER (item);
      PicmanLayer *new_layer = PICMAN_LAYER (new_item);

      picman_layer_set_mode    (new_layer, picman_layer_get_mode (layer),    FALSE);
      picman_layer_set_opacity (new_layer, picman_layer_get_opacity (layer), FALSE);

      if (picman_layer_can_lock_alpha (new_layer))
        picman_layer_set_lock_alpha (new_layer,
                                   picman_layer_get_lock_alpha (layer), FALSE);

      /*  duplicate the layer mask if necessary  */
      if (layer->mask)
        {
          PicmanItem *mask;

          mask = picman_item_duplicate (PICMAN_ITEM (layer->mask),
                                      G_TYPE_FROM_INSTANCE (layer->mask));
          picman_layer_add_mask (new_layer, PICMAN_LAYER_MASK (mask), FALSE, NULL);

          new_layer->apply_mask = layer->apply_mask;
          new_layer->edit_mask  = layer->edit_mask;
          new_layer->show_mask  = layer->show_mask;
        }
    }

  return new_item;
}

static void
picman_layer_convert (PicmanItem  *item,
                    PicmanImage *dest_image)
{
  PicmanLayer         *layer    = PICMAN_LAYER (item);
  PicmanDrawable      *drawable = PICMAN_DRAWABLE (item);
  PicmanImageBaseType  old_base_type;
  PicmanImageBaseType  new_base_type;
  PicmanPrecision      old_precision;
  PicmanPrecision      new_precision;

  old_base_type = picman_drawable_get_base_type (drawable);
  new_base_type = picman_image_get_base_type (dest_image);

  old_precision = picman_drawable_get_precision (drawable);
  new_precision = picman_image_get_precision (dest_image);

  if (old_base_type != new_base_type ||
      old_precision != new_precision)
    {
      picman_drawable_convert_type (drawable, dest_image,
                                  new_base_type, new_precision,
                                  0, 0,
                                  FALSE);
    }

  if (layer->mask)
    picman_item_set_image (PICMAN_ITEM (layer->mask), dest_image);

  PICMAN_ITEM_CLASS (parent_class)->convert (item, dest_image);
}

static gboolean
picman_layer_rename (PicmanItem     *item,
                   const gchar  *new_name,
                   const gchar  *undo_desc,
                   GError      **error)
{
  PicmanLayer *layer = PICMAN_LAYER (item);
  PicmanImage *image = picman_item_get_image (item);
  gboolean   attached;
  gboolean   floating_sel;

  attached     = picman_item_is_attached (item);
  floating_sel = picman_layer_is_floating_sel (layer);

  if (floating_sel)
    {
      if (PICMAN_IS_CHANNEL (picman_layer_get_floating_sel_drawable (layer)))
        {
          g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			       _("Cannot create a new layer from the floating "
				 "selection because it belongs to a layer mask "
				 "or channel."));
          return FALSE;
        }

      if (attached)
        {
          picman_image_undo_group_start (image,
                                       PICMAN_UNDO_GROUP_ITEM_PROPERTIES,
                                       undo_desc);

          floating_sel_to_layer (layer, NULL);
        }
    }

  PICMAN_ITEM_CLASS (parent_class)->rename (item, new_name, undo_desc, error);

  if (attached && floating_sel)
    picman_image_undo_group_end (image);

  return TRUE;
}

static void
picman_layer_translate (PicmanItem *item,
                      gint      offset_x,
                      gint      offset_y,
                      gboolean  push_undo)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  if (push_undo)
    picman_image_undo_push_item_displace (picman_item_get_image (item), NULL, item);

  /*  update the old region  */
  picman_drawable_update (PICMAN_DRAWABLE (layer),
                        0, 0,
                        picman_item_get_width  (item),
                        picman_item_get_height (item));

  /*  invalidate the selection boundary because of a layer modification  */
  picman_drawable_invalidate_boundary (PICMAN_DRAWABLE (layer));

  PICMAN_ITEM_CLASS (parent_class)->translate (item, offset_x, offset_y,
                                             push_undo);

  /*  update the new region  */
  picman_drawable_update (PICMAN_DRAWABLE (layer),
                        0, 0,
                        picman_item_get_width  (item),
                        picman_item_get_height (item));

  if (layer->mask)
    {
      gint off_x, off_y;

      picman_item_get_offset (item, &off_x, &off_y);
      picman_item_set_offset (PICMAN_ITEM (layer->mask), off_x, off_y);

      picman_viewable_invalidate_preview (PICMAN_VIEWABLE (layer->mask));
    }
}

static void
picman_layer_scale (PicmanItem              *item,
                  gint                   new_width,
                  gint                   new_height,
                  gint                   new_offset_x,
                  gint                   new_offset_y,
                  PicmanInterpolationType  interpolation_type,
                  PicmanProgress          *progress)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  PICMAN_ITEM_CLASS (parent_class)->scale (item, new_width, new_height,
                                         new_offset_x, new_offset_y,
                                         interpolation_type, progress);

  if (layer->mask)
    picman_item_scale (PICMAN_ITEM (layer->mask),
                     new_width, new_height,
                     new_offset_x, new_offset_y,
                     interpolation_type, progress);
}

static void
picman_layer_resize (PicmanItem    *item,
                   PicmanContext *context,
                   gint         new_width,
                   gint         new_height,
                   gint         offset_x,
                   gint         offset_y)
{
  PicmanLayer *layer  = PICMAN_LAYER (item);

  PICMAN_ITEM_CLASS (parent_class)->resize (item, context, new_width, new_height,
                                          offset_x, offset_y);

  if (layer->mask)
    picman_item_resize (PICMAN_ITEM (layer->mask), context,
                      new_width, new_height, offset_x, offset_y);
}

static void
picman_layer_flip (PicmanItem            *item,
                 PicmanContext         *context,
                 PicmanOrientationType  flip_type,
                 gdouble              axis,
                 gboolean             clip_result)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  PICMAN_ITEM_CLASS (parent_class)->flip (item, context, flip_type, axis,
                                        clip_result);

  if (layer->mask)
    picman_item_flip (PICMAN_ITEM (layer->mask), context,
                    flip_type, axis, clip_result);
}

static void
picman_layer_rotate (PicmanItem         *item,
                   PicmanContext      *context,
                   PicmanRotationType  rotate_type,
                   gdouble           center_x,
                   gdouble           center_y,
                   gboolean          clip_result)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  PICMAN_ITEM_CLASS (parent_class)->rotate (item, context,
                                          rotate_type, center_x, center_y,
                                          clip_result);

  if (layer->mask)
    picman_item_rotate (PICMAN_ITEM (layer->mask), context,
                      rotate_type, center_x, center_y, clip_result);
}

static void
picman_layer_transform (PicmanItem               *item,
                      PicmanContext            *context,
                      const PicmanMatrix3      *matrix,
                      PicmanTransformDirection  direction,
                      PicmanInterpolationType   interpolation_type,
                      gint                    recursion_level,
                      PicmanTransformResize     clip_result,
                      PicmanProgress           *progress)
{
  PicmanLayer *layer = PICMAN_LAYER (item);

  /* FIXME: make interpolated transformations work on layers without alpha */
  if (interpolation_type != PICMAN_INTERPOLATION_NONE &&
      ! picman_drawable_has_alpha (PICMAN_DRAWABLE (item)))
    picman_layer_add_alpha (layer);

  PICMAN_ITEM_CLASS (parent_class)->transform (item, context, matrix, direction,
                                             interpolation_type,
                                             recursion_level,
                                             clip_result,
                                             progress);

  if (layer->mask)
    picman_item_transform (PICMAN_ITEM (layer->mask), context,
                         matrix, direction,
                         interpolation_type, recursion_level,
                         clip_result, progress);
}

static void
picman_layer_to_selection (PicmanItem       *item,
                         PicmanChannelOps  op,
                         gboolean        antialias,
                         gboolean        feather,
                         gdouble         feather_radius_x,
                         gdouble         feather_radius_y)
{
  PicmanLayer *layer = PICMAN_LAYER (item);
  PicmanImage *image = picman_item_get_image (item);

  picman_channel_select_alpha (picman_image_get_mask (image),
                             PICMAN_DRAWABLE (layer),
                             op,
                             feather, feather_radius_x, feather_radius_y);
}

static gint64
picman_layer_estimate_memsize (const PicmanDrawable *drawable,
                             gint                width,
                             gint                height)
{
  PicmanLayer *layer   = PICMAN_LAYER (drawable);
  gint64     memsize = 0;

  if (layer->mask)
    memsize += picman_drawable_estimate_memsize (PICMAN_DRAWABLE (layer->mask),
                                               width, height);

  return memsize + PICMAN_DRAWABLE_CLASS (parent_class)->estimate_memsize (drawable,
                                                                         width,
                                                                         height);
}

static void
picman_layer_convert_type (PicmanDrawable      *drawable,
                         PicmanImage         *dest_image,
                         const Babl        *new_format,
                         PicmanImageBaseType  new_base_type,
                         PicmanPrecision      new_precision,
                         gint               layer_dither_type,
                         gint               mask_dither_type,
                         gboolean           push_undo)
{
  PicmanLayer  *layer = PICMAN_LAYER (drawable);
  GeglBuffer *dest_buffer;

  dest_buffer =
    gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                     picman_item_get_width  (PICMAN_ITEM (drawable)),
                                     picman_item_get_height (PICMAN_ITEM (drawable))),
                     new_format);

  if (layer_dither_type == 0)
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
                                       dest_buffer, bits, layer_dither_type);
    }

  picman_drawable_set_buffer (drawable, push_undo, NULL, dest_buffer);
  g_object_unref (dest_buffer);

  if (layer->mask &&
      new_precision != picman_drawable_get_precision (PICMAN_DRAWABLE (layer->mask)))
    {
      picman_drawable_convert_type (PICMAN_DRAWABLE (layer->mask), dest_image,
                                  PICMAN_GRAY, new_precision,
                                  layer_dither_type, mask_dither_type,
                                  push_undo);
    }
}

static void
picman_layer_invalidate_boundary (PicmanDrawable *drawable)
{
  PicmanLayer   *layer = PICMAN_LAYER (drawable);
  PicmanImage   *image;
  PicmanChannel *mask;

  if (! (image = picman_item_get_image (PICMAN_ITEM (layer))))
    return;

  /*  Turn the current selection off  */
  picman_image_selection_invalidate (image);

  /*  get the selection mask channel  */
  mask = picman_image_get_mask (image);

  /*  Only bother with the bounds if there is a selection  */
  if (! picman_channel_is_empty (mask))
    {
      mask->bounds_known   = FALSE;
      mask->boundary_known = FALSE;
    }

  if (picman_layer_is_floating_sel (layer))
    floating_sel_invalidate (layer);
}

static void
picman_layer_get_active_components (const PicmanDrawable *drawable,
                                  gboolean           *active)
{
  PicmanLayer  *layer  = PICMAN_LAYER (drawable);
  PicmanImage  *image  = picman_item_get_image (PICMAN_ITEM (drawable));
  const Babl *format = picman_drawable_get_format (drawable);

  /*  first copy the image active channels  */
  picman_image_get_active_array (image, active);

  if (picman_drawable_has_alpha (drawable) && layer->lock_alpha)
    active[babl_format_get_n_components (format) - 1] = FALSE;
}

static PicmanComponentMask
picman_layer_get_active_mask (const PicmanDrawable *drawable)
{
  PicmanLayer         *layer = PICMAN_LAYER (drawable);
  PicmanImage         *image = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanComponentMask  mask  = picman_image_get_active_mask (image);

  if (picman_drawable_has_alpha (drawable) && layer->lock_alpha)
    mask &= ~PICMAN_COMPONENT_ALPHA;

  return mask;
}

static gdouble
picman_layer_get_opacity_at (PicmanPickable *pickable,
                           gint          x,
                           gint          y)
{
  PicmanLayer *layer = PICMAN_LAYER (pickable);
  gdouble    value = PICMAN_OPACITY_TRANSPARENT;

  if (x >= 0 && x < picman_item_get_width  (PICMAN_ITEM (layer)) &&
      y >= 0 && y < picman_item_get_height (PICMAN_ITEM (layer)) &&
      picman_item_is_visible (PICMAN_ITEM (layer)))
    {
      if (! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
        {
          value = PICMAN_OPACITY_OPAQUE;
        }
      else
        {
          gegl_buffer_sample (picman_drawable_get_buffer (PICMAN_DRAWABLE (layer)),
                              x, y, NULL, &value, babl_format ("A double"),
                              GEGL_SAMPLER_NEAREST, GEGL_ABYSS_NONE);
        }

      if (layer->mask)
        {
          gdouble mask_value;

          mask_value = picman_pickable_get_opacity_at (PICMAN_PICKABLE (layer->mask),
                                                     x, y);

          value *= mask_value;
        }
    }

  return value;
}

static void
picman_layer_layer_mask_update (PicmanDrawable *drawable,
                              gint          x,
                              gint          y,
                              gint          width,
                              gint          height,
                              PicmanLayer    *layer)
{
  if (picman_layer_get_apply_mask (layer) ||
      picman_layer_get_show_mask (layer))
    {
      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            x, y, width, height);
    }
}


/*  public functions  */

PicmanLayer *
picman_layer_new (PicmanImage            *image,
                gint                  width,
                gint                  height,
                const Babl           *format,
                const gchar          *name,
                gdouble               opacity,
                PicmanLayerModeEffects  mode)
{
  PicmanLayer *layer;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);
  g_return_val_if_fail (format != NULL, NULL);

  layer = PICMAN_LAYER (picman_drawable_new (PICMAN_TYPE_LAYER,
                                         image, name,
                                         0, 0, width, height,
                                         format));

  opacity = CLAMP (opacity, PICMAN_OPACITY_TRANSPARENT, PICMAN_OPACITY_OPAQUE);

  layer->opacity = opacity;
  layer->mode    = mode;

  return layer;
}

/**
 * picman_layer_new_from_buffer:
 * @buffer:     The buffer to make the new layer from.
 * @dest_image: The image the new layer will be added to.
 * @format:     The #Babl format of the new layer.
 * @name:       The new layer's name.
 * @opacity:    The new layer's opacity.
 * @mode:       The new layer's mode.
 *
 * Copies %buffer to a layer taking into consideration the
 * possibility of transforming the contents to meet the requirements
 * of the target image type
 *
 * Return value: The new layer.
 **/
PicmanLayer *
picman_layer_new_from_buffer (GeglBuffer           *buffer,
                            PicmanImage            *dest_image,
                            const Babl           *format,
                            const gchar          *name,
                            gdouble               opacity,
                            PicmanLayerModeEffects  mode)
{
  PicmanLayer  *layer;
  GeglBuffer *dest;

  g_return_val_if_fail (GEGL_IS_BUFFER (buffer), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (dest_image), NULL);
  g_return_val_if_fail (format != NULL, NULL);

  /*  do *not* use the buffer's format because this function gets
   *  buffers of any format passed, and converts them
   */
  layer = picman_layer_new (dest_image,
                          gegl_buffer_get_width  (buffer),
                          gegl_buffer_get_height (buffer),
                          format,
                          name, opacity, mode);

  dest = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer));
  gegl_buffer_copy (buffer, NULL, dest, NULL);

  return layer;
}

/**
 * picman_layer_new_from_pixbuf:
 * @pixbuf:     The pixbuf to make the new layer from.
 * @dest_image: The image the new layer will be added to.
 * @format:     The #Babl format of the new layer.
 * @name:       The new layer's name.
 * @opacity:    The new layer's opacity.
 * @mode:       The new layer's mode.
 *
 * Copies %pixbuf to a layer taking into consideration the
 * possibility of transforming the contents to meet the requirements
 * of the target image type
 *
 * Return value: The new layer.
 **/
PicmanLayer *
picman_layer_new_from_pixbuf (GdkPixbuf            *pixbuf,
                            PicmanImage            *dest_image,
                            const Babl           *format,
                            const gchar          *name,
                            gdouble               opacity,
                            PicmanLayerModeEffects  mode)
{
  GeglBuffer *buffer;
  PicmanLayer  *layer;

  g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);
  g_return_val_if_fail (PICMAN_IS_IMAGE (dest_image), NULL);
  g_return_val_if_fail (format != NULL, NULL);

  buffer = picman_pixbuf_create_buffer (pixbuf);

  layer = picman_layer_new_from_buffer (buffer, dest_image, format,
                                      name, opacity, mode);

  g_object_unref (buffer);

  return layer;
}

PicmanLayer *
picman_layer_get_parent (PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);

  return PICMAN_LAYER (picman_viewable_get_parent (PICMAN_VIEWABLE (layer)));
}

PicmanLayerMask *
picman_layer_get_mask (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);

  return layer->mask;
}

PicmanLayerMask *
picman_layer_add_mask (PicmanLayer      *layer,
                     PicmanLayerMask  *mask,
                     gboolean        push_undo,
                     GError        **error)
{
  PicmanImage *image;

  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (PICMAN_IS_LAYER_MASK (mask), NULL);
  g_return_val_if_fail (picman_item_get_image (PICMAN_ITEM (layer)) ==
                        picman_item_get_image (PICMAN_ITEM (mask)), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (! picman_item_is_attached (PICMAN_ITEM (layer)))
    push_undo = FALSE;

  image = picman_item_get_image (PICMAN_ITEM (layer));

  if (layer->mask)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Unable to add a layer mask since "
			     "the layer already has one."));
      return NULL;
    }

  if ((picman_item_get_width (PICMAN_ITEM (layer)) !=
       picman_item_get_width (PICMAN_ITEM (mask))) ||
      (picman_item_get_height (PICMAN_ITEM (layer)) !=
       picman_item_get_height (PICMAN_ITEM (mask))))
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			   _("Cannot add layer mask of different "
			     "dimensions than specified layer."));
      return NULL;
    }

  if (push_undo)
    picman_image_undo_push_layer_mask_add (image, C_("undo-type", "Add Layer Mask"),
                                         layer, mask);

  layer->mask = g_object_ref_sink (mask);
  layer->apply_mask = TRUE;
  layer->edit_mask  = TRUE;
  layer->show_mask  = FALSE;

  picman_layer_mask_set_layer (mask, layer);

  if (picman_filter_peek_node (PICMAN_FILTER (layer)))
    {
      GeglNode *mode_node;
      GeglNode *mask;

      mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));

      mask = picman_drawable_get_source_node (PICMAN_DRAWABLE (layer->mask));

      gegl_node_connect_to (mask,                    "output",
                            layer->mask_offset_node, "input");

      if (layer->show_mask)
        {
          gegl_node_connect_to (layer->mask_offset_node, "output",
                                mode_node,               "aux");
        }
      else
        {
          gegl_node_connect_to (layer->mask_offset_node, "output",
                                mode_node,               "aux2");
        }
    }

  if (picman_layer_get_apply_mask (layer) ||
      picman_layer_get_show_mask (layer))
    {
      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));
    }

  g_signal_connect (mask, "update",
                    G_CALLBACK (picman_layer_layer_mask_update),
                    layer);

  g_signal_emit (layer, layer_signals[MASK_CHANGED], 0);

  g_object_notify (G_OBJECT (layer), "mask");

  /*  if the mask came from the undo stack, reset its "removed" state  */
  if (picman_item_is_removed (PICMAN_ITEM (mask)))
    picman_item_unset_removed (PICMAN_ITEM (mask));

  return layer->mask;
}

PicmanLayerMask *
picman_layer_create_mask (const PicmanLayer *layer,
                        PicmanAddMaskType  add_mask_type,
                        PicmanChannel     *channel)
{
  PicmanDrawable  *drawable;
  PicmanItem      *item;
  PicmanLayerMask *mask;
  PicmanImage     *image;
  gchar         *mask_name;
  PicmanRGB        black = { 0.0, 0.0, 0.0, PICMAN_OPACITY_OPAQUE };

  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);
  g_return_val_if_fail (add_mask_type != PICMAN_ADD_CHANNEL_MASK ||
                        PICMAN_IS_CHANNEL (channel), NULL);

  drawable = PICMAN_DRAWABLE (layer);
  item     = PICMAN_ITEM (layer);
  image    = picman_item_get_image (item);

  mask_name = g_strdup_printf (_("%s mask"),
                               picman_object_get_name (layer));

  mask = picman_layer_mask_new (image,
                              picman_item_get_width  (item),
                              picman_item_get_height (item),
                              mask_name, &black);

  g_free (mask_name);

  switch (add_mask_type)
    {
    case PICMAN_ADD_WHITE_MASK:
      picman_channel_all (PICMAN_CHANNEL (mask), FALSE);
      return mask;

    case PICMAN_ADD_BLACK_MASK:
      picman_channel_clear (PICMAN_CHANNEL (mask), NULL, FALSE);
      return mask;

    default:
      break;
    }

  switch (add_mask_type)
    {
    case PICMAN_ADD_WHITE_MASK:
    case PICMAN_ADD_BLACK_MASK:
      break;

    case PICMAN_ADD_ALPHA_MASK:
    case PICMAN_ADD_ALPHA_TRANSFER_MASK:
      if (picman_drawable_has_alpha (drawable))
        {
          GeglBuffer *dest_buffer;
          const Babl *component_format;

          dest_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

          component_format =
            picman_image_get_component_format (image, PICMAN_ALPHA_CHANNEL);

          gegl_buffer_set_format (dest_buffer, component_format);
          gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                            dest_buffer, NULL);
          gegl_buffer_set_format (dest_buffer, NULL);

          if (add_mask_type == PICMAN_ADD_ALPHA_TRANSFER_MASK)
            {
              picman_drawable_push_undo (drawable,
                                       C_("undo-type", "Transfer Alpha to Mask"),
                                       NULL,
                                       0, 0,
                                       picman_item_get_width  (item),
                                       picman_item_get_height (item));

              picman_gegl_apply_set_alpha (picman_drawable_get_buffer (drawable),
                                         NULL, NULL,
                                         picman_drawable_get_buffer (drawable),
                                         1.0);
            }
        }
      break;

    case PICMAN_ADD_SELECTION_MASK:
    case PICMAN_ADD_CHANNEL_MASK:
      {
        gboolean channel_empty;
        gint     offset_x, offset_y;
        gint     copy_x, copy_y;
        gint     copy_width, copy_height;

        if (add_mask_type == PICMAN_ADD_SELECTION_MASK)
          channel = PICMAN_CHANNEL (picman_image_get_mask (image));

        channel_empty = picman_channel_is_empty (channel);

        picman_item_get_offset (item, &offset_x, &offset_y);

        picman_rectangle_intersect (0, 0,
                                  picman_image_get_width  (image),
                                  picman_image_get_height (image),
                                  offset_x, offset_y,
                                  picman_item_get_width  (item),
                                  picman_item_get_height (item),
                                  &copy_x, &copy_y,
                                  &copy_width, &copy_height);

        if (copy_width  < picman_item_get_width  (item) ||
            copy_height < picman_item_get_height (item) ||
            channel_empty)
          picman_channel_clear (PICMAN_CHANNEL (mask), NULL, FALSE);

        if ((copy_width || copy_height) && ! channel_empty)
          {
            GeglBuffer    *src;
            GeglBuffer    *dest;

            src  = picman_drawable_get_buffer (PICMAN_DRAWABLE (channel));
            dest = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

            gegl_buffer_copy (src,
                              GEGL_RECTANGLE (copy_x, copy_y,
                                              copy_width, copy_height),
                              dest,
                              GEGL_RECTANGLE (copy_x - offset_x, copy_y - offset_y,
                                              0, 0));

            PICMAN_CHANNEL (mask)->bounds_known = FALSE;
          }
      }
      break;

    case PICMAN_ADD_COPY_MASK:
      {
        GeglBuffer *src_buffer;
        GeglBuffer *dest_buffer;

        if (! picman_drawable_is_gray (drawable))
          {
            const Babl *copy_format =
              picman_image_get_format (image, PICMAN_GRAY,
                                     picman_drawable_get_precision (drawable),
                                     picman_drawable_has_alpha (drawable));

            src_buffer =
              gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                               picman_item_get_width  (item),
                                               picman_item_get_height (item)),
                               copy_format);

            gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                              src_buffer, NULL);
          }
        else
          {
            src_buffer = picman_drawable_get_buffer (drawable);
            g_object_ref (src_buffer);
          }

        dest_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));

        if (picman_drawable_has_alpha (drawable))
          {
            PicmanRGB background;

            picman_rgba_set (&background, 0.0, 0.0, 0.0, 0.0);

            picman_gegl_apply_flatten (src_buffer, NULL, NULL,
                                     dest_buffer, &background);
          }
        else
          {
            gegl_buffer_copy (src_buffer, NULL, dest_buffer, NULL);
          }

        g_object_unref (src_buffer);
      }

      PICMAN_CHANNEL (mask)->bounds_known = FALSE;
      break;
    }

  return mask;
}

void
picman_layer_apply_mask (PicmanLayer         *layer,
                       PicmanMaskApplyMode  mode,
                       gboolean           push_undo)
{
  PicmanItem      *item;
  PicmanImage     *image;
  PicmanLayerMask *mask;
  gboolean       view_changed = FALSE;

  g_return_if_fail (PICMAN_IS_LAYER (layer));

  mask = picman_layer_get_mask (layer);

  if (! mask)
    return;

  /*  APPLY can only be done to layers with an alpha channel  */
  if (! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    g_return_if_fail (mode == PICMAN_MASK_DISCARD || push_undo == TRUE);

  item  = PICMAN_ITEM (layer);
  image = picman_item_get_image (item);

  if (! image)
    return;

  if (push_undo)
    {
      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_LAYER_APPLY_MASK,
                                   (mode == PICMAN_MASK_APPLY) ?
                                   C_("undo-type", "Apply Layer Mask") :
                                   C_("undo-type", "Delete Layer Mask"));

      picman_image_undo_push_layer_mask_show (image, NULL, layer);
      picman_image_undo_push_layer_mask_apply (image, NULL, layer);
      picman_image_undo_push_layer_mask_remove (image, NULL, layer, mask);

      if (mode == PICMAN_MASK_APPLY &&
          ! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
        {
          picman_layer_add_alpha (layer);
        }
    }

  /*  check if applying the mask changes the projection  */
  if (picman_layer_get_show_mask (layer)                                   ||
      (mode == PICMAN_MASK_APPLY   && ! picman_layer_get_apply_mask (layer)) ||
      (mode == PICMAN_MASK_DISCARD &&   picman_layer_get_apply_mask (layer)))
    {
      view_changed = TRUE;
    }

  if (mode == PICMAN_MASK_APPLY)
    {
      GeglBuffer *mask_buffer;
      GeglBuffer *dest_buffer;

      if (push_undo)
        picman_drawable_push_undo (PICMAN_DRAWABLE (layer), NULL,
                                 NULL,
                                 0, 0,
                                 picman_item_get_width  (item),
                                 picman_item_get_height (item));

      /*  Combine the current layer's alpha channel and the mask  */
      mask_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));
      dest_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (layer));

      picman_gegl_apply_opacity (picman_drawable_get_buffer (PICMAN_DRAWABLE (layer)),
                               NULL, NULL, dest_buffer,
                               mask_buffer, 0, 0, 1.0);
    }

  g_signal_handlers_disconnect_by_func (mask,
                                        picman_layer_layer_mask_update,
                                        layer);

  picman_item_removed (PICMAN_ITEM (mask));
  g_object_unref (mask);
  layer->mask = NULL;

  if (push_undo)
    picman_image_undo_group_end (image);

  if (picman_filter_peek_node (PICMAN_FILTER (layer)))
    {
      GeglNode *mode_node;

      mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));

      if (layer->show_mask)
        {
          gegl_node_connect_to (layer->layer_offset_node, "output",
                                mode_node,                "aux");
        }
      else
        {
          gegl_node_disconnect (mode_node, "aux2");
        }
    }

  /*  If applying actually changed the view  */
  if (view_changed)
    {
      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (item),
                            picman_item_get_height (item));
    }
  else
    {
      picman_viewable_invalidate_preview (PICMAN_VIEWABLE (layer));
    }

  g_signal_emit (layer, layer_signals[MASK_CHANGED], 0);

  g_object_notify (G_OBJECT (layer), "mask");
}

void
picman_layer_set_apply_mask (PicmanLayer *layer,
                           gboolean   apply,
                           gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (layer->mask != NULL);

  if (layer->apply_mask != apply)
    {
      PicmanImage *image = picman_item_get_image (PICMAN_ITEM (layer));

      if (push_undo)
        picman_image_undo_push_layer_mask_apply (image,
                                               apply ?
                                               C_("undo-type", "Enable Layer Mask") :
                                               C_("undo-type", "Disable Layer Mask"),
                                               layer);

      layer->apply_mask = apply ? TRUE : FALSE;

      if (picman_filter_peek_node (PICMAN_FILTER (layer)) &&
          ! picman_layer_get_show_mask (layer))
        {
          GeglNode *mode_node;

          mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));

          if (layer->apply_mask)
            {
              gegl_node_connect_to (layer->mask_offset_node, "output",
                                    mode_node,               "aux2");
            }
          else
            {
              gegl_node_disconnect (mode_node, "aux2");
            }
        }

      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));

      g_signal_emit (layer, layer_signals[APPLY_MASK_CHANGED], 0);
    }
}

gboolean
picman_layer_get_apply_mask (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (layer->mask, FALSE);

  return layer->apply_mask;
}

void
picman_layer_set_edit_mask (PicmanLayer *layer,
                          gboolean   edit)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (layer->mask != NULL);

  if (layer->edit_mask != edit)
    {
      layer->edit_mask = edit ? TRUE : FALSE;

      g_signal_emit (layer, layer_signals[EDIT_MASK_CHANGED], 0);
    }
}

gboolean
picman_layer_get_edit_mask (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (layer->mask, FALSE);

  return layer->edit_mask;
}

void
picman_layer_set_show_mask (PicmanLayer *layer,
                          gboolean   show,
                          gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (layer->mask != NULL);

  if (layer->show_mask != show)
    {
      PicmanImage *image = picman_item_get_image (PICMAN_ITEM (layer));

      if (push_undo)
        picman_image_undo_push_layer_mask_show (image,
                                              C_("undo-type", "Show Layer Mask"),
                                              layer);

      layer->show_mask = show ? TRUE : FALSE;

      if (picman_filter_peek_node (PICMAN_FILTER (layer)))
        {
          GeglNode *mode_node;

          mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));

          if (layer->show_mask)
            {
              gegl_node_disconnect (mode_node, "aux2");

              gegl_node_connect_to (layer->mask_offset_node, "output",
                                    mode_node,               "aux");
            }
          else
            {
              gegl_node_connect_to (layer->layer_offset_node, "output",
                                    mode_node,                "aux");

              if (picman_layer_get_apply_mask (layer))
                {
                  gegl_node_connect_to (layer->mask_offset_node, "output",
                                        mode_node,               "aux2");
                }
            }
        }

      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));

      g_signal_emit (layer, layer_signals[SHOW_MASK_CHANGED], 0);
    }
}

gboolean
picman_layer_get_show_mask (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);
  g_return_val_if_fail (layer->mask, FALSE);

  return layer->show_mask;
}

void
picman_layer_add_alpha (PicmanLayer *layer)
{
  PicmanItem     *item;
  PicmanDrawable *drawable;
  GeglBuffer   *new_buffer;

  g_return_if_fail (PICMAN_IS_LAYER (layer));

  if (picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    return;

  item     = PICMAN_ITEM (layer);
  drawable = PICMAN_DRAWABLE (layer);

  new_buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                                picman_item_get_width  (item),
                                                picman_item_get_height (item)),
                                picman_drawable_get_format_with_alpha (drawable));

  gegl_buffer_copy (picman_drawable_get_buffer (drawable), NULL,
                    new_buffer, NULL);

  picman_drawable_set_buffer (PICMAN_DRAWABLE (layer),
                            picman_item_is_attached (PICMAN_ITEM (layer)),
                            C_("undo-type", "Add Alpha Channel"),
                            new_buffer);
  g_object_unref (new_buffer);
}

void
picman_layer_flatten (PicmanLayer   *layer,
                    PicmanContext *context)
{
  GeglBuffer *new_buffer;
  PicmanRGB     background;

  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  if (! picman_drawable_has_alpha (PICMAN_DRAWABLE (layer)))
    return;

  new_buffer =
    gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                     picman_item_get_width  (PICMAN_ITEM (layer)),
                                     picman_item_get_height (PICMAN_ITEM (layer))),
                     picman_drawable_get_format_without_alpha (PICMAN_DRAWABLE (layer)));

  picman_context_get_background (context, &background);

  picman_gegl_apply_flatten (picman_drawable_get_buffer (PICMAN_DRAWABLE (layer)),
                           NULL, NULL,
                           new_buffer, &background);

  picman_drawable_set_buffer (PICMAN_DRAWABLE (layer),
                            picman_item_is_attached (PICMAN_ITEM (layer)),
                            C_("undo-type", "Remove Alpha Channel"),
                            new_buffer);
  g_object_unref (new_buffer);
}

void
picman_layer_resize_to_image (PicmanLayer   *layer,
                            PicmanContext *context)
{
  PicmanImage *image;
  gint       offset_x;
  gint       offset_y;

  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (layer)));
  g_return_if_fail (PICMAN_IS_CONTEXT (context));

  image = picman_item_get_image (PICMAN_ITEM (layer));

  picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_ITEM_RESIZE,
                               C_("undo-type", "Layer to Image Size"));

  picman_item_get_offset (PICMAN_ITEM (layer), &offset_x, &offset_y);
  picman_item_resize (PICMAN_ITEM (layer), context,
                    picman_image_get_width  (image),
                    picman_image_get_height (image),
                    offset_x, offset_y);

  picman_image_undo_group_end (image);
}

/**********************/
/*  access functions  */
/**********************/

PicmanDrawable *
picman_layer_get_floating_sel_drawable (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), NULL);

  return layer->fs.drawable;
}

void
picman_layer_set_floating_sel_drawable (PicmanLayer    *layer,
                                      PicmanDrawable *drawable)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (drawable == NULL || PICMAN_IS_DRAWABLE (drawable));

  if (layer->fs.drawable != drawable)
    {
      if (layer->fs.segs)
        {
          g_free (layer->fs.segs);
          layer->fs.segs     = NULL;
          layer->fs.num_segs = 0;
        }

      layer->fs.drawable = drawable;

      g_object_notify (G_OBJECT (layer), "floating-selection");
    }
}

gboolean
picman_layer_is_floating_sel (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);

  return (picman_layer_get_floating_sel_drawable (layer) != NULL);
}

void
picman_layer_set_opacity (PicmanLayer *layer,
                        gdouble    opacity,
                        gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));

  opacity = CLAMP (opacity, PICMAN_OPACITY_TRANSPARENT, PICMAN_OPACITY_OPAQUE);

  if (layer->opacity != opacity)
    {
      if (push_undo && picman_item_is_attached (PICMAN_ITEM (layer)))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (layer));

          picman_image_undo_push_layer_opacity (image, NULL, layer);
        }

      layer->opacity = opacity;

      g_signal_emit (layer, layer_signals[OPACITY_CHANGED], 0);
      g_object_notify (G_OBJECT (layer), "opacity");

      if (picman_filter_peek_node (PICMAN_FILTER (layer)))
        {
          GeglNode *mode_node;

          mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));

          picman_gegl_mode_node_set_opacity (mode_node, layer->opacity);
        }

      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));
    }
}

gdouble
picman_layer_get_opacity (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), PICMAN_OPACITY_OPAQUE);

  return layer->opacity;
}

void
picman_layer_set_mode (PicmanLayer            *layer,
                     PicmanLayerModeEffects  mode,
                     gboolean              push_undo)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));

  if (layer->mode != mode)
    {
      if (push_undo && picman_item_is_attached (PICMAN_ITEM (layer)))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (layer));

          picman_image_undo_push_layer_mode (image, NULL, layer);
        }

      layer->mode = mode;

      g_signal_emit (layer, layer_signals[MODE_CHANGED], 0);
      g_object_notify (G_OBJECT (layer), "mode");

      if (picman_filter_peek_node (PICMAN_FILTER (layer)))
        {
          GeglNode *mode_node;
          gboolean  linear;

          mode_node = picman_drawable_get_mode_node (PICMAN_DRAWABLE (layer));
          linear    = picman_drawable_get_linear (PICMAN_DRAWABLE (layer));

          picman_gegl_mode_node_set_mode (mode_node,
                                        picman_layer_get_visible_mode (layer),
                                        linear);
        }

      picman_drawable_update (PICMAN_DRAWABLE (layer),
                            0, 0,
                            picman_item_get_width  (PICMAN_ITEM (layer)),
                            picman_item_get_height (PICMAN_ITEM (layer)));
    }
}

PicmanLayerModeEffects
picman_layer_get_mode (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), PICMAN_NORMAL_MODE);

  return layer->mode;
}

void
picman_layer_set_lock_alpha (PicmanLayer *layer,
                           gboolean   lock_alpha,
                           gboolean   push_undo)
{
  g_return_if_fail (PICMAN_IS_LAYER (layer));
  g_return_if_fail (picman_layer_can_lock_alpha (layer));

  lock_alpha = lock_alpha ? TRUE : FALSE;

  if (layer->lock_alpha != lock_alpha)
    {
      if (push_undo && picman_item_is_attached (PICMAN_ITEM (layer)))
        {
          PicmanImage *image = picman_item_get_image (PICMAN_ITEM (layer));

          picman_image_undo_push_layer_lock_alpha (image, NULL, layer);
        }

      layer->lock_alpha = lock_alpha;

      g_signal_emit (layer, layer_signals[LOCK_ALPHA_CHANGED], 0);
      g_object_notify (G_OBJECT (layer), "lock-alpha");
    }
}

gboolean
picman_layer_get_lock_alpha (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);

  return layer->lock_alpha;
}

gboolean
picman_layer_can_lock_alpha (const PicmanLayer *layer)
{
  g_return_val_if_fail (PICMAN_IS_LAYER (layer), FALSE);

  if (picman_viewable_get_children (PICMAN_VIEWABLE (layer)))
    return FALSE;

  return TRUE;
}
