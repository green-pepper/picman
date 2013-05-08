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

#include <gegl.h>

#include "libpicmanbase/picmanbase.h"

#include "paint-types.h"

#include "core/picman.h"
#include "core/picmandrawable.h"
#include "core/picmandynamics.h"
#include "core/picmanerror.h"
#include "core/picmanimage.h"
#include "core/picmanpattern.h"
#include "core/picmanpickable.h"

#include "picmanclone.h"
#include "picmancloneoptions.h"

#include "picman-intl.h"


static gboolean   picman_clone_start      (PicmanPaintCore     *paint_core,
                                         PicmanDrawable      *drawable,
                                         PicmanPaintOptions  *paint_options,
                                         const PicmanCoords  *coords,
                                         GError           **error);

static void       picman_clone_motion     (PicmanSourceCore    *source_core,
                                         PicmanDrawable      *drawable,
                                         PicmanPaintOptions  *paint_options,
                                         const PicmanCoords  *coords,
                                         gdouble            opacity,
                                         PicmanPickable      *src_pickable,
                                         GeglBuffer        *src_buffer,
                                         GeglRectangle     *src_rect,
                                         gint               src_offset_x,
                                         gint               src_offset_y,
                                         GeglBuffer        *paint_buffer,
                                         gint               paint_buffer_x,
                                         gint               paint_buffer_y,
                                         gint               paint_area_offset_x,
                                         gint               paint_area_offset_y,
                                         gint               paint_area_width,
                                         gint               paint_area_height);

static gboolean   picman_clone_use_source (PicmanSourceCore    *source_core,
                                         PicmanSourceOptions *options);


G_DEFINE_TYPE (PicmanClone, picman_clone, PICMAN_TYPE_SOURCE_CORE)

#define parent_class picman_clone_parent_class


void
picman_clone_register (Picman                      *picman,
                     PicmanPaintRegisterCallback  callback)
{
  (* callback) (picman,
                PICMAN_TYPE_CLONE,
                PICMAN_TYPE_CLONE_OPTIONS,
                "picman-clone",
                _("Clone"),
                "picman-tool-clone");
}

static void
picman_clone_class_init (PicmanCloneClass *klass)
{
  PicmanPaintCoreClass  *paint_core_class  = PICMAN_PAINT_CORE_CLASS (klass);
  PicmanSourceCoreClass *source_core_class = PICMAN_SOURCE_CORE_CLASS (klass);

  paint_core_class->start       = picman_clone_start;

  source_core_class->use_source = picman_clone_use_source;
  source_core_class->motion     = picman_clone_motion;
}

static void
picman_clone_init (PicmanClone *clone)
{
}

static gboolean
picman_clone_start (PicmanPaintCore     *paint_core,
                  PicmanDrawable      *drawable,
                  PicmanPaintOptions  *paint_options,
                  const PicmanCoords  *coords,
                  GError           **error)
{
  PicmanCloneOptions *options = PICMAN_CLONE_OPTIONS (paint_options);

  if (! PICMAN_PAINT_CORE_CLASS (parent_class)->start (paint_core, drawable,
                                                     paint_options, coords,
                                                     error))
    {
      return FALSE;
    }

  if (options->clone_type == PICMAN_PATTERN_CLONE)
    {
      if (! picman_context_get_pattern (PICMAN_CONTEXT (options)))
        {
          g_set_error_literal (error, PICMAN_ERROR, PICMAN_FAILED,
			       _("No patterns available for use with this tool."));
          return FALSE;
        }
    }

  return TRUE;
}

static void
picman_clone_motion (PicmanSourceCore   *source_core,
                   PicmanDrawable     *drawable,
                   PicmanPaintOptions *paint_options,
                   const PicmanCoords *coords,
                   gdouble           opacity,
                   PicmanPickable     *src_pickable,
                   GeglBuffer       *src_buffer,
                   GeglRectangle    *src_rect,
                   gint              src_offset_x,
                   gint              src_offset_y,
                   GeglBuffer       *paint_buffer,
                   gint              paint_buffer_x,
                   gint              paint_buffer_y,
                   gint              paint_area_offset_x,
                   gint              paint_area_offset_y,
                   gint              paint_area_width,
                   gint              paint_area_height)
{
  PicmanPaintCore     *paint_core     = PICMAN_PAINT_CORE (source_core);
  PicmanCloneOptions  *options        = PICMAN_CLONE_OPTIONS (paint_options);
  PicmanSourceOptions *source_options = PICMAN_SOURCE_OPTIONS (paint_options);
  PicmanContext       *context        = PICMAN_CONTEXT (paint_options);
  PicmanImage         *image          = picman_item_get_image (PICMAN_ITEM (drawable));
  gdouble            fade_point;
  gdouble            force;

  if (picman_source_core_use_source (source_core, source_options))
    {
      gegl_buffer_copy (src_buffer,
                        GEGL_RECTANGLE (src_rect->x,
                                        src_rect->y,
                                        paint_area_width,
                                        paint_area_height),
                        paint_buffer,
                        GEGL_RECTANGLE (paint_area_offset_x,
                                        paint_area_offset_y,
                                        0, 0));
    }
  else if (options->clone_type == PICMAN_PATTERN_CLONE)
    {
      PicmanPattern *pattern    = picman_context_get_pattern (context);
      GeglBuffer  *src_buffer = picman_pattern_create_buffer (pattern);

      gegl_buffer_set_pattern (paint_buffer,
                               GEGL_RECTANGLE (paint_area_offset_x,
                                               paint_area_offset_y,
                                               paint_area_width,
                                               paint_area_height),
                               src_buffer,
                               - paint_buffer_x - src_offset_x,
                               - paint_buffer_y - src_offset_y);

      g_object_unref (src_buffer);
    }
  else
    {
      g_return_if_reached ();
    }

  fade_point = picman_paint_options_get_fade (paint_options, image,
                                            paint_core->pixel_dist);

  force = picman_dynamics_get_linear_value (PICMAN_BRUSH_CORE (paint_core)->dynamics,
                                          PICMAN_DYNAMICS_OUTPUT_FORCE,
                                          coords,
                                          paint_options,
                                          fade_point);

  picman_brush_core_paste_canvas (PICMAN_BRUSH_CORE (paint_core), drawable,
                                coords,
                                MIN (opacity, PICMAN_OPACITY_OPAQUE),
                                picman_context_get_opacity (context),
                                picman_context_get_paint_mode (context),
                                picman_paint_options_get_brush_mode (paint_options),
                                force,

                                /* In fixed mode, paint incremental so the
                                 * individual brushes are properly applied
                                 * on top of each other.
                                 * Otherwise the stuff we paint is seamless
                                 * and we don't need intermediate masking.
                                 */
                                source_options->align_mode ==
                                PICMAN_SOURCE_ALIGN_FIXED ?
                                PICMAN_PAINT_INCREMENTAL : PICMAN_PAINT_CONSTANT);
}

static gboolean
picman_clone_use_source (PicmanSourceCore    *source_core,
                       PicmanSourceOptions *options)
{
  return PICMAN_CLONE_OPTIONS (options)->clone_type == PICMAN_IMAGE_CLONE;
}
