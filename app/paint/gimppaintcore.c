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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"
#include "libpicmanmath/picmanmath.h"

#include "paint-types.h"

#include "gegl/picman-gegl-loops.h"
#include "gegl/picman-gegl-nodes.h"
#include "gegl/picman-gegl-utils.h"
#include "gegl/picmanapplicator.h"

#include "core/picman.h"
#include "core/picman-utils.h"
#include "core/picmanchannel.h"
#include "core/picmanimage.h"
#include "core/picmanimage-undo.h"
#include "core/picmanpickable.h"
#include "core/picmanprojection.h"
#include "core/picmantempbuf.h"

#include "picmanpaintcore.h"
#include "picmanpaintcoreundo.h"
#include "picmanpaintoptions.h"

#include "picmanairbrush.h"

#include "picman-intl.h"


#define STROKE_BUFFER_INIT_SIZE 2000

enum
{
  PROP_0,
  PROP_UNDO_DESC
};


/*  local function prototypes  */

static void      picman_paint_core_finalize            (GObject          *object);
static void      picman_paint_core_set_property        (GObject          *object,
                                                      guint             property_id,
                                                      const GValue     *value,
                                                      GParamSpec       *pspec);
static void      picman_paint_core_get_property        (GObject          *object,
                                                      guint             property_id,
                                                      GValue           *value,
                                                      GParamSpec       *pspec);

static gboolean  picman_paint_core_real_start          (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *paint_options,
                                                      const PicmanCoords *coords,
                                                      GError          **error);
static gboolean  picman_paint_core_real_pre_paint      (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *options,
                                                      PicmanPaintState    paint_state,
                                                      guint32           time);
static void      picman_paint_core_real_paint          (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *options,
                                                      const PicmanCoords *coords,
                                                      PicmanPaintState    paint_state,
                                                      guint32           time);
static void      picman_paint_core_real_post_paint     (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *options,
                                                      PicmanPaintState    paint_state,
                                                      guint32           time);
static void      picman_paint_core_real_interpolate    (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *options,
                                                      guint32           time);
static GeglBuffer *
               picman_paint_core_real_get_paint_buffer (PicmanPaintCore    *core,
                                                      PicmanDrawable     *drawable,
                                                      PicmanPaintOptions *options,
                                                      const PicmanCoords *coords,
                                                      gint             *paint_buffer_x,
                                                      gint             *paint_buffer_y);
static PicmanUndo* picman_paint_core_real_push_undo      (PicmanPaintCore    *core,
                                                      PicmanImage        *image,
                                                      const gchar      *undo_desc);


G_DEFINE_TYPE (PicmanPaintCore, picman_paint_core, PICMAN_TYPE_OBJECT)

#define parent_class picman_paint_core_parent_class

static gint global_core_ID = 1;


static void
picman_paint_core_class_init (PicmanPaintCoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize     = picman_paint_core_finalize;
  object_class->set_property = picman_paint_core_set_property;
  object_class->get_property = picman_paint_core_get_property;

  klass->start               = picman_paint_core_real_start;
  klass->pre_paint           = picman_paint_core_real_pre_paint;
  klass->paint               = picman_paint_core_real_paint;
  klass->post_paint          = picman_paint_core_real_post_paint;
  klass->interpolate         = picman_paint_core_real_interpolate;
  klass->get_paint_buffer    = picman_paint_core_real_get_paint_buffer;
  klass->push_undo           = picman_paint_core_real_push_undo;

  g_object_class_install_property (object_class, PROP_UNDO_DESC,
                                   g_param_spec_string ("undo-desc", NULL, NULL,
                                                        _("Paint"),
                                                        PICMAN_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));
}

static void
picman_paint_core_init (PicmanPaintCore *core)
{
  core->ID = global_core_ID++;
}

