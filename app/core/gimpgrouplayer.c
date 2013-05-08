/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * PicmanGroupLayer
 * Copyright (C) 2009  Michael Natterer <mitch@picman.org>
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

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "core-types.h"

#include "picmangrouplayer.h"
#include "picmanimage.h"
#include "picmanimage-undo-push.h"
#include "picmandrawablestack.h"
#include "picmanpickable.h"
#include "picmanprojectable.h"
#include "picmanprojection.h"

#include "picman-intl.h"


typedef struct _PicmanGroupLayerPrivate PicmanGroupLayerPrivate;

struct _PicmanGroupLayerPrivate
{
  PicmanContainer  *children;
  PicmanProjection *projection;
  GeglNode       *graph;
  GeglNode       *offset_node;
  gint            suspend_resize;
  gboolean        expanded;

  /*  hackish temp states to make the projection/tiles stuff work  */
  const Babl     *convert_format;
  gboolean        reallocate_projection;
  gint            reallocate_width;
  gint            reallocate_height;
};

#define GET_PRIVATE(item) G_TYPE_INSTANCE_GET_PRIVATE (item, \
                                                       PICMAN_TYPE_GROUP_LAYER, \
                                                       PicmanGroupLayerPrivate)


static void            picman_projectable_iface_init   (PicmanProjectableInterface  *iface);
static void            picman_pickable_iface_init      (PicmanPickableInterface     *iface);

static void            picman_group_layer_finalize     (GObject         *object);
static void            picman_group_layer_set_property (GObject         *object,
                                                      guint            property_id,
                                                      const GValue    *value,
                                                      GParamSpec      *pspec);
static void            picman_group_layer_get_property (GObject         *object,
                                                      guint            property_id,
                                                      GValue          *value,
                                                      GParamSpec      *pspec);

static gint64          picman_group_layer_get_memsize  (PicmanObject      *object,
                                                      gint64          *gui_size);

static gboolean        picman_group_layer_get_size     (PicmanViewable    *viewable,
                                                      gint            *width,
                                                      gint            *height);
static PicmanContainer * picman_group_layer_get_children (PicmanViewable    *viewable);
static gboolean        picman_group_layer_get_expanded (PicmanViewable    *viewable);
static void            picman_group_layer_set_expanded (PicmanViewable    *viewable,
                                                      gboolean         expanded);

static gboolean  picman_group_layer_is_position_locked (const PicmanItem  *item);
static PicmanItem      * picman_group_layer_duplicate    (PicmanItem        *item,
                                                      GType            new_type);
static void            picman_group_layer_convert      (PicmanItem        *item,
                                                      PicmanImage       *dest_image);
static void            picman_group_layer_translate    (PicmanItem        *item,
                                                      gint             offset_x,
                                                      gint             offset_y,
                                                      gboolean         push_undo);
static void            picman_group_layer_scale        (PicmanItem        *item,
                                                      gint             new_width,
                                                      gint             new_height,
                                                      gint             new_offset_x,
                                                      gint             new_offset_y,
                                                      PicmanInterpolationType  interp_type,
                                                      PicmanProgress    *progress);
static void            picman_group_layer_resize       (PicmanItem        *item,
                                                      PicmanContext     *context,
                                                      gint             new_width,
                                                      gint             new_height,
                                                      gint             offset_x,
                                                      gint             offset_y);
static void            picman_group_layer_flip         (PicmanItem        *item,
                                                      PicmanContext     *context,
                                                      PicmanOrientationType flip_type,
                                                      gdouble          axis,
                                                      gboolean         clip_result);
static void            picman_group_layer_rotate       (PicmanItem        *item,
                                                      PicmanContext     *context,
                                                      PicmanRotationType rotate_type,
                                                      gdouble          center_x,
                                                      gdouble          center_y,
                                                      gboolean         clip_result);
static void            picman_group_layer_transform    (PicmanItem        *item,
                                                      PicmanContext     *context,
                                                      const PicmanMatrix3 *matrix,
                                                      PicmanTransformDirection direction,
                                                      PicmanInterpolationType  interpolation_type,
                                                      gint             recursion_level,
                                                      PicmanTransformResize clip_result,
                                                      PicmanProgress    *progress);

static gint64      picman_group_layer_estimate_memsize (const PicmanDrawable *drawable,
                                                      gint             width,
                                                      gint             height);
