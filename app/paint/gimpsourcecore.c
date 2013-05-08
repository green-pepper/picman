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

#include "paint-types.h"

#include "gegl/picman-gegl-utils.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanpickable.h"

#include "picmansourcecore.h"
#include "picmansourceoptions.h"

#include "picman-intl.h"


enum
{
  PROP_0,
  PROP_SRC_DRAWABLE,
  PROP_SRC_X,
  PROP_SRC_Y
};


static void     picman_source_core_set_property    (GObject           *object,
                                                  guint              property_id,
                                                  const GValue      *value,
                                                  GParamSpec        *pspec);
static void     picman_source_core_get_property    (GObject           *object,
                                                  guint              property_id,
                                                  GValue            *value,
                                                  GParamSpec        *pspec);

static gboolean picman_source_core_start           (PicmanPaintCore     *paint_core,
                                                  PicmanDrawable      *drawable,
                                                  PicmanPaintOptions  *paint_options,
                                                  const PicmanCoords  *coords,
                                                  GError           **error);
static void     picman_source_core_paint           (PicmanPaintCore     *paint_core,
                                                  PicmanDrawable      *drawable,
                                                  PicmanPaintOptions  *paint_options,
                                                  const PicmanCoords  *coords,
                                                  PicmanPaintState     paint_state,
                                                  guint32            time);

#if 0
static void     picman_source_core_motion          (PicmanSourceCore    *source_core,
                                                  PicmanDrawable      *drawable,
                                                  PicmanPaintOptions  *paint_options,
                                                  const PicmanCoords  *coords);
#endif

static gboolean picman_source_core_real_use_source (PicmanSourceCore    *source_core,
                                                  PicmanSourceOptions *options);
static GeglBuffer *
                picman_source_core_real_get_source (PicmanSourceCore    *source_core,
                                                  PicmanDrawable      *drawable,
                                                  PicmanPaintOptions  *paint_options,
                                                  PicmanPickable      *src_pickable,
                                                  gint               src_offset_x,
                                                  gint               src_offset_y,
                                                  GeglBuffer        *paint_buffer,
                                                  gint               paint_buffer_x,
                                                  gint               paint_buffer_y,
                                                  gint              *paint_area_offset_x,
                                                  gint              *paint_area_offset_y,
                                                  gint              *paint_area_width,
                                                  gint              *paint_area_height,
                                                  GeglRectangle     *src_rect);

static void    picman_source_core_set_src_drawable (PicmanSourceCore    *source_core,
                                                  PicmanDrawable      *drawable);


G_DEFINE_TYPE (PicmanSourceCore, picman_source_core, PICMAN_TYPE_BRUSH_CORE)

#define parent_class picman_source_core_parent_class