static void
picman_paint_core_finalize (GObject *object)
{
  PicmanPaintCore *core = PICMAN_PAINT_CORE (object);

  picman_paint_core_cleanup (core);

  g_free (core->undo_desc);
  core->undo_desc = NULL;

  if (core->stroke_buffer)
    {
      g_array_free (core->stroke_buffer, TRUE);
      core->stroke_buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
picman_paint_core_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PicmanPaintCore *core = PICMAN_PAINT_CORE (object);

  switch (property_id)
    {
    case PROP_UNDO_DESC:
      g_free (core->undo_desc);
      core->undo_desc = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_paint_core_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PicmanPaintCore *core = PICMAN_PAINT_CORE (object);

  switch (property_id)
    {
    case PROP_UNDO_DESC:
      g_value_set_string (value, core->undo_desc);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_paint_core_real_start (PicmanPaintCore    *core,
                            PicmanDrawable     *drawable,
                            PicmanPaintOptions *paint_options,
                            const PicmanCoords *coords,
                            GError          **error)
{
  return TRUE;
}

static gboolean
picman_paint_core_real_pre_paint (PicmanPaintCore    *core,
                                PicmanDrawable     *drawable,
                                PicmanPaintOptions *paint_options,
                                PicmanPaintState    paint_state,
                                guint32           time)
{
  return TRUE;
}

static void
picman_paint_core_real_paint (PicmanPaintCore    *core,
                            PicmanDrawable     *drawable,
                            PicmanPaintOptions *paint_options,
                            const PicmanCoords *coords,
                            PicmanPaintState    paint_state,
                            guint32           time)
{
}

static void
picman_paint_core_real_post_paint (PicmanPaintCore    *core,
                                 PicmanDrawable     *drawable,
                                 PicmanPaintOptions *paint_options,
                                 PicmanPaintState    paint_state,
                                 guint32           time)
{
}

static void
picman_paint_core_real_interpolate (PicmanPaintCore    *core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  guint32           time)
{
  picman_paint_core_paint (core, drawable, paint_options,
                         PICMAN_PAINT_STATE_MOTION, time);

  core->last_coords = core->cur_coords;
}

static GeglBuffer *
picman_paint_core_real_get_paint_buffer (PicmanPaintCore    *core,
                                       PicmanDrawable     *drawable,
                                       PicmanPaintOptions *paint_options,
                                       const PicmanCoords *coords,
                                       gint             *paint_buffer_x,
                                       gint             *paint_buffer_y)
{
  return NULL;
}

static PicmanUndo *
picman_paint_core_real_push_undo (PicmanPaintCore *core,
                                PicmanImage     *image,
                                const gchar   *undo_desc)
{
  return picman_image_undo_push (image, PICMAN_TYPE_PAINT_CORE_UNDO,
                               PICMAN_UNDO_PAINT, undo_desc,
                               0,
                               "paint-core", core,
                               NULL);
}


/*  public functions  */

void
picman_paint_core_paint (PicmanPaintCore    *core,
                       PicmanDrawable     *drawable,
                       PicmanPaintOptions *paint_options,
                       PicmanPaintState    paint_state,
                       guint32           time)
{
  PicmanPaintCoreClass *core_class;

  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options));

  core_class = PICMAN_PAINT_CORE_GET_CLASS (core);

  if (core_class->pre_paint (core, drawable,
                             paint_options,
                             paint_state, time))
    {

      if (paint_state == PICMAN_PAINT_STATE_MOTION)
        {
          /* Save coordinates for picman_paint_core_interpolate() */
          core->last_paint.x = core->cur_coords.x;
          core->last_paint.y = core->cur_coords.y;
        }

      core_class->paint (core, drawable,
                         paint_options,
                         &core->cur_coords,
                         paint_state, time);

      core_class->post_paint (core, drawable,
                              paint_options,
                              paint_state, time);
    }
}

gboolean
picman_paint_core_start (PicmanPaintCore     *core,
                       PicmanDrawable      *drawable,
                       PicmanPaintOptions  *paint_options,
                       const PicmanCoords  *coords,
                       GError           **error)
{
  PicmanImage   *image;
  PicmanItem    *item;
  PicmanChannel *mask;

  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), FALSE);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), FALSE);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), FALSE);
  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), FALSE);
  g_return_val_if_fail (coords != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  item  = PICMAN_ITEM (drawable);
  image = picman_item_get_image (item);

  if (core->stroke_buffer)
    {
      g_array_free (core->stroke_buffer, TRUE);
      core->stroke_buffer = NULL;
    }

  core->stroke_buffer = g_array_sized_new (TRUE, TRUE,
                                           sizeof (PicmanCoords),
                                           STROKE_BUFFER_INIT_SIZE);

  core->cur_coords = *coords;

  if (! PICMAN_PAINT_CORE_GET_CLASS (core)->start (core, drawable,
                                                 paint_options,
                                                 coords, error))
    {
      return FALSE;
    }

  /*  Allocate the undo structure  */
  if (core->undo_buffer)
    g_object_unref (core->undo_buffer);

  core->undo_buffer = gegl_buffer_dup (picman_drawable_get_buffer (drawable));

  /*  Allocate the saved proj structure  */
  if (core->saved_proj_buffer)
    {
      g_object_unref (core->saved_proj_buffer);
      core->saved_proj_buffer = NULL;
    }

  if (core->use_saved_proj)
    {
      PicmanPickable *pickable = PICMAN_PICKABLE (picman_image_get_projection (image));
      GeglBuffer   *buffer   = picman_pickable_get_buffer (pickable);

      core->saved_proj_buffer = gegl_buffer_dup (buffer);
    }

  /*  Allocate the canvas blocks structure  */
  if (core->canvas_buffer)
    g_object_unref (core->canvas_buffer);

  core->canvas_buffer =
    gegl_buffer_new (GEGL_RECTANGLE (0, 0,
                                     picman_item_get_width  (item),
                                     picman_item_get_height (item)),
                     babl_format ("Y float"));

  /*  Get the initial undo extents  */

  core->x1 = core->x2 = core->cur_coords.x;
  core->y1 = core->y2 = core->cur_coords.y;

  core->last_paint.x = -1e6;
  core->last_paint.y = -1e6;

  mask = picman_image_get_mask (image);

  /*  don't apply the mask to itself and don't apply an empty mask  */
  if (PICMAN_DRAWABLE (mask) == drawable || picman_channel_is_empty (mask))
    mask = NULL;

  core->applicator = picman_applicator_new (NULL,
                                          picman_drawable_get_linear (drawable));

  if (mask)
    {
      GeglBuffer *mask_buffer;
      gint        offset_x;
      gint        offset_y;

      mask_buffer = picman_drawable_get_buffer (PICMAN_DRAWABLE (mask));
      picman_item_get_offset (item, &offset_x, &offset_y);

      picman_applicator_set_mask_buffer (core->applicator, mask_buffer);
      picman_applicator_set_mask_offset (core->applicator,
                                       -offset_x, -offset_y);
    }

  picman_applicator_set_affect (core->applicator,
                              picman_drawable_get_active_mask (drawable));
  picman_applicator_set_dest_buffer (core->applicator,
                                   picman_drawable_get_buffer (drawable));

  /*  Freeze the drawable preview so that it isn't constantly updated.  */
  picman_viewable_preview_freeze (PICMAN_VIEWABLE (drawable));

  return TRUE;
}