static void            picman_group_layer_convert_type (PicmanDrawable      *drawable,
                                                      PicmanImage         *dest_image,
                                                      const Babl        *new_format,
                                                      PicmanImageBaseType  new_base_type,
                                                      PicmanPrecision      new_precision,
                                                      gint               layer_dither_type,
                                                      gint               mask_dither_type,
                                                      gboolean           push_undo);

static const Babl    * picman_group_layer_get_format   (PicmanProjectable *projectable);
static GeglNode      * picman_group_layer_get_graph    (PicmanProjectable *projectable);
static gdouble       picman_group_layer_get_opacity_at (PicmanPickable    *pickable,
                                                      gint             x,
                                                      gint             y);


static void            picman_group_layer_child_add    (PicmanContainer   *container,
                                                      PicmanLayer       *child,
                                                      PicmanGroupLayer  *group);
static void            picman_group_layer_child_remove (PicmanContainer   *container,
                                                      PicmanLayer       *child,
                                                      PicmanGroupLayer  *group);
static void            picman_group_layer_child_move   (PicmanLayer       *child,
                                                      GParamSpec      *pspec,
                                                      PicmanGroupLayer  *group);
static void            picman_group_layer_child_resize (PicmanLayer       *child,
                                                      PicmanGroupLayer  *group);

static void            picman_group_layer_update       (PicmanGroupLayer  *group);
static void            picman_group_layer_update_size  (PicmanGroupLayer  *group);

static void            picman_group_layer_stack_update (PicmanDrawableStack *stack,
                                                      gint               x,
                                                      gint               y,
                                                      gint               width,
                                                      gint               height,
                                                      PicmanGroupLayer    *group);
static void            picman_group_layer_proj_update  (PicmanProjection    *proj,
                                                      gboolean           now,
                                                      gint               x,
                                                      gint               y,
                                                      gint               width,
                                                      gint               height,
                                                      PicmanGroupLayer    *group);


G_DEFINE_TYPE_WITH_CODE (PicmanGroupLayer, picman_group_layer, PICMAN_TYPE_LAYER,
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PROJECTABLE,
                                                picman_projectable_iface_init)
                         G_IMPLEMENT_INTERFACE (PICMAN_TYPE_PICKABLE,
                                                picman_pickable_iface_init))


#define parent_class picman_group_layer_parent_class


static void
picman_group_layer_class_init (PicmanGroupLayerClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);
  PicmanDrawableClass *drawable_class    = PICMAN_DRAWABLE_CLASS (klass);

  object_class->set_property       = picman_group_layer_set_property;
  object_class->get_property       = picman_group_layer_get_property;
  object_class->finalize           = picman_group_layer_finalize;

  picman_object_class->get_memsize   = picman_group_layer_get_memsize;

  viewable_class->default_stock_id = "gtk-directory";
  viewable_class->get_size         = picman_group_layer_get_size;
  viewable_class->get_children     = picman_group_layer_get_children;
  viewable_class->set_expanded     = picman_group_layer_set_expanded;
  viewable_class->get_expanded     = picman_group_layer_get_expanded;

  item_class->is_position_locked   = picman_group_layer_is_position_locked;
  item_class->duplicate            = picman_group_layer_duplicate;
  item_class->convert              = picman_group_layer_convert;
  item_class->translate            = picman_group_layer_translate;
  item_class->scale                = picman_group_layer_scale;
  item_class->resize               = picman_group_layer_resize;
  item_class->flip                 = picman_group_layer_flip;
  item_class->rotate               = picman_group_layer_rotate;
  item_class->transform            = picman_group_layer_transform;

  item_class->default_name         = _("Layer Group");
  item_class->rename_desc          = C_("undo-type", "Rename Layer Group");
  item_class->translate_desc       = C_("undo-type", "Move Layer Group");
  item_class->scale_desc           = C_("undo-type", "Scale Layer Group");
  item_class->resize_desc          = C_("undo-type", "Resize Layer Group");
  item_class->flip_desc            = C_("undo-type", "Flip Layer Group");
  item_class->rotate_desc          = C_("undo-type", "Rotate Layer Group");
  item_class->transform_desc       = C_("undo-type", "Transform Layer Group");

  drawable_class->estimate_memsize = picman_group_layer_estimate_memsize;
  drawable_class->convert_type     = picman_group_layer_convert_type;

  g_type_class_add_private (klass, sizeof (PicmanGroupLayerPrivate));
}