static void
picman_source_core_class_init (PicmanSourceCoreClass *klass)
{
  GObjectClass       *object_class     = G_OBJECT_CLASS (klass);
  PicmanPaintCoreClass *paint_core_class = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanBrushCoreClass *brush_core_class = PICMAN_BRUSH_CORE_CLASS (klass);

  object_class->set_property               = picman_source_core_set_property;
  object_class->get_property               = picman_source_core_get_property;

  paint_core_class->start                  = picman_source_core_start;
  paint_core_class->paint                  = picman_source_core_paint;

  brush_core_class->handles_changing_brush = TRUE;

  klass->use_source                        = picman_source_core_real_use_source;
  klass->get_source                        = picman_source_core_real_get_source;
  klass->motion                            = NULL;

  g_object_class_install_property (object_class, PROP_SRC_DRAWABLE,
                                   g_param_spec_object ("src-drawable",
                                                        NULL, NULL,
                                                        PICMAN_TYPE_DRAWABLE,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_SRC_X,
                                   g_param_spec_double ("src-x", NULL, NULL,
                                                        0, PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));

  g_object_class_install_property (object_class, PROP_SRC_Y,
                                   g_param_spec_double ("src-y", NULL, NULL,
                                                        0, PICMAN_MAX_IMAGE_SIZE,
                                                        0.0,
                                                        PICMAN_PARAM_READWRITE));
}

static void
picman_source_core_init (PicmanSourceCore *source_core)
{
  source_core->set_source   = FALSE;

  source_core->src_drawable = NULL;
  source_core->src_x        = 0.0;
  source_core->src_y        = 0.0;

  source_core->orig_src_x   = 0.0;
  source_core->orig_src_y   = 0.0;

  source_core->offset_x     = 0.0;
  source_core->offset_y     = 0.0;
  source_core->first_stroke = TRUE;
}

static void
picman_source_core_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  PicmanSourceCore *source_core = PICMAN_SOURCE_CORE (object);

  switch (property_id)
    {
    case PROP_SRC_DRAWABLE:
      picman_source_core_set_src_drawable (source_core,
                                         g_value_get_object (value));
      break;
    case PROP_SRC_X:
      source_core->src_x = g_value_get_double (value);
      break;
    case PROP_SRC_Y:
      source_core->src_y = g_value_get_double (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
picman_source_core_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  PicmanSourceCore *source_core = PICMAN_SOURCE_CORE (object);

  switch (property_id)
    {
    case PROP_SRC_DRAWABLE:
      g_value_set_object (value, source_core->src_drawable);
      break;
    case PROP_SRC_X:
      g_value_set_int (value, source_core->src_x);
      break;
    case PROP_SRC_Y:
      g_value_set_int (value, source_core->src_y);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
picman_source_core_start (PicmanPaintCore     *paint_core,
                        PicmanDrawable      *drawable,
                        PicmanPaintOptions  *paint_options,
                        const PicmanCoords  *coords,
                        GError           **error)
{
  PicmanSourceCore    *source_core = PICMAN_SOURCE_CORE (paint_core);
  PicmanSourceOptions *options     = PICMAN_SOURCE_OPTIONS (paint_options);

  if (! PICMAN_PAINT_CORE_CLASS (parent_class)->start (paint_core, drawable,
                                                     paint_options, coords,
                                                     error))
    {
      return FALSE;
    }

  paint_core->use_saved_proj = FALSE;

  if (! source_core->set_source &&
      picman_source_core_use_source (source_core, options))
    {
      if (! source_core->src_drawable)
        {
          g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			       _("Set a source image first."));
          return FALSE;
        }

      if (options->sample_merged &&
          picman_item_get_image (PICMAN_ITEM (source_core->src_drawable)) ==
          picman_item_get_image (PICMAN_ITEM (drawable)))
        {
          paint_core->use_saved_proj = TRUE;
        }
    }

  return TRUE;
}

static void
picman_source_core_paint (PicmanPaintCore    *paint_core,
                        PicmanDrawable     *drawable,
                        PicmanPaintOptions *paint_options,
                        const PicmanCoords *coords,
                        PicmanPaintState    paint_state,
                        guint32           time)
{
  PicmanSourceCore    *source_core = PICMAN_SOURCE_CORE (paint_core);
  PicmanSourceOptions *options     = PICMAN_SOURCE_OPTIONS (paint_options);

  switch (paint_state)
    {
    case PICMAN_PAINT_STATE_INIT:
      if (source_core->set_source)
        {
          picman_source_core_set_src_drawable (source_core, drawable);

          source_core->src_x = coords->x;
          source_core->src_y = coords->y;

          source_core->first_stroke = TRUE;
        }
      else if (options->align_mode == PICMAN_SOURCE_ALIGN_NO)
        {
          source_core->orig_src_x = source_core->src_x;
          source_core->orig_src_y = source_core->src_y;

          source_core->first_stroke = TRUE;
        }
      break;

    case PICMAN_PAINT_STATE_MOTION:
      if (source_core->set_source)
        {
          /*  If the control key is down, move the src target and return */

          source_core->src_x = coords->x;
          source_core->src_y = coords->y;

          source_core->first_stroke = TRUE;
        }
      else
        {
          /*  otherwise, update the target  */

          gint dest_x;
          gint dest_y;

          dest_x = coords->x;
          dest_y = coords->y;

          if (options->align_mode == PICMAN_SOURCE_ALIGN_REGISTERED)
            {
              source_core->offset_x = 0;
              source_core->offset_y = 0;
            }
          else if (options->align_mode == PICMAN_SOURCE_ALIGN_FIXED)
            {
              source_core->offset_x = source_core->src_x - dest_x;
              source_core->offset_y = source_core->src_y - dest_y;
            }
          else if (source_core->first_stroke)
            {
              source_core->offset_x = source_core->src_x - dest_x;
              source_core->offset_y = source_core->src_y - dest_y;

              source_core->first_stroke = FALSE;
            }

          source_core->src_x = dest_x + source_core->offset_x;
          source_core->src_y = dest_y + source_core->offset_y;

          picman_source_core_motion (source_core, drawable, paint_options, coords);
        }
      break;

    case PICMAN_PAINT_STATE_FINISH:
      if (options->align_mode == PICMAN_SOURCE_ALIGN_NO &&
          ! source_core->first_stroke)
        {
          source_core->src_x = source_core->orig_src_x;
          source_core->src_y = source_core->orig_src_y;
        }
      break;

    default:
      break;
    }

  g_object_notify (G_OBJECT (source_core), "src-x");
  g_object_notify (G_OBJECT (source_core), "src-y");
}

void
picman_source_core_motion (PicmanSourceCore   *source_core,
                         PicmanDrawable     *drawable,
                         PicmanPaintOptions *paint_options,
                         const PicmanCoords *coords)

{
  PicmanPaintCore     *paint_core   = PICMAN_PAINT_CORE (source_core);
  PicmanSourceOptions *options      = PICMAN_SOURCE_OPTIONS (paint_options);
  PicmanDynamics      *dynamics     = PICMAN_BRUSH_CORE (paint_core)->dynamics;
  PicmanImage         *image        = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanPickable      *src_pickable = NULL;
  GeglBuffer        *src_buffer   = NULL;
  GeglRectangle      src_rect;
  gint               src_offset_x;
  gint               src_offset_y;
  GeglBuffer        *paint_buffer;
  gint               paint_buffer_x;
  gint               paint_buffer_y;
  gint               paint_area_offset_x;
  gint               paint_area_offset_y;
  gint               paint_area_width;
  gint               paint_area_height;
  gdouble            fade_point;
  gdouble            opacity;

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  opacity = picman_dynamics_get_linear_value (dynamics,
                                            PICMAN_DYNAMICS_OUTPUT_OPACITY,
                                            coords,
                                            paint_options,
                                            fade_point);
  if (opacity == 0.0)
    return;

  src_offset_x = source_core->offset_x;
  src_offset_y = source_core->offset_y;

  if (picman_source_core_use_source (source_core, options))
    {
      src_pickable = PICMAN_PICKABLE (source_core->src_drawable);

      if (options->sample_merged)
        {
          PicmanImage *src_image = picman_pickable_get_image (src_pickable);
          gint       off_x, off_y;

          src_pickable = PICMAN_PICKABLE (picman_image_get_projection (src_image));

          picman_item_get_offset (PICMAN_ITEM (source_core->src_drawable),
                                &off_x, &off_y);

          src_offset_x += off_x;
          src_offset_y += off_y;
        }

      picman_pickable_flush (src_pickable);
    }

  paint_buffer = picman_paint_core_get_paint_buffer (paint_core, drawable,
                                                   paint_options, coords,
                                                   &paint_buffer_x,
                                                   &paint_buffer_y);
  if (! paint_buffer)
    return;

  paint_area_offset_x = 0;
  paint_area_offset_y = 0;
  paint_area_width    = gegl_buffer_get_width  (paint_buffer);
  paint_area_height   = gegl_buffer_get_height (paint_buffer);

  if (picman_source_core_use_source (source_core, options))
    {
      src_buffer =
        PICMAN_SOURCE_CORE_GET_CLASS (source_core)->get_source (source_core,
                                                              drawable,
                                                              paint_options,
                                                              src_pickable,
                                                              src_offset_x,
                                                              src_offset_y,
                                                              paint_buffer,
                                                              paint_buffer_x,
                                                              paint_buffer_y,
                                                              &paint_area_offset_x,
                                                              &paint_area_offset_y,
                                                              &paint_area_width,
                                                              &paint_area_height,
                                                              &src_rect);
      if (! src_buffer)
        return;
    }

  /*  Set the paint buffer to transparent  */
  gegl_buffer_clear (paint_buffer, NULL);

  PICMAN_SOURCE_CORE_GET_CLASS (source_core)->motion (source_core,
                                                    drawable,
                                                    paint_options,
                                                    coords,
                                                    opacity,
                                                    src_pickable,
                                                    src_buffer,
                                                    &src_rect,
                                                    src_offset_x,
                                                    src_offset_y,
                                                    paint_buffer,
                                                    paint_buffer_x,
                                                    paint_buffer_y,
                                                    paint_area_offset_x,
                                                    paint_area_offset_y,
                                                    paint_area_width,
                                                    paint_area_height);

  if (src_buffer)
    g_object_unref (src_buffer);
}

gboolean
picman_source_core_use_source (PicmanSourceCore    *source_core,
                             PicmanSourceOptions *options)
{
  return PICMAN_SOURCE_CORE_GET_CLASS (source_core)->use_source (source_core,
                                                               options);
}

static gboolean
picman_source_core_real_use_source (PicmanSourceCore    *source_core,
                                  PicmanSourceOptions *options)
{
  return TRUE;
}

static GeglBuffer *
picman_source_core_real_get_source (PicmanSourceCore   *source_core,
                                  PicmanDrawable     *drawable,
                                  PicmanPaintOptions *paint_options,
                                  PicmanPickable     *src_pickable,
                                  gint              src_offset_x,
                                  gint              src_offset_y,
                                  GeglBuffer       *paint_buffer,
                                  gint              paint_buffer_x,
                                  gint              paint_buffer_y,
                                  gint             *paint_area_offset_x,
                                  gint             *paint_area_offset_y,
                                  gint             *paint_area_width,
                                  gint             *paint_area_height,
                                  GeglRectangle    *src_rect)
{
  PicmanSourceOptions *options    = PICMAN_SOURCE_OPTIONS (paint_options);
  PicmanImage         *image      = picman_item_get_image (PICMAN_ITEM (drawable));
  PicmanImage         *src_image  = picman_pickable_get_image (src_pickable);
  GeglBuffer        *src_buffer = picman_pickable_get_buffer (src_pickable);
  GeglBuffer        *dest_buffer;
  gint               x, y;
  gint               width, height;

  if (! picman_rectangle_intersect (paint_buffer_x + src_offset_x,
                                  paint_buffer_y + src_offset_y,
                                  gegl_buffer_get_width  (paint_buffer),
                                  gegl_buffer_get_height (paint_buffer),
                                  0, 0,
                                  gegl_buffer_get_width  (src_buffer),
                                  gegl_buffer_get_height (src_buffer),
                                  &x, &y,
                                  &width, &height))
    {
      return FALSE;
    }

  /*  If the source image is different from the destination,
   *  then we should copy straight from the source image
   *  to the canvas.
   *  Otherwise, we need a call to get_orig_image to make sure
   *  we get a copy of the unblemished (offset) image
   */
  if ((  options->sample_merged && (src_image                 != image)) ||
      (! options->sample_merged && (source_core->src_drawable != drawable)))
    {
      dest_buffer = src_buffer;
    }
  else
    {
      /*  get the original image  */
      if (options->sample_merged)
        dest_buffer = picman_paint_core_get_orig_proj (PICMAN_PAINT_CORE (source_core));
      else
        dest_buffer = picman_paint_core_get_orig_image (PICMAN_PAINT_CORE (source_core));
    }

  *paint_area_offset_x = x - (paint_buffer_x + src_offset_x);
  *paint_area_offset_y = y - (paint_buffer_y + src_offset_y);
  *paint_area_width    = width;
  *paint_area_height   = height;

  *src_rect = *GEGL_RECTANGLE (x, y, width, height);

  return g_object_ref (dest_buffer);
}

static void
picman_source_core_src_drawable_removed (PicmanDrawable   *drawable,
                                       PicmanSourceCore *source_core)
{
  if (drawable == source_core->src_drawable)
    {
      source_core->src_drawable = NULL;
    }

  g_signal_handlers_disconnect_by_func (drawable,
                                        picman_source_core_src_drawable_removed,
                                        source_core);
}

static void
picman_source_core_set_src_drawable (PicmanSourceCore *source_core,
                                   PicmanDrawable   *drawable)
{
  if (source_core->src_drawable == drawable)
    return;

  if (source_core->src_drawable)
    g_signal_handlers_disconnect_by_func (source_core->src_drawable,
                                          picman_source_core_src_drawable_removed,
                                          source_core);

  source_core->src_drawable = drawable;

  if (source_core->src_drawable)
    g_signal_connect (source_core->src_drawable, "removed",
                      G_CALLBACK (picman_source_core_src_drawable_removed),
                      source_core);

  g_object_notify (G_OBJECT (source_core), "src-drawable");
}