void
picman_paint_core_finish (PicmanPaintCore *core,
                        PicmanDrawable  *drawable,
                        gboolean       push_undo)
{
  PicmanImage *image;

  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));

  if (core->applicator)
    {
      g_object_unref (core->applicator);
      core->applicator = NULL;
    }

  if (core->stroke_buffer)
    {
      g_array_free (core->stroke_buffer, TRUE);
      core->stroke_buffer = NULL;
    }

  image = picman_item_get_image (PICMAN_ITEM (drawable));

  /*  Determine if any part of the image has been altered--
   *  if nothing has, then just return...
   */
  if ((core->x2 == core->x1) || (core->y2 == core->y1))
    {
      picman_viewable_preview_thaw (PICMAN_VIEWABLE (drawable));
      return;
    }

  if (push_undo)
    {
      GeglBuffer *buffer;
      gint        x, y, width, height;

      picman_rectangle_intersect (core->x1, core->y1,
                                core->x2 - core->x1, core->y2 - core->y1,
                                0, 0,
                                picman_item_get_width  (PICMAN_ITEM (drawable)),
                                picman_item_get_height (PICMAN_ITEM (drawable)),
                                &x, &y, &width, &height);

      picman_image_undo_group_start (image, PICMAN_UNDO_GROUP_PAINT,
                                   core->undo_desc);

      PICMAN_PAINT_CORE_GET_CLASS (core)->push_undo (core, image, NULL);

      buffer = gegl_buffer_new (GEGL_RECTANGLE (0, 0, width, height),
                                picman_drawable_get_format (drawable));

      gegl_buffer_copy (core->undo_buffer,
                        GEGL_RECTANGLE (x, y, width, height),
                        buffer,
                        GEGL_RECTANGLE (0, 0, 0, 0));

      picman_drawable_push_undo (drawable, NULL,
                               buffer, x, y, width, height);

      g_object_unref (buffer);

      picman_image_undo_group_end (image);
    }

  g_object_unref (core->undo_buffer);
  core->undo_buffer = NULL;

  if (core->saved_proj_buffer)
    {
      g_object_unref (core->saved_proj_buffer);
      core->saved_proj_buffer = NULL;
    }

  picman_viewable_preview_thaw (PICMAN_VIEWABLE (drawable));
}