static void
picman_projectable_iface_init (PicmanProjectableInterface *iface)
{
  iface->get_image          = (PicmanImage * (*) (PicmanProjectable *)) picman_item_get_image;
  iface->get_format         = picman_group_layer_get_format;
  iface->get_offset         = (void (*) (PicmanProjectable*, gint*, gint*)) picman_item_get_offset;
  iface->get_size           = (void (*) (PicmanProjectable*, gint*, gint*)) picman_viewable_get_size;
  iface->get_graph          = picman_group_layer_get_graph;
  iface->invalidate_preview = (void (*) (PicmanProjectable*)) picman_viewable_invalidate_preview;
}

static void
picman_pickable_iface_init (PicmanPickableInterface *iface)
{
  iface->get_opacity_at = picman_group_layer_get_opacity_at;
}

static void
picman_group_layer_init (PicmanGroupLayer *group)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (group);

  private->children = picman_drawable_stack_new (PICMAN_TYPE_LAYER);
  private->expanded = TRUE;

  g_signal_connect (private->children, "add",
                    G_CALLBACK (picman_group_layer_child_add),
                    group);
  g_signal_connect (private->children, "remove",
                    G_CALLBACK (picman_group_layer_child_remove),
                    group);

  picman_container_add_handler (private->children, "notify::offset-x",
                              G_CALLBACK (picman_group_layer_child_move),
                              group);
  picman_container_add_handler (private->children, "notify::offset-y",
                              G_CALLBACK (picman_group_layer_child_move),
                              group);
  picman_container_add_handler (private->children, "size-changed",
                              G_CALLBACK (picman_group_layer_child_resize),
                              group);

  g_signal_connect (private->children, "update",
                    G_CALLBACK (picman_group_layer_stack_update),
                    group);

  private->projection = picman_projection_new (PICMAN_PROJECTABLE (group));

  g_signal_connect (private->projection, "update",
                    G_CALLBACK (picman_group_layer_proj_update),
                    group);
}

static void
picman_group_layer_finalize (GObject *object)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (object);

  if (private->children)
    {
      g_signal_handlers_disconnect_by_func (private->children,
                                            picman_group_layer_child_add,
                                            object);
      g_signal_handlers_disconnect_by_func (private->children,
                                            picman_group_layer_child_remove,
                                            object);
      g_signal_handlers_disconnect_by_func (private->children,
                                            picman_group_layer_stack_update,
                                            object);

      g_object_unref (private->children);
      private->children = NULL;
    }

  if (private->projection)
    {
      g_object_unref (private->projection);
      private->projection = NULL;
    }

  if (private->graph)
    {
      g_object_unref (private->graph);
      private->graph = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_group_layer_set_property (GObject      *object,
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
picman_group_layer_get_property (GObject    *object,
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
picman_group_layer_get_memsize (PicmanObject *object,
                              gint64     *gui_size)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (object);
  gint64                 memsize = 0;

  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->children), gui_size);
  memsize += picman_object_get_memsize (PICMAN_OBJECT (private->projection), gui_size);

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_group_layer_get_size (PicmanViewable *viewable,
                           gint         *width,
                           gint         *height)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (viewable);

  if (private->reallocate_width  != 0 &&
      private->reallocate_height != 0)
    {
      *width  = private->reallocate_width;
      *height = private->reallocate_height;

      return TRUE;
    }

  return PICMAN_VIEWABLE_CLASS (parent_class)->get_size (viewable, width, height);
}

static PicmanContainer *
picman_group_layer_get_children (PicmanViewable *viewable)
{
  return GET_PRIVATE (viewable)->children;
}

static gboolean
picman_group_layer_get_expanded (PicmanViewable *viewable)
{
  PicmanGroupLayer *group = PICMAN_GROUP_LAYER (viewable);

  return GET_PRIVATE (group)->expanded;
}

static void
picman_group_layer_set_expanded (PicmanViewable *viewable,
                               gboolean      expanded)
{
  PicmanGroupLayer *group = PICMAN_GROUP_LAYER (viewable);

  GET_PRIVATE (group)->expanded = expanded;
}

static gboolean
picman_group_layer_is_position_locked (const PicmanItem *item)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  GList                 *list;

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      if (picman_item_is_position_locked (child))
        return TRUE;
    }

  return PICMAN_ITEM_CLASS (parent_class)->is_position_locked (item);
}

