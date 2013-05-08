/* PICMAN - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * picmanvectors.c
 * Copyright (C) 2002 Simon Budig  <simon@picman.org>
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

#include "vectors-types.h"

#include "core/picman.h"
#include "core/picman-transform-utils.h"
#include "core/picmanbezierdesc.h"
#include "core/picmanchannel-select.h"
#include "core/picmancontainer.h"
#include "core/picmancontext.h"
#include "core/picmandrawable-stroke.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo-push.h"
#include "core/picmanmarshal.h"
#include "core/picmanpaintinfo.h"
#include "core/picmanstrokeoptions.h"

#include "paint/picmanpaintcore-stroke.h"
#include "paint/picmanpaintoptions.h"

#include "picmananchor.h"
#include "picmanstroke.h"
#include "picmanvectors.h"
#include "picmanvectors-preview.h"

#include "picman-intl.h"


enum
{
  FREEZE,
  THAW,
  LAST_SIGNAL
};


static void       picman_vectors_finalize      (GObject           *object);

static gint64     picman_vectors_get_memsize   (PicmanObject        *object,
                                              gint64            *gui_size);

static gboolean   picman_vectors_is_attached   (const PicmanItem    *item);
static PicmanItemTree * picman_vectors_get_tree  (PicmanItem          *item);
static PicmanItem * picman_vectors_duplicate     (PicmanItem          *item,
                                              GType              new_type);
static void       picman_vectors_convert       (PicmanItem          *item,
                                              PicmanImage         *dest_image);
static void       picman_vectors_translate     (PicmanItem          *item,
                                              gint               offset_x,
                                              gint               offset_y,
                                              gboolean           push_undo);
static void       picman_vectors_scale         (PicmanItem          *item,
                                              gint               new_width,
                                              gint               new_height,
                                              gint               new_offset_x,
                                              gint               new_offset_y,
                                              PicmanInterpolationType  interp_type,
                                              PicmanProgress      *progress);
static void       picman_vectors_resize        (PicmanItem          *item,
                                              PicmanContext       *context,
                                              gint               new_width,
                                              gint               new_height,
                                              gint               offset_x,
                                              gint               offset_y);
static void       picman_vectors_flip          (PicmanItem          *item,
                                              PicmanContext       *context,
                                              PicmanOrientationType  flip_type,
                                              gdouble            axis,
                                              gboolean           clip_result);
static void       picman_vectors_rotate        (PicmanItem          *item,
                                              PicmanContext       *context,
                                              PicmanRotationType   rotate_type,
                                              gdouble            center_x,
                                              gdouble            center_y,
                                              gboolean           clip_result);
static void       picman_vectors_transform     (PicmanItem          *item,
                                              PicmanContext       *context,
                                              const PicmanMatrix3 *matrix,
                                              PicmanTransformDirection direction,
                                              PicmanInterpolationType interp_type,
                                              gint               recursion_level,
                                              PicmanTransformResize   clip_result,
                                              PicmanProgress      *progress);
static gboolean   picman_vectors_stroke        (PicmanItem          *item,
                                              PicmanDrawable      *drawable,
                                              PicmanStrokeOptions *stroke_options,
                                              gboolean           push_undo,
                                              PicmanProgress      *progress,
                                              GError           **error);
static void       picman_vectors_to_selection  (PicmanItem          *item,
                                              PicmanChannelOps     op,
                                              gboolean           antialias,
                                              gboolean           feather,
                                              gdouble            feather_radius_x,
                                              gdouble            feather_radius_y);

static void       picman_vectors_real_freeze          (PicmanVectors       *vectors);
static void       picman_vectors_real_thaw            (PicmanVectors       *vectors);
static void       picman_vectors_real_stroke_add      (PicmanVectors       *vectors,
                                                     PicmanStroke        *stroke);
static void       picman_vectors_real_stroke_remove   (PicmanVectors       *vectors,
                                                     PicmanStroke        *stroke);
static PicmanStroke * picman_vectors_real_stroke_get    (const PicmanVectors *vectors,
                                                     const PicmanCoords  *coord);
static PicmanStroke *picman_vectors_real_stroke_get_next(const PicmanVectors *vectors,
                                                     const PicmanStroke  *prev);
static gdouble picman_vectors_real_stroke_get_length  (const PicmanVectors *vectors,
                                                     const PicmanStroke  *prev);
static PicmanAnchor * picman_vectors_real_anchor_get    (const PicmanVectors *vectors,
                                                     const PicmanCoords  *coord,
                                                     PicmanStroke       **ret_stroke);
static void       picman_vectors_real_anchor_delete   (PicmanVectors       *vectors,
                                                     PicmanAnchor        *anchor);
static gdouble    picman_vectors_real_get_length      (const PicmanVectors *vectors,
                                                     const PicmanAnchor  *start);
static gdouble    picman_vectors_real_get_distance    (const PicmanVectors *vectors,
                                                     const PicmanCoords  *coord);
static gint       picman_vectors_real_interpolate     (const PicmanVectors *vectors,
                                                     const PicmanStroke  *stroke,
                                                     gdouble            precision,
                                                     gint               max_points,
                                                     PicmanCoords        *ret_coords);

static PicmanBezierDesc * picman_vectors_make_bezier      (const PicmanVectors *vectors);
static PicmanBezierDesc * picman_vectors_real_make_bezier (const PicmanVectors *vectors);


G_DEFINE_TYPE (PicmanVectors, picman_vectors, PICMAN_TYPE_ITEM)

#define parent_class picman_vectors_parent_class

static guint picman_vectors_signals[LAST_SIGNAL] = { 0 };


static void
picman_vectors_class_init (PicmanVectorsClass *klass)
{
  GObjectClass      *object_class      = G_OBJECT_CLASS (klass);
  PicmanObjectClass   *picman_object_class = PICMAN_OBJECT_CLASS (klass);
  PicmanViewableClass *viewable_class    = PICMAN_VIEWABLE_CLASS (klass);
  PicmanItemClass     *item_class        = PICMAN_ITEM_CLASS (klass);

  picman_vectors_signals[FREEZE] =
    g_signal_new ("freeze",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (PicmanVectorsClass, freeze),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  picman_vectors_signals[THAW] =
    g_signal_new ("thaw",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (PicmanVectorsClass, thaw),
                  NULL, NULL,
                  picman_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  object_class->finalize           = picman_vectors_finalize;

  picman_object_class->get_memsize   = picman_vectors_get_memsize;

  viewable_class->get_new_preview  = picman_vectors_get_new_preview;
  viewable_class->default_stock_id = "picman-path";

  item_class->is_attached          = picman_vectors_is_attached;
  item_class->get_tree             = picman_vectors_get_tree;
  item_class->duplicate            = picman_vectors_duplicate;
  item_class->convert              = picman_vectors_convert;
  item_class->translate            = picman_vectors_translate;
  item_class->scale                = picman_vectors_scale;
  item_class->resize               = picman_vectors_resize;
  item_class->flip                 = picman_vectors_flip;
  item_class->rotate               = picman_vectors_rotate;
  item_class->transform            = picman_vectors_transform;
  item_class->stroke               = picman_vectors_stroke;
  item_class->to_selection         = picman_vectors_to_selection;
  item_class->default_name         = _("Path");
  item_class->rename_desc          = C_("undo-type", "Rename Path");
  item_class->translate_desc       = C_("undo-type", "Move Path");
  item_class->scale_desc           = C_("undo-type", "Scale Path");
  item_class->resize_desc          = C_("undo-type", "Resize Path");
  item_class->flip_desc            = C_("undo-type", "Flip Path");
  item_class->rotate_desc          = C_("undo-type", "Rotate Path");
  item_class->transform_desc       = C_("undo-type", "Transform Path");
  item_class->stroke_desc          = C_("undo-type", "Stroke Path");
  item_class->to_selection_desc    = C_("undo-type", "Path to Selection");
  item_class->reorder_desc         = C_("undo-type", "Reorder Path");
  item_class->raise_desc           = C_("undo-type", "Raise Path");
  item_class->raise_to_top_desc    = C_("undo-type", "Raise Path to Top");
  item_class->lower_desc           = C_("undo-type", "Lower Path");
  item_class->lower_to_bottom_desc = C_("undo-type", "Lower Path to Bottom");
  item_class->raise_failed         = _("Path cannot be raised higher.");
  item_class->lower_failed         = _("Path cannot be lowered more.");

  klass->freeze                    = picman_vectors_real_freeze;
  klass->thaw                      = picman_vectors_real_thaw;

  klass->stroke_add                = picman_vectors_real_stroke_add;
  klass->stroke_remove             = picman_vectors_real_stroke_remove;
  klass->stroke_get                = picman_vectors_real_stroke_get;
  klass->stroke_get_next           = picman_vectors_real_stroke_get_next;
  klass->stroke_get_length         = picman_vectors_real_stroke_get_length;

  klass->anchor_get                = picman_vectors_real_anchor_get;
  klass->anchor_delete             = picman_vectors_real_anchor_delete;

  klass->get_length                = picman_vectors_real_get_length;
  klass->get_distance              = picman_vectors_real_get_distance;
  klass->interpolate               = picman_vectors_real_interpolate;

  klass->make_bezier               = picman_vectors_real_make_bezier;
}

static void
picman_vectors_init (PicmanVectors *vectors)
{
  picman_item_set_visible (PICMAN_ITEM (vectors), FALSE, FALSE);

  vectors->strokes        = NULL;
  vectors->last_stroke_ID = 0;
  vectors->freeze_count   = 0;
  vectors->precision      = 0.2;

  vectors->bezier_desc    = NULL;
  vectors->bounds_valid   = FALSE;
}

static void
picman_vectors_finalize (GObject *object)
{
  PicmanVectors *vectors = PICMAN_VECTORS (object);

  if (vectors->bezier_desc)
    {
      picman_bezier_desc_free (vectors->bezier_desc);
      vectors->bezier_desc = NULL;
    }

  if (vectors->strokes)
    {
      g_list_free_full (vectors->strokes, (GDestroyNotify) g_object_unref);
      vectors->strokes = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint64
picman_vectors_get_memsize (PicmanObject *object,
                          gint64     *gui_size)
{
  PicmanVectors *vectors;
  GList       *list;
  gint64       memsize = 0;

  vectors = PICMAN_VECTORS (object);

  for (list = vectors->strokes; list; list = g_list_next (list))
    memsize += (picman_object_get_memsize (PICMAN_OBJECT (list->data), gui_size) +
                sizeof (GList));

  return memsize + PICMAN_OBJECT_CLASS (parent_class)->get_memsize (object,
                                                                  gui_size);
}

static gboolean
picman_vectors_is_attached (const PicmanItem *item)
{
  PicmanImage *image = picman_item_get_image (item);

  return (PICMAN_IS_IMAGE (image) &&
          picman_container_have (picman_image_get_vectors (image),
                               PICMAN_OBJECT (item)));
}

static PicmanItemTree *
picman_vectors_get_tree (PicmanItem *item)
{
  if (picman_item_is_attached (item))
    {
      PicmanImage *image = picman_item_get_image (item);

      return picman_image_get_vectors_tree (image);
    }

  return NULL;
}

static PicmanItem *
picman_vectors_duplicate (PicmanItem *item,
                        GType     new_type)
{
  PicmanItem *new_item;

  g_return_val_if_fail (g_type_is_a (new_type, PICMAN_TYPE_VECTORS), NULL);

  new_item = PICMAN_ITEM_CLASS (parent_class)->duplicate (item, new_type);

  if (PICMAN_IS_VECTORS (new_item))
    {
      PicmanVectors *vectors     = PICMAN_VECTORS (item);
      PicmanVectors *new_vectors = PICMAN_VECTORS (new_item);

      picman_vectors_copy_strokes (vectors, new_vectors);
    }

  return new_item;
}

static void
picman_vectors_convert (PicmanItem  *item,
                      PicmanImage *dest_image)
{
  picman_item_set_size (item,
                      picman_image_get_width  (dest_image),
                      picman_image_get_height (dest_image));

  PICMAN_ITEM_CLASS (parent_class)->convert (item, dest_image);
}

static void
picman_vectors_translate (PicmanItem *item,
                        gint      offset_x,
                        gint      offset_y,
                        gboolean  push_undo)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  GList       *list;

  picman_vectors_freeze (vectors);

  if (push_undo)
    picman_image_undo_push_vectors_mod (picman_item_get_image (item),
                                      _("Move Path"),
                                      vectors);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_translate (stroke, offset_x, offset_y);
    }

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_scale (PicmanItem              *item,
                    gint                   new_width,
                    gint                   new_height,
                    gint                   new_offset_x,
                    gint                   new_offset_y,
                    PicmanInterpolationType  interpolation_type,
                    PicmanProgress          *progress)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  PicmanImage   *image   = picman_item_get_image (item);
  GList       *list;

  picman_vectors_freeze (vectors);

  if (picman_item_is_attached (item))
    picman_image_undo_push_vectors_mod (image, NULL, vectors);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_scale (stroke,
                         (gdouble) new_width  / (gdouble) picman_item_get_width  (item),
                         (gdouble) new_height / (gdouble) picman_item_get_height (item));
      picman_stroke_translate (stroke, new_offset_x, new_offset_y);
    }

  PICMAN_ITEM_CLASS (parent_class)->scale (item,
                                         picman_image_get_width  (image),
                                         picman_image_get_height (image),
                                         0, 0,
                                         interpolation_type, progress);

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_resize (PicmanItem    *item,
                     PicmanContext *context,
                     gint         new_width,
                     gint         new_height,
                     gint         offset_x,
                     gint         offset_y)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  PicmanImage   *image   = picman_item_get_image (item);
  GList       *list;

  picman_vectors_freeze (vectors);

  if (picman_item_is_attached (item))
    picman_image_undo_push_vectors_mod (image, NULL, vectors);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_translate (stroke, offset_x, offset_y);
    }

  PICMAN_ITEM_CLASS (parent_class)->resize (item, context,
                                          picman_image_get_width  (image),
                                          picman_image_get_height (image),
                                          0, 0);

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_flip (PicmanItem            *item,
                   PicmanContext         *context,
                   PicmanOrientationType  flip_type,
                   gdouble              axis,
                   gboolean             clip_result)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  GList       *list;
  PicmanMatrix3  matrix;

  picman_matrix3_identity (&matrix);
  picman_transform_matrix_flip (&matrix, flip_type, axis);

  picman_vectors_freeze (vectors);

  picman_image_undo_push_vectors_mod (picman_item_get_image (item),
                                    _("Flip Path"),
                                    vectors);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_transform (stroke, &matrix);
    }

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_rotate (PicmanItem         *item,
                     PicmanContext      *context,
                     PicmanRotationType  rotate_type,
                     gdouble           center_x,
                     gdouble           center_y,
                     gboolean          clip_result)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  GList       *list;
  PicmanMatrix3  matrix;

  picman_matrix3_identity (&matrix);
  picman_transform_matrix_rotate (&matrix, rotate_type, center_x, center_y);

  picman_vectors_freeze (vectors);

  picman_image_undo_push_vectors_mod (picman_item_get_image (item),
                                    _("Rotate Path"),
                                    vectors);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_transform (stroke, &matrix);
    }

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_transform (PicmanItem               *item,
                        PicmanContext            *context,
                        const PicmanMatrix3      *matrix,
                        PicmanTransformDirection  direction,
                        PicmanInterpolationType   interpolation_type,
                        gint                    recursion_level,
                        PicmanTransformResize     clip_result,
                        PicmanProgress           *progress)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  PicmanMatrix3  local_matrix;
  GList       *list;

  picman_vectors_freeze (vectors);

  picman_image_undo_push_vectors_mod (picman_item_get_image (item),
                                    _("Transform Path"),
                                    vectors);

  local_matrix = *matrix;

  if (direction == PICMAN_TRANSFORM_BACKWARD)
    picman_matrix3_invert (&local_matrix);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_transform (stroke, &local_matrix);
    }

  picman_vectors_thaw (vectors);
}

static gboolean
picman_vectors_stroke (PicmanItem           *item,
                     PicmanDrawable       *drawable,
                     PicmanStrokeOptions  *stroke_options,
                     gboolean            push_undo,
                     PicmanProgress       *progress,
                     GError            **error)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  gboolean     retval  = FALSE;

  if (! vectors->strokes)
    {
      g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
                           _("Not enough points to stroke"));
      return FALSE;
    }

  switch (picman_stroke_options_get_method (stroke_options))
    {
    case PICMAN_STROKE_METHOD_LIBART:
      retval = picman_drawable_stroke_vectors (drawable,
                                             stroke_options,
                                             vectors, push_undo, error);
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

        retval = picman_paint_core_stroke_vectors (core, drawable,
                                                 paint_options,
                                                 emulate_dynamics,
                                                 vectors, push_undo, error);

        g_object_unref (core);
      }
      break;

    default:
      g_return_val_if_reached (FALSE);
    }

  return retval;
}

static void
picman_vectors_to_selection (PicmanItem       *item,
                           PicmanChannelOps  op,
                           gboolean        antialias,
                           gboolean        feather,
                           gdouble         feather_radius_x,
                           gdouble         feather_radius_y)
{
  PicmanVectors *vectors = PICMAN_VECTORS (item);
  PicmanImage   *image   = picman_item_get_image (item);

  picman_channel_select_vectors (picman_image_get_mask (image),
                               PICMAN_ITEM_GET_CLASS (item)->to_selection_desc,
                               vectors,
                               op, antialias,
                               feather, feather_radius_x, feather_radius_x,
                               TRUE);
}

static void
picman_vectors_real_freeze (PicmanVectors *vectors)
{
  /*  release cached bezier representation  */
  if (vectors->bezier_desc)
    {
      picman_bezier_desc_free (vectors->bezier_desc);
      vectors->bezier_desc = NULL;
    }

  /*  invalidate bounds  */
  vectors->bounds_valid = FALSE;
}