void
picman_paint_core_cancel (PicmanPaintCore *core,
                        PicmanDrawable  *drawable)
{
  gint x, y;
  gint width, height;

  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));

  /*  Determine if any part of the image has been altered--
   *  if nothing has, then just return...
   */
  if ((core->x2 == core->x1) || (core->y2 == core->y1))
    return;

  if (picman_rectangle_intersect (core->x1, core->y1,
                                core->x2 - core->x1,
                                core->y2 - core->y1,
                                0, 0,
                                picman_item_get_width  (PICMAN_ITEM (drawable)),
                                picman_item_get_height (PICMAN_ITEM (drawable)),
                                &x, &y, &width, &height))
    {
      gegl_buffer_copy (core->undo_buffer,
                        GEGL_RECTANGLE (x, y, width, height),
                        picman_drawable_get_buffer (drawable),
                        GEGL_RECTANGLE (x, y, width, height));
    }

  g_object_unref (core->undo_buffer);
  core->undo_buffer = NULL;

  if (core->saved_proj_buffer)
    {
      g_object_unref (core->saved_proj_buffer);
      core->saved_proj_buffer = NULL;
    }

  picman_drawable_update (drawable, x, y, width, height);

  picman_viewable_preview_thaw (PICMAN_VIEWABLE (drawable));
}

void
picman_paint_core_cleanup (PicmanPaintCore *core)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));

  if (core->undo_buffer)
    {
      g_object_unref (core->undo_buffer);
      core->undo_buffer = NULL;
    }

  if (core->saved_proj_buffer)
    {
      g_object_unref (core->saved_proj_buffer);
      core->saved_proj_buffer = NULL;
    }

  if (core->canvas_buffer)
    {
      g_object_unref (core->canvas_buffer);
      core->canvas_buffer = NULL;
    }

  if (core->paint_buffer)
    {
      g_object_unref (core->paint_buffer);
      core->paint_buffer = NULL;
    }
}

void
picman_paint_core_interpolate (PicmanPaintCore    *core,
                             PicmanDrawable     *drawable,
                             PicmanPaintOptions *paint_options,
                             const PicmanCoords *coords,
                             guint32           time)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (PICMAN_IS_DRAWABLE (drawable));
  g_return_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options));
  g_return_if_fail (coords != NULL);

  core->cur_coords = *coords;

  PICMAN_PAINT_CORE_GET_CLASS (core)->interpolate (core, drawable,
                                                 paint_options, time);
}