static PicmanItem *
picman_group_layer_duplicate (PicmanItem *item,
                            GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_DRAWABLE), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_GROUP_LAYER (new_item))
    {
      PicmanGroupLayerPrivate *private     = GET_PRIVATE (item);
      PicmanGroupLayer        *new_group   = PICMAN_GROUP_LAYER (new_item);
      PicmanGroupLayerPrivate *new_private = GET_PRIVATE (new_item);
      gint                   position    = 0;
      GList                 *list;

      picman_group_layer_suspend_resize (new_group, FALSE);

      for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
           list;
           list = g_list_next (list))
        {
          PicmanItem      *child = list->data;
          PicmanItem      *new_child;
          PicmanLayerMask *mask;

          new_child = picman_item_duplicate (child, G_TYPE_FROM_INSTANCE (child));

          picman_object_set_name (PICMAN_OBJECT (new_child),
                                picman_object_get_name (child));

          mask = picman_layer_get_mask (PICMAN_LAYER (child));

          if (mask)
            {
              PicmanLayerMask *new_mask;

              new_mask = picman_layer_get_mask (PICMAN_LAYER (new_child));

              picman_object_set_name (PICMAN_OBJECT (new_mask),
                                    picman_object_get_name (mask));
            }

          picman_viewable_set_parent (PICMAN_VIEWABLE (new_child),
                                    PICMAN_VIEWABLE (new_group));

          picman_container_insert (new_private->children,
                                 PICMAN_OBJECT (new_child),
                                 position++);
        }

      /*  force the projection to reallocate itself  */
      GET_PRIVATE (new_group)->reallocate_projection = TRUE;

      picman_group_layer_resume_resize (new_group, FALSE);
    }

  return new_item;
}

static void
picman_group_layer_convert (PicmanItem  *item,
                          PicmanImage *dest_image)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  GList                 *list;

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      PICMAN_ITEM_GET_CLASS (child)->convert (child, dest_image);
    }

  PICMAN_ITEM_CLASS (parent_class)->convert (item, dest_image);
}

static void
picman_group_layer_translate (PicmanItem *item,
                            gint      offset_x,
                            gint      offset_y,
                            gboolean  push_undo)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;

  /*  don't push an undo here because undo will call us again  */
  picman_group_layer_suspend_resize (group, FALSE);

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      picman_item_translate (child, offset_x, offset_y, push_undo);
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    {
      gint off_x, off_y;

      picman_item_get_offset (item, &off_x, &off_y);
      picman_item_set_offset (PICMAN_ITEM (mask), off_x, off_y);

      picman_viewable_invalidate_preview (PICMAN_VIEWABLE (mask));
    }

  /*  don't push an undo here because undo will call us again  */
  picman_group_layer_resume_resize (group, FALSE);
}

static void
picman_group_layer_scale (PicmanItem              *item,
                        gint                   new_width,
                        gint                   new_height,
                        gint                   new_offset_x,
                        gint                   new_offset_y,
                        PicmanInterpolationType  interpolation_type,
                        PicmanProgress          *progress)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;
  gdouble                width_factor;
  gdouble                height_factor;
  gint                   old_offset_x;
  gint                   old_offset_y;

  width_factor  = (gdouble) new_width  / (gdouble) picman_item_get_width  (item);
  height_factor = (gdouble) new_height / (gdouble) picman_item_get_height (item);

  old_offset_x = picman_item_get_offset_x (item);
  old_offset_y = picman_item_get_offset_y (item);

  picman_group_layer_suspend_resize (group, TRUE);

  list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));

  while (list)
    {
      PicmanItem *child = list->data;
      gint      child_width;
      gint      child_height;
      gint      child_offset_x;
      gint      child_offset_y;

      list = g_list_next (list);

      child_width    = ROUND (width_factor  * picman_item_get_width  (child));
      child_height   = ROUND (height_factor * picman_item_get_height (child));
      child_offset_x = ROUND (width_factor  * (picman_item_get_offset_x (child) -
                                               old_offset_x));
      child_offset_y = ROUND (height_factor * (picman_item_get_offset_y (child) -
                                               old_offset_y));

      child_offset_x += new_offset_x;
      child_offset_y += new_offset_y;

      if (child_width > 0 && child_height > 0)
        {
          picman_item_scale (child,
                           child_width, child_height,
                           child_offset_x, child_offset_y,
                           interpolation_type, progress);
        }
      else if (picman_item_is_attached (item))
        {
          picman_image_remove_layer (picman_item_get_image (item),
                                   PICMAN_LAYER (child),
                                   TRUE, NULL);
        }
      else
        {
          picman_container_remove (private->children, PICMAN_OBJECT (child));
        }
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    picman_item_scale (PICMAN_ITEM (mask),
                     new_width, new_height,
                     new_offset_x, new_offset_y,
                     interpolation_type, progress);

  picman_group_layer_resume_resize (group, TRUE);
}