static void
picman_vectors_real_thaw (PicmanVectors *vectors)
{
  picman_viewable_invalidate_preview (PICMAN_VIEWABLE (vectors));
}


/*  public functions  */

PicmanVectors *
picman_vectors_new (PicmanImage   *image,
                  const gchar *name)
{
  PicmanVectors *vectors;

  g_return_val_if_fail (PICMAN_IS_IMAGE (image), NULL);

  vectors = PICMAN_VECTORS (picman_item_new (PICMAN_TYPE_VECTORS,
                                         image, name,
                                         0, 0,
                                         picman_image_get_width  (image),
                                         picman_image_get_height (image)));

  return vectors;
}

PicmanVectors *
picman_vectors_get_parent (PicmanVectors *vectors)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  return PICMAN_VECTORS (picman_viewable_get_parent (PICMAN_VIEWABLE (vectors)));
}

void
picman_vectors_freeze (PicmanVectors *vectors)
{
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));

  vectors->freeze_count++;

  if (vectors->freeze_count == 1)
    g_signal_emit (vectors, picman_vectors_signals[FREEZE], 0);
}

void
picman_vectors_thaw (PicmanVectors *vectors)
{
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));
  g_return_if_fail (vectors->freeze_count > 0);

  vectors->freeze_count--;

  if (vectors->freeze_count == 0)
    g_signal_emit (vectors, picman_vectors_signals[THAW], 0);
}