void
picman_paint_core_set_current_coords (PicmanPaintCore    *core,
                                    const PicmanCoords *coords)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (coords != NULL);

  core->cur_coords = *coords;
}

void
picman_paint_core_get_current_coords (PicmanPaintCore    *core,
                                    PicmanCoords       *coords)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (coords != NULL);

  *coords = core->cur_coords;

}

void
picman_paint_core_set_last_coords (PicmanPaintCore    *core,
                                 const PicmanCoords *coords)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (coords != NULL);

  core->last_coords = *coords;
}

void
picman_paint_core_get_last_coords (PicmanPaintCore *core,
                                 PicmanCoords    *coords)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (coords != NULL);

  *coords = core->last_coords;
}

/**
 * picman_paint_core_round_line:
 * @core:                 the #PicmanPaintCore
 * @options:              the #PicmanPaintOptions to use
 * @constrain_15_degrees: the modifier state
 *
 * Adjusts core->last_coords and core_cur_coords in preparation to
 * drawing a straight line. If @center_pixels is TRUE the endpoints
 * get pushed to the center of the pixels. This avoids artefacts
 * for e.g. the hard mode. The rounding of the slope to 15 degree
 * steps if ctrl is pressed happens, as does rounding the start and
 * end coordinates (which may be fractional in high zoom modes) to
 * the center of pixels.
 **/
void
picman_paint_core_round_line (PicmanPaintCore    *core,
                            PicmanPaintOptions *paint_options,
                            gboolean          constrain_15_degrees)
{
  g_return_if_fail (PICMAN_IS_PAINT_CORE (core));
  g_return_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options));

  if (picman_paint_options_get_brush_mode (paint_options) == PICMAN_BRUSH_HARD)
    {
      core->last_coords.x = floor (core->last_coords.x) + 0.5;
      core->last_coords.y = floor (core->last_coords.y) + 0.5;
      core->cur_coords.x  = floor (core->cur_coords.x ) + 0.5;
      core->cur_coords.y  = floor (core->cur_coords.y ) + 0.5;
    }

  if (constrain_15_degrees)
    picman_constrain_line (core->last_coords.x, core->last_coords.y,
                         &core->cur_coords.x, &core->cur_coords.y,
                         PICMAN_CONSTRAIN_LINE_15_DEGREES);
}


/*  protected functions  */

GeglBuffer *
picman_paint_core_get_paint_buffer (PicmanPaintCore    *core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  const PicmanCoords *coords,
                                  gint             *paint_buffer_x,
                                  gint             *paint_buffer_y)
{
  GeglBuffer *paint_buffer;

  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), NULL);
  g_return_val_if_fail (PICMAN_IS_DRAWABLE (drawable), NULL);
  g_return_val_if_fail (picman_item_is_attached (PICMAN_ITEM (drawable)), NULL);
  g_return_val_if_fail (PICMAN_IS_PAINT_OPTIONS (paint_options), NULL);
  g_return_val_if_fail (coords != NULL, NULL);
  g_return_val_if_fail (paint_buffer_x != NULL, NULL);
  g_return_val_if_fail (paint_buffer_y != NULL, NULL);

  paint_buffer =
    PICMAN_PAINT_CORE_GET_CLASS (core)->get_paint_buffer (core, drawable,
                                                        paint_options,
                                                        coords,
                                                        paint_buffer_x,
                                                        paint_buffer_y);

  core->paint_buffer_x = *paint_buffer_x;
  core->paint_buffer_y = *paint_buffer_y;

  return paint_buffer;
}

GeglBuffer *
picman_paint_core_get_orig_image (PicmanPaintCore *core)
{
  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), NULL);
  g_return_val_if_fail (core->undo_buffer != NULL, NULL);

  return core->undo_buffer;
}

GeglBuffer *
picman_paint_core_get_orig_proj (PicmanPaintCore *core)
{
  g_return_val_if_fail (PICMAN_IS_PAINT_CORE (core), NULL);
  g_return_val_if_fail (core->saved_proj_buffer != NULL, NULL);

  return core->saved_proj_buffer;
}