static void
picman_group_layer_resize (PicmanItem    *item,
                         PicmanContext *context,
                         gint         new_width,
                         gint         new_height,
                         gint         offset_x,
                         gint         offset_y)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;
  gint                   x, y;

  x = picman_item_get_offset_x (item) - offset_x;
  y = picman_item_get_offset_y (item) - offset_y;

  picman_group_layer_suspend_resize (group, TRUE);

  list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));

  while (list)
    {
      PicmanItem *child = list->data;
      gint      child_width;
      gint      child_height;
      gint      child_x;
      gint      child_y;

      list = g_list_next (list);

      if (picman_rectangle_intersect (x,
                                    y,
                                    new_width,
                                    new_height,
                                    picman_item_get_offset_x (child),
                                    picman_item_get_offset_y (child),
                                    picman_item_get_width  (child),
                                    picman_item_get_height (child),
                                    &child_x,
                                    &child_y,
                                    &child_width,
                                    &child_height))
        {
          gint child_offset_x = picman_item_get_offset_x (child) - child_x;
          gint child_offset_y = picman_item_get_offset_y (child) - child_y;

          picman_item_resize (child, context,
                            child_width, child_height,
                            child_offset_x, child_offset_y);
        }
      else if (picman_item_is_attached (item))
        {
          picman_image_remove_layer (picman_item_get_image (item),
                                   PICMAN_LAYER (child),
                                   TRUE, NULL);
        }
      else
        {
          picman_container_remove (private->children, PICMAN_OBJECT (child));
        }
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    picman_item_resize (PICMAN_ITEM (mask), context,
                      new_width, new_height, offset_x, offset_y);

  picman_group_layer_resume_resize (group, TRUE);
}

static void
picman_group_layer_flip (PicmanItem            *item,
                       PicmanContext         *context,
                       PicmanOrientationType  flip_type,
                       gdouble              axis,
                       gboolean             clip_result)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;

  picman_group_layer_suspend_resize (group, TRUE);

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      picman_item_flip (child, context,
                      flip_type, axis, clip_result);
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    picman_item_flip (PICMAN_ITEM (mask), context,
                    flip_type, axis, clip_result);

  picman_group_layer_resume_resize (group, TRUE);
}

static void
picman_group_layer_rotate (PicmanItem         *item,
                         PicmanContext      *context,
                         PicmanRotationType  rotate_type,
                         gdouble           center_x,
                         gdouble           center_y,
                         gboolean          clip_result)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;

  picman_group_layer_suspend_resize (group, TRUE);

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      picman_item_rotate (child, context,
                        rotate_type, center_x, center_y, clip_result);
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    picman_item_rotate (PICMAN_ITEM (mask), context,
                      rotate_type, center_x, center_y, clip_result);

  picman_group_layer_resume_resize (group, TRUE);
}

static void
picman_group_layer_transform (PicmanItem               *item,
                            PicmanContext            *context,
                            const PicmanMatrix3      *matrix,
                            PicmanTransformDirection  direction,
                            PicmanInterpolationType   interpolation_type,
                            gint                    recursion_level,
                            PicmanTransformResize     clip_result,
                            PicmanProgress           *progress)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (item);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (item);
  PicmanLayerMask         *mask;
  GList                 *list;

  picman_group_layer_suspend_resize (group, TRUE);

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      picman_item_transform (child, context,
                           matrix, direction,
                           interpolation_type, recursion_level,
                           clip_result, progress);
    }

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask)
    picman_item_transform (PICMAN_ITEM (mask), context,
                         matrix, direction,
                         interpolation_type, recursion_level,
                         clip_result, progress);

  picman_group_layer_resume_resize (group, TRUE);
}