void
picman_vectors_copy_strokes (const PicmanVectors *src_vectors,
                           PicmanVectors       *dest_vectors)
{
  g_return_if_fail (PICMAN_IS_VECTORS (src_vectors));
  g_return_if_fail (PICMAN_IS_VECTORS (dest_vectors));

  picman_vectors_freeze (dest_vectors);

  if (dest_vectors->strokes)
    {
      g_list_free_full (dest_vectors->strokes, (GDestroyNotify) g_object_unref);
    }

  dest_vectors->strokes = NULL;
  dest_vectors->last_stroke_ID = 0;

  picman_vectors_add_strokes (src_vectors, dest_vectors);

  picman_vectors_thaw (dest_vectors);
}


void
picman_vectors_add_strokes (const PicmanVectors *src_vectors,
                          PicmanVectors       *dest_vectors)
{
  GList *current_lstroke;
  GList *strokes_copy;

  g_return_if_fail (PICMAN_IS_VECTORS (src_vectors));
  g_return_if_fail (PICMAN_IS_VECTORS (dest_vectors));

  picman_vectors_freeze (dest_vectors);

  strokes_copy = g_list_copy (src_vectors->strokes);
  current_lstroke = strokes_copy;

  while (current_lstroke)
    {
      current_lstroke->data = picman_stroke_duplicate (current_lstroke->data);
      dest_vectors->last_stroke_ID ++;
      picman_stroke_set_ID (current_lstroke->data,
                          dest_vectors->last_stroke_ID);
      current_lstroke = g_list_next (current_lstroke);
    }

  dest_vectors->strokes = g_list_concat (dest_vectors->strokes, strokes_copy);

  picman_vectors_thaw (dest_vectors);
}