void
picman_paint_core_paste (PicmanPaintCore            *core,
                       GeglBuffer               *paint_mask,
                       const GeglRectangle      *paint_mask_rect,
                       PicmanDrawable             *drawable,
                       gdouble                   paint_opacity,
                       gdouble                   image_opacity,
                       PicmanLayerModeEffects      paint_mode,
                       PicmanPaintApplicationMode  mode)
{
  gint width  = gegl_buffer_get_width  (core->paint_buffer);
  gint height = gegl_buffer_get_height (core->paint_buffer);

  /*  If the mode is CONSTANT:
   *   combine the canvas buf, the paint mask to the canvas buffer
   */
  if (mode == PICMAN_PAINT_CONSTANT)
    {
      /* Some tools (ink) paint the mask to paint_core->canvas_buffer
       * directly. Don't need to copy it in this case.
       */
      if (paint_mask != core->canvas_buffer)
        {
          picman_gegl_combine_mask_weird (paint_mask, paint_mask_rect,
                                        core->canvas_buffer,
                                        GEGL_RECTANGLE (core->paint_buffer_x,
                                                        core->paint_buffer_y,
                                                        width, height),
                                        paint_opacity,
                                        PICMAN_IS_AIRBRUSH (core));
        }

      picman_gegl_apply_mask (core->canvas_buffer,
                            GEGL_RECTANGLE (core->paint_buffer_x,
                                            core->paint_buffer_y,
                                            width, height),
                            core->paint_buffer,
                            GEGL_RECTANGLE (0, 0, width, height),
                            1.0);

      picman_applicator_set_src_buffer (core->applicator,
                                      core->undo_buffer);
    }
  /*  Otherwise:
   *   combine the canvas buf and the paint mask to the canvas buf
   */
  else
    {
      picman_gegl_apply_mask (paint_mask, paint_mask_rect,
                            core->paint_buffer,
                            GEGL_RECTANGLE (0, 0, width, height),
                            paint_opacity);

      picman_applicator_set_src_buffer (core->applicator,
                                      picman_drawable_get_buffer (drawable));
    }

  picman_applicator_set_apply_buffer (core->applicator,
                                    core->paint_buffer);
  picman_applicator_set_apply_offset (core->applicator,
                                    core->paint_buffer_x,
                                    core->paint_buffer_y);

  picman_applicator_set_mode (core->applicator,
                            image_opacity, paint_mode);

  /*  apply the paint area to the image  */
  picman_applicator_blit (core->applicator,
                        GEGL_RECTANGLE (core->paint_buffer_x,
                                        core->paint_buffer_y,
                                        width, height));

  /*  Update the undo extents  */
  core->x1 = MIN (core->x1, core->paint_buffer_x);
  core->y1 = MIN (core->y1, core->paint_buffer_y);
  core->x2 = MAX (core->x2, core->paint_buffer_x + width);
  core->y2 = MAX (core->y2, core->paint_buffer_y + height);

  /*  Update the drawable  */
  picman_drawable_update (drawable,
                        core->paint_buffer_x,
                        core->paint_buffer_y,
                        width, height);
}

/* This works similarly to picman_paint_core_paste. However, instead of
 * combining the canvas to the paint core drawable using one of the
 * combination modes, it uses a "replace" mode (i.e. transparent
 * pixels in the canvas erase the paint core drawable).

 * When not drawing on alpha-enabled images, it just paints using
 * NORMAL mode.
 */