static gint64
picman_group_layer_estimate_memsize (const PicmanDrawable *drawable,
                                   gint                width,
                                   gint                height)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (drawable);
  GList                 *list;
  PicmanImageBaseType      base_type;
  gint64                 memsize = 0;

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanDrawable *child = list->data;
      gint          child_width;
      gint          child_height;

      child_width  = (picman_item_get_width (PICMAN_ITEM (child)) *
                      width /
                      picman_item_get_width (PICMAN_ITEM (drawable)));
      child_height = (picman_item_get_height (PICMAN_ITEM (child)) *
                      height /
                      picman_item_get_height (PICMAN_ITEM (drawable)));

      memsize += picman_drawable_estimate_memsize (child,
                                                 child_width,
                                                 child_height);
    }

  base_type = picman_drawable_get_base_type (drawable);

  memsize += picman_projection_estimate_memsize (base_type,
                                               picman_drawable_get_precision (drawable),
                                               width, height);

  return memsize + PICMAN_DRAWABLE_CLASS (parent_class)->estimate_memsize (drawable,
                                                                         width,
                                                                         height);
}

static const Babl *
get_projection_format (PicmanProjectable   *projectable,
                       PicmanImageBaseType  base_type,
                       PicmanPrecision      precision)
{
  PicmanImage *image = picman_item_get_image (PICMAN_ITEM (projectable));

  switch (base_type)
    {
    case PICMAN_RGB:
    case PICMAN_INDEXED:
      return picman_image_get_format (image, PICMAN_RGB, precision, TRUE);

    case PICMAN_GRAY:
      return picman_image_get_format (image, PICMAN_GRAY, precision, TRUE);
    }

  g_assert_not_reached ();

  return NULL;
}

static void
picman_group_layer_convert_type (PicmanDrawable      *drawable,
                               PicmanImage         *dest_image,
                               const Babl        *new_format /* unused */,
                               PicmanImageBaseType  new_base_type,
                               PicmanPrecision      new_precision,
                               gint               layer_dither_type,
                               gint               mask_dither_type,
                               gboolean           push_undo)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (drawable);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (drawable);
  PicmanLayerMask         *mask;
  GeglBuffer            *buffer;

  if (push_undo)
    {
      PicmanImage *image = picman_item_get_image (PICMAN_ITEM (group));

      picman_image_undo_push_group_layer_convert (image, NULL, group);
    }

  /*  Need to temporarily set the projectable's format to the new
   *  values so the projection will create its tiles with the right
   *  depth
   */
  private->convert_format = get_projection_format (PICMAN_PROJECTABLE (drawable),
                                                   new_base_type,
                                                   new_precision);
  picman_projectable_structure_changed (PICMAN_PROJECTABLE (drawable));

  buffer = picman_pickable_get_buffer (PICMAN_PICKABLE (private->projection));

  picman_drawable_set_buffer_full (drawable,
                                 FALSE, NULL,
                                 buffer,
                                 picman_item_get_offset_x (PICMAN_ITEM (drawable)),
                                 picman_item_get_offset_y (PICMAN_ITEM (drawable)));

  /*  reset, the actual format is right now  */
  private->convert_format = NULL;

  mask = picman_layer_get_mask (PICMAN_LAYER (group));

  if (mask &&
      new_precision != picman_drawable_get_precision (PICMAN_DRAWABLE (mask)))
    {
      picman_drawable_convert_type (PICMAN_DRAWABLE (mask), dest_image,
                                  PICMAN_GRAY, new_precision,
                                  layer_dither_type, mask_dither_type,
                                  push_undo);
    }
}

static const Babl *
picman_group_layer_get_format (PicmanProjectable *projectable)
{
  PicmanGroupLayerPrivate *private = GET_PRIVATE (projectable);
  PicmanImageBaseType      base_type;
  PicmanPrecision          precision;

  if (private->convert_format)
    return private->convert_format;

  base_type = picman_drawable_get_base_type (PICMAN_DRAWABLE (projectable));
  precision = picman_drawable_get_precision (PICMAN_DRAWABLE (projectable));

  return get_projection_format (projectable, base_type, precision);
}