void
picman_vectors_stroke_add (PicmanVectors *vectors,
                         PicmanStroke  *stroke)
{
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  picman_vectors_freeze (vectors);

  PICMAN_VECTORS_GET_CLASS (vectors)->stroke_add (vectors, stroke);

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_real_stroke_add (PicmanVectors *vectors,
                              PicmanStroke  *stroke)
{
  /*  Don't g_list_prepend() here.  See ChangeLog 2003-05-21 --Mitch  */

  vectors->strokes = g_list_append (vectors->strokes, stroke);
  vectors->last_stroke_ID ++;
  picman_stroke_set_ID (stroke, vectors->last_stroke_ID);
  g_object_ref (stroke);
}

void
picman_vectors_stroke_remove (PicmanVectors *vectors,
                            PicmanStroke  *stroke)
{
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));
  g_return_if_fail (PICMAN_IS_STROKE (stroke));

  picman_vectors_freeze (vectors);

  PICMAN_VECTORS_GET_CLASS (vectors)->stroke_remove (vectors, stroke);

  picman_vectors_thaw (vectors);
}

static void
picman_vectors_real_stroke_remove (PicmanVectors *vectors,
                                 PicmanStroke  *stroke)
{
  GList *list;

  list = g_list_find (vectors->strokes, stroke);

  if (list)
    {
      vectors->strokes = g_list_delete_link (vectors->strokes, list);
      g_object_unref (stroke);
    }
}