void
picman_paint_core_replace (PicmanPaintCore            *core,
                         GeglBuffer               *paint_mask,
                         const GeglRectangle      *paint_mask_rect,
                         PicmanDrawable             *drawable,
                         gdouble                   paint_opacity,
                         gdouble                   image_opacity,
                         PicmanPaintApplicationMode  mode)
{
  GeglRectangle mask_rect;
  gint          width, height;

  if (! picman_drawable_has_alpha (drawable))
    {
      picman_paint_core_paste (core, paint_mask, paint_mask_rect,
                             drawable,
                             paint_opacity,
                             image_opacity, PICMAN_NORMAL_MODE,
                             mode);
      return;
    }

  width  = gegl_buffer_get_width  (core->paint_buffer);
  height = gegl_buffer_get_height (core->paint_buffer);

  if (mode == PICMAN_PAINT_CONSTANT &&

      /* Some tools (ink) paint the mask to paint_core->canvas_buffer
       * directly. Don't need to copy it in this case.
       */
      paint_mask != core->canvas_buffer)
    {
      /* combine the paint mask and the canvas buffer */
      picman_gegl_combine_mask_weird (paint_mask, paint_mask_rect,
                                    core->canvas_buffer,
                                    GEGL_RECTANGLE (core->paint_buffer_x,
                                                    core->paint_buffer_y,
                                                    width, height),
                                    paint_opacity,
                                    PICMAN_IS_AIRBRUSH (core));

      /* initialize the maskPR from the canvas buffer */
      paint_mask = core->canvas_buffer;

      mask_rect = *GEGL_RECTANGLE (core->paint_buffer_x,
                                   core->paint_buffer_y,
                                   width, height);
    }
  else
    {
      mask_rect = *paint_mask_rect;
    }

  /*  apply the paint area to the image  */
  picman_drawable_replace_buffer (drawable, core->paint_buffer,
                                GEGL_RECTANGLE (0, 0, width, height),
                                FALSE, NULL,
                                image_opacity,
                                paint_mask, &mask_rect,
                                core->paint_buffer_x,
                                core->paint_buffer_y);

  /*  Update the undo extents  */
  core->x1 = MIN (core->x1, core->paint_buffer_x);
  core->y1 = MIN (core->y1, core->paint_buffer_y);
  core->x2 = MAX (core->x2, core->paint_buffer_x + width);
  core->y2 = MAX (core->y2, core->paint_buffer_y + height);

  /*  Update the drawable  */
  picman_drawable_update (drawable,
                        core->paint_buffer_x,
                        core->paint_buffer_y,
                        width, height);
}

/**
 * Smooth and store coords in the stroke buffer
 */

void
picman_paint_core_smooth_coords (PicmanPaintCore    *core,
                               PicmanPaintOptions *paint_options,
                               PicmanCoords       *coords)
{
  PicmanSmoothingOptions *smoothing_options = paint_options->smoothing_options;
  GArray               *history           = core->stroke_buffer;

  if (core->stroke_buffer == NULL)
    return; /* Paint core has not initalized yet */

  if (smoothing_options->use_smoothing &&
      smoothing_options->smoothing_quality > 0)
    {
      gint       i;
      guint      length;
      gint       min_index;
      gdouble    gaussian_weight  = 0.0;
      gdouble    gaussian_weight2 = SQR (smoothing_options->smoothing_factor);
      gdouble    velocity_sum     = 0.0;
      gdouble    scale_sum        = 0.0;

      g_array_append_val (history, *coords);

      if (history->len < 2)
        return; /* Just dont bother, nothing to do */

      coords->x = coords->y = 0.0;

      length = MIN (smoothing_options->smoothing_quality, history->len);

      min_index = history->len - length;

      if (gaussian_weight2 != 0.0)
        gaussian_weight = 1 / (sqrt (2 * G_PI) * smoothing_options->smoothing_factor);

      for (i = history->len - 1; i >= min_index; i--)
        {
          gdouble     rate        = 0.0;
          PicmanCoords *next_coords = &g_array_index (history,
                                                    PicmanCoords, i);

          if (gaussian_weight2 != 0.0)
            {
              /* We use gaussian function with velocity as a window function */
              velocity_sum += next_coords->velocity * 100;
              rate = gaussian_weight * exp (-velocity_sum * velocity_sum /
                                            (2 * gaussian_weight2));
            }

          scale_sum += rate;
          coords->x += rate * next_coords->x;
          coords->y += rate * next_coords->y;
        }

      if (scale_sum != 0.0)
        {
          coords->x /= scale_sum;
          coords->y /= scale_sum;
        }
    }
}