static GeglNode *
picman_group_layer_get_graph (PicmanProjectable *projectable)
{
  PicmanGroupLayer        *group   = PICMAN_GROUP_LAYER (projectable);
  PicmanGroupLayerPrivate *private = GET_PRIVATE (projectable);
  GeglNode              *layers_node;
  GeglNode              *output;
  gint                   off_x;
  gint                   off_y;

  if (private->graph)
    return private->graph;

  private->graph = gegl_node_new ();

  layers_node =
    picman_filter_stack_get_graph (PICMAN_FILTER_STACK (private->children));

  gegl_node_add_child (private->graph, layers_node);

  picman_item_get_offset (PICMAN_ITEM (group), &off_x, &off_y);

  private->offset_node = gegl_node_new_child (private->graph,
                                              "operation", "gegl:translate",
                                              "x",         (gdouble) -off_x,
                                              "y",         (gdouble) -off_y,
                                              NULL);

  gegl_node_connect_to (layers_node,          "output",
                        private->offset_node, "input");

  output = gegl_node_get_output_proxy (private->graph, "output");

  gegl_node_connect_to (private->offset_node, "output",
                        output,               "input");

  return private->graph;
}

static gdouble
picman_group_layer_get_opacity_at (PicmanPickable *pickable,
                                 gint          x,
                                 gint          y)
{
  /* Only consider child layers as having content */

  return PICMAN_OPACITY_TRANSPARENT;
}


/*  public functions  */

PicmanLayer *
picman_group_layer_new (PicmanImage *image)
{
  PicmanGroupLayer *group;
  const Babl     *format;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  format = picman_image_get_layer_format (image, TRUE);

  group = PICMAN_GROUP_LAYER (picman_drawable_new (PICMAN_TYPE_GROUP_LAYER,
                                               image, NULL,
                                               0, 0, 1, 1,
                                               format));

  return PICMAN_LAYER (group);
}

PicmanProjection *
picman_group_layer_get_projection (PicmanGroupLayer *group)
{
  g_return_val_if_fail (PICMAN_IS_GROUP_LAYER (group), NULL);

  return GET_PRIVATE (group)->projection;
}

void
picman_group_layer_suspend_resize (PicmanGroupLayer *group,
                                 gboolean        push_undo)
{
  PicmanItem *item;

  g_return_if_fail (PICMAN_IS_GROUP_LAYER (group));

  item = PICMAN_ITEM (group);

  if (! picman_item_is_attached (item))
    push_undo = FALSE;

  if (push_undo)
    picman_image_undo_push_group_layer_suspend (picman_item_get_image (item),
                                              NULL, group);

  GET_PRIVATE (group)->suspend_resize++;
}

void
picman_group_layer_resume_resize (PicmanGroupLayer *group,
                                gboolean        push_undo)
{
  PicmanGroupLayerPrivate *private;
  PicmanItem              *item;

  g_return_if_fail (PICMAN_IS_GROUP_LAYER (group));

  private = GET_PRIVATE (group);

  g_return_if_fail (private->suspend_resize > 0);

  item = PICMAN_ITEM (group);

  if (! picman_item_is_attached (item))
    push_undo = FALSE;

  if (push_undo)
    picman_image_undo_push_group_layer_resume (picman_item_get_image (item),
                                             NULL, group);

  private->suspend_resize--;

  if (private->suspend_resize == 0)
    {
      picman_group_layer_update_size (group);
    }
}


/*  private functions  */

static void
picman_group_layer_child_add (PicmanContainer  *container,
                            PicmanLayer      *child,
                            PicmanGroupLayer *group)
{
  picman_group_layer_update (group);
}

static void
picman_group_layer_child_remove (PicmanContainer  *container,
                               PicmanLayer      *child,
                               PicmanGroupLayer *group)
{
  picman_group_layer_update (group);
}

static void
picman_group_layer_child_move (PicmanLayer      *child,
                             GParamSpec     *pspec,
                             PicmanGroupLayer *group)
{
  picman_group_layer_update (group);
}

static void
picman_group_layer_child_resize (PicmanLayer      *child,
                               PicmanGroupLayer *group)
{
  picman_group_layer_update (group);
}

static void
picman_group_layer_update (PicmanGroupLayer *group)
{
  if (GET_PRIVATE (group)->suspend_resize == 0)
    {
      picman_group_layer_update_size (group);
    }
}