gint
picman_vectors_get_n_strokes (const PicmanVectors *vectors)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0);

  return g_list_length (vectors->strokes);
}


PicmanStroke *
picman_vectors_stroke_get (const PicmanVectors *vectors,
                         const PicmanCoords  *coord)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  return PICMAN_VECTORS_GET_CLASS (vectors)->stroke_get (vectors, coord);
}

static PicmanStroke *
picman_vectors_real_stroke_get (const PicmanVectors *vectors,
                              const PicmanCoords  *coord)
{
  PicmanStroke *minstroke = NULL;
  gdouble     mindist   = G_MAXDOUBLE;
  GList      *list;

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;
      PicmanAnchor *anchor = picman_stroke_anchor_get (stroke, coord);

      if (anchor)
        {
          gdouble dx = coord->x - anchor->position.x;
          gdouble dy = coord->y - anchor->position.y;

          if (mindist > dx * dx + dy * dy)
            {
              mindist   = dx * dx + dy * dy;
              minstroke = stroke;
            }
        }
    }

  return minstroke;
}

PicmanStroke *
picman_vectors_stroke_get_by_ID (const PicmanVectors *vectors,
                               gint               id)
{
  GList *list;

  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      if (picman_stroke_get_ID (list->data) == id)
        return list->data;
    }

  return NULL;
}


PicmanStroke *
picman_vectors_stroke_get_next (const PicmanVectors *vectors,
                              const PicmanStroke  *prev)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  return PICMAN_VECTORS_GET_CLASS (vectors)->stroke_get_next (vectors, prev);
}

static PicmanStroke *
picman_vectors_real_stroke_get_next (const PicmanVectors *vectors,
                                   const PicmanStroke  *prev)
{
  if (! prev)
    {
      return vectors->strokes ? vectors->strokes->data : NULL;
    }
  else
    {
      GList *stroke;

      stroke = g_list_find (vectors->strokes, prev);

      g_return_val_if_fail (stroke != NULL, NULL);

      return stroke->next ? PICMAN_STROKE (stroke->next->data) : NULL;
    }
}


gdouble
picman_vectors_stroke_get_length (const PicmanVectors *vectors,
                                const PicmanStroke  *stroke)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0.0);
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), 0.0);

  return PICMAN_VECTORS_GET_CLASS (vectors)->stroke_get_length (vectors, stroke);
}

static gdouble
picman_vectors_real_stroke_get_length (const PicmanVectors *vectors,
                                     const PicmanStroke  *stroke)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0.0);
  g_return_val_if_fail (PICMAN_IS_STROKE (stroke), 0.0);

  return (picman_stroke_get_length (stroke, vectors->precision));

  return 0.0;
}