static void
picman_group_layer_update_size (PicmanGroupLayer *group)
{
  PicmanGroupLayerPrivate *private    = GET_PRIVATE (group);
  PicmanItem              *item       = PICMAN_ITEM (group);
  gint                   old_x      = picman_item_get_offset_x (item);
  gint                   old_y      = picman_item_get_offset_y (item);
  gint                   old_width  = picman_item_get_width  (item);
  gint                   old_height = picman_item_get_height (item);
  gint                   x          = 0;
  gint                   y          = 0;
  gint                   width      = 1;
  gint                   height     = 1;
  gboolean               first      = TRUE;
  GList                 *list;

  for (list = picman_item_stack_get_item_iter (PICMAN_ITEM_STACK (private->children));
       list;
       list = g_list_next (list))
    {
      PicmanItem *child = list->data;

      if (first)
        {
          x      = picman_item_get_offset_x (child);
          y      = picman_item_get_offset_y (child);
          width  = picman_item_get_width    (child);
          height = picman_item_get_height   (child);

          first = FALSE;
        }
      else
        {
          picman_rectangle_union (x, y, width, height,
                                picman_item_get_offset_x (child),
                                picman_item_get_offset_y (child),
                                picman_item_get_width    (child),
                                picman_item_get_height   (child),
                                &x, &y, &width, &height);
        }
    }

  if (private->reallocate_projection ||
      x      != old_x                ||
      y      != old_y                ||
      width  != old_width            ||
      height != old_height)
    {
      if (private->reallocate_projection ||
          width  != old_width            ||
          height != old_height)
        {
          GeglBuffer *buffer;

          private->reallocate_projection = FALSE;

          /*  temporarily change the return values of picman_viewable_get_size()
           *  so the projection allocates itself correctly
           */
          private->reallocate_width  = width;
          private->reallocate_height = height;

          picman_projectable_structure_changed (PICMAN_PROJECTABLE (group));

          buffer = picman_pickable_get_buffer (PICMAN_PICKABLE (private->projection));

          picman_drawable_set_buffer_full (PICMAN_DRAWABLE (group),
                                         FALSE, NULL,
                                         buffer,
                                         x, y);

          /*  reset, the actual size is correct now  */
          private->reallocate_width  = 0;
          private->reallocate_height = 0;
        }
      else
        {
          picman_item_set_offset (item, x, y);

          /*  invalidate the entire projection since the position of
           *  the children relative to each other might have changed
           *  in a way that happens to leave the group's width and
           *  height the same
           */
          picman_projectable_invalidate (PICMAN_PROJECTABLE (group),
                                       x, y, width, height);
        }

      if (private->offset_node)
        gegl_node_set (private->offset_node,
                       "x", (gdouble) -x,
                       "y", (gdouble) -y,
                       NULL);
    }
}

static void
picman_group_layer_stack_update (PicmanDrawableStack *stack,
                               gint               x,
                               gint               y,
                               gint               width,
                               gint               height,
                               PicmanGroupLayer    *group)
{
#if 0
  g_printerr ("%s (%s) %d, %d (%d, %d)\n",
              G_STRFUNC, picman_object_get_name (group),
              x, y, width, height);
#endif

  /*  the layer stack's update signal speaks in image coordinates,
   *  pass to the projection as-is.
   */
  picman_projectable_invalidate (PICMAN_PROJECTABLE (group),
                               x, y, width, height);

  /*  flush the pickable not the projectable because flushing the
   *  pickable will finish all invalidation on the projection so it
   *  can be used as source (note that it will still be constructed
   *  when the actual read happens, so this it not a performance
   *  problem)
   */
  picman_pickable_flush (PICMAN_PICKABLE (GET_PRIVATE (group)->projection));
}

static void
picman_group_layer_proj_update (PicmanProjection *proj,
                              gboolean        now,
                              gint            x,
                              gint            y,
                              gint            width,
                              gint            height,
                              PicmanGroupLayer *group)
{
#if 0
  g_printerr ("%s (%s) %d, %d (%d, %d)\n",
              G_STRFUNC, picman_object_get_name (group),
              x, y, width, height);
#endif

  /*  the projection speaks in image coordinates, transform to layer
   *  coordinates when emitting our own update signal.
   */
  picman_drawable_update (PICMAN_DRAWABLE (group),
                        x - picman_item_get_offset_x (PICMAN_ITEM (group)),
                        y - picman_item_get_offset_y (PICMAN_ITEM (group)),
                        width, height);
}