PicmanAnchor *
picman_vectors_anchor_get (const PicmanVectors *vectors,
                         const PicmanCoords  *coord,
                         PicmanStroke       **ret_stroke)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  return PICMAN_VECTORS_GET_CLASS (vectors)->anchor_get (vectors, coord,
                                                       ret_stroke);
}

static PicmanAnchor *
picman_vectors_real_anchor_get (const PicmanVectors *vectors,
                              const PicmanCoords  *coord,
                              PicmanStroke       **ret_stroke)
{
  PicmanAnchor *minanchor = NULL;
  gdouble     mindist   = -1;
  GList      *list;

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;
      PicmanAnchor *anchor = picman_stroke_anchor_get (stroke, coord);

      if (anchor)
        {
          gdouble dx = coord->x - anchor->position.x;
          gdouble dy = coord->y - anchor->position.y;

          if (mindist > dx * dx + dy * dy || mindist < 0)
            {
              mindist   = dx * dx + dy * dy;
              minanchor = anchor;

              if (ret_stroke)
                *ret_stroke = stroke;
            }
        }
    }

  return minanchor;
}


void
picman_vectors_anchor_delete (PicmanVectors *vectors,
                            PicmanAnchor  *anchor)
{
  g_return_if_fail (PICMAN_IS_VECTORS (vectors));
  g_return_if_fail (anchor != NULL);

  PICMAN_VECTORS_GET_CLASS (vectors)->anchor_delete (vectors, anchor);
}

static void
picman_vectors_real_anchor_delete (PicmanVectors *vectors,
                                 PicmanAnchor  *anchor)
{
}


void
picman_vectors_anchor_select (PicmanVectors *vectors,
                            PicmanStroke  *target_stroke,
                            PicmanAnchor  *anchor,
                            gboolean     selected,
                            gboolean     exclusive)
{
  GList *list;

  for (list = vectors->strokes; list; list = g_list_next (list))
    {
      PicmanStroke *stroke = list->data;

      picman_stroke_anchor_select (stroke,
                                 stroke == target_stroke ? anchor : NULL,
                                 selected, exclusive);
    }
}


gdouble
picman_vectors_get_length (const PicmanVectors *vectors,
                         const PicmanAnchor  *start)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0.0);

  return PICMAN_VECTORS_GET_CLASS (vectors)->get_length (vectors, start);
}

static gdouble
picman_vectors_real_get_length (const PicmanVectors *vectors,
                              const PicmanAnchor  *start)
{
  g_printerr ("picman_vectors_get_length: default implementation\n");

  return 0;
}


gdouble
picman_vectors_get_distance (const PicmanVectors *vectors,
                           const PicmanCoords  *coord)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0.0);

  return PICMAN_VECTORS_GET_CLASS (vectors)->get_distance (vectors, coord);
}

static gdouble
picman_vectors_real_get_distance (const PicmanVectors *vectors,
                                const PicmanCoords  *coord)
{
  g_printerr ("picman_vectors_get_distance: default implementation\n");

  return 0;
}

gboolean
picman_vectors_bounds (PicmanVectors *vectors,
                     gdouble     *x1,
                     gdouble     *y1,
                     gdouble     *x2,
                     gdouble     *y2)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), FALSE);
  g_return_val_if_fail (x1 != NULL && y1 != NULL &&
                        x2 != NULL && y2 != NULL, FALSE);

  if (! vectors->bounds_valid)
    {
      PicmanStroke *stroke;

      vectors->bounds_empty = TRUE;
      vectors->bounds_x1 = vectors->bounds_x2 = 0.0;
      vectors->bounds_x1 = vectors->bounds_x2 = 0.0;

      for (stroke = picman_vectors_stroke_get_next (vectors, NULL);
           stroke;
           stroke = picman_vectors_stroke_get_next (vectors, stroke))
        {
          GArray   *stroke_coords;
          gboolean  closed;

          stroke_coords = picman_stroke_interpolate (stroke, 1.0, &closed);

          if (stroke_coords)
            {
              PicmanCoords point;
              gint       i;

              if (vectors->bounds_empty && stroke_coords->len > 0)
                {
                  point = g_array_index (stroke_coords, PicmanCoords, 0);

                  vectors->bounds_x1 = vectors->bounds_x2 = point.x;
                  vectors->bounds_y1 = vectors->bounds_y2 = point.y;

                  vectors->bounds_empty = FALSE;
                }

              for (i = 0; i < stroke_coords->len; i++)
                {
                  point = g_array_index (stroke_coords, PicmanCoords, i);

                  vectors->bounds_x1 = MIN (vectors->bounds_x1, point.x);
                  vectors->bounds_y1 = MIN (vectors->bounds_y1, point.y);
                  vectors->bounds_x2 = MAX (vectors->bounds_x2, point.x);
                  vectors->bounds_y2 = MAX (vectors->bounds_y2, point.y);
                }

              g_array_free (stroke_coords, TRUE);
            }
        }

      vectors->bounds_valid = TRUE;
    }

  *x1 = vectors->bounds_x1;
  *y1 = vectors->bounds_y1;
  *x2 = vectors->bounds_x2;
  *y2 = vectors->bounds_y2;

  return (! vectors->bounds_empty);
}

gint
picman_vectors_interpolate (const PicmanVectors *vectors,
                          const PicmanStroke  *stroke,
                          gdouble            precision,
                          gint               max_points,
                          PicmanCoords        *ret_coords)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), 0);

  return PICMAN_VECTORS_GET_CLASS (vectors)->interpolate (vectors, stroke,
                                                        precision, max_points,
                                                        ret_coords);
}

static gint
picman_vectors_real_interpolate (const PicmanVectors *vectors,
                               const PicmanStroke  *stroke,
                               gdouble            precision,
                               gint               max_points,
                               PicmanCoords        *ret_coords)
{
  g_printerr ("picman_vectors_interpolate: default implementation\n");

  return 0;
}

const PicmanBezierDesc *
picman_vectors_get_bezier (PicmanVectors *vectors)
{
  g_return_val_if_fail (PICMAN_IS_VECTORS (vectors), NULL);

  if (! vectors->bezier_desc)
    {
      vectors->bezier_desc = picman_vectors_make_bezier (vectors);
    }

  return  vectors->bezier_desc;
}

static PicmanBezierDesc *
picman_vectors_make_bezier (const PicmanVectors *vectors)
{
  return PICMAN_VECTORS_GET_CLASS (vectors)->make_bezier (vectors);
}

static PicmanBezierDesc *
picman_vectors_real_make_bezier (const PicmanVectors *vectors)
{
  PicmanStroke     *stroke;
  GArray         *cmd_array;
  PicmanBezierDesc *ret_bezdesc = NULL;

  cmd_array = g_array_new (FALSE, FALSE, sizeof (cairo_path_data_t));

  for (stroke = picman_vectors_stroke_get_next (vectors, NULL);
       stroke;
       stroke = picman_vectors_stroke_get_next (vectors, stroke))
    {
      PicmanBezierDesc *bezdesc = picman_stroke_make_bezier (stroke);

      if (bezdesc)
        {
          cmd_array = g_array_append_vals (cmd_array, bezdesc->data,
                                           bezdesc->num_data);
          picman_bezier_desc_free (bezdesc);
        }
    }

  if (cmd_array->len > 0)
    ret_bezdesc = picman_bezier_desc_new ((cairo_path_data_t *) cmd_array->data,
                                        cmd_array->len);

  g_array_free (cmd_array, FALSE);

  return ret_bezdesc;
}
